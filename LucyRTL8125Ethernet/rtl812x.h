//
//  rtl812x.h
//  RTL812xLucy
//
//  Created by Laura Müller on 21.01.26.
//
// This driver is based on version 9.016.01 of Realtek's r8125 driver.

#ifndef rtl812x_h
#define rtl812x_h

#include "linux/if_ether.h"
#include "linux/if_vlan.h"
#include "linux/ethtool.h"
#include "linux/uapi/linux/mii.h"
#include "linux/uapi/linux/mdio.h"

struct RtlChipInfo {
    const char *name;
    const char *speed_name;
    UInt8 mcfg;
    UInt32 RCR_Cfg;
    UInt32 RxConfigMask;    /* Clears the bits supported by this chip */
    UInt32 jumbo_frame_sz;
};

#define NUM_CHIPS 16

#define FIRMWARE_8125A_3    "rtl8125a-3.fw"
#define FIRMWARE_8125B_1    "rtl8125b-1.fw"
#define FIRMWARE_8125B_2    "rtl8125b-2.fw"
#define FIRMWARE_8125BP_1   "rtl8125bp-1.fw"
#define FIRMWARE_8125BP_2   "rtl8125bp-2.fw"
#define FIRMWARE_8125D_1    "rtl8125d-1.fw"
#define FIRMWARE_8125D_2    "rtl8125d-2.fw"
#define FIRMWARE_8125CP_1   "rtl8125cp-1.fw"

#define FIRMWARE_8126A_2    "rtl8126a-2.fw"
#define FIRMWARE_8126A_3    "rtl8126a-3.fw"

enum eetype {
        EEPROM_TYPE_NONE=0,
        EEPROM_TYPE_93C46,
        EEPROM_TYPE_93C56,
        EEPROM_TWSI
};

enum mcfg {
    CFG_METHOD_2=2,
    CFG_METHOD_3,
    CFG_METHOD_4,
    CFG_METHOD_5,
    CFG_METHOD_6,
    CFG_METHOD_7,
    CFG_METHOD_8,
    CFG_METHOD_9,
    CFG_METHOD_10,
    CFG_METHOD_11,
    CFG_METHOD_12,
    CFG_METHOD_13,
    CFG_METHOD_31,
    CFG_METHOD_32,
    CFG_METHOD_33,
    CFG_METHOD_DEFAULT,
    CFG_METHOD_MAX
};

#define Reserved2_data  7
#define RX_DMA_BURST_unlimited  7   /* Maximum PCI burst, '7' is unlimited */
#define RX_DMA_BURST_512    5
#define RX_DMA_BURST_256    4
#define TX_DMA_BURST_unlimited  7
#define TX_DMA_BURST_1024   6
#define TX_DMA_BURST_512    5
#define TX_DMA_BURST_256    4
#define TX_DMA_BURST_128    3
#define TX_DMA_BURST_64     2
#define TX_DMA_BURST_32     1
#define TX_DMA_BURST_16     0
#define Reserved1_data  0x3F
#define RxPacketMaxSize 0x3FE8  /* 16K - 1 - ETH_HLEN - VLAN - CRC... */
#define Jumbo_Frame_1k  ETH_DATA_LEN
#define Jumbo_Frame_2k  (2*1024 - ETH_HLEN - VLAN_HLEN - ETH_FCS_LEN)
#define Jumbo_Frame_3k  (3*1024 - ETH_HLEN - VLAN_HLEN - ETH_FCS_LEN)
#define Jumbo_Frame_4k  (4*1024 - ETH_HLEN - VLAN_HLEN - ETH_FCS_LEN)
#define Jumbo_Frame_5k  (5*1024 - ETH_HLEN - VLAN_HLEN - ETH_FCS_LEN)
#define Jumbo_Frame_6k  (6*1024 - ETH_HLEN - VLAN_HLEN - ETH_FCS_LEN)
#define Jumbo_Frame_7k  (7*1024 - ETH_HLEN - VLAN_HLEN - ETH_FCS_LEN)
#define Jumbo_Frame_8k  (8*1024 - ETH_HLEN - VLAN_HLEN - ETH_FCS_LEN)
#define Jumbo_Frame_9k  (9*1024 - ETH_HLEN - VLAN_HLEN - ETH_FCS_LEN)
#define InterFrameGap   0x03    /* 3 means InterFrameGap = the shortest one */
#define RxEarly_off_V1 (0x07 << 11)
#define RxEarly_off_V2 (1 << 11)
#define Rx_Single_fetch_V2 (1 << 14)
#define Rx_Close_Multiple (1 << 21)
#define Rx_Fetch_Number_8 (1 << 30)

#define R8125_REGS_SIZE     (256)
#define R8125_MAC_REGS_SIZE     (256)
#define R8125_PHY_REGS_SIZE     (16*2)
#define R8125_EPHY_REGS_SIZE      (31*2)
#define R8125_ERI_REGS_SIZE      (0x100)
#define R8125_REGS_DUMP_SIZE     (0x400)
#define R8125_PCI_REGS_SIZE      (0x100)
#define R8125_NAPI_WEIGHT   64

#define R8125_MAX_MSIX_VEC_8125A   4
#define R8125_MAX_MSIX_VEC_8125B   32
#define R8125_MAX_MSIX_VEC_8125D   32
#define R8125_MIN_MSIX_VEC_8125B   22
#define R8125_MIN_MSIX_VEC_8125BP  32
#define R8125_MIN_MSIX_VEC_8125CP  31
#define R8125_MIN_MSIX_VEC_8125D   20
#define R8125_MAX_MSIX_VEC   32
#define R8125_MAX_RX_QUEUES_VEC_V3 (16)

#define RTL8125_TX_TIMEOUT  (6 * HZ)
#define RTL8125_LINK_TIMEOUT    (1 * HZ)
#define RTL8125_ESD_TIMEOUT (2 * HZ)
#define RTL8125_DASH_TIMEOUT    (0)

/* Tx NO CLOSE */
#define MAX_TX_NO_CLOSE_DESC_PTR_V2 0x10000
#define MAX_TX_NO_CLOSE_DESC_PTR_MASK_V2 0xFFFF
#define MAX_TX_NO_CLOSE_DESC_PTR_V3 0x100000000
#define MAX_TX_NO_CLOSE_DESC_PTR_MASK_V3 0xFFFFFFFF
#define MAX_TX_NO_CLOSE_DESC_PTR_V4 0x80000000
#define MAX_TX_NO_CLOSE_DESC_PTR_MASK_V4 0x7FFFFFFF
#define TX_NO_CLOSE_SW_PTR_MASK_V2 0x1FFFF

#define D0_SPEED_UP_SPEED_DISABLE    0
#define D0_SPEED_UP_SPEED_1000       1
#define D0_SPEED_UP_SPEED_2500       2
#define D0_SPEED_UP_SPEED_5000       3

#define RTL8125_MAC_MCU_PAGE_SIZE 256 //256 words

#define R8125_LINK_STATE_OFF 0
#define R8125_LINK_STATE_ON 1
#define R8125_LINK_STATE_UNKNOWN 2

enum r8125_dash_req_flag {
    R8125_RCV_REQ_SYS_OK = 0,
    R8125_RCV_REQ_DASH_OK,
    R8125_SEND_REQ_HOST_OK,
    R8125_CMAC_RESET,
    R8125_CMAC_DISALE_RX_FLAG_MAX,
    R8125_DASH_REQ_FLAG_MAX
};

//Rx Desc Type
enum rx_desc_ring_type {
        RX_DESC_RING_TYPE_UNKNOWN=0,
        RX_DESC_RING_TYPE_1,
        RX_DESC_RING_TYPE_2,
        RX_DESC_RING_TYPE_3,
        RX_DESC_RING_TYPE_4,
        RX_DESC_RING_TYPE_MAX
};

#define ethtool_keee ethtool_eee
#define rtl8125_ethtool_adv_to_mmd_eee_adv_cap1_t ethtool_adv_to_mmd_eee_adv_t
static inline u32 rtl8125_ethtool_adv_to_mmd_eee_adv_cap2_t(u32 adv)
{
    u32 result = 0;

    if (adv & SUPPORTED_2500baseX_Full)
        result |= MDIO_EEE_2_5GT;

    return result;
}

