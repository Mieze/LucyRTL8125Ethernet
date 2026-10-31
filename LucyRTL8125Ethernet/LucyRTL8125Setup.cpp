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

#define ADV_ALL (ADVERTISED_10baseT_Half | ADVERTISED_10baseT_Full | ADVERTISED_100baseT_Half | ADVERTISED_100baseT_Full | ADVERTISED_1000baseT_Full)

struct rtlMediumTable mediumArray[MIDX_COUNT] = {
    { .type = kIOMediumEthernetAuto, .spd = 0, .idx = MIDX_AUTO, .speed = 0, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_none, .eee = kEEETypeNo, .adv = ADV_ALL },

    /* 10Base-T */
    { .type = (kIOMediumEthernet10BaseT | IFM_HDX), .spd = kSpeed10MBit, .idx = MIDX_10HD, .speed = SPEED_10, .duplex = DUPLEX_HALF, .fc = rtl8125_fc_none, .eee = kEEETypeNo, .adv = ADVERTISED_10baseT_Half },
    { .type = (kIOMediumEthernet10BaseT | IFM_FDX), .spd = kSpeed10MBit, .idx = MIDX_10FD, .speed = SPEED_10, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_none, .eee = kEEETypeNo, .adv = ADVERTISED_10baseT_Full },

    /* 100Base-T */
    { .type = (kIOMediumEthernet100BaseTX | IFM_HDX), .spd = kSpeed100MBit, .idx = MIDX_100HD, .speed = SPEED_100, .duplex = DUPLEX_HALF, .fc = rtl8125_fc_none, .eee = kEEETypeNo, .adv = ADVERTISED_100baseT_Half },
    { .type = (kIOMediumEthernet100BaseTX | IFM_FDX), .spd = kSpeed100MBit, .idx = MIDX_100FD, .speed = SPEED_100, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_none, .eee = kEEETypeNo, .adv = ADVERTISED_100baseT_Full },
    { .type = (kIOMediumEthernet100BaseTX | IFM_FDX | IFM_FLOW), .spd = kSpeed100MBit, .idx = MIDX_100FDFC, .speed = SPEED_100, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_full, .eee = kEEETypeNo, .adv = ADVERTISED_100baseT_Full },
    { .type = (kIOMediumEthernet100BaseTX | IFM_FDX | IFM_EEE), .spd = kSpeed100MBit, .idx = MIDX_100FD_EEE, .speed = SPEED_100, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_none, .eee = kEEETypeYes, .adv = ADVERTISED_100baseT_Full },
    { .type = (kIOMediumEthernet100BaseTX | IFM_FDX | IFM_FLOW | IFM_EEE), .spd = kSpeed100MBit, .idx = MIDX_100FDFC_EEE, .speed = SPEED_100, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_full, .eee = kEEETypeYes, .adv = ADVERTISED_100baseT_Full },

    /* 1000Base-T */
    { .type = (kIOMediumEthernet1000BaseT | IFM_FDX), .spd = kSpeed1000MBit, .idx = MIDX_1000FD, .speed = SPEED_1000, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_none, .eee = kEEETypeNo, .adv = ADVERTISED_1000baseT_Full },
    { .type = (kIOMediumEthernet1000BaseT | IFM_FDX | IFM_FLOW), .spd = kSpeed1000MBit, .idx = MIDX_1000FDFC, .speed = SPEED_1000, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_full, .eee = kEEETypeNo, .adv = ADVERTISED_1000baseT_Full },
    { .type = (kIOMediumEthernet1000BaseT | IFM_FDX | IFM_EEE), .spd = kSpeed1000MBit, .idx = MIDX_1000FD_EEE, .speed = SPEED_1000, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_none, .eee = kEEETypeYes, .adv = ADVERTISED_1000baseT_Full },
    { .type = (kIOMediumEthernet1000BaseT | IFM_FDX | IFM_FLOW | IFM_EEE), .spd = kSpeed1000MBit, .idx = MIDX_1000FDFC_EEE, .speed = SPEED_1000, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_full, .eee = kEEETypeYes, .adv = ADVERTISED_1000baseT_Full },

