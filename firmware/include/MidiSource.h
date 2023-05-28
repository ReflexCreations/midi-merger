#pragma once

#include <stdint.h>
#include <stddef.h>
#include "UartRx.h"
#include "Midi.h"

#define MIDISOURCE_FIFO_DEPTH 64


enum SystemStatus : uint8_t {
  SystemStatus_SysExStart = 0b11110000,
  SystemStatus_TimeCodeQuarterFrame = 0b11110001,
  SystemStatus_SongPositionPointer = 0b11110010,
  SystemStatus_SongSelect = 0b11110011,
  SystemStatus_Reserved1 = 0b11110100,
  SystemStatus_Reserved2 = 0b11110101,
  SystemStatus_TuneRequest = 0b11110110,
  SystemStatus_SysExEnd = 0b11110111,
};

enum MessageState: uint8_t {
  MessageState_Blank = 0x00,
  MessageState_Ongoing = 0x01,
  MessageState_OngoingSysex = 0x02,
  MessageState_FinalByte = 0xFF
};

struct MessageFragment {
  MessageState state;
  uint8_t midiByte;
};

class MidiFifo {

public:
  MidiFifo();
  bool push(MessageFragment);
  MessageFragment pop();
  MessageFragment* peek();
  bool hasData();
  bool isFull();

private:
  MessageFragment buffer[MIDISOURCE_FIFO_DEPTH];
  static MessageFragment getBlankMessageFragment();
  size_t frontIndex;
  size_t rearIndex;
  size_t count;
};

enum SourceState: uint8_t {
  // Idle: Expect a status byte starting a new message
  SourceState_Idle = 0x00,

  // Ongoing message that expects two more data bytes to come in
  SourceState_ExpectTwoData = 0x01,

  // Ongoing message that expects one more data byte to come in
  SourceState_ExpectOneData = 0x02,

  // Received Sysex start, waiting for first manufacturer ID byte
  // System realtime messages also fine
  SourceState_SysExExpectMfrId = 0xF0,

  // Received first sysex mfr ID byte and it was 0x00, indicating it's the 3-byte format,
  // expect 2 more ID bytes
  // System realtime messages also fine
  SourceState_SysExExpectTwoMfrIdBytes = 0xF1,

  // Received first 2 of 3-byte manufacturer ID, expecting one more
  // System realtime messages also fine
  SourceState_SysExExpectOneMfrIdByte = 0xF2,

  // Received full manufacturer ID, now expecting arbitrary data, Sysex end, or system realtime
  // bytes
  SourceState_SysExExpectData = 0xF3,
};

enum DataType: uint8_t {
  DataType_None = 0x00,
  DataType_Realtime = 0x01,
  DataType_Other = 0x02,
};

// TODO perhaps keep track of when we last received a byte, so a source can time out if we don't
// get a timely follow-up byte through.

class MidiSource {
public:
  MidiSource(uint8_t gpioPin);
  MidiFifo fifo;

  // Takes bytes from UART_RX and evaluates their validity before sticking them in the fifo
  // with state
  DataType read();

private:
  SourceState state;
  UartRx uartRx;

  void processByte(uint8_t);
  inline void processByteWhileIdle(uint8_t rawByte);
  inline void processByteWhileExpectingDataBytes(uint8_t rawByte, uint8_t bytesExpected);
  inline void processByteWhileExpectingSysExMfrId(uint8_t rawByte);
  inline void processByteWhileExpectingSysExMoreMfrId(uint8_t rawByte, uint8_t idBytesExpected);
  inline void processByteWhileExpectingSysExData(uint8_t rawByte);
  inline bool preprocessByteWhileExpectingAnySysEx(uint8_t rawByte);
  inline void pushOngoing(uint8_t);
  inline void pushOngoingSysEx(uint8_t);
  inline void pushFinal(uint8_t);
};