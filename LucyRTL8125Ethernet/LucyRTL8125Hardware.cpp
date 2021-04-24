/* LucyRTL8125Hardware.cpp -- RTL8125 hardware initialzation methods.
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

#pragma mark --- hardware initialization methods ---

bool LucyRTL8125::initPCIConfigSpace(IOPCIDevice *provider)
{
    UInt32 pcieLinkCap;
    UInt16 pcieLinkCtl;
    UInt16 cmdReg;
    UInt16 pmCap;
    UInt8 pcieCapOffset;
    UInt8 pmCapOffset;
    bool result = false;
    
    /* Get vendor and device info. */
    pciDeviceData.vendor = provider->configRead16(kIOPCIConfigVendorID);
    pciDeviceData.device = provider->configRead16(kIOPCIConfigDeviceID);
    pciDeviceData.subsystem_vendor = provider->configRead16(kIOPCIConfigSubSystemVendorID);
    pciDeviceData.subsystem_device = provider->configRead16(kIOPCIConfigSubSystemID);
    
    /* Setup power management. */
    if (provider->findPCICapability(kIOPCIPowerManagementCapability, &pmCapOffset)) {
        pmCap = provider->extendedConfigRead16(pmCapOffset + kIOPCIPMCapability);
        DebugLog("PCI power management capabilities: 0x%x.\n", pmCap);
        
        if (pmCap & kPCIPMCPMESupportFromD3Cold) {
            wolCapable = true;
            DebugLog("PME# from D3 (cold) supported.\n");
        }
        pciPMCtrlOffset = pmCapOffset + kIOPCIPMControl;
    } else {
        IOLog("PCI power management unsupported.\n");
    }
    provider->enablePCIPowerManagement(kPCIPMCSPowerStateD0);
    
    /* Get PCIe link information. */
    if (provider->findPCICapability(kIOPCIPCIExpressCapability, &pcieCapOffset)) {
        pcieLinkCap = provider->configRead32(pcieCapOffset + kIOPCIELinkCapability);
        pcieLinkCtl = provider->configRead16(pcieCapOffset + kIOPCIELinkControl);
        DebugLog("PCIe link capabilities: 0x%08x, link control: 0x%04x.\n", pcieLinkCap, pcieLinkCtl);
        
        if (linuxData.configASPM == 0) {
            IOLog("Disable PCIe ASPM.\n");
            provider->setASPMState(this, 0);
        } else {
            IOLog("Warning: Enable PCIe ASPM.\n");
            provider->setASPMState(this, kIOPCIELinkCtlASPM | kIOPCIELinkCtlClkPM);
            linuxData.configASPM = 1;
        }
    }
    /* Enable the device. */
    cmdReg    = provider->configRead16(kIOPCIConfigCommand);
    cmdReg  &= ~kIOPCICommandIOSpace;
    cmdReg    |= (kIOPCICommandBusMaster | kIOPCICommandMemorySpace | kIOPCICommandMemWrInvalidate);
    provider->configWrite16(kIOPCIConfigCommand, cmdReg);
    //provider->configWrite8(kIOPCIConfigLatencyTimer, 0x40);
    
    baseMap = provider->mapDeviceMemoryWithRegister(kIOPCIConfigBaseAddress2, kIOMapInhibitCache);
    
    if (!baseMap) {
        IOLog("region #2 not an MMIO resource, aborting.\n");
        goto done;
    }
    baseAddr = reinterpret_cast<volatile void *>(baseMap->getVirtualAddress());
    linuxData.mmio_addr = baseAddr;
    result = true;
    
done:
    return result;
}

IOReturn LucyRTL8125::setPowerStateWakeAction(OSObject *owner, void *arg1, void *arg2, void *arg3, void *arg4)
{
    LucyRTL8125 *ethCtlr = OSDynamicCast(LucyRTL8125, owner);
    IOPCIDevice *dev;
    UInt16 val16;
    UInt8 offset;
    
    if (ethCtlr && ethCtlr->pciPMCtrlOffset) {
        dev = ethCtlr->pciDevice;
        offset = ethCtlr->pciPMCtrlOffset;
        
        val16 = dev->extendedConfigRead16(offset);
        
        val16 &= ~(kPCIPMCSPowerStateMask | kPCIPMCSPMEStatus | kPCIPMCSPMEEnable);
        val16 |= kPCIPMCSPowerStateD0;
        
        dev->extendedConfigWrite16(offset, val16);
    }
    return kIOReturnSuccess;
}

IOReturn LucyRTL8125::setPowerStateSleepAction(OSObject *owner, void *arg1, void *arg2, void *arg3, void *arg4)
{
    LucyRTL8125 *ethCtlr = OSDynamicCast(LucyRTL8125, owner);
    IOPCIDevice *dev;
    UInt16 val16;
    UInt8 offset;

    if (ethCtlr && ethCtlr->pciPMCtrlOffset) {
        dev = ethCtlr->pciDevice;
        offset = ethCtlr->pciPMCtrlOffset;
        
        val16 = dev->extendedConfigRead16(offset);
        
        val16 &= ~(kPCIPMCSPowerStateMask | kPCIPMCSPMEStatus | kPCIPMCSPMEEnable);

        if (ethCtlr->wolActive)
            val16 |= (kPCIPMCSPMEStatus | kPCIPMCSPMEEnable | kPCIPMCSPowerStateD3);
        else
            val16 |= kPCIPMCSPowerStateD3;
        
        dev->extendedConfigWrite16(offset, val16);
    }
    return kIOReturnSuccess;
}

/*
 * These functions have to be rewritten after every update
 * of the underlying Linux sources.
 */

IOReturn LucyRTL8125::identifyChip()
{
    struct rtl8125_private *tp = &linuxData;
    IOReturn result = kIOReturnSuccess;
    UInt32 reg, val32;
    UInt32 version;

    val32 = ReadReg32(TxConfig);
    reg = val32 & 0x7c800000;
    version = val32 & 0x00700000;

    switch (reg) {
        case 0x60800000:
            if (version == 0x00000000) {
                tp->mcfg = CFG_METHOD_2;
                tp->chipset = 0;
            } else if (version == 0x100000) {
                tp->mcfg = CFG_METHOD_3;
                tp->chipset = 1;
            } else {
                tp->mcfg = CFG_METHOD_3;
                tp->chipset = 1;
                tp->HwIcVerUnknown = TRUE;
            }
            tp->efuse_ver = EFUSE_SUPPORT_V4;
            break;
            
        case 0x64000000:
            if (version == 0x00000000) {
                tp->mcfg = CFG_METHOD_4;
                tp->chipset = 2;
            } else if (version == 0x100000) {
                tp->mcfg = CFG_METHOD_5;
                tp->chipset = 3;
            } else {
                tp->mcfg = CFG_METHOD_5;
                tp->chipset = 3;
                tp->HwIcVerUnknown = TRUE;
            }
            tp->efuse_ver = EFUSE_SUPPORT_V4;
            break;
            
        default:
            tp->mcfg = CFG_METHOD_DEFAULT;
            tp->HwIcVerUnknown = TRUE;
            tp->efuse_ver = EFUSE_NOT_SUPPORT;
            result = kIOReturnError;
            break;
    }
    return result;
}

