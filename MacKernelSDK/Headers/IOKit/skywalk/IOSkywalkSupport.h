/*
 * Copyright (c) 2015-2022 Apple, Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _IOSKYWALKSUPPORT_H
#define _IOSKYWALKSUPPORT_H

#include <sys/queue.h>
#include <IOKit/IOService.h>
#include <IOKit/IOBufferMemoryDescriptor.h>
#include <IOKit/IOMultiMemoryDescriptor.h>

typedef uint32_t        IOSKSize;
typedef uint32_t        IOSKIndex;
typedef uint32_t        IOSKCount;
typedef uint32_t        IOSKOffset;

typedef struct
{
    boolean_t userWritable;         /* writable by user task */
    boolean_t kernelWritable;       /* writable by kernel task */
    boolean_t iodirIn;              /* direction: device-to-host */
    boolean_t iodirOut;             /* direction: host-to-device */
    boolean_t purgeable;            /* purgeable (not wired) */
    boolean_t inhibitCache;         /* cache-inhibit */
    boolean_t physContig;           /* physically contiguous */
    boolean_t pureData;             /* data only, no pointers */
} IOSKMemoryBufferSpec;

typedef struct
{
    boolean_t noRedirect;
} IOSKRegionSpec;

const OSSymbol * IOSKCopyKextIdentifierWithAddress( vm_address_t address );

class IOMemoryDescriptor;
typedef IOMemoryDescriptor *        IOSKMemoryRef;
typedef IOSKMemoryRef               IOSKMemoryArrayRef;
typedef IOSKMemoryRef               IOSKMemoryBufferRef;

class IOSKArena;
typedef IOSKArena *                 IOSKArenaRef;

class IOSKRegion;
typedef IOSKRegion *                IOSKRegionRef;

class IOSKMapper;
typedef IOSKMapper *                IOSKMapperRef;
typedef IOSKMemoryRef               IOSKMemoryDescriptor;

class IOMemoryMap;
typedef IOMemoryMap *               IOSKMemoryMapRef;

#define IOSK_CONSUMED LIBKERN_CONSUMED

__BEGIN_DECLS

IOSKMemoryBufferRef IOSKMemoryBufferCreate( mach_vm_size_t capacity,
    const IOSKMemoryBufferSpec * spec,
    mach_vm_address_t * kvaddr );

IOSKMemoryArrayRef  IOSKMemoryArrayCreate( const IOSKMemoryRef refs[],
    uint32_t count );

void                IOSKMemoryDestroy( IOSK_CONSUMED IOSKMemoryRef reference );

IOSKMemoryMapRef    IOSKMemoryMapToTask( IOSKMemoryRef reference,
    task_t intoTask,
    mach_vm_address_t * mapAddr,
    mach_vm_size_t * mapSize );

IOSKMemoryMapRef    IOSKMemoryMapToKernelTask( IOSKMemoryRef reference,
    mach_vm_address_t * mapAddr,
    mach_vm_size_t * mapSize );

void                IOSKMemoryMapDestroy(
    IOSK_CONSUMED IOSKMemoryMapRef reference );

IOReturn            IOSKMemoryReclaim( IOSKMemoryRef reference );
IOReturn            IOSKMemoryDiscard( IOSKMemoryRef reference );

IOReturn            IOSKMemoryWire( IOSKMemoryRef reference );
IOReturn            IOSKMemoryUnwire( IOSKMemoryRef reference );

IOSKArenaRef
IOSKArenaCreate( IOSKRegionRef * regionList, IOSKCount regionCount );

void
IOSKArenaDestroy( IOSK_CONSUMED IOSKArenaRef arena );

void
IOSKArenaRedirect( IOSKArenaRef arena );

IOSKRegionRef
IOSKRegionCreate( const IOSKRegionSpec * regionSpec,
    IOSKSize segmentSize, IOSKCount segmentCount );

void
IOSKRegionDestroy( IOSK_CONSUMED IOSKRegionRef region );

IOReturn
IOSKRegionSetBuffer( IOSKRegionRef region, IOSKIndex segmentIndex,
    IOSKMemoryBufferRef buffer );

void
IOSKRegionClearBuffer( IOSKRegionRef region, IOSKIndex segmentIndex );

void
IOSKRegionClearBufferDebug( IOSKRegionRef region, IOSKIndex segmentIndex,
    IOSKMemoryBufferRef * prevBufferRef );

