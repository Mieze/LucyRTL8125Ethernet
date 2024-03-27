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

#ifndef _IOKIT_IOBLUETOOTHTIMEREVENTSOURCE_H
#define _IOKIT_IOBLUETOOTHTIMEREVENTSOURCE_H

#include <IOKit/IOTimerEventSource.h>

struct IOBluetoothTimer
{
    AbsoluteTime       abstime;        // 0
    UInt32             index;          // 8
    void *             refcon;         // 16
    bool               enabled;        // 24
    IOBluetoothTimer * timerChainNext; // 32
};

class IOBluetoothTimerEventSource : public IOTimerEventSource
{
    OSDeclareDefaultStructors(IOBluetoothTimerEventSource)

public:
    static IOBluetoothTimerEventSource * timerEventSource(OSObject * owner, Action action = NULL);

    static void  timerAction(OSObject * owner, IOBluetoothTimerEventSource * sender);
    virtual void timerFired(IOBluetoothTimerEventSource * sender);
    virtual bool init(OSObject * owner, Action action = NULL) APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual bool addTimer(AbsoluteTime abstime, UInt32 index, void * refcon);
    virtual bool removeTimer(UInt32 index);
    virtual bool removeAllTimers();

    virtual IOReturn setTimeoutMS(UInt32, UInt32, void *);
    virtual IOReturn setTimeoutUS(UInt32, UInt32, void *);
    virtual IOReturn setTimeout(UInt32, UInt32, UInt32, void *);
    virtual IOReturn setTimeout(AbsoluteTime abstime, UInt32, void *);
    virtual void     cancelTimeout(UInt32);
    virtual void     cancelTimeout() APPLE_KEXT_OVERRIDE;

protected:
    IOBluetoothTimer * mCurrentTimer;   // 96
    OSObject *         mOwner;          // 104
    Action             mTimeoutHandler; // 112
    IOBluetoothTimer * mFiringTimer;    // 120
};

#endif
