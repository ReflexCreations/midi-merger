#pragma once

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"

class UartRx {

public:
  UartRx(uint8_t gpioPin, uint baudRate);
  bool haveData();
  uint8_t getByte();

private:
  uint stateMachine;
  PIO pio;

  static uint8_t instanceCount;
  static bool pio0ProgramAdded;
  static bool pio1ProgramAdded;
  static uint pio0ProgramOffset;
  static uint pio1ProgramOffset;
  static inline uint getStateMachine(uint8_t);
  static inline PIO getPioInstance(uint8_t);
  static inline uint addProgram(PIO);
};