bool LucyRTL8125::initRTL8125()
{
    struct rtl8125_private *tp = &linuxData;
    UInt32 i;
    UInt8 macAddr[MAC_ADDR_LEN];
    bool result = false;
    
    /* Identify chip attached to board. */
    if(identifyChip()) {
        IOLog("Unsupported chip found. Aborting...\n");
        goto done;
    }
    
    /* Setup EEE support. */
    tp->eee_adv_t = eeeCap = (MDIO_EEE_100TX | MDIO_EEE_1000T);
    
    tp->phy_reset_enable = rtl8125_xmii_reset_enable;
    tp->phy_reset_pending = rtl8125_xmii_reset_pending;

    tp->max_jumbo_frame_size = rtl_chip_info[tp->chipset].jumbo_frame_sz;
    
    rtl8125_get_bios_setting(tp);
    
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            //tp->HwSuppDashVer = 3;
            break;
        default:
            tp->HwSuppDashVer = 0;
            break;
    }

    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            tp->HwPkgDet = rtl8125_mac_ocp_read(tp, 0xDC00);
            tp->HwPkgDet = (tp->HwPkgDet >> 3) & 0x07;
            break;
    }

    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            tp->HwSuppNowIsOobVer = 1;
            break;
    }

    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            tp->HwPcieSNOffset = 0x16C;
            break;
    }

    #ifdef ENABLE_REALWOW_SUPPORT
            rtl8125_get_realwow_hw_version(dev);
    #endif //ENABLE_REALWOW_SUPPORT

    if (linuxData.configASPM) {
        switch (tp->mcfg) {
            case CFG_METHOD_2:
            case CFG_METHOD_3:
            case CFG_METHOD_4:
            case CFG_METHOD_5:
                    tp->org_pci_offset_99 = csiFun0ReadByte(0x99);
                    tp->org_pci_offset_99 &= ~(BIT_5|BIT_6);
                    break;
        }
        switch (tp->mcfg) {
            case CFG_METHOD_2:
            case CFG_METHOD_3:
                    tp->org_pci_offset_180 = csiFun0ReadByte(0x264);
                    break;
            case CFG_METHOD_4:
            case CFG_METHOD_5:
                    tp->org_pci_offset_180 = csiFun0ReadByte(0x214);
                    break;
        }
    }
    tp->org_pci_offset_80 = pciDevice->configRead8(0x80);
    tp->org_pci_offset_81 = pciDevice->configRead8(0x81);
    tp->use_timer_interrrupt = FALSE;

    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            tp->HwSuppMagicPktVer = WAKEUP_MAGIC_PACKET_V3;
            break;
        default:
            tp->HwSuppMagicPktVer = WAKEUP_MAGIC_PACKET_NOT_SUPPORT;
            break;
    }
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            tp->HwSuppLinkChgWakeUpVer = 3;
            break;
    }
    switch (tp->mcfg) {
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            tp->HwSuppD0SpeedUpVer = 1;
            break;
    }
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            tp->HwSuppCheckPhyDisableModeVer = 3;
            break;
    }
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            tp->HwSuppGigaForceMode = TRUE;
            break;
    }
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            tp->HwSuppTxNoCloseVer = 3;
            break;
    }
    if (tp->HwSuppTxNoCloseVer > 0)
        tp->EnableTxNoClose = TRUE;

    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
            tp->RequireLSOPatch = TRUE;
            break;
    }

    switch (tp->mcfg) {
        case CFG_METHOD_2:
            tp->sw_ram_code_ver = NIC_RAMCODE_VERSION_CFG_METHOD_2;
            break;
        case CFG_METHOD_3:
            tp->sw_ram_code_ver = NIC_RAMCODE_VERSION_CFG_METHOD_3;
            break;
        case CFG_METHOD_4:
            tp->sw_ram_code_ver = NIC_RAMCODE_VERSION_CFG_METHOD_4;
            break;
        case CFG_METHOD_5:
            tp->sw_ram_code_ver = NIC_RAMCODE_VERSION_CFG_METHOD_5;
            break;
    }

    if (tp->HwIcVerUnknown) {
            tp->NotWrRamCodeToMicroP = TRUE;
            tp->NotWrMcuPatchCode = TRUE;
    }

    switch (tp->mcfg) {
    case CFG_METHOD_3:
        if ((rtl8125_mac_ocp_read(tp, 0xD442) & BIT_5) &&
            (mdio_direct_read_phy_ocp(tp, 0xD068) & BIT_1)
            ) {
                tp->RequirePhyMdiSwapPatch = TRUE;
        }
        break;
    }
    switch (tp->mcfg) {
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            tp->HwSuppNumTxQueues = 2;
            tp->HwSuppNumRxQueues = 4;
            break;
        default:
            tp->HwSuppNumTxQueues = 1;
            tp->HwSuppNumRxQueues = 1;
            break;
    }
    switch (tp->mcfg) {
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            tp->HwSuppRssVer = 5;
            tp->HwSuppIndirTblEntries = 128;
            break;
    }
    switch (tp->mcfg) {
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            tp->HwSuppPtpVer = 1;
            break;
    }

    //init interrupt
    switch (tp->mcfg) {
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            //tp->HwSuppIsrVer = 2;
            tp->HwSuppIsrVer = 1;
            break;
        default:
            tp->HwSuppIsrVer = 1;
            break;
    }
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
            tp->HwSuppIntMitiVer = 3;
            break;
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            tp->HwSuppIntMitiVer = 4;
            break;
    }

    tp->NicCustLedValue = ReadReg16(CustomLED);

    tp->wol_opts = rtl8125_get_hw_wol(tp);
    tp->wol_enabled = (tp->wol_opts) ? WOL_ENABLED : WOL_DISABLED;

    /* Set wake on LAN support. */
    wolCapable = (tp->wol_enabled == WOL_ENABLED);

    //tp->eee_enabled = eee_enable;
    tp->eee_adv_t = MDIO_EEE_1000T | MDIO_EEE_100TX;
    
    exitOOB();
    rtl8125_hw_init(tp);
    rtl8125_nic_reset(tp);
    
    /* Get production from EEPROM */
    rtl8125_eeprom_type(tp);

    if (tp->eeprom_type == EEPROM_TYPE_93C46 || tp->eeprom_type == EEPROM_TYPE_93C56)
            rtl8125_set_eeprom_sel_low(tp);

    for (i = 0; i < MAC_ADDR_LEN; i++)
            macAddr[i] = ReadReg8(MAC0 + i);

    if(tp->mcfg == CFG_METHOD_2 ||
        tp->mcfg == CFG_METHOD_3 ||
        tp->mcfg == CFG_METHOD_4 ||
        tp->mcfg == CFG_METHOD_5) {
            *(UInt32*)&macAddr[0] = ReadReg32(BACKUP_ADDR0_8125);
            *(UInt16*)&macAddr[4] = ReadReg16(BACKUP_ADDR1_8125);
    }

    if (is_valid_ether_addr((UInt8 *) macAddr)) {
        rtl8125_rar_set(tp, macAddr);
    } else {
        IOLog("Using fallback MAC.\n");
        rtl8125_rar_set(tp, fallBackMacAddr.bytes);
    }
    for (i = 0; i < MAC_ADDR_LEN; i++) {
        currMacAddr.bytes[i] = ReadReg8(MAC0 + i);
        origMacAddr.bytes[i] = currMacAddr.bytes[i]; /* keep the original MAC address */
    }
    IOLog("%s: (Chipset %d), %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n",
          rtl_chip_info[tp->chipset].name, tp->chipset,
          origMacAddr.bytes[0], origMacAddr.bytes[1],
          origMacAddr.bytes[2], origMacAddr.bytes[3],
          origMacAddr.bytes[4], origMacAddr.bytes[5]);
    
    tp->cp_cmd = (ReadReg16(CPlusCmd) | RxChkSum);
    
    intrMaskRxTx = (SYSErr | LinkChg | RxDescUnavail | TxErr | TxOK | RxErr | RxOK);
    intrMaskPoll = (SYSErr | LinkChg);
    intrMask = intrMaskRxTx;
    
    /* Get the RxConfig parameters. */
    rxConfigReg = rtl_chip_info[tp->chipset].RCR_Cfg;
    rxConfigMask = rtl_chip_info[tp->chipset].RxConfigMask;
  
    /* Reset the tally counter. */
    WriteReg32(CounterAddrHigh, (statPhyAddr >> 32));
    WriteReg32(CounterAddrLow, (statPhyAddr & 0x00000000ffffffff) | CounterReset);

    rtl8125_disable_rxdvgate(tp);
    
#ifdef DEBUG
    
    if (wolCapable)
        IOLog("Device is WoL capable.\n");
    
#endif
    
    result = true;
    
done:
    return result;
}

void LucyRTL8125::enableRTL8125()
{
    struct rtl8125_private *tp = &linuxData;
    
    setLinkStatus(kIONetworkLinkValid);
    
    intrMask = intrMaskRxTx;
    polling = false;
    
    exitOOB();
    rtl8125_hw_init(tp);
    rtl8125_nic_reset(tp);
    rtl8125_powerup_pll(tp);
    rtl8125_hw_ephy_config(tp);
    configPhyHardware();
    setupRTL8125(intrMitigateValue, true);
    
    setPhyMedium();
}

void LucyRTL8125::disableRTL8125()
{
    struct rtl8125_private *tp = &linuxData;
    
    /* Disable all interrupts by clearing the interrupt mask. */
    WriteReg32(IMR0_8125, 0);
    WriteReg16(IntrStatus, ReadReg16(IntrStatus));

    rtl8125_nic_reset(tp);
    hardwareD3Para();
    powerDownPLL();
    
    if (linkUp) {
        linkUp = false;
        setLinkStatus(kIONetworkLinkValid);
        IOLog("Link down on en%u\n", netif->getUnitNumber());
    }
}

/* Reset the NIC in case a tx deadlock or a pci error occurred. timerSource and txQueue
 * are stopped immediately but will be restarted by checkLinkStatus() when the link has
 * been reestablished.
 */

