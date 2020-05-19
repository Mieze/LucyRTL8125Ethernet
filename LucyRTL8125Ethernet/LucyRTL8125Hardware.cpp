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

bool LucyRTL8125::initRTL8125()
{
    struct rtl8125_private *tp = &linuxData;
    UInt32 i;
    UInt8 macAddr[MAC_ADDR_LEN];
    bool result = false;
    
    /* Identify chip attached to board. */
    rtl8125_get_mac_version(tp);
    
    if (tp->mcfg == CFG_METHOD_DEFAULT) {
        DebugLog("Retry chip recognition.\n");
        
        /* In case chip recognition failed clear corresponding bits... */
        WriteReg32(TxConfig, ReadReg32(TxConfig) & ~0x7CF00000);
        
        /* ...and try again. */
        rtl8125_get_mac_version(tp);
    }
    if (tp->mcfg >= CFG_METHOD_MAX) {
        DebugLog("Unsupported chip found. Aborting...\n");
        goto done;
    }
    tp->chipset =  tp->mcfg;
    
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

    if (HW_DASH_SUPPORT_TYPE_3(tp) && tp->HwPkgDet == 0x06)
            eee_enable = 0;

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
    
    switch (tp->mcfg) {
        case CFG_METHOD_2:
        case CFG_METHOD_3:
        case CFG_METHOD_4:
        case CFG_METHOD_5:
        default:
            tp->use_timer_interrrupt = TRUE;
            break;
    }
    tp->use_timer_interrrupt = FALSE;

    switch (tp->mcfg) {
        default:
            tp->SwPaddingShortPktLen = ETH_ZLEN;
            break;
    }
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
    tp->NicCustLedValue = ReadReg16(CustomLED);

    rtl8125_get_hw_wol(tp);

    //rtl8125_link_option((u8*)&autoneg_mode, (u32*)&speed_mode, (u8*)&duplex_mode, (u32*)&advertising_mode);
/*
    tp->autoneg = autoneg_mode;
    tp->speed = speed_mode;
    tp->duplex = duplex_mode;
    tp->advertising = advertising_mode;
*/
    tp->eee_enabled = eee_enable;
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

    /* Set wake on LAN support. */
    wolCapable = (tp->wol_enabled == WOL_ENABLED);
    
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
    rtl8125_hw_phy_config(tp);
    setupRTL8125(intrMitigateValue, true);
    //rtl8168_dsm(tp, DSM_IF_UP);
    
    setPhyMedium();
}