enum RTL8125_registers {
    MAC0            = 0x00,     /* Ethernet hardware address. */
    MAC4            = 0x04,
    MAR0            = 0x08,     /* Multicast filter. */
    MAR1            = 0x0C,        /* Multicast filter. */
    CounterAddrLow      = 0x10,
    CounterAddrHigh     = 0x14,
    CustomLED       = 0x18,
    TxDescStartAddrLow  = 0x20,
    TxDescStartAddrHigh = 0x24,
    TxHDescStartAddrLow = 0x28,
    TxHDescStartAddrHigh    = 0x2c,
    FLASH           = 0x30,
    INT_CFG0_8125   = 0x34,
    ERSR            = 0x36,
    ChipCmd         = 0x37,
    TxPoll          = 0x38,
    IntrMask        = 0x3C,
    IntrStatus      = 0x3E,
    TxConfig        = 0x40,
    RxConfig        = 0x44,
    TCTR            = 0x48,
    Cfg9346         = 0x50,
    Config0         = 0x51,
    Config1         = 0x52,
    Config2         = 0x53,
    Config3         = 0x54,
    Config4         = 0x55,
    Config5         = 0x56,
    TDFNR           = 0x57,
    TimeInt0        = 0x58,
    TimeInt1        = 0x5C,
    PHYAR           = 0x60,
    CSIDR           = 0x64,
    CSIAR           = 0x68,
    PHYstatus       = 0x6C,
    MACDBG          = 0x6D,
    GPIO            = 0x6E,
    PMCH            = 0x6F,
    ERIDR           = 0x70,
    ERIAR           = 0x74,
    INT_CFG1_8125   = 0x7A,
    EPHY_RXER_NUM   = 0x7C,
    EPHYAR          = 0x80,
    LEDSEL_2_8125   = 0x84,
    LEDSEL_1_8125   = 0x86,
    TimeInt2        = 0x8C,
    LEDSEL_3_8125   = 0x96,
    OCPDR           = 0xB0,
    MACOCP          = 0xB0,
    OCPAR           = 0xB4,
    SecMAC0         = 0xB4,
    SecMAC4         = 0xB8,
    PHYOCP          = 0xB8,
    DBG_reg         = 0xD1,
    TwiCmdReg       = 0xD2,
    MCUCmd_reg      = 0xD3,
    RxMaxSize       = 0xDA,
    EFUSEAR         = 0xDC,
    CPlusCmd        = 0xE0,
    IntrMitigate    = 0xE2,
    RxDescAddrLow   = 0xE4,
    RxDescAddrHigh  = 0xE8,
    MTPS            = 0xEC,
    FuncEvent       = 0xF0,
    PPSW            = 0xF2,
    FuncEventMask   = 0xF4,
    TimeInt3        = 0xF4,
    FuncPresetState = 0xF8,
    CMAC_IBCR0      = 0xF8,
    CMAC_IBCR2      = 0xF9,
    CMAC_IBIMR0     = 0xFA,
    CMAC_IBISR0     = 0xFB,
    FuncForceEvent  = 0xFC,
    //8125
    IMR0_8125          = 0x38,
    ISR0_8125          = 0x3C,
    TPPOLL_8125        = 0x90,
    IMR1_8125          = 0x800,
    ISR1_8125          = 0x802,
    IMR2_8125          = 0x804,
    ISR2_8125          = 0x806,
    IMR3_8125          = 0x808,
    ISR3_8125          = 0x80A,
    BACKUP_ADDR0_8125  = 0x19E0,
    BACKUP_ADDR1_8125  = 0X19E4,
    TCTR0_8125         = 0x0048,
    TCTR1_8125         = 0x004C,
    TCTR2_8125         = 0x0088,
    TCTR3_8125         = 0x001C,
    TIMER_INT0_8125    = 0x0058,
    TIMER_INT1_8125    = 0x005C,
    TIMER_INT2_8125    = 0x008C,
    TIMER_INT3_8125    = 0x00F4,
    INT_MITI_V2_0_RX   = 0x0A00,
    INT_MITI_V2_0_TX   = 0x0A02,
    INT_MITI_V2_1_RX   = 0x0A08,
    INT_MITI_V2_1_TX   = 0x0A0A,
    IMR_V2_CLEAR_REG_8125 = 0x0D00,
    ISR_V2_8125           = 0x0D04,
    IMR_V2_SET_REG_8125   = 0x0D0C,
    TDU_STA_8125       = 0x0D08,
    RDU_STA_8125       = 0x0D0A,
    IMR_V4_L2_CLEAR_REG_8125 = 0x0D10,
    IMR_V4_L2_SET_REG_8125   = 0x0D18,
    ISR_V4_L2_8125     = 0x0D14,
    SW_TAIL_PTR0_8125BP = 0x0D30,
    SW_TAIL_PTR1_8125BP = 0x0D38,
    HW_CLO_PTR0_8125BP = 0x0D34,
    HW_CLO_PTR1_8125BP = 0x0D3C,
    DOUBLE_VLAN_CONFIG = 0x1000,
    TX_NEW_CTRL        = 0x203E,
    TNPDS_Q1_LOW_8125  = 0x2100,
    PLA_TXQ0_IDLE_CREDIT = 0x2500,
    PLA_TXQ1_IDLE_CREDIT = 0x2504,
    SW_TAIL_PTR0_8125  = 0x2800,
    HW_CLO_PTR0_8125   = 0x2802,
    SW_TAIL_PTR0_8126  = 0x2800,
    HW_CLO_PTR0_8126   = 0x2800,
    RDSAR_Q1_LOW_8125  = 0x4000,
    RSS_CTRL_8125      = 0x4500,
    Q_NUM_CTRL_8125    = 0x4800,
    RSS_KEY_8125       = 0x4600,
    RSS_INDIRECTION_TBL_8125_V2 = 0x4700,
    EEE_TXIDLE_TIMER_8125   = 0x6048,
    /* mac ptp */
    PTP_CTRL_8125      = 0x6800,
    PTP_STATUS_8125    = 0x6802,
    PTP_ISR_8125       = 0x6804,
    PTP_IMR_8125       = 0x6805,
    PTP_TIME_CORRECT_CMD_8125    = 0x6806,
    PTP_SOFT_CONFIG_Time_NS_8125 = 0x6808,
    PTP_SOFT_CONFIG_Time_S_8125  = 0x680C,
    PTP_SOFT_CONFIG_Time_Sign    = 0x6812,
    PTP_LOCAL_Time_SUB_NS_8125   = 0x6814,
    PTP_LOCAL_Time_NS_8125       = 0x6818,
    PTP_LOCAL_Time_S_8125        = 0x681C,
    PTP_Time_SHIFTER_S_8125      = 0x6856,
    PPS_RISE_TIME_NS_8125        = 0x68A0,
    PPS_RISE_TIME_S_8125         = 0x68A4,
    PTP_EGRESS_TIME_BASE_NS_8125 = 0XCF20,
    PTP_EGRESS_TIME_BASE_S_8125  = 0XCF24,
    /* phy ptp */
    PTP_CTL                 = 0xE400,
    PTP_INER                = 0xE402,
    PTP_INSR                = 0xE404,
    PTP_SYNCE_CTL           = 0xE406,
    PTP_GEN_CFG             = 0xE408,
    PTP_CLK_CFG_8126        = 0xE410,
    PTP_CFG_NS_LO_8126      = 0xE412,
    PTP_CFG_NS_HI_8126      = 0xE414,
    PTP_CFG_S_LO_8126       = 0xE416,
    PTP_CFG_S_MI_8126       = 0xE418,
    PTP_CFG_S_HI_8126       = 0xE41A,
    PTP_TAI_CFG             = 0xE420,
    PTP_TAI_TS_S_LO         = 0xE42A,
    PTP_TAI_TS_S_HI         = 0xE42C,
    PTP_TRX_TS_STA          = 0xE430,
    PTP_TRX_TS_NS_LO        = 0xE446,
    PTP_TRX_TS_NS_HI        = 0xE448,
    PTP_TRX_TS_S_LO         = 0xE44A,
    PTP_TRX_TS_S_MI         = 0xE44C,
    PTP_TRX_TS_S_HI         = 0xE44E,


