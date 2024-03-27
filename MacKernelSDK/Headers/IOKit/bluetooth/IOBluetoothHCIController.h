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

#ifndef _IOKIT_IOBLUETOOTHHCICONTROLLER_H
#define _IOKIT_IOBLUETOOTHHCICONTROLLER_H

#include <IOKit/bluetooth/Bluetooth.h>
#include <IOKit/bluetooth/IOBluetoothHostController.h>
#include <IOKit/usb/IOUSBHostDevice.h>
#include <os/log.h>

#include <Availability.h>

#ifndef __MAC_OS_X_VERSION_MIN_REQUIRED
#error "Missing macOS target version"
#endif

class IOACPIPlatformDevice;
class IOWorkQueue;
class IODisplayWrangler;
class IOBluetoothACPIMethods;
class IOBluetoothPacketLogger;
class IOBluetoothHCIPacketLogUserClient;
class IOBluetoothHostController;
class IOBluetoothHostControllerTransport;
class IOBluetoothHostControllerUSBTransport;
class IOBluetoothHostControllerUARTTransport;

struct BluetoothHardwareListType
{
    IOBluetoothHostControllerTransport * mBluetoothTransport;      // 0
    IOBluetoothHostController *          mBluetoothHostController; // 8
    bool                                 mFoundWithTransport;      // 16
    bool                                 mCreated;                 // 17
    bool                                 unknown3;                 // 18
    UInt8                                mHardResetCounter;        // 19
    UInt8                                unknown4;                 // 20
    BluetoothHardwareListType *          mNextHardware;            // 24
    BluetoothHardwareListType *          mPreviousHardware;        // 32
};

struct HCIEventNotificationListener
{
    task_t      owningTask;
    mach_port_t port;
    void *      refCon;
    UInt64      unknown; // All set to 1
};

#if __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_15
struct BluetoothPacketLogData
{
    UInt32 creationTime;
    UInt32 packetSize;
    UInt8  packetType;
};
#endif

IOBLUETOOTH_EXPORT UInt32 IOBluetoothRingBufferRead(UInt32 *, void * outBuffer, UInt32, UInt32);
IOBLUETOOTH_EXPORT UInt32 IOBluetoothRingBufferReadAtOffset(UInt32 *, void *, UInt32, IOByteCount offset, UInt32);
IOBLUETOOTH_EXPORT UInt32 IOBluetoothRingBufferWrite(UInt32 *, void *, UInt32, UInt32);
IOBLUETOOTH_EXPORT UInt32 IOBluetoothRingBufferWriteAtOffset(UInt32 *, void *, UInt32, IOByteCount offset, UInt32);

extern bool SearchForTransportEventTimeOutOccurred(OSObject * owner, IOTimerEventSource * timer);
extern void FullWakeTimeOutOccurred(OSObject * owner, IOTimerEventSource * timer);
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_11_0
extern void RecoveryTimeOutOccurred(OSObject * owner, IOTimerEventSource * timer);
#endif

class IOBluetoothHCIController : public IOService
{
    OSDeclareDefaultStructors(IOBluetoothHCIController)

    friend class IOBluetoothHostController;
    friend class IOBluetoothHostControllerTransport;
    friend class IOBluetoothHostControllerUSBTransport;
    friend class IOBluetoothHostControllerUARTTransport;

public:
    virtual bool     init(OSDictionary * dictionary = NULL) APPLE_KEXT_OVERRIDE;
    virtual void     free() APPLE_KEXT_OVERRIDE;
    virtual bool     start(IOService * provider) APPLE_KEXT_OVERRIDE;
    virtual void     stop(IOService * provider) APPLE_KEXT_OVERRIDE;
    static IOReturn  stopAction(OSObject * owner, void * arg0, void * arg1, void * arg2, void * arg3);
    virtual IOReturn stopWL(IOService * provider);
    virtual bool     terminate(IOOptionBits options = 0) APPLE_KEXT_OVERRIDE;
    static IOReturn  terminateAction(OSObject * owner, void * arg0, void * arg1, void * arg2, void * arg3);
    virtual IOReturn terminateWL(IOOptionBits options = 0);

    virtual IOReturn newUserClient(task_t owningTask, void * securityID, UInt32 type, LIBKERN_RETURNS_RETAINED IOUserClient ** handler) APPLE_KEXT_OVERRIDE;
#if __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_15
    virtual IOReturn AddPacketLogUserClient(IOBluetoothHCIPacketLogUserClient * client);
#endif
    virtual IOReturn AddCommandUserClient(void * userClient);
    virtual void     DetachUserClients();