    /* 2500Base-T */
    { .type = (kIOMediumEthernet2500BaseT | IFM_FDX), .spd = kSpeed2500MBit, .idx = MIDX_2500FD, .speed = SPEED_2500, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_none, .eee = kEEETypeNo, .adv = ADVERTISED_2500baseX_Full },
    { .type = (kIOMediumEthernet2500BaseT | IFM_FDX | IFM_FLOW), .spd = kSpeed2500MBit, .idx = MIDX_2500FDFC, .speed = SPEED_2500, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_full, .eee = kEEETypeNo, .adv = ADVERTISED_2500baseX_Full },
    { .type = (kIOMediumEthernet2500BaseT | IFM_FDX | IFM_EEE), .spd = kSpeed2500MBit, .idx = MIDX_2500FD_EEE, .speed = SPEED_2500, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_none, .eee = kEEETypeYes, .adv = ADVERTISED_2500baseX_Full },
    { .type = (kIOMediumEthernet2500BaseT | IFM_FDX | IFM_FLOW | IFM_EEE), .spd = kSpeed2500MBit, .idx = MIDX_2500FDFC_EEE, .speed = SPEED_2500, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_full, .eee = kEEETypeYes, .adv = ADVERTISED_2500baseX_Full },
    
    /* 5000Base-T */
    { .type = (kIOMediumEthernet5000BaseT | IFM_FDX), .spd = kSpeed5000MBit, .idx = MIDX_5000FD, .speed = SPEED_5000, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_none, .eee = kEEETypeNo, .adv = RTK_ADVERTISED_5000baseX_Full },
    { .type = (kIOMediumEthernet5000BaseT | IFM_FDX | IFM_FLOW), .spd = kSpeed5000MBit, .idx = MIDX_5000FDFC, .speed = SPEED_5000, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_full, .eee = kEEETypeNo, .adv = RTK_ADVERTISED_5000baseX_Full },
    { .type = (kIOMediumEthernet5000BaseT | IFM_FDX | IFM_EEE), .spd = kSpeed5000MBit, .idx = MIDX_5000FD_EEE, .speed = SPEED_5000, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_none, .eee = kEEETypeYes, .adv = RTK_ADVERTISED_5000baseX_Full },
    { .type = (kIOMediumEthernet5000BaseT | IFM_FDX | IFM_FLOW | IFM_EEE), .spd = kSpeed5000MBit, .idx = MIDX_5000FDFC_EEE, .speed = SPEED_5000, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_full, .eee = kEEETypeYes, .adv = RTK_ADVERTISED_5000baseX_Full },
    
    /* 10GBase-T */
    { .type = (kIOMediumEthernet10GBaseT | IFM_FDX), .spd = kSpeed10000MBit, .idx = MIDX_10000FD, .speed = SPEED_10000, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_none, .eee = kEEETypeNo, .adv = 0 },
    { .type = (kIOMediumEthernet10GBaseT | IFM_FDX | IFM_FLOW), .spd = kSpeed10000MBit, .idx = MIDX_10000FDFC, .speed = SPEED_10000, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_full, .eee = kEEETypeNo, .adv = 0 },
    { .type = (kIOMediumEthernet10GBaseT | IFM_FDX | IFM_EEE), .spd = kSpeed10000MBit, .idx = MIDX_10000FD_EEE, .speed = SPEED_10000, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_none, .eee = kEEETypeYes, .adv = 0 },
    { .type = (kIOMediumEthernet10GBaseT | IFM_FDX | IFM_FLOW | IFM_EEE), .spd = kSpeed10000MBit, .idx = MIDX_10000FDFC_EEE, .speed = SPEED_10000, .duplex = DUPLEX_FULL, .fc = rtl8125_fc_full, .eee = kEEETypeYes, .adv = 0 },
};

#pragma mark --- data structure initialization methods ---