    //TCAM
    TCAM_NOTVALID_ADDR           = 0xA000,
    TCAM_VALID_ADDR              = 0xA800,
    TCAM_MAC_ADDR                = 448,
    TCAM_VLAN_TAG                = 496,
    //TCAM V2
    TCAM_NOTVALID_ADDR_V2           = 0xA000,
    TCAM_VALID_ADDR_V2              = 0xB000,
    TCAM_MAC_ADDR_V2                = 0x00,
    TCAM_VLAN_TAG_V2                = 0x03,
    //ipc2
    IB2SOC_SET     = 0x0010,
    IB2SOC_DATA    = 0x0014,
    IB2SOC_CMD     = 0x0018,
    IB2SOC_IMR     = 0x001C,

    RISC_IMR_8125BP     = 0x0D20,
    RISC_ISR_8125BP     = 0x0D22,
};

enum RTL8125_register_content {
    /* InterruptStatusBits */
    SYSErr      = 0x8000,
    PCSTimeout  = 0x4000,
    SWInt       = 0x0100,
    TxDescUnavail   = 0x0080,
    RxFIFOOver  = 0x0040,
    LinkChg     = 0x0020,
    RxDescUnavail   = 0x0010,
    TxErr       = 0x0008,
    TxOK        = 0x0004,
    RxErr       = 0x0002,
    RxOK        = 0x0001,
    RxDU1       = 0x0002,
    RxOK1       = 0x0001,

    /* RxStatusDesc */
    RxRWT = (1 << 22),
    RxRES = (1 << 21),
    RxRUNT = (1 << 20),
    RxCRC = (1 << 19),

    RxRWT_V3 = (1 << 18),
    RxRES_V3 = (1 << 20),
    RxRUNT_V3 = (1 << 19),
    RxCRC_V3 = (1 << 17),

    RxRES_V4 = (1 << 22),
    RxRUNT_V4 = (1 << 21),
    RxCRC_V4 = (1 << 20),

    /* ChipCmdBits */
    StopReq  = 0x80,
    CmdReset = 0x10,
    CmdRxEnb = 0x08,
    CmdTxEnb = 0x04,
    RxBufEmpty = 0x01,

    /* Cfg9346Bits */
    Cfg9346_EEM_MASK = 0xC0,
    Cfg9346_Lock = 0x00,
    Cfg9346_Unlock = 0xC0,
    Cfg9346_EEDO = (1 << 0),
    Cfg9346_EEDI = (1 << 1),
    Cfg9346_EESK = (1 << 2),
    Cfg9346_EECS = (1 << 3),
    Cfg9346_EEM0 = (1 << 6),
    Cfg9346_EEM1 = (1 << 7),

    /* rx_mode_bits */
    AcceptErr = 0x20,
    AcceptRunt = 0x10,
    AcceptBroadcast = 0x08,
    AcceptMulticast = 0x04,
    AcceptMyPhys = 0x02,
    AcceptAllPhys = 0x01,
    AcceppVlanPhys = 0x8000,

    /* Transmit Priority Polling*/
    HPQ = 0x80,
    NPQ = 0x40,
    FSWInt = 0x01,

    /* RxConfigBits */
    Reserved2_shift = 13,
    RxCfgDMAShift = 8,
    EnableRxDescV3 = (1 << 24),
    EnableRxDescV4_1 = (1 << 24),
    EnableOuterVlan = (1 << 23),
    EnableInnerVlan = (1 << 22),
    RxCfg_128_int_en = (1 << 15),
    RxCfg_fet_multi_en = (1 << 14),
    RxCfg_half_refetch = (1 << 13),
    RxCfg_pause_slot_en = (1 << 11),
    RxCfg_9356SEL = (1 << 6),
    EnableRxDescV4_0 = (1 << 1), //not in rcr

    /* TxConfigBits */
    TxInterFrameGapShift = 24,
    TxDMAShift = 8, /* DMA burst value (0-7) is shift this many bits */
    TxMACLoopBack = (1 << 17),  /* MAC loopback */

    /* Config1 register */
    LEDS1       = (1 << 7),
    LEDS0       = (1 << 6),
    Speed_down  = (1 << 4),
    MEMMAP      = (1 << 3),
    IOMAP       = (1 << 2),
    VPD         = (1 << 1),
    PMEnable    = (1 << 0), /* Power Management Enable */

    /* Config2 register */
    PMSTS_En    = (1 << 5),

    /* Config3 register */
    Isolate_en  = (1 << 12), /* Isolate enable */
    MagicPacket = (1 << 5), /* Wake up when receives a Magic Packet */
    LinkUp      = (1 << 4), /* This bit is reserved in RTL8125B.*/
    /* Wake up when the cable connection is re-established */
    ECRCEN      = (1 << 3), /* This bit is reserved in RTL8125B*/
    Jumbo_En0   = (1 << 2), /* This bit is reserved in RTL8125B*/
    RDY_TO_L23  = (1 << 1), /* This bit is reserved in RTL8125B*/
    Beacon_en   = (1 << 0), /* This bit is reserved in RTL8125B*/

    /* Config4 register */
    Jumbo_En1   = (1 << 1), /* This bit is reserved in RTL8125B*/

    /* Config5 register */
    BWF     = (1 << 6), /* Accept Broadcast wakeup frame */
    MWF     = (1 << 5), /* Accept Multicast wakeup frame */
    UWF     = (1 << 4), /* Accept Unicast wakeup frame */
    LanWake     = (1 << 1), /* LanWake enable/disable */
    PMEStatus   = (1 << 0), /* PME status can be reset by PCI RST# */

    /* CPlusCmd */
    EnableBist  = (1 << 15),
    Macdbgo_oe  = (1 << 14),
    Normal_mode = (1 << 13),
    Force_halfdup   = (1 << 12),
    Force_rxflow_en = (1 << 11),
    Force_txflow_en = (1 << 10),
    Cxpl_dbg_sel    = (1 << 9),//This bit is reserved in RTL8125B
    ASF     = (1 << 8),//This bit is reserved in RTL8125C
    PktCntrDisable  = (1 << 7),
    RxVlan      = (1 << 6),
    RxChkSum    = (1 << 5),
    Macdbgo_sel = 0x001C,
    INTT_0      = 0x0000,
    INTT_1      = 0x0001,
    INTT_2      = 0x0002,
    INTT_3      = 0x0003,

    /* rtl8125_PHYstatus */
    PowerSaveStatus = 0x80,
    _1000bpsL = 0x80000,
    _5000bpsF = 0x1000,
    _5000bpsL = 0x800,
    _2500bpsF = 0x400,
    _2500bpsL = 0x200,
    TxFlowCtrl = 0x40,
    RxFlowCtrl = 0x20,
    _1000bpsF = 0x10,
    _100bps = 0x08,
    _10bps = 0x04,
    LinkStatus = 0x02,
    FullDup = 0x01,

    /* DBG_reg */
    Fix_Nak_1 = (1 << 4),
    Fix_Nak_2 = (1 << 3),
    DBGPIN_E2 = (1 << 0),

    /* ResetCounterCommand */
    CounterReset = 0x1,
    /* DumpCounterCommand */
    CounterDump = 0x8,

    /* PHY access */
    PHYAR_Flag = 0x80000000,
    PHYAR_Write = 0x80000000,
    PHYAR_Read = 0x00000000,
    PHYAR_Reg_Mask = 0x1f,
    PHYAR_Reg_shift = 16,
    PHYAR_Data_Mask = 0xffff,

    /* EPHY access */
    EPHYAR_Flag = 0x80000000,
    EPHYAR_Write = 0x80000000,
    EPHYAR_Read = 0x00000000,
    EPHYAR_Reg_Mask = 0x3f,
    EPHYAR_Reg_Mask_v2 = 0x7f,
    EPHYAR_Reg_shift = 16,
    EPHYAR_Data_Mask = 0xffff,

