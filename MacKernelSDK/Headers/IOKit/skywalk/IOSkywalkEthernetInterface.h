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

#ifndef _IOSKYWALKETHERNETINTERFACE_H
#define _IOSKYWALKETHERNETINTERFACE_H

#include <IOKit/skywalk/IOSkywalkNetworkInterface.h>
#include <net/if_dl.h>

class IOSkywalkEthernetInterface : public IOSkywalkNetworkInterface
{
    OSDeclareAbstractStructors( IOSkywalkEthernetInterface )

    struct RegistrationInfo;
    
public:
    virtual bool start( IOService * provider ) APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;
    
    bool initRegistrationInfo( RegistrationInfo * info, IOOptionBits options, size_t size );
    IOReturn registerEthernetInterface( const RegistrationInfo * info, IOSkywalkPacketQueue ** queues, IOOptionBits queueOptions, IOSkywalkPacketBufferPool * pool1, IOSkywalkPacketBufferPool * pool2, IOOptionBits options = 0 );
    IOReturn deregisterEthernetInterface( IOOptionBits options = 0 );
    
    virtual IOReturn initBSDInterfaceParameters( struct ifnet_init_eparams * params, sockaddr_dl ** ll ) APPLE_KEXT_OVERRIDE;
    virtual IOReturn prepareBSDInterface( ifnet_t interface, IOOptionBits options ) APPLE_KEXT_OVERRIDE;
    virtual errno_t processBSDCommand( ifnet_t interface, UInt32 cmd, void * data ) APPLE_KEXT_OVERRIDE;
    
    errno_t ioctl_sifflags( ifnet_t interface );
    errno_t ioctl_multicast( ifnet_t interface, bool );
    
    virtual IOReturn getPacketTapInfo(UInt32 *, UInt32 *) APPLE_KEXT_OVERRIDE;
    virtual UInt32 getMaxTransferUnit() APPLE_KEXT_OVERRIDE;
    virtual UInt32 getMinPacketSize() APPLE_KEXT_OVERRIDE;
    virtual UInt32 getHardwareAssists() APPLE_KEXT_OVERRIDE;
    
    virtual IOReturn configureMulticastFilter( UInt32, const ether_addr * addresses, uint32_t count );
    virtual IOReturn setPromiscuousModeEnable( bool enable, IOOptionBits options );
    
    OSMetaClassDeclareReservedUnused( IOSkywalkEthernetInterface,  0 );
    OSMetaClassDeclareReservedUnused( IOSkywalkEthernetInterface,  1 );
    OSMetaClassDeclareReservedUnused( IOSkywalkEthernetInterface,  2 );
    OSMetaClassDeclareReservedUnused( IOSkywalkEthernetInterface,  3 );
    OSMetaClassDeclareReservedUnused( IOSkywalkEthernetInterface,  4 );
    OSMetaClassDeclareReservedUnused( IOSkywalkEthernetInterface,  5 );
    OSMetaClassDeclareReservedUnused( IOSkywalkEthernetInterface,  6 );
    OSMetaClassDeclareReservedUnused( IOSkywalkEthernetInterface,  7 );
    OSMetaClassDeclareReservedUnused( IOSkywalkEthernetInterface,  8 );
    OSMetaClassDeclareReservedUnused( IOSkywalkEthernetInterface,  9 );
    OSMetaClassDeclareReservedUnused( IOSkywalkEthernetInterface, 10 );
    
protected:
    void * mReserved;
    
    struct ExpansionData
    {
        RegistrationInfo * eRegistrationInfo;
        OSData           * eMulticastAddresses;
        UInt32             eNumMulticastAddresses;
        UInt16             eBSDInterfaceFlags;
        sockaddr_dl        eLinkLayerSockAddress;
    };
    ExpansionData * mExpansionData;
};

#endif
