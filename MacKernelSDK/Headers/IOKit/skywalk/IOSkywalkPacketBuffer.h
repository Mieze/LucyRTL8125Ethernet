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

#ifndef _IOSKYWALKPACKETBUFFER_H
#define _IOSKYWALKPACKETBUFFER_H

#include <skywalk/os_skywalk.h>
#include <IOKit/IOCommand.h>
#include <IOKit/skywalk/IOSkywalkTypes.h>

class IOSkywalkPacket;
class IOSkywalkPacketQueue;
class IOSkywalkPacketBufferPool;
class IOSkywalkMemorySegment;

struct IOSkywalkPacketBufferDescriptor
{
    IOMemoryDescriptor * memDescriptor;
    IOSkywalkMemorySegment * memSegment;
    UInt64 memSegmentOffset;
};

class IOSkywalkPacketBuffer : public IOCommand
{
    OSDeclareDefaultStructors( IOSkywalkPacketBuffer )

public:
    static IOSkywalkPacketBuffer * withPool( IOSkywalkPacketBufferPool * pool, IOSkywalkPacketBufferDescriptor * desc, IOOptionBits options = 0 );
    virtual bool initWithPool( IOSkywalkPacketBufferPool * pool, IOSkywalkPacketBufferDescriptor * desc, IOOptionBits options = 0 );
    virtual void free() APPLE_KEXT_OVERRIDE;

    IOSkywalkPacketBufferPool * getPacketBufferPool();
    UInt32 getBufferSize();

    UInt32 getDataLength();
    IOReturn setDataLength( UInt32 length );
    UInt16 getDataOffset();
    IOReturn setDataOffset( UInt16 offset );
    IOReturn setDataOffsetAndLength( UInt16 offset, UInt32 length );

    IOSkywalkPacket * getPacket();
    virtual void setPacket( IOSkywalkPacket * packet, IOOptionBits options );

    virtual IOReturn prepare( IODirection forDirection = kIODirectionNone );
    virtual IOReturn complete( IODirection forDirection = kIODirectionNone );

    IOSkywalkMemorySegment * getMemorySegment();
    UInt64 getMemorySegmentOffset();
    virtual IOReturn prepareWithMemorySegment( IOSkywalkMemorySegment * segment, UInt64 offset, IOOptionBits options = 0 );
    virtual IOReturn completeWithMemorySegment( IOSkywalkMemorySegment * segment, UInt64 offset, IOOptionBits options = 0 );

    IOSkywalkPacketQueue * getSourceQueue();
    virtual IOReturn prepareWithQueue( IOSkywalkPacketQueue * queue, IODirection direction = kIODirectionNone, IOOptionBits options = 0 );
    virtual IOReturn completeWithQueue( IOSkywalkPacketQueue * queue, IODirection direction = kIODirectionNone, IOOptionBits options = 0 );

    kern_buflet_t getBufletHandle();
    void setBufletHandle( kern_buflet_t handle );
    virtual void acquireWithBufletHandle( kern_buflet_t handle );

    UInt64 getBufferHandle();
    virtual void acquireWithBufferHandle( UInt64 handle );

    virtual void disposePacketBuffer();
    void cancelCompletionCallback();

public:
    void                      * mReserved;     // 32
    IOSkywalkPacketBufferPool * mPool;         // 40
    kern_buflet_t               mBufletHandle; // 48
    UInt64                      mBufferHandle; // 56

    IOSkywalkPacket        * mPacket;        // 64
    IOSkywalkPacketQueue   * mSourceQueue;   // 72
    IOMemoryDescriptor     * mMemDescriptor; // 80
    IOSkywalkMemorySegment * mMemorySegment; // 88
    uint64_t                 __reserved0[2]; // 96

    UInt64 mMemorySegmentOffset; // 112
    UInt32 mBufferState;         // 120
    UInt32 mDataLength;          // 124
    UInt16 mDataOffset;          // 128

    uint64_t _reserved1; // 136
};

#endif
