#pragma once

#include <hardware/uart.h>
#include <stdint.h>

class MidiSink {

public:
  MidiSink(uart_inst* uart, uint txPin, uint rxPin);
  void send(uint8_t midiByte);

private:
  uart_inst* uart;
};