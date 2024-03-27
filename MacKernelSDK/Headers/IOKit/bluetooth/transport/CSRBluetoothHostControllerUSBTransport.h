/*
 *  Released under "The GNU General Public License (GPL-2.0)"
 *
 *  Copyright (c) 2021 cjiang. All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 *  for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#ifndef _IOKIT_CSRBLUETOOTHHOSTCONTROLLERUSBTRANSPORT_H
#define _IOKIT_CSRBLUETOOTHHOSTCONTROLLERUSBTRANSPORT_H

#include <IOKit/bluetooth/transport/IOBluetoothHostControllerUSBTransport.h>

class CSRBluetoothHostControllerUSBTransport : public IOBluetoothHostControllerUSBTransport
{
    OSDeclareDefaultStructors(CSRBluetoothHostControllerUSBTransport)

public:
    virtual bool init(OSDictionary * dictionary = NULL) APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual IOService * probe(IOService * provider, SInt32 * score) APPLE_KEXT_OVERRIDE;
    virtual bool start(IOService * provider) APPLE_KEXT_OVERRIDE;
    virtual bool InitializeTransportWL(IOService * provider);
    virtual void systemWillShutdownWL(IOOptionBits options, void * parameter);
    
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 0);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 1);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 2);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 3);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 4);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 5);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 6);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 7);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 8);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 9);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 10);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 11);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 12);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 13);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 14);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 15);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 16);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 17);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 18);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 19);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 20);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 21);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 22);
    OSMetaClassDeclareReservedUnused(CSRBluetoothHostControllerUSBTransport, 23);

protected:
    struct ExpansionData
    {
        void * mRefCon;
    };
    ExpansionData * mExpansionData;
}

#endif
