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

#ifndef _IOSKYWALKNETWORKINTERFACE_H
#define _IOSKYWALKNETWORKINTERFACE_H

#include <IOKit/skywalk/IOSkywalkInterface.h>
#include <IOKit/skywalk/IOSkywalkTypes.h>
#include <IOKit/80211/apple80211_ioctl.h>

class IOSkywalkNetworkKDPPoller;

class IOSkywalkNetworkInterface : public IOSkywalkInterface
{
    OSDeclareAbstractStructors( IOSkywalkNetworkInterface )
    
    struct RegistrationInfo;
    
public:
    virtual void free() APPLE_KEXT_OVERRIDE;
    
    virtual bool start( IOService * provider ) APPLE_KEXT_OVERRIDE;
    virtual void stop( IOService * provider ) APPLE_KEXT_OVERRIDE;
    
    IOReturn registerNetworkInterface( const RegistrationInfo * info, IOSkywalkPacketQueue ** queues, IOOptionBits queueOptions, IOSkywalkPacketBufferPool * pool1, IOSkywalkPacketBufferPool * pool2, IOOptionBits options = 0 );
    IOReturn deregisterNetworkInterface( IOOptionBits options = 0 );
    
    void handleLinkStatusEvent();
    IOReturn reportEventType( UInt32 type, void * argument, vm_size_t argSize = 0 );
    IOReturn reportLinkStatus( IOSkywalkNetworkLinkStatus linkStatus, IOSkywalkNetworkMediaType activeMediaType );
    IOReturn reportLinkQuality( IOSkywalkNetworkLinkQuality linkQuality );
    IOReturn reportDataBandwidths( uint64_t maxInputBandwidth, uint64_t maxOutputBandwidth, uint64_t effectiveInputBandwidth, uint64_t effectiveOutputBandwidth );
    IOReturn reportInterfaceAdvisory( const struct ifnet_interface_advisory * advisory );
    
    virtual IOReturn setAggressiveness( unsigned long type, unsigned long newLevel ) APPLE_KEXT_OVERRIDE;
    
    virtual void joinPMtree( IOService * driver ) APPLE_KEXT_OVERRIDE;
    
    virtual IOReturn initBSDInterfaceParameters( struct ifnet_init_eparams * params, struct sockaddr_dl ** ll ) = 0;
    virtual IOReturn prepareBSDInterface( ifnet_t interface, IOOptionBits options );
    virtual void finalizeBSDInterface( ifnet_t interface, IOOptionBits options );
    virtual ifnet_t getBSDInterface();
    virtual void setBSDName( const char * name );
    virtual const char * getBSDName();
    
    virtual errno_t processBSDCommand( ifnet_t interface, UInt32 cmd, void * data );
    errno_t ioctl_gifmedia( ifnet_t interface, UInt32 sockioc, void * data );
    errno_t ioctl_sifmedia( ifnet_t interface, void * data );
    errno_t ioctl_gifdevmtu( ifnet_t interface, void * data );
    errno_t ioctl_sifmtu( ifnet_t interface, void * data, bool useOriginalLength );
    errno_t ioctl_gifcap( ifnet_t interface, void * data );
    errno_t ioctl_sifcap( ifnet_t interface, void * data );
    
    virtual void setRunningState( bool state );
    
    virtual IOReturn handleChosenMedia( UInt32 );
    virtual IOReturn getSupportedMediaArray( UInt32 *, UInt32 * );
    
    virtual IOReturn getPacketTapInfo( UInt32 *, UInt32 * );
    virtual IOReturn getUnsentDataByteCount( UInt32 *, UInt32 *, UInt32 );
    virtual IOReturn getSupportedWakeFlags( UInt32 * flags );
    virtual IOReturn enableNetworkWake( UInt32 flags );
    
    virtual IOReturn calculateRingSizeForQueue( const IOSkywalkPacketQueue * queue, UInt32 * size );
    IOReturn getServiceClassIndex( kern_packet_svc_class_t serviceClass, UInt32 * index );
    virtual UInt32 getMaxTransferUnit();
    virtual UInt32 getMinPacketSize();
    virtual IOReturn getMTU( UInt32 * mtu );
    virtual void setMTU( UInt32 mtu );
    
    virtual UInt32 getHardwareAssists();
    virtual void setHardwareAssists( UInt32 hardwareAssists );
    IOReturn setKDPPoller( IOSkywalkNetworkKDPPoller * poller );
    
    virtual void captureInterfaceState( UInt32 state );
    virtual void restoreInterfaceState( UInt32 state );
    virtual IOReturn handleDoorbellForQueue( IOSkywalkPacketQueue * queue, UInt32 );
    
    OSMetaClassDeclareReservedUnused( IOSkywalkNetworkInterface,  0 );
    OSMetaClassDeclareReservedUnused( IOSkywalkNetworkInterface,  1 );
    OSMetaClassDeclareReservedUnused( IOSkywalkNetworkInterface,  2 );
    OSMetaClassDeclareReservedUnused( IOSkywalkNetworkInterface,  3 );
    OSMetaClassDeclareReservedUnused( IOSkywalkNetworkInterface,  4 );
    OSMetaClassDeclareReservedUnused( IOSkywalkNetworkInterface,  5 );
    OSMetaClassDeclareReservedUnused( IOSkywalkNetworkInterface,  6 );
    OSMetaClassDeclareReservedUnused( IOSkywalkNetworkInterface,  7 );
    OSMetaClassDeclareReservedUnused( IOSkywalkNetworkInterface,  8 );
    OSMetaClassDeclareReservedUnused( IOSkywalkNetworkInterface,  9 );
    OSMetaClassDeclareReservedUnused( IOSkywalkNetworkInterface, 10 );
    
protected:
    void * mReserved;
    
    struct ExpansionData
    {
        RegistrationInfo          * eRegistrationInfo;
        ifnet_t                     eBSDInterface;
        OSArray                   * ePacketQueueArray;
        IOLock                    * eDataLock;
        IOLock                    * eInterfacePreparedLock;
        IOSkywalkNetworkKDPPoller * eKDPPoller;
        thread_call_t               eLinkStatusEventThread;
        UInt32                      eLinkStatusMessageType;
        IOSkywalkNetworkMediaType   eActiveMediaType;
        UInt32                      eChosenMedia;
        IOSkywalkNetworkLinkStatus  eLinkStatus;
        bool                        eInterfacePrepared;
        const char                  eBSDName[80];
        UInt32                      eMTU;
    };
    ExpansionData * mExpansionData;
};

#endif