void LucyRTL8125::getParams()
{
    OSDictionary *params;
    OSIterator *iterator;
    OSString *versionString;
    OSString *fbAddr;
    OSBoolean *tsoV4;
    OSBoolean *tsoV6;
    OSBoolean *aspm;
    OSNumber *tv;
    UInt32 interval;
    
    if (version_major >= Tahoe) {
        params = serviceMatching("AppleVTD");
        
        if (params) {
            iterator = IOService::getMatchingServices(params);
            
            if (iterator) {
                IOMapper *mp = OSDynamicCast(IOMapper, iterator->getNextObject());
                
                if (mp) {
                    IOLog("AppleVTD is enabled.");
                    useAppleVTD = true;
                }
                iterator->release();
            }
            params->release();
        }
    }
    versionString = OSDynamicCast(OSString, getProperty(kDriverVersionName));

    params = OSDynamicCast(OSDictionary, getProperty(kParamName));
    
    if (params) {
        tsoV4 = OSDynamicCast(OSBoolean, params->getObject(kEnableTSO4Name));
        enableTSO4 = (tsoV4 != NULL) ? tsoV4->getValue() : false;
        
        IOLog("TCP/IPv4 segmentation offload %s.\n", enableTSO4 ? onName : offName);
        
        tsoV6 = OSDynamicCast(OSBoolean, params->getObject(kEnableTSO6Name));
        enableTSO6 = (tsoV6 != NULL) ? tsoV6->getValue() : false;
        
        IOLog("TCP/IPv6 segmentation offload %s.\n", enableTSO6 ? onName : offName);
        
        aspm = OSDynamicCast(OSBoolean, params->getObject(kEnableASPM));
        linuxData.aspm = (aspm != NULL) ? aspm->getValue() : false;
        
        IOLog("Active State Power Management %s.\n", enableASPM ? onName : offName);

        tv = OSDynamicCast(OSNumber, params->getObject(kPollTime10GName));

        if (tv != NULL) {
            interval = tv->unsigned32BitValue();
            
            if (interval > 120)
                pollTime10G = 120000;
            else if (interval < 25)
                pollTime10G = 25000;
            else
                pollTime10G = interval * 1000;
        } else {
            pollTime10G = 100000;
        }
        tv = OSDynamicCast(OSNumber, params->getObject(kPollTime5GName));

        if (tv != NULL) {
            interval = tv->unsigned32BitValue();
            
            if (interval > 200)
                pollTime5G = 200000;
            else if (interval < 100)
                pollTime5G = 100000;
            else
                pollTime5G = interval * 1000;
        } else {
            pollTime5G = 120000;
        }
        tv = OSDynamicCast(OSNumber, params->getObject(kPollTime2GName));

        if (tv != NULL) {
            interval = tv->unsigned32BitValue();
            
            if (interval > 200)
                pollTime2G = 200000;
            else if (interval < 100)
                pollTime2G = 100000;
            else
                pollTime2G = interval * 1000;
        } else {
            pollTime2G = 120000;
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
        /* Use default values in case of missing config data. */
        enableTSO4 = false;
        enableTSO6 = false;
        enableASPM = false;
        pollTime10G = 100000;
        pollTime5G = 120000;
        pollTime2G = 160000;
    }
    if (versionString)
        IOLog("Version %s\n", versionString->getCStringNoCopy());
}

bool LucyRTL8125::setupMediumDict()
{
    struct rtl8125_private *tp = &linuxData;
    IONetworkMedium *medium;
    UInt32 limit;
    UInt32 i;
    bool result = false;

    if (HW_SUPP_PHY_LINK_SPEED_5000M(tp)) {
        mediumArray[MIDX_AUTO].adv |= (ADVERTISED_2500baseX_Full | RTK_ADVERTISED_5000baseX_Full);
        limit = MIDX_10000FD;
    } else if (HW_SUPP_PHY_LINK_SPEED_2500M(tp)) {
        mediumArray[MIDX_AUTO].adv |= ADVERTISED_2500baseX_Full;
        limit = (tp->mcfg < CFG_METHOD_4) ? MIDX_2500FD_EEE : MIDX_5000FD;
    } else {
        limit = MIDX_2500FD;
    }
    mediumDict = OSDictionary::withCapacity(limit + 1);

    if (mediumDict) {
        for (i = MIDX_AUTO; i < limit; i++) {
            medium = IONetworkMedium::medium(mediumArray[i].type, mediumArray[i].spd, 0, i);
            
            if (!medium)
                goto error_med;

            result = IONetworkMedium::addMedium(mediumDict, medium);
            medium->release();

            if (!result)
                goto error_med;

            mediumTable[i] = medium;
        }
    }
    result = publishMediumDictionary(mediumDict);
    
    if (!result)
        goto error_med;

done:
    return result;
    
error_med:
    IOLog("Error creating medium dictionary.\n");
    mediumDict->release();
    
    for (i = MIDX_AUTO; i < MIDX_COUNT; i++)
        mediumTable[i] = NULL;

    goto done;
}

void LucyRTL8125::rtl812xMedium2Adv(struct rtl8125_private *tp, UInt32 index)
{
    tp->speed = mediumArray[index].speed;
    tp->duplex = mediumArray[index].duplex;
    tp->fcpause = mediumArray[index].fc;
    tp->advertising = mediumArray[index].adv;
    tp->eee.eee_enabled = mediumArray[index].eee;
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
        
        if (useAppleVTD) {
            interruptSource = IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventSource::Action, this, &LucyRTL8125::interruptOccurredVTD), provider, msiIndex);
        } else {
            interruptSource = IOInterruptEventSource::interruptEventSource(this, OSMemberFunctionCast(IOInterruptEventSource::Action, this, &LucyRTL8125::interruptOccurred), provider, msiIndex);
        }
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
    IOPhysicalAddress64 pa = 0;
    IODMACommand::Segment64 seg;
    mbuf_t m;
    UInt64 offset = 0;
    UInt64 word1;
    UInt32 numSegs = 1;
    UInt32 i;
    bool result = false;
    
    /* Alloc rx mbuf_t array. */
    rxBufArrayMem = IOMallocZero(kRxBufArraySize);
    
    if (!rxBufArrayMem) {
        IOLog("Couldn't alloc receive buffer array.\n");
        goto done;
    }
    rxBufArray = (rtlRxBufferInfo *)rxBufArrayMem;

    /* Create receiver descriptor array. */
    rxBufDesc = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(kernel_task, (kIODirectionInOut | kIOMemoryPhysicallyContiguous | kIOMemoryHostPhysicallyContiguous | kIOMapInhibitCache), kRxDescSize, 0xFFFFFFFFFFFFFF00ULL);
    
    if (!rxBufDesc) {
        IOLog("Couldn't alloc rxBufDesc.\n");
        goto error_buff;
    }
    if (rxBufDesc->prepare() != kIOReturnSuccess) {
        IOLog("rxBufDesc->prepare() failed.\n");
        goto error_prep;
    }
    rxDescArray = (RtlRxDesc *)rxBufDesc->getBytesNoCopy();

    rxDescDmaCmd = IODMACommand::withSpecification(kIODMACommandOutputHost64, 64, 0, IODMACommand::kMapped, 0, 1, mapper, NULL);
    
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
    rxDescArray[kRxLastDesc].cmd.opts1 = OSSwapHostToLittleInt32(RingEnd);

    rxNextDescIndex = 0;
    rxMapNextIndex = 0;

    rxPool = RTL8125LucyRxPool::withCapacity(kRxPoolMbufCap, kRxPoolClstCap);

    if (!rxPool) {
        IOLog("Couldn't alloc receive buffer pool.\n");
        goto error_segm;
    }

    /* Alloc receive buffers. */
    for (i = 0; i < kNumRxDesc; i++) {
        m = rxPool->getPacket(kRxBufferSize, MBUF_WAITOK);

        if (!m) {
            IOLog("Couldn't get receive buffer from pool.\n");
            goto error_buf;
        }
        rxBufArray[i].mbuf = m;

        if (!useAppleVTD) {
            word1 = (kRxBufferSize | DescOwn);

            if (i == kRxLastDesc)
                word1 |= RingEnd;

            pa = mbuf_data_to_physical(mbuf_datastart(m));
            rxBufArray[i].phyAddr = pa;

            rxDescArray[i].buf.blen = OSSwapHostToLittleInt64(word1);
            rxDescArray[i].buf.addr = OSSwapHostToLittleInt64(pa);
        }
    }
    if (useAppleVTD)
        result = setupRxMap();
    else
        result = true;