    /* CSI access */
    CSIAR_Flag = 0x80000000,
    CSIAR_Write = 0x80000000,
    CSIAR_Read = 0x00000000,
    CSIAR_ByteEn = 0x0f,
    CSIAR_ByteEn_shift = 12,
    CSIAR_Addr_Mask = 0x0fff,

    /* ERI access */
    ERIAR_Flag = 0x80000000,
    ERIAR_Write = 0x80000000,
    ERIAR_Read = 0x00000000,
    ERIAR_Addr_Align = 4, /* ERI access register address must be 4 byte alignment */
    ERIAR_ExGMAC = 0,
    ERIAR_MSIX = 1,
    ERIAR_ASF = 2,
    ERIAR_OOB = 2,
    ERIAR_Type_shift = 16,
    ERIAR_ByteEn = 0x0f,
    ERIAR_ByteEn_shift = 12,

    /* OCP GPHY access */
    OCPDR_Write = 0x80000000,
    OCPDR_Read = 0x00000000,
    OCPDR_Reg_Mask = 0xFF,
    OCPDR_Data_Mask = 0xFFFF,
    OCPDR_GPHY_Reg_shift = 16,
    OCPAR_Flag = 0x80000000,
    OCPAR_GPHY_Write = 0x8000F060,
    OCPAR_GPHY_Read = 0x0000F060,
    OCPR_Write = 0x80000000,
    OCPR_Read = 0x00000000,
    OCPR_Addr_Reg_shift = 16,
    OCPR_Flag = 0x80000000,
    OCP_STD_PHY_BASE_PAGE = 0x0A40,

    /* MCU Command */
    Now_is_oob = (1 << 7),
    Txfifo_empty = (1 << 5),
    Rxfifo_empty = (1 << 4),

    /* E-FUSE access */
    EFUSE_WRITE = 0x80000000,
    EFUSE_WRITE_OK  = 0x00000000,
    EFUSE_READ  = 0x00000000,
    EFUSE_READ_OK   = 0x80000000,
    EFUSE_WRITE_V3 = 0x40000000,
    EFUSE_WRITE_OK_V3  = 0x00000000,
    EFUSE_READ_V3  = 0x80000000,
    EFUSE_READ_OK_V3   = 0x00000000,
    EFUSE_Reg_Mask  = 0x03FF,
    EFUSE_Reg_Shift = 8,
    EFUSE_Check_Cnt = 300,
    EFUSE_READ_FAIL = 0xFF,
    EFUSE_Data_Mask = 0x000000FF,

    /* GPIO */
    GPIO_en = (1 << 0),

    /* PTP */
    PTP_ISR_TOK = (1 << 1),
    PTP_ISR_TER = (1 << 2),
    PTP_EXEC_CMD = (1 << 7),
    PTP_ADJUST_TIME_NS_NEGATIVE = (1 << 30),
    PTP_ADJUST_TIME_S_NEGATIVE = (1ULL << 48),
    PTP_SOFT_CONFIG_TIME_NS_NEGATIVE = (1 << 30),
    PTP_SOFT_CONFIG_TIME_S_NEGATIVE = (1ULL << 48),

    /* New Interrupt Bits */
    INT_CFG0_ENABLE_8125 = (1 << 0),
    INT_CFG0_TIMEOUT0_BYPASS_8125 = (1 << 1),
    INT_CFG0_MITIGATION_BYPASS_8125 = (1 << 2),
    INT_CFG0_RDU_BYPASS_8126 = (1 << 4),
    INT_CFG0_MSIX_ENTRY_NUM_MODE = (1 << 5),
    INT_CFG0_AUTO_CLEAR_IMR = (1 << 5),
    INT_CFG0_AVOID_MISS_INTR = (1 << 6),
    ISRIMR_V2_ROK_Q0     = (1 << 0),
    ISRIMR_TOK_Q0        = (1 << 16),
    ISRIMR_TOK_Q1        = (1 << 18),
    ISRIMR_V2_LINKCHG    = (1 << 21),

    ISRIMR_V4_ROK_Q0     = (1 << 0),
    ISRIMR_V4_LINKCHG    = (1 << 29),
    ISRIMR_V4_LAYER2_INTR_STS = (1 << 31),
    ISRIMR_V4_L2_IPC2    = (1 << 17),

    ISRIMR_V5_ROK_Q0     = (1 << 0),
    ISRIMR_V5_TOK_Q0     = (1 << 16),
    ISRIMR_V5_TOK_Q1     = (1 << 17),
    ISRIMR_V5_LINKCHG    = (1 << 18),

    ISRIMR_V7_ROK_Q0     = (1 << 0),
    ISRIMR_V7_TOK_Q0     = (1 << 27),
    ISRIMR_V7_TOK_Q1     = (1 << 28),
    ISRIMR_V7_LINKCHG    = (1 << 29),

    /* IPC2 */
    RISC_IPC2_INTR    = (1 << 1),

    /* AVB */
    AVB_MODE_ENABLE      = (1 << 0),
    DVLAN_MODE_ENABLE    = (1 << 1),

    /* Magic Number */
    RTL8125_MAGIC_NUMBER = 0x0badbadbadbadbadull,
};

enum _DescStatusBit {
        DescOwn     = (1 << 31), /* Descriptor is owned by NIC */
        RingEnd     = (1 << 30), /* End of descriptor ring */
        FirstFrag   = (1 << 29), /* First segment of a packet */
        LastFrag    = (1 << 28), /* Final segment of a packet */

        DescOwn_V3     = (DescOwn), /* Descriptor is owned by NIC */
        RingEnd_V3     = (RingEnd), /* End of descriptor ring */
        FirstFrag_V3   = (1 << 25), /* First segment of a packet */
        LastFrag_V3    = (1 << 24), /* Final segment of a packet */

        DescOwn_V4     = (DescOwn), /* Descriptor is owned by NIC */
        RingEnd_V4     = (RingEnd), /* End of descriptor ring */
        FirstFrag_V4   = (FirstFrag), /* First segment of a packet */
        LastFrag_V4    = (LastFrag), /* Final segment of a packet */

        /* Tx private */
        /*------ offset 0 of tx descriptor ------*/
        LargeSend   = (1 << 27), /* TCP Large Send Offload (TSO) */
        GiantSendv4 = (1 << 26), /* TCP Giant Send Offload V4 (GSOv4) */
        GiantSendv6 = (1 << 25), /* TCP Giant Send Offload V6 (GSOv6) */
        LargeSend_DP = (1 << 16), /* TCP Large Send Offload (TSO) */
        MSSShift    = 16,        /* MSS value position */
        MSSMask     = 0x7FFU,    /* MSS value 11 bits */
        TxIPCS      = (1 << 18), /* Calculate IP checksum */
        TxUDPCS     = (1 << 17), /* Calculate UDP/IP checksum */
        TxTCPCS     = (1 << 16), /* Calculate TCP/IP checksum */
        TxVlanTag   = (1 << 17), /* Add VLAN tag */

        /*@@@@@@ offset 4 of tx descriptor => bits for RTL8125 only     begin @@@@@@*/
        TxUDPCS_C   = (1 << 31), /* Calculate UDP/IP checksum */
        TxTCPCS_C   = (1 << 30), /* Calculate TCP/IP checksum */
        TxIPCS_C    = (1 << 29), /* Calculate IP checksum */
        TxIPV6F_C   = (1 << 28), /* Indicate it is an IPv6 packet */
        /*@@@@@@ offset 4 of tx descriptor => bits for RTL8125 only     end @@@@@@*/


        /* Rx private */
        /*------ offset 0 of rx descriptor ------*/
        PID1        = (1 << 18), /* Protocol ID bit 1/2 */
        PID0        = (1 << 17), /* Protocol ID bit 2/2 */

#define RxProtoUDP  (PID1)
#define RxProtoTCP  (PID0)
#define RxProtoIP   (PID1 | PID0)
#define RxProtoMask RxProtoIP

        RxIPF       = (1 << 16), /* IP checksum failed */
        RxUDPF      = (1 << 15), /* UDP/IP checksum failed */
        RxTCPF      = (1 << 14), /* TCP/IP checksum failed */
        RxVlanTag   = (1 << 16), /* VLAN tag available */

