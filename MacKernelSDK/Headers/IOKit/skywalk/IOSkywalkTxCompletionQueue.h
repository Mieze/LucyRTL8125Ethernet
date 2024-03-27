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

#ifndef _IOSKYWALKTXCOMPLETIONQUEUE_H
#define _IOSKYWALKTXCOMPLETIONQUEUE_H

#include <IOKit/skywalk/IOSkywalkPacketQueue.h>

extern void queueNotifyAction( IOSkywalkPacketQueue * target, void * refCon );

class IOSkywalkTxCompletionQueue;
class IOSkywalkPacketTable;

typedef IOReturn (*IOSkywalkTxCompletionQueueAction)( OSObject * owner, IOSkywalkTxCompletionQueue *, IOSkywalkPacket **, UInt32, void * );

/*! @struct IOSkywalkTxCompletionQueueStats
    @abstract A structure containing counters of serveral functions in the IOSkywalkTxCompletionQueue class.
    @field enqueueCounter Increment with each successful enqueue operation, regardless of the function utilized.
    @field checkForWorkCounter Increment with each call to checkForWork() if enqueueAndCompletePackets() is executed.
    @field enqueueFailureCounter Increment with each failed enqueue operation.
    @field completeCounter The number of packets completed, updated each time completePackets() is called.
*/
struct IOSkywalkTxCompletionQueueStats
{
    UInt64 enqueueCounter;
    UInt64 checkForWorkCounter;
    UInt64 enqueueFailureCounter;
    UInt64 completeCounter;
};

class IOSkywalkTxCompletionQueue : public IOSkywalkPacketQueue
{
    OSDeclareDefaultStructors( IOSkywalkTxCompletionQueue )

public:
    virtual IOReturn initialize( void * refCon ) APPLE_KEXT_OVERRIDE;
    virtual void finalize() APPLE_KEXT_OVERRIDE;
    virtual void enable() APPLE_KEXT_OVERRIDE;
    virtual void disable() APPLE_KEXT_OVERRIDE;

    virtual IOReturn performCommand( UInt32 command, void * data, size_t dataSize ) APPLE_KEXT_OVERRIDE;
    virtual IOReturn requestEnqueue( void * packets, IOOptionBits options );
    virtual IOReturn enqueuePackets( const IOSkywalkPacket ** packets, UInt32 packetCount, IOOptionBits options );
    virtual IOReturn enqueuePackets( const queue_entry * packets, UInt32 packetCount, IOOptionBits options );
    void completePackets();
    IOReturn enqueueAndCompletePackets( void * packets );
    virtual UInt32 getPacketCount() APPLE_KEXT_OVERRIDE;
    UInt32 getEffectiveCapacity( IOOptionBits options );
    virtual bool checkForWork() APPLE_KEXT_OVERRIDE;

    static IOSkywalkTxCompletionQueue * withPool( IOSkywalkPacketBufferPool * pool, UInt32 capacity, UInt32 queueId, OSObject * owner, IOSkywalkTxCompletionQueueAction action, void * refCon, IOOptionBits options );
    virtual bool initWithPool( IOSkywalkPacketBufferPool * pool, UInt32 capacity, UInt32 queueId, OSObject * owner, IOSkywalkTxCompletionQueueAction action, void * refCon, IOOptionBits options );
    virtual void free() APPLE_KEXT_OVERRIDE;

    void addReporters( IOService * target, OSSet * set );
    UInt64 getReportChannelValue( UInt64 reportChannel );

    OSMetaClassDeclareReservedUnused( IOSkywalkTxCompletionQueue,  0 );
    OSMetaClassDeclareReservedUnused( IOSkywalkTxCompletionQueue,  1 );
    OSMetaClassDeclareReservedUnused( IOSkywalkTxCompletionQueue,  2 );
    OSMetaClassDeclareReservedUnused( IOSkywalkTxCompletionQueue,  3 );
    OSMetaClassDeclareReservedUnused( IOSkywalkTxCompletionQueue,  4 );
    OSMetaClassDeclareReservedUnused( IOSkywalkTxCompletionQueue,  5 );
    OSMetaClassDeclareReservedUnused( IOSkywalkTxCompletionQueue,  6 );
    OSMetaClassDeclareReservedUnused( IOSkywalkTxCompletionQueue,  7 );
    OSMetaClassDeclareReservedUnused( IOSkywalkTxCompletionQueue,  8 );
    OSMetaClassDeclareReservedUnused( IOSkywalkTxCompletionQueue,  9 );
    OSMetaClassDeclareReservedUnused( IOSkywalkTxCompletionQueue, 10 );

protected:
    /*! @var mReserved
     *   The first variable of every class in IOSkywalkFamily is reserved.
     *   Offset: 192. */
    void * mReserved;

    /*! @var mTable
     *   The IOSkywalkPacketTable in which the packets of this queue are stored.
     *   Offset: 200. */
    IOSkywalkPacketTable * mTable;

    /*! @var mInitLock
     *   An IOLock governing the initialize() and finalize() routines.
     *   Offset: 208. */
    IOLock * mInitLock;

    /*! @var mStats
     *   Structure containing various counters related to this class.
     *   Offset: 216. */
    IOSkywalkTxCompletionQueueStats * mStats;

    /*! @var mCommandData
     *   Data for command 0x50000.
     *   Offset: 224. */
    void * mCommandData;

    uint64_t _reserved0[2];

    /*! @var mActionStatus
     *   Set when $link action runs and clear when it completes. No new enqueue request should happen when this flag is set.
     *   Offset: 248. */
    UInt32 mActionStatus;

    /*! @var mInitCounter
     *   Incremented by initialize() and decremented by finalize().
     *   Offset: 252. */
    UInt32 mInitCounter;

    /*! @var mWorkCounter
     *   The number of works to do, incremented in requestEnqueue(). A work in this class could be considered as an enqueue request.
     *   Offset: 256. */
    UInt32 mWorkCounter;

    /*! @var mNumCheckedWorks
     *   Updated with the value of $link mWorkCounter whenever checkForWork() is called.
     *   Offset: 260. */
    UInt32 mNumCheckedWorks;

    uint64_t _reserved1[2];
};

#endif
