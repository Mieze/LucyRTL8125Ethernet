/* LucyRTL8125Ethernet.cpp -- RTL8125 driver class implementation.
*
* Copyright (c) 2020 Laura Müller <laura-mueller@uni-duesseldorf.de>
* All rights reserved.
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the Free
* Software Foundation; either version 2 of the License, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
* more details.
*
* Driver for Realtek RTL8125 PCIe 2.5GB ethernet controllers.
*
* This driver is based on Realtek's r8125 Linux driver (9.003.04).
*/

#include "LucyRTL8125Ethernet.hpp"

#pragma mark --- function prototypes ---

static inline UInt32 adjustIPv6Header(mbuf_t m);

static inline u32 ether_crc(int length, unsigned char *data);

#pragma mark --- public methods ---

OSDefineMetaClassAndStructors(LucyRTL8125, super)

/* IOService (or its superclass) methods. */

bool LucyRTL8125::init(OSDictionary *properties)
{
    bool result;
    
    result = super::init(properties);
    
    if (result) {
        workLoop = NULL;
        commandGate = NULL;
        pciDevice = NULL;
        mediumDict = NULL;
        txQueue = NULL;
        interruptSource = NULL;
        timerSource = NULL;
        netif = NULL;
        netStats = NULL;
        etherStats = NULL;
        baseMap = NULL;
        baseAddr = NULL;
        rxMbufCursor = NULL;
        txMbufCursor = NULL;
        statBufDesc = NULL;
        statPhyAddr = (IOPhysicalAddress64)NULL;
        statData = NULL;
        rxPacketHead = NULL;
        rxPacketTail = NULL;
        rxPacketSize = 0;
        isEnabled = false;
        promiscusMode = false;
        multicastMode = false;
        linkUp = false;
        
        polling = false;
        
        mtu = ETH_DATA_LEN;
        powerState = 0;
        speed = SPEED_1000;
        duplex = DUPLEX_FULL;
        autoneg = AUTONEG_ENABLE;
        flowCtl = kFlowControlOff;
        eeeCap = 0;
        linuxData.configASPM = 0;
        linuxData.configEEE = 0;
        linuxData.s0MagicPacket = 0;
        linuxData.hwoptimize = 0;
        linuxData.DASH = 0;
        pciDeviceData.vendor = 0;
        pciDeviceData.device = 0;
        pciDeviceData.subsystem_vendor = 0;
        pciDeviceData.subsystem_device = 0;
        linuxData.pci_dev = &pciDeviceData;
        intrMitigateValue = 0x5f51;
        lastIntrTime = 0;
        wolCapable = false;
        wolActive = false;
        enableTSO4 = false;
        enableTSO6 = false;
        enableCSO6 = false;
        pciPMCtrlOffset = 0;
        memset(fallBackMacAddr.bytes, 0, kIOEthernetAddressSize);
    }
    
done:
    return result;
}

void LucyRTL8125::free()
{
    UInt32 i;
    
    DebugLog("free() ===>\n");
    
    if (workLoop) {
        if (interruptSource) {
            workLoop->removeEventSource(interruptSource);
            RELEASE(interruptSource);
        }
        if (timerSource) {
            workLoop->removeEventSource(timerSource);
            RELEASE(timerSource);
        }
        workLoop->release();
        workLoop = NULL;
    }
    RELEASE(commandGate);
    RELEASE(txQueue);
    RELEASE(mediumDict);
    
    for (i = MEDIUM_INDEX_AUTO; i < MEDIUM_INDEX_COUNT; i++)
        mediumTable[i] = NULL;
    
    RELEASE(baseMap);
    baseAddr = NULL;
    linuxData.mmio_addr = NULL;
    
    RELEASE(pciDevice);
    freeDMADescriptors();
    
    DebugLog("free() <===\n");
    
    super::free();
}

static const char *onName = "enabled";
static const char *offName = "disabled";

bool LucyRTL8125::start(IOService *provider)
{
    bool result;
    
    result = super::start(provider);
    
    if (!result) {
        IOLog("IOEthernetController::start failed.\n");
        goto done;
    }
    multicastMode = false;
    promiscusMode = false;
    multicastFilter = 0;

    pciDevice = OSDynamicCast(IOPCIDevice, provider);
    
    if (!pciDevice) {
        IOLog("No provider.\n");
        goto done;
    }
    pciDevice->retain();
    
    if (!pciDevice->open(this)) {
        IOLog("Failed to open provider.\n");
        goto error1;
    }
    getParams();
    
    if (!initPCIConfigSpace(pciDevice)) {
        goto error2;
    }

    if (!initRTL8125()) {
        goto error2;
    }
    
    if (!setupMediumDict()) {
        IOLog("Failed to setup medium dictionary.\n");
        goto error2;
    }
    commandGate = getCommandGate();
    
    if (!commandGate) {
        IOLog("getCommandGate() failed.\n");
        goto error2;
    }
    commandGate->retain();
    
    if (!setupDMADescriptors()) {
        IOLog("Error allocating DMA descriptors.\n");
        goto error3;
    }

    if (!initEventSources(provider)) {
        IOLog("initEventSources() failed.\n");
        goto error4;
    }
    
    result = attachInterface(reinterpret_cast<IONetworkInterface**>(&netif));

    if (!result) {
        IOLog("attachInterface() failed.\n");
        goto error4;
    }
    pciDevice->close(this);
    result = true;
    
done:
    return result;

error4:
    freeDMADescriptors();

error3:
    RELEASE(commandGate);
        
error2:
    pciDevice->close(this);
    
error1:
    pciDevice->release();
    pciDevice = NULL;
    goto done;
}

void LucyRTL8125::stop(IOService *provider)
{
    UInt32 i;
    
    if (netif) {
        detachInterface(netif);
        netif = NULL;
    }
    if (workLoop) {
        if (interruptSource) {
            workLoop->removeEventSource(interruptSource);
            RELEASE(interruptSource);
        }
        if (timerSource) {
            workLoop->removeEventSource(timerSource);
            RELEASE(timerSource);
        }
        workLoop->release();
        workLoop = NULL;
    }
    RELEASE(commandGate);
    RELEASE(txQueue);
    RELEASE(mediumDict);
    
    for (i = MEDIUM_INDEX_AUTO; i < MEDIUM_INDEX_COUNT; i++)
        mediumTable[i] = NULL;

    freeDMADescriptors();
    RELEASE(baseMap);
    baseAddr = NULL;
    linuxData.mmio_addr = NULL;

    RELEASE(pciDevice);
    
    super::stop(provider);
}

/* Power Management Support */
static IOPMPowerState powerStateArray[kPowerStateCount] =
{
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {1, kIOPMDeviceUsable, kIOPMPowerOn, kIOPMPowerOn, 0, 0, 0, 0, 0, 0, 0, 0}
};

IOReturn LucyRTL8125::registerWithPolicyMaker(IOService *policyMaker)
{
    DebugLog("registerWithPolicyMaker() ===>\n");
    
    powerState = kPowerStateOn;
    
    DebugLog("registerWithPolicyMaker() <===\n");

    return policyMaker->registerPowerDriver(this, powerStateArray, kPowerStateCount);
}

IOReturn LucyRTL8125::setPowerState(unsigned long powerStateOrdinal, IOService *policyMaker)
{
    IOReturn result = IOPMAckImplied;
    
    DebugLog("setPowerState() ===>\n");
        
    if (powerStateOrdinal == powerState) {
        DebugLog("Already in power state %lu.\n", powerStateOrdinal);
        goto done;
    }
    DebugLog("switching to power state %lu.\n", powerStateOrdinal);
    
    if (powerStateOrdinal == kPowerStateOff)
        commandGate->runAction(setPowerStateSleepAction);
    else
        commandGate->runAction(setPowerStateWakeAction);

    powerState = powerStateOrdinal;
    
done:
    DebugLog("setPowerState() <===\n");

    return result;
}

void LucyRTL8125::systemWillShutdown(IOOptionBits specifier)
{
    DebugLog("systemWillShutdown() ===>\n");
    
    if ((kIOMessageSystemWillPowerOff | kIOMessageSystemWillRestart) & specifier) {
        disable(netif);
        
        /* Restore the original MAC address. */
        rtl8125_rar_set(&linuxData, (UInt8 *)&origMacAddr.bytes);
    }
    
    DebugLog("systemWillShutdown() <===\n");

    /* Must call super shutdown or system will stall. */
    super::systemWillShutdown(specifier);
}

/* IONetworkController methods. */
IOReturn LucyRTL8125::enable(IONetworkInterface *netif)
{
    const IONetworkMedium *selectedMedium;
    IOReturn result = kIOReturnError;
    
    DebugLog("enable() ===>\n");

    if (isEnabled) {
        DebugLog("Interface already enabled.\n");
        result = kIOReturnSuccess;
        goto done;
    }
    if (!pciDevice || pciDevice->isOpen()) {
        IOLog("Unable to open PCI device.\n");
        goto done;
    }
    pciDevice->open(this);
    
    selectedMedium = getSelectedMedium();
    
    if (!selectedMedium) {
        DebugLog("No medium selected. Falling back to autonegotiation.\n");
        selectedMedium = mediumTable[MEDIUM_INDEX_AUTO];
    }
    selectMedium(selectedMedium);
    enableRTL8125();
    
    /* We have to enable the interrupt because we are using a msi interrupt. */
    interruptSource->enable();

    rxPacketHead = rxPacketTail = NULL;
    rxPacketSize = 0;
    txDescDoneCount = txDescDoneLast = 0;
    deadlockWarn = 0;
    needsUpdate = false;
    isEnabled = true;
    polling = false;
    
    result = kIOReturnSuccess;
    
    DebugLog("enable() <===\n");

done:
    return result;
}

