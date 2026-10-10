//
// acia_modem.cpp
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "acia_modem.h"

#include "acia6551.hpp"

extern "C" {
#include "../../vice-3.3/src/rs232drv/rs232bmc.h"
}

namespace {

// The modem's status bits, as rs232bmc passes VICE's rs232handshake_out.
const int kModemRts = 0x01;
const int kModemDtr = 0x02;

// Same calls VICE's aciacore makes through rs232drv/rs232bmc.c.
class BmcModemLine : public Plus4::ACIA6551::SerialLine {
 public:
  void open() override { bmcmodem_open(0); }
  void close() override { bmcmodem_close(0); }
  void setLines(bool dtr, bool rts) override {
    bmcmodem_set_status((dtr ? kModemDtr : 0) | (rts ? kModemRts : 0));
  }
  void noteDataWrite(uint8_t value) override { bmcmodem_note_acia_tx(value); }
  void put(uint8_t value) override { bmcmodem_putc(0, value); }
  bool get(uint8_t &value) override { return bmcmodem_getc(0, &value) == 1; }
  bool hasCarrier() override { return bmcmodem_has_carrier() != 0; }
};

BmcModemLine modem_line;

}  // namespace

extern "C" void acia_modem_attach(Plus4VM *vm) {
  bmcmodem_init();
  Plus4VM_SetEnableACIAEmulation(vm, 1);
  Plus4VM_SetACIASerialLine(vm, &modem_line);
}
