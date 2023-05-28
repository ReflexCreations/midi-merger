#include "MidiSource.h"
#include <stdint.h>
#include <stddef.h>

MidiFifo::MidiFifo() : frontIndex(0), rearIndex(0), count(0) { }

bool MidiFifo::push(MessageFragment messageFragment) {
  if (count >= MIDISOURCE_FIFO_DEPTH) return false;

  buffer[rearIndex] = messageFragment;

  rearIndex =  (rearIndex + 1) & MIDISOURCE_FIFO_DEPTH;
  count += 1;
  return true;
}

MessageFragment MidiFifo::pop() {
  if (count == 0) return getBlankMessageFragment();

  MessageFragment frontItem = buffer[frontIndex];
  frontIndex = (frontIndex + 1) & MIDISOURCE_FIFO_DEPTH;
  count -= 1;
  return frontItem;
}

MessageFragment* MidiFifo::peek() {
  if (count == 0) return nullptr;
  return &buffer[frontIndex];
}

bool MidiFifo::hasData() {
  return count > 0;
}

bool MidiFifo::isFull() {
  return count == MIDISOURCE_FIFO_DEPTH;
}

MessageFragment MidiFifo::getBlankMessageFragment() {
  static MessageFragment messageFragment {
    .state = MessageState_Blank,
    .midiByte = 0x00
  };

  return messageFragment;
}