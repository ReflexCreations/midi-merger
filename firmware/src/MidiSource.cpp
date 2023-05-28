#include "MidiSource.h"
#include "Midi.h"

MidiSource::MidiSource(uint8_t gpioPin)
 : state(SourceState_Idle),
   uartRx(gpioPin, MIDI_BAUD_RATE) { }

DataType MidiSource::read() {
  if (uartRx.haveData() && !fifo.isFull()) {
    MidiSource::processByte(uartRx.getByte());
  }

  MessageFragment* nextMessage = fifo.peek();
  if (nextMessage == nullptr) return DataType_None;
  return (MIDI_IS_SYSTEM_REALTIME_STATUS_BYTE(nextMessage->midiByte))
    ? DataType_Realtime
    : DataType_Other;
}

void MidiSource::processByte(uint8_t rawByte) {
  switch (state) {
    case SourceState_Idle:
     return processByteWhileIdle(rawByte);

    case SourceState_ExpectTwoData:
      return processByteWhileExpectingDataBytes(rawByte, 2);

    case SourceState_ExpectOneData:
      return processByteWhileExpectingDataBytes(rawByte, 1);

    case SourceState_SysExExpectMfrId:
      return processByteWhileExpectingSysExMfrId(rawByte);

    case SourceState_SysExExpectTwoMfrIdBytes:
      return processByteWhileExpectingSysExMoreMfrId(rawByte, 2);

    case SourceState_SysExExpectOneMfrIdByte:
      return processByteWhileExpectingSysExMoreMfrId(rawByte, 1);

    case SourceState_SysExExpectData:
     return processByteWhileExpectingSysExData(rawByte);
  }
}

void MidiSource::processByteWhileIdle(uint8_t rawByte) {
  // When Idle, we expect an incoming byte to be status byte. If it's not, discard.
  if (!MIDI_IS_STATUS_BYTE(rawByte)) return;

  if (MIDI_IS_CHANNEL_STATUS_BYTE(rawByte)) {
    state = SourceState_ExpectTwoData;
    pushOngoing(rawByte);
    return;
  }

  if (MIDI_IS_SYSTEM_REALTIME_STATUS_BYTE(rawByte)) {
    state = SourceState_Idle;
    pushFinal(rawByte);
    return;
  }

  switch ((SystemStatus)rawByte) {
    case SystemStatus_SysExStart:
      state = SourceState_SysExExpectMfrId;
      pushOngoingSysEx(rawByte);
      return;

    case SystemStatus_SongPositionPointer:
      state = SourceState_ExpectTwoData;
      pushOngoing(rawByte);
      return;

    case SystemStatus_TimeCodeQuarterFrame:
    case SystemStatus_SongSelect:
      state = SourceState_ExpectOneData;
      pushOngoing(rawByte);
      return;

    case SystemStatus_Reserved1:
    case SystemStatus_Reserved2:
    case SystemStatus_TuneRequest:
      state = SourceState_Idle;
      pushFinal(rawByte);
      return;
  }
}

void MidiSource::processByteWhileExpectingDataBytes(uint8_t rawByte, uint8_t dataBytesExpected) {
  // Expecting data bytes only, so if we're getting something else, discard.
  if (!MIDI_IS_DATA_BYTE(rawByte)) return;

  state = dataBytesExpected == 2 ? SourceState_ExpectOneData : SourceState_Idle;
  fifo.push(
    MessageFragment {
      .state = dataBytesExpected > 1 ? MessageState_Ongoing : MessageState_FinalByte,
      .midiByte = rawByte
    }
  );
}

void MidiSource::processByteWhileExpectingSysExMfrId(uint8_t rawByte) {
  bool handled = preprocessByteWhileExpectingAnySysEx(rawByte);
  if (handled) return;

  if (MIDI_MFR_ID_LONG_ID_FIRST_BYTE == rawByte) {
    state = SourceState_SysExExpectTwoMfrIdBytes;
  } else {
    state = SourceState_SysExExpectData;
  }

  pushOngoingSysEx(rawByte);
}

void MidiSource::processByteWhileExpectingSysExMoreMfrId(uint8_t rawByte, uint8_t idBytesExpected) {
  bool handled = preprocessByteWhileExpectingAnySysEx(rawByte);
  if (handled) return;

  state = idBytesExpected == 2 ? SourceState_SysExExpectOneMfrIdByte : SourceState_SysExExpectData;
  pushOngoingSysEx(rawByte);
}

void MidiSource::processByteWhileExpectingSysExData(uint8_t rawByte) {
  bool handled = preprocessByteWhileExpectingAnySysEx(rawByte);
  if (handled) return;
  pushOngoingSysEx(rawByte);
}

// Pre-process a byte while in any stage of SysEx. Returns true if no further action is required,
// false if the caller must handle further processing.
bool MidiSource::preprocessByteWhileExpectingAnySysEx(uint8_t rawByte) {
  // While in a sysex stream, system realtime messages are allowed.
  // Push it through, don't alter status.
  if (MIDI_IS_SYSTEM_REALTIME_STATUS_BYTE(rawByte)) {
    pushOngoingSysEx(rawByte);
    return true;
  }

  // If at any point during sysex we hit sysex end, accept it and return to idle.
  // Technically we're meant to finish the manufacturer ID but I feel like if the sender wants
  // to stop early, we should allow it.
  if (SystemStatus_SysExEnd == rawByte) {
    state = SourceState_Idle;
    pushFinal(rawByte);
    return true;
  }

  // Special cases out of the way, we only want to deal with data bytes further.
  // If what we've got is not that, we're skipping it and not doing anything.
  if (!MIDI_IS_DATA_BYTE(rawByte)) return true;

  return false;
}

void MidiSource::pushOngoing(uint8_t midiByte) {
  fifo.push(
    MessageFragment {
      .state = MessageState_Ongoing,
      .midiByte = midiByte
    }
  );
}

void MidiSource::pushOngoingSysEx(uint8_t midiByte) {
  fifo.push(
    MessageFragment {
      .state = MessageState_OngoingSysex,
      .midiByte = midiByte
    }
  );
}

void MidiSource::pushFinal(uint8_t midiByte) {
  fifo.push(
    MessageFragment {
      .state = MessageState_FinalByte,
      .midiByte = midiByte
    }
  );
}
