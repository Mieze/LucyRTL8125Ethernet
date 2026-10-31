/* LucyRTL8125Ethernet.hpp -- RTL8125 driver class definition.
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

//#include "LucyRTL8125Linux-900501.hpp"
#include "rtl812x.h"
#include "rtl812x_dash.h"
#include "RTL8125LucyRxPool.hpp"

#ifdef DEBUG
#define DebugLog(args...) IOLog(args)
#else
#define DebugLog(args...)
#endif

#define    RELEASE(x)    if(x){(x)->release();(x)=NULL;}

#define super IOEthernetController

enum
{
    MIDX_AUTO = 0,
    MIDX_10HD,
    MIDX_10FD,
    MIDX_100HD,
    MIDX_100FD,
    MIDX_100FDFC,
    MIDX_100FD_EEE,
    MIDX_100FDFC_EEE,
    MIDX_1000FD,
    MIDX_1000FDFC,
    MIDX_1000FD_EEE,
    MIDX_1000FDFC_EEE,
    MIDX_2500FD,
    MIDX_2500FDFC,
    MIDX_2500FD_EEE,
    MIDX_2500FDFC_EEE,
    MIDX_5000FD,
    MIDX_5000FDFC,
    MIDX_5000FD_EEE,
    MIDX_5000FDFC_EEE,
    MIDX_10000FD,
    MIDX_10000FDFC,
    MIDX_10000FD_EEE,
    MIDX_10000FDFC_EEE,
    MIDX_COUNT
};

#define MBit 1000000ULL

enum {
    kSpeed10000MBit = 10000*MBit,
    kSpeed5000MBit = 5000*MBit,
    kSpeed2500MBit = 2500*MBit,
    kSpeed1000MBit = 1000*MBit,
    kSpeed100MBit = 100*MBit,
    kSpeed10MBit = 10*MBit,
};

enum {
    kEEETypeNo = 0,
    kEEETypeYes = 1,
    kEEETypeCount
};

struct rtlMediumTable {
    IOMediumType    type;
    UInt64          spd;
    UInt32          idx;
    UInt32          speed;
    UInt32          duplex;
    UInt32          fc;
    UInt32          eee;
    UInt64          adv;
};

#define kMacHdrLen      14
#define kIPv4HdrLen     20
#define kIPv6HdrLen     40

enum RtlStateFlags {
    __ENABLED = 0,      /* driver is enabled */
    __LINK_UP = 1,      /* link is up */
    __PROMISC = 2,      /* promiscuous mode enabled */
    __M_CAST = 3,       /* multicast mode enabled */
    __POLL_MODE = 4,    /* poll mode is active */
    __POLLING = 5,      /* poll routine is polling */
};

enum RtlStateMask {
    __ENABLED_M = (1 << __ENABLED),
    __LINK_UP_M = (1 << __LINK_UP),
    __PROMISC_M = (1 << __PROMISC),
    __M_CAST_M = (1 << __M_CAST),
    __POLL_MODE_M = (1 << __POLL_MODE),
    __POLLING_M = (1 << __POLLING),
};

/* RTL8125's Rx descriptor. */
typedef union RtlRxDesc {
    struct {
        UInt32 opts1;
        UInt32 opts2;
        UInt64 addr;
    } cmd;
    struct {
        UInt64 blen;
        UInt64 addr;
    } buf;
} RtlRxDesc;

/* RTL8125's Tx descriptor. */
typedef struct RtlTxDesc {
    UInt32 opts1;
    UInt32 opts2;
    UInt64 addr; /*
    UInt32 reserved0;
    UInt32 reserved1;
    UInt32 reserved2;
    UInt32 reserved3; */
} RtlTxDesc;