void LucyRTL8125::restartRTL8125()
{
    /* Stop output thread and flush txQueue */
    netif->stopOutputThread();
    netif->flushOutputQueue();
    
    linkUp = false;
    setLinkStatus(kIONetworkLinkValid);
    
    /* Reset NIC and cleanup both descriptor rings. */
    rtl8125_nic_reset(&linuxData);
/*
    if (rxInterrupt(netif, kNumRxDesc, NULL, NULL))
        netif->flushInputQueue();
*/
    clearDescriptors();

    /* Reinitialize NIC. */
    enableRTL8125();
}

void LucyRTL8125::setupRTL8125(UInt16 newIntrMitigate, bool enableInterrupts)
{
    struct rtl8125_private *tp = &linuxData;
    UInt32 i;
    UInt16 mac_ocp_data;
    
    WriteReg32(RxConfig, (RX_DMA_BURST << RxCfgDMAShift));
    
    rtl8125_nic_reset(tp);
    
    WriteReg8(Cfg9346, ReadReg8(Cfg9346) | Cfg9346_Unlock);
    
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            WriteReg8(0xF1, ReadReg8(0xF1) & ~BIT_7);
            WriteReg8(Config2, ReadReg8(Config2) & ~BIT_7);
            WriteReg8(Config5, ReadReg8(Config5) & ~BIT_0);
            break;
    }

    //clear io_rdy_l23
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            WriteReg8(Config3, ReadReg8(Config3) & ~BIT_1);
            break;
    }

    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            //IntMITI_0-IntMITI_31
            for (i=0xA00; i<0xB00; i+=4)
                    WriteReg32(i, 0x00000000);
            break;
    }

    //keep magic packet only
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xC0B6);
            mac_ocp_data &= BIT_0;
            rtl8125_mac_ocp_write(tp, 0xC0B6, mac_ocp_data);
            break;
    }
    /* Fill tally counter address. */
    WriteReg32(CounterAddrHigh, (statPhyAddr >> 32));
    WriteReg32(CounterAddrLow, (statPhyAddr & 0x00000000ffffffff));

    /* Setup the descriptor rings. */
    txTailPtr0 = txClosePtr0 = 0;
    txNextDescIndex = txDirtyDescIndex = 0;
    txNumFreeDesc = kNumTxDesc;
    rxNextDescIndex = 0;
    
    WriteReg32(TxDescStartAddrLow, (txPhyAddr & 0x00000000ffffffff));
    WriteReg32(TxDescStartAddrHigh, (txPhyAddr >> 32));
    WriteReg32(RxDescAddrLow, (rxPhyAddr & 0x00000000ffffffff));
    WriteReg32(RxDescAddrHigh, (rxPhyAddr >> 32));

    /* Set DMA burst size and Interframe Gap Time */
    WriteReg32(TxConfig, (TX_DMA_BURST_unlimited << TxDMAShift) |
            (InterFrameGap << TxInterFrameGapShift));

    if (tp->EnableTxNoClose)
            WriteReg32(TxConfig, (ReadReg32(TxConfig) | BIT_6));
    
    if (tp->mcfg == CFG_METHOD_2 ||
        tp->mcfg == CFG_METHOD_3 ||
        tp->mcfg == CFG_METHOD_4 ||
        tp->mcfg == CFG_METHOD_5) {
        set_offset70F(tp, 0x27);
        setOffset79(0x50);

        WriteReg16(0x382, 0x221B);

        /* Disable RSS. */
        WriteReg8(RSS_CTRL_8125, 0x00);
        WriteReg16(Q_NUM_CTRL_8125, 0x0000);

        WriteReg8(Config1, ReadReg8(Config1) & ~0x10);

        rtl8125_mac_ocp_write(tp, 0xC140, 0xFFFF);
        rtl8125_mac_ocp_write(tp, 0xC142, 0xFFFF);

        /*
         * Disabling the new tx descriptor format seems to prevent
         * tx timeouts when using TSO.
         */
        mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xEB58);
        //mac_ocp_data |= (BIT_0);
        mac_ocp_data &= ~(BIT_0);
        rtl8125_mac_ocp_write(tp, 0xEB58, mac_ocp_data);

        mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xE614);
        mac_ocp_data &= ~( BIT_10 | BIT_9 | BIT_8);
        if (tp->mcfg == CFG_METHOD_4 || tp->mcfg == CFG_METHOD_5) {
            mac_ocp_data |= ((2 & 0x07) << 8);
        } else {
            if (tp->DASH && !(csiFun0ReadByte(0x79) & BIT_0))
                mac_ocp_data |= ((3 & 0x07) << 8);
            else
                mac_ocp_data |= ((4 & 0x07) << 8);
        }
        rtl8125_mac_ocp_write(tp, 0xE614, mac_ocp_data);
        
        //rtl8125_set_tx_q_num(tp, tp->HwSuppNumTxQueues);
        
        /* Set tx queue num to one. */
        mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xE63E);
        mac_ocp_data &= ~(BIT_11 | BIT_10);
        mac_ocp_data |= ((0 & 0x03) << 10);
        rtl8125_mac_ocp_write(tp, 0xE63E, mac_ocp_data);

        mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xE63E);
        mac_ocp_data &= ~(BIT_5 | BIT_4);
        
        if (tp->mcfg == CFG_METHOD_2 || tp->mcfg == CFG_METHOD_3)
            mac_ocp_data |= ((0x02 & 0x03) << 4);
        
        rtl8125_mac_ocp_write(tp, 0xE63E, mac_ocp_data);
        
        mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xC0B4);
        mac_ocp_data &= ~BIT_0;
        rtl8125_mac_ocp_write(tp, 0xC0B4, mac_ocp_data);
        mac_ocp_data |= BIT_0;
        rtl8125_mac_ocp_write(tp, 0xC0B4, mac_ocp_data);
        
        mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xC0B4);
        mac_ocp_data |= (BIT_3|BIT_2);
        rtl8125_mac_ocp_write(tp, 0xC0B4, mac_ocp_data);
        
        mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xEB6A);
        mac_ocp_data &= ~(BIT_7 | BIT_6 | BIT_5 | BIT_4 | BIT_3 | BIT_2 | BIT_1 | BIT_0);
        mac_ocp_data |= (BIT_5 | BIT_4 | BIT_1 | BIT_0);
        rtl8125_mac_ocp_write(tp, 0xEB6A, mac_ocp_data);
        
        mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xEB50);
        mac_ocp_data &= ~(BIT_9 | BIT_8 | BIT_7 | BIT_6 | BIT_5);
        mac_ocp_data |= (BIT_6);
        rtl8125_mac_ocp_write(tp, 0xEB50, mac_ocp_data);
        
        mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xE056);
        mac_ocp_data &= ~(BIT_7 | BIT_6 | BIT_5 | BIT_4);
        mac_ocp_data |= (BIT_4 | BIT_5);
        rtl8125_mac_ocp_write(tp, 0xE056, mac_ocp_data);
        
        WriteReg8(TDFNR, 0x10);
        
        WriteReg8(0xD0, RTL_R8(tp, 0xD0) | BIT_7);
        
        mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xE040);
        mac_ocp_data &= ~(BIT_12);
        rtl8125_mac_ocp_write(tp, 0xE040, mac_ocp_data);
        
        mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xEA1C);
        mac_ocp_data &= ~(BIT_1 | BIT_0);
        mac_ocp_data |= (BIT_0);
        rtl8125_mac_ocp_write(tp, 0xEA1C, mac_ocp_data);
        
        mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xE0C0);
        mac_ocp_data &= ~(BIT_14 | BIT_11 | BIT_10 | BIT_9 | BIT_8 | BIT_3 | BIT_2 | BIT_1 | BIT_0);
        mac_ocp_data |= (BIT_14 | BIT_10 | BIT_1 | BIT_0);
        rtl8125_mac_ocp_write(tp, 0xE0C0, mac_ocp_data);
        
        SetMcuAccessRegBit(tp, 0xE052, (BIT_6|BIT_5|BIT_3));
        ClearMcuAccessRegBit(tp, 0xE052, BIT_7);
        
        mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xD430);
        mac_ocp_data &= ~(BIT_11 | BIT_10 | BIT_9 | BIT_8 | BIT_7 | BIT_6 | BIT_5 | BIT_4 | BIT_3 | BIT_2 | BIT_1 | BIT_0);
        mac_ocp_data |= 0x47F;
        rtl8125_mac_ocp_write(tp, 0xD430, mac_ocp_data);
        
        //rtl8125_mac_ocp_write(tp, 0xE0C0, 0x4F87);
        WriteReg8(0xD0, RTL_R8(tp, 0xD0) | BIT_6 | BIT_7);
        
        if (tp->mcfg == CFG_METHOD_2 || tp->mcfg == CFG_METHOD_3)
            WriteReg8(0xD3, RTL_R8(tp, 0xD3) | BIT_0);
        
        rtl8125_disable_eee_plus(tp);
        
        mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xEA1C);
        mac_ocp_data &= ~(BIT_2);
        rtl8125_mac_ocp_write(tp, 0xEA1C, mac_ocp_data);
        
        SetMcuAccessRegBit(tp, 0xEB54, BIT_0);
        udelay(1);
        ClearMcuAccessRegBit(tp, 0xEB54, BIT_0);
        WriteReg16(0x1880, RTL_R16(tp, 0x1880) & ~(BIT_4 | BIT_5));
    }
    //other hw parameters
    rtl8125_hw_clear_timer_int(tp);

    rtl8125_hw_clear_int_miti(tp);

    rtl8125_enable_exit_l1_mask(tp);

    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            rtl8125_mac_ocp_write(tp, 0xE098, 0xC302);
            break;
    }

    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            if (linuxData.configASPM) {
                initPCIOffset99();
            }
            break;
    }
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            if (linuxData.configASPM) {
                rtl8125_init_pci_offset_180(tp);
            }
            break;
    }

    tp->cp_cmd &= ~(EnableBist | Macdbgo_oe | Force_halfdup |
                    Force_rxflow_en | Force_txflow_en | Cxpl_dbg_sel |
                    ASF | Macdbgo_sel);

    WriteReg16(CPlusCmd, tp->cp_cmd);

    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5: {
            int timeout;
            for (timeout = 0; timeout < 10; timeout++) {
                if ((rtl8125_mac_ocp_read(tp, 0xE00E) & BIT_13)==0)
                    break;
                mdelay(1);
            }
        }
        break;
    }
    /* Needs further investigation if mtu + 18 or mtu + 22 should be used. */
    WriteReg16(RxMaxSize, mtu + (ETH_HLEN + ETH_FCS_LEN + VLAN_HLEN + 1));

    rtl8125_disable_rxdvgate(tp);

    /* Set receiver mode. */
    setMulticastMode(multicastMode);

    #ifdef ENABLE_DASH_SUPPORT
            if (tp->DASH && !tp->dash_printer_enabled)
                    NICChkTypeEnableDashInterrupt(tp);
    #endif

    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            if (linuxData.configASPM) {
                WriteReg8(Config5, ReadReg8(Config5) | BIT_0);
                WriteReg8(Config2, ReadReg8(Config2) | BIT_7);
            } else {
                WriteReg8(Config2, ReadReg8(Config2) & ~BIT_7);
                WriteReg8(Config5, ReadReg8(Config5) & ~BIT_0);
            }
            break;
    }

    WriteReg8(Cfg9346, ReadReg8(Cfg9346) & ~Cfg9346_Unlock);
    
    if (enableInterrupts) {
        /* Enable all known interrupts by setting the interrupt mask. */
        WriteReg32(IMR0_8125, intrMask);
    }
    udelay(10);
}