IOSKMapperRef
IOSKMapperCreate( IOSKArenaRef arena, task_t task );

void
IOSKMapperDestroy( IOSK_CONSUMED IOSKMapperRef mapper );

void
IOSKMapperRedirect( IOSKMapperRef mapper );

IOReturn
IOSKMapperGetAddress( IOSKMapperRef mapper,
    mach_vm_address_t * address, mach_vm_size_t * size );

boolean_t
IOSKBufferIsWired( IOSKMemoryBufferRef buffer );

__END_DECLS

class IOSKMemoryArray : public IOMultiMemoryDescriptor
{
    OSDeclareFinalStructors( IOSKMemoryArray )

public:
    bool overwriteMappingInTask(
        task_t              intoTask,
        mach_vm_address_t * startAddr,
        IOOptionBits        options );
};

class IOSKMemoryBuffer : public IOBufferMemoryDescriptor
{
    OSDeclareFinalStructors( IOSKMemoryBuffer )

public:
    bool initWithSpec( task_t            inTask,
        mach_vm_size_t    capacity,
        mach_vm_address_t alignment,
        const IOSKMemoryBufferSpec * spec );

    virtual void * getBytesNoCopy( void ) APPLE_KEXT_OVERRIDE;

    virtual void * getBytesNoCopy(vm_size_t start, vm_size_t withLength) APPLE_KEXT_OVERRIDE;

    bool
    isWired( void ) const
    {
        return _wireCount != 0;
    }

    IOSKMemoryBufferSpec    fSpec;
    void                    *fKernelAddr;
    IOMemoryMap             *fKernelReadOnlyMapping;

protected:
    virtual void taggedRelease(const void *tag = NULL) const APPLE_KEXT_OVERRIDE;
    virtual void free( void ) APPLE_KEXT_OVERRIDE;
};

// FIXME: rename IOSKMemoryBuffer -> IOSKBuffer
typedef IOSKMemoryBuffer    IOSKBuffer;

/*! @class IOSKRegionMapper
    @abstract IOSkywalk memory mapper of a IOSKRegion.
    @discussion Tracks all memory mappings of a single IOSKRegion, with an array of IOMemoryMaps to map the region's memory segments. Created and released by the parent IOSKMapper.
*/

class IOSKRegionMapper : public OSObject
{
    OSDeclareFinalStructors( IOSKRegionMapper )

public:
    bool initWithMapper( IOSKMapper * mapper, IOSKRegion * region,
        IOSKOffset regionOffset );

    IOReturn    map( IOSKIndex segIndex, IOSKBuffer * buffer );
    void        unmap( IOSKIndex segIndex, vm_prot_t prot );

    kern_return_t mapOverwrite( vm_map_offset_t addr,
        vm_map_size_t size, vm_prot_t prot );

private:
    virtual void free( void ) APPLE_KEXT_OVERRIDE;

    IOSKMapper *        fMapper;
    IOSKRegion *        fRegion;
    IOMemoryMap **      fMemoryMaps;
    IOSKCount           fMemoryMapCount;
    IOSKOffset          fRegionOffset;
};

/*! @class IOSKMapper
    @abstract IOSkywalk memory mapper of a task.
    @discussion Manages all memory mappings of a single task, with an array of IOSKRegionMappers to map all memory regions of a memory arena. Retains the IOSKArena.
*/

class IOSKMapper : public OSObject
{
    OSDeclareFinalStructors( IOSKMapper )
    friend class IOSKRegionMapper;

public:
    bool     initWithTask( task_t task, IOSKArena * arena );

    IOReturn map( IOSKIndex regIndex, IOSKIndex segIndex, IOSKBuffer * buffer );
    void     unmap( IOSKIndex regIndex, IOSKIndex segIndex, vm_prot_t prot );

    mach_vm_address_t
    getMapAddress( mach_vm_size_t * size ) const
    {
        if (size)
            *size = fMapSize;
        return fMapAddr;
    }

    IOSKArena *
    getArena( void ) const
    {
        return fArena;
    }
    bool
    isRedirected( void ) const
    {
        return fRedirected;
    }
    void
    redirectMap( void )
    {
        fRedirected = true;
    }

private:
    virtual void free( void ) APPLE_KEXT_OVERRIDE;

    task_t              fTask;
    vm_map_t            fTaskMap;
    IOSKArena *         fArena;
    OSArray *           fSubMaps;
    mach_vm_address_t   fMapAddr;
    mach_vm_size_t      fMapSize;
    bool                fRedirected;
};