IOReturn LucyRTL8125::disable(IONetworkInterface *netif)
{
    IOReturn result = kIOReturnSuccess;
    
    DebugLog("disable() ===>\n");
    
    if (!isEnabled)
        goto done;
    
    netif->stopOutputThread();
    netif->flushOutputQueue();
    
    polling = false;
    isEnabled = false;

    timerSource->cancelTimeout();
    needsUpdate = false;
    txDescDoneCount = txDescDoneLast = 0;

    /* Disable interrupt as we are using msi. */
    interruptSource->disable();

    disableRTL8125();
    
    clearDescriptors();
    
    if (pciDevice && pciDevice->isOpen())
        pciDevice->close(this);
        
    DebugLog("disable() <===\n");
    
done:
    return result;
}

IOReturn LucyRTL8125::outputStart(IONetworkInterface *interface, IOOptionBits options )
{
    IOPhysicalSegment txSegments[kMaxSegs];
    mbuf_t m;
    RtlTxDesc *desc, *firstDesc;
    IOReturn result = kIOReturnNoResources;
    UInt32 cmd;
    UInt32 opts2;
    mbuf_tso_request_flags_t tsoFlags;
    mbuf_csum_request_flags_t checksums;
    UInt32 mssValue;
    UInt32 opts1;
    UInt32 vlanTag;
    UInt32 numSegs;
    UInt32 lastSeg;
    UInt32 index;
    UInt32 i;
    
    //DebugLog("outputStart() ===>\n");
    
    if (!(isEnabled && linkUp)) {
        DebugLog("Interface down. Dropping packets.\n");
        goto done;
    }
    while ((txNumFreeDesc > (kMaxSegs + 3)) && (interface->dequeueOutputPackets(1, &m, NULL, NULL, NULL) == kIOReturnSuccess)) {
        cmd = 0;
        opts2 = 0;

        if (mbuf_get_tso_requested(m, &tsoFlags, &mssValue)) {
            DebugLog("mbuf_get_tso_requested() failed. Dropping packet.\n");
            freePacket(m);
            continue;
        }
        if (tsoFlags & (MBUF_TSO_IPV4 | MBUF_TSO_IPV6)) {
            if (tsoFlags & MBUF_TSO_IPV4) {
                getTso4Command(&cmd, &opts2, mssValue, tsoFlags);
            } else {
                /* The pseudoheader checksum has to be adjusted first. */
                adjustIPv6Header(m);
                getTso6Command(&cmd, &opts2, mssValue, tsoFlags);
            }
        } else {
            /* We use mssValue as a dummy here because it isn't needed anymore. */
            mbuf_get_csum_requested(m, &checksums, &mssValue);
            getChecksumCommand(&cmd, &opts2, checksums);
        }
        /* Finally get the physical segments. */
        numSegs = txMbufCursor->getPhysicalSegmentsWithCoalesce(m, &txSegments[0], kMaxSegs);

        /* Alloc required number of descriptors. As the descriptor which has been freed last must be
         * considered to be still in use we never fill the ring completely but leave at least one
         * unused.
         */
        if (!numSegs) {
            DebugLog("getPhysicalSegmentsWithCoalesce() failed. Dropping packet.\n");
            freePacket(m);
            continue;
        }
        OSAddAtomic(-numSegs, &txNumFreeDesc);
        index = txNextDescIndex;
        txNextDescIndex = (txNextDescIndex + numSegs) & kTxDescMask;
        txTailPtr0 += numSegs;
        firstDesc = &txDescArray[index];
        lastSeg = numSegs - 1;
        
        /* Next fill in the VLAN tag. */
        opts2 |= (getVlanTagDemand(m, &vlanTag)) ? (OSSwapInt16(vlanTag) | TxVlanTag) : 0;
        
        /* And finally fill in the descriptors. */
        for (i = 0; i < numSegs; i++) {
            desc = &txDescArray[index];
            opts1 = (((UInt32)txSegments[i].length) | cmd);
            opts1 |= (i == 0) ? FirstFrag : DescOwn;
            
            if (i == lastSeg) {
                opts1 |= LastFrag;
                txMbufArray[index] = m;
            } else {
                txMbufArray[index] = NULL;
            }
            if (index == kTxLastDesc)
                opts1 |= RingEnd;
            
            desc->addr = OSSwapHostToLittleInt64(txSegments[i].location);
            desc->opts2 = OSSwapHostToLittleInt32(opts2);
            desc->opts1 = OSSwapHostToLittleInt32(opts1);
            
            //DebugLog("opts1=0x%x, opts2=0x%x, addr=0x%llx, len=0x%llx\n", opts1, opts2, txSegments[i].location, txSegments[i].length);
            ++index &= kTxDescMask;
        }
        firstDesc->opts1 |= DescOwn;
    }
    /* Set the polling bit. */
    //WriteReg16(TPPOLL_8125, BIT_0);
    WriteReg16(SW_TAIL_PTR0_8125, txTailPtr0 & 0xffff);

    result = (txNumFreeDesc > (kMaxSegs + 3)) ? kIOReturnSuccess : kIOReturnNoResources;
    
done:
    //DebugLog("outputStart() <===\n");
    
    return result;
}

void LucyRTL8125::getPacketBufferConstraints(IOPacketBufferConstraints *constraints) const
{
    DebugLog("getPacketBufferConstraints() ===>\n");

    constraints->alignStart = kIOPacketBufferAlign1;
    constraints->alignLength = kIOPacketBufferAlign1;
    
    DebugLog("getPacketBufferConstraints() <===\n");
}

IOOutputQueue* LucyRTL8125::createOutputQueue()
{
    DebugLog("createOutputQueue() ===>\n");
    
    DebugLog("createOutputQueue() <===\n");

    return IOBasicOutputQueue::withTarget(this);
}

const OSString* LucyRTL8125::newVendorString() const
{
    DebugLog("newVendorString() ===>\n");
    
    DebugLog("newVendorString() <===\n");

    return OSString::withCString("Realtek");
}

const OSString* LucyRTL8125::newModelString() const
{
    DebugLog("newModelString() ===>\n");
    DebugLog("newModelString() <===\n");
    
    return OSString::withCString(rtl_chip_info[linuxData.chipset].name);
}

bool LucyRTL8125::configureInterface(IONetworkInterface *interface)
{
    char modelName[kNameLenght];
    IONetworkData *data;
    IOReturn error;
    bool result;

    DebugLog("configureInterface() ===>\n");

    result = super::configureInterface(interface);
    
    if (!result)
        goto done;
    
    /* Get the generic network statistics structure. */
    data = interface->getParameter(kIONetworkStatsKey);
    
    if (data) {
        netStats = (IONetworkStats *)data->getBuffer();
        
        if (!netStats) {
            IOLog("Error getting IONetworkStats\n.");
            result = false;
            goto done;
        }
    }
    /* Get the Ethernet statistics structure. */
    data = interface->getParameter(kIOEthernetStatsKey);
    
    if (data) {
        etherStats = (IOEthernetStats *)data->getBuffer();
        
        if (!etherStats) {
            IOLog("Error getting IOEthernetStats\n.");
            result = false;
            goto done;
        }
    }
    error = interface->configureOutputPullModel(512, 0, 0, IONetworkInterface::kOutputPacketSchedulingModelNormal);
    
    if (error != kIOReturnSuccess) {
        IOLog("configureOutputPullModel() failed\n.");
        result = false;
        goto done;
    }
    error = interface->configureInputPacketPolling(kNumRxDesc, kIONetworkWorkLoopSynchronous);
    
    if (error != kIOReturnSuccess) {
        IOLog("configureInputPacketPolling() failed\n.");
        result = false;
        goto done;
    }
    snprintf(modelName, kNameLenght, "Realtek %s PCI Express 2.5 Gigabit Ethernet", rtl_chip_info[linuxData.chipset].name);
    setProperty("model", modelName);
    
    DebugLog("configureInterface() <===\n");

done:
    return result;
}

bool LucyRTL8125::createWorkLoop()
{
    DebugLog("createWorkLoop() ===>\n");
    
    workLoop = IOWorkLoop::workLoop();
    
    DebugLog("createWorkLoop() <===\n");

    return workLoop ? true : false;
}

IOWorkLoop* LucyRTL8125::getWorkLoop() const
{
    DebugLog("getWorkLoop() ===>\n");
    
    DebugLog("getWorkLoop() <===\n");

    return workLoop;
}

/* Methods inherited from IOEthernetController. */
IOReturn LucyRTL8125::getHardwareAddress(IOEthernetAddress *addr)
{
    IOReturn result = kIOReturnError;
    
    DebugLog("getHardwareAddress() ===>\n");
    
    if (addr) {
        bcopy(&currMacAddr.bytes, addr->bytes, kIOEthernetAddressSize);
        result = kIOReturnSuccess;
    }
    
    DebugLog("getHardwareAddress() <===\n");

    return result;
}