        /*@@@@@@ offset 0 of rx descriptor => bits for RTL8125 only     begin @@@@@@*/
        RxUDPT      = (1 << 18),
        RxTCPT      = (1 << 17),
        /*@@@@@@ offset 0 of rx descriptor => bits for RTL8125 only     end @@@@@@*/

        /*@@@@@@ offset 4 of rx descriptor => bits for RTL8125 only     begin @@@@@@*/
        RxV6F       = (1 << 31),
        RxV4F       = (1 << 30),
        /*@@@@@@ offset 4 of rx descriptor => bits for RTL8125 only     end @@@@@@*/


        PID1_v3        = (1 << 29), /* Protocol ID bit 1/2 */
        PID0_v3        = (1 << 28), /* Protocol ID bit 2/2 */

#define RxProtoUDP_v3  (PID1_v3)
#define RxProtoTCP_v3  (PID0_v3)
#define RxProtoIP_v3   (PID1_v3 | PID0_v3)
#define RxProtoMask_v3 RxProtoIP_v3

        RxIPF_v3       = (1 << 26), /* IP checksum failed */
        RxUDPF_v3      = (1 << 25), /* UDP/IP checksum failed */
        RxTCPF_v3      = (1 << 24), /* TCP/IP checksum failed */
        RxSCTPF_v3     = (1 << 23), /* SCTP checksum failed */
        RxVlanTag_v3   = (RxVlanTag), /* VLAN tag available */

        /*@@@@@@ offset 0 of rx descriptor => bits for RTL8125 only     begin @@@@@@*/
        RxUDPT_v3      = (1 << 29),
        RxTCPT_v3      = (1 << 28),
        RxSCTP_v3      = (1 << 27),
        /*@@@@@@ offset 0 of rx descriptor => bits for RTL8125 only     end @@@@@@*/

        /*@@@@@@ offset 4 of rx descriptor => bits for RTL8125 only     begin @@@@@@*/
        RxV6F_v3       = (RxV6F),
        RxV4F_v3       = (RxV4F),
        /*@@@@@@ offset 4 of rx descriptor => bits for RTL8125 only     end @@@@@@*/

        RxIPF_v4       = (1 << 17), /* IP checksum failed */
        RxUDPF_v4      = (1 << 16), /* UDP/IP checksum failed */
        RxTCPF_v4      = (1 << 15), /* TCP/IP checksum failed */
        RxSCTPF_v4     = (1 << 19), /* SCTP checksum failed */
        RxVlanTag_v4   = (RxVlanTag), /* VLAN tag available */

        /*@@@@@@ offset 0 of rx descriptor => bits for RTL8125 only     begin @@@@@@*/
        RxUDPT_v4      = (1 << 19),
        RxTCPT_v4      = (1 << 18),
        RxSCTP_v4      = (1 << 19),
        /*@@@@@@ offset 0 of rx descriptor => bits for RTL8125 only     end @@@@@@*/

        /*@@@@@@ offset 4 of rx descriptor => bits for RTL8125 only     begin @@@@@@*/
        RxV6F_v4       = (RxV6F),
        RxV4F_v4       = (RxV4F),
        /*@@@@@@ offset 4 of rx descriptor => bits for RTL8125 only     end @@@@@@*/
};

enum features {
//  RTL_FEATURE_WOL = (1 << 0),
        RTL_FEATURE_MSI = (1 << 1),
        RTL_FEATURE_MSIX = (1 << 2),
};

enum wol_capability {
        WOL_DISABLED = 0,
        WOL_ENABLED = 1
};

enum bits {
        BIT_0 = (1 << 0),
        BIT_1 = (1 << 1),
        BIT_2 = (1 << 2),
        BIT_3 = (1 << 3),
        BIT_4 = (1 << 4),
        BIT_5 = (1 << 5),
        BIT_6 = (1 << 6),
        BIT_7 = (1 << 7),
        BIT_8 = (1 << 8),
        BIT_9 = (1 << 9),
        BIT_10 = (1 << 10),
        BIT_11 = (1 << 11),
        BIT_12 = (1 << 12),
        BIT_13 = (1 << 13),
        BIT_14 = (1 << 14),
        BIT_15 = (1 << 15),
        BIT_16 = (1 << 16),
        BIT_17 = (1 << 17),
        BIT_18 = (1 << 18),
        BIT_19 = (1 << 19),
        BIT_20 = (1 << 20),
        BIT_21 = (1 << 21),
        BIT_22 = (1 << 22),
        BIT_23 = (1 << 23),
        BIT_24 = (1 << 24),
        BIT_25 = (1 << 25),
        BIT_26 = (1 << 26),
        BIT_27 = (1 << 27),
        BIT_28 = (1 << 28),
        BIT_29 = (1 << 29),
        BIT_30 = (1 << 30),
        BIT_31 = (1 << 31)
};

#define OCP_STD_PHY_BASE    0xa400

//Channel Wait Count
#define R8125_CHANNEL_WAIT_COUNT (20000)
#define R8125_CHANNEL_WAIT_TIME (1)  // 1us
#define R8125_CHANNEL_EXIT_DELAY_TIME (20)  //20us

/* Phy Fuse Dout */
#define R8125_PHY_FUSE_DOUT_NUM (32)
#define R8125_MAX_PHY_FUSE_DOUT_NUM R8125_PHY_FUSE_DOUT_NUM

#define RTL8125_CP_NUM 4
#define RTL8125_MAX_SUPPORT_CP_LEN 110

enum rtl8125_cp_status {
        rtl8125_cp_normal = 0,
        rtl8125_cp_short,
        rtl8125_cp_open,
        rtl8125_cp_mismatch,
        rtl8125_cp_unknown
};

enum efuse {
        EFUSE_NOT_SUPPORT = 0,
        EFUSE_SUPPORT_V1,
        EFUSE_SUPPORT_V2,
        EFUSE_SUPPORT_V3,
        EFUSE_SUPPORT_V4,
};
#define RsvdMask    0x3fffc000
#define RsvdMaskV3  0x3fff8000
#define RsvdMaskV4  RsvdMaskV3

/* Flow Control Settings */
enum rtl8125_fc_mode {
    rtl8125_fc_none = 0,
    rtl8125_fc_rx_pause,
    rtl8125_fc_tx_pause,
    rtl8125_fc_full,
    rtl8125_fc_default
};

struct rtl8125_private {
    void __iomem *mmio_addr;    /* memory map physical address */
    struct pci_dev *pci_dev;    /* Index of PCI device */
    
    //unsigned long state;
    //u8 flags;

    u32 chipset;
    u8 mcfg;
    u32 rtl8125_rx_config;
    u16 rms;
    u16 cp_cmd;

    int phy_auto_nego_reg;
    int phy_1000_ctrl_reg;
    int phy_2500_ctrl_reg;

    u8 aspm;
    u8  wol_enabled;
    u32 wol_opts;
    u8  efuse_ver;
    u8  eeprom_type;
    u8  autoneg;
    u8  duplex;
    u32 speed;
    u32 fcpause;
    u64 advertising;
    u32 HwSuppMaxPhyLinkSpeed;
    u16 eeprom_len;
    u16 cur_page;
    u32 bios_setting;
    
    void (*get_settings)(struct rtl8125_private *, struct ethtool_link_ksettings *);
    void (*phy_reset_enable)(struct rtl8125_private *);
    unsigned int (*phy_reset_pending)(struct rtl8125_private *);
    unsigned int (*link_ok)(struct rtl8125_private *);

    u8 org_pci_offset_99;
    u8 org_pci_offset_180;
    u8 issue_offset_99_event;

    u8 org_pci_offset_80;
    u8 org_pci_offset_81;
    //u8 use_timer_interrupt;

    u32 keep_intr_cnt;

    u8  HwIcVerUnknown;
    u8  NotWrRamCodeToMicroP;
    u8  NotWrMcuPatchCode;
    u8  HwHasWrRamCodeToMicroP;

    u16 sw_ram_code_ver;
    u16 hw_ram_code_ver;

    u8 rtk_enable_diag;

    //u8 ShortPacketSwChecksum;

    //u8 UseSwPaddingShortPkt;

    u8 RequireAdcBiasPatch;
    u16 AdcBiasPatchIoffset;

