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

#ifndef _IOKIT_IOBLUETOOTHINACTIVITYTIMEREVENTSOURCE_H
#define _IOKIT_IOBLUETOOTHINACTIVITYTIMEREVENTSOURCE_H

#include <IOKit/IOTimerEventSource.h>

class IOBluetoothInactivityTimerEventSource : public IOTimerEventSource
{
    OSDeclareDefaultStructors(IOBluetoothInactivityTimerEventSource)

public:
    virtual void updateTimer();
    virtual void handleTimer(OSObject * owner);

    static IOBluetoothInactivityTimerEventSource * createWithTimeoutInterval(AbsoluteTime interval, OSObject * owner, Action action = NULL);
    static IOBluetoothInactivityTimerEventSource * createWithTimeoutIntervalMS(UInt32 ms, OSObject * owner, Action action = NULL);
    static IOBluetoothInactivityTimerEventSource * createWithTimeoutIntervalUS(UInt32 us, OSObject * owner, Action action = NULL);

    /*! @function init
     *   @abstract Initializes the timer with an owner, and a handler to call when the timeout expires.
     *   @discussion This functions starts by calling IOTimerEventSource::init() with IOBluetoothInactivityTimerEventSource::timerFired as its timeout handler. Afterwards, it initializes
     * mActivityCount to 0 and sets mTimeoutHandler to the parameter action.
     *   @param owner The owner of the timer.
     *   @param action A handler to call when the timeout expires.
     *   @result A true is returned on success and a false is returned on failure.
     */

    virtual bool init(OSObject * owner, Action action = NULL) APPLE_KEXT_OVERRIDE;
    virtual bool initWithTimeoutInterval(AbsoluteTime interval, OSObject * owner, Action action = NULL);
    virtual bool initWithTimeoutIntervalMS(UInt32 ms, OSObject * owner, Action action = NULL);
    virtual bool initWithTimeoutIntervalUS(UInt32 us, OSObject * owner, Action action = NULL);

    static void      timerFired(OSObject * owner, IOBluetoothInactivityTimerEventSource * sender);
    virtual IOReturn setTimeout(AbsoluteTime interval) APPLE_KEXT_OVERRIDE;
    virtual void     cancelTimeout() APPLE_KEXT_OVERRIDE;

    virtual IOReturn     setTimeoutInterval(AbsoluteTime interval);
    virtual IOReturn     setTimeoutIntervalMS(UInt32 ms);
    virtual IOReturn     setTimeoutIntervalUS(UInt32 us);
    virtual AbsoluteTime getTimeoutInterval();
    virtual UInt32       getTimeoutIntervalMS();
    virtual UInt32       getTimeoutIntervalUS();

    virtual void   incrementActivityCount();
    virtual void   decrementActivityCount();
    virtual UInt32 getActivityCount();

    virtual IOReturn resetTimer();

protected:
    AbsoluteTime mTimeoutInterval; // 96
    UInt32       mActivityCount;   // 104
    Action       mTimeoutHandler;  // 112
};

#endif
