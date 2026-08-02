extern "C" {
    #include "third_party/vice-3.3/src/rs232drv/rs232bmc.h"
}

#include <string.h>

#include <circle/logger.h>
#include <circle/net/dnsclient.h>
#include <circle/net/in.h>
#include <circle/net/netsubsystem.h>
#include <circle/net/socket.h>
#include <circle/netdevice.h>

namespace {

const unsigned kQueueSize = 4096;
const unsigned kCommandSize = 256;
const char FromBmcModem[] = "bmc-modem";

class BmcModem {
public:
  BmcModem()
      : socket_(0), read_(0), write_(0), transmitLength_(0),
        commandLength_(0), open_(false), dataMode_(false), echo_(true),
        plusCount_(0) {}

  ~BmcModem() { Disconnect(); }

  int Open(int device) {
    Reset();
    open_ = true;
    CLogger::Get()->Write(FromBmcModem, LogNotice,
                          "opened RS232 device %d", device + 1);
    return 0;
  }

  void Close(int) {
    Disconnect();
    open_ = false;
  }

  void Reset() {
    Disconnect();
    read_ = 0;
    write_ = 0;
    transmitLength_ = 0;
    commandLength_ = 0;
    dataMode_ = false;
    echo_ = true;
    plusCount_ = 0;
  }

  int Put(uint8_t byte) {
    if (!open_) {
      return -1;
    }

    Pump();
    if (dataMode_) {
      PutData(byte);
    } else {
      PutCommand(byte);
    }
    Pump();
    return 0;
  }

  int Get(uint8_t *byte) {
    if (!open_ || byte == 0) {
      return -1;
    }

    Pump();
    if (read_ == write_) {
      return 0;
    }

    *byte = queue_[read_];
    read_ = (read_ + 1) % kQueueSize;
    return 1;
  }

private:
  void Disconnect() {
    delete socket_;
    socket_ = 0;
    dataMode_ = false;
    plusCount_ = 0;
  }

  unsigned Available() const {
    return (write_ + kQueueSize - read_) % kQueueSize;
  }

  unsigned Free() const { return kQueueSize - Available() - 1; }

  void QueueByte(uint8_t byte) {
    if (Free() == 0) {
      return;
    }
    queue_[write_] = byte;
    write_ = (write_ + 1) % kQueueSize;
  }

  void QueueText(const char *text) {
    while (*text != '\0') {
      QueueByte(static_cast<uint8_t>(*text++));
    }
  }

  void Result(const char *text) {
    QueueText("\r\n");
    QueueText(text);
    QueueText("\r\n");
  }

  void PutCommand(uint8_t byte) {
    if (echo_) {
      QueueByte(byte);
    }

    if (byte == '\r' || byte == '\n') {
      if (commandLength_ != 0) {
        command_[commandLength_] = '\0';
        ExecuteCommand();
      }
      commandLength_ = 0;
      return;
    }

    if (byte == '\b' || byte == 0x14) {
      if (commandLength_ != 0) {
        --commandLength_;
      }
      return;
    }

    if (commandLength_ + 1 < kCommandSize) {
      command_[commandLength_++] = static_cast<char>(byte);
    }
  }

  void PutData(uint8_t byte) {
    if (byte == '+') {
      ++plusCount_;
      if (plusCount_ == 3) {
        dataMode_ = false;
        plusCount_ = 0;
        Result("OK");
      }
      return;
    }

    while (plusCount_ != 0) {
      QueueTransmit('+');
      --plusCount_;
    }
    QueueTransmit(byte);
  }

  void QueueTransmit(uint8_t byte) {
    if (transmitLength_ == sizeof transmit_) {
      return;
    }
    transmit_[transmitLength_++] = byte;
  }

  static char Upper(char value) {
    if (value >= 'a' && value <= 'z') {
      return static_cast<char>(value - 'a' + 'A');
    }
    return value;
  }

  bool HasPrefix(const char *prefix) const {
    for (unsigned index = 0; prefix[index] != '\0'; ++index) {
      if (Upper(command_[index]) != prefix[index]) {
        return false;
      }
    }
    return true;
  }