    u8 RequireAdjustUpsTxLinkPulseTiming;
    u16 SwrCnt1msIni;

    //u8 HwSuppNowIsOobVer;

    u8 RequiredSecLanDonglePatch;

    u8 RequiredPfmPatch;

    u8 RequirePhyMdiSwapPatch;

    u8 RequireLSOPatch;

    u32 HwFiberModeVer;
    u32 HwFiberStat;
    u8 HwSwitchMdiToFiber;

    u16 BackupLedSel[4];

    u8 HwSuppMagicPktVer;

    //u8 HwSuppLinkChgWakeUpVer;

    u8 HwSuppCheckPhyDisableModeVer;

    u8 random_mac;

    u16 phy_reg_aner;
    u16 phy_reg_anlpar;
    u16 phy_reg_gbsr;
    u16 phy_reg_status_2500;

    u32 HwPcieSNOffset;

    u8 HwSuppEsdVer;
    u8 TestPhyOcpReg;
    u16 BackupPhyFuseDout[R8125_MAX_PHY_FUSE_DOUT_NUM];

    u32 MaxTxDescPtrMask;
    u16 HwCloPtrReg;
    u16 SwTailPtrReg;
    u8 HwSuppTxNoCloseVer;
    //u8 EnableTxNoClose;

    //u8 HwSuppIsrVer;
    //u8 HwCurrIsrVer;

    u8 HwSuppIntMitiVer;

    //u8 HwSuppExtendTallyCounterVer;

    u8 check_keep_link_speed;
    u8 resume_not_chg_speed;

    u8 HwSuppD0SpeedUpVer;
    u8 D0SpeedUpSpeed;
    
    const char *fw_name;
    struct rtl8125_fw *rtl_fw;
    u32 ocp_base;

    //Dash+++++++++++++++++
    u8 HwSuppDashVer;
    u8 DASH;
    u8 HwPkgDet;
    u8 HwSuppOcpChannelVer;
    u32 DashFirmwareVersion;
    u32 SizeOfSendToFwBuffer;
    u32 SizeOfRecvFromFwBuffer;
    u8 AllowAccessDashOcp;
    DECLARE_BITMAP(dash_req_flags, R8125_DASH_REQ_FLAG_MAX);

    struct ethtool_keee eee;
    
    u8 HwSuppRxDescType;

    u8 HwSuppMacMcuVer;
    u16 MacMcuPageSize;
    u64 hw_mcu_patch_code_ver;
    u64 bin_mcu_patch_code_ver;

    u8 HwSuppTcamVer;

    u16 TcamNotValidReg;
    u16 TcamValidReg;
    u16 TcamMaAddrcOffset;
    u16 TcamVlanTagOffset;
};

#define GTTCPHO_SHIFT                   18
#define GTTCPHO_MAX                     0x70U
#define GTPKTSIZE_MAX                   0x3ffffU
#define TCPHO_SHIFT                     18
#define TCPHO_MAX                       0x3ffU
#define LSOPKTSIZE_MAX                  0xffffU
#define MSS_MAX                         0x07ffu /* MSS value */

#define RTL8168FP_OOBMAC_BASE 0xBAF70000

#define OOB_CMD_RESET       0x00
#define OOB_CMD_DRIVER_START    0x05
#define OOB_CMD_DRIVER_STOP 0x06
#define OOB_CMD_SET_IPMAC   0x41

#define WAKEUP_MAGIC_PACKET_NOT_SUPPORT (0)
#define WAKEUP_MAGIC_PACKET_V1 (1)
#define WAKEUP_MAGIC_PACKET_V2 (2)
#define WAKEUP_MAGIC_PACKET_V3 (3)

#define WAKE_ANY (WAKE_PHY | WAKE_MAGIC | WAKE_UCAST | WAKE_BCAST | WAKE_MCAST)

//Ram Code Version
#define NIC_RAMCODE_VERSION_CFG_METHOD_2 (0x0b11)
#define NIC_RAMCODE_VERSION_CFG_METHOD_3 (0x0b33)
#define NIC_RAMCODE_VERSION_CFG_METHOD_4 (0x0b17)
#define NIC_RAMCODE_VERSION_CFG_METHOD_5 (0x0b55)
#define NIC_RAMCODE_VERSION_CFG_METHOD_8 (0x0013)
#define NIC_RAMCODE_VERSION_CFG_METHOD_9 (0x0001)
#define NIC_RAMCODE_VERSION_CFG_METHOD_10 (0x0027)
#define NIC_RAMCODE_VERSION_CFG_METHOD_11 (0x0031)
#define NIC_RAMCODE_VERSION_CFG_METHOD_12 (0x0010)
#define NIC_RAMCODE_VERSION_CFG_METHOD_31 (0x0023)
#define NIC_RAMCODE_VERSION_CFG_METHOD_32 (0x0033)
#define NIC_RAMCODE_VERSION_CFG_METHOD_33 (0x0060)

//hwoptimize
#define HW_PATCH_SOC_LAN (BIT_0)
#define HW_PATCH_SAMSUNG_LAN_DONGLE (BIT_2)

static const u16 other_q_intr_mask = (RxOK1 | RxDU1);

#define HW_PHY_STATUS_INI       1
#define HW_PHY_STATUS_EXT_INI   2
#define HW_PHY_STATUS_LAN_ON    3

#define RTL_W8(tp, reg, val8)   _OSWriteInt8((tp)->mmio_addr, (reg), (val8))
#define RTL_W16(tp, reg, val16) OSWriteLittleInt16((tp)->mmio_addr, (reg), (val16))
#define RTL_W32(tp, reg, val32) OSWriteLittleInt32((tp)->mmio_addr, (reg), (val32))
#define RTL_R8(tp, reg)         _OSReadInt8((tp)->mmio_addr, (reg))
#define RTL_R16(tp, reg)        OSReadLittleInt16((tp)->mmio_addr, (reg))
#define RTL_R32(tp, reg)        OSReadLittleInt32((tp)->mmio_addr, (reg))

#define RTK_ADVERTISED_5000baseX_Full  BIT_ULL(48)
#define RTK_SUPPORTED_5000baseX_Full BIT_ULL(48)

#define RTK_ADVERTISE_2500FULL  0x80
#define RTK_ADVERTISE_5000FULL  0x100
#define RTK_ADVERTISE_10000FULL  0x1000
#define RTK_LPA_ADVERTISE_2500FULL  0x20
#define RTK_LPA_ADVERTISE_5000FULL  0x40
#define RTK_LPA_ADVERTISE_10000FULL  0x800

#define RTK_EEE_ADVERTISE_2500FULL  BIT(0)
#define RTK_EEE_ADVERTISE_5000FULL  BIT(1)
#define RTK_LPA_EEE_ADVERTISE_2500FULL  BIT(0)
#define RTK_LPA_EEE_ADVERTISE_5000FULL  BIT(1)

#define HW_SUPPORT_D0_SPEED_UP(_M)        ((_M)->HwSuppD0SpeedUpVer > 0)

#define HW_SUPPORT_CHECK_PHY_DISABLE_MODE(_M)        ((_M)->HwSuppCheckPhyDisableModeVer > 0)
#define HW_HAS_WRITE_PHY_MCU_RAM_CODE(_M)        (((_M)->HwHasWrRamCodeToMicroP == TRUE) ? 1 : 0)
#define HW_SUPPORT_D0_SPEED_UP(_M)        ((_M)->HwSuppD0SpeedUpVer > 0)
#define HW_SUPPORT_MAC_MCU(_M)        ((_M)->HwSuppMacMcuVer > 0)
#define HW_SUPPORT_TCAM(_M)        ((_M)->HwSuppTcamVer > 0)

#define HW_SUPP_PHY_LINK_SPEED_GIGA(_M)        ((_M)->HwSuppMaxPhyLinkSpeed >= 1000)
#define HW_SUPP_PHY_LINK_SPEED_2500M(_M)        ((_M)->HwSuppMaxPhyLinkSpeed >= 2500)
#define HW_SUPP_PHY_LINK_SPEED_5000M(_M)        ((_M)->HwSuppMaxPhyLinkSpeed >= 5000)