void LucyRTL8125::setPhyMedium()
{
    struct rtl8125_private *tp = netdev_priv(&linuxData);
    int auto_nego = 0;
    int giga_ctrl = 0;
    int ctrl_2500 = 0;
    
    if (speed != SPEED_2500 && (speed != SPEED_1000) &&
        (speed != SPEED_100) && (speed != SPEED_10)) {
        duplex = DUPLEX_FULL;
        autoneg = AUTONEG_ENABLE;
    }
    /* Enable or disable EEE support according to selected medium. */
    if ((linuxData.eee_adv_t != 0) && (autoneg == AUTONEG_ENABLE)) {
        rtl8125_enable_eee(tp);
        DebugLog("Enable EEE support.\n");
    } else {
        rtl8125_disable_eee(tp);
        DebugLog("Disable EEE support.\n");
    }
    //Disable Giga Lite
    ClearEthPhyOcpBit(tp, 0xA428, BIT_9);
    ClearEthPhyOcpBit(tp, 0xA5EA, BIT_0);

    giga_ctrl = rtl8125_mdio_read(tp, MII_CTRL1000);
    giga_ctrl &= ~(ADVERTISE_1000HALF | ADVERTISE_1000FULL);
    
    ctrl_2500 = mdio_direct_read_phy_ocp(tp, 0xA5D4);
    ctrl_2500 &= ~(RTK_ADVERTISE_2500FULL);
    
    auto_nego = rtl8125_mdio_read(tp, MII_ADVERTISE);
    auto_nego &= ~(ADVERTISE_10HALF | ADVERTISE_10FULL |
                   ADVERTISE_100HALF | ADVERTISE_100FULL |
                   ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);

    if (autoneg == AUTONEG_ENABLE) {
        /* The default medium has been selected. */
        auto_nego |= (ADVERTISE_10HALF | ADVERTISE_10FULL | ADVERTISE_100HALF | ADVERTISE_100FULL);
        giga_ctrl |= ADVERTISE_1000FULL;
        ctrl_2500 |= RTK_ADVERTISE_2500FULL;
    } else if (speed == SPEED_2500) {
        ctrl_2500 |= RTK_ADVERTISE_2500FULL;
    } else if (speed == SPEED_1000) {
        if (duplex == DUPLEX_HALF) {
            giga_ctrl |= ADVERTISE_1000HALF;
        } else {
            giga_ctrl |= ADVERTISE_1000FULL;
        }
    } else if (speed == SPEED_100) {
        if (duplex == DUPLEX_HALF) {
            auto_nego |= ADVERTISE_100HALF;
        } else {
            auto_nego |=  ADVERTISE_100FULL;
        }
    } else { /* speed == SPEED_10 */
        if (duplex == DUPLEX_HALF) {
            auto_nego |= ADVERTISE_10HALF;
        } else {
            auto_nego |= ADVERTISE_10FULL;
        }
    }
    /* Set flow control support. */
    if (flowCtl == kFlowControlOn)
        auto_nego |= (ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);

    tp->phy_auto_nego_reg = auto_nego;
    tp->phy_1000_ctrl_reg = giga_ctrl;

    tp->phy_2500_ctrl_reg = ctrl_2500;

    rtl8125_mdio_write(tp, 0x1f, 0x0000);
    rtl8125_mdio_write(tp, MII_ADVERTISE, auto_nego);
    rtl8125_mdio_write(tp, MII_CTRL1000, giga_ctrl);
    mdio_direct_write_phy_ocp(tp, 0xA5D4, ctrl_2500);
    rtl8125_phy_restart_nway(tp);
    mdelay(20);

    tp->autoneg = AUTONEG_ENABLE;
    tp->speed = speed;
    tp->duplex = duplex;
}

/* Set PCI configuration space offset 0x79 to setting. */

void LucyRTL8125::setOffset79(UInt8 setting)
{
    UInt8 deviceControl;
    
    DebugLog("setOffset79() ===>\n");
    
    if (!(linuxData.hwoptimize & HW_PATCH_SOC_LAN)) {
        deviceControl = pciDevice->configRead8(0x79);
        deviceControl &= ~0x70;
        deviceControl |= setting;
        pciDevice->configWrite8(0x79, deviceControl);
    }
    
    DebugLog("setOffset79() <===\n");
}

UInt8 LucyRTL8125::csiFun0ReadByte(UInt32 addr)
{
    struct rtl8125_private *tp = &linuxData;
    UInt8 retVal = 0;
    
    if (tp->mcfg == CFG_METHOD_DEFAULT) {
        retVal = pciDevice->configRead8(addr);
    } else {
        UInt32 tmpUlong;
        UInt8 shiftByte;
        
        shiftByte = addr & (0x3);
        tmpUlong = rtl8125_csi_other_fun_read(&linuxData, 0, addr);
        tmpUlong >>= (8 * shiftByte);
        retVal = (UInt8)tmpUlong;
    }
    udelay(20);

    return retVal;
}

void LucyRTL8125::csiFun0WriteByte(UInt32 addr, UInt8 value)
{
    struct rtl8125_private *tp = &linuxData;

    if (tp->mcfg == CFG_METHOD_DEFAULT) {
        pciDevice->configWrite8(addr, value);
    } else {
        UInt32 tmpUlong;
        UInt16 regAlignAddr;
        UInt8 shiftByte;
        
        regAlignAddr = addr & ~(0x3);
        shiftByte = addr & (0x3);
        tmpUlong = rtl8125_csi_other_fun_read(&linuxData, 0, regAlignAddr);
        tmpUlong &= ~(0xFF << (8 * shiftByte));
        tmpUlong |= (value << (8 * shiftByte));
        rtl8125_csi_other_fun_write(&linuxData, 0, regAlignAddr, tmpUlong );
    }
    udelay(20);
}

