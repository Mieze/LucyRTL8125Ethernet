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

#ifndef _IOKIT_IOBLUETOOTHDATAQUEUE_H
#define _IOKIT_IOBLUETOOTHDATAQUEUE_H

#include <IOKit/IOMemoryDescriptor.h>
#include <IOKit/IOSharedDataQueue.h>

#ifdef enqueue
#undef enqueue
#endif

/*!
 * @class IOBluetoothDataQueue : public IOSharedDataQueue
 * @abstract A generic data queue providing utilities specific to the IOBluetoothFamily.
 * @discussion The IOBluetoothDataQueue is a subclass of IOSharedDataQueue that provides some functions that would be used by other IOBluetoothFamily classes. It should be used and only used in this
 * family.
 */

class IOBluetoothDataQueue : public IOSharedDataQueue
{
    OSDeclareDefaultStructors(IOBluetoothDataQueue)

public:
    static IOBluetoothDataQueue * withCapacity(UInt32 size);
    static IOBluetoothDataQueue * withEntries(UInt32 numEntries, UInt32 entrySize);
    static IOBluetoothDataQueue * withBuffer(void * buffer, UInt32 size);
    static IOBluetoothDataQueue * withClientBuffer(mach_vm_address_t address, UInt32 length, task_t task);

    virtual void   free() APPLE_KEXT_OVERRIDE;
    virtual void   sendDataAvailableNotification() APPLE_KEXT_OVERRIDE;
    virtual void   setNotificationPort(mach_port_t port) APPLE_KEXT_OVERRIDE;
    virtual bool   setQueueSize(UInt32 size, bool isWithBuffer);
    virtual bool   initWithCapacity(UInt32 size) APPLE_KEXT_OVERRIDE;
    virtual bool   initWithBuffer(void * buffer, UInt32 size);
    virtual bool   initWithClientBuffer(mach_vm_address_t address, UInt32 length, task_t task);
    virtual bool   enqueue(void * data, UInt32 dataSize) APPLE_KEXT_OVERRIDE;
    virtual UInt32 numberOfFreeBlocks(UInt64 offset);
    virtual UInt32 freeSpaceInQueue();
    virtual void   setDataQueueOwnerName(char * name);

protected:
    bool                 mInitializedWithBuffer;       // 40, determines if the data queue should be freed
    char                 mDataQueueOwnerName[0x30];    // 41
    IOMemoryMap *        mMemoryMap;                   // 96
    IOMemoryDescriptor * mMemoryDescriptor;            // 104
    bool                 mInitializedWithClientBuffer; // 112, will setTag(3) in free if it is true, set to true in initWithClientBuffer
    UInt32               mNotifyMsgSize;               // 116
};

#endif
