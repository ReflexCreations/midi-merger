#include "MidiMerger.h"
#include "MidiSource.h"
#include "MidiSink.h"
#include <pico/stdlib.h>
#include <hardware/uart.h>
#include <stdint.h>
#include <Arduino.h>

#define PIN_MIDI_SOURCE_A 27
#define PIN_MIDI_SOURCE_B 26
#define PIN_MIDI_SOURCE_C 22

#define PIN_MIDI_SINK_TX 8
#define PIN_MIDI_SINK_RX 9

MidiMerger* merger;

void setup() {
  merger = new MidiMerger(new MidiSink(uart1, PIN_MIDI_SINK_TX, PIN_MIDI_SINK_RX));
  merger->addSource(new MidiSource(PIN_MIDI_SOURCE_A));
  merger->addSource(new MidiSource(PIN_MIDI_SOURCE_B));
  merger->addSource(new MidiSource(PIN_MIDI_SOURCE_C));
}

void loop() {
  merger->loop();
}