    virtual IOReturn setProperties(OSObject * properties) APPLE_KEXT_OVERRIDE;
    static IOReturn  setPropertiesAction(OSObject *, void *, void *, void *, void *);
    virtual IOReturn setPropertiesWL(OSObject * properties);
    virtual bool     ValidProperty(OSSymbol *);

    virtual void terminateServiceNubs();
    virtual bool terminateServiceNubsComplete();

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    static void USBHardResetEntry(OSObject * owner);
    static bool PacketLoggerPublishNotificationHandler(void *, void *, IOService *, IONotifier *);
    static bool HandlePacketLoggerPublishNotification(void *, IOService *);
#else
    static void HardwareResetEntry(OSObject * owner);
#endif
    static bool DisplayManagerPublishNotificationHandler(void *, void *, IOService *, IONotifier *);
    static bool HandleDisplayManagerPublishNotification(void *, IOService *);
    static bool ExternalDisplayTerminateNotificationHandler(void *, void *, IOService *, IONotifier *);
    static bool HandleExternalDisplayTerminateNotification(void *, IOService *);
    static bool ExternalDisplayPublishNotificationHandler(void *, void *, IOService *, IONotifier *);
    static bool HandleExternalDisplayPublishNotification(void *, IOService *);
    static bool staticBluetoothTransportGoesAway(void *, void *, IOService *, IONotifier *);
    static bool staticBluetoothTransportShowsUp(void *, void *, IOService *, IONotifier *);

    virtual IOReturn AddHCIEventNotification(task_t inOwningTask, mach_port_t inPort, void * refCon);
    virtual IOReturn RemoveHCIEventNotification(task_t inOwningTask);

    virtual bool GetNvramPacketLoggerBufferSize(UInt32 *);

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_11_0
    virtual bool NeedToWaitForControllerToShowUp();
#endif

    virtual IOWorkLoop *    getWorkLoop() const APPLE_KEXT_OVERRIDE;
    virtual IOCommandGate * getCommandGate() const;

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    virtual IOWorkLoop *    getUSBHardResetWorkLoop() const;
    virtual IOCommandGate * getUSBHardResetCommandGate() const;
#else
    virtual bool PacketLogFillBufferedData(IOBluetoothHCIPacketLogUserClient * client);
    virtual void PacketLogSetBufferEmptiedFlag();
    virtual void PacketLogClientClosed(IOBluetoothHCIPacketLogUserClient * client);
#endif
    virtual void LogPacket(UInt8 packetType, void * packetData, size_t packetSize);
#if __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_15
    static IOReturn LogPacketAction(OSObject * owner, void * arg0, void * arg1, void * arg2, void * arg3, void * arg4, void * arg5);
    virtual void LogPacketData(UInt8 packetType, void * packetData, size_t packetSize);
#endif

    virtual bool     shouldOverrideExistingController(IOBluetoothHCIController *, BluetoothHardwareListType *);
    virtual IOReturn SwitchToSelectedHostController(UInt32 locationID);

    static IOReturn  ProcessBluetoothTransportGoesAwayAction(IOBluetoothHCIController *, UInt8 *, UInt32);
    virtual IOReturn ProcessBluetoothTransportGoesAwayActionWL(IOBluetoothHostControllerTransport *);
    static IOReturn  ProcessBluetoothTransportShowsUpAction(IOBluetoothHCIController *, UInt8 *, UInt32);
    virtual IOReturn ProcessBluetoothTransportShowsUpActionWL(IOBluetoothHostControllerTransport *);

    virtual IOReturn SetBluetoothTransportTerminateState(IOBluetoothHostControllerTransport *, UInt8);
    virtual IOReturn GetControllerDeviceAddress(IOBluetoothHostController *, BluetoothDeviceAddress *);

    virtual BluetoothHardwareListType * FindFirstBluetoothHardwareHasTransport();
    virtual BluetoothHardwareListType * FindBluetoothHardware(BluetoothDeviceAddress *);
    virtual BluetoothHardwareListType * FindBluetoothHardware(UInt16 vendorID, UInt16 productID, UInt32 locationID);
    virtual BluetoothHardwareListType * FindBluetoothHardware(IOBluetoothHostController *);
    virtual BluetoothHardwareListType * FindBluetoothHardware(IOBluetoothHostControllerTransport *);
    virtual BluetoothHardwareListType * FindBluetoothHardware(BluetoothHardwareListType *);
    virtual BluetoothHardwareListType * FindBluetoothHardware(UInt32);

    virtual BluetoothHardwareListType * FindInternalBluetoothController();