IOReturn LucyRTL8125::setPromiscuousMode(bool active)
{
    UInt32 *filterAddr = (UInt32 *)&multicastFilter;
    UInt32 mcFilter[2];
    UInt32 rxMode;

    DebugLog("setPromiscuousMode() ===>\n");
    
    if (active) {
        DebugLog("Promiscuous mode enabled.\n");
        rxMode = (AcceptBroadcast | AcceptMulticast | AcceptMyPhys | AcceptAllPhys);
        mcFilter[1] = mcFilter[0] = 0xffffffff;
    } else {
        DebugLog("Promiscuous mode disabled.\n");
        rxMode = (AcceptBroadcast | AcceptMulticast | AcceptMyPhys);
        mcFilter[0] = *filterAddr++;
        mcFilter[1] = *filterAddr;
    }
    promiscusMode = active;
    rxMode |= rxConfigReg | (ReadReg32(RxConfig) & rxConfigMask);
    WriteReg32(RxConfig, rxMode);
    WriteReg32(MAR0, mcFilter[0]);
    WriteReg32(MAR1, mcFilter[1]);

    DebugLog("setPromiscuousMode() <===\n");

    return kIOReturnSuccess;
}

IOReturn LucyRTL8125::setMulticastMode(bool active)
{
    UInt32 *filterAddr = (UInt32 *)&multicastFilter;
    UInt32 mcFilter[2];
    UInt32 rxMode;

    DebugLog("setMulticastMode() ===>\n");
    
    if (active) {
        rxMode = (AcceptBroadcast | AcceptMulticast | AcceptMyPhys);
        mcFilter[0] = *filterAddr++;
        mcFilter[1] = *filterAddr;
    } else{
        rxMode = (AcceptBroadcast | AcceptMyPhys);
        mcFilter[1] = mcFilter[0] = 0;
    }
    multicastMode = active;
    rxMode |= rxConfigReg | (ReadReg32(RxConfig) & rxConfigMask);
    WriteReg32(RxConfig, rxMode);
    WriteReg32(MAR0, mcFilter[0]);
    WriteReg32(MAR1, mcFilter[1]);
    
    DebugLog("setMulticastMode() <===\n");
    
    return kIOReturnSuccess;
}

IOReturn LucyRTL8125::setMulticastList(IOEthernetAddress *addrs, UInt32 count)
{
    UInt32 *filterAddr = (UInt32 *)&multicastFilter;
    UInt64 filter = 0;
    UInt32 i, bitNumber;
    
    DebugLog("setMulticastList() ===>\n");
    
    if (count <= kMCFilterLimit) {
        for (i = 0; i < count; i++, addrs++) {
            bitNumber = ether_crc(6, reinterpret_cast<unsigned char *>(addrs)) >> 26;
            filter |= (1 << (bitNumber & 0x3f));
        }
        multicastFilter = OSSwapInt64(filter);
    } else {
        multicastFilter = 0xffffffffffffffff;
    }
    WriteReg32(MAR0, *filterAddr++);
    WriteReg32(MAR1, *filterAddr);

    DebugLog("setMulticastList() <===\n");

    return kIOReturnSuccess;
}

IOReturn LucyRTL8125::getChecksumSupport(UInt32 *checksumMask, UInt32 checksumFamily, bool isOutput)
{
    IOReturn result = kIOReturnUnsupported;

    DebugLog("getChecksumSupport() ===>\n");

    if ((checksumFamily == kChecksumFamilyTCPIP) && checksumMask) {
        if (isOutput) {
            *checksumMask = (enableCSO6) ? (kChecksumTCP | kChecksumUDP | kChecksumIP | kChecksumTCPIPv6 | kChecksumUDPIPv6) : (kChecksumTCP | kChecksumUDP | kChecksumIP);
        } else {
            *checksumMask = (kChecksumTCP | kChecksumUDP | kChecksumIP | kChecksumTCPIPv6 | kChecksumUDPIPv6);
        }
        result = kIOReturnSuccess;
    }
    DebugLog("getChecksumSupport() <===\n");

    return result;
}

UInt32 LucyRTL8125::getFeatures() const
{
    UInt32 features = (kIONetworkFeatureMultiPages | kIONetworkFeatureHardwareVlan);
    
    DebugLog("getFeatures() ===>\n");
    
    if (enableTSO4)
        features |= kIONetworkFeatureTSOIPv4;
    
    if (enableTSO6)
        features |= kIONetworkFeatureTSOIPv6;
    
    DebugLog("getFeatures() <===\n");
    
    return features;
}

IOReturn LucyRTL8125::setWakeOnMagicPacket(bool active)
{
    IOReturn result = kIOReturnUnsupported;

    DebugLog("setWakeOnMagicPacket() ===>\n");

    if (wolCapable) {
        linuxData.wol_enabled = active ? WOL_ENABLED : WOL_DISABLED;
        wolActive = active;
        
        DebugLog("WakeOnMagicPacket %s.\n", active ? "enabled" : "disabled");

        result = kIOReturnSuccess;
    }
    
    DebugLog("setWakeOnMagicPacket() <===\n");

    return result;
}

IOReturn LucyRTL8125::getPacketFilters(const OSSymbol *group, UInt32 *filters) const
{
    IOReturn result = kIOReturnSuccess;

    DebugLog("getPacketFilters() ===>\n");

    if ((group == gIOEthernetWakeOnLANFilterGroup) && wolCapable) {
        *filters = kIOEthernetWakeOnMagicPacket;
        DebugLog("kIOEthernetWakeOnMagicPacket added to filters.\n");
    } else {
        result = super::getPacketFilters(group, filters);
    }
    
    DebugLog("getPacketFilters() <===\n");

    return result;
}

IOReturn LucyRTL8125::setHardwareAddress(const IOEthernetAddress *addr)
{
    IOReturn result = kIOReturnError;
    
    DebugLog("setHardwareAddress() ===>\n");
    
    if (addr) {
        bcopy(addr->bytes, &currMacAddr.bytes, kIOEthernetAddressSize);
        rtl8125_rar_set(&linuxData, (UInt8 *)&currMacAddr.bytes);
        result = kIOReturnSuccess;
    }
    
    DebugLog("setHardwareAddress() <===\n");
    
    return result;
}

IOReturn LucyRTL8125::selectMedium(const IONetworkMedium *medium)
{
    IOReturn result = kIOReturnSuccess;
    
    DebugLog("selectMedium() ===>\n");
    
    if (medium) {
        flowCtl = kFlowControlOff;
        linuxData.eee_adv_t = 0;
        
        switch (medium->getIndex()) {
            case MEDIUM_INDEX_AUTO:
                autoneg = AUTONEG_ENABLE;
                speed = 0;
                duplex = DUPLEX_FULL;
                linuxData.eee_adv_t = eeeCap;
                break;
                
            case MEDIUM_INDEX_10HD:
                autoneg = AUTONEG_DISABLE;
                speed = SPEED_10;
                duplex = DUPLEX_HALF;
                break;
                
            case MEDIUM_INDEX_10FD:
                autoneg = AUTONEG_ENABLE;
                speed = SPEED_10;
                duplex = DUPLEX_FULL;
                break;
                
            case MEDIUM_INDEX_100HD:
                autoneg = AUTONEG_DISABLE;
                speed = SPEED_100;
                duplex = DUPLEX_HALF;
                break;
                
            case MEDIUM_INDEX_100FD:
                autoneg = AUTONEG_ENABLE;
                speed = SPEED_100;
                duplex = DUPLEX_FULL;
                break;
                
            case MEDIUM_INDEX_100FDFC:
                autoneg = AUTONEG_ENABLE;
                speed = SPEED_100;
                duplex = DUPLEX_FULL;
                flowCtl = kFlowControlOn;
                break;
                
            case MEDIUM_INDEX_1000FD:
                autoneg = AUTONEG_ENABLE;
                speed = SPEED_1000;
                duplex = DUPLEX_FULL;
                break;
                
            case MEDIUM_INDEX_1000FDFC:
                autoneg = AUTONEG_ENABLE;
                speed = SPEED_1000;
                duplex = DUPLEX_FULL;
                flowCtl = kFlowControlOn;
                break;
                
            case MEDIUM_INDEX_100FDEEE:
                autoneg = AUTONEG_ENABLE;
                speed = SPEED_100;
                duplex = DUPLEX_FULL;
                linuxData.eee_adv_t = eeeCap;
                break;
                
            case MEDIUM_INDEX_100FDFCEEE:
                autoneg = AUTONEG_ENABLE;
                speed = SPEED_100;
                duplex = DUPLEX_FULL;
                flowCtl = kFlowControlOn;
                linuxData.eee_adv_t = eeeCap;
                break;
                
            case MEDIUM_INDEX_1000FDEEE:
                autoneg = AUTONEG_ENABLE;
                speed = SPEED_1000;
                duplex = DUPLEX_FULL;
                linuxData.eee_adv_t = eeeCap;
                break;
                
            case MEDIUM_INDEX_1000FDFCEEE:
                autoneg = AUTONEG_ENABLE;
                speed = SPEED_1000;
                duplex = DUPLEX_FULL;
                flowCtl = kFlowControlOn;
                linuxData.eee_adv_t = eeeCap;
                break;
                
            case MEDIUM_INDEX_2500FD:
                autoneg = AUTONEG_ENABLE;
                speed = SPEED_2500;
                duplex = DUPLEX_FULL;
                break;
                
            case MEDIUM_INDEX_2500FDFC:
                autoneg = AUTONEG_ENABLE;
                speed = SPEED_2500;
                duplex = DUPLEX_FULL;
                flowCtl = kFlowControlOn;
                break;                
        }
        setPhyMedium();
        setCurrentMedium(medium);
    }
    
    DebugLog("selectMedium() <===\n");
    
done:
    return result;
}

IOReturn LucyRTL8125::getMaxPacketSize(UInt32 * maxSize) const
{
    DebugLog("getMaxPacketSize() ===>\n");
    
    *maxSize = kMaxPacketSize;
    
    DebugLog("getMaxPacketSize() <===\n");
    
    return kIOReturnSuccess;
}

