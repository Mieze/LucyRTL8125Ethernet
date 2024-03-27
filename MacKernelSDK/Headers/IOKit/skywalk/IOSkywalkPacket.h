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

#ifndef _IOSKYWALKPACKET_H
#define _IOSKYWALKPACKET_H

#include <IOKit/IOCommand.h>
#include <IOKit/skywalk/IOSkywalkPacketBufferPool.h>

typedef uint32_t IOSkywalkPacketDirection;
enum
{
    kIOSkywalkPacketDirectionNone = 0x00000000,
    kIOSkywalkPacketDirectionTx   = 0x00000001,
    kIOSkywalkPacketDirectionRx   = 0x00000002
};

enum IOSkywalkPacketTypes
{
    kIOSkywalkPacketTypeGeneric = 0,
    kIOSkywalkPacketTypeNetwork,
    kIOSkywalkPacketTypeCloneable
};

struct IOSkywalkPacketDescriptor
{
    kern_packet_idx_t packetIndex;
    bool singleBuffer;
};

class IOSkywalkPacket : public IOCommand
{
    OSDeclareDefaultStructors( IOSkywalkPacket )

public:
    static IOSkywalkPacket * withPool( IOSkywalkPacketBufferPool * pool, IOSkywalkPacketDescriptor * desc, IOOptionBits options );
    virtual bool initWithPool( IOSkywalkPacketBufferPool * pool, IOSkywalkPacketDescriptor * desc, IOOptionBits options );
    virtual void free() APPLE_KEXT_OVERRIDE;

    IOSkywalkPacketBufferPool * getPacketBufferPool();
    UInt32 getBufferSize();
    virtual UInt32 getPacketBuffers( IOSkywalkPacketBuffer ** buffers, UInt32 maxBuffers );
    virtual UInt32 getPacketBufferCount();
    virtual IOMemoryDescriptor * getMemoryDescriptor();
    virtual IOReturn setDataLength( UInt32 length );
    virtual UInt32 getDataLength();
    virtual IOReturn setDataOffset( UInt16 offset );
    virtual UInt16 getDataOffset();
    virtual IOReturn setDataOffsetAndLength( UInt16 offset, UInt32 length );

    IOSkywalkPacketQueue * getSourceQueue();
    virtual IOReturn prepareWithQueue( IOSkywalkPacketQueue * queue, IOSkywalkPacketDirection direction = kIOSkywalkPacketDirectionNone, IOOptionBits options = 0 );
    virtual IOReturn prepare( IOSkywalkPacketQueue * queue, UInt64 offset, IOOptionBits options = 0 );
    virtual IOReturn completeWithQueue( IOSkywalkPacketQueue * queue, IOSkywalkPacketDirection direction = kIOSkywalkPacketDirectionNone, IOOptionBits options = 0 );
    virtual IOReturn complete( IOSkywalkPacketQueue * queue, IOOptionBits options = 0 );

    void setTransferDirection( IOSkywalkPacketDirection direction );
    IOSkywalkPacketDirection getTransferDirection();
    bool clearTransferDirection( IOSkywalkPacketDirection direction );

    void setSlotReference( void * ref );
    void * getSlotReference();
    virtual UInt32 getPacketType();
    virtual kern_buflet_t acquireWithPacketHandle( kern_packet_t handle, IOOptionBits options );
    void cancelCompletionCallback();
    virtual void disposePacket();

protected:
    void *                      mReserved;      // 32
    kern_packet_t               mPacketHandle;  // 40
    IOSkywalkPacketBuffer    ** mPacketBuffers; // 48
    IOSkywalkPacketQueue      * mSourceQueue;   // 56
    IOSkywalkPacketBufferPool * mBufferPool;    // 64
    kern_buflet_t               mBuflet;        // 72

    uint64_t                 _reserved0;         // 80
    UInt32                   mPacketState;       // 88 - 0 on complete, 1 on acquire, 2 on prepare
    UInt32                   mMaxNumBuffers;     // 92
    UInt32                   mActualNumBuffers;  // 96
    kern_packet_idx_t        mPacketIndex;       // 100
    IOSkywalkPacketDirection mTransferDirection; // 104
    void *                   mSlotReference;     // 112
};

#endif
