extern "C" {
    #include "third_party/vice-3.3/src/rs232drv/rs232bmc.h"
}

#include <string.h>

#include <circle/logger.h>
#include <circle/net/dnsclient.h>
#include <circle/net/error.h>
#include <circle/net/in.h>
#include <circle/net/netsubsystem.h>
#include <circle/net/socket.h>
#include <circle/netdevice.h>
#include <circle/sched/mutex.h>
#include <circle/sched/scheduler.h>
#include <circle/sched/task.h>
#include <circle/timer.h>

namespace {

const unsigned kQueueSize = 16384;
const unsigned kCommandSize = 256;
const unsigned kReceivePreviewSize = 96;
const unsigned kSocketSendSize = 256;
const u64 kEscapeGuardMicroseconds = 1000000;
const u64 kTelnetCandidateMicroseconds = 100000;
const uint8_t kTelnetIac = 255;
const uint8_t kTelnetWill = 251;
const uint8_t kTelnetWont = 252;
const uint8_t kTelnetDo = 253;
const uint8_t kTelnetDont = 254;
const uint8_t kTelnetSubnegotiation = 250;
const uint8_t kTelnetSubnegotiationEnd = 240;
const uint8_t kTelnetBinary = 0;
const char FromBmcModem[] = "bmc-modem";

class BmcModem;

class NetworkTask : public CTask {
public:
  explicit NetworkTask(BmcModem *modem)
      : CTask(12 * 1024), modem_(modem) {
    SetName("bmc-modem-net");
  }

  void Run() override;

private:
  BmcModem *modem_;
};

class BmcModem {
public:
  BmcModem()
      : socket_(0), read_(0), write_(0), transmitLength_(0),
        commandLength_(0), open_(false), dataMode_(false), echo_(true),
        commandInputActive_(false), receiveEnabled_(true),
        remoteDisconnectPending_(false), bps_(0),
        plusCount_(0), worker_(0), telnetState_(kTelnetData),
        telnetCommand_(0), telnetDetected_(false), telnetOptions_(0),
        receivedTotal_(0), deliveredTotal_(0), sentTotal_(0),
        aciaReadTotal_(0),
        receiveDropped_(0), transmitDropped_(0), receiveHighWater_(0),
        transmitHighWater_(0), lastTransmitTime_(0), escapeDeadline_(0),
        telnetCandidateDeadline_(0) {}

  ~BmcModem() { Disconnect(); }

  void Initialize() {
    CMutexGuard guard(lock_);
    if (worker_ == 0) {
      worker_ = new NetworkTask(this);
    }
    Reset();
    CLogger::Get()->Write(FromBmcModem, LogNotice, "initialized");
  }

  int Open(int device) {
    CMutexGuard guard(lock_);
    Reset();
    open_ = true;
    CLogger::Get()->Write(FromBmcModem, LogNotice,
                          "opened RS232 device %d", device + 1);
    return 0;
  }

  void Close(int) {
    CMutexGuard guard(lock_);
    Disconnect();
    open_ = false;
  }

  void Reset() {
    CMutexGuard guard(lock_);
    Disconnect();
    read_ = 0;
    write_ = 0;
    transmitLength_ = 0;
    commandLength_ = 0;
    dataMode_ = false;
    echo_ = true;
    commandInputActive_ = false;
    receiveEnabled_ = true;
    plusCount_ = 0;
    telnetState_ = kTelnetData;
    telnetCommand_ = 0;
    telnetDetected_ = false;
    telnetOptions_ = 0;
    receivedTotal_ = 0;
    deliveredTotal_ = 0;
    sentTotal_ = 0;
    aciaReadTotal_ = 0;
    receiveDropped_ = 0;
    transmitDropped_ = 0;
    receiveHighWater_ = 0;
    transmitHighWater_ = 0;
    lastTransmitTime_ = 0;
    escapeDeadline_ = 0;
    telnetCandidateDeadline_ = 0;
  }

  int Put(uint8_t byte) {
    CMutexGuard guard(lock_);
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
    CMutexGuard guard(lock_);
    if (!open_ || byte == 0) {
      return -1;
    }

    Pump();
    if (read_ == write_) {
      return 0;
    }

    *byte = queue_[read_];
    read_ = (read_ + 1) % kQueueSize;
    ++deliveredTotal_;
    return 1;
  }

