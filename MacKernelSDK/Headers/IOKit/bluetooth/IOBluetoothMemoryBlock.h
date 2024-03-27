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

#ifndef _IOKIT_IOBLUETOOTHMEMORYBLOCK_H
#define _IOKIT_IOBLUETOOTHMEMORYBLOCK_H

#include <IOKit/IOMemoryDescriptor.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/bluetooth/Bluetooth.h>

class IOBluetoothMemoryBlock : public OSObject
{
    OSDeclareDefaultStructors(IOBluetoothMemoryBlock)

    typedef void (*IOBluetoothMemoryBlockCallBack)(IOBluetoothMemoryBlock * target, IOOptionBits options, UInt64 arg0, UInt64 arg1, UInt64 arg2, UInt64 arg3, UInt64 arg4);

public:
    static IOBluetoothMemoryBlock * withAddress(void * address, IOByteCount withLength, IODirection withDirection);
    static IOBluetoothMemoryBlock * withCapacity(IOByteCount capacity, IODirection direction);
    static IOBluetoothMemoryBlock * withSubBlock(IOBluetoothMemoryBlock * subBlock, IODirection direction);

    virtual bool initWithAddress(void * address, IOByteCount withLength, IODirection withDirection);
    virtual bool initWithCapacity(IOByteCount capacity, IODirection direction);
    virtual bool initWithSubBlock(IOBluetoothMemoryBlock * subBlock, IODirection direction);

    virtual bool init() APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;

    virtual IOByteCount readBytes(IOByteCount offset, void * bytes, IOByteCount length);
    virtual IOByteCount writeBytes(IOByteCount offset, const void * bytes, IOByteCount length);
    virtual IODirection getDirection();
    virtual IOByteCount getLength();

    virtual IOReturn                 appendBlock(IOBluetoothMemoryBlock * block);
    virtual IOBluetoothMemoryBlock * getBlock();

    virtual IOReturn     setCallBack(IOBluetoothMemoryBlockCallBack callBack, UInt64 data1, UInt64 data2, UInt64 data3, UInt64 data4, UInt64 data5);
    virtual bool         sent(IOOptionBits status);
    virtual bool         hasCallBack();
    virtual IOOptionBits packetSentStatus();

protected:
    void *                         mBytes;            // 16
    IOByteCount                    mBytesLength;      // 24
    bool                           mIsWithCapacity;   // 32
    IOBluetoothMemoryBlock *       mSubBlock;         // 40
    IODirection                    mDirection;        // 48
    IOBluetoothMemoryBlockCallBack mCallBack;         // 56
    UInt64                         mCallBackArgs[5];  // 64
    IOOptionBits                   mPacketSentStatus; // 104
};

class IOBluetoothL2CAPMemoryBlock : public IOBluetoothMemoryBlock
{
	OSDeclareDefaultStructors(IOBluetoothL2CAPMemoryBlock)

public:
    static IOBluetoothL2CAPMemoryBlock * withAddress(void * address, IOByteCount withLength, IODirection withDirection);
    static IOBluetoothL2CAPMemoryBlock * withCapacity(IOByteCount capacity, IODirection direction);
    static IOBluetoothL2CAPMemoryBlock * withSubBlock(IOBluetoothMemoryBlock * subBlock, IODirection direction);

    virtual bool init() APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;

    virtual IOByteCount readBytes(IOByteCount offset, void * bytes, IOByteCount length) APPLE_KEXT_OVERRIDE;
    virtual IOByteCount writeBytes(IOByteCount offset, const void * bytes, IOByteCount length) APPLE_KEXT_OVERRIDE;
    virtual IOByteCount getLength() APPLE_KEXT_OVERRIDE;

    virtual BluetoothL2CAPChannelID readChannelID();
    virtual void                    writeChannelID(BluetoothL2CAPChannelID channelID);

protected:
    BluetoothL2CAPChannelID mChannelID; // 108
};

extern UInt16 IOBluetoothL2CAPFCS(UInt16, UInt8);

class IOBluetoothL2CAPSupervisoryFrameMemoryBlock : public IOBluetoothL2CAPMemoryBlock
{
    OSDeclareDefaultStructors(IOBluetoothL2CAPSupervisoryFrameMemoryBlock)

    typedef UInt8 SupervisoryFunctionMask;

public:
    static IOBluetoothL2CAPSupervisoryFrameMemoryBlock * withDirection(IODirection direction);

    virtual bool init() APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;