void LucyRTL8125::enablePCIOffset99()
{
    struct rtl8125_private *tp = &linuxData;
    u32 csi_tmp;
    
    switch (linuxData.mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            csiFun0WriteByte(0x99, linuxData.org_pci_offset_99);
            break;
    }
    
    switch (linuxData.mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            csi_tmp = rtl8125_mac_ocp_read(tp, 0xE032);
            csi_tmp &= ~(BIT_0 | BIT_1);
            
            if (!(tp->org_pci_offset_99 & (BIT_5 | BIT_6)))
                    csi_tmp |= BIT_1;
            
            if (!(tp->org_pci_offset_99 & BIT_2))
                    csi_tmp |= BIT_0;
            
            rtl8125_mac_ocp_write(tp, 0xE032, csi_tmp);
            break;
    }
}

void LucyRTL8125::disablePCIOffset99()
{
    struct rtl8125_private *tp = &linuxData;

    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            rtl8125_mac_ocp_write(tp, 0xE032,  rtl8125_mac_ocp_read(tp, 0xE032) & ~(BIT_0 | BIT_1));
            break;
    }

    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            csiFun0WriteByte(0x99, 0x00);
            break;
    }
}

void LucyRTL8125::initPCIOffset99()
{
    struct rtl8125_private *tp = &linuxData;
    u32 csi_tmp;

    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            rtl8125_mac_ocp_write(tp, 0xCDD0, 0x9003);
            csi_tmp = rtl8125_mac_ocp_read(tp, 0xE034);
            csi_tmp |= (BIT_15 | BIT_14);
            rtl8125_mac_ocp_write(tp, 0xE034, csi_tmp);
            rtl8125_mac_ocp_write(tp, 0xCDD2, 0x889C);
            rtl8125_mac_ocp_write(tp, 0xCDD8, 0x9003);
            rtl8125_mac_ocp_write(tp, 0xCDD4, 0x8C30);
            rtl8125_mac_ocp_write(tp, 0xCDDA, 0x9003);
            rtl8125_mac_ocp_write(tp, 0xCDD6, 0x9003);
            rtl8125_mac_ocp_write(tp, 0xCDDC, 0x9003);
            rtl8125_mac_ocp_write(tp, 0xCDE8, 0x883E);
            rtl8125_mac_ocp_write(tp, 0xCDEA, 0x9003);
            rtl8125_mac_ocp_write(tp, 0xCDEC, 0x889C);
            rtl8125_mac_ocp_write(tp, 0xCDEE, 0x9003);
            rtl8125_mac_ocp_write(tp, 0xCDF0, 0x8C09);
            rtl8125_mac_ocp_write(tp, 0xCDF2, 0x9003);
            csi_tmp = rtl8125_mac_ocp_read(tp, 0xE032);
            csi_tmp |= (BIT_14);
            rtl8125_mac_ocp_write(tp, 0xE032, csi_tmp);
            csi_tmp = rtl8125_mac_ocp_read(tp, 0xE0A2);
            csi_tmp |= (BIT_0);
            rtl8125_mac_ocp_write(tp, 0xE0A2, csi_tmp);
            break;
    }
    enablePCIOffset99();
}

void LucyRTL8125::setPCI99_180ExitDriverPara()
{
    struct rtl8125_private *tp = &linuxData;
    
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
                rtl8125_issue_offset_99_event(tp);
            break;
    }
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            disablePCIOffset99();
            break;
    }
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            rtl8125_disable_pci_offset_180(tp);
            break;
    }
}

void LucyRTL8125::hardwareD3Para()
{
    struct rtl8125_private *tp = &linuxData;
    
    /* Set RxMaxSize register */
    WriteReg16(RxMaxSize, RX_BUF_SIZE);
    
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            WriteReg8(0xF1, ReadReg8(0xF1) & ~BIT_7);
            WriteReg8(Cfg9346, ReadReg8(Cfg9346) | Cfg9346_Unlock);
            WriteReg8(Config2, ReadReg8(Config2) & ~BIT_7);
            WriteReg8(Config5, ReadReg8(Config5) & ~BIT_0);
            WriteReg8(Cfg9346, ReadReg8(Cfg9346) & ~Cfg9346_Unlock);
            break;
    }
    rtl8125_disable_exit_l1_mask(tp);

    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            rtl8125_mac_ocp_write(tp, 0xEA18, 0x0064);
            break;
    }
    setPCI99_180ExitDriverPara();

    /*disable ocp phy power saving*/
    if (tp->mcfg == CFG_METHOD_2 || tp->mcfg == CFG_METHOD_3 ||
        tp->mcfg == CFG_METHOD_4 || tp->mcfg == CFG_METHOD_5) {
            rtl8125_disable_ocp_phy_power_saving(tp);
    }
    rtl8125_disable_rxdvgate(tp);
}

UInt16 LucyRTL8125::getEEEMode()
{
    struct rtl8125_private *tp = &linuxData;
    UInt16 eee = 0;
    UInt16 sup, adv, lpa, ena;

    if (eeeCap) {
        /* Get supported EEE. */
        sup = mdio_direct_read_phy_ocp(tp, 0xA5C4);
        DebugLog("EEE supported: %u\n", sup);

        /* Get advertisement EEE. */
        adv = mdio_direct_read_phy_ocp(tp, 0xA5D0);
        DebugLog("EEE advertised: %u\n", adv);

        /* Get LP advertisement EEE. */
        lpa = mdio_direct_read_phy_ocp(tp, 0xA5D2);
        DebugLog("EEE link partner: %u\n", lpa);

        ena = rtl8125_mac_ocp_read(tp, 0xE040);
        ena &= BIT_1 | BIT_0;
        DebugLog("EEE enabled: %u\n", ena);

        eee = (sup & adv & lpa);
    }
    return eee;
}
void LucyRTL8125::exitOOB()
{
    struct rtl8125_private *tp = &linuxData;
    UInt16 data16;
    
    WriteReg32(RxConfig, ReadReg32(RxConfig) & ~(AcceptErr | AcceptRunt | AcceptBroadcast | AcceptMulticast | AcceptMyPhys |  AcceptAllPhys));
    
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            //rtl8125_dash2_disable_txrx(tp);
            break;
    }

    //Disable realwow  function
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            rtl8125_mac_ocp_write(tp, 0xC0BC, 0x00FF);
            break;
    }

    rtl8125_nic_reset(tp);

    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            rtl8125_disable_now_is_oob(tp);

            data16 = rtl8125_mac_ocp_read(tp, 0xE8DE) & ~BIT_14;
            rtl8125_mac_ocp_write(tp, 0xE8DE, data16);
            rtl8125_wait_ll_share_fifo_ready(tp);

            rtl8125_mac_ocp_write(tp, 0xC0AA, 0x07D0);
            rtl8125_mac_ocp_write(tp, 0xC0A6, 0x01B5);
            rtl8125_mac_ocp_write(tp, 0xC01E, 0x5555);

            rtl8125_wait_ll_share_fifo_ready(tp);
            break;
    }

    //wait ups resume (phy state 2)
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            if (rtl8125_is_ups_resume(tp)) {
                rtl8125_wait_phy_ups_resume(tp, 2);
                rtl8125_clear_ups_resume_bit(tp);
                rtl8125_clear_phy_ups_reg(tp);
            }
            break;
    };
    tp->phy_reg_anlpar = 0;
}

