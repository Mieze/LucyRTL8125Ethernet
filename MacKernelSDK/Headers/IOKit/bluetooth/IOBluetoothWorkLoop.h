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
 *   @header IOBluetoothWorkLoop.h
 *   This header contains the definitions of the IOBluetoothLocalUtilityEventSource and IOBluetoothWorkLoop classes.
 */

#ifndef _IOKIT_IOBLUETOOTHWORKLOOP_H
#define _IOKIT_IOBLUETOOTHWORKLOOP_H

#include <IOKit/IOEventSource.h>
#include <IOKit/IOWorkLoop.h>

/*!
 * @class IOBluetoothLocalUtilityEventSource : public IOEventSource
 * @abstract An event source providing utilities specific to the IOBluetooth family.
 * @discussion The IOBluetoothLocalUtilityEventSource expands the functionality of its super class, IOEventSource, by implementing the functions closeGateOn() and openGateOn(), as well as a factory
 * member function for organized construction of itself. This event source should be used and only used in the IOBluetooth family.
 */

class IOBluetoothLocalUtilityEventSource : public IOEventSource
{
    OSDeclareDefaultStructors(IOBluetoothLocalUtilityEventSource)

public:
    /*! @function ioBluetoothLocalUtilityEventSource
     *   @abstract Factory member function to constuct and intialize an IOBluetoothLocalUtilityEventSource.
     *   @result Returns a IOBluetoothLocalUtilityEventSource instance if constructed successfully, 0 otherwise.
     */

    static IOBluetoothLocalUtilityEventSource * ioBluetoothLocalUtilityEventSource();

    /*! @function checkForWork
     *    @abstract Virtual member function used by IOWorkLoop for work scheduling.
     *    @discussion This function overrides IOEventSource::checkForWork() so that it does nothing.
     *    @result Returns false.
     */

    virtual bool checkForWork() APPLE_KEXT_OVERRIDE;

    /*! @function closeGateOn
     *   @abstract Closes gate and sets the work loop.
     *   @discussion This function calls IOEventSource::setWorkLoop() and then IOEventSource::closeGate().
     *   @param workLoop The parameter used in IOEventSource::setWorkLoop(). */

    virtual void closeGateOn(IOWorkLoop * workLoop);

    /*! @function openGateOn
     *   @abstract Opens gate and sets the work loop.
     *   @discussion This function calls IOEventSource::setWorkLoop() and then IOEventSource::openGate().
     *   @param workLoop The parameter used in IOEventSource::setWorkLoop(). */

    virtual void openGateOn(IOWorkLoop * workLoop);
};

/*!
 * @class IOBluetoothWorkLoop : public IOWorkLoop
 * @abstract A work loop instance providing functions used exclusively by the IOBluetooth family.
 * @discussion The IOBluetoothWorkLoop provides the functions handoffFrom() and returnTo(), which could help with linking between different work loops, a feature that could be used by the
 * IOBluetoothFamily. This class also contains a factory member function for organized construction of itself. This work loop should be used and only used in the IOBluetooth family.
 */

class IOBluetoothWorkLoop : public IOWorkLoop
{
    OSDeclareDefaultStructors(IOBluetoothWorkLoop)

public:
    /*! @function workLoop
     *   @abstract Factory member function to construct and intialize an IOBluetoothWorkLoop.
     *   @result Returns a IOBluetoothWorkLoop instance if constructed successfully, 0 otherwise.
     */

    static IOBluetoothWorkLoop * workLoop();

    /*! @function handoffFrom
     *   @abstract Handoffs from a work loop instance.
     *   @discussion This function will first check if workLoop is inGate(). Then, it calls openGateOn(workLoop) using an IOBluetoothLocalUtilityEventSource instance and closeGate() on this.
     *   @param workLoop A work loop instance.
     *   @result An IOReturn indicating whether the operation is successful.
     */

    virtual IOReturn handoffFrom(IOWorkLoop * workLoop);

    /*! @function returnTo
     *   @abstract Returns to a work loop instance.
     *   @discussion This function will first check if workLoop is inGate(). Then, it calls openGate() on this and closeGateOn(workLoop) using an IOBluetoothLocalUtilityEventSource instance.
     *   @param workLoop A work loop instance.
     *   @result An IOReturn indicating whether the operation is successful.
     */

    virtual IOReturn returnTo(IOWorkLoop * workLoop);
};

#endif
