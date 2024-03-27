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

#ifndef _IOKIT_IOBLUETOOTHHOSTCONTROLLERUARTTRANSPORT_H
#define _IOKIT_IOBLUETOOTHHOSTCONTROLLERUARTTRANSPORT_H

#include <IOKit/IOInterruptEventSource.h>
#include <IOKit/bluetooth/serial/IOBluetoothSerialManager.h>
#include <IOKit/bluetooth/transport/IOBluetoothHostControllerTransport.h>

#ifndef __MAC_OS_X_VERSION_MIN_REQUIRED
#error "Missing macOS target version"
#endif

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
extern void TimeOutHandler(OSObject * owner, IOTimerEventSource * sender);
#endif

enum BluetoothUARTPacketTypes
{
    kBluetoothUARTPacketTypeHCIRequest = 1,
    kBluetoothUARTPacketTypeBulk       = 2,
    kBluetoothUARTPacketTypeIsoch      = 3,
    kBluetoothUARTPacketTypeEvent      = 4,
    kBluetoothUARTPacketTypeLMP        = 7
};

class IOBluetoothHostControllerUARTTransport : public IOBluetoothHostControllerTransport
{
    OSDeclareAbstractStructors(IOBluetoothHostControllerUARTTransport)

public:
    virtual bool        init(OSDictionary * dictionary = NULL) APPLE_KEXT_OVERRIDE;
    virtual void        free() APPLE_KEXT_OVERRIDE;
    virtual IOService * probe(IOService * provider, SInt32 * score) APPLE_KEXT_OVERRIDE;
    virtual bool        start(IOService * provider) APPLE_KEXT_OVERRIDE;
    virtual void        stop(IOService * provider) APPLE_KEXT_OVERRIDE;
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    virtual bool        terminateWL(IOOptionBits options = 0) APPLE_KEXT_OVERRIDE;
#else
    virtual bool        terminate(IOOptionBits options = 0) APPLE_KEXT_OVERRIDE;
#endif
    virtual bool     ConfigurePM(IOService * policyMaker) APPLE_KEXT_OVERRIDE;
    virtual IOReturn setPowerStateWL(unsigned long powerStateOrdinal, IOService * whatDevice) APPLE_KEXT_OVERRIDE;
    virtual void     CompletePowerStateChange(char *) APPLE_KEXT_OVERRIDE;
    virtual IOReturn powerStateWillChangeToWL(IOOptionBits options, void *) APPLE_KEXT_OVERRIDE;
    virtual IOReturn RequestTransportPowerStateChange(IOBluetoothHCIControllerInternalPowerState powerState, char *) APPLE_KEXT_OVERRIDE;
    virtual void     systemWillShutdownWL(IOOptionBits options, void *) APPLE_KEXT_OVERRIDE;

    virtual UInt16 GetControllerVendorID() APPLE_KEXT_OVERRIDE;
    virtual UInt16 GetControllerProductID() APPLE_KEXT_OVERRIDE;

    virtual IOReturn SendHCIRequest(UInt8 * data, IOByteCount size) APPLE_KEXT_OVERRIDE;
    virtual IOReturn TransportBulkOutWrite(void * data) APPLE_KEXT_OVERRIDE;
    virtual IOReturn TransportIsochOutWrite(void * data, void * retainer, IOOptionBits options) APPLE_KEXT_OVERRIDE;
    virtual IOReturn TransportSendSCOData(void *) APPLE_KEXT_OVERRIDE;

    virtual IOReturn SendUART(UInt8 * data, UInt32 size);
    static IOReturn  StaticProcessACLSCOEventData(void * data, int size);
    virtual IOReturn ProcessACLSCOEventData();
    virtual void     GetInfo(void * outInfo) APPLE_KEXT_OVERRIDE;
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    virtual IOReturn AcquirePort(bool);
#else
    virtual IOReturn AcquirePort();
#endif
    virtual IOReturn ReleasePort();
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    virtual IOReturn WaitForReceiveToBeReady(bool);
#else
    virtual IOReturn WaitForReceiveToBeReady();
#endif
    virtual void     NewSCOConnection() APPLE_KEXT_OVERRIDE;

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    virtual IOReturn SetLMPLogging();
    virtual bool     StartLMPLogging() APPLE_KEXT_OVERRIDE;
    virtual bool     StopLMPLogging();
#endif
    virtual IOReturn ToggleLMPLogging(UInt8 *) APPLE_KEXT_OVERRIDE;

    virtual IOReturn DoDeviceReset(UInt16) APPLE_KEXT_OVERRIDE;
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    virtual IOReturn DequeueDataInterruptEventGated(IOInterruptEventSource * sender, int count);
#endif

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_11_0
    virtual void DumpTransportProviderState() APPLE_KEXT_OVERRIDE;
#endif

protected:
    UInt8 *                        mDataToSend;                         // 328, the UART packet sent through SendUART()
    UInt8 *                        mDataReceived;                       // 336, obtained from mProvider->DequeueData()
    IORS232SerialStreamSync *      mProvider;                           // 344
    void *                         mIsochOutWriteRetainer;              // 352, second param of TransportIsochOutWrite
    bool                           mReadyToReceive;                     // 360
    UInt32                         mIsochOutWriteCounter;               // 364
    bool                           mPowerStateChangeInProgress;         // 368
    bool                           mSkipBluetoothFirmwareBoot;          // 369
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    IOWorkLoop *                   mUARTTransportWorkLoop;              // 376
    IOInterruptEventSource::Action mDequeueDataInterruptEventAction;    // 384
    IOInterruptEventSource *       mDequeueDataInterruptEvent;          // 392
    IOWorkLoop *                   mUARTTransportTimerWorkLoop;         // 400
    IOCommandGate *                mUARTTransportTimerCommandGate;      // 408
    IOTimerEventSource *           mUARTTransportTimer;                 // 416
    bool                           mUARTTransportTimerHasTimeout;       // 424
    UInt32                         mSlowEnqueueData;                    // 428
    UInt32                         mLongestEnqueueDataCallMicroseconds; // 432
#endif
};

#endif
