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

#ifndef _IOKIT_IOWORKQUEUE_H
#define _IOKIT_IOWORKQUEUE_H

#include <IOKit/IOCommandGate.h>
#include <IOKit/IOLib.h>

/*! @class IOWorkQueue : public IOEventSource
 *   @abstract A circular queue that contains IOWorkQueueCall instances as its entries.
 *   @discussion ???
 */

class IOWorkQueue : public IOEventSource
{
    OSDeclareDefaultStructors(IOWorkQueue)

    typedef IOReturn (*Action)(OSObject * owner, void * arg0, void * arg1, void * arg2, void * arg3, void * arg4, void * arg5);

    // TO-DO: Make this clear
    typedef int IOWorkQueueOperationMode;
    enum IOWorkQueueOperationModes
    {
        kIOWorkQueueOperationModeWorkAvailable    = 0,
        kIOWorkQueueOperationModeWorkNotAvailable = 1,
        kIOWorkQueueOperationModeCount
    };

    struct IOWorkQueueCall
    {
        Action action; // 0
        void * arg0;   // 8
        void * arg1;   // 16
        void * arg2;   // 24
        void * arg3;   // 32
        void * arg4;   // 40
        void * arg5;   // 48

        IOWorkQueueCall * nextCall;     // 56
        IOWorkQueueCall * previousCall; // 64
    };

public:
    virtual void unusedCall002(IOWorkQueueCall **, IOWorkQueueCall **, IOWorkQueueCall *);
    virtual void unusedCall003(IOWorkQueueCall **, IOWorkQueueCall **);

    virtual void              enqueueWorkCall(IOWorkQueueCall *);
    virtual IOWorkQueueCall * dequeueWorkCall();

    virtual void unusedCall000(IOWorkQueueCall *);
    virtual void unusedCall001();

    virtual bool     checkForWork() APPLE_KEXT_OVERRIDE;
    virtual IOReturn executeWorkCall(IOWorkQueueCall *);

    virtual void  processWorkCallFromSeparateThread(IOWorkQueueCall *);
    virtual void  processWorkCallFromSeparateThreadWL();
    static void * ThreadCallMain(void *, wait_result_t);

    virtual void unusedCall004(IOWorkQueueCall *);

    static IOWorkQueue * withCapacity(OSObject * owner, UInt32 size, IOWorkQueueOperationMode mode, bool isDebug, char * workQueueName);
    virtual bool         initWithCapacity(OSObject * owner, UInt32 size, IOWorkQueueOperationMode mode, bool isDebug, char * workQueueName);

    virtual void free() APPLE_KEXT_OVERRIDE;

    virtual IOReturn enqueueAction(Action action, void * arg0, void * arg1, void * arg2, void * arg3, void * arg4, void * arg5);

    virtual void setCountedActionLimit(UInt32 limit);
    virtual bool allDoneProcessing();
    virtual void setEnqueueEnable(bool enabled);
    virtual void removeAllEntries();
    virtual void disable() APPLE_KEXT_OVERRIDE;
    virtual void disableWL();

    OSMetaClassDeclareReservedUnused(IOWorkQueue, 0);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 1);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 2);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 3);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 4);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 5);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 6);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 7);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 8);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 9);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 10);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 11);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 12);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 13);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 14);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 15);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 16);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 17);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 18);
    OSMetaClassDeclareReservedUnused(IOWorkQueue, 19);

protected:
    IOWorkQueueCall * mWorkQueueHeadCall;       // 72
    IOWorkQueueCall * mWorkQueueTailCall;       // 80
    IOSimpleLock *    mWorkQueueCallSimpleLock; // 88

    struct ExpansionData
    {
        IOWorkLoop *    eWorkLoop;        // 0
        IOCommandGate * eCommandGate;     // 8
        bool            eStop;            // 16
        IOThread        eThread[20];      // 24
        uint8_t         eThreadLimit[20]; // 184
    };
    ExpansionData * mExpansionData; // 96

#define eWorkLoop    IOWorkQueue::mExpansionData->eWorkLoop
#define eCommandGate IOWorkQueue::mExpansionData->eCommandGate
#define eStop        IOWorkQueue::mExpansionData->eStop
#define eThread      IOWorkQueue::mExpansionData->eThread
#define eThreadLimit IOWorkQueue::mExpansionData->eThreadLimit

    void * mUnusedPointer; // 104

    IOWorkQueueOperationMode mOperationMode;       // 112
    SInt32                   mThreadCounter;       // 116
    UInt32                   mCountedActionLimit;  // 120
    bool                     mDebug;               // 124
    bool                     mEnqueueEnabled;      // 125
    char                     mWorkQueueName[0x64]; // 126
};

#endif
