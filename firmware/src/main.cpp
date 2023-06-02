#include "MidiMerger.h"
#include "MidiSource.h"
#include "MidiSink.h"
#include <pico/stdlib.h>
#include <hardware/uart.h>
#include <stdint.h>
#include <Arduino.h>

// Mapping as per rev0 of the PCB
#define PIN_MIDI_SOURCE_1 16
#define PIN_MIDI_SOURCE_2 17
#define PIN_MIDI_SOURCE_3 21
#define PIN_MIDI_SOURCE_4 28
#define PIN_MIDI_SOURCE_5 5
#define PIN_MIDI_SOURCE_6 10
#define PIN_MIDI_SOURCE_7 14
#define PIN_MIDI_SOURCE_8 15

#define PIN_MIDI_SINK_TX 8
#define PIN_MIDI_SINK_RX 9

MidiMerger* merger;

void setup() {
  merger = new MidiMerger(new MidiSink(uart1, PIN_MIDI_SINK_TX, PIN_MIDI_SINK_RX));
  merger->addSource(new MidiSource(PIN_MIDI_SOURCE_1));
  merger->addSource(new MidiSource(PIN_MIDI_SOURCE_2));
  merger->addSource(new MidiSource(PIN_MIDI_SOURCE_3));
  merger->addSource(new MidiSource(PIN_MIDI_SOURCE_4));
  merger->addSource(new MidiSource(PIN_MIDI_SOURCE_5));
  merger->addSource(new MidiSource(PIN_MIDI_SOURCE_6));
  merger->addSource(new MidiSource(PIN_MIDI_SOURCE_7));
  merger->addSource(new MidiSource(PIN_MIDI_SOURCE_8));
}

void loop() {
  merger->loop();
}