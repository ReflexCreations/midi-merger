#include <Arduino.h>
#include <MIDI.h>
#include <SoftwareSerial.h>

#define PIN_MIDI_IN_A 27
#define PIN_MIDI_OUT_A 6 // actually unused, but need something
#define PIN_MIDI_IN_B 26
#define PIN_MIDI_OUT_B 7 // actually unused, but need something
#define PIN_MIDI_IN_C 22
#define PIN_MIDI_OUT_C 8 // actually unused, but need something

#define PIN_MIDI_OUT 0

typedef midi::MidiInterface<midi::SerialMIDI<arduino::HardwareSerial>> HWMidiInterface;
typedef midi::MidiInterface<midi::SerialMIDI<SoftwareSerial>> SWMidiInterface;

SoftwareSerial midiASerial(PIN_MIDI_IN_A, PIN_MIDI_OUT_A, false);
SoftwareSerial midiBSerial(PIN_MIDI_IN_B, PIN_MIDI_OUT_B, false);
SoftwareSerial midiCSerial(PIN_MIDI_IN_C, PIN_MIDI_OUT_C, false);

midi::SerialMIDI<SoftwareSerial> midiInASerial(midiASerial);
midi::SerialMIDI<SoftwareSerial> midiInBSerial(midiBSerial);
midi::SerialMIDI<SoftwareSerial> midiInCSerial(midiCSerial);

SWMidiInterface midiInA(midiInASerial);
SWMidiInterface midiInB(midiInBSerial);
SWMidiInterface midiInC(midiInCSerial);

midi::SerialMIDI<HardwareSerial> midiOutSerial(Serial1);

HWMidiInterface midiOut(midiOutSerial);

void sendThruFrom(SWMidiInterface*);

void setup() {
  Serial1.setTX(PIN_MIDI_OUT);

  midiInA.begin(MIDI_CHANNEL_OMNI);
  midiInB.begin(MIDI_CHANNEL_OMNI);
  midiInC.begin(MIDI_CHANNEL_OMNI);

  midiOut.begin(MIDI_CHANNEL_OFF);
}

void loop() {
  byte haveMsgFlags = 0x00;
  if (midiInA.read()) haveMsgFlags |= (1 << 0);
  if (midiInB.read()) haveMsgFlags |= (1 << 1);
  if (midiInC.read()) haveMsgFlags |= (1 << 2);
  if (haveMsgFlags & (1 << 0)) sendThruFrom(&midiInA);
  if (haveMsgFlags & (1 << 1)) sendThruFrom(&midiInB);
  if (haveMsgFlags & (1 << 2)) sendThruFrom(&midiInC);
}

void sendThruFrom(SWMidiInterface* midiIn) {
  midi::MidiType msgType = midiIn->getType();
  midi::Channel channel = midiIn->getChannel();
  midi::DataByte data1 = midiIn->getData1();
  midi::DataByte data2 = midiIn->getData2();
  midiOut.send(msgType, data1, data2, channel);
}