#include "MidiMerger.h"
#include "MidiSource.h"

MidiMerger::MidiMerger(MidiSink* sink)
  : sourceCount(0),
    ongoingSource(nullptr),
    isOngoingSendingSysEx(false),
    sink(sink) { }

bool MidiMerger::addSource(MidiSource* source) {
  if (sourceCount == MAX_NUM_SOURCES) return false;
  sources[sourceCount++] = source;
  return true;
}

void MidiMerger::loop() {
  if (ongoingSource != nullptr) {
    pollSource(ongoingSource);
  }

  if (ongoingSource != nullptr) {
    if (isOngoingSendingSysEx) pollAllSourcesForRealtime();
  } else {
    pollAllSources();
  }
}

void MidiMerger::pollAllSourcesForRealtime() {
  for (size_t i = 0; i < sourceCount; i++) {
    pollSourceForRealtime(sources[i]);
  }
}

void MidiMerger::pollAllSources() {
  for (size_t i = 0; i < sourceCount; i++) {
    if (ongoingSource == nullptr || ongoingSource == sources[i]) {
      pollSource(sources[i]);
    } else if (isOngoingSendingSysEx) {
      pollSourceForRealtime(sources[i]);
    }
  }
}

void MidiMerger::pollSource(MidiSource* source) {
  if (source->read() == DataType_None) return;

  MessageFragment frag = source->fifo.pop();

  switch (frag.state) {
    case MessageState_Ongoing:
      ongoingSource = source;
      isOngoingSendingSysEx = false;
      break;

    case MessageState_OngoingSysex:
      ongoingSource = source;
      isOngoingSendingSysEx = true;
      break;

    case MessageState_FinalByte:
      ongoingSource = nullptr;
      isOngoingSendingSysEx = false;
      break;

    default: break;
  }

  send(frag.midiByte);
}

void MidiMerger::pollSourceForRealtime(MidiSource* source) {
  if (source->read() == DataType_Realtime) send(source->fifo.pop().midiByte);
}

void MidiMerger::send(uint8_t byte) {
  sink->send(byte);
}