IOReturn LucyRTL8125::setMaxPacketSize(UInt32 maxSize)
{
    IOReturn result = kIOReturnError;
    ifnet_t ifnet = netif->getIfnet();
    ifnet_offload_t offload;
    //UInt32 mask = (IFNET_CSUM_IP | IFNET_CSUM_TCP | IFNET_CSUM_UDP);
    UInt32 mask = 0;

    DebugLog("setMaxPacketSize() ===>\n");
    
    if (maxSize <= linuxData.max_jumbo_frame_size) {
        mtu = maxSize - (ETH_HLEN + ETH_FCS_LEN);

        DebugLog("maxSize: %u, mtu: %u\n", maxSize, mtu);
        
        if (enableTSO4)
            mask |= IFNET_TSO_IPV4;
        
        if (enableTSO6)
            mask |= IFNET_TSO_IPV6;
/*
        if (enableCSO6)
            mask |= (IFNET_CSUM_TCPIPV6 | IFNET_CSUM_UDPIPV6);
*/
        offload = ifnet_offload(ifnet);
        
        if (mtu > MSS_MAX) {
            offload &= ~mask;
            DebugLog("Disable hardware offload features: %x!\n", mask);
        } else {
            offload |= mask;
            DebugLog("Enable hardware offload features: %x!\n", mask);
        }
        if (ifnet_set_offload(ifnet, offload))
            IOLog("Error setting hardware offload: %x!\n", offload);

        /* Force reinitialization. */
        setLinkDown();
        timerSource->cancelTimeout();
        //updateStatistics(&adapterData);
        restartRTL8125();

        result = kIOReturnSuccess;
    }
    
    DebugLog("setMaxPacketSize() <===\n");
    
    return result;
}

#pragma mark --- data structure initialization methods ---

void LucyRTL8125::getParams()
{
    OSDictionary *params;
    OSNumber *intrMit;
    OSBoolean *enableEEE;
    OSBoolean *tso4;
    OSBoolean *tso6;
    OSBoolean *csoV6;
    OSBoolean *noASPM;
    OSString *versionString;
    OSString *fbAddr;

    versionString = OSDynamicCast(OSString, getProperty(kDriverVersionName));

    params = OSDynamicCast(OSDictionary, getProperty(kParamName));
    
    if (params) {
        noASPM = OSDynamicCast(OSBoolean, params->getObject(kDisableASPMName));
        linuxData.configASPM = (noASPM) ? !(noASPM->getValue()) : 0;
        
        DebugLog("PCIe ASPM support %s.\n", linuxData.configASPM ? onName : offName);
        
        enableEEE = OSDynamicCast(OSBoolean, params->getObject(kEnableEeeName));
        
        if (enableEEE)
            linuxData.eee_enabled = (enableEEE->getValue()) ? 1 : 0;
        else
            linuxData.eee_enabled = 0;
        
        IOLog("EEE support %s.\n", linuxData.eee_enabled ? onName : offName);
        
        tso4 = OSDynamicCast(OSBoolean, params->getObject(kEnableTSO4Name));
        enableTSO4 = (tso4) ? tso4->getValue() : false;
        
        IOLog("TCP/IPv4 segmentation offload %s.\n", enableTSO4 ? onName : offName);
        
        tso6 = OSDynamicCast(OSBoolean, params->getObject(kEnableTSO6Name));
        enableTSO6 = (tso6) ? tso6->getValue() : false;
        
        IOLog("TCP/IPv6 segmentation offload %s.\n", enableTSO6 ? onName : offName);
        
        csoV6 = OSDynamicCast(OSBoolean, params->getObject(kEnableCSO6Name));
        enableCSO6 = (csoV6) ? csoV6->getValue() : false;
        
        IOLog("TCP/IPv6 checksum offload %s.\n", enableCSO6 ? onName : offName);
        
        intrMit = OSDynamicCast(OSNumber, params->getObject(kIntrMitigateName));
/*
        if (intrMit && !rxPoll)
            intrMitigateValue = intrMit->unsigned16BitValue();
*/
        fbAddr = OSDynamicCast(OSString, params->getObject(kFallbackName));
        
        if (fbAddr) {
            const char *s = fbAddr->getCStringNoCopy();
            UInt8 *addr = fallBackMacAddr.bytes;
            
            if (fbAddr->getLength()) {
                sscanf(s, "%2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx", &addr[0], &addr[1], &addr[2], &addr[3], &addr[4], &addr[5]);
                
                IOLog("Fallback MAC: %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n",
                      fallBackMacAddr.bytes[0], fallBackMacAddr.bytes[1],
                      fallBackMacAddr.bytes[2], fallBackMacAddr.bytes[3],
                      fallBackMacAddr.bytes[4], fallBackMacAddr.bytes[5]);
            }
        }
    } else {
        enableTSO4 = true;
        enableTSO6 = true;
        intrMitigateValue = 0x5f51;
    }
    if (versionString)
        IOLog("Version %s using interrupt mitigate value 0x%x. Please don't support tonymacx86.com!\n", versionString->getCStringNoCopy(), intrMitigateValue);
    else
        IOLog("Using interrupt mitigate value 0x%x. Please don't support tonymacx86.com!\n", intrMitigateValue);
}

static IOMediumType mediumTypeArray[MEDIUM_INDEX_COUNT] = {
    kIOMediumEthernetAuto,
    (kIOMediumEthernet10BaseT | kIOMediumOptionHalfDuplex),
    (kIOMediumEthernet10BaseT | kIOMediumOptionFullDuplex),
    (kIOMediumEthernet100BaseTX | kIOMediumOptionHalfDuplex),
    (kIOMediumEthernet100BaseTX | kIOMediumOptionFullDuplex),
    (kIOMediumEthernet100BaseTX | kIOMediumOptionFullDuplex | kIOMediumOptionFlowControl),
    (kIOMediumEthernet1000BaseT | kIOMediumOptionFullDuplex),
    (kIOMediumEthernet1000BaseT | kIOMediumOptionFullDuplex | kIOMediumOptionFlowControl),
    (kIOMediumEthernet100BaseTX | kIOMediumOptionFullDuplex | kIOMediumOptionEEE),
    (kIOMediumEthernet100BaseTX | kIOMediumOptionFullDuplex | kIOMediumOptionFlowControl | kIOMediumOptionEEE),
    (kIOMediumEthernet1000BaseT | kIOMediumOptionFullDuplex | kIOMediumOptionEEE),
    (kIOMediumEthernet1000BaseT | kIOMediumOptionFullDuplex | kIOMediumOptionFlowControl | kIOMediumOptionEEE),
    (kIOMediumEthernet2500BaseT | kIOMediumOptionFullDuplex),
    (kIOMediumEthernet2500BaseT | kIOMediumOptionFullDuplex | kIOMediumOptionFlowControl)
};

static UInt64 mediumSpeedArray[MEDIUM_INDEX_COUNT] = {
    0,
    10 * MBit,
    10 * MBit,
    100 * MBit,
    100 * MBit,
    100 * MBit,
    1000 * MBit,
    1000 * MBit,
    100 * MBit,
    100 * MBit,
    1000 * MBit,
    1000 * MBit,
    2500 * MBit,
    2500 * MBit,
};

bool LucyRTL8125::setupMediumDict()
{
    IONetworkMedium *medium;
    UInt32 i;
    bool result = false;

    mediumDict = OSDictionary::withCapacity(MEDIUM_INDEX_COUNT + 1);

    if (mediumDict) {
        for (i = MEDIUM_INDEX_AUTO; i < MEDIUM_INDEX_COUNT; i++) {
            medium = IONetworkMedium::medium(mediumTypeArray[i], mediumSpeedArray[i], 0, i);
            
            if (!medium)
                goto error1;

            result = IONetworkMedium::addMedium(mediumDict, medium);
            medium->release();

            if (!result)
                goto error1;

            mediumTable[i] = medium;
        }
    }
    result = publishMediumDictionary(mediumDict);
    
    if (!result)
        goto error1;

done:
    return result;
    
error1:
    IOLog("Error creating medium dictionary.\n");
    mediumDict->release();
    
    for (i = MEDIUM_INDEX_AUTO; i < MEDIUM_INDEX_COUNT; i++)
        mediumTable[i] = NULL;

    goto done;
}

bool LucyRTL8125::initEventSources(IOService *provider)
{
    IOReturn intrResult;
    int msiIndex = -1;
    int intrIndex = 0;
    int intrType = 0;
    bool result = false;
    
    txQueue = reinterpret_cast<IOBasicOutputQueue *>(getOutputQueue());
    
    if (txQueue == NULL) {
        IOLog("Failed to get output queue.\n");
        goto done;
    }
    txQueue->retain();
    
    while ((intrResult = pciDevice->getInterruptType(intrIndex, &intrType)) == kIOReturnSuccess) {
        if (intrType & kIOInterruptTypePCIMessaged){
            msiIndex = intrIndex;
            break;
        }
        intrIndex++;
    }
    if (msiIndex != -1) {
        DebugLog("MSI interrupt index: %d\n", msiIndex);
        
        interruptSource = IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventSource::Action, this, &LucyRTL8125::interruptOccurredPoll), provider, msiIndex);
    }
    if (!interruptSource) {
        IOLog("Error: MSI index was not found or MSI interrupt could not be enabled.\n");
        goto error1;
    }
    workLoop->addEventSource(interruptSource);
    
    timerSource = IOTimerEventSource::timerEventSource(this, OSMemberFunctionCast(IOTimerEventSource::Action, this, &LucyRTL8125::timerActionRTL8125));
    
    if (!timerSource) {
        IOLog("Failed to create IOTimerEventSource.\n");
        goto error2;
    }
    workLoop->addEventSource(timerSource);

    result = true;
    