void LucyRTL8125::powerDownPLL()
{
    struct rtl8125_private *tp = &linuxData;

    if (tp->wol_enabled == WOL_ENABLED || tp->DASH || tp->EnableKCPOffload) {
        int auto_nego;
        int giga_ctrl;
        u16 anlpar;

        rtl8125_set_hw_wol(tp, tp->wol_opts);

        if (tp->mcfg == CFG_METHOD_2 || tp->mcfg == CFG_METHOD_3 ||
            tp->mcfg == CFG_METHOD_4 || tp->mcfg == CFG_METHOD_5) {
            WriteReg8(Cfg9346, ReadReg8(Cfg9346) | Cfg9346_Unlock);
            WriteReg8(Config2, ReadReg8(Config2) | PMSTS_En);
            WriteReg8(Cfg9346, ReadReg8(Cfg9346) & ~Cfg9346_Unlock);
        }

        rtl8125_mdio_write(tp, 0x1F, 0x0000);
        auto_nego = rtl8125_mdio_read(tp, MII_ADVERTISE);
        auto_nego &= ~(ADVERTISE_10HALF | ADVERTISE_10FULL
                       | ADVERTISE_100HALF | ADVERTISE_100FULL);

        if (isEnabled)
            anlpar = tp->phy_reg_anlpar;
        else
            anlpar = rtl8125_mdio_read(tp, MII_LPA);

        if (anlpar & (LPA_10HALF | LPA_10FULL))
            auto_nego |= (ADVERTISE_10HALF | ADVERTISE_10FULL);
        else
            auto_nego |= (ADVERTISE_100FULL | ADVERTISE_100HALF | ADVERTISE_10HALF | ADVERTISE_10FULL);

        if (tp->DASH)
            auto_nego |= (ADVERTISE_100FULL | ADVERTISE_100HALF | ADVERTISE_10HALF | ADVERTISE_10FULL);

            giga_ctrl = rtl8125_mdio_read(tp, MII_CTRL1000) & ~(ADVERTISE_1000HALF | ADVERTISE_1000FULL);
            rtl8125_mdio_write(tp, MII_ADVERTISE, auto_nego);
            rtl8125_mdio_write(tp, MII_CTRL1000, giga_ctrl);
        
            if (tp->mcfg == CFG_METHOD_2 || tp->mcfg == CFG_METHOD_3 ||
                tp->mcfg == CFG_METHOD_4 || tp->mcfg == CFG_METHOD_5) {
                int ctrl_2500;

                ctrl_2500 = mdio_direct_read_phy_ocp(tp, 0xA5D4);
                ctrl_2500 &= ~(RTK_ADVERTISE_2500FULL);
                mdio_direct_write_phy_ocp(tp, 0xA5D4, ctrl_2500);
            }
            rtl8125_phy_restart_nway(tp);

            WriteReg32(RxConfig, ReadReg32(RxConfig) | AcceptBroadcast | AcceptMulticast | AcceptMyPhys);

            return;
        }

        if (tp->DASH)
                return;

        rtl8125_phy_power_down(tp);

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            WriteReg8(PMCH, ReadReg8(PMCH) & ~BIT_7);
            break;
        }

        switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
            WriteReg8(0xF2, ReadReg8(0xF2) & ~BIT_6);
            break;
        }
}

void LucyRTL8125::configPhyHardware()
{
    struct rtl8125_private *tp = &linuxData;

    if (tp->resume_not_chg_speed) return;
    
    tp->phy_reset_enable(tp);
    
    if (HW_DASH_SUPPORT_TYPE_3(tp) && tp->HwPkgDet == 0x06) return;
    
    rtl8125_set_hw_phy_before_init_phy_mcu(tp);
    
    rtl8125_init_hw_phy_mcu(tp);
    
    switch (tp->mcfg) {
        case CFG_METHOD_2:
            configPhyHardware8125a1();
            break;
        case CFG_METHOD_3:
            configPhyHardware8125a2();
            break;
        case CFG_METHOD_4:
            configPhyHardware8125b1();
            break;
        case CFG_METHOD_5:
            configPhyHardware8125b2();
            break;
    }
    
    //legacy force mode(Chap 22)
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        default:
            rtl8125_mdio_write(tp, 0x1F, 0x0A5B);
            rtl8125_clear_eth_phy_bit(tp, 0x12, BIT_15);
            rtl8125_mdio_write(tp, 0x1F, 0x0000);
            break;
    }
    
    /*ocp phy power saving*/
    /*
     if (aspm) {
     if (tp->mcfg == CFG_METHOD_2 || tp->mcfg == CFG_METHOD_3)
     rtl8125_enable_ocp_phy_power_saving(dev);
     }
     */
    
    rtl8125_mdio_write(tp, 0x1F, 0x0000);
    
    if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
        if (tp->eee_enabled == 1)
            rtl8125_enable_eee(tp);
        else
            rtl8125_disable_eee(tp);
    }
}

void LucyRTL8125::configPhyHardware8125a1()
{
    struct rtl8125_private *tp = &linuxData;

    ClearAndSetEthPhyOcpBit(tp,
                            0xAD40,
                            0x03FF,
                            0x84
                            );
    
    SetEthPhyOcpBit(tp, 0xAD4E, BIT_4);
    ClearAndSetEthPhyOcpBit(tp,
                            0xAD16,
                            0x03FF,
                            0x0006
                            );
    ClearAndSetEthPhyOcpBit(tp,
                            0xAD32,
                            0x003F,
                            0x0006
                            );
    ClearEthPhyOcpBit(tp, 0xAC08, BIT_12);
    ClearEthPhyOcpBit(tp, 0xAC08, BIT_8);
    ClearAndSetEthPhyOcpBit(tp,
                            0xAC8A,
                            BIT_15|BIT_14|BIT_13|BIT_12,
                            BIT_14|BIT_13|BIT_12
                            );
    SetEthPhyOcpBit(tp, 0xAD18, BIT_10);
    SetEthPhyOcpBit(tp, 0xAD1A, 0x3FF);
    SetEthPhyOcpBit(tp, 0xAD1C, 0x3FF);
    
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80EA);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0xC400
                            );
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80EB);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0x0700,
                            0x0300
                            );
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80F8);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0x1C00
                            );
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80F1);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0x3000
                            );
    
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80FE);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0xA500
                            );
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x8102);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0x5000
                            );
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x8105);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0x3300
                            );
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x8100);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0x7000
                            );
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x8104);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0xF000
                            );
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x8106);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0x6500
                            );
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80DC);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0xED00
                            );
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80DF);
    SetEthPhyOcpBit(tp, 0xA438, BIT_8);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80E1);
    ClearEthPhyOcpBit(tp, 0xA438, BIT_8);
    
    ClearAndSetEthPhyOcpBit(tp,
                            0xBF06,
                            0x003F,
                            0x38
                            );
    
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x819F);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0xD0B6);
    
    mdio_direct_write_phy_ocp(tp, 0xBC34, 0x5555);
    ClearAndSetEthPhyOcpBit(tp,
                            0xBF0A,
                            BIT_11|BIT_10|BIT_9,
                            BIT_11|BIT_9
                            );
    
    ClearEthPhyOcpBit(tp, 0xA5C0, BIT_10);
    
    SetEthPhyOcpBit(tp, 0xA442, BIT_11);
    
    //enable aldps
    //GPHY OCP 0xA430 bit[2] = 0x1 (en_aldps)
    if (aspm) {
        if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
            rtl8125_enable_phy_aldps(tp);
        }
    }
}