  void ExecuteCommand() {
    CLogger::Get()->Write(FromBmcModem, LogNotice, "received command: %s",
                          command_);
    if (commandLength_ < 2 || Upper(command_[0]) != 'A' ||
        Upper(command_[1]) != 'T') {
      Result("ERROR");
      return;
    }

    if (commandLength_ == 2) {
      Result("OK");
    } else if (HasPrefix("ATE0") && commandLength_ == 4) {
      echo_ = false;
      Result("OK");
    } else if (HasPrefix("ATE1") && commandLength_ == 4) {
      echo_ = true;
      Result("OK");
    } else if (HasPrefix("ATZ") && commandLength_ == 3) {
      Reset();
      Result("OK");
    } else if (HasPrefix("ATH") && commandLength_ == 3) {
      Disconnect();
      Result("OK");
    } else if (HasPrefix("ATI") && commandLength_ == 3) {
      Result("BMC64 WIFI MODEM");
      Result("OK");
    } else if (HasPrefix("ATO") && commandLength_ == 3 && socket_ != 0) {
      dataMode_ = true;
      Result("CONNECT");
    } else if (HasPrefix("ATD")) {
      Dial(command_ + 3);
    } else {
      Result("ERROR");
    }
  }

  void Dial(char *target) {
    if (Upper(command_[2]) != 'D') {
      Result("ERROR");
      return;
    }
    if (Upper(*target) == 'T' || Upper(*target) == 'P') {
      ++target;
    }
    if (*target == '\0') {
      Result("ERROR");
      return;
    }

    unsigned port = 23;
    char *portDelimiter = 0;
    for (char *cursor = target; *cursor != '\0'; ++cursor) {
      if (*cursor == ':') {
        portDelimiter = cursor;
      }
    }
    if (portDelimiter != 0) {
      *portDelimiter++ = '\0';
      port = 0;
      while (*portDelimiter >= '0' && *portDelimiter <= '9') {
        port = port * 10 + static_cast<unsigned>(*portDelimiter++ - '0');
      }
      if (*portDelimiter != '\0' || port == 0 || port > 65535) {
        Result("ERROR");
        return;
      }
    }

    CNetSubSystem *net = CNetSubSystem::Get();
    if (net == 0 || !net->IsRunning()) {
      Result("NO CARRIER");
      return;
    }

    CIPAddress address;
    CDNSClient dns(net);
    if (!dns.Resolve(target, &address)) {
      Result("NO CARRIER");
      return;
    }

    Disconnect();
    socket_ = new CSocket(net, IPPROTO_TCP);
    if (socket_ == 0 || socket_->Connect(address, static_cast<u16>(port)) < 0) {
      Disconnect();
      Result("NO CARRIER");
      return;
    }

    dataMode_ = true;
    Result("CONNECT");
  }

  void Pump() {
    if (socket_ == 0) {
      return;
    }

    FlushTransmit();
    ReceiveData();
  }

  void FlushTransmit() {
    if (transmitLength_ == 0) {
      return;
    }

    int sent = socket_->Send(transmit_, transmitLength_, MSG_DONTWAIT);
    if (sent > 0) {
      unsigned count = static_cast<unsigned>(sent);
      CLogger::Get()->Write(FromBmcModem, LogNotice,
                            "sent %u bytes, first byte $%02X", count,
                            transmit_[0]);
      if (count < transmitLength_) {
        memmove(transmit_, transmit_ + count, transmitLength_ - count);
      }
      transmitLength_ -= count;
    }
  }

  void ReceiveData() {
    unsigned free = Free();
    if (free == 0) {
      return;
    }

    uint8_t received[FRAME_BUFFER_SIZE];
    unsigned length = free < sizeof received ? free : sizeof received;
    int count = socket_->Receive(received, length, MSG_DONTWAIT);
    if (count < 0) {
      CLogger::Get()->Write(FromBmcModem, LogNotice,
                            "socket receive failed: %d", count);
      Disconnect();
      Result("NO CARRIER");
      return;
    }
    for (int index = 0; index < count; ++index) {
      QueueByte(received[index]);
    }
  }

private:
  CSocket *socket_;
  uint8_t queue_[kQueueSize];
  uint8_t transmit_[kQueueSize];
  unsigned read_;
  unsigned write_;
  unsigned transmitLength_;
  char command_[kCommandSize];
  unsigned commandLength_;
  bool open_;
  bool dataMode_;
  bool echo_;
  unsigned plusCount_;
};

BmcModem modem;

}  // namespace

extern "C" int bmcmodem_open(int device) { return modem.Open(device); }

extern "C" void bmcmodem_close(int device) { modem.Close(device); }

extern "C" int bmcmodem_putc(int device, uint8_t byte) {
  (void)device;
  return modem.Put(byte);
}

extern "C" int bmcmodem_getc(int device, uint8_t *byte) {
  (void)device;
  return modem.Get(byte);
}

extern "C" void bmcmodem_init(void) {
  modem.Reset();
  CLogger::Get()->Write(FromBmcModem, LogNotice, "initialized");
}

extern "C" void bmcmodem_reset(void) { modem.Reset(); }