done:
    return result;
    
error2:
    workLoop->removeEventSource(interruptSource);
    RELEASE(interruptSource);

error1:
    IOLog("Error initializing event sources.\n");
    txQueue->release();
    txQueue = NULL;
    goto done;
}

bool LucyRTL8125::setupDMADescriptors()
{
    IOPhysicalSegment rxSegment;
    mbuf_t spareMbuf[kRxNumSpareMbufs];
    mbuf_t m;
    UInt32 i;
    UInt32 opts1;
    bool result = false;
    
    /* Create transmitter descriptor array. */
    txBufDesc = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(kernel_task, (kIODirectionInOut | kIOMemoryPhysicallyContiguous | kIOMapInhibitCache), kTxDescSize, 0xFFFFFFFFFFFFFF00ULL);
            
    if (!txBufDesc) {
        IOLog("Couldn't alloc txBufDesc.\n");
        goto done;
    }
    if (txBufDesc->prepare() != kIOReturnSuccess) {
        IOLog("txBufDesc->prepare() failed.\n");
        goto error1;
    }
    txDescArray = (RtlTxDesc *)txBufDesc->getBytesNoCopy();
    txPhyAddr = OSSwapHostToLittleInt64(txBufDesc->getPhysicalAddress());
    
    /* Initialize txDescArray. */
    bzero(txDescArray, kTxDescSize);
    txDescArray[kTxLastDesc].opts1 = OSSwapHostToLittleInt32(RingEnd);
    
    for (i = 0; i < kNumTxDesc; i++) {
        txMbufArray[i] = NULL;
    }
    txNextDescIndex = txDirtyDescIndex = 0;
    txTailPtr0 = txClosePtr0 = 0;
    txNumFreeDesc = kNumTxDesc;
    txMbufCursor = IOMbufNaturalMemoryCursor::withSpecification(0x1000, kMaxSegs);
    
    if (!txMbufCursor) {
        IOLog("Couldn't create txMbufCursor.\n");
        goto error2;
    }
    
    /* Create receiver descriptor array. */
    rxBufDesc = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(kernel_task, (kIODirectionInOut | kIOMemoryPhysicallyContiguous | kIOMapInhibitCache), kRxDescSize, 0xFFFFFFFFFFFFFF00ULL);
    
    if (!rxBufDesc) {
        IOLog("Couldn't alloc rxBufDesc.\n");
        goto error3;
    }
    
    if (rxBufDesc->prepare() != kIOReturnSuccess) {
        IOLog("rxBufDesc->prepare() failed.\n");
        goto error4;
    }
    rxDescArray = (RtlRxDesc *)rxBufDesc->getBytesNoCopy();
    rxPhyAddr = OSSwapHostToLittleInt64(rxBufDesc->getPhysicalAddress());
    
    /* Initialize rxDescArray. */
    bzero(rxDescArray, kRxDescSize);
    rxDescArray[kRxLastDesc].opts1 = OSSwapHostToLittleInt32(RingEnd);

    for (i = 0; i < kNumRxDesc; i++) {
        rxMbufArray[i] = NULL;
    }
    rxNextDescIndex = 0;
    
    rxMbufCursor = IOMbufNaturalMemoryCursor::withSpecification(PAGE_SIZE, 1);
    
    if (!rxMbufCursor) {
        IOLog("Couldn't create rxMbufCursor.\n");
        goto error5;
    }
    /* Alloc receive buffers. */
    for (i = 0; i < kNumRxDesc; i++) {
        m = allocatePacket(kRxBufferPktSize);
        
        if (!m) {
            IOLog("Couldn't alloc receive buffer.\n");
            goto error6;
        }
        rxMbufArray[i] = m;
        
        if (rxMbufCursor->getPhysicalSegments(m, &rxSegment, 1) != 1) {
            
            IOLog("getPhysicalSegments() for receive buffer failed.\n");
            goto error6;
        }
        opts1 = (UInt32)rxSegment.length;
        opts1 |= (i == kRxLastDesc) ? (RingEnd | DescOwn) : DescOwn;
        rxDescArray[i].opts1 = OSSwapHostToLittleInt32(opts1);
        rxDescArray[i].opts2 = 0;
        rxDescArray[i].addr = OSSwapHostToLittleInt64(rxSegment.location);
    }
    /* Create statistics dump buffer. */
    statBufDesc = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(kernel_task, (kIODirectionIn | kIOMemoryPhysicallyContiguous | kIOMapInhibitCache), sizeof(RtlStatData), 0xFFFFFFFFFFFFFF00ULL);
    
    if (!statBufDesc) {
        IOLog("Couldn't alloc statBufDesc.\n");
        goto error6;
    }
    
    if (statBufDesc->prepare() != kIOReturnSuccess) {
        IOLog("statBufDesc->prepare() failed.\n");
        goto error7;
    }
    statData = (RtlStatData *)statBufDesc->getBytesNoCopy();
    statPhyAddr = OSSwapHostToLittleInt64(statBufDesc->getPhysicalAddress());
    
    /* Initialize statData. */
    bzero(statData, sizeof(RtlStatData));

    /* Allocate some spare mbufs and free them in order to increase the buffer pool.
     * This seems to avoid the replaceOrCopyPacket() errors under heavy load.
     */
    for (i = 0; i < kRxNumSpareMbufs; i++)
        spareMbuf[i] = allocatePacket(kRxBufferPktSize);

    for (i = 0; i < kRxNumSpareMbufs; i++) {
        if (spareMbuf[i])
            freePacket(spareMbuf[i]);
    }
    result = true;
    
done:
    return result;

error7:
    statBufDesc->release();
    statBufDesc = NULL;
    
error6:
    for (i = 0; i < kNumRxDesc; i++) {
        if (rxMbufArray[i]) {
            freePacket(rxMbufArray[i]);
            rxMbufArray[i] = NULL;
        }
    }
    RELEASE(rxMbufCursor);

error5:
    rxBufDesc->complete();
    
error4:
    rxBufDesc->release();
    rxBufDesc = NULL;

error3:
    RELEASE(txMbufCursor);
    
error2:
    txBufDesc->complete();

error1:
    txBufDesc->release();
    txBufDesc = NULL;
    goto done;
}

void LucyRTL8125::freeDMADescriptors()
{
    UInt32 i;
    
    if (txBufDesc) {
        txBufDesc->complete();
        txBufDesc->release();
        txBufDesc = NULL;
        txPhyAddr = (IOPhysicalAddress64)NULL;
    }
    RELEASE(txMbufCursor);
    
    if (rxBufDesc) {
        rxBufDesc->complete();
        rxBufDesc->release();
        rxBufDesc = NULL;
        rxPhyAddr = (IOPhysicalAddress64)NULL;
    }
    RELEASE(rxMbufCursor);
    
    for (i = 0; i < kNumRxDesc; i++) {
        if (rxMbufArray[i]) {
            freePacket(rxMbufArray[i]);
            rxMbufArray[i] = NULL;
        }
    }
    if (statBufDesc) {
        statBufDesc->complete();
        statBufDesc->release();
        statBufDesc = NULL;
        statPhyAddr = (IOPhysicalAddress64)NULL;
        statData = NULL;
    }
}

void LucyRTL8125::clearDescriptors()
{
    mbuf_t m;
    UInt32 lastIndex = kTxLastDesc;
    UInt32 opts1;
    UInt32 i;
    
    DebugLog("clearDescriptors() ===>\n");
    
    for (i = 0; i < kNumTxDesc; i++) {
        txDescArray[i].opts1 = OSSwapHostToLittleInt32((i != lastIndex) ? 0 : RingEnd);
        m = txMbufArray[i];
        
        if (m) {
            freePacket(m);
            txMbufArray[i] = NULL;
        }
    }
    txTailPtr0 = txClosePtr0 = 0;
    txDirtyDescIndex = txNextDescIndex = 0;
    txNumFreeDesc = kNumTxDesc;
    
    lastIndex = kRxLastDesc;
    
    for (i = 0; i < kNumRxDesc; i++) {
        opts1 = (UInt32)kRxBufferPktSize;
        opts1 |= (i == kRxLastDesc) ? (RingEnd | DescOwn) : DescOwn;
        rxDescArray[i].opts1 = OSSwapHostToLittleInt32(opts1);
        rxDescArray[i].opts2 = 0;
    }
    rxNextDescIndex = 0;
    deadlockWarn = 0;

    /* Free packet fragments which haven't been upstreamed yet.  */
    discardPacketFragment();
    
    DebugLog("clearDescriptors() <===\n");
}

void LucyRTL8125::discardPacketFragment()
{
    /*
     * In case there is a packet fragment which hasn't been enqueued yet
     * we have to free it in order to prevent a memory leak.
     */
    if (rxPacketHead)
        freePacket(rxPacketHead);
    
    rxPacketHead = rxPacketTail = NULL;
    rxPacketSize = 0;
}

#pragma mark --- common interrupt methods ---

