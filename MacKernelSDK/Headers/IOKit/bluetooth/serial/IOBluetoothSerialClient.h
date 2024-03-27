/*
 * Released under "The BSD 3-Clause License"
 *
 * Copyright (c) 2021 cjiang. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The names of its contributors may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef _IOKIT_IOBLUETOOTHSERIALCLIENT_H
#define _IOKIT_IOBLUETOOTHSERIALCLIENT_H

#include <IOKit/bluetooth/IOBluetoothMemoryBlock.h>
#include <IOKit/serial/IOModemSerialStreamSync.h>
#include <IOKit/serial/IORS232SerialStreamSync.h>
#include <IOKit/serial/IOSerialDriverSync.h>

class IOBluetoothRFCOMMChannel;
class IOBluetoothSerialManager;

enum ParityType
{
    NoParity = 0,
    OddParity,
    EvenParity,
    MaxParity
};

#define kDefaultBaudRate       9600
#define kMaxBaudRate           3000000
#define kMaxCircularBufferSize 4096

typedef struct CirQueue
{
    UInt8 *     Start;
    UInt8 *     End;
    UInt8 *     NextChar;
    UInt8 *     LastChar;
    IOByteCount Size;
    IOByteCount InQueue;
    IOLock *    InUse;
} CirQueue;

typedef enum QueueStatus
{
    queueNoError = 0,
    queueFull,
    queueEmpty,
    queueMaxStatus
} QueueStatus;

QueueStatus InitQueue(CirQueue * Queue, UInt8 * Buffer, size_t Size);
QueueStatus CloseQueue(CirQueue * Queue);
size_t      AddtoQueue(CirQueue * Queue, UInt8 * Buffer, size_t Size);
size_t      FreeSpaceinQueue(CirQueue * Queue);
QueueStatus PrivateAddBytetoQueue(CirQueue * Queue, char Value);
size_t      GetSingleBlockPointer(CirQueue * Queue, size_t MaxSize, UInt8 ** Pointer);
size_t      ConsiderThisBlockRead(CirQueue * Queue, UInt8 * Buffer, size_t Size);

size_t      FillBlockWithQueue(CirQueue * Queue, IOBluetoothMemoryBlock * Block);
size_t      RemovefromQueue(CirQueue * Queue, UInt8 * Buffer, size_t Size);
QueueStatus PrivateGetBytetoQueue(CirQueue * Queue, UInt8 * Value);
size_t      UsedSpaceinQueue(CirQueue * Queue);
size_t      GetQueueSize(CirQueue * Queue);
QueueStatus AddBytetoQueue(CirQueue * Queue, char Value);
QueueStatus GetBytetoQueue(CirQueue * Queue, UInt8 * Value);
QueueStatus GetQueueStatus(CirQueue * Queue);

typedef struct BufferMarks
{
    UInt32 BufferSize;
    UInt32 HighWater;
    UInt32 LowWater;
    bool   OverRun;
} BufferMarks;

typedef struct Stats_t
{
    UInt32 ints;
    UInt32 txInts;
    UInt32 rxInts;
    UInt32 mdmInts;
    UInt32 txChars;
    UInt32 rxChars;
} Stats_t;

typedef struct PortInfo_t
{
    UInt32         Instance;          // 0
    const UInt8 *  PortName;          // 8
    UInt32         State;             // 16
    UInt32         WatchStateMask;    // 20
    IOSimpleLock * WatchLock;         // 24
    IOSimpleLock * serialRequestLock; // 32

    // queue control structures:
    CirQueue RX; // 40
    CirQueue TX; // 96

    BufferMarks RXStats; // 152
    BufferMarks TXStats; // 168

    // dbdma memory control
    IOLock * IODBDMARxLock; // 184
    IOLock * IODBDMATrLock; // 192

    // UART configuration info:
    UInt32 Base;              // 200
    UInt32 CharLength;        // 204
    UInt32 StopBits;          // 208
    UInt32 TXParity;          // 212
    UInt32 RXParity;          // 216
    UInt32 BreakLength;       // 220
    UInt32 BaudRate;          // 224
    UInt16 DLRimage;          // 228
    UInt8  LCRimage;          // 230
    UInt8  FCRimage;          // 231
    UInt8  IERmask;           // 232
    UInt8  RBRmask;           // 233
    UInt32 MasterClock;       // 236
    bool   MinLatency;        // 237
    bool   WaitingForTXIdle;  // 238
    bool   JustDoneInterrupt; // 239
    bool   PCMCIA;            // 240
    bool   PCMCIAYanked;      // 241

    // flow control state & configuration:
    UInt8  XONchar;          // 242
    UInt8  XOFFchar;         // 243
    UInt32 SWspecial[8];     // 244
    UInt32 FlowControl;      // 276 notify-on-delta & auto_control
    UInt32 FlowControlState; // 280
    int    RXOstate;         // receive state 284
    int    TXOstate;         // transmit state if any Flow Control is received 288

    UInt8 xOffSent; // 292

    // Globals in the Copeland Version
    UInt32 GlobalRecvChars; // 296
    UInt32 OverRunChars;    // 300

    // callout entries:
    IOThread      FrameTOEntry;      // 304
    IOThread      DataLatTOEntry;    // 312
    IOThread      DelayTOEntry;      // 320
    IOThread      HeartBeatTOEntry;  // 328
    mach_timespec FrameInterval;     // 336
    mach_timespec DataLatInterval;   // 344
    mach_timespec CharLatInterval;   // 352
    mach_timespec HeartBeatInterval; // 360

    // Statistics
    Stats_t Stats;           // 368
    bool    AreTransmitting; // 392
    bool    GotTXInterrupt;  // 393

    IOBluetoothSerialManager * mProvider;                // 400
    IOBluetoothRFCOMMChannel * mRFCOMMChannel;           // 408
    IOLock *                   mTransmitLock;            // 416
    bool                       mBlockConsideredRead;     // 424
    thread_call_t              mSetUpTransmitThreadCall; // 432
    bool                       mNeedToSendBlock;         // 440
} PortInfo_t;

class IOBluetoothSerialClientModemStreamSync : public IOModemSerialStreamSync
{
    OSDeclareDefaultStructors(IOBluetoothSerialClientModemStreamSync)

public:
    virtual bool compareName(OSString * name, OSString ** matched = NULL) const APPLE_KEXT_OVERRIDE;
};

class IOBluetoothSerialClientSerialStreamSync : public IORS232SerialStreamSync
{
    OSDeclareDefaultStructors(IOBluetoothSerialClientSerialStreamSync)

public:
    virtual bool compareName(OSString * name, OSString ** matched = NULL) const APPLE_KEXT_OVERRIDE;
};

class IOBluetoothSerialClient : public IOSerialDriverSync
{
    OSDeclareDefaultStructors(IOBluetoothSerialClient)

public:
    static void serialSignalsCallBack(IOService * provider, int);

    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual bool requestTerminate(IOService * provider, IOOptionBits options) APPLE_KEXT_OVERRIDE;

    virtual IOService * probe(IOService * provider, SInt32 * score) APPLE_KEXT_OVERRIDE;
    virtual bool        start(IOService * provider) APPLE_KEXT_OVERRIDE;
    virtual void        stop(IOService * provider) APPLE_KEXT_OVERRIDE;
    virtual bool        attach(IOService * provider) APPLE_KEXT_OVERRIDE;
    virtual void        detach(IOService * provider) APPLE_KEXT_OVERRIDE;

    static void   changeState(PortInfo_t * port, UInt32 state, UInt32 mask);
    static void   dataInFunction(IOService * target, UInt16, void *);
    static UInt32 readPortState(PortInfo_t * port);
    static void   CheckQueues(PortInfo_t * port);
    static bool   BlockIsGone(IOBluetoothMemoryBlock * block, int, UInt64, UInt64, UInt64, UInt64, UInt64);
    static bool   SendNextBlock(PortInfo_t * port);
    static bool   SetUpTransmit(void * port, void * refCon);
    static void   detachRFCOMMLink(PortInfo_t * port);

    static void SetStructureDefaults(PortInfo_t * port, bool init);
    bool        createSerialStream();

    static bool allocateRingBuffer(CirQueue * Queue, size_t BufferSize);
    static void freeRingBuffer(CirQueue * Queue);

    virtual IOReturn acquirePort(bool sleep, void * refCon) APPLE_KEXT_OVERRIDE;
    virtual IOReturn releasePort(void * refCon) APPLE_KEXT_OVERRIDE;
    virtual IOReturn setState(UInt32 state, UInt32 mask, void * refCon) APPLE_KEXT_OVERRIDE;
    virtual UInt32   getState(void * refCon) APPLE_KEXT_OVERRIDE;
    virtual IOReturn watchState(UInt32 * state, UInt32 mask, void * refCon) APPLE_KEXT_OVERRIDE;
    static IOReturn  watchState(PortInfo_t * port, UInt32 * state, UInt32 mask);
    virtual UInt32   nextEvent(void * refCon) APPLE_KEXT_OVERRIDE;
    virtual IOReturn executeEvent(UInt32 event, UInt32 data, void * refCon) APPLE_KEXT_OVERRIDE;
    static IOReturn  executeEvent(PortInfo_t *, UInt32 event, UInt32 data, UInt32 * oldState, UInt32 * newState);
    virtual IOReturn requestEvent(UInt32 event, UInt32 * data, void * refCon) APPLE_KEXT_OVERRIDE;
    virtual IOReturn enqueueEvent(UInt32 event, UInt32 data, bool sleep, void * refCon) APPLE_KEXT_OVERRIDE;
    virtual IOReturn dequeueEvent(UInt32 * event, UInt32 * data, bool sleep, void * refCon) APPLE_KEXT_OVERRIDE;
    virtual IOReturn enqueueData(UInt8 * buffer, UInt32 size, UInt32 * count, bool sleep, void * refCon) APPLE_KEXT_OVERRIDE;
    virtual IOReturn dequeueData(UInt8 * buffer, UInt32 size, UInt32 * count, UInt32 min, void * refCon) APPLE_KEXT_OVERRIDE;

    static IOReturn activatePort(PortInfo_t * port);
    static void     deactivatePort(PortInfo_t * port);

    static void dataLatTOHandler(PortInfo_t * port);
    static void frameTOHandler(PortInfo_t * port);
    static void delayTOHandler(PortInfo_t * port);
    static void heartBeatTOHandler(PortInfo_t * port);

protected:
    PortInfo_t mSerialPort; // 136
};

#endif