  bool HasCarrier() {
    CMutexGuard guard(lock_);
    return socket_ != 0 || remoteDisconnectPending_;
  }

  void SetStatus(int status) {
    CMutexGuard guard(lock_);
    bool receiveEnabled = (status & 0x01) != 0;
    if (receiveEnabled != receiveEnabled_) {
      receiveEnabled_ = receiveEnabled;
      CLogger::Get()->Write(FromBmcModem, LogNotice,
                            "RTS receive %s",
                            receiveEnabled_ ? "enabled" : "paused");
    }
  }

  void SetBps(unsigned int bps) {
    CMutexGuard guard(lock_);
    if (bps != bps_) {
      bps_ = bps;
      CLogger::Get()->Write(FromBmcModem, LogNotice,
                            "ACIA configured for %u bps", bps_);
    }
  }

  void ServiceNetwork() {
    CMutexGuard guard(lock_);
    Pump();
  }

  void NoteAciaRead() {
    CMutexGuard guard(lock_);
    ++aciaReadTotal_;
  }

private:
  class CMutexGuard {
  public:
    explicit CMutexGuard(CMutex &lock) : lock_(lock) { lock_.Acquire(); }
    ~CMutexGuard() { lock_.Release(); }

  private:
    CMutex &lock_;
  };

  void Disconnect() {
    if (receivedTotal_ != 0 || sentTotal_ != 0 || receiveDropped_ != 0 ||
        transmitDropped_ != 0) {
      CLogger::Get()->Write(FromBmcModem, LogNotice,
                            "transport: rx %lu loaded %lu read %lu queued %u "
                            "rx-max %u rx-drop %lu tx %lu pending %u "
                            "tx-max %u tx-drop %lu",
                            receivedTotal_, deliveredTotal_, aciaReadTotal_,
                            Available(), receiveHighWater_, receiveDropped_,
                            sentTotal_, transmitLength_, transmitHighWater_,
                            transmitDropped_);
    }
    if (telnetOptions_ != 0) {
      CLogger::Get()->Write(FromBmcModem, LogNotice,
                            "telnet negotiation: %u options handled",
                            telnetOptions_);
    }
    delete socket_;
    socket_ = 0;
    remoteDisconnectPending_ = false;
    dataMode_ = false;
    plusCount_ = 0;
    escapeDeadline_ = 0;
    transmitLength_ = 0;
    telnetState_ = kTelnetData;
    telnetCommand_ = 0;
    telnetDetected_ = false;
    telnetCandidateDeadline_ = 0;
    telnetOptions_ = 0;
    receivedTotal_ = 0;
    deliveredTotal_ = 0;
    sentTotal_ = 0;
    aciaReadTotal_ = 0;
    receiveDropped_ = 0;
    transmitDropped_ = 0;
    receiveHighWater_ = Available();
    transmitHighWater_ = 0;
  }

  void BeginRemoteDisconnect() {
    if (telnetState_ == kTelnetCandidate) {
      QueueByte(kTelnetIac);
    }
    unsigned queued = Available();
    Disconnect();
    if (queued != 0) {
      remoteDisconnectPending_ = true;
      dataMode_ = true;
      CLogger::Get()->Write(FromBmcModem, LogNotice,
                            "delaying carrier loss for %u queued bytes",
                            queued);
    } else {
      Result("NO CARRIER");
    }
  }

  void FinishRemoteDisconnect() {
    if (!remoteDisconnectPending_ || Available() != 0) {
      return;
    }
    remoteDisconnectPending_ = false;
    dataMode_ = false;
    CLogger::Get()->Write(FromBmcModem, LogNotice,
                          "queued receive data drained; dropping carrier");
    Result("NO CARRIER");
  }

  unsigned Available() const {
    return (write_ + kQueueSize - read_) % kQueueSize;
  }

  unsigned Free() const { return kQueueSize - Available() - 1; }