    virtual bool sent(IOOptionBits status) APPLE_KEXT_OVERRIDE;
    virtual bool hasCallBack() APPLE_KEXT_OVERRIDE;

    virtual IOByteCount readBytes(IOByteCount offset, void * bytes, IOByteCount length) APPLE_KEXT_OVERRIDE;
    virtual IOByteCount writeBytes(IOByteCount offset, void * bytes, IOByteCount length);
    virtual IOByteCount getLength() APPLE_KEXT_OVERRIDE;

    UInt8         getSupervisoryFunctionMask(UInt16);
    virtual UInt8 getSupervisoryFunctionMask();
    virtual UInt8 setSupervisoryFunctionMask(SupervisoryFunctionMask mask);

    bool          getRetransmissionDisableBit(UInt16);
    virtual bool  getRetransmissionDisableBit();
    virtual UInt8 setRetransmissionDisableBit(bool);

    UInt8         getReceiveSequenceNumber(UInt16);
    virtual UInt8 getReceiveSequenceNumber();
    virtual UInt8 setReceiveSequenceNumber(UInt8);

    virtual IOReturn readFCS(UInt16 *);
    virtual IOReturn writeFCS(UInt16);

protected:
    UInt16 mStandardControlField; // 110
    UInt16 mFrameCheckSequence;   // 112
};

class IOBluetoothL2CAPInformationFrameMemoryBlock : public IOBluetoothL2CAPSupervisoryFrameMemoryBlock
{
    OSDeclareDefaultStructors(IOBluetoothL2CAPInformationFrameMemoryBlock)

    typedef UInt16 SegmentationAndReassemblyMask;

public:
    static IOBluetoothL2CAPInformationFrameMemoryBlock * withDataFromBlock(IOBluetoothL2CAPInformationFrameMemoryBlock * block, IOByteCount offset, IOByteCount size, IODirection direction);
    virtual bool                                         initWithDataFromBlock(IOBluetoothL2CAPInformationFrameMemoryBlock * block, IOByteCount offset, IOByteCount size, IODirection direction);

    virtual bool init() APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;

    virtual UInt64                   readBytes(IOByteCount offset, void * bytes, IOByteCount length) APPLE_KEXT_OVERRIDE;
    virtual UInt64                   writeBytes(IOByteCount offset, void * bytes, IOByteCount length) APPLE_KEXT_OVERRIDE;
    virtual UInt64                   getLength() APPLE_KEXT_OVERRIDE;
    virtual IOBluetoothMemoryBlock * getBlock() APPLE_KEXT_OVERRIDE;

    virtual bool sent(IOOptionBits status) APPLE_KEXT_OVERRIDE;
    virtual bool hasCallBack() APPLE_KEXT_OVERRIDE;

    UInt16         getSegmentationMask(UInt16);
    virtual UInt16 getSegmentationMask();
    virtual UInt16 setSegmentationMask(SegmentationAndReassemblyMask);
    UInt16         getTransmitSequenceNumber(UInt16);
    virtual UInt16 getTransmitSequenceNumber();
    virtual UInt16 setTransmitSequenceNumber(UInt8);
    virtual UInt16 getSDU();
    virtual int    setSDU(UInt16);
    virtual UInt16 getPayload();
    virtual int    acked(int);

protected:
    UInt16 mInfoPacketOffset; // 114
    UInt16 mInfoPacketSize;   // 116
    UInt16 mSDU;              // 118
};

class IOBluetoothRFCOMMMemoryBlock : public IOBluetoothMemoryBlock
{
	OSDeclareDefaultStructors(IOBluetoothRFCOMMMemoryBlock)

public:
    static IOBluetoothRFCOMMMemoryBlock * withAddress(void * address, IOByteCount withLength, IODirection withDirection);
    static IOBluetoothRFCOMMMemoryBlock * withCapacity(IOByteCount capacity, IODirection direction);
    static IOBluetoothRFCOMMMemoryBlock * withSubBlock(IOBluetoothMemoryBlock * subBlock, IODirection direction);
    virtual bool                          init() APPLE_KEXT_OVERRIDE;
    virtual void                          free() APPLE_KEXT_OVERRIDE;

    virtual UInt64 readBytes(UInt64 offset, void * bytes, UInt64 length) APPLE_KEXT_OVERRIDE;
    virtual UInt64 getLength() APPLE_KEXT_OVERRIDE;
    virtual UInt64 getLogicalLength();