void LucyRTL8125::disableRTL8125()
{
    struct rtl8125_private *tp = &linuxData;
    
    //rtl8168_dsm(tp, DSM_IF_DOWN);
    
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
    RTL_W32(tp, TxConfig, (TX_DMA_BURST_unlimited << TxDMAShift) |
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

            WriteReg8(0x4500, 0x00);
            WriteReg16(0x4800, 0x0000);

            WriteReg8(Config1, ReadReg8(Config1) & ~0x10);

            rtl8125_mac_ocp_write(tp, 0xC140, 0xFFFF);
            rtl8125_mac_ocp_write(tp, 0xC142, 0xFFFF);

            mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xD3E2);
            mac_ocp_data &= 0xF000;
            mac_ocp_data |= 0x3A9;
            rtl8125_mac_ocp_write(tp, 0xD3E2, mac_ocp_data);

            mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xD3E4);
            mac_ocp_data &= 0xFF00;
            rtl8125_mac_ocp_write(tp, 0xD3E4, mac_ocp_data);

            mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xE860);
            mac_ocp_data |= (BIT_7);
            rtl8125_mac_ocp_write(tp, 0xE860, mac_ocp_data);

            /*
             * Disabling the new tx descriptor format saves 16 bytes
             * per descriptor and allows us to reuse code which was
             * written for the RTL8111.
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

            //WriteReg8(tp, 0xD0, ReadReg8(tp, 0xD0) | BIT_7);

            mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xE040);
            mac_ocp_data &= ~(BIT_12);
            rtl8125_mac_ocp_write(tp, 0xE040, mac_ocp_data);

            mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xE0C0);
            mac_ocp_data &= ~(BIT_14 | BIT_11 | BIT_10 | BIT_9 | BIT_8 | BIT_3 | BIT_2 | BIT_1 | BIT_0);
            mac_ocp_data |= (BIT_14 | BIT_10 | BIT_1 | BIT_0);
            rtl8125_mac_ocp_write(tp, 0xE0C0, mac_ocp_data);

            SetMcuAccessRegBit(tp, 0xE052, (BIT_6|BIT_5|BIT_3));
            ClearMcuAccessRegBit(tp, 0xE052, BIT_7);

            mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xC0AC);
            mac_ocp_data &= ~(BIT_7);
            mac_ocp_data |= (BIT_8|BIT_9|BIT_10|BIT_11|BIT_12);
            rtl8125_mac_ocp_write(tp, 0xC0AC, mac_ocp_data);

            mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xD430);
            mac_ocp_data &= ~(BIT_11 | BIT_10 | BIT_9 | BIT_8 | BIT_7 | BIT_6 | BIT_5 | BIT_4 | BIT_3 | BIT_2 | BIT_1 | BIT_0);
            mac_ocp_data |= 0x47F;
            rtl8125_mac_ocp_write(tp, 0xD430, mac_ocp_data);

            //rtl8125_mac_ocp_write(tp, 0xE0C0, 0x4F87);
            mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xE84C);
            mac_ocp_data |= (BIT_7 | BIT_6);
            rtl8125_mac_ocp_write(tp, 0xE84C, mac_ocp_data);

            WriteReg8(0xD0, ReadReg8(0xD0) | BIT_6);

            rtl8125_disable_eee_plus(tp);

            mac_ocp_data = rtl8125_mac_ocp_read(tp, 0xEA1C);
            mac_ocp_data &= ~(BIT_2);
            rtl8125_mac_ocp_write(tp, 0xEA1C, mac_ocp_data);

            SetMcuAccessRegBit(tp, 0xEB54, BIT_0);
            udelay(1);
            ClearMcuAccessRegBit(tp, 0xEB54, BIT_0);
            WriteReg16(0x1880, ReadReg16(0x1880)&~(BIT_4 | BIT_5));
    }
    //other hw parameters
    rtl8125_hw_clear_timer_int(tp);

    rtl8125_hw_clear_int_miti(tp);

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
/*
            rtl8125_hw_set_features(dev, dev->features);
*/

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
    WriteReg16(RxMaxSize, mtu + (ETH_HLEN + ETH_FCS_LEN + VLAN_HLEN));

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
    int use_default = 0;
    
    //Disable Giga Lite
    ClearEthPhyOcpBit(tp, 0xA428, BIT_9);
    ClearEthPhyOcpBit(tp, 0xA5EA, BIT_0);

    if (speed != SPEED_2500 && (speed != SPEED_1000) &&
        (speed != SPEED_100) && (speed != SPEED_10)) {
        speed = SPEED_2500;
        duplex = DUPLEX_FULL;
        autoneg = AUTONEG_ENABLE;
        use_default = 1;
    }
    giga_ctrl = rtl8125_mdio_read(tp, MII_CTRL1000);
    giga_ctrl &= ~(ADVERTISE_1000HALF | ADVERTISE_1000FULL);
    ctrl_2500 = mdio_direct_read_phy_ocp(tp, 0xA5D4);
    ctrl_2500 &= ~(RTK_ADVERTISE_2500FULL);

    /* Enable or disable EEE support according to selected medium. */
    if ((linuxData.eee_adv_t != 0) && (autoneg == AUTONEG_ENABLE)) {
        rtl8125_enable_eee(tp);
        DebugLog("Enable EEE support.\n");
    } else {
        rtl8125_disable_eee(tp);
        DebugLog("Disable EEE support.\n");
    }
    if (autoneg == AUTONEG_ENABLE) {
        /*n-way force*/
        auto_nego = rtl8125_mdio_read(tp, MII_ADVERTISE);
        auto_nego &= ~(ADVERTISE_10HALF | ADVERTISE_10FULL |
                       ADVERTISE_100HALF | ADVERTISE_100FULL |
                       ADVERTISE_PAUSE_CAP | ADVERTISE_PAUSE_ASYM);

        if (speed == SPEED_2500) {
            if (use_default) {
                /* The default medium has been selected. */
                auto_nego |= (ADVERTISE_10HALF | ADVERTISE_10FULL | ADVERTISE_100HALF | ADVERTISE_100FULL);
                giga_ctrl |= (ADVERTISE_1000HALF | ADVERTISE_1000FULL);
                ctrl_2500 |= RTK_ADVERTISE_2500FULL;
            } else {
                ctrl_2500 |= RTK_ADVERTISE_2500FULL;
            }
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
    } else {
        /*true force*/
        if (speed == SPEED_10 || speed == SPEED_100 ||
            (speed == SPEED_1000 && duplex == DUPLEX_FULL &&
             tp->HwSuppGigaForceMode)) {
                rtl8125_phy_setup_force_mode(tp, speed, duplex);
        }
    }

    tp->autoneg = autoneg;
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
            rtl8125_mac_ocp_write(tp, 0xCDD0, 0x9003);
            csi_tmp = rtl8125_mac_ocp_read(tp, 0xE034);
            csi_tmp |= (BIT_15|BIT_14);
            rtl8125_mac_ocp_write(tp, 0xE034, csi_tmp);
            rtl8125_mac_ocp_write(tp, 0xCDD8, 0x9003);
            rtl8125_mac_ocp_write(tp, 0xCDDA, 0x9003);
            rtl8125_mac_ocp_write(tp, 0xCDDC, 0x9003);
            rtl8125_mac_ocp_write(tp, 0xCDD2, 0x883C);
            rtl8125_mac_ocp_write(tp, 0xCDD4, 0x8C12);
            rtl8125_mac_ocp_write(tp, 0xCDD6, 0x9003);
            csi_tmp = rtl8125_mac_ocp_read(tp, 0xE032);
            csi_tmp |= (BIT_14);
            rtl8125_mac_ocp_write(tp, 0xE032, csi_tmp);
            csi_tmp = rtl8125_mac_ocp_read(tp, 0xE0A2);
            csi_tmp |= (BIT_0);
            rtl8125_mac_ocp_write(tp, 0xE0A2, csi_tmp);
            break;
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

