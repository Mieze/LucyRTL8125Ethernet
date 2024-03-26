/* LucyRTL8125Setup.cpp -- RTL8125 data structure initialzation methods.
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

static const char *onName = "enabled";
static const char *offName = "disabled";


#pragma mark --- data structure initialization methods ---

void LucyRTL8125::getParams()
{
    OSDictionary *params;
    OSNumber *pollInt;
    OSBoolean *enableEEE;
    OSBoolean *tso4;
    OSBoolean *tso6;
    OSBoolean *csoV6;
    OSBoolean *noASPM;
    OSString *versionString;
    OSString *fbAddr;
    UInt32 usInterval;
    
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
        
        pollInt = OSDynamicCast(OSNumber, params->getObject(kPollInt2500Name));

        if (pollInt) {
            usInterval = pollInt->unsigned32BitValue();
            
            if (usInterval > 200)
                pollInterval2500 = 0;
            else if (usInterval < 100)
                pollInterval2500 = 0;
            else
                pollInterval2500 = usInterval * 1000;
        } else {
            pollInterval2500 = 0;
        }
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
        pollInterval2500 = 0;
    }
    if (versionString)
        IOLog("LucyRTL8125Ethernet version %s starting. Please don't support tonymacx86.com!\n", versionString->getCStringNoCopy());
    else
        IOLog("LucyRTL8125Ethernet starting. Please don't support tonymacx86.com!\n");
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
        
        interruptSource = IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventSource::Action, this, &LucyRTL8125::interruptHandler), provider, msiIndex);
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

bool LucyRTL8125::setupRxResources()
{
    IOPhysicalSegment rxSegment;
    IODMACommand::Segment64 seg;
    mbuf_t spareMbuf[kRxNumSpareMbufs];
    mbuf_t m;
    UInt64 offset = 0;
    UInt32 numSegs = 1;
    UInt32 i;
    UInt32 opts1;
    bool result = false;
    
    /* Create receiver descriptor array. */
    rxBufDesc = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(kernel_task, (kIODirectionInOut | kIOMemoryPhysicallyContiguous | kIOMapInhibitCache), kRxDescSize, 0xFFFFFFFFFFFFFF00ULL);
    
    if (!rxBufDesc) {
        IOLog("Couldn't alloc rxBufDesc.\n");
        goto done;
    }
    if (rxBufDesc->prepare() != kIOReturnSuccess) {
        IOLog("rxBufDesc->prepare() failed.\n");
        goto error_prep;
    }
    rxDescArray = (RtlRxDesc *)rxBufDesc->getBytesNoCopy();

    rxDescDmaCmd = IODMACommand::withSpecification(kIODMACommandOutputHost64, 64, 0, IODMACommand::kMapped, 0, 1);
    
    if (!rxDescDmaCmd) {
        IOLog("Couldn't alloc rxDescDmaCmd.\n");
        goto error_dma;
    }
    
    if (rxDescDmaCmd->setMemoryDescriptor(rxBufDesc) != kIOReturnSuccess) {
        IOLog("setMemoryDescriptor() failed.\n");
        goto error_set_desc;
    }
    
    if (rxDescDmaCmd->gen64IOVMSegments(&offset, &seg, &numSegs) != kIOReturnSuccess) {
        IOLog("gen64IOVMSegments() failed.\n");
        goto error_segm;
    }
    /* And the rx ring's physical address too. */
    rxPhyAddr = seg.fIOVMAddr;
    
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
        goto error_segm;
    }

    /* Alloc receive buffers. */
    for (i = 0; i < kNumRxDesc; i++) {
        m = allocatePacket(kRxBufferPktSize);
        
        if (!m) {
            IOLog("Couldn't alloc receive buffer.\n");
            goto error_buf;
        }
        rxMbufArray[i] = m;
        
        if (rxMbufCursor->getPhysicalSegments(m, &rxSegment, 1) != 1) {
            IOLog("getPhysicalSegments() for receive buffer failed.\n");
            goto error_buf;
        }
        opts1 = (UInt32)rxSegment.length;
        opts1 |= (i == kRxLastDesc) ? (RingEnd | DescOwn) : DescOwn;
        rxDescArray[i].opts1 = OSSwapHostToLittleInt32(opts1);
        rxDescArray[i].opts2 = 0;
        rxDescArray[i].addr = OSSwapHostToLittleInt64(rxSegment.location);
    }

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
    
error_buf:
    for (i = 0; i < kNumRxDesc; i++) {
        if (rxMbufArray[i]) {
            freePacket(rxMbufArray[i]);
            rxMbufArray[i] = NULL;
        }
    }
    RELEASE(rxMbufCursor);

error_segm:
    rxDescDmaCmd->clearMemoryDescriptor();

error_set_desc:
    RELEASE(rxDescDmaCmd);

error_dma:
    rxBufDesc->complete();
    
error_prep:
    RELEASE(rxBufDesc);

    goto done;
}

