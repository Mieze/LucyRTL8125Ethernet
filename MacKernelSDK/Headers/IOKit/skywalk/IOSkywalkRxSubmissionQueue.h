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

#ifndef _IOSKYWALKRXSUBMISSIONQUEUE_H
#define _IOSKYWALKRXSUBMISSIONQUEUE_H

#include <IOKit/skywalk/IOSkywalkPacketQueue.h>

class IOSkywalkRxSubmissionQueue;
class IOSkywalkPacketTable;

typedef IOReturn (*IOSkywalkRxSubmissionQueueAction)( OSObject * owner, IOSkywalkRxSubmissionQueue *, const IOSkywalkPacket **, UInt32, void * );

struct IOSkywalkRxSubmissionQueueStats; // 56

class IOSkywalkRxSubmissionQueue : public IOSkywalkPacketQueue
{
    OSDeclareDefaultStructors( IOSkywalkRxSubmissionQueue )

public:
    virtual IOReturn initialize( void * refCon ) APPLE_KEXT_OVERRIDE;
    virtual void finalize() APPLE_KEXT_OVERRIDE;
    virtual void enable() APPLE_KEXT_OVERRIDE;
    virtual void disable() APPLE_KEXT_OVERRIDE;

    void gatedDequeue( void * refCon );
    virtual IOReturn requestDequeue( void * refCon, IOOptionBits options );
    void retryDequeue();

    virtual IOReturn performCommand( UInt32 command, void * data, size_t dataSize ) APPLE_KEXT_OVERRIDE;
    virtual void packetCompletion( IOSkywalkPacket * packet, IOSkywalkPacketQueue * queue, IOOptionBits options ) APPLE_KEXT_OVERRIDE;
    void adjustPacketCounters();
    virtual UInt32 getPacketCount() APPLE_KEXT_OVERRIDE;
    UInt32 getEffectiveCapacity( IOOptionBits options );
    virtual bool checkForWork() APPLE_KEXT_OVERRIDE;

    static IOSkywalkRxSubmissionQueue * withPool( IOSkywalkPacketBufferPool * pool, UInt32 capacity, UInt32 queueId, kern_packet_svc_class_t serviceClass, OSObject * owner, IOSkywalkRxSubmissionQueueAction action, void * refCon, IOOptionBits options );
    virtual bool initWithPool( IOSkywalkPacketBufferPool * pool, UInt32 capacity, UInt32 queueId, kern_packet_svc_class_t serviceClass, OSObject * owner, IOSkywalkRxSubmissionQueueAction action, void * refCon, IOOptionBits options )
    virtual void free() APPLE_KEXT_OVERRIDE;

    void addReporters( IOService * target, OSSet * set );
    UInt64 getReportChannelValue( UInt64 reportChannel );

    OSMetaClassDeclareReservedUnused( IOSkywalkRxSubmissionQueue,  0 );
    OSMetaClassDeclareReservedUnused( IOSkywalkRxSubmissionQueue,  1 );
    OSMetaClassDeclareReservedUnused( IOSkywalkRxSubmissionQueue,  2 );
    OSMetaClassDeclareReservedUnused( IOSkywalkRxSubmissionQueue,  3 );
    OSMetaClassDeclareReservedUnused( IOSkywalkRxSubmissionQueue,  4 );
    OSMetaClassDeclareReservedUnused( IOSkywalkRxSubmissionQueue,  5 );
    OSMetaClassDeclareReservedUnused( IOSkywalkRxSubmissionQueue,  6 );
    OSMetaClassDeclareReservedUnused( IOSkywalkRxSubmissionQueue,  7 );
    OSMetaClassDeclareReservedUnused( IOSkywalkRxSubmissionQueue,  8 );
    OSMetaClassDeclareReservedUnused( IOSkywalkRxSubmissionQueue,  9 );
    OSMetaClassDeclareReservedUnused( IOSkywalkRxSubmissionQueue, 10 );

protected:
    void * mReserved; // 192
    IOSkywalkPacketTable * mTable; // 200
    IOLock * mInitLock; // 208
    IOSkywalkRxSubmissionQueueStats * mStats; // 216
    thread_call_t mRetryDequeueThread; // 224
    uint64_t _reserved0; // 232
    UInt32 mQueueId; // 240
    UInt32 ; // 244
    UInt32 ; // 248
    UInt32 ; // 252
    UInt32 ; // 256
    UInt32 ; // 260
    UInt32 ; // 264
    uint64_t _reserved1[2]; // 272
};

#endif
