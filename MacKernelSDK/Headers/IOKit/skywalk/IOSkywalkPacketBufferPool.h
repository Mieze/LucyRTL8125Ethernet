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

#ifndef _IOSKYWALKPACKETBUFFERPOOL_H
#define _IOSKYWALKPACKETBUFFERPOOL_H

#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/skywalk/IOSkywalkPacketBuffer.h>

extern void pp_ctx_retain( void * target );
extern void pp_ctx_release( void * target );

struct SegmentStats
{
    UInt64 constructorCount;
    UInt64 destructorCount;
    UInt64 prepareErrors;
    UInt32 prepareMaxRetries;
    UInt32 prepareErrorCode;
};

struct IOSkywalkPacketDescriptor;
struct IOSkywalkPacketBufferDescriptor;
struct IOSkywalkMemorySegmentDescriptor;

class IOSkywalkInterface;

class IOSkywalkPacketBufferPool : public OSObject
{
    OSDeclareDefaultStructors( IOSkywalkPacketBufferPool )

    /*! @enum IOSkywalkPacketBufferPoolFlags
        @abstract Flags for PoolOptions::poolFlags.
        @constant PoolFlagSingleMemorySegment The pool will be constucted with single memory segment of buffers.
        @constant PoolFlagPersistentMemory The pool memory will be persistently wired.
    */

    enum IOSkywalkPacketBufferPoolFlags
    {
        kIOSkywalkPacketBufferPoolFlagSingleMemorySegment = 0x00000002,
        kIOSkywalkPacketBufferPoolFlagPersistentMemory    = 0x00000004,
        kIOSkywalkPacketBufferPoolFlagFlagMask            = kIOSkywalkPacketBufferPoolFlagSingleMemorySegment | kIOSkywalkPacketBufferPoolFlagPersistentMemory
    };

    struct PoolOptions
    {
        uint32_t packetCount;         // 0
        uint32_t bufferCount;         // 4
        uint32_t bufferSize;          // 8
        uint32_t maxBuffersPerPacket; // 12
        uint32_t memorySegmentSize;   // 16
        uint32_t poolFlags;           // 20
        uint64_t _reserved;           // 24
    };

public:
    static IOSkywalkPacketBufferPool * withName( const char * name, OSObject * owner, UInt32 packetType, const PoolOptions * options );
    virtual bool initWithName( const char * name, void * owner, UInt32 packetType, const PoolOptions * options );
    virtual bool initWithName( const char * name, OSObject * owner, UInt32 packetType, const PoolOptions * options );
    const char * getPoolName();
    virtual void free() APPLE_KEXT_OVERRIDE;
    void disposeAllPackets();
    void disposeAllBuffers();
    virtual void release() const APPLE_KEXT_OVERRIDE;
    void releaseAllPackets();
    void releaseAllMemorySegments();

    void checkInPacketQueue( const IOSkywalkPacketQueue * queue );
    void checkOutPacketQueue( const IOSkywalkPacketQueue * queue );
    
    void reportingChangeNotification();
    void checkInReportingService( IOService * service, const IOSkywalkInterface * interface );
    void checkOutReportingService( const IOService * service );
    void addPoolReporter( IOService * service, OSSet * reportSet );
    void createReportersForService( IOService * service, const IOSkywalkInterface * interface );
    void removeReportersForService( const IOService * service );
    OSSet * copyReportersForService( const IOService * service );
    UInt64 getReportChannelValue( UInt64 reportChannel );

    virtual IOReturn allocatePacket( UInt32 bufCount, IOSkywalkPacket ** outPacket, IOOptionBits options ); // no sleep if (options & 1)
    virtual IOReturn allocatePackets( UInt32 bufCount, UInt32 * numPackets, IOSkywalkPacket ** outPackets, IOOptionBits options );
    virtual IOReturn deallocatePacket( IOSkywalkPacket * packet );
    virtual IOReturn deallocatePackets( IOSkywalkPacket ** packets, uint32_t numPackets );
    virtual IOReturn deallocatePacketChain( kern_packet_t packet );
    
    virtual IOReturn allocatePacketBuffer( IOSkywalkPacketBuffer ** outBuf, IOOptionBits options );
    virtual IOReturn allocatePacketBuffers( UInt32 * numBufs, IOSkywalkPacketBuffer ** outBufs, IOOptionBits options );
    virtual IOReturn deallocatePacketBuffer( IOSkywalkPacketBuffer * buf );
    virtual IOReturn deallocatePacketBuffers( IOSkywalkPacketBuffer ** bufs, uint32_t numBufs );
    
    static void segmentConstructor( kern_pbufpool_t pbufPool, kern_segment_t segment, IOMemoryDescriptor * md );
    static void segmentDestructor( kern_pbufpool_t pbufPool, kern_segment_t segment, IOMemoryDescriptor * md );
    bool createSegmentBuffers( IOSkywalkMemorySegment * segment, UInt32 numBuffers, bool subDesc );
    void destroySegmentBuffers( IOSkywalkMemorySegment * segment );
    IOReturn prepareMemorySegment( IOSkywalkMemorySegment * segment, IOBufferMemoryDescriptor * buffer, UInt64 offset );

    virtual IOReturn newPacket( IOSkywalkPacketDescriptor * desc, IOSkywalkPacket ** outPacket );
    IOSkywalkPacket * getPacketWithHandle( kern_packet_t handle );
    IOSkywalkPacket * getPacketWithIndex( kern_packet_idx_t index );
    
    virtual IOReturn newPacketBuffer( IOSkywalkPacketBufferDescriptor * desc, IOSkywalkPacketBuffer ** outBuffer );
    IOSkywalkPacketBuffer * getPacketBufferWithBufletHandle( kern_buflet_t handle );
    IOSkywalkPacketBuffer * getPacketBufferWithSegmentInfo( kern_segment_t segment, UInt32 bufIndex );
    
    virtual IOReturn newMemorySegment( IOSkywalkMemorySegmentDescriptor * desc, IOSkywalkMemorySegment ** outSegment );
    IOSkywalkMemorySegment * getMemorySegmentWithHandle( kern_segment_t handle );

protected:
    void                 * mReserved;  // 16
    kern_pbufpool_t        mPbufPool;  // 24
    OSObject             * mProvider;  // 32

    UInt32 mPacketCount;       // 40
    UInt32 mBufferCount;       // 44
    UInt32 mBufferSize;        // 48
    UInt32 mMaxFragments;      // 52
    UInt32 mSegmentCount;      // 56
    UInt32 mPacketType;        // 60
    UInt32 mSegmentSize;       // 64
    UInt32 mBuffersPerSegment; // 68
    UInt32 mPoolFlags;         // 72
    
    uint64_t                 _reserved;     // 80
    IOSkywalkMemorySegment * mMemSegment;   // 88
    OSArray                * mPacketArray;  // 96
    OSArray                * mSegmentArray; // 104
    uint64_t                 _reserved2;    // 112
    SegmentStats           * mSegmentStats; // 120

    IOLock   *    mSetLock;                           // 128 - protect the OSSets
    OSSet    *    mPacketQueueSet;                    // 136
    OSString *    mPoolName;                          // 144
    OSSet    *    mReporterSet;                       // 152
    OSSet    *    mReportingServiceSet;               // 160
    thread_call_t mReportingChangeNotificationThread; // 168
    bool          mSingleMemorySemgent;               // 176
    bool          mPersistentMemory;                  // 177
    bool          mSegmentBuffersWithSubRange;        // 178
    bool          mDisposed;                          // 179
};

#endif
