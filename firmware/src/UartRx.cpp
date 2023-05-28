#include "UartRx.h"
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "uart_rx_pio.h"
#include "hardware/platform_defs.h"

uint8_t UartRx::instanceCount = 0;
bool UartRx::pio0ProgramAdded = false;
bool UartRx::pio1ProgramAdded = false;
uint UartRx::pio0ProgramOffset = 0;
uint UartRx::pio1ProgramOffset = 0;


UartRx::UartRx(uint8_t gpioPin, uint baudRate) {
  uint8_t index = instanceCount++;
  stateMachine = getStateMachine(index);
  pio = getPioInstance(index);
  uint programOffset = addProgram(pio);

  uart_rx_program_init(pio, stateMachine, programOffset, gpioPin, baudRate);
}

bool UartRx::haveData() {
  return uart_rx_program_have_data(pio, stateMachine);
}

uint8_t UartRx::getByte() {
  return uart_rx_program_getc(pio, stateMachine);
}

uint UartRx::getStateMachine(uint8_t index) {
  return index % NUM_PIO_STATE_MACHINES;
}

PIO UartRx::getPioInstance(uint8_t index) {
  return index < NUM_PIO_STATE_MACHINES ? pio0 : pio1;
}

uint UartRx::addProgram(PIO pio) {
  // This probably could be done cleaner, less repetitively, but I don't think it's worth making
  // some struct to point to for the grand total of 2 possibilities.

  if (pio == pio0) {
    if (pio0ProgramAdded) return pio0ProgramOffset;
    pio0ProgramAdded = true;
    pio0ProgramOffset = pio_add_program(pio, &uart_rx_program);
    return pio0ProgramOffset;
  } else {
    if (pio1ProgramAdded) return pio1ProgramOffset;
    pio1ProgramAdded = true;
    pio1ProgramOffset = pio_add_program(pio, &uart_rx_program);
    return pio1ProgramOffset;
  }
}