    virtual IOReturn AddBluetoothHardware(BluetoothHardwareListType *);
    virtual IOReturn RemoveBluetoothHardware(BluetoothHardwareListType *, bool);
    virtual IOReturn SwitchBluetoothHardware(BluetoothHardwareListType *);
    virtual void     PrintBluetoothHardwareList(bool);

    virtual void     SearchForTransportEventTimeOutHandler();
    virtual bool     WriteActiveControllerInfoToNVRAM(UInt16 productID, UInt16 vendorID, UInt8 * deviceAddress, UInt32 locationID, UInt16 activeConnections);
    virtual bool     ReadActiveControllerInfoFromNVRAM(UInt16 * productID, UInt16 * vendorID, UInt8 * deviceAddress, UInt32 * locationID, UInt16 * activeConnections);
    virtual IOReturn UpdateNVRAMControllerInfo();
    virtual bool     WriteInternalControllerInfoToNVRAM(UInt16, UInt16, UInt8 *, UInt32);
    virtual bool     ReadInternalControllerInfoFromNVRAM(UInt16 *, UInt16 *, UInt8 *, UInt32 *);
    virtual IOReturn CreateBluetoothHostControllerObject(BluetoothHardwareListType *);
    virtual IOReturn RemoveBluetoothHostControllerObject(BluetoothHardwareListType *);
    virtual IOReturn DisableBluetoothHostControllerObject(BluetoothHardwareListType *);
    virtual IOReturn EnableBluetoothHostControllerObject(BluetoothHardwareListType *);

    virtual void     WakeUpDisplay();
    virtual void     FullWakeTimeOutHandler();
    virtual IOReturn StartFullWakeTimer();
    virtual void     CancelFullWakeTimer();

    virtual bool IsTBFCSupported();
    virtual bool IsAnyDevicesTBFCPageCapable();
    virtual bool SupportDeepIdle();
    virtual bool isSystemPortable();
    virtual void SetIdlePolicy(void *, bool);
    virtual bool BluetoothRemoteWakeEnabled();
    virtual bool G3StandbySleepWithFileVaultOn();

    virtual bool     SetActiveControllerInfoToPropertyList(UInt16 productID, UInt16 vendorID, UInt8 * deviceAddress, UInt32 locationID, UInt16 activeConnections);
    virtual bool     AllControllersFinishedSettingUp();
    virtual IOReturn FamilyCommandSleep(void * event, UInt32 milliseconds, char * calledByFunction, bool panicMachine);
    virtual UInt32   GetCurrentTime();
    virtual UInt32   ConvertAddressToUInt32(void *);
    virtual bool     ActiveControllerOnline();
    virtual bool     SpecialWakeReason();

    virtual bool setProperty(const OSSymbol * aKey, OSObject * anObject) APPLE_KEXT_OVERRIDE;
    virtual bool setProperty(const OSString * aKey, OSObject * anObject) APPLE_KEXT_OVERRIDE;
    virtual bool setProperty(const char * aKey, OSObject * anObject) APPLE_KEXT_OVERRIDE;
    virtual bool setProperty(const char * aKey, const char * aString) APPLE_KEXT_OVERRIDE;
    virtual bool setProperty(const char * aKey, bool aBoolean) APPLE_KEXT_OVERRIDE;
    virtual bool setProperty(const char * aKey, unsigned long long aValue, unsigned int aNumberOfBits) APPLE_KEXT_OVERRIDE;
    virtual bool setProperty(const char * aKey, void * bytes, unsigned int length) APPLE_KEXT_OVERRIDE;

    virtual bool SetBluetoothFamilyProperty(const OSSymbol * aKey, OSObject * anObject);
    virtual bool SetBluetoothFamilyProperty(const OSString * aKey, OSObject * anObject);
    virtual bool SetBluetoothFamilyProperty(const char * aKey, OSObject * anObject);
    virtual bool SetBluetoothFamilyProperty(const char * aKey, const char * aString);
    virtual bool SetBluetoothFamilyProperty(const char * aKey, bool aBoolean);
    virtual bool SetBluetoothFamilyProperty(const char * aKey, unsigned long long aValue, unsigned int aNumberOfBits);
    virtual bool SetBluetoothFamilyProperty(const char * aKey, void * bytes, unsigned int length);

