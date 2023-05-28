#pragma once

#include "MidiSource.h"
#include "MidiSink.h"
#include <stddef.h>

#define MAX_NUM_SOURCES 8

class MidiMerger {
public:
  MidiMerger(MidiSink* sink);
  bool addSource(MidiSource* source);
  void loop();

private:
  MidiSource* sources[MAX_NUM_SOURCES];
  size_t sourceCount;
  MidiSource* ongoingSource;
  bool isOngoingSendingSysEx;
  MidiSink* sink;

  void pollAllSourcesForRealtime();
  void pollAllSources();
  void pollSource(MidiSource*);
  void inline pollSourceForRealtime(MidiSource*);
  void send(uint8_t);
};