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

static inline void prepareTSO4(mbuf_t m, UInt32 *tcpOffset, UInt32 *mss);
static inline void prepareTSO6(mbuf_t m, UInt32 *tcpOffset, UInt32 *mss);

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

        /* Initialize state flags. */
        stateFlags = 0;
        
        mtu = ETH_DATA_LEN;
        powerState = 0;
        speed = 0;
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
        pollInterval2500 = 0;
        wolCapable = false;
        wolActive = false;
        enableTSO4 = false;
        enableTSO6 = false;
        enableCSO6 = false;
        pciPMCtrlOffset = 0;
        memset(fallBackMacAddr.bytes, 0, kIOEthernetAddressSize);
        
        lastRxIntrupts = lastTxIntrupts = lastTmrIntrupts = tmrInterrupts = 0;
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
    freeTxResources();
    freeRxResources();
    freeStatResources();
    
    DebugLog("free() <===\n");
    
    super::free();
}

bool LucyRTL8125::start(IOService *provider)
{
    bool result;
    
    result = super::start(provider);
    
    if (!result) {
        IOLog("IOEthernetController::start failed.\n");
        goto done;
    }
    clear_mask((__M_CAST_M | __PROMISC_M), &stateFlags);
    multicastFilter = 0;

    pciDevice = OSDynamicCast(IOPCIDevice, provider);
    
    if (!pciDevice) {
        IOLog("No provider.\n");
        goto done;
    }
    pciDevice->retain();
    
    if (!pciDevice->open(this)) {
        IOLog("Failed to open provider.\n");
        goto error_open;
    }
    getParams();
    
    if (!initPCIConfigSpace(pciDevice)) {
        goto error_cfg;
    }

    if (!initRTL8125()) {
        IOLog("Failed to initialize chip.\n");
        goto error_cfg;
    }
    
    if (!setupMediumDict()) {
        IOLog("Failed to setup medium dictionary.\n");
        goto error_cfg;
    }
    commandGate = getCommandGate();
    
    if (!commandGate) {
        IOLog("getCommandGate() failed.\n");
        goto error_cfg;
    }
    commandGate->retain();
    
    if (!setupTxResources()) {
        IOLog("Error allocating Tx resources.\n");
        goto error_dma1;
    }

    if (!setupRxResources()) {
        IOLog("Error allocating Rx resources.\n");
        goto error_dma2;
    }

    if (!setupStatResources()) {
        IOLog("Error allocating Stat resources.\n");
        goto error_dma3;
    }

    if (!initEventSources(provider)) {
        IOLog("initEventSources() failed.\n");
        goto error_src;
    }
    
    result = attachInterface(reinterpret_cast<IONetworkInterface**>(&netif));

    if (!result) {
        IOLog("attachInterface() failed.\n");
        goto error_src;
    }
    pciDevice->close(this);
    result = true;
    
done:
    return result;

error_src:
    freeStatResources();

error_dma3:
    freeRxResources();

error_dma2:
    freeTxResources();
    
error_dma1:
    RELEASE(commandGate);
        
error_cfg:
    pciDevice->close(this);
    
error_open:
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

    freeStatResources();
    freeRxResources();
    freeTxResources();

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

    if (test_bit(__ENABLED, &stateFlags)) {
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

    txDescDoneCount = txDescDoneLast = 0;
    deadlockWarn = 0;
    needsUpdate = false;
    set_bit(__ENABLED, &stateFlags);
    clear_bit(__POLL_MODE, &stateFlags);

    result = kIOReturnSuccess;
    
    DebugLog("enable() <===\n");

done:
    return result;
}

IOReturn LucyRTL8125::disable(IONetworkInterface *netif)
{
    UInt64 timeout;
    UInt64 delay;
    UInt64 now;
    UInt64 t;

    DebugLog("disable() ===>\n");
    
    if (!test_bit(__ENABLED, &stateFlags))
        goto done;
    
    netif->stopOutputThread();
    netif->flushOutputQueue();
    
    if (test_bit(__POLLING, &stateFlags)) {
        nanoseconds_to_absolutetime(5000, &delay);
        clock_get_uptime(&now);
        timeout = delay * 10;
        t = delay;

        while (test_bit(__POLLING, &stateFlags) && (t < timeout)) {
            clock_delay_until(now + t);
            t += delay;
        }
    }
    clear_mask((__ENABLED_M | __LINK_UP_M | __POLL_MODE_M | __POLLING_M), &stateFlags);

    timerSource->cancelTimeout();
    needsUpdate = false;
    txDescDoneCount = txDescDoneLast = 0;

    /* Disable interrupt as we are using msi. */
    interruptSource->disable();

    disableRTL8125();
    
    clearRxTxRings();
    
    if (pciDevice && pciDevice->isOpen())
        pciDevice->close(this);
        
    DebugLog("disable() <===\n");
    
done:
    return kIOReturnSuccess;
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
    mbuf_csum_request_flags_t csum;
    UInt32 mss;
    UInt32 tcpOff;
    UInt32 opts1;
    UInt32 vlanTag;
    UInt32 numSegs;
    UInt32 lastSeg;
    UInt32 index;
    UInt32 i;
    
    //DebugLog("outputStart() ===>\n");
    
    if (!(test_mask((__ENABLED_M | __LINK_UP_M), &stateFlags)))  {
        DebugLog("Interface down. Dropping packets.\n");
        goto done;
    }
    while ((txNumFreeDesc > (kMaxSegs + 3)) && (interface->dequeueOutputPackets(1, &m, NULL, NULL, NULL) == kIOReturnSuccess)) {
        cmd = 0;
        opts2 = 0;

        if (mbuf_get_tso_requested(m, &tsoFlags, &mss)) {
            DebugLog("mbuf_get_tso_requested() failed. Dropping packet.\n");
            freePacket(m);
            continue;
        }
        if (tsoFlags & (MBUF_TSO_IPV4 | MBUF_TSO_IPV6)) {
            if (tsoFlags & MBUF_TSO_IPV4) {
                prepareTSO4(m, &tcpOff, &mss);
                
                cmd = (GiantSendv4 | (tcpOff << GTTCPHO_SHIFT));
                opts2 = ((mss & MSSMask) << MSSShift_8125);
            } else {
                /* The pseudoheader checksum has to be adjusted first. */
                prepareTSO6(m, &tcpOff, &mss);
                
                cmd = (GiantSendv6 | (tcpOff << GTTCPHO_SHIFT));
                opts2 = ((mss & MSSMask) << MSSShift_8125);
            }
        } else {
            /* We use mss as a dummy here because it isn't needed anymore. */
            mbuf_get_csum_requested(m, &csum, &mss);
            
            if (csum & kChecksumTCP)
                opts2 = (TxIPCS_C | TxTCPCS_C);
            else if (csum & kChecksumTCPIPv6)
                opts2 = (TxTCPCS_C | TxIPV6F_C | (((kMacHdrLen + kIPv6HdrLen) & TCPHO_MAX) << TCPHO_SHIFT));
            else if (csum & kChecksumUDP)
                opts2 = (TxIPCS_C | TxUDPCS_C);
            else if (csum & kChecksumUDPIPv6)
                opts2 = (TxUDPCS_C | TxIPV6F_C | (((kMacHdrLen + kIPv6HdrLen) & TCPHO_MAX) << TCPHO_SHIFT));
            else if (csum & kChecksumIP)
                opts2 = TxIPCS_C;
        }
        /* Finally get the physical segments. */
        numSegs = txMbufCursor->getPhysicalSegmentsWithCoalesce(m, &txSegments[0], kMaxSegs);

        /* Alloc required number of descriptors. As the descriptor
         * which has been freed last must be considered to be still
         * in use we never fill the ring completely but leave at
         * least one unused.
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
    /* Update tail pointer. */
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
    rxMode |= rxConfigReg | (ReadReg32(RxConfig) & rxConfigMask);
    WriteReg32(RxConfig, rxMode);
    WriteReg32(MAR0, mcFilter[0]);
    WriteReg32(MAR1, mcFilter[1]);

    if (active)
        set_bit(__PROMISC, &stateFlags);
    else
        clear_bit(__PROMISC, &stateFlags);

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
    rxMode |= rxConfigReg | (ReadReg32(RxConfig) & rxConfigMask);
    WriteReg32(RxConfig, rxMode);
    WriteReg32(MAR0, mcFilter[0]);
    WriteReg32(MAR1, mcFilter[1]);
    
    if (active)
        set_bit(__M_CAST, &stateFlags);
    else
        clear_bit(__M_CAST, &stateFlags);

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
        autoneg = AUTONEG_DISABLE;
        flowCtl = kFlowControlOff;
        linuxData.eee_adv_t = 0;
        
        switch (medium->getIndex()) {
            case MEDIUM_INDEX_AUTO:
                autoneg = AUTONEG_ENABLE;
                speed = 0;
                duplex = DUPLEX_FULL;
                //linuxData.eee_adv_t = eeeCap;
                break;
                
            case MEDIUM_INDEX_10HD:
                speed = SPEED_10;
                duplex = DUPLEX_HALF;
                break;
                
            case MEDIUM_INDEX_10FD:
                speed = SPEED_10;
                duplex = DUPLEX_FULL;
                break;
                
            case MEDIUM_INDEX_100HD:
                speed = SPEED_100;
                duplex = DUPLEX_HALF;
                break;
                
            case MEDIUM_INDEX_100FD:
                speed = SPEED_100;
                duplex = DUPLEX_FULL;
                break;
                
            case MEDIUM_INDEX_100FDFC:
                speed = SPEED_100;
                duplex = DUPLEX_FULL;
                flowCtl = kFlowControlOn;
                break;
                
            case MEDIUM_INDEX_1000FD:
                speed = SPEED_1000;
                duplex = DUPLEX_FULL;
                break;
                
            case MEDIUM_INDEX_1000FDFC:
                speed = SPEED_1000;
                duplex = DUPLEX_FULL;
                flowCtl = kFlowControlOn;
                break;
                
            case MEDIUM_INDEX_100FDEEE:
                speed = SPEED_100;
                duplex = DUPLEX_FULL;
                linuxData.eee_adv_t = eeeCap;
                break;
                
            case MEDIUM_INDEX_100FDFCEEE:
                speed = SPEED_100;
                duplex = DUPLEX_FULL;
                flowCtl = kFlowControlOn;
                linuxData.eee_adv_t = eeeCap;
                break;
                
            case MEDIUM_INDEX_1000FDEEE:
                speed = SPEED_1000;
                duplex = DUPLEX_FULL;
                linuxData.eee_adv_t = eeeCap;
                break;
                
            case MEDIUM_INDEX_1000FDFCEEE:
                speed = SPEED_1000;
                duplex = DUPLEX_FULL;
                flowCtl = kFlowControlOn;
                linuxData.eee_adv_t = eeeCap;
                break;
                
            case MEDIUM_INDEX_2500FD:
                speed = SPEED_2500;
                duplex = DUPLEX_FULL;
                break;
                
            case MEDIUM_INDEX_2500FDFC:
                speed = SPEED_2500;
                duplex = DUPLEX_FULL;
                flowCtl = kFlowControlOn;
                break;                
        }
        //setPhyMedium();
        setCurrentMedium(medium);
        setLinkDown();
    }
    
    DebugLog("selectMedium() <===\n");
    
done:
    return result;
}

IOReturn LucyRTL8125::getMaxPacketSize(UInt32 * maxSize) const
{
    DebugLog("getMaxPacketSize() ===>\n");
    
    if (version_major >= 22) {
        /*
         * Starting with Ventura we can be honest about jumbo
         * frame support.
         */
        *maxSize = kRxBufferPktSize - 2;
    } else {
        /*
         * In case we reported a maximum packet size below 9018
         * the network preferences panel wouldn't allow the user
         * to set an MTU above 1500 which would disable jumbo
         * frame support completely. Therefore we fake a maximum
         * packet size of 9018 although trying to set anything
         * above 4076 in setMaxPacketSize() will fail. This is
         * ugly but the only solution.
         */
        *maxSize = kMaxPacketSize;
    }
    DebugLog("maxSize: %u, version_major: %u\n", *maxSize, version_major);

    DebugLog("getMaxPacketSize() <===\n");
    
    return kIOReturnSuccess;
}

IOReturn LucyRTL8125::setMaxPacketSize(UInt32 maxSize)
{
    ifnet_t ifnet = netif->getIfnet();
    ifnet_offload_t offload;
    UInt32 mask = (IFNET_CSUM_IP | IFNET_CSUM_TCP | IFNET_CSUM_UDP);
    IOReturn result = kIOReturnError;

    DebugLog("setMaxPacketSize() ===>\n");
    
    if (maxSize <= (kRxBufferPktSize - 2)) {
        mtu = maxSize - (ETH_HLEN + ETH_FCS_LEN);

        DebugLog("maxSize: %u, mtu: %u\n", maxSize, mtu);
        
        if (enableTSO4)
            mask |= IFNET_TSO_IPV4;
        
        if (enableTSO6)
            mask |= IFNET_TSO_IPV6;

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
        restartRTL8125();

        result = kIOReturnSuccess;
    }
    
    DebugLog("setMaxPacketSize() <===\n");
    
    return result;
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
        
        /* As we don't support fragmented packets we treat them as errors. */
        if (unlikely((descStatus1 & (FirstFrag|LastFrag)) != (FirstFrag|LastFrag))) {
            DebugLog("Fragmented packet.\n");
            etherStats->dot3StatsEntry.frameTooLongs++;
            opts1 |= kRxBufferPktSize;
            goto nextDesc;
        }
        
        /* Drop packets with receive errors. */
        if (unlikely(descStatus1 & RxRES)) {
            DebugLog("Rx error.\n");
            
            if (descStatus1 & (RxRWT | RxRUNT))
                etherStats->dot3StatsEntry.frameTooLongs++;

            if (descStatus1 & RxCRC)
                etherStats->dot3StatsEntry.fcsErrors++;

            opts1 |= kRxBufferPktSize;
            goto nextDesc;
        }
        
        descStatus2 = OSSwapLittleToHostInt32(desc->opts2);
        pktSize = (descStatus1 & 0x1fff) - kIOEthernetCRCSize;
        bufPkt = rxMbufArray[rxNextDescIndex];
        //DebugLog("rxInterrupt(): descStatus1=0x%x, descStatus2=0x%x, pktSize=%u\n", descStatus1, descStatus2, pktSize);
        
        newPkt = replaceOrCopyPacket(&bufPkt, pktSize, &replaced);
        
        if (unlikely(!newPkt)) {
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

        getChecksumResult(newPkt, descStatus1, descStatus2);

        /* Also get the VLAN tag if there is any. */
        if (descStatus2 & RxVlanTag)
            setVlanTag(newPkt, OSSwapInt16(descStatus2 & 0xffff));

        mbuf_pkthdr_setlen(newPkt, pktSize);
        interface->enqueueInputPacket(newPkt, pollQueue);
        goodPkts++;
        
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
    UInt16 currLinkState;
    
    DebugLog("Link change interrupt: Check link status.\n");

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
        } else if (currLinkState & _1000bpsF) {
                speed = SPEED_1000;
                duplex = DUPLEX_FULL;
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
        setupRTL8125();
        
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

void LucyRTL8125::interruptHandler(OSObject *client, IOInterruptEventSource *src, int count)
{
    UInt32 packets;
    UInt32 status;
    
    status = ReadReg32(ISR0_8125);
    
    //DebugLog("interruptHandler: status = 0x%x.\n", status);

    /* hotplug/major error/no more work/shared irq */
    if ((status == 0xFFFFFFFF) || !status)
        goto done;
    
    WriteReg32(IMR0_8125, 0x0000);
    WriteReg32(ISR0_8125, (status & ~RxFIFOOver));

    if (status & SYSErr) {
        pciErrorInterrupt();
        goto done;
    }
    if (!test_bit(__POLL_MODE, &stateFlags) &&
        !test_and_set_bit(__POLLING, &stateFlags)) {
        /* Rx interrupt */
        if (status & (RxOK | RxDescUnavail)) {
            packets = rxInterrupt(netif, kNumRxDesc, NULL, NULL);
            
            if (packets)
                netif->flushInputQueue();
            
            etherStats->dot3RxExtraEntry.interrupts++;
        }
        /* Tx interrupt */
        if (status & (TxOK | RxOK | PCSTimeout)) {
            txInterrupt();
            
            if (status & TxOK)
                etherStats->dot3TxExtraEntry.interrupts++;
        }
        if ((status & (TxOK | RxOK)) || (keepIntrCnt > 0)) {
            if (status & (TxOK | RxOK))
                keepIntrCnt = RTK_KEEP_INTERRUPT_COUNT;
            else
                keepIntrCnt--;
            
            WriteReg32(TIMER_INT0_8125, 0x2600);
            WriteReg32(TCTR0_8125, 0x2600);
            intrMask = intrMaskTimer;
        } else {
            WriteReg32(TIMER_INT0_8125, 0x0000);
            keepIntrCnt = 0;
            intrMask = intrMaskRxTx;
        }
        if (status & PCSTimeout)
            tmrInterrupts++;

        clear_bit(__POLLING, &stateFlags);
    }
    if (status & LinkChg) {
        checkLinkStatus();
        WriteReg32(TIMER_INT0_8125, 0x000);
        keepIntrCnt = 0;
        intrMask = intrMaskRxTx;
    }
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
            DebugLog("Warning: Tx timeout, ISR0=0x%x, IMR0=0x%x, polling=%u.\n", ReadReg32(ISR0_8125),
                     ReadReg32(IMR0_8125), test_bit(__POLL_MODE, &stateFlags));
            etherStats->dot3TxExtraEntry.timeouts++;
            txInterrupt();
        } else if (deadlockWarn >= kTxDeadlockTreshhold) {
#ifdef DEBUG
            UInt32 i, index;
            
            for (i = 0; i < 10; i++) {
                index = ((txDirtyDescIndex - 1 + i) & kTxDescMask);
                IOLog("desc[%u]: opts1=0x%x, opts2=0x%x, addr=0x%llx.\n", index,
                      txDescArray[index].opts1, txDescArray[index].opts2, txDescArray[index].addr);
            }
#endif
            IOLog("Tx stalled? Resetting chipset. ISR0=0x%x, IMR0=0x%x.\n", ReadReg32(ISR0_8125),
                  ReadReg32(IMR0_8125));
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

    if (test_bit(__ENABLED, &stateFlags)) {
        if (enabled) {
            set_bit(__POLL_MODE, &stateFlags);

            intrMask = intrMaskPoll;
        } else {
            clear_bit(__POLL_MODE, &stateFlags);

            intrMask = intrMaskRxTx;
        }
        WriteReg32(IMR0_8125, intrMask);
    }
    DebugLog("Input polling %s.\n", enabled ? "enabled" : "disabled");

    //DebugLog("setInputPacketPollingEnable() <===\n");
    
    return kIOReturnSuccess;
}

void LucyRTL8125::pollInputPackets(IONetworkInterface *interface, uint32_t maxCount, IOMbufQueue *pollQueue, void *context )
{
    //DebugLog("pollInputPackets() ===>\n");
    
    if (test_bit(__POLL_MODE, &stateFlags) &&
        !test_and_set_bit(__POLLING, &stateFlags)) {

        rxInterrupt(interface, maxCount, pollQueue, context);
        
        /* Finally cleanup the transmitter ring. */
        txInterrupt();
        
        clear_bit(__POLLING, &stateFlags);
    }
    //DebugLog("pollInputPackets() <===\n");
}

#pragma mark --- hardware specific methods ---

#if 0
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

inline void LucyRTL8125::getChecksumResult(mbuf_t m, UInt32 status1, UInt32 status2)
{
    mbuf_csum_performed_flags_t performed = 0;
    UInt32 value = 0;

    if ((status2 & RxV4F) && !(status1 & RxIPF))
        performed |= (MBUF_CSUM_DID_IP | MBUF_CSUM_IP_GOOD);

    if (((status1 & RxTCPT) && !(status1 & RxTCPF)) ||
        ((status1 & RxUDPT) && !(status1 & RxUDPF))) {
        performed |= (MBUF_CSUM_DID_DATA | MBUF_CSUM_PSEUDO_HDR);
        value = 0xffff; // fake a valid checksum value
    }
    if (performed)
        mbuf_set_csum_performed(m, performed, value);
}

static const char *speed25GName = "2.5 Gigabit";
static const char *speed1GName = "1 Gigabit";
static const char *speed100MName = "100 Megabit";
static const char *speed10MName = "10 Megabit";
static const char *duplexFullName = "full-duplex";
static const char *duplexHalfName = "half-duplex";
static const char *offFlowName = "no flow-control";
static const char *onFlowName = "flow-control";

static const char* eeeNames[kEEETypeCount] = {
    "",
    ", energy-efficient-ethernet"
};

void LucyRTL8125::setLinkUp()
{
    IONetworkPacketPollingParameters pParams;
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

    set_bit(__LINK_UP, &stateFlags);
    setLinkStatus(kIONetworkLinkValid | kIONetworkLinkActive, mediumTable[mediumIndex], mediumSpeed, NULL);

    /* Start output thread, statistics update and watchdog. Also
     * update poll params according to link speed.
     */
    bzero(&pParams, sizeof(IONetworkPacketPollingParameters));
    
    if (speed == SPEED_10) {
        pParams.lowThresholdPackets = 2;
        pParams.highThresholdPackets = 8;
        pParams.lowThresholdBytes = 0x400;
        pParams.highThresholdBytes = 0x1800;
        pParams.pollIntervalTime = 1000000;  /* 1ms */
    } else {
        pParams.lowThresholdPackets = 10;
        pParams.highThresholdPackets = 40;
        pParams.lowThresholdBytes = 0x1000;
        pParams.highThresholdBytes = 0x10000;
        
        if (speed == SPEED_2500)
            if (pollInterval2500) {
                /*
                 * Use fixed polling interval taken from usPollInt2500.
                 */
                pParams.pollIntervalTime = pollInterval2500;
            } else {
                /*
                 * Setting usPollInt2500 to 0 enables use of an
                 * adaptive polling interval based on mtu vale.
                 */
                pParams.pollIntervalTime = ((mtu * 100) / 20) + 135000;
            }
        else if (speed == SPEED_1000)
            pParams.pollIntervalTime = 170000;   /* 170µs */
        else
            pParams.pollIntervalTime = 1000000;  /* 1ms */
    }
    netif->setPacketPollingParameters(&pParams, 0);
    DebugLog("pollIntervalTime: %lluus\n", (pParams.pollIntervalTime / 1000));

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
    clear_mask((__LINK_UP_M | __POLL_MODE_M), &stateFlags);
    setLinkStatus(kIONetworkLinkValid);

    rtl8125_nic_reset(&linuxData);

    /* Cleanup descriptor ring. */
    clearRxTxRings();
    
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
    if (test_bit(__LINK_UP, &stateFlags) && (ReadReg8(ChipCmd) & CmdRxEnb)) {
        WriteReg32(CounterAddrHigh, (statPhyAddr >> 32));
        cmd = (statPhyAddr & 0x00000000ffffffff);
        WriteReg32(CounterAddrLow, cmd);
        WriteReg32(CounterAddrLow, cmd | CounterDump);
        needsUpdate = true;
    }
}

void LucyRTL8125::timerActionRTL8125(IOTimerEventSource *timer)
{
    UInt32 rxIntr = etherStats->dot3RxExtraEntry.interrupts - lastRxIntrupts;
    UInt32 txIntr = etherStats->dot3TxExtraEntry.interrupts - lastTxIntrupts;
    UInt32 tmrIntr = tmrInterrupts - lastTmrIntrupts;

    lastRxIntrupts = etherStats->dot3RxExtraEntry.interrupts;
    lastTxIntrupts = etherStats->dot3TxExtraEntry.interrupts;
    lastTmrIntrupts = tmrInterrupts;
    
    DebugLog("rxIntr/s: %u, txIntr/s: %u, timerIntr/s: %u\n", rxIntr, txIntr, tmrIntr);

    updateStatitics();

    if (!test_bit(__LINK_UP, &stateFlags))
        goto done;

    /* Check for tx deadlock. */
    if (checkForDeadlock())
        goto done;
    
    timerSource->setTimeoutMS(kTimeoutMS);
        
done:
    txDescDoneLast = txDescDoneCount;
    
}

#pragma mark --- miscellaneous functions ---

static inline void prepareTSO4(mbuf_t m, UInt32 *tcpOffset, UInt32 *mss)
{
    UInt8 *p = (UInt8 *)mbuf_data(m) + kMacHdrLen;
    struct ip4_hdr_be *ip = (struct ip4_hdr_be *)p;
    struct tcp_hdr_be *tcp;
    UInt32 csum32 = 6;
    //UInt32 max;
    UInt32 i, il, tl;
    
    for (i = 0; i < 4; i++) {
        csum32 += ntohs(ip->addr[i]);
        csum32 += (csum32 >> 16);
        csum32 &= 0xffff;
    }
    il = ((ip->hdr_len & 0x0f) << 2);
    
    tcp = (struct tcp_hdr_be *)(p + il);
    tl = ((tcp->dat_off & 0xf0) >> 2);
    //max = ETH_DATA_LEN - (il + tl);

    /* Fill in the pseudo header checksum for TSOv4. */
    tcp->csum = htons((UInt16)csum32);

    *tcpOffset = kMacHdrLen + il;
    
    if (*mss > MSS_MAX)
        *mss = MSS_MAX;
}

static inline void prepareTSO6(mbuf_t m, UInt32 *tcpOffset, UInt32 *mss)
{
    UInt8 *p = (UInt8 *)mbuf_data(m) + kMacHdrLen;
    struct ip6_hdr_be *ip6 = (struct ip6_hdr_be *)p;
    struct tcp_hdr_be *tcp;
    UInt32 csum32 = 6;
    UInt32 i, tl;
    //UInt32 max;

    ip6->pay_len = 0;

    for (i = 0; i < 16; i++) {
        csum32 += ntohs(ip6->addr[i]);
        csum32 += (csum32 >> 16);
        csum32 &= 0xffff;
    }
    /* Get the length of the TCP header. */
    tcp = (struct tcp_hdr_be *)(p + kIPv6HdrLen);
    tl = ((tcp->dat_off & 0xf0) >> 2);
    //max = ETH_DATA_LEN - (kIPv6HdrLen + tl);

    /* Fill in the pseudo header checksum for TSOv6. */
    tcp->csum = htons((UInt16)csum32);

    *tcpOffset = kMacHdrLen + kIPv6HdrLen;
    
    if (*mss > MSS_MAX)
        *mss = MSS_MAX;
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