done:
    return result;
    
error_buf:
    for (i = 0; i < kNumRxDesc; i++) {
        if (rxBufArray[i].mbuf) {
            mbuf_freem_list(rxBufArray[i].mbuf);
            rxBufArray[i].mbuf = NULL;
            rxBufArray[i].phyAddr = 0;
        }
    }
    RELEASE(rxPool);

error_segm:
    rxDescDmaCmd->clearMemoryDescriptor();

error_set_desc:
    RELEASE(rxDescDmaCmd);

error_dma:
    rxBufDesc->complete();
    
error_prep:
    RELEASE(rxBufDesc);

error_buff:
    IOFree(rxBufArrayMem, kRxBufArraySize);
    rxBufArrayMem = NULL;
    rxBufArray = NULL;

    goto done;
}

bool LucyRTL8125::setupTxResources()
{
    IODMACommand::Segment64 seg;
    UInt64 offset = 0;
    UInt32 numSegs = 1;
    bool result = false;
    
    /* Alloc tx mbuf_t array. */
    txBufArrayMem = IOMallocZero(kTxBufArraySize);
    
    if (!txBufArrayMem) {
        IOLog("Couldn't alloc transmit buffer array.\n");
        goto done;
    }
    txMbufArray = (mbuf_t *)txBufArrayMem;
    
    /* Create transmitter descriptor array. */
    txBufDesc = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(kernel_task, (kIODirectionInOut | kIOMemoryPhysicallyContiguous | kIOMemoryHostPhysicallyContiguous | kIOMapInhibitCache), kTxDescSize, 0xFFFFFFFFFFFFFF00ULL);
            
    if (!txBufDesc) {
        IOLog("Couldn't alloc txBufDesc.\n");
        goto error_buff;
    }
    if (txBufDesc->prepare() != kIOReturnSuccess) {
        IOLog("txBufDesc->prepare() failed.\n");
        goto error_prep;
    }
    txDescArray = (RtlTxDesc *)txBufDesc->getBytesNoCopy();
    
    txDescDmaCmd = IODMACommand::withSpecification(kIODMACommandOutputHost64, 64, 0, IODMACommand::kMapped, 0, 1, mapper, NULL);
    
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
    
    txNextDescIndex = txDirtyDescIndex = 0;
    txNumFreeDesc = kNumTxDesc;
    txTailPtr0 = txClosePtr0 = 0;

    if (useAppleVTD) {
        result = setupTxMap();
        
        if (!result)
            goto error_segm;
    } else {
        txMbufCursor = IOMbufNaturalMemoryCursor::withSpecification(PAGE_SIZE, kMaxSegs);
        
        if (!txMbufCursor) {
            IOLog("Couldn't create txMbufCursor.\n");
            goto error_segm;
        }
        result = true;
    }

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
    
error_buff:
    IOFree(txBufArrayMem, kTxBufArraySize);
    txBufArrayMem = NULL;
    txMbufArray = NULL;
    
    goto done;
}

