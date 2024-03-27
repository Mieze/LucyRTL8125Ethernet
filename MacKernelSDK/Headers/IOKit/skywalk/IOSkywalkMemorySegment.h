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

#ifndef _IOSKYWALKMEMORYSEGMENT_H
#define _IOSKYWALKMEMORYSEGMENT_H

#include <IOKit/IODMACommand.h>
#include <IOKit/skywalk/IOSkywalkPacketBufferPool.h>

struct IOSkywalkMemorySegmentDescriptor
{
    UInt32 packetBufferCount;

    struct
    {
        IODMACommand::SegmentFunction outSegFunc;
        UInt8 numAddressBits;
        UInt64 maxSegmentSize;
        IODMACommand::MappingOptions mappingOptions;
        UInt64 maxTransferSize;
        UInt32 alignment;
        IOMapper * mapper;
    } * specs;
};

class IOSkywalkMemorySegment : public OSObject
{
    OSDeclareDefaultStructors( IOSkywalkMemorySegment )

public:
    static IOSkywalkMemorySegment * withPool( IOSkywalkPacketBufferPool * pool, IOSkywalkMemorySegmentDescriptor * desc, IOOptionBits options = 0 );
    virtual bool initWithPool( IOSkywalkPacketBufferPool * pool, IOSkywalkMemorySegmentDescriptor * desc, IOOptionBits options = 0 );
    virtual void free() APPLE_KEXT_OVERRIDE;

    IOSkywalkPacketBufferPool * getPacketBufferPool();
    virtual IOReturn setDMACommand( IODMACommand * dmaCommand, IOOptionBits options = 0 );
    virtual IODMACommand * getDMACommand();

    virtual IOReturn prepare( IODirection forDirection = kIODirectionNone );
    virtual IOReturn complete( IODirection forDirection = kIODirectionNone );

    virtual IOReturn setBufferMemoryDescriptor( IOBufferMemoryDescriptor * md );
    IOMemoryDescriptor * getMemoryDescriptor();
    virtual IOReturn setMemoryDescriptor( IOMemoryDescriptor * md, UInt64 address );
    UInt64 getVirtualAddress();

protected:
    void                      * mReserved; // 16
    IOSkywalkPacketBufferPool * mPool; // 24
    IOMemoryDescriptor        * mMemoryDescriptor; // 32
    OSObject                  * _unknown; // 40
    UInt64                      mVirtualAddress; // 48
    IODMACommand              * mDMACommand; // 56
    OSArray                   * mPacketBufferArray; // 64, stores IOSkywalkPacketBuffers
    uint64_t                    _reserved1[6]; // 72
};

#endif