/* RTL8125's statistics dump data structure */
typedef struct RtlStatData {
    UInt64 txPackets;
    UInt64 rxPackets;
    UInt64 txErrors;
    UInt32 rxErrors;
    UInt16 rxMissed;
    UInt16 alignErrors;
    UInt32 txOneCollision;
    UInt32 txMultiCollision;
    UInt64 rxUnicast;
    UInt64 rxBroadcast;
    UInt32 rxMulticast;
    UInt16 txAborted;
    UInt16 txUnderun;
    /* new since RTL8125 */
    UInt64 txOctets;
    UInt64 rxOctets;
    UInt64 rxMulticast64;
    UInt64 txUnicast64;
    UInt64 txBroadcast64;
    UInt64 txMulticast64;
    UInt32 txPauseOn;
    UInt32 txPauseOff;
    UInt32 txPauseAll;
    UInt32 txDeferred;
    UInt32 txLateCollision;
    UInt32 txAllCollision;
    UInt32 txAborted32;
    UInt32 alignErrors32;
    UInt32 rxFrame2Long;
    UInt32 rxRunt;
    UInt32 rxPauseOn;
    UInt32 rxPauseOff;
    UInt32 rxPauseAll;
    UInt32 rxUnknownOpcode;
    UInt32 rxMacError;
    UInt32 txUnderrun32;
    UInt32 rxMacMissed;
    UInt32 rxTcamDropped;
    UInt32 tdu;
    UInt32 rdu;
} RtlStatData;

#define kTransmitQueueCapacity  1024

/* With up to 40 segments we should be on the save side. */
#define kMaxSegs 40

/* The number of descriptors must be a power of 2. */
#define kNumTxDesc    1024    /* Number of Tx descriptors */
#define kNumRxDesc    512    /* Number of Rx descriptors */
#define kTxLastDesc    (kNumTxDesc - 1)
#define kRxLastDesc    (kNumRxDesc - 1)
#define kTxDescMask    (kNumTxDesc - 1)
#define kRxDescMask    (kNumRxDesc - 1)
#define kTxDescSize    (kNumTxDesc*sizeof(struct RtlTxDesc))
#define kRxDescSize    (kNumRxDesc*sizeof(union RtlRxDesc))
#define kRxBufArraySize (kNumRxDesc * sizeof(struct rtlRxBufferInfo))
#define kTxBufArraySize (kNumTxDesc * sizeof(mbuf_t))

/* Numbers of IOMemoryDescriptors and IORanges for tx */
#define kNumTxMemDesc       (kNumTxDesc / 2)
#define kTxMemDescMask      (kNumTxMemDesc - 1)
#define kNumTxRanges        (kNumTxDesc + kMaxSegs)
#define kTxRangeMask        kTxDescMask
#define kTxMapMemSize       sizeof(struct rtlTxMapInfo)

/* Numbers of IOMemoryDescriptors and batch size for rx */
#define kRxMemBaseShift 4
#define kNumRxMemDesc   (kNumRxDesc >> kRxMemBaseShift)
#define kRxMapMemSize   (sizeof(struct rtlRxMapInfo))
#define kRxMemBatchSize (kNumRxDesc / kNumRxMemDesc)
#define kRxMemDescMask  (kRxMemBatchSize - 1)
#define kRxMemBaseMask  ~kRxMemDescMask


/* This is the receive buffer size (must be large enough to hold a packet). */
#define kRxBufferSize   PAGE_SIZE

#define kMCFilterLimit  32
#define kMaxMtu 9000
#define kMaxPacketSize (kMaxMtu + ETH_HLEN + VLAN_HLEN + ETH_FCS_LEN)

/* statitics dump delay in ns. */
#define kStatDelayTime 1000000UL    /* 1ms */

/* RealtekRxPool capacities */
#define kRxPoolClstCap   100    /* mbufs with 4k cluster*/
#define kRxPoolMbufCap   50     /* mbufs without clusters */

/* Treshhold value to wake a stalled queue */
#define kTxQueueWakeTreshhold (kNumTxDesc / 10)

/* transmitter deadlock treshhold in seconds. */
#define kTxDeadlockTreshhold 6
#define kTxCheckTreshhold (kTxDeadlockTreshhold - 1)
#define kTimerPeriod    1000000000UL

/* MSS value position */
#define MSSShift_8125 18

/* This definitions should have been in IOPCIDevice.h. */
enum
{
    kIOPCIPMCapability = 2,
    kIOPCIPMControl = 4,
};

enum
{
    kIOPCIELinkCapability = 12,
    kIOPCIELinkControl = 16,
};