/*! @class IOSKArena
    @abstract IOSkywalk memory buffer arena consisting of IOSKRegions.
    @discussion An array of IOSKRegions is used to create an IOSKArena. One or more IOSKMapper can map the arena memory to tasks. Retains the IOSKRegions, also circularly retains the IOSKMapper(s) until the client calls IOSKMapperDestroy().
*/

class IOSKArena : public OSObject
{
    OSDeclareFinalStructors( IOSKArena )

public:
    bool     initWithRegions( IOSKRegion ** regions,
        IOSKCount regionCount );

    IOReturn createMapperForTask( task_t task,
        LIBKERN_RETURNS_RETAINED IOSKMapper ** mapper );
    void     redirectMap( IOSKMapper * mapper );

    IOSKSize
    getArenaSize( void ) const
    {
        return fArenaSize;
    }
    IOSKCount
    getRegionCount( void ) const
    {
        return fRegions->getCount();
    }
    IOSKRegion * getRegion( IOSKIndex regIndex ) const;

    IOReturn map( const IOSKRegion * region,
        IOSKOffset regionOffset,
        IOSKIndex regionIndex,
        IOSKIndex segmentIndex,
        IOSKMemoryBuffer * buffer );

    void     unmap( const IOSKRegion * region,
        IOSKOffset regionOffset,
        IOSKIndex regionIndex,
        IOSKIndex segmentIndex,
        vm_prot_t prot,
        bool isRedirected,
        const void * context );

    bool     addMapper( const IOSKMapper * mapper );
    void     removeMapper( const IOSKMapper * mapper );

private:
    virtual void free( void ) APPLE_KEXT_OVERRIDE;

    IOLock *        fArenaLock;
    OSSet *         fMappers;
    OSArray *       fRegions;
    IOSKSize        fArenaSize;
};

/*! @class IOSKRegion
    @abstract IOSkywalk memory buffer region.
    @discussion An IOSKRegion manages a dynamic array of IOSKBuffers representing each memory segment in the region. Each IOSKRegion can be shared by multiple IOSKArenas, and the IOSKRegion keeps state specific to each arena - the offset and the index of the region within the arena. A lock is used to serialize updates to the IOSKBuffer array and the arenas. Retains the IOSKBuffers.
*/

class IOSKRegion : public OSObject
{
    OSDeclareFinalStructors( IOSKRegion )

public:
    bool     initWithSpec( const IOSKRegionSpec * spec,
        IOSKSize segSize, IOSKCount segCount );

    IOReturn setSegmentBuffer( IOSKIndex index, IOSKBuffer * buf );
    void     clearSegmentBuffer( IOSKIndex index, IOSKMemoryBufferRef * prevBuffer );

    bool     attachArena( IOSKArena * arena,
        IOSKOffset regionOffset, IOSKIndex regionIndex );
    void     detachArena( const IOSKArena * arena );

    IOReturn updateMappingsForArena( IOSKArena * arena, bool redirect,
        const void * context = NULL );

    IOSKCount
    getSegmentCount( void ) const
    {
        return fSegmentCount;
    }
    IOSKSize
    getSegmentSize( void ) const
    {
        return fSegmentSize;
    }
    IOSKSize
    getRegionSize( void ) const
    {
        return fSegmentCount * fSegmentSize;
    }

private:
    virtual void free( void ) APPLE_KEXT_OVERRIDE;

    struct Segment
    {
        IOSKBuffer *  fBuffer;
    };

    struct ArenaEntry
    {
        SLIST_ENTRY(ArenaEntry) link;
        IOSKArena *   fArena;
        IOSKOffset    fRegionOffset;
        IOSKIndex     fRegionIndex;
    };
    SLIST_HEAD(ArenaHead, ArenaEntry);

    IOReturn     _setSegmentBuffer( const IOSKIndex index, IOSKMemoryBuffer * buf );
    void         _clearSegmentBuffer( const IOSKIndex index, IOSKMemoryBufferRef * prevBuffer );
    ArenaEntry * findArenaEntry( const IOSKArena * arena );

    IOSKRegionSpec fSpec;
    IOLock       * fRegionLock;
    ArenaHead    * fArenaHead;
    Segment      * fSegments;
    IOSKCount      fSegmentCount;
    IOSKSize       fSegmentSize;
};

#endif
