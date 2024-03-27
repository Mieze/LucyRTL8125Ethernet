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
 *   @header IOBluetoothObject.h
 *   This header contains the definition of the IOBluetoothObject class, a base for many classes, mostly channels, in the IOBluetoothFamily.
 */

#ifndef _IOKIT_IOBLUETOOTHOBJECT_H
#define _IOKIT_IOBLUETOOTHOBJECT_H

#include <IOKit/IOCommandGate.h>
#include <IOKit/IOService.h>
#include <IOKit/IOWorkLoop.h>

class IOBluetoothHCIController;

/*!
 * @class IOBluetoothObject : public IOService
 * @abstract An abstract class representing an object in the IOBluetoothFamily.
 * @discussion The IOBluetoothObject is the base for a lot of IOBluetooth classes due to its abstractness. Each IOBluetoothObject has a specific object ID, which could be obtained from the family. One
 * of its most significant features is to provide handle pending IOs.
 */

class IOBluetoothObject : public IOService
{
	OSDeclareDefaultStructors(IOBluetoothObject)

public:
    virtual bool start(IOService * provider) APPLE_KEXT_OVERRIDE;

    virtual void free() APPLE_KEXT_OVERRIDE;

    virtual bool terminate(IOOptionBits options = 0) APPLE_KEXT_OVERRIDE;

    /*! @function willTerminate
     *   @abstract Passes a termination up the stack.
     *   @discussion Retains and releases the class per se, runs staticPerformTerminate from the command gate, and calls IOService::willTerminate().
     *   @param provider The terminated provider of this object.
     *   @param options Options originally passed to terminate.
     *   @result An boolean indicating whether the operation was successful. */

    virtual bool willTerminate(IOService * provider, IOOptionBits options) APPLE_KEXT_OVERRIDE;

    /*! @function performTerminate
     *   @abstract Use the command gate to sleep all pending IOs.
     *   @result For now, this function always returns kIOReturnSuccess. */

    virtual IOReturn performTerminate();

    /*! @function staticPerformTerminate
     *   @abstract Static version of performTerminate, implemented by converting owner to an IOBluetoothObject and calling performTerminate from there.
     *   @param owner The function's provider that will be converted to an IOBluetoothObject.
     *   @param arg0 Argument to action from run operation.
     *   @param arg1 Argument to action from run operation.
     *   @param arg2 Argument to action from run operation.
     *   @param arg3 Argument to action from run operation.
     *   @result An IOReturn indicating whether the operation was successful. */

    static IOReturn staticPerformTerminate(OSObject * owner, void * arg0, void * arg1, void * arg2, void * arg3);

    /*! @function getWorkLoop
     *   @abstract Returns the current work loop.
     *   @result The current work loop, specifically mWorkLoop. */

    virtual IOWorkLoop * getWorkLoop() const APPLE_KEXT_OVERRIDE;

    /*! @function getCommandGate
     *   @abstract Returns the current command gate.
     *   @result The current command gate, specifically mCommandGate. */

    virtual IOCommandGate * getCommandGate() const;

    /*! @function incrementNumberOfPendingIO
     *   @abstract Increments the number of pending IOs by 1.
     *   @discussion This function would increment mNumPendingIO by 1 only if the workLoop is inGate() and the integer is within the range of an UInt32.
     *   @result An IOReturn indicating whether the operation was successful. */

    virtual IOReturn incrementNumberOfPendingIO();

    /*! @function decrementNumberOfPendingIO
     *   @abstract Decrements the number of pending IOs by 1.
     *   @discussion This function would decrement mNumPendingIO by 1 only if the workLoop is inGate() and the integer is within the range of an UInt32. If there are no pending IOs left after the
     * decrement, it would call commandWakeup() to wakeup the threads.
     *   @result An IOReturn indicating whether the operation was successful. */

    virtual IOReturn decrementNumberOfPendingIO();

    /*! @function pendingIO
     *   @abstract Returns the number of attached pending IOs.
     *   @result The current number of pending IOs, specifically mNumPendingIO. */

    virtual UInt32 pendingIO() const;

protected:
    /*! @var mWorkLoop
     *   The work loop of this class. For now, mCommandGate is the only event source that will be added.
     *   Offset: 136. */
    IOWorkLoop * mWorkLoop;

    /*! @var mWorkLoop
     *   The command gate of this class.
     *   Offset: 144. */
    IOCommandGate * mCommandGate;

    /*! @var mNumPendingIO
     *   This integer records to number of attached pending IOs. It could be obtained through pendingIO().
     *   Offset: 152. */
    UInt32 mNumPendingIO;

    /*! @var mTerminateMask
     *   This unsigned char is used to mark if the class has been terminated. The second bit is set to true when termiante() is called.
     *   Offset: 156. */
    UInt8 mTerminateMask;

    /*! @var mBluetoothObjectID
     *   This integer records the BluetoothObjectID of this class, derived from mBluetoothFamily.
     *   Offset: 160. */
    UInt32 mBluetoothObjectID;

    /*! @var mBluetoothFamily
     *   A  IOBluetoothHCIController that helps the class with several bluetooth-specific operations.
     *   Offset: 168. */
    IOBluetoothHCIController * mBluetoothFamily;
};

#endif