#ifdef __cplusplus
extern "C" {
    bool rtl8125_aspm_is_safe(struct rtl8125_private *tp);
    bool rtl8125_is_speed_mode_valid(u32 speed);
    void rtl8125_set_link_option(struct rtl8125_private *tp, u8 autoneg, u32 speed, u8 duplex,  enum rtl8125_fc_mode fc);

    void rtl8125_gset_xmii(struct rtl8125_private *tp, struct ethtool_link_ksettings *cmd);
    void rtl8125_xmii_reset_enable(struct rtl8125_private *tp);
    unsigned int rtl8125_xmii_reset_pending(struct rtl8125_private *tp);
    unsigned int rtl8125_xmii_link_ok(struct rtl8125_private *tp);

    void rtl8125_init_software_variable(struct rtl8125_private *tp);
    void rtl8125_exit_oob(struct rtl8125_private *tp);

    void rtl8125_nic_reset(struct rtl8125_private *tp);
    void rtl8125_hw_reset(struct rtl8125_private *tp);
    void rtl8125_powerup_pll(struct rtl8125_private *tp);
    void rtl8125_powerdown_pll(struct rtl8125_private *tp);

    void rtl8125_rar_set(struct rtl8125_private *tp, const u8 *addr);
    void rtl8125_disable_rx_packet_filter(struct rtl8125_private *tp);
    void rtl8125_enable_cfg9346_write(struct rtl8125_private *tp);
    void rtl8125_disable_cfg9346_write(struct rtl8125_private *tp);
    void rtl8125_enable_force_clkreq(struct rtl8125_private *tp, bool enable);
    void rtl8125_enable_aspm_clkreq_lock(struct rtl8125_private *tp, bool enable);
    void rtl8125_set_eee_lpi_timer(struct rtl8125_private *tp);
    u32 mdio_direct_read_phy_ocp(struct rtl8125_private *tp, u16 RegAddr);
    void mdio_direct_write_phy_ocp(struct rtl8125_private *tp, u16 RegAddr, u16 value);
    u32 rtl8125_mdio_read(struct rtl8125_private *tp, u16 RegAddr);
    void rtl8125_mdio_write(struct rtl8125_private *tp, u16 RegAddr, u16 value);
    void rtl8125_mac_ocp_write(struct rtl8125_private *tp, u16 reg_addr, u16 value);
    u16 rtl8125_mac_ocp_read(struct rtl8125_private *tp, u16 reg_addr);
    void mac_mcu_write(struct rtl8125_private *tp, u16 reg, u16 value);
    u32 mac_mcu_read(struct rtl8125_private *tp, u16 reg);
    void rtl8125_enable_tcam(struct rtl8125_private *tp);
    void rtl8125_set_l1_l0s_entry_latency(struct rtl8125_private *tp);
    void rtl8126_disable_l1_timeout(struct rtl8125_private *tp);
    void rtl8125_enable_mcu(struct rtl8125_private *tp, bool enable);
    void rtl8125_oob_mutex_lock(struct rtl8125_private *tp);;
    void rtl8125_oob_mutex_unlock(struct rtl8125_private *tp);;
    int rtl8125_disable_eee_plus(struct rtl8125_private *tp);
    void rtl8125_clear_tcam_entries(struct rtl8125_private *tp);
    void rtl8125_enable_exit_l1_mask(struct rtl8125_private *tp);
    void rtl8125_init_pci_offset_99(struct rtl8125_private *tp);
    void rtl8125_disable_pci_offset_180(struct rtl8125_private *tp);
    void rtl8125_set_pfm_patch(struct rtl8125_private *tp, bool enable);
    void rtl8125_set_rms(struct rtl8125_private *tp, u16 rms);
    void rtl8125_disable_rxdvgate(struct rtl8125_private *tp);
    void rtl8125_disable_pci_offset_99(struct rtl8125_private *tp);
    void rtl8125_init_pci_offset_180(struct rtl8125_private *tp);
    u8 rtl8125_get_l1off_cap_bits(struct rtl8125_private *tp);
    void rtl8125_hw_clear_timer_int(struct rtl8125_private *tp);
    void rtl8125_hw_clear_int_miti(struct rtl8125_private *tp);
    void rtl8125_ephy_write(struct rtl8125_private *tp, int RegAddr, int value);
    void rtl8125_hw_ephy_config(struct rtl8125_private *tp);
    void rtl8125_hw_phy_config(struct rtl8125_private *tp);
    void rtl8125_phy_restart_nway(struct rtl8125_private *tp);
    int rtl8125_wait_phy_nway_complete_sleep(struct rtl8125_private *tp);
    void rtl8125_setup_mqs_reg(struct rtl8125_private *tp);
    void rtl8125_hw_d3_para(struct rtl8125_private *tp);
    int rtl8125_hw_d3_not_power_off(struct rtl8125_private *tp);
    void rtl8125_irq_mask_and_ack(struct rtl8125_private *tp);
    void rtl8125_driver_stop(struct rtl8125_private *tp);
    int rtl8125_enable_eee(struct rtl8125_private *tp);
    int rtl8125_disable_eee(struct rtl8125_private *tp);
    int rtl8125_enable_eee_plus(struct rtl8125_private *tp);
    void rtl8125_set_reg_oobs_en_sel(struct rtl8125_private *tp, bool enable);
    void rtl8125_disable_ocp_phy_power_saving(struct rtl8125_private *tp);
    u32 rtl8125_csi_read(struct rtl8125_private *tp, u32 addr);
    void rtl8125_csi_write(struct rtl8125_private *tp, u32 addr, u32 value);
    void rtl8125_disable_linkchg_wakeup(struct rtl8125_private *tp);
    void rtl8125_enable_magic_packet(struct rtl8125_private *tp);
    void rtl8125_disable_magic_packet(struct rtl8125_private *tp);
    void rtl8125_disable_d0_speedup(struct rtl8125_private *tp);
    void rtl8125_set_d0_speedup_speed(struct rtl8125_private *tp);
    int rtl8125_check_hw_phy_mcu_code_ver(struct rtl8125_private *tp);

    void rtl8125_enable_giga_lite(struct rtl8125_private *tp, u64 adv);
    void rtl8125_disable_giga_lite(struct rtl8125_private *tp);
    void rtl8125_phy_setup_force_mode(struct rtl8125_private *tp, u32 speed, u8 duplex);

    void rtl8125_apply_firmware(struct rtl8125_private *tp);
    void rtl8125_hw_mac_mcu_config(struct rtl8125_private *tp);
    void rtl8125_get_bios_setting(struct rtl8125_private *tp);
    void rtl8125_set_bios_setting(struct rtl8125_private *tp);
    u8 rtl8125_csi_fun0_read_byte(struct rtl8125_private *tp, u32 addr);
    u32 rtl8125_get_hw_wol(struct rtl8125_private *tp);
    void rtl8125_set_l1_l0s_entry_latency(struct rtl8125_private *tp);
    void rtl8125_hw_init(struct rtl8125_private *tp);
    void ClearMcuAccessRegBit( struct rtl8125_private *tp, u16 addr, u16 mask);
    void SetMcuAccessRegBit(struct rtl8125_private *tp, u16 addr, u16 mask);
    void rtl8125_phy_power_down(struct rtl8125_private *tp);
    void rtl8125_set_hw_phy_before_init_phy_mcu(struct rtl8125_private *tp);
    void rtl8125_set_hw_wol(struct rtl8125_private *tp, u32 wolopts);
    void rtl8125_init_hw_phy_mcu(struct rtl8125_private *tp);
    void rtl8125_clear_eth_phy_bit(struct rtl8125_private *tp, u8 addr, u16 mask);
    void ClearAndSetEthPhyOcpBit(struct rtl8125_private *tp, u16 addr, u16 clearmask, u16 setmask);
    void ClearEthPhyOcpBit(struct rtl8125_private *tp, u16 addr, u16 mask);
    void SetEthPhyOcpBit(struct rtl8125_private *tp,  u16 addr, u16 mask);
    void rtl8125_set_mac_ocp_bit(struct rtl8125_private *tp, u16 addr, u16 mask);

    void linkmode_mod_bit(unsigned int nbit, unsigned long *dst, u32 value);
    void linkmode_set_bit(unsigned int nbit, unsigned int *dst);
}
#else
bool rtl8125_aspm_is_safe(struct rtl8125_private *tp);
bool rtl8125_is_speed_mode_valid(u32 speed);
void rtl8125_set_link_option(struct rtl8125_private *tp, u8 autoneg, u32 speed, u8 duplex,  enum rtl8125_fc_mode fc);