void LucyRTL8125::pciErrorInterrupt()
{
    UInt16 cmdReg = pciDevice->configRead16(kIOPCIConfigCommand);
    UInt16 statusReg = pciDevice->configRead16(kIOPCIConfigStatus);
    
    DebugLog("PCI error: cmdReg=0x%x, statusReg=0x%x\n", cmdReg, statusReg);

    cmdReg |= (kIOPCICommandSERR | kIOPCICommandParityError);
    statusReg &= (kIOPCIStatusParityErrActive | kIOPCIStatusSERRActive | kIOPCIStatusMasterAbortActive | kIOPCIStatusTargetAbortActive | kIOPCIStatusTargetAbortCapable);
    pciDevice->configWrite16(kIOPCIConfigCommand, cmdReg);
    pciDevice->configWrite16(kIOPCIConfigStatus, statusReg);
    
    /* Reset the NIC in order to resume operation. */
    restartRTL8125();
}
/*
void LucyRTL8125::txInterrupt()
{
    mbuf_t m;
    SInt32 numDirty = kNumTxDesc - txNumFreeDesc;
    UInt32 oldDirtyIndex = txDirtyDescIndex;
    UInt32 descStatus;
    
    while (numDirty-- > 0) {
        descStatus = OSSwapLittleToHostInt32(txDescArray[txDirtyDescIndex].opts1);
        
        if (descStatus & DescOwn)
            break;

        m = txMbufArray[txDirtyDescIndex];
        txMbufArray[txDirtyDescIndex] = NULL;

        if (m)
            freePacket(m, kDelayFree);
 
        txDescDoneCount++;
        OSIncrementAtomic(&txNumFreeDesc);
        ++txDirtyDescIndex &= kTxDescMask;
    }
    if (oldDirtyIndex != txDirtyDescIndex) {
        if (txNumFreeDesc > kTxQueueWakeTreshhold)
            netif->signalOutputThread();
        
        WriteReg16(TPPOLL_8125, BIT_0);
        releaseFreePackets();
    }
    if (!polling)
        etherStats->dot3TxExtraEntry.interrupts++;
}
*/
void LucyRTL8125::txInterrupt()
{
    mbuf_t m;
    UInt32 nextClosePtr = ReadReg16(HW_CLO_PTR0_8125);
    UInt32 oldDirtyIndex = txDirtyDescIndex;
    UInt32 numDone;

    numDone = ((nextClosePtr - txClosePtr0) & 0xffff);
    
    //DebugLog("txInterrupt() txClosePtr0: %u, nextClosePtr: %u, numDone: %u.\n", txClosePtr0, nextClosePtr, numDone);
    
    txClosePtr0 = nextClosePtr;

    while (numDone-- > 0) {
        m = txMbufArray[txDirtyDescIndex];
        txMbufArray[txDirtyDescIndex] = NULL;

        if (m)
            freePacket(m, kDelayFree);

        txDescDoneCount++;
        OSIncrementAtomic(&txNumFreeDesc);
        ++txDirtyDescIndex &= kTxDescMask;
    }
    if (oldDirtyIndex != txDirtyDescIndex) {
        if (txNumFreeDesc > kTxQueueWakeTreshhold)
            netif->signalOutputThread();
        
        releaseFreePackets();
    }
    if (!polling)
        etherStats->dot3TxExtraEntry.interrupts++;
}

UInt32 LucyRTL8125::rxInterrupt(IONetworkInterface *interface, uint32_t maxCount, IOMbufQueue *pollQueue, void *context)
{
    IOPhysicalSegment rxSegment;
    RtlRxDesc *desc = &rxDescArray[rxNextDescIndex];
    mbuf_t bufPkt, newPkt;
    UInt64 addr;
    UInt32 opts1, opts2;
    UInt32 descStatus1, descStatus2;
    UInt32 pktSize;
    UInt32 goodPkts = 0;
    bool replaced;
    
    while (!((descStatus1 = OSSwapLittleToHostInt32(desc->opts1)) & DescOwn) && (goodPkts < maxCount)) {
        opts1 = (rxNextDescIndex == kRxLastDesc) ? (RingEnd | DescOwn) : DescOwn;
        opts2 = 0;
        addr = 0;
        
        /* As we don't support jumbo frames we consider fragmented packets as errors. *//*
        if ((descStatus1 & (FirstFrag|LastFrag)) != (FirstFrag|LastFrag)) {
            DebugLog("Fragmented packet.\n");
            etherStats->dot3StatsEntry.frameTooLongs++;
            opts1 |= kRxBufferPktSize;
            goto nextDesc;
        }*/
        
        descStatus2 = OSSwapLittleToHostInt32(desc->opts2);
        pktSize = (descStatus1 & 0x1fff) - kIOEthernetCRCSize;
        bufPkt = rxMbufArray[rxNextDescIndex];
        //DebugLog("rxInterrupt(): descStatus1=0x%x, descStatus2=0x%x, pktSize=%u\n", descStatus1, descStatus2, pktSize);
        
        newPkt = replaceOrCopyPacket(&bufPkt, pktSize, &replaced);
        
        if (!newPkt) {
            /* Allocation of a new packet failed so that we must leave the original packet in place. */
            DebugLog("replaceOrCopyPacket() failed.\n");
            etherStats->dot3RxExtraEntry.resourceErrors++;
            opts1 |= kRxBufferPktSize;
            goto nextDesc;
        }
        
        /* If the packet was replaced we have to update the descriptor's buffer address. */
        if (replaced) {
            if (rxMbufCursor->getPhysicalSegments(bufPkt, &rxSegment, 1) != 1) {
                DebugLog("getPhysicalSegments() failed.\n");
                etherStats->dot3RxExtraEntry.resourceErrors++;
                freePacket(bufPkt);
                opts1 |= kRxBufferPktSize;
                goto nextDesc;
            }
            opts1 |= ((UInt32)rxSegment.length & 0x0000ffff);
            addr = rxSegment.location;
            rxMbufArray[rxNextDescIndex] = bufPkt;
        } else {
            opts1 |= kRxBufferPktSize;
        }
        /* Set the length of the buffer. */
        mbuf_setlen(newPkt, pktSize);

        if (descStatus1 & LastFrag) {
            if (rxPacketHead) {
                /* This is the last buffer of a jumbo frame. */
                mbuf_setflags_mask(newPkt, 0, MBUF_PKTHDR);
                mbuf_setnext(rxPacketTail, newPkt);
                
                rxPacketSize += pktSize;
                rxPacketTail = newPkt;
            } else {
                /*
                 * We've got a complete packet in one buffer.
                 * It can be enqueued directly.
                 */
                rxPacketHead = newPkt;
                rxPacketSize = pktSize;
            }
            getChecksumResult(newPkt, descStatus1, descStatus2);

            /* Also get the VLAN tag if there is any. */
            if (descStatus2 & RxVlanTag)
                setVlanTag(rxPacketHead, OSSwapInt16(descStatus2 & 0xffff));

            mbuf_pkthdr_setlen(rxPacketHead, rxPacketSize);
            interface->enqueueInputPacket(rxPacketHead, pollQueue);
            
            rxPacketHead = rxPacketTail = NULL;
            rxPacketSize = 0;

            goodPkts++;
        } else {
            if (rxPacketHead) {
                /* We are in the middle of a jumbo frame. */
                mbuf_setflags_mask(newPkt, 0, MBUF_PKTHDR);
                mbuf_setnext(rxPacketTail, newPkt);

                rxPacketTail = newPkt;
                rxPacketSize += pktSize;
            } else {
                /* This is the first buffer of a jumbo frame. */
                rxPacketHead = rxPacketTail = newPkt;
                rxPacketSize = pktSize;
            }
        }
        
        /* Finally update the descriptor and get the next one to examine. */
    nextDesc:
        if (addr)
            desc->addr = OSSwapHostToLittleInt64(addr);
        
        desc->opts2 = OSSwapHostToLittleInt32(opts2);
        desc->opts1 = OSSwapHostToLittleInt32(opts1);
        
        ++rxNextDescIndex &= kRxDescMask;
        desc = &rxDescArray[rxNextDescIndex];
    }
    return goodPkts;
}

void LucyRTL8125::checkLinkStatus()
{
    struct rtl8125_private *tp = &linuxData;
    UInt16 newIntrMitigate = 0x5f51;
    UInt16 currLinkState;
        
    currLinkState = ReadReg16(PHYstatus);
    
    if (currLinkState & LinkStatus) {
        /* Get EEE mode. */
        eeeMode = getEEEMode();
        
        /* Get link speed, duplex and flow-control mode. */
        if (currLinkState & (TxFlowCtrl | RxFlowCtrl)) {
            flowCtl = kFlowControlOn;
        } else {
            flowCtl = kFlowControlOff;
        }
        if (currLinkState & _2500bpsF) {
            speed = SPEED_2500;
            duplex = DUPLEX_FULL;

            newIntrMitigate = intrMitigateValue;
        } else if (currLinkState & _1000bpsF) {
                speed = SPEED_1000;
                duplex = DUPLEX_FULL;

                newIntrMitigate = intrMitigateValue;
        } else if (currLinkState & _100bps) {
            speed = SPEED_100;
            
            if (currLinkState & FullDup) {
                duplex = DUPLEX_FULL;
            } else {
                duplex = DUPLEX_HALF;
            }
        } else {
            speed = SPEED_10;
            
            if (currLinkState & FullDup) {
                duplex = DUPLEX_FULL;
            } else {
                duplex = DUPLEX_HALF;
            }
        }
        setupRTL8125(newIntrMitigate, true);
        
        if (tp->mcfg == CFG_METHOD_2) {
            if (ReadReg16(PHYstatus) & FullDup)
                WriteReg32(TxConfig, (ReadReg32(TxConfig) | (BIT_24 | BIT_25)) & ~BIT_19);
            else
                WriteReg32(TxConfig, (ReadReg32(TxConfig) | BIT_25) & ~(BIT_19 | BIT_24));
        }

        if ((tp->mcfg == CFG_METHOD_2 || tp->mcfg == CFG_METHOD_3 ||
             tp->mcfg == CFG_METHOD_4 || tp->mcfg == CFG_METHOD_5) &&
            (ReadReg16(PHYstatus) & _10bps))
                rtl8125_enable_eee_plus(tp);

        setLinkUp();
        timerSource->setTimeoutMS(kTimeoutMS);
        
        rtl8125_mdio_write(tp, 0x1F, 0x0000);
        linuxData.phy_reg_anlpar = rtl8125_mdio_read(tp, MII_LPA);
    } else {
        tp->phy_reg_anlpar = 0;

        if (tp->mcfg == CFG_METHOD_2 ||
            tp->mcfg == CFG_METHOD_3 ||
            tp->mcfg == CFG_METHOD_4 ||
            tp->mcfg == CFG_METHOD_5)
                rtl8125_disable_eee_plus(tp);

        /* Stop watchdog and statistics updates. */
        timerSource->cancelTimeout();
        setLinkDown();
        
    }
}

