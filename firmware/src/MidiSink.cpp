#include "MidiSink.h"
#include <hardware/uart.h>
#include <pico/stdlib.h>
#include "Midi.h"
#include <stdint.h>

MidiSink::MidiSink(uart_inst* uart, uint txPin, uint rxPin) : uart(uart) {
  uart_init(uart, MIDI_BAUD_RATE);
  gpio_set_function(txPin, GPIO_FUNC_UART);
  gpio_set_function(rxPin, GPIO_FUNC_UART);
}

void MidiSink::send(uint8_t midiByte) {
  uart_putc_raw(uart, midiByte);
}