enum
{
    kIOPCIELinkCtlASPM = 0x0003,    /* ASPM Control */
    kIOPCIELinkCtlL0s = 0x0001,     /* L0s Enable */
    kIOPCIELinkCtlL1 = 0x0002,      /* L1 Enable */
    kIOPCIELinkCtlClkPM = 0x0004,   /* Clock PM Enable */
    kIOPCIELinkCtlCcc = 0x0040,     /* Common Clock Configuration */
    kIOPCIELinkCtlClkReqEn = 0x100, /* Enable clkreq */
};

enum
{
    kPowerStateOff = 0,
    kPowerStateOn,
    kPowerStateCount
};

#define kParamName "Driver Parameters"
#define kEnableASPM "enableASPM"
#define kEnableCSO6Name "enableCSO6"
#define kEnableTSO4Name "enableTSO4"
#define kEnableTSO6Name "enableTSO6"
#define kPollTime10GName "µsPollTime10G"
#define kPollTime5GName "µsPollTime5G"
#define kPollTime2GName "µsPollTime2G"
#define kDriverVersionName "Driver Version"
#define kFallbackName "fallbackMAC"
#define kNameLenght 64

extern const struct RTLChipInfo rtl_chip_info[];

/*
 * Indicates if a tx IOMemoryDescriptor is in the prepared
 * (active) or completed state (inactive).
 */
enum
{
    kIOMemoryInactive = 0,
    kIOMemoryActive = 1
};

typedef struct rtlTxMapInfo {
    UInt16 txNextMem2Use;
    UInt16 txNextMem2Free;
    SInt16 txNumFreeMem;
    IOMemoryDescriptor *txMemIO[kNumTxMemDesc];
    IOAddressRange txMemRange[kNumTxRanges];
    IOAddressRange txSCRange[kMaxSegs];
} rtlTxMapInfo;

typedef struct rtlRxMapInfo {
    IOMemoryDescriptor *rxMemIO[kNumRxMemDesc];
    IOAddressRange rxMemRange[kNumRxDesc];
} rtlRxMapInfo;

typedef struct rtlRxBufferInfo {
    mbuf_t mbuf;
    IOPhysicalAddress64 phyAddr;
} rtlRxBufferInfo;



/**
 *  Known kernel versions
 */
enum KernelVersion {
    Tiger         = 8,
    Leopard       = 9,
    SnowLeopard   = 10,
    Lion          = 11,
    MountainLion  = 12,
    Mavericks     = 13,
    Yosemite      = 14,
    ElCapitan     = 15,
    Sierra        = 16,
    HighSierra    = 17,
    Mojave        = 18,
    Catalina      = 19,
    BigSur        = 20,
    Monterey      = 21,
    Ventura       = 22,
    Sonoma        = 23,
    Sequoia       = 24,
    Tahoe         = 25,
};

/**
 *  Kernel version major
 */
extern const int version_major;

class LucyRTL8125 : public super
{
    
    OSDeclareDefaultStructors(LucyRTL8125)
    
public:
    /* IOService (or its superclass) methods. */
    virtual bool start(IOService *provider) override;
    virtual void stop(IOService *provider) override;
    virtual bool init(OSDictionary *properties) override;
    virtual void free() override;
    
    /* Power Management Support */
    virtual IOReturn registerWithPolicyMaker(IOService *policyMaker) override;
    virtual IOReturn setPowerState(unsigned long powerStateOrdinal, IOService *policyMaker ) override;
    virtual void systemWillShutdown(IOOptionBits specifier) override;

    /* IONetworkController methods. */
    virtual IOReturn enable(IONetworkInterface *netif) override;
    virtual IOReturn disable(IONetworkInterface *netif) override;
    
    virtual IOReturn outputStart(IONetworkInterface *interface, IOOptionBits options ) override;
    virtual IOReturn setInputPacketPollingEnable(IONetworkInterface *interface, bool enabled) override;
    virtual void pollInputPackets(IONetworkInterface *interface, uint32_t maxCount, IOMbufQueue *pollQueue, void *context) override;
    
    virtual void getPacketBufferConstraints(IOPacketBufferConstraints *constraints) const override;
    
    virtual IOOutputQueue* createOutputQueue() override;
    
