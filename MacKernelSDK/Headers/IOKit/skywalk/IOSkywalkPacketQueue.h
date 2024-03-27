/*
 * Released under "The BSD 3-Clause License"
 *
 * Copyright (c) 2022 cjiang. All rights reserved.
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

#ifndef _IOSKYWALKPACKETQUEUE_H
#define _IOSKYWALKPACKETQUEUE_H

#include <IOKit/skywalk/IOSkywalkPacket.h>
#include <IOKit/IOEventSource.h>

class IOSkywalkPacketPoller;

typedef void (*IOSkywalkPacketQueueNotifyAction)(IOSkywalkPacketQueue * owner, void * arg);

class IOSkywalkPacketQueue : public IOEventSource
{
    OSDeclareAbstractStructors( IOSkywalkPacketQueue )

public:
    virtual IOReturn initialize( void * refCon );
    virtual void finalize();
    virtual IOReturn synchronize( IOOptionBits options );
    virtual IOReturn syncPackets( kern_packet_t * packetArray, UInt32 * packetCount, IOOptionBits options );
    virtual IOReturn sendNotification( IOOptionBits options );
    virtual IOReturn handleDoorbell( IOOptionBits options );

    void * getRefcon();
    virtual IOSkywalkPacketBufferPool * getPacketBufferPool();
    virtual IOReturn setPacketBufferPool( IOSkywalkPacketBufferPool * pool );
    IOSkywalkInterface * getInterface();
    void setInterface( IOSkywalkInterface * interface );

    bool isQueueReady();
    kern_packet_svc_class_t getServiceClass();

    IODirection getDirection();
    UInt32 getQueueId();
    UInt32 getCapacity();
    UInt32 getBufferSize();
    virtual UInt32 getPacketCount() = 0;
    virtual UInt32 getPacketBufferCount();
    virtual UInt32 getFreeSpace();
    virtual IOReturn getDataByteCount( UInt32 * count );

    virtual IOReturn setPacketPoller( IOSkywalkPacketPoller * poller );
    virtual IOReturn performCommand( UInt32 command, void * data, size_t dataSize );
    // Call to default completion methods cause panics
    virtual void packetCompletion( IOSkywalkPacket * packet, IOSkywalkPacketQueue * queue, IOOptionBits options );
    virtual void bufferCompletion( IOSkywalkPacketBuffer * buffer, IOSkywalkPacketQueue * queue, IOOptionBits options );
    void scheduleQueueNotification( IOSkywalkPacketQueue * queue );
    void deliverQueueNotifications( IOSkywalkPacketQueueNotifyAction notification, void * param );
    static UInt64 statGetElapsedTime( const AbsoluteTime * startTime );

    virtual bool initWithPool( IOSkywalkPacketBufferPool * pool, UInt32 bufferSize, IODirection direction, UInt32 capacity, UInt32 queueId, OSObject * owner, IOEventSource::Action action, void * refCon );
    virtual void free() APPLE_KEXT_OVERRIDE;

    OSMetaClassDeclareReservedUnused( IOSkywalkPacketQueue,  0 );
    OSMetaClassDeclareReservedUnused( IOSkywalkPacketQueue,  1 );
    OSMetaClassDeclareReservedUnused( IOSkywalkPacketQueue,  2 );
    OSMetaClassDeclareReservedUnused( IOSkywalkPacketQueue,  3 );
    OSMetaClassDeclareReservedUnused( IOSkywalkPacketQueue,  4 );
    OSMetaClassDeclareReservedUnused( IOSkywalkPacketQueue,  5 );
    OSMetaClassDeclareReservedUnused( IOSkywalkPacketQueue,  6 );
    OSMetaClassDeclareReservedUnused( IOSkywalkPacketQueue,  7 );
    OSMetaClassDeclareReservedUnused( IOSkywalkPacketQueue,  8 );
    OSMetaClassDeclareReservedUnused( IOSkywalkPacketQueue,  9 );
    OSMetaClassDeclareReservedUnused( IOSkywalkPacketQueue, 10 );

protected:
    void * mReserved; // 72
    IOSkywalkPacketBufferPool * mPool; // 80
    void * mRefcon; // 88
    IOSkywalkPacketQueue * mNotificationOwner; // 96
    OSArray * mNotificationArray; // 104
    IOSkywalkInterface * mInterface; // 112
    
    uint64_t _reserved0[4]; // 120
    
    UInt32 mQueueId; // 152
    UInt32 mCapacity; // 156
    UInt32 mBufferSize; // 160
    IODirection mDirection; // 164
    kern_packet_svc_class_t mServiceClass; // 168
    uint64_t _reserved1[2]; // 172
    bool mQueueReady; // 188
};

#endif