    virtual UInt8  readAddressByte();
    virtual void   writeAddressByte(UInt8 whatByte);
    virtual UInt8  readControlByte();
    virtual void   writeControlByte(UInt8 whatByte);
    virtual UInt8  readFCFByte();
    virtual void   writeFCFByte(UInt8 whatByte);
    virtual UInt8  readHeaderBytes(void * bytes, UInt64 length);
    virtual UInt64 writeBytes(UInt64 offset, void * bytes, UInt64 length);

protected:
    UInt8  mAddressByte;     // 108
    UInt8  mControlByte;     // 109
    UInt16 mHeaderBytes;     // 110
    UInt8  mHeaderBytesSize; // 112
    UInt8  mFCFByte;         // 113
};

class IOBluetoothACLMemoryDescriptor : public IOGeneralMemoryDescriptor
{
    OSDeclareDefaultStructors(IOBluetoothACLMemoryDescriptor)

public:
    static IOBluetoothACLMemoryDescriptor * withL2CAPMemoryBlock(IOBluetoothL2CAPMemoryBlock * block, UInt64, UInt64 withOffset, UInt16, IOOptionBits options, bool asReference = false);

    virtual bool initWithAddress(void * address, IOByteCount withLength, IODirection withDirection);
    virtual bool initWithAddress(IOVirtualAddress address, IOByteCount withLength, IODirection withDirection, task_t withTask);
    virtual bool initWithPhysicalAddress(IOPhysicalAddress address, IOByteCount withLength, IODirection withDirection);
    virtual bool initWithPhysicalRanges(IOPhysicalRange * ranges, UInt32 withCount, IODirection withDirection, bool asReference = false);
    virtual bool initWithRanges(IOVirtualRange * ranges, UInt32 withCount, IODirection withDirection, task_t withTask, bool asReference = false);
    virtual bool initWithL2CAPMemoryBlock(IOBluetoothL2CAPMemoryBlock * block, UInt64, UInt64 withOffset, UInt16, IOOptionBits options, bool asReference = false);
    virtual void free() APPLE_KEXT_OVERRIDE;

    virtual IOByteCount readBytes(IOByteCount offset, void * bytes, IOByteCount withLength) APPLE_KEXT_OVERRIDE;
    virtual IOByteCount writeBytes(IOByteCount offset, const void * bytes, IOByteCount withLength) APPLE_KEXT_OVERRIDE;
    virtual IOReturn    prepare(IODirection forDirection = kIODirectionNone) APPLE_KEXT_OVERRIDE;
    virtual IOReturn    complete(IODirection forDirection = kIODirectionNone) APPLE_KEXT_OVERRIDE;

    virtual IOByteCount               getLength() const APPLE_KEXT_OVERRIDE;
    virtual IOByteCount               getPayloadLength();
    virtual bool                      sent(IOOptionBits status);
    virtual int                       packetSentStatus();
    virtual bool                      hasCallBack();
    virtual BluetoothConnectionHandle getConnectionHandle();
    virtual void                      setPacketSentStatusToInvalid();

    bool prepareHCIBlock();

protected:
    IOBluetoothMemoryBlock *  mBlock;            // 176
    IOByteCount               mBuffersOffset;    // 184
    IOByteCount               mLength;           // 192
    BluetoothConnectionHandle mConnectionHandle; // 200
    UInt16                    mHeaderBytes;      // 202
    UInt32                    unknown1;          // 204
    bool                      mAsReference;      // 208
    IOOptionBits              mPacketSentStatus; // 212
    void *                    mData;             // 216
    IOByteCount               mDataSize;         // 224
    uint64_t                  reserved1[2];      // 232
    uint16_t                  reserved2;         // 248
    uint8_t                   reserved3;         // 250
};

class IOBluetoothMemoryDescriptorRetainer : public OSObject
{
    OSDeclareDefaultStructors(IOBluetoothMemoryDescriptorRetainer)

    struct MemoryDescriptorCallBack
    {
        IOMemoryDescriptor * buffer;
        void *               data;
        void *               owner;
        IOTimerEventSource * timer;
    };

public:
    virtual bool init() APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;

    MemoryDescriptorCallBack mMemoryDescriptorCallBack; // 16
};

class IOBluetoothSCOMemoryDescriptorRetainer : public IOBluetoothMemoryDescriptorRetainer
{
    OSDeclareDefaultStructors(IOBluetoothSCOMemoryDescriptorRetainer)

public:
    virtual bool init() APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;

protected:
    MemoryDescriptorCallBack mSCOMemoryDescriptorCallBack; // 48
};

#endif