void LucyRTL8125::configPhyHardware8125a2()
{
    struct rtl8125_private *tp = &linuxData;

    SetEthPhyOcpBit(tp, 0xAD4E, BIT_4);
    ClearAndSetEthPhyOcpBit(tp,
                            0xAD16,
                            0x03FF,
                            0x03FF
                            );
    ClearAndSetEthPhyOcpBit(tp,
                            0xAD32,
                            0x003F,
                            0x0006
                            );
    ClearEthPhyOcpBit(tp, 0xAC08, BIT_12);
    ClearEthPhyOcpBit(tp, 0xAC08, BIT_8);
    ClearAndSetEthPhyOcpBit(tp,
                            0xACC0,
                            BIT_1|BIT_0,
                            BIT_1
                            );
    ClearAndSetEthPhyOcpBit(tp,
                            0xAD40,
                            BIT_7|BIT_6|BIT_5,
                            BIT_6
                            );
    ClearAndSetEthPhyOcpBit(tp,
                            0xAD40,
                            BIT_2|BIT_1|BIT_0,
                            BIT_2
                            );
    ClearEthPhyOcpBit(tp, 0xAC14, BIT_7);
    ClearEthPhyOcpBit(tp, 0xAC80, BIT_9|BIT_8);
    ClearAndSetEthPhyOcpBit(tp,
                            0xAC5E,
                            BIT_2|BIT_1|BIT_0,
                            BIT_1
                            );
    mdio_direct_write_phy_ocp(tp, 0xAD4C, 0x00A8);
    mdio_direct_write_phy_ocp(tp, 0xAC5C, 0x01FF);
    ClearAndSetEthPhyOcpBit(tp,
                            0xAC8A,
                            BIT_7|BIT_6|BIT_5|BIT_4,
                            BIT_5|BIT_4
                            );
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8157);
    ClearAndSetEthPhyOcpBit(tp,
                            0xB87E,
                            0xFF00,
                            0x0500
                            );
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8159);
    ClearAndSetEthPhyOcpBit(tp,
                            0xB87E,
                            0xFF00,
                            0x0700
                            );
    
    WriteReg16(EEE_TXIDLE_TIMER_8125, mtu + ETH_HLEN + 0x20);
    
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x80A2);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0153);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x809C);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0153);
    
    
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x81B3);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0043);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x00A7);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x00D6);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x00EC);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x00F6);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x00FB);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x00FD);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x00FF);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x00BB);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0058);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0029);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0013);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0009);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0004);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0002);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0000);
    
    
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x8257);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x020F);
    
    
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80EA);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x7843);
    
    
    rtl8125_set_phy_mcu_patch_request(tp);
    
    ClearEthPhyOcpBit(tp, 0xB896, BIT_0);
    ClearEthPhyOcpBit(tp, 0xB892, 0xFF00);
    
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC091);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x6E12);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC092);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x1214);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC094);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x1516);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC096);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x171B);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC098);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x1B1C);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC09A);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x1F1F);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC09C);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x2021);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC09E);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x2224);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC0A0);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x2424);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC0A2);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x2424);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC0A4);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x2424);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC018);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x0AF2);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC01A);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x0D4A);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC01C);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x0F26);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC01E);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x118D);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC020);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x14F3);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC022);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x175A);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC024);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x19C0);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC026);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x1C26);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC089);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x6050);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC08A);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x5F6E);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC08C);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x6E6E);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC08E);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x6E6E);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC090);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x6E12);
    
    SetEthPhyOcpBit(tp, 0xB896, BIT_0);
    
    rtl8125_clear_phy_mcu_patch_request(tp);
    
    
    SetEthPhyOcpBit(tp, 0xD068, BIT_13);
    
    
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x81A2);
    SetEthPhyOcpBit(tp, 0xA438, BIT_8);
    ClearAndSetEthPhyOcpBit(tp,
                            0xB54C,
                            0xFF00,
                            0xDB00);
    
    
    ClearEthPhyOcpBit(tp, 0xA454, BIT_0);
    
    
    SetEthPhyOcpBit(tp, 0xA5D4, BIT_5);
    ClearEthPhyOcpBit(tp, 0xAD4E, BIT_4);
    ClearEthPhyOcpBit(tp, 0xA86A, BIT_0);
    
    
    SetEthPhyOcpBit(tp, 0xA442, BIT_11);
    
    
    if (tp->RequirePhyMdiSwapPatch) {
        u16 adccal_offset_p0;
        u16 adccal_offset_p1;
        u16 adccal_offset_p2;
        u16 adccal_offset_p3;
        u16 rg_lpf_cap_xg_p0;
        u16 rg_lpf_cap_xg_p1;
        u16 rg_lpf_cap_xg_p2;
        u16 rg_lpf_cap_xg_p3;
        u16 rg_lpf_cap_p0;
        u16 rg_lpf_cap_p1;
        u16 rg_lpf_cap_p2;
        u16 rg_lpf_cap_p3;
        
        ClearAndSetEthPhyOcpBit(tp,
                                0xD068,
                                0x0007,
                                0x0001
                                );
        ClearAndSetEthPhyOcpBit(tp,
                                0xD068,
                                0x0018,
                                0x0000
                                );
        adccal_offset_p0 = mdio_direct_read_phy_ocp(tp, 0xD06A);
        adccal_offset_p0 &= 0x07FF;
        ClearAndSetEthPhyOcpBit(tp,
                                0xD068,
                                0x0018,
                                0x0008
                                );
        adccal_offset_p1 = mdio_direct_read_phy_ocp(tp, 0xD06A);
        adccal_offset_p1 &= 0x07FF;
        ClearAndSetEthPhyOcpBit(tp,
                                0xD068,
                                0x0018,
                                0x0010
                                );
        adccal_offset_p2 = mdio_direct_read_phy_ocp(tp, 0xD06A);
        adccal_offset_p2 &= 0x07FF;
        ClearAndSetEthPhyOcpBit(tp,
                                0xD068,
                                0x0018,
                                0x0018
                                );
        adccal_offset_p3 = mdio_direct_read_phy_ocp(tp, 0xD06A);
        adccal_offset_p3 &= 0x07FF;
        
        
        ClearAndSetEthPhyOcpBit(tp,
                                0xD068,
                                0x0018,
                                0x0000
                                );
        ClearAndSetEthPhyOcpBit(tp,
                                0xD06A,
                                0x07FF,
                                adccal_offset_p3
                                );
        ClearAndSetEthPhyOcpBit(tp,
                                0xD068,
                                0x0018,
                                0x0008
                                );
        ClearAndSetEthPhyOcpBit(tp,
                                0xD06A,
                                0x07FF,
                                adccal_offset_p2
                                );
        ClearAndSetEthPhyOcpBit(tp,
                                0xD068,
                                0x0018,
                                0x0010
                                );
        ClearAndSetEthPhyOcpBit(tp,
                                0xD06A,
                                0x07FF,
                                adccal_offset_p1
                                );
        ClearAndSetEthPhyOcpBit(tp,
                                0xD068,
                                0x0018,
                                0x0018
                                );
        ClearAndSetEthPhyOcpBit(tp,
                                0xD06A,
                                0x07FF,
                                adccal_offset_p0
                                );
        
        
        rg_lpf_cap_xg_p0 = mdio_direct_read_phy_ocp(tp, 0xBD5A);
        rg_lpf_cap_xg_p0 &= 0x001F;
        rg_lpf_cap_xg_p1 = mdio_direct_read_phy_ocp(tp, 0xBD5A);
        rg_lpf_cap_xg_p1 &= 0x1F00;
        rg_lpf_cap_xg_p2 = mdio_direct_read_phy_ocp(tp, 0xBD5C);
        rg_lpf_cap_xg_p2 &= 0x001F;
        rg_lpf_cap_xg_p3 = mdio_direct_read_phy_ocp(tp, 0xBD5C);
        rg_lpf_cap_xg_p3 &= 0x1F00;
        rg_lpf_cap_p0 = mdio_direct_read_phy_ocp(tp, 0xBC18);
        rg_lpf_cap_p0 &= 0x001F;
        rg_lpf_cap_p1 = mdio_direct_read_phy_ocp(tp, 0xBC18);
        rg_lpf_cap_p1 &= 0x1F00;
        rg_lpf_cap_p2 = mdio_direct_read_phy_ocp(tp, 0xBC1A);
        rg_lpf_cap_p2 &= 0x001F;
        rg_lpf_cap_p3 = mdio_direct_read_phy_ocp(tp, 0xBC1A);
        rg_lpf_cap_p3 &= 0x1F00;
        
        
        ClearAndSetEthPhyOcpBit(tp,
                                0xBD5A,
                                0x001F,
                                rg_lpf_cap_xg_p3 >> 8
                                );
        ClearAndSetEthPhyOcpBit(tp,
                                0xBD5A,
                                0x1F00,
                                rg_lpf_cap_xg_p2 << 8
                                );
        ClearAndSetEthPhyOcpBit(tp,
                                0xBD5C,
                                0x001F,
                                rg_lpf_cap_xg_p1 >> 8
                                );
        ClearAndSetEthPhyOcpBit(tp,
                                0xBD5C,
                                0x1F00,
                                rg_lpf_cap_xg_p0 << 8
                                );
        ClearAndSetEthPhyOcpBit(tp,
                                0xBC18,
                                0x001F,
                                rg_lpf_cap_p3 >> 8
                                );
        ClearAndSetEthPhyOcpBit(tp,
                                0xBC18,
                                0x1F00,
                                rg_lpf_cap_p2 << 8
                                );
        ClearAndSetEthPhyOcpBit(tp,
                                0xBC1A,
                                0x001F,
                                rg_lpf_cap_p1 >> 8
                                );
        ClearAndSetEthPhyOcpBit(tp,
                                0xBC1A,
                                0x1F00,
                                rg_lpf_cap_p0 << 8
                                );
    }
    
    
    if (aspm) {
        if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
            rtl8125_enable_phy_aldps(tp);
        }
    }
}