    virtual const OSString* newVendorString() const override;
    virtual const OSString* newModelString() const override;
    
    virtual IOReturn selectMedium(const IONetworkMedium *medium) override;
    virtual bool configureInterface(IONetworkInterface *interface) override;
    
    virtual bool createWorkLoop() override;
    virtual IOWorkLoop* getWorkLoop() const override;
    
    /* Methods inherited from IOEthernetController. */
    virtual IOReturn getHardwareAddress(IOEthernetAddress *addr) override;
    virtual IOReturn setHardwareAddress(const IOEthernetAddress *addr) override;
    virtual IOReturn setPromiscuousMode(bool active) override;
    virtual IOReturn setMulticastMode(bool active) override;
    virtual IOReturn setMulticastList(IOEthernetAddress *addrs, UInt32 count) override;
    virtual IOReturn getChecksumSupport(UInt32 *checksumMask, UInt32 checksumFamily, bool isOutput) override;
    virtual IOReturn setWakeOnMagicPacket(bool active) override;
    virtual IOReturn getPacketFilters(const OSSymbol *group, UInt32 *filters) const override;
    
    virtual UInt32 getFeatures() const override;
    virtual IOReturn getMaxPacketSize(UInt32 * maxSize) const override;
    virtual IOReturn setMaxPacketSize(UInt32 maxSize) override;
    
private:
    bool initPCIConfigSpace(IOPCIDevice *provider);
    static IOReturn setPowerStateWakeAction(OSObject *owner, void *arg1, void *arg2, void *arg3, void *arg4);
    static IOReturn setPowerStateSleepAction(OSObject *owner, void *arg1, void *arg2, void *arg3, void *arg4);
    void getParams();
    bool setupMediumDict();
    bool initEventSources(IOService *provider);
    
    void interruptOccurred(OSObject *client, IOInterruptEventSource *src, int count);
    UInt32 rxInterrupt(IONetworkInterface *interface, uint32_t maxCount, IOMbufQueue *pollQueue, void *context);
    void txInterrupt();

    bool setupRxResources();
    bool setupTxResources();
    bool setupStatResources();
    void freeRxResources();
    void freeTxResources();
    void freeStatResources();
    
    static IOReturn refillAction(OSObject *owner, void *arg1, void *arg2, void *arg3, void *arg4);

    void clearRxTxRings();
    void discardPacketFragment();
    void rtl812xCheckLinkStatus(struct rtl8125_private *tp);
    void setLinkUp();
    void setLinkDown();
    bool txHangCheck();

    /* Hardware initialization methods. */
    bool rtl812xIdentifyChip(struct rtl8125_private *tp);
    bool rtl812xInit();
    void initMacAddr(struct rtl8125_private *tp);
    void rtl812xEnable();
    void rtl812xDisable();
    void rtl812xHwInit(struct rtl8125_private *tp);
    void rtl812xHwConfig(struct rtl8125_private *tp);
    void rtl812xSetMrrs(struct rtl8125_private *tp, UInt8 setting);
    void rtl812xSetOffloadFeatures(bool active);
    void rtl812xRestart();
    void rtl812xSetPhyMedium(struct rtl8125_private *tp, UInt8 autoneg, UInt32 speed, UInt8 duplex, UInt64 adv);
    void rtl812xMedium2Adv(struct rtl8125_private *tp, UInt32 index);
    void rtl812xGetEEEMode(struct rtl8125_private *tp);
    void rtl812xLinkDownPatch(struct rtl8125_private *tp);
    void rtl812xLinkOnPatch(struct rtl8125_private *tp);
    UInt32 rtl812xGetHwCloPtr(struct rtl8125_private *tp);
    void rtl812xDoorbell(struct rtl8125_private *tp, UInt32 txTailPtr);

    void rtl812xDumpTallyCounter(struct rtl8125_private *tp);
    static void runStatUpdateThread(thread_call_param_t param0);
    void statUpdateThread();

    /* Descriptor related methods. */
    void getChecksumResult(mbuf_t m, UInt32 status1, UInt32 status2);
    
    /* AppleVTD support methods*/
    bool setupRxMap();
    void freeRxMap();
    bool setupTxMap();
    void freeTxMap();