void LucyRTL8125::interruptOccurredPoll(OSObject *client, IOInterruptEventSource *src, int count)
{
    UInt32 packets;
    UInt32 status;
    
    status = ReadReg32(ISR0_8125);
    
    //DebugLog("interruptOccurredPoll: status = 0x%x.\n", status);

    /* hotplug/major error/no more work/shared irq */
    if ((status == 0xFFFFFFFF) || !status)
        goto done;
    
    WriteReg32(IMR0_8125, 0x0000);
    WriteReg32(ISR0_8125, (status & ~RxFIFOOver));

    if (status & SYSErr) {
        pciErrorInterrupt();
        goto done;
    }
    if (!polling) {
        /* Rx interrupt */
        if (status & (RxOK | RxDescUnavail | RxFIFOOver)) {
            packets = rxInterrupt(netif, kNumRxDesc, NULL, NULL);
            
            if (packets)
                netif->flushInputQueue();
        }
        /* Tx interrupt */
        if (status & (TxOK | TxErr | TxDescUnavail))
            txInterrupt();
    }
    if (status & LinkChg)
        checkLinkStatus();
    
done:
    WriteReg32(IMR0_8125, intrMask);
}

bool LucyRTL8125::checkForDeadlock()
{
    bool deadlock = false;
    
    if ((txDescDoneCount == txDescDoneLast) && (txNumFreeDesc < kNumTxDesc)) {
        if (++deadlockWarn == kTxCheckTreshhold) {
            /* Some members of the RTL8125 family seem to be prone to lose transmitter rinterrupts.
             * In order to avoid false positives when trying to detect transmitter deadlocks, check
             * the transmitter ring once for completed descriptors before we assume a deadlock.
             */
            DebugLog("Warning: Tx timeout, ISR0=0x%x, IMR0=0x%x, polling=%u.\n", ReadReg32(ISR0_8125), ReadReg32(IMR0_8125), polling);
            etherStats->dot3TxExtraEntry.timeouts++;
            txInterrupt();
        } else if (deadlockWarn >= kTxDeadlockTreshhold) {
#ifdef DEBUG
            UInt32 i, index;
            
            for (i = 0; i < 10; i++) {
                index = ((txDirtyDescIndex - 1 + i) & kTxDescMask);
                IOLog("desc[%u]: opts1=0x%x, opts2=0x%x, addr=0x%llx.\n", index, txDescArray[index].opts1, txDescArray[index].opts2, txDescArray[index].addr);
            }
#endif
            IOLog("Tx stalled? Resetting chipset. ISR0=0x%x, IMR0=0x%x.\n", ReadReg32(ISR0_8125), ReadReg32(IMR0_8125));
            etherStats->dot3TxExtraEntry.resets++;
            restartRTL8125();
            deadlock = true;
        }
    } else {
        deadlockWarn = 0;
    }
    return deadlock;
}

#pragma mark --- rx poll methods ---

IOReturn LucyRTL8125::setInputPacketPollingEnable(IONetworkInterface *interface, bool enabled)
{
    //DebugLog("setInputPacketPollingEnable() ===>\n");

    if (enabled) {
        intrMask = intrMaskPoll;
        polling = true;
    } else {
        intrMask = intrMaskRxTx;
        polling = false;
    }
    if(isEnabled)
        WriteReg32(IMR0_8125, intrMask);

    //DebugLog("input polling %s.\n", enabled ? "enabled" : "disabled");

    //DebugLog("setInputPacketPollingEnable() <===\n");
    
    return kIOReturnSuccess;
}

void LucyRTL8125::pollInputPackets(IONetworkInterface *interface, uint32_t maxCount, IOMbufQueue *pollQueue, void *context )
{
    //DebugLog("pollInputPackets() ===>\n");
    
    rxInterrupt(interface, maxCount, pollQueue, context);
    
    /* Finally cleanup the transmitter ring. */
    txInterrupt();
    
    //DebugLog("pollInputPackets() <===\n");
}

#pragma mark --- hardware specific methods ---

void LucyRTL8125::getTso4Command(UInt32 *cmd1, UInt32 *cmd2, UInt32 mssValue, mbuf_tso_request_flags_t tsoFlags)
{
    *cmd1 = (GiantSendv4 | (kMinL4HdrOffsetV4 << GTTCPHO_SHIFT));
    *cmd2 = ((mssValue & MSSMask) << MSSShift_8125);
}

void LucyRTL8125::getTso6Command(UInt32 *cmd1, UInt32 *cmd2, UInt32 mssValue, mbuf_tso_request_flags_t tsoFlags)
{
    *cmd1 = (GiantSendv6 | (kMinL4HdrOffsetV6 << GTTCPHO_SHIFT));
    *cmd2 = ((mssValue & MSSMask) << MSSShift_8125);
}

void LucyRTL8125::getChecksumCommand(UInt32 *cmd1, UInt32 *cmd2, mbuf_csum_request_flags_t checksums)
{
    if (checksums & kChecksumTCP)
        *cmd2 = (TxIPCS_C | TxTCPCS_C);
    else if (checksums & kChecksumUDP)
        *cmd2 = (TxIPCS_C | TxUDPCS_C);
    else if (checksums & kChecksumIP)
        *cmd2 = TxIPCS_C;
    else if (checksums & kChecksumTCPIPv6)
        *cmd2 = (TxTCPCS_C | TxIPV6F_C | ((kMinL4HdrOffsetV6 & TCPHO_MAX) << TCPHO_SHIFT));
    else if (checksums & kChecksumUDPIPv6)
        *cmd2 = (TxUDPCS_C | TxIPV6F_C | ((kMinL4HdrOffsetV6 & TCPHO_MAX) << TCPHO_SHIFT));
}

#ifdef DEBUG

void LucyRTL8125::getChecksumResult(mbuf_t m, UInt32 status1, UInt32 status2)
{
    UInt32 resultMask = 0;
    UInt32 validMask = 0;
    UInt32 pktType = (status1 & RxProtoMask);
    
    /* Get the result of the checksum calculation and store it in the packet. */
    if (pktType == RxTCPT) {
        /* TCP packet */
        if (status2 & RxV4F) {
            resultMask = (kChecksumTCP | kChecksumIP);
            validMask = (status1 & RxTCPF) ? 0 : (kChecksumTCP | kChecksumIP);
        } else if (status2 & RxV6F) {
            resultMask = kChecksumTCPIPv6;
            validMask = (status1 & RxTCPF) ? 0 : kChecksumTCPIPv6;
        }
    } else if (pktType == RxUDPT) {
        /* UDP packet */
        if (status2 & RxV4F) {
            resultMask = (kChecksumUDP | kChecksumIP);
            validMask = (status1 & RxUDPF) ? 0 : (kChecksumUDP | kChecksumIP);
        } else if (status2 & RxV6F) {
            resultMask = kChecksumUDPIPv6;
            validMask = (status1 & RxUDPF) ? 0 : kChecksumUDPIPv6;
        }
    } else if ((pktType == 0) && (status2 & RxV4F)) {
        /* IP packet */
        resultMask = kChecksumIP;
        validMask = (status1 & RxIPF) ? 0 : kChecksumIP;
    }
    if (validMask != resultMask)
        IOLog("checksums applied: 0x%x, checksums valid: 0x%x\n", resultMask, validMask);

    if (validMask)
        setChecksumResult(m, kChecksumFamilyTCPIP, resultMask, validMask);
}

#else

void LucyRTL8125::getChecksumResult(mbuf_t m, UInt32 status1, UInt32 status2)
{
    UInt32 resultMask = 0;
    UInt32 pktType = (status1 & RxProtoMask);
    
    /* Get the result of the checksum calculation and store it in the packet. */
    if (pktType == RxTCPT) {
        /* TCP packet */
        if (status2 & RxV4F)
            resultMask = (status1 & RxTCPF) ? 0 : (kChecksumTCP | kChecksumIP);
        else if (status2 & RxV6F)
            resultMask = (status1 & RxTCPF) ? 0 : kChecksumTCPIPv6;
    } else if (pktType == RxUDPT) {
        /* UDP packet */
        if (status2 & RxV4F)
            resultMask = (status1 & RxUDPF) ? 0 : (kChecksumUDP | kChecksumIP);
        else if (status2 & RxV6F)
            resultMask = (status1 & RxUDPF) ? 0 : kChecksumUDPIPv6;
    } else if ((pktType == 0) && (status2 & RxV4F)) {
        /* IP packet */
        resultMask = (status1 & RxIPF) ? 0 : kChecksumIP;
    }
    if (resultMask)
        setChecksumResult(m, kChecksumFamilyTCPIP, resultMask, resultMask);
}