void LucyRTL8125::configPhyHardware8125b1()
{
    struct rtl8125_private *tp = &linuxData;

    SetEthPhyOcpBit(tp, 0xA442, BIT_11);
    
    
    SetEthPhyOcpBit(tp, 0xBC08, (BIT_3 | BIT_2));
    
    
    if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x8FFF);
        ClearAndSetEthPhyOcpBit(tp,
                                0xA438,
                                0xFF00,
                                0x0400
                                );
    }
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8560);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x19CC);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8562);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x19CC);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8564);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x19CC);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8566);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x147D);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8568);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x147D);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x856A);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x147D);
    if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
        mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FFE);
        mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0907);
    }
    ClearAndSetEthPhyOcpBit(tp,
                            0xACDA,
                            0xFF00,
                            0xFF00
                            );
    ClearAndSetEthPhyOcpBit(tp,
                            0xACDE,
                            0xF000,
                            0xF000
                            );
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x80D6);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x2801);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x80F2);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x2801);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x80F4);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x6077);
    mdio_direct_write_phy_ocp(tp, 0xB506, 0x01E7);
    mdio_direct_write_phy_ocp(tp, 0xAC8C, 0x0FFC);
    mdio_direct_write_phy_ocp(tp, 0xAC46, 0xB7B4);
    mdio_direct_write_phy_ocp(tp, 0xAC50, 0x0FBC);
    mdio_direct_write_phy_ocp(tp, 0xAC3C, 0x9240);
    mdio_direct_write_phy_ocp(tp, 0xAC4E, 0x0DB4);
    mdio_direct_write_phy_ocp(tp, 0xACC6, 0x0707);
    mdio_direct_write_phy_ocp(tp, 0xACC8, 0xA0D3);
    mdio_direct_write_phy_ocp(tp, 0xAD08, 0x0007);
    
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8013);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0700);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FB9);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x2801);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FBA);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0100);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FBC);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x1900);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FBE);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0xE100);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FC0);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0800);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FC2);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0xE500);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FC4);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0F00);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FC6);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0xF100);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FC8);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0400);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FCa);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0xF300);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FCc);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0xFD00);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FCe);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0xFF00);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FD0);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0xFB00);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FD2);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0100);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FD4);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0xF400);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FD6);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0xFF00);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8FD8);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0xF600);
    
    
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x813D);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x390E);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x814F);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x790E);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x80B0);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0F31);
    SetEthPhyOcpBit(tp, 0xBF4C, BIT_1);
    SetEthPhyOcpBit(tp, 0xBCCA, (BIT_9 | BIT_8));
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8141);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x320E);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8153);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x720E);
    ClearEthPhyOcpBit(tp, 0xA432, BIT_6);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8529);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x050E);
    
    WriteReg16(EEE_TXIDLE_TIMER_8125, mtu + ETH_HLEN + 0x20);

    mdio_direct_write_phy_ocp(tp, 0xA436, 0x816C);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0xC4A0);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x8170);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0xC4A0);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x8174);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x04A0);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x8178);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x04A0);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x817C);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0719);
    if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x8FF4);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0400);
        mdio_direct_write_phy_ocp(tp, 0xA436, 0x8FF1);
        mdio_direct_write_phy_ocp(tp, 0xA438, 0x0404);
    }
    mdio_direct_write_phy_ocp(tp, 0xBF4A, 0x001B);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8033);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x7C13);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8037);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x7C13);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x803B);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0xFC32);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x803F);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x7C13);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8043);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x7C13);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8047);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x7C13);
    
    
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8145);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x370E);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8157);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x770E);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8169);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x0D0A);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x817B);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x1D0A);
    
    
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x8217);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0x5000
                            );
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x821A);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0x5000
                            );
    
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80DA);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0403);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80DC);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0x1000
                            );
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80B3);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x0384);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80B7);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x2007);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80BA);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0x6C00
                            );
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80B5);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0xF009);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80BD);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0x9F00
                            );
    
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80C7);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0xf083);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80DD);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x03f0);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80DF);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0x1000
                            );
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80CB);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x2007);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80CE);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0x6C00
                            );
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80C9);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x8009);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80D1);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0x8000
                            );
    
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80A3);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x200A);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80A5);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0xF0AD);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x809F);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x6073);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80A1);
    mdio_direct_write_phy_ocp(tp, 0xA438, 0x000B);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x80A9);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            0xFF00,
                            0xC000
                            );
    
    rtl8125_set_phy_mcu_patch_request(tp);
    
    ClearEthPhyOcpBit(tp, 0xB896, BIT_0);
    ClearEthPhyOcpBit(tp, 0xB892, 0xFF00);
    
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC23E);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x0000);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC240);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x0103);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC242);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x0507);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC244);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x090B);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC246);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x0C0E);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC248);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x1012);
    mdio_direct_write_phy_ocp(tp, 0xB88E, 0xC24A);
    mdio_direct_write_phy_ocp(tp, 0xB890, 0x1416);
    
    SetEthPhyOcpBit(tp, 0xB896, BIT_0);
    
    rtl8125_clear_phy_mcu_patch_request(tp);
    
    
    SetEthPhyOcpBit(tp, 0xA86A, BIT_0);
    SetEthPhyOcpBit(tp, 0xA6F0, BIT_0);
    
    
    mdio_direct_write_phy_ocp(tp, 0xBFA0, 0xD70D);
    mdio_direct_write_phy_ocp(tp, 0xBFA2, 0x4100);
    mdio_direct_write_phy_ocp(tp, 0xBFA4, 0xE868);
    mdio_direct_write_phy_ocp(tp, 0xBFA6, 0xDC59);
    mdio_direct_write_phy_ocp(tp, 0xB54C, 0x3C18);
    ClearEthPhyOcpBit(tp, 0xBFA4, BIT_5);
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x817D);
    SetEthPhyOcpBit(tp, 0xA438, BIT_12);
    
    
    if (aspm) {
        if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
            rtl8125_enable_phy_aldps(tp);
        }
    }
}

void LucyRTL8125::configPhyHardware8125b2()
{
    struct rtl8125_private *tp = &linuxData;
    
    SetEthPhyOcpBit(tp, 0xA442, BIT_11);
    
    
    ClearAndSetEthPhyOcpBit(tp,
                            0xAC46,
                            0x00F0,
                            0x0090
                            );
    ClearAndSetEthPhyOcpBit(tp,
                            0xAD30,
                            0x0003,
                            0x0001
                            );
    
    
    WriteReg16(EEE_TXIDLE_TIMER_8125, mtu + ETH_HLEN + 0x20);
    
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x80F5);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x760E);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8107);
    mdio_direct_write_phy_ocp(tp, 0xB87E, 0x360E);
    mdio_direct_write_phy_ocp(tp, 0xB87C, 0x8551);
    ClearAndSetEthPhyOcpBit(tp,
                            0xB87E,
                            BIT_15 | BIT_14 | BIT_13 | BIT_12 | BIT_11 | BIT_10 | BIT_9 | BIT_8,
                            BIT_11
                            );
    
    ClearAndSetEthPhyOcpBit(tp,
                            0xbf00,
                            0xE000,
                            0xA000
                            );
    ClearAndSetEthPhyOcpBit(tp,
                            0xbf46,
                            0x0F00,
                            0x0300
                            );
    mdio_direct_write_phy_ocp(tp, 0xa436, 0x8044);
    mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
    mdio_direct_write_phy_ocp(tp, 0xa436, 0x804A);
    mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
    mdio_direct_write_phy_ocp(tp, 0xa436, 0x8050);
    mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
    mdio_direct_write_phy_ocp(tp, 0xa436, 0x8056);
    mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
    mdio_direct_write_phy_ocp(tp, 0xa436, 0x805C);
    mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
    mdio_direct_write_phy_ocp(tp, 0xa436, 0x8062);
    mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
    mdio_direct_write_phy_ocp(tp, 0xa436, 0x8068);
    mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
    mdio_direct_write_phy_ocp(tp, 0xa436, 0x806E);
    mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
    mdio_direct_write_phy_ocp(tp, 0xa436, 0x8074);
    mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
    mdio_direct_write_phy_ocp(tp, 0xa436, 0x807A);
    mdio_direct_write_phy_ocp(tp, 0xa438, 0x2417);
    
    
    SetEthPhyOcpBit(tp, 0xA4CA, BIT_6);
    
    
    ClearAndSetEthPhyOcpBit(tp,
                            0xBF84,
                            BIT_15 | BIT_14 | BIT_13,
                            BIT_15 | BIT_13
                            );
    
    
    mdio_direct_write_phy_ocp(tp, 0xA436, 0x8170);
    ClearAndSetEthPhyOcpBit(tp,
                            0xA438,
                            BIT_13 | BIT_10 | BIT_9 | BIT_8,
                            BIT_15 | BIT_14 | BIT_12 | BIT_11
                            );
    
    /*
     mdio_direct_write_phy_ocp(tp, 0xBFA0, 0xD70D);
     mdio_direct_write_phy_ocp(tp, 0xBFA2, 0x4100);
     mdio_direct_write_phy_ocp(tp, 0xBFA4, 0xE868);
     mdio_direct_write_phy_ocp(tp, 0xBFA6, 0xDC59);
     mdio_direct_write_phy_ocp(tp, 0xB54C, 0x3C18);
     ClearEthPhyOcpBit(tp, 0xBFA4, BIT_5);
     mdio_direct_write_phy_ocp(tp, 0xA436, 0x817D);
     SetEthPhyOcpBit(tp, 0xA438, BIT_12);
     */
    
    
    if (aspm) {
        if (HW_HAS_WRITE_PHY_MCU_RAM_CODE(tp)) {
            rtl8125_enable_phy_aldps(tp);
        }
    }
}