    virtual bool     UpdateActiveControllerProperties(char *);
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    virtual UInt32   GetNextBluetoothObjectID();
    virtual IOReturn FreeBluetoothObjectID(UInt32);
    virtual void     CheckACPIMethodsAvailabilities();
#else
    virtual void     CheckSpecialGPIO();
    virtual IOReturn SetBTRS();
    virtual IOReturn SetBTPU();
    virtual IOReturn SetBTPD();
    virtual IOReturn SetBTRB(bool);
    virtual IOReturn SetBTLP(bool);
#endif
    virtual IOReturn CallRegisterService();

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    virtual bool  ReachHardResetLimit(IOBluetoothHostController *);
    virtual void  IncrementHardResetCounter(IOBluetoothHostController *);
    virtual void  ResetHardResetCounter(IOBluetoothHostController *);
    virtual UInt8 GetHardResetCounter(IOBluetoothHostController *);

    virtual IOReturn USBHardReset();
    static IOReturn  USBHardResetAction(OSObject * owner, void * arg0, void * arg1, void * arg2, void * arg3);
    virtual void     USBHardResetWL();
#else
    virtual IOReturn PerformHardwareReset();
    static IOReturn  HardwareResetAction(OSObject * owner, void * arg0, void * arg1, void * arg2, void * arg3);
    virtual void     HardwareResetWL();
#endif
    virtual void     ReleaseAllHardResetObjects();
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    virtual bool     FindBroadcomUSBHostDevice();
    virtual bool     USBBluetoothModuleWithROMBootCapability();
    virtual IOReturn RecoverX238EModule(BluetoothHardwareListType *);
#else
    virtual bool     FindBroadcomUSBHostController();
#endif

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_11_0
    virtual void     RecoveryTimeOutHandler();
    virtual IOReturn StartRecoveryTimer();
    virtual void     CancelRecoveryTimer();
#endif

    virtual IOReturn DumpStats();
    virtual void     BeginSignPost();
    virtual void     EndSignPost();

    virtual void ConvertErrorCodeToString(UInt32 errorCode, char * outStringLong, char * outStringShort);
    virtual void ConvertOpCodeToString(UInt16 opCode, char * outString);
    virtual void ConvertEventCodeToString(UInt8 eventCode, char * outString);
    virtual void ConvertVendorSpecificEventCodeToString(UInt8 eventCode, char * outString);
    virtual void ConvertEventStatusToString(UInt8 eventStatus, char * outString);

    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 0);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 1);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 2);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 3);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 4);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 5);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 6);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 7);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 8);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 9);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 10);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 11);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 12);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 13);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 14);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 15);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 16);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 17);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 18);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 19);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 20);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 21);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 22);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 23);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 24);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 25);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 26);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 27);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 28);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 29);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 30);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 31);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 32);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 33);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 34);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 35);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 36);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 37);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 38);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 39);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 40);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 41);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 42);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 43);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 44);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 45);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 46);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 47);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 48);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 49);
    OSMetaClassDeclareReservedUnused(IOBluetoothHCIController, 50);

public:
    IOWorkLoop *    mWorkLoop;                // 136
    IOCommandGate * mCommandGate;             // 144
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    IOWorkLoop *    mUSBHardResetWorkLoop;    // 152
    IOCommandGate * mUSBHardResetCommandGate; // 160
#else
    OSSet * mPacketLoggerUserClientSet;       /// 152
#endif
    OSSet * mCommandUserClientSet;            // 168

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    IOBluetoothPacketLogger * mPacketLogger;                // 176
    IONotifier *              mPacketLoggerPublishNotifier; // 184
#else
    BluetoothPacketLogData * mPacketLogBuffer;             /// 168
    UInt32                   mPacketLogBufferSize;         /// 176
    OSArray *                mLogPackets;                  /// 184
    UInt32                   mMaxNumberOfPreStoredPackets; /// 192
    UInt32                   mLogPacketIndex;              /// 196
    UInt32                   mPacketLogFillDataIndex;      /// 200
    bool                     mNewPacketLogUserClient;      /// 204
    bool                     mPacketLogBufferEmptied;      /// 205
#endif

    IODisplayWrangler * mDisplayManager;                   // 192
    IONotifier *        mDisplayManagerPublishNotifier;    // 200
    IONotifier *        mExternalDisplayPublishNotifier;   // 208
    IONotifier *        mExternalDisplayTerminateNotifier; // 216

    bool                 mSearchForTransportEventTimerHasTimeout; // 224
    IOTimerEventSource * mSearchForTransportEventTimer;           // 232
    bool                 mFullWakeWithAppleExternalDisplay;       // 240
    bool                 mExternalDisplayPublished;               // 241

    HCIEventNotificationListener * mHCIEventListenersList;     // 248
    IOByteCount                    mHCIEventListenersListSize; // 256
    bool                           mDebugMode;                 // 264

    IOWorkQueue * mFamilyWorkQueue;             // 272
    IOWorkQueue * mExternalControllerWorkQueue; // 280
