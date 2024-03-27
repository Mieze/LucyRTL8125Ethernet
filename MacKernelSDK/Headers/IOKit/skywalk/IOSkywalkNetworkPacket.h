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

#ifndef _IOSKYWALKNETWORKPACKET_H
#define _IOSKYWALKNETWORKPACKET_H

#include <IOKit/skywalk/IOSkywalkPacket.h>

class IOSkywalkNetworkPacket : public IOSkywalkPacket
{
    OSDeclareDefaultStructors( IOSkywalkNetworkPacket )

public:
    static IOSkywalkNetworkPacket * withPool( IOSkywalkPacketBufferPool * pool, IOSkywalkPacketDescriptor * desc, IOOptionBits options );
    virtual UInt32 getPacketType() APPLE_KEXT_OVERRIDE;

    IOReturn setHeadroom( uint8_t headroom );
    uint8_t  getHeadroom();
    IOReturn setLinkHeaderLength( uint8_t length );
    uint8_t  getLinkHeaderLength();
    IOReturn setLinkHeaderOffset( uint32_t offset );
    IOReturn getLinkHeaderOffset( uint32_t * offset );
    IOReturn setNetworkHeaderOffset( uint32_t offset );
    IOReturn getNetworkHeaderOffset( uint32_t * offset );
    IOReturn setDataContainsFCS( bool contain );
    bool     getDataContainsFCS();
    kern_packet_svc_class_t getServiceClass();

    IOReturn setTimestamp( AbsoluteTime timestamp );
    IOReturn getTimestamp( AbsoluteTime * timestamp );
    IOReturn clearTimestamp();
    bool     isTimestampRequested();
    IOReturn setCompletionStatus( int status );
    IOReturn getExpiryTime( AbsoluteTime * time );

    IOReturn getTokenData( void * data, uint16_t * size );
    IOReturn getPacketID( packet_id_t * packetID );

    bool     isPacketGroupStart();
    bool     isPacketGroupEnd();
    bool     isHighPriority();
    bool     isTransportNewFlow();
    bool     isTransportLastPacket();
    IOReturn setIsLinkBroadcast( bool broadcast );
    bool     isLinkBroadcast();
    IOReturn setIsLinkMulticast( bool multicast );
    bool     isLinkMulticast();

protected:
    void * mReserved;
};

#endif