bool LucyRTL8125::setupTxResources()
{
    IODMACommand::Segment64 seg;
    UInt64 offset = 0;
    UInt32 numSegs = 1;
    UInt32 i;
    bool result = false;
    
    /* Create transmitter descriptor array. */
    txBufDesc = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(kernel_task, (kIODirectionInOut | kIOMemoryPhysicallyContiguous | kIOMapInhibitCache), kTxDescSize, 0xFFFFFFFFFFFFFF00ULL);
            
    if (!txBufDesc) {
        IOLog("Couldn't alloc txBufDesc.\n");
        goto done;
    }
    if (txBufDesc->prepare() != kIOReturnSuccess) {
        IOLog("txBufDesc->prepare() failed.\n");
        goto error_prep;
    }
    txDescArray = (RtlTxDesc *)txBufDesc->getBytesNoCopy();
    
    txDescDmaCmd = IODMACommand::withSpecification(kIODMACommandOutputHost64, 64, 0, IODMACommand::kMapped, 0, 1);
    
    if (!txDescDmaCmd) {
        IOLog("Couldn't alloc txDescDmaCmd.\n");
        goto error_dma;
    }
    
    if (txDescDmaCmd->setMemoryDescriptor(txBufDesc) != kIOReturnSuccess) {
        IOLog("setMemoryDescriptor() failed.\n");
        goto error_set_desc;
    }
    
    if (txDescDmaCmd->gen64IOVMSegments(&offset, &seg, &numSegs) != kIOReturnSuccess) {
        IOLog("gen64IOVMSegments() failed.\n");
        goto error_segm;
    }
    /* Now get tx ring's physical address. */
    txPhyAddr = seg.fIOVMAddr;
    
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
        goto error_segm;
    }
    result = true;
    
done:
    return result;
    
error_segm:
    txDescDmaCmd->clearMemoryDescriptor();

error_set_desc:
    RELEASE(txDescDmaCmd);
    
error_dma:
    txBufDesc->complete();

error_prep:
    RELEASE(txBufDesc);
    goto done;
}

bool LucyRTL8125::setupStatResources()
{
    IODMACommand::Segment64 seg;
    UInt64 offset = 0;
    UInt32 numSegs = 1;
    bool result = false;

    /* Create statistics dump buffer. */
    statBufDesc = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(kernel_task, (kIODirectionIn | kIOMemoryPhysicallyContiguous | kIOMapInhibitCache), sizeof(RtlStatData), 0xFFFFFFFFFFFFFF00ULL);
    
    if (!statBufDesc) {
        IOLog("Couldn't alloc statBufDesc.\n");
        goto done;
    }
    
    if (statBufDesc->prepare() != kIOReturnSuccess) {
        IOLog("statBufDesc->prepare() failed.\n");
        goto error_prep;
    }
    statData = (RtlStatData *)statBufDesc->getBytesNoCopy();
    
    statDescDmaCmd = IODMACommand::withSpecification(kIODMACommandOutputHost64, 64, 0, IODMACommand::kMapped, 0, 1);
    
    if (!statDescDmaCmd) {
        IOLog("Couldn't alloc statDescDmaCmd.\n");
        goto error_dma;
    }
    
    if (statDescDmaCmd->setMemoryDescriptor(statBufDesc) != kIOReturnSuccess) {
        IOLog("setMemoryDescriptor() failed.\n");
        goto error_set_desc;
    }
    
    if (statDescDmaCmd->gen64IOVMSegments(&offset, &seg, &numSegs) != kIOReturnSuccess) {
        IOLog("gen64IOVMSegments() failed.\n");
        goto error_segm;
    }
    /* And the rx ring's physical address too. */
    statPhyAddr = seg.fIOVMAddr;
    
    /* Initialize statData. */
    bzero(statData, sizeof(RtlStatData));

    result = true;
    
done:
    return result;

error_segm:
    statDescDmaCmd->clearMemoryDescriptor();

error_set_desc:
    RELEASE(statDescDmaCmd);
    
error_dma:
    statBufDesc->complete();

error_prep:
    RELEASE(statBufDesc);
    goto done;
}

void LucyRTL8125::freeRxResources()
{
    UInt32 i;
        
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

void LucyRTL8125::freeTxResources()
{
    if (txBufDesc) {
        txBufDesc->complete();
        txBufDesc->release();
        txBufDesc = NULL;
        txPhyAddr = (IOPhysicalAddress64)NULL;
    }
    if (txDescDmaCmd) {
        txDescDmaCmd->clearMemoryDescriptor();
        txDescDmaCmd->release();
        txDescDmaCmd = NULL;
    }
    RELEASE(txMbufCursor);
}

void LucyRTL8125::freeStatResources()
{
    if (statBufDesc) {
        statBufDesc->complete();
        statBufDesc->release();
        statBufDesc = NULL;
        statPhyAddr = (IOPhysicalAddress64)NULL;
    }
    if (statDescDmaCmd) {
        statDescDmaCmd->clearMemoryDescriptor();
        statDescDmaCmd->release();
        statDescDmaCmd = NULL;
    }
}

void LucyRTL8125::clearRxTxRings()
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
    
    DebugLog("clearDescriptors() <===\n");
}
