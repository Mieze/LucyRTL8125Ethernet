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
/*!
 *   @header IOBluetoothMemoryBlockQueue.h
 *   This header contains the definition of the IOBluetoothMemoryBlockQueue instance.
 */

#ifndef _IOKIT_IOBLUETOOTHMEMORYBLOCKQUEUE_H
#define _IOKIT_IOBLUETOOTHMEMORYBLOCKQUEUE_H

class IOBluetoothMemoryBlock;

struct IOBluetoothMemoryBlockQueueNode
{
    IOBluetoothMemoryBlock *          block;
    IOBluetoothMemoryBlockQueueNode * next;
    bool                              completionCalled;
};

class IOBluetoothMemoryBlockQueue : public OSObject
{
    OSDeclareDefaultStructors(IOBluetoothMemoryBlockQueue)

public:
    virtual bool init() APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;

    /*! @function queueIsEmpty
     *   @abstract Determines if the queue is empty.
     *   @discussion The queue would be counted as empty if mHeadNode is NULL and mNumEntriesInQueue is 0.
     *   @result If the queue is empty, the result would be true. Otherwise, it would be false. */

    virtual bool queueIsEmpty();

    /*! @function enqueueBlock
     *   @abstract Adds a new memory block to the tail of the queue.
     *   @discussion This function adopts the same approach of enqueuing an object as any other queue, but it is not done directly. The block is to be placed into an
     * IOBluetoothMemoryBlockQueueNode, which will instead be enqueued. If the function is successfully executed, mNumEntriesInQueue would increment.
     *   @param block The memory block to enqueue.
     *   @result An IOReturn indicating whether the operation was successful. */

    virtual IOReturn enqueueBlock(IOBluetoothMemoryBlock * block);

    /*! @function peekNextBlock
     *   @abstract Derives the memory block at the head of the queue.
     *   @result The memory block of mHeadNode. If it does not exist, NULL will be returned. */

    virtual IOBluetoothMemoryBlock * peekNextBlock();

    /*! @function enqueueBlock
     *   @abstract Removes the memory block at the head of the queue.
     *   @discussion If the queue is not empty, this function will remove mHeadNode by releasing it and moving the next entry to its address. If the function is successfully executed,
     * mNumEntriesInQueue would decrement.
     *   @result The IOBluetoothMemoryBlock that is in the entry dequeued. If nothing is dequeued, it would be NULL. */

    virtual IOBluetoothMemoryBlock * dequeueBlock();

    /*! @function flushQueue
     *   @abstract Removes all entries from the queue.
     *   @discussion This functions continuously calls dequeueBlock() until there are no more entries left.
     *   @param releaseBlock If this parameter is true, the IOBluetoothMemoryBlock instances in the entries dequeued will be released. Vice versa.*/

    virtual void     flushQueue(bool releaseBlock);
    virtual UInt32   numberOfEntriesInQueue();
    virtual IOReturn setCompletionCalled(IOBluetoothMemoryBlock * block);
    virtual bool     completionRoutineIsCalled();
    virtual IOReturn removeBlock(IOBluetoothMemoryBlock * block);
    virtual void     printQueue();

protected:
    IOBluetoothMemoryBlockQueueNode * mHeadNode;          // 16
    IOBluetoothMemoryBlockQueueNode * mTailNode;          // 24
    UInt32                            mNumEntriesInQueue; // 32
};

#endif
