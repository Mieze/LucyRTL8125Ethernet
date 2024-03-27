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

#ifndef _IOSKYWALKCLONEABLENETWORKPACKET_H
#define _IOSKYWALKCLONEABLENETWORKPACKET_H

#include <IOKit/skywalk/IOSkywalkNetworkPacket.h>

class IOSkywalkCloneableNetworkPacket : public IOSkywalkNetworkPacket
{
    OSDeclareDefaultStructors( IOSkywalkCloneableNetworkPacket )

public:
    static IOSkywalkPacket * withPool( IOSkywalkPacketBufferPool * pool, IOSkywalkPacketDescriptor * desc, IOOptionBits options );
    virtual bool initWithPool( IOSkywalkPacketBufferPool * pool, IOSkywalkPacketDescriptor * desc, IOOptionBits options ) APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;

    IOSkywalkCloneableNetworkPacket * packetClone();
    IOReturn packetCloneWithBaseAndLimit( int64_t base, size_t limit, IOSkywalkCloneableNetworkPacket ** packet );
    IOReturn setPacketBufferBaseAndLimit( int64_t base, size_t limit );
    IOReturn getPacketBufferBaseAddr( uint8_t ** baseAddr );
    IOReturn getPacketBufferObjectBaseAddr( uint8_t ** baseAddr );
    IOReturn getPacketBufferObjectIOBusBaseAddr( uint8_t ** baseAddr );

    virtual UInt32 getPacketBuffers( IOSkywalkPacketBuffer ** buffers, UInt32 count ) APPLE_KEXT_OVERRIDE;
    virtual UInt32 getPacketBufferCount() APPLE_KEXT_OVERRIDE;
    virtual IOMemoryDescriptor * getMemoryDescriptor() APPLE_KEXT_OVERRIDE;
    virtual IOReturn setDataLength( UInt32 length ) APPLE_KEXT_OVERRIDE;
    virtual UInt32 getDataLength() APPLE_KEXT_OVERRIDE;
    virtual IOReturn setDataOffset( UInt16 offset ) APPLE_KEXT_OVERRIDE;
    virtual UInt16 getDataOffset() APPLE_KEXT_OVERRIDE;
    virtual IOReturn setDataOffsetAndLength( UInt16 offset, UInt32 length ) APPLE_KEXT_OVERRIDE;

    virtual IOReturn prepareWithQueue( IOSkywalkPacketQueue * queue, IOSkywalkPacketDirection direction = kIOSkywalkPacketDirectionNone, IOOptionBits options = 0 ) APPLE_KEXT_OVERRIDE;
    virtual IOReturn completeWithQueue( IOSkywalkPacketQueue * queue, IOSkywalkPacketDirection direction = kIOSkywalkPacketDirectionNone, IOOptionBits options = 0 ) APPLE_KEXT_OVERRIDE;

    virtual UInt32 getPacketType() APPLE_KEXT_OVERRIDE;
    virtual kern_buflet_t acquireWithPacketHandle( UInt64 handle, IOOptionBits options ) APPLE_KEXT_OVERRIDE;
    virtual void disposePacket() APPLE_KEXT_OVERRIDE;
    void printPacket();

protected:
    uint64_t _reserved[3]; // 128... I bet they are used or set in another class!
};

#endif