void rtl8125_gset_xmii(struct rtl8125_private *tp, struct ethtool_link_ksettings *cmd);
void rtl8125_xmii_reset_enable(struct rtl8125_private *tp);
unsigned int rtl8125_xmii_reset_pending(struct rtl8125_private *tp);
unsigned int rtl8125_xmii_link_ok(struct rtl8125_private *tp);

void rtl8125_init_software_variable(struct rtl8125_private *tp);
void rtl8125_exit_oob(struct rtl8125_private *tp);

void rtl8125_nic_reset(struct rtl8125_private *tp);
void rtl8125_hw_reset(struct rtl8125_private *tp);
void rtl8125_powerup_pll(struct rtl8125_private *tp);
void rtl8125_powerdown_pll(struct rtl8125_private *tp);

void rtl8125_rar_set(struct rtl8125_private *tp, const u8 *addr);
void rtl8125_disable_rx_packet_filter(struct rtl8125_private *tp);
void rtl8125_enable_cfg9346_write(struct rtl8125_private *tp);
void rtl8125_disable_cfg9346_write(struct rtl8125_private *tp);
void rtl8125_enable_force_clkreq(struct rtl8125_private *tp, bool enable);
void rtl8125_enable_aspm_clkreq_lock(struct rtl8125_private *tp, bool enable);
void rtl8125_set_eee_lpi_timer(struct rtl8125_private *tp);
u32 rtl8125_mdio_direct_read_phy_ocp(struct rtl8125_private *tp, u16 RegAddr);
void mdio_direct_write_phy_ocp(struct rtl8125_private *tp, u16 RegAddr, u16 value);
u32 rtl8125_mdio_read(struct rtl8125_private *tp, u16 RegAddr);
void rtl8125_mdio_write(struct rtl8125_private *tp, u16 RegAddr, u16 value);
void rtl8125_mac_ocp_write(struct rtl8125_private *tp, u16 reg_addr, u16 value);
u16 rtl8125_mac_ocp_read(struct rtl8125_private *tp, u16 reg_addr);
void mac_mcu_write(struct rtl8125_private *tp, u16 reg, u16 value);
u32 mac_mcu_read(struct rtl8125_private *tp, u16 reg);
void rtl8125_enable_tcam(struct rtl8125_private *tp);
void rtl8125_set_l1_l0s_entry_latency(struct rtl8125_private *tp);
void rtl8126_disable_l1_timeout(struct rtl8125_private *tp);
void rtl8125_enable_mcu(struct rtl8125_private *tp, bool enable);
void rtl8125_oob_mutex_lock(struct rtl8125_private *tp);;
void rtl8125_oob_mutex_unlock(struct rtl8125_private *tp);;
int rtl8125_disable_eee_plus(struct rtl8125_private *tp);
void rtl8125_clear_tcam_entries(struct rtl8125_private *tp);
void rtl8125_enable_exit_l1_mask(struct rtl8125_private *tp);
void rtl8125_init_pci_offset_99(struct rtl8125_private *tp);
void rtl8125_disable_pci_offset_180(struct rtl8125_private *tp);
void rtl8125_set_pfm_patch(struct rtl8125_private *tp, bool enable);
void rtl8125_set_rms(struct rtl8125_private *tp, u16 rms);
void rtl8125_disable_rxdvgate(struct rtl8125_private *tp);
void rtl8125_disable_pci_offset_99(struct rtl8125_private *tp);
void rtl8125_init_pci_offset_180(struct rtl8125_private *tp);
u8 rtl8125_get_l1off_cap_bits(struct rtl8125_private *tp);
void rtl8125_hw_clear_timer_int(struct rtl8125_private *tp);
void rtl8125_hw_clear_int_miti(struct rtl8125_private *tp);
void rtl8125_ephy_write(struct rtl8125_private *tp, int RegAddr, int value);
void rtl8125_hw_ephy_config(struct rtl8125_private *tp);
void rtl8125_hw_phy_config(struct rtl8125_private *tp);
void rtl8125_phy_restart_nway(struct rtl8125_private *tp);
int rtl8125_wait_phy_nway_complete_sleep(struct rtl8125_private *tp);
void rtl8125_setup_mqs_reg(struct rtl8125_private *tp);
void rtl8125_hw_d3_para(struct rtl8125_private *tp);
int rtl8125_hw_d3_not_power_off(struct rtl8125_private *tp);
void rtl8125_irq_mask_and_ack(struct rtl8125_private *tp);
void rtl8125_driver_stop(struct rtl8125_private *tp);
int rtl8125_enable_eee(struct rtl8125_private *tp);
int rtl8125_disable_eee(struct rtl8125_private *tp);
int rtl8125_enable_eee_plus(struct rtl8125_private *tp);
void rtl8125_set_reg_oobs_en_sel(struct rtl8125_private *tp, bool enable);
void rtl8125_disable_ocp_phy_power_saving(struct rtl8125_private *tp);
u32 rtl8125_csi_read(struct rtl8125_private *tp, u32 addr);
void rtl8125_csi_write(struct rtl8125_private *tp, u32 addr, u32 value);
void rtl8125_disable_linkchg_wakeup(struct rtl8125_private *tp);
void rtl8125_enable_magic_packet(struct rtl8125_private *tp);
void rtl8125_disable_magic_packet(struct rtl8125_private *tp);
void rtl8125_disable_d0_speedup(struct rtl8125_private *tp);
void rtl8125_set_d0_speedup_speed(struct rtl8125_private *tp);
int rtl8125_check_hw_phy_mcu_code_ver(struct rtl8125_private *tp);

void rtl8125_enable_giga_lite(struct rtl8125_private *tp, u64 adv);
void rtl8125_disable_giga_lite(struct rtl8125_private *tp);
void rtl8125_phy_setup_force_mode(struct rtl8125_private *tp, u32 speed, u8 duplex);

void rtl8125_apply_firmware(struct rtl8125_private *tp);
void rtl8125_hw_mac_mcu_config(struct rtl8125_private *tp);
void rtl8125_get_bios_setting(struct rtl8125_private *tp);
void rtl8125_set_bios_setting(struct rtl8125_private *tp);
u8 rtl8125_csi_fun0_read_byte(struct rtl8125_private *tp, u32 addr);
u32 rtl8125_get_hw_wol(struct rtl8125_private *tp);
void rtl8125_set_l1_l0s_entry_latency(struct rtl8125_private *tp);
void ClearMcuAccessRegBit( struct rtl8125_private *tp, u16 addr, u16 mask);
void SetMcuAccessRegBit(struct rtl8125_private *tp, u16 addr, u16 mask);
void rtl8125_phy_power_down(struct rtl8125_private *tp);
void rtl8125_set_hw_phy_before_init_phy_mcu(struct rtl8125_private *tp);
void rtl8125_set_hw_wol(struct rtl8125_private *tp, u32 wolopts);
void rtl8125_init_hw_phy_mcu(struct rtl8125_private *tp);
void rtl8125_clear_eth_phy_bit(struct rtl8125_private *tp, u8 addr, u16 mask);
void ClearAndSetEthPhyOcpBit(struct rtl8125_private *tp, u16 addr, u16 clearmask, u16 setmask);
void ClearEthPhyOcpBit(struct rtl8125_private *tp, u16 addr, u16 mask);
void SetEthPhyOcpBit(struct rtl8125_private *tp,  u16 addr, u16 mask);
void rtl8125_set_mac_ocp_bit(struct rtl8125_private *tp, u16 addr, u16 mask);

void linkmode_mod_bit(unsigned int nbit, unsigned long *dst, u32 value);
void linkmode_set_bit(unsigned int nbit, unsigned int *dst);
#endif // __cplusplus

#endif /* rtl812x_h */
