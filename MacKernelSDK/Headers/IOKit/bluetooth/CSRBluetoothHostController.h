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

#ifndef _IOKIT_CSRBLUETOOTHHOSTCONTROLLER_H
#define _IOKIT_CSRBLUETOOTHHOSTCONTROLLER_H

#include <IOKit/bluetooth/IOBluetoothHostController.h>

class CSRBluetoothHostController : public IOBluetoothHostController
{
    OSDeclareDefaultStructors(CSRBluetoothHostController)

public:
    virtual bool init(IOBluetoothHCIController * family, IOBluetoothHostControllerTransport * transport) APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual bool InitializeController();
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    virtual IOReturn SetupController(bool * hardReset) APPLE_KEXT_OVERRIDE;
#else
    virtual IOReturn SetupController() APPLE_KEXT_OVERRIDE;
#endif
    
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    virtual IOReturn GetOpCodeAndEventCode(UInt8 * inDataPtr, UInt32 inDataSize, BluetoothHCICommandOpCode * outOpCode, BluetoothHCIEventCode * eventCode, BluetoothHCIEventStatus * outStatus, UInt8 *, BluetoothDeviceAddress * outDeviceAddress, BluetoothConnectionHandle * outConnectionHandle, bool *) APPLE_KEXT_OVERRIDE;
#else
    virtual IOReturn GetOpCodeAndEventCode(UInt8 * inDataPtr, BluetoothHCICommandOpCode * outOpCode, BluetoothHCIEventCode * eventCode, BluetoothHCIEventStatus * outStatus, UInt8 *, BluetoothDeviceAddress * outDeviceAddress, BluetoothConnectionHandle * outConnectionHandle) APPLE_KEXT_OVERRIDE;
#endif
    virtual void ProcessEventDataWL(UInt8 * inDataPtr, UInt32 inDataSize, UInt32 sequenceNumber) APPLE_KEXT_OVERRIDE;
    
    virtual IOReturn TransportRadioPowerOff(BluetoothHCICommandOpCode opCode, char * processName, int processID, IOBluetoothHCIRequest * request) APPLE_KEXT_OVERRIDE;
    
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 0);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 1);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 2);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 3);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 4);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 5);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 6);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 7);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 8);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 9);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 10);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 11);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 12);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 13);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 14);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 15);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 16);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 17);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 18);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 19);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 20);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 21);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 22);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostController, 23);
};

#endif