#endif

static const char *speed25GName = "2.5 Gigabit";
static const char *speed1GName = "1 Gigabit";
static const char *speed100MName = "100 Megabit";
static const char *speed10MName = "10 Megabit";
static const char *duplexFullName = "Full-duplex";
static const char *duplexHalfName = "Half-duplex";
static const char *offFlowName = "No flow-control";
static const char *onFlowName = "flow-control";

static const char* eeeNames[kEEETypeCount] = {
    "",
    ", energy-efficient-ethernet"
};

void LucyRTL8125::setLinkUp()
{
    UInt64 mediumSpeed;
    UInt32 mediumIndex = MEDIUM_INDEX_AUTO;
    const char *speedName;
    const char *duplexName;
    const char *flowName;
    const char *eeeName;
    
    eeeName = eeeNames[kEEETypeNo];

    /* Get link speed, duplex and flow-control mode. */
    if (flowCtl == kFlowControlOn) {
        flowName = onFlowName;
    } else {
        flowName = offFlowName;
    }
    if (speed == SPEED_2500) {
        mediumSpeed = kSpeed2500MBit;
        speedName = speed25GName;
        duplexName = duplexFullName;
       
        if (flowCtl == kFlowControlOn) {
            mediumIndex = MEDIUM_INDEX_2500FDFC;
        } else {
            mediumIndex = MEDIUM_INDEX_2500FD;
        }
    } else if (speed == SPEED_1000) {
        mediumSpeed = kSpeed1000MBit;
        speedName = speed1GName;
        duplexName = duplexFullName;
       
        if (flowCtl == kFlowControlOn) {
            if (eeeMode & MDIO_EEE_1000T) {
                mediumIndex = MEDIUM_INDEX_1000FDFCEEE;
                eeeName = eeeNames[kEEETypeYes];
            } else {
                mediumIndex = MEDIUM_INDEX_1000FDFC;
            }
        } else {
            if (eeeMode & MDIO_EEE_1000T) {
                mediumIndex = MEDIUM_INDEX_1000FDEEE;
                eeeName = eeeNames[kEEETypeYes];
            } else {
                mediumIndex = MEDIUM_INDEX_1000FD;
            }
        }
    } else if (speed == SPEED_100) {
        mediumSpeed = kSpeed100MBit;
        speedName = speed100MName;
        
        if (duplex == DUPLEX_FULL) {
            duplexName = duplexFullName;
            
            if (flowCtl == kFlowControlOn) {
                if (eeeMode & MDIO_EEE_100TX) {
                    mediumIndex =  MEDIUM_INDEX_100FDFCEEE;
                    eeeName = eeeNames[kEEETypeYes];
                } else {
                    mediumIndex = MEDIUM_INDEX_100FDFC;
                }
            } else {
                if (eeeMode & MDIO_EEE_100TX) {
                    mediumIndex =  MEDIUM_INDEX_100FDEEE;
                    eeeName = eeeNames[kEEETypeYes];
                } else {
                    mediumIndex = MEDIUM_INDEX_100FD;
                }
            }
        } else {
            mediumIndex = MEDIUM_INDEX_100HD;
            duplexName = duplexHalfName;
        }
    } else {
        mediumSpeed = kSpeed10MBit;
        speedName = speed10MName;
        
        if (duplex == DUPLEX_FULL) {
            mediumIndex = MEDIUM_INDEX_10FD;
            duplexName = duplexFullName;
        } else {
            mediumIndex = MEDIUM_INDEX_10HD;
            duplexName = duplexHalfName;
        }
    }
    /* Enable receiver and transmitter. */
    WriteReg8(ChipCmd, CmdTxEnb | CmdRxEnb);

    linkUp = true;
    setLinkStatus(kIONetworkLinkValid | kIONetworkLinkActive, mediumTable[mediumIndex], mediumSpeed, NULL);

    /* Start output thread, statistics update and watchdog. Also
     * update poll params according to link speed.
     */
    bzero(&pollParams, sizeof(IONetworkPacketPollingParameters));
    
    if (speed == SPEED_10) {
        pollParams.lowThresholdPackets = 2;
        pollParams.highThresholdPackets = 8;
        pollParams.lowThresholdBytes = 0x400;
        pollParams.highThresholdBytes = 0x1800;
        pollParams.pollIntervalTime = 1000000;  /* 1ms */
    } else {
        pollParams.lowThresholdPackets = 10;
        pollParams.highThresholdPackets = 40;
        pollParams.lowThresholdBytes = 0x1000;
        pollParams.highThresholdBytes = 0x10000;
        
        if (speed == SPEED_2500)
            //pollParams.pollIntervalTime = 150000 - ((kMaxMtu - mtu) * 4);   /* 120-150µs */
            pollParams.pollIntervalTime = 140000;   /* 140µs */
        else if (speed == SPEED_1000)
            pollParams.pollIntervalTime = 170000;   /* 170µs */
        else
            pollParams.pollIntervalTime = 1000000;  /* 1ms */
    }
    netif->setPacketPollingParameters(&pollParams, 0);
    DebugLog("pollIntervalTime: %lluus\n", (pollParams.pollIntervalTime / 1000));

    netif->startOutputThread();

    IOLog("Link up on en%u, %s, %s, %s%s\n", netif->getUnitNumber(), speedName, duplexName, flowName, eeeName);
}

void LucyRTL8125::setLinkDown()
{
    deadlockWarn = 0;
    needsUpdate = false;

    /* Stop output thread and flush output queue. */
    netif->stopOutputThread();
    netif->flushOutputQueue();

    /* Update link status. */
    linkUp = false;
    setLinkStatus(kIONetworkLinkValid);

    rtl8125_nic_reset(&linuxData);

    /* Cleanup descriptor ring. */
    clearDescriptors();
    
    setPhyMedium();
    
    IOLog("Link down on en%u\n", netif->getUnitNumber());
}

void LucyRTL8125::updateStatitics()
{
    UInt32 sgColl, mlColl;
    UInt32 cmd;

    /* Check if a statistics dump has been completed. */
    if (needsUpdate && !(ReadReg32(CounterAddrLow) & CounterDump)) {
        needsUpdate = false;
        netStats->inputPackets = OSSwapLittleToHostInt64(statData->rxPackets) & 0x00000000ffffffff;
        netStats->inputErrors = OSSwapLittleToHostInt32(statData->rxErrors);
        netStats->outputPackets = OSSwapLittleToHostInt64(statData->txPackets) & 0x00000000ffffffff;
        netStats->outputErrors = OSSwapLittleToHostInt32(statData->txErrors);
        
        sgColl = OSSwapLittleToHostInt32(statData->txOneCollision);
        mlColl = OSSwapLittleToHostInt32(statData->txMultiCollision);
        netStats->collisions = sgColl + mlColl;
        
        etherStats->dot3StatsEntry.singleCollisionFrames = sgColl;
        etherStats->dot3StatsEntry.multipleCollisionFrames = mlColl;
        etherStats->dot3StatsEntry.alignmentErrors = OSSwapLittleToHostInt16(statData->alignErrors);
        etherStats->dot3StatsEntry.missedFrames = OSSwapLittleToHostInt16(statData->rxMissed);
        etherStats->dot3TxExtraEntry.underruns = OSSwapLittleToHostInt16(statData->txUnderun);
    }
    /* Some chips are unable to dump the tally counter while the receiver is disabled. */
    if (ReadReg8(ChipCmd) & CmdRxEnb) {
        WriteReg32(CounterAddrHigh, (statPhyAddr >> 32));
        cmd = (statPhyAddr & 0x00000000ffffffff);
        WriteReg32(CounterAddrLow, cmd);
        WriteReg32(CounterAddrLow, cmd | CounterDump);
        needsUpdate = true;
    }
}

void LucyRTL8125::timerActionRTL8125(IOTimerEventSource *timer)
{
    if (!linkUp) {
        DebugLog("Timer fired while link down.\n");
        goto done;
    }
    /* Check for tx deadlock. */
    if (checkForDeadlock())
        goto done;
    
    updateStatitics();
    timerSource->setTimeoutMS(kTimeoutMS);
        
done:
    txDescDoneLast = txDescDoneCount;
    
}

#pragma mark --- miscellaneous functions ---

static inline UInt32 adjustIPv6Header(mbuf_t m)
{
    struct ip6_hdr *ip6Hdr = (struct ip6_hdr *)((UInt8 *)mbuf_data(m) + ETHER_HDR_LEN);
    struct tcphdr *tcpHdr = (struct tcphdr *)((UInt8 *)ip6Hdr + sizeof(struct ip6_hdr));
    UInt32 plen = ntohs(ip6Hdr->ip6_ctlun.ip6_un1.ip6_un1_plen);
    UInt32 csum = ntohs(tcpHdr->th_sum) - plen;
    
    csum += (csum >> 16);
    ip6Hdr->ip6_ctlun.ip6_un1.ip6_un1_plen = 0;
    tcpHdr->th_sum = htons((UInt16)csum);
    
    return (plen + kMinL4HdrOffsetV6);
}

static unsigned const ethernet_polynomial = 0x04c11db7U;

static inline u32 ether_crc(int length, unsigned char *data)
{
    int crc = -1;
    
    while(--length >= 0) {
        unsigned char current_octet = *data++;
        int bit;
        for (bit = 0; bit < 8; bit++, current_octet >>= 1) {
            crc = (crc << 1) ^
            ((crc < 0) ^ (current_octet & 1) ? ethernet_polynomial : 0);
        }
    }
    return crc;
}