    void    interruptOccurredVTD(OSObject *client, IOInterruptEventSource *src, int count);
    UInt32  rxInterruptVTD(IONetworkInterface *interface, uint32_t maxCount,
                           IOMbufQueue *pollQueue, void *context);
    UInt32  txMapPacket(mbuf_t packet, IOPhysicalSegment *vector, UInt32 maxSegs);
    void    txUnmapPacket();
    UInt16  rxMapBuffers(UInt16 index, UInt16 count);

    /* Watchdog timer method. */
    void timerActionRTL8125(IOTimerEventSource *timer);

private:
    IOWorkLoop *workLoop;
    IOCommandGate *commandGate;
    IOPCIDevice *pciDevice;
    OSDictionary *mediumDict;
    IONetworkMedium *mediumTable[MIDX_COUNT];
    IOBasicOutputQueue *txQueue;
    
    IOInterruptEventSource *interruptSource;
    IOTimerEventSource *timerSource;
    IOEthernetInterface *netif;
    IOMemoryMap *baseMap;
    IOMapper *mapper;
    
    /* transmitter data */
    IOBufferMemoryDescriptor *txBufDesc;
    IOPhysicalAddress64 txPhyAddr;
    IODMACommand *txDescDmaCmd;
    struct RtlTxDesc *txDescArray;
    IOMbufNaturalMemoryCursor *txMbufCursor;
    mbuf_t *txMbufArray;
    void *txBufArrayMem;
    rtlTxMapInfo *txMapInfo;
    void *txMapMem;
    UInt64 txDescDoneCount;
    UInt64 txDescDoneLast;
    UInt32 txNextDescIndex;
    UInt32 txDirtyDescIndex;
    UInt32 txTailPtr0;
    UInt32 txClosePtr0;
    SInt32 txNumFreeDesc;

    /* receiver data */
    IOBufferMemoryDescriptor *rxBufDesc;
    IOPhysicalAddress64 rxPhyAddr;
    IODMACommand *rxDescDmaCmd;
    RtlRxDesc *rxDescArray;
    rtlRxBufferInfo *rxBufArray;
    void *rxBufArrayMem;
    void *rxMapMem;
    rtlRxMapInfo *rxMapInfo;
    RTL8125LucyRxPool *rxPool;
    UInt64 multicastFilter;
    mbuf_t rxPacketHead;
    mbuf_t rxPacketTail;
    SInt32 rxPacketSize;
    UInt16 rxNextDescIndex;
    UInt16 rxMapNextIndex;

    /* power management data */
    unsigned long powerState;
    IOByteCount pcieCapOffset;
    IOByteCount pciPMCtrlOffset;

    /* statistics data */
    UInt32 deadlockWarn;
    IONetworkStats *netStats;
    IOEthernetStats *etherStats;
    IOBufferMemoryDescriptor *statBufDesc;
    IOPhysicalAddress64 statPhyAddr;
    IODMACommand *statDescDmaCmd;
    thread_call_t statCall;
    struct RtlStatData *statData;

    UInt32 mtu;
    struct pci_dev pciDeviceData;
    struct rtl8125_private linuxData;
    const struct RtlChipInfo *rtlChipInfos;
    struct IOEthernetAddress currMacAddr;
    struct IOEthernetAddress origMacAddr;
    struct IOEthernetAddress fallBackMacAddr;
    IONetworkPacketPollingParameters pollParms;

    UInt32 intrMask;
    UInt32 intrMaskRxTx;
    UInt32 intrMaskTimer;
    UInt32 intrMaskPoll;

    /* poll intervals in ns */
    UInt64 pollTime10G;
    UInt64 pollTime5G;
    UInt64 pollTime2G;
    UInt64 statDelay;
    UInt64 timerInterval;

    /* flags */
    UInt32 stateFlags;
    
    bool wolCapable;
    bool wolActive;
    bool enableASPM;
    bool enableTSO4;
    bool enableTSO6;
    bool useAppleVTD;
    
#ifdef DEBUG_INTR
    UInt32 tmrInterrupts;
    UInt32 lastRxIntrupts;
    UInt32 lastTxIntrupts;
    UInt32 lastTmrIntrupts;
#endif
};