bool LucyRTL8125::setupStatResources()
{
    IODMACommand::Segment64 seg;
    UInt64 offset = 0;
    UInt32 numSegs = 1;
    bool result = false;

    statCall = thread_call_allocate_with_options((thread_call_func_t) &runStatUpdateThread, (void *) this, THREAD_CALL_PRIORITY_KERNEL, 0);
    
    if (!statCall) {
        IOLog("Couldn't alloc thread_call.\n");
        goto done;
    }

    /* Create statistics dump buffer. */
    statBufDesc = IOBufferMemoryDescriptor::inTaskWithPhysicalMask(kernel_task, (kIODirectionIn | kIOMemoryPhysicallyContiguous | kIOMemoryHostPhysicallyContiguous | kIOMapInhibitCache), sizeof(RtlStatData), 0xFFFFFFFFFFFFFF00ULL);
    
    if (!statBufDesc) {
        IOLog("Couldn't alloc statBufDesc.\n");
        goto error_mem;
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
    
error_mem:
    thread_call_free(statCall);
    goto done;
}

void LucyRTL8125::freeRxResources()
{
    UInt32 i;
        
    if (useAppleVTD)
        freeRxMap();

    if (rxDescDmaCmd) {
        rxDescDmaCmd->complete();
        rxDescDmaCmd->clearMemoryDescriptor();
        rxDescDmaCmd->release();
        rxDescDmaCmd = NULL;
    }
    if (rxBufDesc) {
        rxBufDesc->complete();
        rxBufDesc->release();
        rxBufDesc = NULL;
        rxPhyAddr = (IOPhysicalAddress64)NULL;
    }
    RELEASE(rxPool);
    
    if (rxBufArrayMem) {
        for (i = 0; i < kNumRxDesc; i++) {
            if (rxBufArray[i].mbuf) {
                mbuf_freem_list(rxBufArray[i].mbuf);
                rxBufArray[i].mbuf = NULL;
            }
        }
        IOFree(rxBufArrayMem, kRxBufArraySize);
        rxBufArrayMem = NULL;
        rxBufArray = NULL;
    }
}

void LucyRTL8125::freeTxResources()
{
    if (useAppleVTD)
        freeTxMap();
    else
        RELEASE(txMbufCursor);

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
    if (txBufArrayMem) {
        IOFree(txBufArrayMem, kTxBufArraySize);
        txBufArrayMem = NULL;
        txMbufArray = NULL;
    }
}

void LucyRTL8125::freeStatResources()
{
    if (statCall) {
        thread_call_cancel(statCall);
        IOSleep(2);
        thread_call_free(statCall);
        statCall = NULL;
    }
    if (statBufDesc) {
        statBufDesc->complete();
        statBufDesc->release();
        statBufDesc = NULL;
        statPhyAddr = 0;
    }
    if (statDescDmaCmd) {
        statDescDmaCmd->clearMemoryDescriptor();
        statDescDmaCmd->release();
        statDescDmaCmd = NULL;
    }
}

void LucyRTL8125::clearRxTxRings()
{
    IOMemoryDescriptor *md;
    mbuf_t m;
    UInt64 word1;
    UInt32 i;
    
    DebugLog("clearRxTxRings() ===>\n");
    
    if (useAppleVTD && txMapInfo) {
        for (i = 0; i < kNumTxMemDesc; i++) {
            md = txMapInfo->txMemIO[i];
            
            if (md && (md->getTag() == kIOMemoryActive)) {
                md->complete();
                md->setTag(kIOMemoryInactive);
            }
        }
        txMapInfo->txNextMem2Use = txMapInfo->txNextMem2Free = 0;
        txMapInfo->txNumFreeMem = kNumTxMemDesc;
    }
    for (i = 0; i < kNumTxDesc; i++) {
        txDescArray[i].opts1 = OSSwapHostToLittleInt32((i != kTxLastDesc) ? 0 : RingEnd);
        m = txMbufArray[i];
        
        if (m) {
            freePacket(m);
            txMbufArray[i] = NULL;
        }
    }
    txTailPtr0 = txClosePtr0 = 0;

    txDirtyDescIndex = txNextDescIndex = 0;
    txNumFreeDesc = kNumTxDesc;
        
    if (useAppleVTD)
        rxMapBuffers(0, kNumRxMemDesc);

    for (i = 0; i < kNumRxDesc; i++) {
        word1 = (kRxBufferSize | DescOwn);
        
        if (i == kRxLastDesc)
            word1 |= RingEnd;
        
        rxDescArray[i].buf.blen = OSSwapHostToLittleInt64(word1);
        rxDescArray[i].buf.addr = OSSwapHostToLittleInt64(rxBufArray[i].phyAddr);
    }
    rxNextDescIndex = 0;
    rxMapNextIndex = 0;
    deadlockWarn = 0;

    /* Free packet fragments which haven't been upstreamed yet.  */
    discardPacketFragment();

    DebugLog("clearRxTxRings() <===\n");
}

void LucyRTL8125::discardPacketFragment()
{
    /*
     * In case there is a packet fragment which hasn't been enqueued yet
     * we have to free it in order to prevent a memory leak.
     */
    if (rxPacketHead)
        mbuf_freem_list(rxPacketHead);
    
    rxPacketHead = rxPacketTail = NULL;
    rxPacketSize = 0;
}