#if __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_15
    IOWorkQueue * mLogPacketWorkQueue;          /// 304
#endif

    BluetoothHardwareListType * mBluetoothHardwareListHead;         // 288
    BluetoothHardwareListType * mBluetoothHardwareListTail;         // 296
    BluetoothHardwareListType * mActiveBluetoothHardware;           // 304
    BluetoothHardwareListType * mActiveControllerBluetoothHardware; // 312
    BluetoothHardwareListType * mActiveTransportBluetoothHardware;  // 320

    IONotifier * mBluetoothTransportShowsUpNotifier;  // 328
    IONotifier * mBluetoothTransportGoesAwayNotifier; // 336
    bool         mReceivedTransportNotification;      // 344

    UInt16                 mActiveControllerProductID;         // 346
    UInt16                 mActiveControllerVendorID;          // 348
    BluetoothDeviceAddress mActiveControllerAddress;           // 350
    UInt32                 mActiveControllerLocationID;        // 356
    UInt16                 mActiveControllerActiveConnections; // 360
    bool                   mActiveControllerValid;             // 362

    UInt16                 mInternalControllerProductID;  // 364
    UInt16                 mInternalControllerVendorID;   // 366
    BluetoothDeviceAddress mInternalControllerAddress;    // 368
    UInt32                 mInternalControllerLocationID; // 376
    bool                   mInternalControllerValid;      // 380

    bool         mControllersFinishedSettingUp;           // 381
    bool         __reserved;                              // 382, never used
    bool         mSwitchingHostController;                // 383
    bool         mNVRAMControllerInfoUpdated;             // 384
    UInt32       mCheckACPIMethodsAvailabilitiesCallTime; // 388
    bool         mUserClientsAttached;                    // 392
    IOOptionBits mTransportPowerStateOptions;             // 396
    UInt16       mNumCommandSleepsInWorkLoop;             // 400, +1 when commandSleep is called in FamilyCommandSleep and -1 when it's done

    UInt32               mActivityTickleCallTime;  // 404
    IOTimerEventSource * mFullWakeTimer;           // 408
    bool                 mFullWakeTimerHasTimeout; // 416
    bool                 mFullWakeTimerStarted;    // 417
    bool                 mSignPostSupported;       // 418
    bool                 mSignPostStarted;         // 419

    os_log_t mInternalOSLogObject; // 424
    bool     mCanPowerOn;          // 432

#if __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_14
    IOACPIPlatformDevice * mACPIDevice;  /// 464
    bool                   SupportBTRS;  /// 472
    bool                   SupportBTRB;  /// 473
    bool                   mSupportBTPU; /// 474
    bool                   mSupportBTPD; /// 475
    bool                   mSupportBTLP; /// 476
#endif

    bool mBootFromROM;           // 433
    bool mBTGPIOResult;          // 434, BTRB result
    bool mRegisterServiceCalled; // 435

    thread_call_t mHardResetThreadCall;                    // 440
    UInt32        mUSBHardResetWLCallTime;                 // 448

    bool          mTestNotRespondingHardReset;    		   // 452
    bool          mTestHCICommandTimeoutHardReset;         // 453
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    bool          unknown1;                                // 454, see DispatchHardwareResetTest
    bool          mTestNoHardResetWhenSleepCommandTimeout; // 455
    bool          mTestHCICommandTimeoutWhenWake;          // 456
    bool          unknown2;                                // 457
#endif
    UInt8                                   mNumHardResetRetries;    // 458
    IOBluetoothHostControllerUSBTransport * mHardResetUSBTransport;  // 464
    IOUSBHostDevice *                       mHardResetUSBHostDevice; // 472
    IOUSBHostDevice *                       mHardResetUSBHub;        // 480

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    IOBluetoothACPIMethods *                mACPIMethods;            // 488
#endif

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_11_0
    IOTimerEventSource * mRecoveryTimer;           // 496
    bool                 mRecoveryTimerHasTimeout; // 504
#endif

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    UInt16   mCurrentBluetoothObjectID;     // 506
    bool     mBluetoothObjects[0xFFFF];     // 508
    IOLock * mBluetoothObjectsLock;         // 66048
    UInt8    mNumHCIEventListeners;         // 66056
#endif
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    bool     mNotTriedToRecoverX238EModule; // 66057
#endif

    struct ExpansionData
    {
        void * mUnusedPointer1;
        void * mUnusedPointer2;
    };
    ExpansionData * mExpansionData; // 66064
};

#endif