  void QueueByte(uint8_t byte) {
    if (Free() == 0) {
      ++receiveDropped_;
      return;
    }
    queue_[write_] = byte;
    write_ = (write_ + 1) % kQueueSize;
    unsigned available = Available();
    if (available > receiveHighWater_) {
      receiveHighWater_ = available;
    }
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

  // Hayes command/data handling: bytes from the emulated ACIA.
  void PutCommand(uint8_t byte) {
    if (echo_) {
      QueueByte(byte);
    }

    if (byte == '\r' || byte == '\n') {
      if (commandLength_ != 0) {
        command_[commandLength_] = '\0';
        CLogger::Get()->Write(FromBmcModem, LogNotice,
                              "received modem command (%u bytes)",
                              commandLength_);
        ExecuteCommand();
      }
      commandLength_ = 0;
      commandInputActive_ = false;
      return;
    }

    if (byte == '\b' || byte == 0x14) {
      if (commandLength_ != 0) {
        --commandLength_;
      }
      return;
    }

    if (commandLength_ + 1 < kCommandSize) {
      if (!commandInputActive_) {
        CLogger::Get()->Write(FromBmcModem, LogNotice,
                              "received modem command data");
        commandInputActive_ = true;
      }
      command_[commandLength_++] = static_cast<char>(byte);
    }
  }

  void PutData(uint8_t byte) {
    u64 now = CTimer::GetClockTicks64();
    if (byte == '+' && plusCount_ < 3 &&
        (plusCount_ != 0 || now - lastTransmitTime_ >=
                                kEscapeGuardMicroseconds)) {
      if (plusCount_ < 3) {
        ++plusCount_;
      }
      escapeDeadline_ = now + kEscapeGuardMicroseconds;
      return;
    }

    FlushPendingPluses();
    QueueTransmit(byte);
    lastTransmitTime_ = now;
  }

  void FlushPendingPluses() {
    while (plusCount_ != 0) {
      QueueTransmit('+');
      --plusCount_;
    }
    escapeDeadline_ = 0;
  }

  void ResolveEscapeSequence() {
    if (plusCount_ == 0 || CTimer::GetClockTicks64() < escapeDeadline_) {
      return;
    }

    if (plusCount_ == 3) {
      plusCount_ = 0;
      escapeDeadline_ = 0;
      dataMode_ = false;
      Result("OK");
    } else {
      FlushPendingPluses();
      lastTransmitTime_ = CTimer::GetClockTicks64();
    }
  }

  void QueueTransmit(uint8_t byte) {
    unsigned required = telnetDetected_ && byte == kTelnetIac ? 2 : 1;
    if (transmitLength_ + required > sizeof transmit_) {
      ++transmitDropped_;
      return;
    }
    transmit_[transmitLength_++] = byte;
    if (required == 2) {
      transmit_[transmitLength_++] = byte;
    }
    if (transmitLength_ > transmitHighWater_) {
      transmitHighWater_ = transmitLength_;
    }
  }

  void QueueTelnetReply(uint8_t command, uint8_t option) {
    if (transmitLength_ + 3 > sizeof transmit_) {
      transmitDropped_ += 3;
      return;
    }
    transmit_[transmitLength_++] = kTelnetIac;
    transmit_[transmitLength_++] = command;
    transmit_[transmitLength_++] = option;
    if (transmitLength_ > transmitHighWater_) {
      transmitHighWater_ = transmitLength_;
    }
  }

  // Telnet filtering: control sequences stay on TCP, payload reaches the C64.
  void HandleTelnetOption(uint8_t option) {
    if (telnetCommand_ == kTelnetWill) {
      QueueTelnetReply(option == kTelnetBinary ? kTelnetDo : kTelnetDont,
                       option);
    } else if (telnetCommand_ == kTelnetDo) {
      QueueTelnetReply(option == kTelnetBinary ? kTelnetWill : kTelnetWont,
                       option);
    }
    ++telnetOptions_;
  }

  void HandleTelnetByte(uint8_t byte) {
  // TCP transport: the scheduler task calls this independently of ACIA polls.
    switch (telnetState_) {
      case kTelnetData:
        if (byte == kTelnetIac) {
          telnetState_ = telnetDetected_ ? kTelnetCommand : kTelnetCandidate;
          if (!telnetDetected_) {
            telnetCandidateDeadline_ =
                CTimer::GetClockTicks64() + kTelnetCandidateMicroseconds;
          }
        } else {
          QueueByte(byte);
        }
        break;

      case kTelnetCandidate:
        telnetCandidateDeadline_ = 0;
        if (byte == kTelnetIac) {
          QueueByte(kTelnetIac);
          QueueByte(kTelnetIac);
          telnetState_ = kTelnetData;
        } else if (byte == kTelnetWill || byte == kTelnetWont ||
                   byte == kTelnetDo || byte == kTelnetDont) {
          telnetCommand_ = byte;
          telnetState_ = kTelnetCandidateOption;
        } else {
          QueueByte(kTelnetIac);
          QueueByte(byte);
          telnetState_ = kTelnetData;
        }
        break;

      case kTelnetCandidateOption:
        telnetDetected_ = true;
        HandleTelnetOption(byte);
        telnetState_ = kTelnetData;
        break;

      case kTelnetCommand:
        if (byte == kTelnetIac) {
          QueueByte(byte);
          telnetState_ = kTelnetData;
        } else if (byte == kTelnetWill || byte == kTelnetWont ||
                   byte == kTelnetDo || byte == kTelnetDont) {
          telnetCommand_ = byte;
          telnetState_ = kTelnetOption;
        } else if (byte == kTelnetSubnegotiation) {
          telnetState_ = kTelnetSubnegotiation;
        } else {
          telnetState_ = kTelnetData;
        }
        break;

      case kTelnetOption:
        HandleTelnetOption(byte);
        telnetState_ = kTelnetData;
        break;

      case kTelnetSubnegotiation:
        if (byte == kTelnetIac) {
          telnetState_ = kTelnetSubnegotiationIac;
        }
        break;

      case kTelnetSubnegotiationIac:
        if (byte == kTelnetSubnegotiationEnd) {
          telnetState_ = kTelnetData;
        } else {
          telnetState_ = kTelnetSubnegotiation;
        }
        break;
    }
  }

  void ResolveTelnetCandidate() {
    if (telnetState_ == kTelnetCandidate &&
        CTimer::GetClockTicks64() >= telnetCandidateDeadline_) {
      QueueByte(kTelnetIac);
      telnetState_ = kTelnetData;
      telnetCandidateDeadline_ = 0;
    }
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
      lastTransmitTime_ = CTimer::GetClockTicks64();
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

    if (*target == '"') {
      ++target;
      char *end = target;
      while (*end != '\0') {
        ++end;
      }
      if (end == target || *--end != '"') {
        Result("ERROR");
        return;
      }
      *end = '\0';
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
      CLogger::Get()->Write(FromBmcModem, LogNotice,
                            "dial failed: network unavailable");
      Result("NO CARRIER");
      return;
    }

    CLogger::Get()->Write(FromBmcModem, LogNotice, "dialing %s:%u", target,
                          port);
    CIPAddress address;
    CDNSClient dns(net);
    if (!dns.Resolve(target, &address)) {
      CLogger::Get()->Write(FromBmcModem, LogNotice,
                            "dial failed: DNS lookup failed");
      Result("NO CARRIER");
      return;
    }

    Disconnect();
    socket_ = new CSocket(net, IPPROTO_TCP);
    if (socket_ == 0 || socket_->Connect(address, static_cast<u16>(port)) < 0) {
      Disconnect();
      CLogger::Get()->Write(FromBmcModem, LogNotice,
                            "dial failed: TCP connection refused");
      Result("NO CARRIER");
      return;
    }

    dataMode_ = true;
    lastTransmitTime_ = CTimer::GetClockTicks64();
    CLogger::Get()->Write(FromBmcModem, LogNotice, "TCP connection established");
    Result("CONNECT");
  }

  void Pump() {
    if (socket_ == 0) {
      FinishRemoteDisconnect();
      return;
    }

    ResolveEscapeSequence();
    FlushTransmit();
    ReceiveData();
    ResolveTelnetCandidate();
  }

  void FlushTransmit() {
    if (transmitLength_ == 0) {
      return;
    }

    unsigned sendLength = transmitLength_;
    if (sendLength > kSocketSendSize) {
      sendLength = kSocketSendSize;
    }
    int sent = socket_->Send(transmit_, sendLength, MSG_DONTWAIT);
    if (sent > 0) {
      unsigned count = static_cast<unsigned>(sent);
      sentTotal_ += count;
      if (count == 1) {
        CLogger::Get()->Write(FromBmcModem, LogNotice,
                              "sent byte: %02X", transmit_[0]);
      } else {
        CLogger::Get()->Write(FromBmcModem, LogNotice,
                              "sent %u bytes", count);
      }
      if (count < transmitLength_) {
        memmove(transmit_, transmit_ + count, transmitLength_ - count);
      }
      transmitLength_ -= count;
    } else if (sent < 0 && sent != -NET_ERROR_WOULD_BLOCK) {
      CLogger::Get()->Write(FromBmcModem, LogNotice,
                            "socket send failed: %d", sent);
      BeginRemoteDisconnect();
    }
  }

  void ReceiveData() {
    if (!receiveEnabled_) {
      return;
    }

    unsigned free = Free();
    if (free < FRAME_BUFFER_SIZE) {
      return;
    }

    uint8_t received[FRAME_BUFFER_SIZE];
    int count = socket_->Receive(received, sizeof received, MSG_DONTWAIT);
    if (count < 0 && count != -NET_ERROR_WOULD_BLOCK) {
      CLogger::Get()->Write(FromBmcModem, LogNotice,
                            "socket receive failed: %d", count);
      BeginRemoteDisconnect();
      return;
    }
    if (count > 0) {
      receivedTotal_ += static_cast<unsigned long>(count);
      CLogger::Get()->Write(FromBmcModem, LogNotice,
                            "received %d bytes", count);
      char preview[kReceivePreviewSize + 1];
      unsigned previewLength = static_cast<unsigned>(count);
      if (previewLength > kReceivePreviewSize) {
        previewLength = kReceivePreviewSize;
      }
      for (unsigned index = 0; index < previewLength; ++index) {
        uint8_t byte = received[index];
        preview[index] = byte >= 32 && byte <= 126
                             ? static_cast<char>(byte)
                             : '.';
      }
      preview[previewLength] = '\0';
      CLogger::Get()->Write(FromBmcModem, LogNotice,
                            "received preview: %s", preview);
    }
    for (int index = 0; index < count; ++index) {
      HandleTelnetByte(received[index]);
    }
  }

private:
  // Protocol parsing state.
  enum TelnetState {
    kTelnetData,
    kTelnetCandidate,
    kTelnetCandidateOption,
    kTelnetCommand,
    kTelnetOption,
    kTelnetSubnegotiation,
    kTelnetSubnegotiationIac
  };

  CMutex lock_;
  CSocket *socket_;

  // Serial data visible to VICE and TCP data waiting to be transmitted.
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
  bool commandInputActive_;
  bool receiveEnabled_;
  bool remoteDisconnectPending_;
  unsigned int bps_;
  unsigned plusCount_;
  NetworkTask *worker_;
  TelnetState telnetState_;
  uint8_t telnetCommand_;
  bool telnetDetected_;
  unsigned telnetOptions_;
  unsigned long receivedTotal_;
  unsigned long deliveredTotal_;
  unsigned long sentTotal_;
  unsigned long aciaReadTotal_;
  unsigned long receiveDropped_;
  unsigned long transmitDropped_;
  unsigned receiveHighWater_;
  unsigned transmitHighWater_;
  u64 lastTransmitTime_;
  u64 escapeDeadline_;
  u64 telnetCandidateDeadline_;
};

void NetworkTask::Run() {
  for (;;) {
    modem_->ServiceNetwork();
    CScheduler::Get()->MsSleep(1);
  }
}

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

extern "C" int bmcmodem_has_carrier(void) {
  return modem.HasCarrier() ? 1 : 0;
}

extern "C" void bmcmodem_set_status(int status) { modem.SetStatus(status); }

extern "C" void bmcmodem_set_bps(unsigned int bps) { modem.SetBps(bps); }

extern "C" void bmcmodem_init(void) { modem.Initialize(); }

extern "C" void bmcmodem_reset(void) { modem.Reset(); }
