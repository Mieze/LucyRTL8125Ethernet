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

#ifndef _APPLE_APPLECSRBLUETOOTHHOSTCONTROLLER_H
#define _APPLE_APPLECSRBLUETOOTHHOSTCONTROLLER_H

#include <IOKit/bluetooth/CSRBluetoothHostController.h>

class AppleCSRBluetoothHostController : public CSRBluetoothHostController
{
    OSDeclareDefaultStructors(AppleCSRBluetoothHostController)

public:
    virtual bool init(IOBluetoothHCIController * family, IOBluetoothHostControllerTransport * transport) APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;
    
    virtual IOReturn CleanUpBeforeTransportTerminate(IOBluetoothHostControllerTransport * transport) APPLE_KEXT_OVERRIDE;
    virtual IOReturn setPropertiesWL(OSObject * properties) APPLE_KEXT_OVERRIDE;
    virtual bool GetCompleteCodeForCommand(BluetoothHCICommandOpCode inOpCode, BluetoothHCIEventCode * outEventCode) APPLE_KEXT_OVERRIDE;
    virtual bool SetHCIRequestRequireEvents(UInt16, IOBluetoothHCIRequest * request) APPLE_KEXT_OVERRIDE;
    virtual bool NeedToIncreaseInactivityTime() APPLE_KEXT_OVERRIDE;
    
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 0);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 1);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 2);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 3);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 4);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 5);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 6);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 7);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 8);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 9);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 10);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 11);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 12);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 13);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 14);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 15);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 16);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 17);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 18);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 19);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 20);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 21);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 22);
    OSMetaClassDeclareReservedUnused(AppleCSRBluetoothHostController, 23);
    
protected:
    struct ExpansionData
    {
        void * mRefCon;
    };
    ExpansionData * mExpansionData;
};

#endif
