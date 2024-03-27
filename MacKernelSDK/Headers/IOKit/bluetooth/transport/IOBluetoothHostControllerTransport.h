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
/*!
 *   @header
 *   This header contains the definition of the IOBluetoothHostControllerTransport class, a subclass of IOService that serves as the base class for all the various transports in the IOBluetoothFamily.
 *
 */

#ifndef _IOKIT_IOBLUETOOTHHOSTCONTROLLERTRANSPORT_H
#define _IOKIT_IOBLUETOOTHHOSTCONTROLLERTRANSPORT_H

#include <Availability.h>
#include <IOKit/IOCommandGate.h>
#include <IOKit/IOService.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/IOWorkLoop.h>
#include <IOKit/bluetooth/Bluetooth.h>
#include <IOKit/bluetooth/IOBluetoothACPIMethods.h>
#include <IOKit/bluetooth/IOBluetoothHCIController.h>
#include <IOKit/bluetooth/IOBluetoothHostController.h>

#ifndef __MAC_OS_X_VERSION_MIN_REQUIRED
#error "Missing macOS target version"
#endif

extern void BluetoothSleepTimeOutOccurred(OSObject * owner, IOTimerEventSource * sender);

static const char * gInternalPowerStateString[7]         = { "OFF", "ON", "SLEEP", "IDLE", "OFF", "IDLE", "ON" };
static const char * gOrdinalPowerStateString[3]          = { "OFF", "IDLE", "ON" };
static const char * gPowerManagerSleepTypeString[9]      = { "kIOPMSleepTypeInvalid", "kIOPMSleepTypeAbortedSleep", "kIOPMSleepTypeNormalSleep", "kIOPMSleepTypeSafeSleep", "kIOPMSleepTypeHibernate", "kIOPMSleepTypeStandby", "kIOPMSleepTypePowerOff", "kIOPMSleepTypeDeepIdle", "kIOPMSleepTypeLast" };
static const char * gPowerManagerSleepTypeShortString[9] = { "SleepTypeInvalid", "SleepTypeAbortedSleep", "SleepTypeNormalSleep", "SleepTypeSafeSleep", "SleepTypeHibernate", "SleepTypeStandby", "SleepTypePowerOff", "SleepTypeDeepIdle", "SleepTypeLast" };

extern const IORegistryPlane * gIODTPlane;

/*! @class IOBluetoothHostControllerTransport
 *   @abstract The base class for IOBluetoothFamily transports.
 *   @discussion ???
 *
 */

class IOBluetoothHostControllerTransport : public IOService
{
    OSDeclareAbstractStructors(IOBluetoothHostControllerTransport)

    friend class IOBluetoothHCIController;
    friend class IOBluetoothHostController;

public:
    /*! @function init
     *   @abstract Initializes member variables of IOBluetoothHostControllerTransport.
         @discussion This function calls CreateOSLogObject() to create mInternalOSLogObject, sets the initial value for various members of the class, calls GetNVRAMSettingForSwitchBehavior() to set up
     mSwitchBehavior, and creates the ExpansionData. It calls super::init() at the end to initialize its inherited members.
     *   @result Result of IOService::init(). */

    virtual bool init(OSDictionary * dictionary = NULL) APPLE_KEXT_OVERRIDE;

    /*! @function free
     *   @abstract Frees data structures that were allocated the class was initialized. */

    virtual void free() APPLE_KEXT_OVERRIDE;

    /*! @function probe
     *   @abstract Probes a matched device to see if it can be used.
     *   @discussion Aside from running probe() of the super class, this function would check the SkipIOBluetoothHostControllerUSBTransport property in the gIODTPlane. If it exists, the function will
     * terminate returning NULL as Bluetooth USB Transport should be skipped.
     *   @param provider The registered IOService object that matches a driver personality's matching dictionary.
     *   @param score The driver's probe score.
     *   @result If SkipIOBluetoothHostControllerUSBTransport does not exist, the result of IOService::probe() would be returned. Otherwise, the result would be NULL. */

    virtual IOService * probe(IOService * provider, SInt32 * score) APPLE_KEXT_OVERRIDE;

    virtual bool start(IOService * provider) APPLE_KEXT_OVERRIDE;
    virtual void stop(IOService * provider) APPLE_KEXT_OVERRIDE;

    virtual IOWorkLoop *    getWorkLoop() const APPLE_KEXT_OVERRIDE;
    virtual IOCommandGate * getCommandGate() const;
    virtual bool            setTransportWorkLoop(void *, IOWorkLoop * inWorkLoop);

    virtual bool    terminate(IOOptionBits options = 0) APPLE_KEXT_OVERRIDE;
    static IOReturn terminateAction(OSObject * owner, void * arg1, void * arg2, void * arg3, void * arg4);
    virtual bool    terminateWL(IOOptionBits options = 0);

    virtual bool    InitializeTransport();
    static IOReturn InitializeTransportAction(OSObject * owner, void * arg1, void * arg2, void * arg3, void * arg4);
    virtual bool    InitializeTransportWL(IOService * provider);

    virtual OSObject * getPropertyFromTransport(const OSSymbol * aKey);
    virtual OSObject * getPropertyFromTransport(const OSString * aKey);
    virtual OSObject * getPropertyFromTransport(const char * aKey);

    virtual IOReturn SetRemoteWakeUp(bool);
    virtual IOReturn DoDeviceReset(UInt16);

    virtual void AbortPipesAndClose(bool, bool);
    virtual bool HostSupportsSleepOnUSB();

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    virtual bool StartLMPLogging();
#endif
    virtual bool StartLMPLoggingBulkPipeRead();
    virtual bool StartInterruptPipeRead();
    virtual bool StopInterruptPipeRead();
    virtual bool StartBulkPipeRead();
    virtual bool StopBulkPipeRead();

    virtual IOReturn TransportBulkOutWrite(void *);
    virtual IOReturn TransportIsochOutWrite(void *, void *, IOOptionBits);
    virtual IOReturn TransportSendSCOData(void *);
    virtual IOReturn TransportLMPLoggingBulkOutWrite(UInt8, UInt8);

    virtual IOReturn SendHCIRequest(UInt8 * buffer, IOByteCount size);
    virtual void     UpdateSCOConnections(UInt8, UInt32);
    virtual IOReturn ToggleLMPLogging(UInt8 *);

    virtual IOReturn CallConfigPM();
    virtual bool     ConfigurePM(IOService * policyMaker);

    virtual unsigned long maxCapabilityForDomainState(IOPMPowerFlags domainState) APPLE_KEXT_OVERRIDE;
    virtual unsigned long initialPowerStateForDomainState(IOPMPowerFlags domainState) APPLE_KEXT_OVERRIDE;

    virtual IOReturn setPowerState(unsigned long powerStateOrdinal, IOService * whatDevice) APPLE_KEXT_OVERRIDE;
    static IOReturn  setPowerStateAction(OSObject * owner, void * arg1, void * arg2, void * arg3, void * arg4);
    virtual IOReturn setPowerStateWL(unsigned long powerStateOrdinal, IOService * whatDevice);

    virtual IOReturn RequestTransportPowerStateChange(IOBluetoothHCIControllerInternalPowerState powerState, char * name);
    virtual IOReturn WaitForControllerPowerState(IOBluetoothHCIControllerInternalPowerState powerState, char *);
    virtual IOReturn WaitForControllerPowerStateWithTimeout(IOBluetoothHCIControllerInternalPowerState powerState, uint32_t timeout, char *, bool);
    virtual void     CompletePowerStateChange(char *);
    virtual IOReturn ProcessPowerStateChangeAfterResumed(char *);

    virtual IOReturn setAggressiveness(unsigned long type, unsigned long newLevel) APPLE_KEXT_OVERRIDE;
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    static IOReturn  setAggressivenessAction(OSObject * owner, void * arg1, void * arg2, void * arg3, void * arg4);
    virtual bool     setAggressivenessWL(unsigned long type, unsigned long newLevel);
#endif

    virtual IOReturn powerStateWillChangeTo(IOPMPowerFlags capabilities, unsigned long stateNumber, IOService * whatDevice) APPLE_KEXT_OVERRIDE;
    static IOReturn  powerStateWillChangeToAction(OSObject * owner, void * arg1, void * arg2, void * arg3, void * arg4);
    virtual IOReturn powerStateWillChangeToWL(IOOptionBits options, void *);

    virtual void    systemWillShutdown(IOOptionBits specifier) APPLE_KEXT_OVERRIDE;
    static IOReturn systemWillShutdownAction(OSObject * owner, void * arg1, void * arg2, void * arg3, void * arg4);
    virtual void    systemWillShutdownWL(IOOptionBits options, void *);

    virtual IOBluetoothHCIControllerInternalPowerState GetCurrentPowerState();
    virtual IOBluetoothHCIControllerInternalPowerState GetPendingPowerState();

    virtual IOReturn ChangeTransportPowerStateFromIdleToOn(char * name);
    virtual IOReturn ChangeTransportPowerState(unsigned long ordinal, bool willWait, IOBluetoothHCIControllerInternalPowerState powerState, char * name);
    virtual IOReturn WaitForControllerPowerStateChange(IOBluetoothHCIControllerInternalPowerState, char *);

    virtual IOReturn WakeupSleepingPowerStateThread();
    virtual bool     ControllerSupportWoBT();
    virtual UInt16   GetControllerVendorID();
    virtual UInt16   GetControllerProductID();

    virtual UInt8 GetRadioPowerState();
    virtual void SetRadioPowerState(UInt8 powerState);

    virtual bool     GetNVRAMSettingForSwitchBehavior();
    virtual UInt32   GetControllerLocationID();
    virtual bool     GetBuiltIn();
    virtual bool     GetSupportPowerOff();
    virtual bool     IsHardwareInitialized();
    virtual UInt32   GetHardwareStatus();
    virtual void     ResetHardwareStatus();
    virtual UInt32   ConvertAddressToUInt32(void * address);
    virtual void     SetActiveController(bool);
    virtual void     ResetBluetoothDevice();
    virtual IOReturn TransportCommandSleep(void *, UInt32, char *, bool);
    virtual void     ReadyToGo(bool oneThread);
    virtual bool     TerminateCalled();
    virtual void     GetInfo(void * outInfo);
    virtual IOReturn CallPowerManagerChangePowerStateTo(unsigned long ordinal, char *);
    virtual UInt16   GetControllerTransportType();
    virtual bool     SupportNewIdlePolicy();

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    virtual void     CheckACPIMethodsAvailabilities();
#else
    virtual void     CheckSpecialGPIO();
#endif
    virtual IOReturn SetBTRS();
    virtual IOReturn SetBTPU();
    virtual IOReturn SetBTPD();
    virtual IOReturn SetBTRB(bool);
    virtual IOReturn SetBTLP(bool);

    virtual void NewSCOConnection();

    virtual void retain() const APPLE_KEXT_OVERRIDE;
    virtual void release() const APPLE_KEXT_OVERRIDE;
    virtual void RetainTransport(char *);
    virtual void ReleaseTransport(char *);

    virtual IOReturn SetIdlePolicyValue(uint32_t idleTimeoutMs);
    virtual bool     TransportWillReEnumerate();
    virtual void     ConvertPowerFlagsToString(IOPMPowerFlags, char *);
    virtual void     SetupTransportSCOParameters();
    virtual void     DestroyTransportSCOParameters();
    virtual bool     WaitForSystemReadyForSleep(char *);
    virtual IOReturn StartBluetoothSleepTimer();
    virtual void     CancelBluetoothSleepTimer();
#if __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_15
    virtual bool     StopAllReads();
#endif
    virtual os_log_t CreateOSLogObject();

    virtual IOReturn setProperties(OSObject * properties) APPLE_KEXT_OVERRIDE;
    static IOReturn  setPropertiesAction(OSObject * owner, void * arg1, void * arg2, void * arg3, void * arg4);
    virtual IOReturn setPropertiesWL(OSObject * properties);

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    virtual IOReturn HardReset();
#else
    virtual IOReturn PerformHardReset();
#endif

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_11_0
    virtual void DumpTransportProviderState();
#endif

    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 0);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 1);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 2);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 3);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 4);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 5);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 6);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 7);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 8);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 9);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 10);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 11);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 12);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 13);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 14);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 15);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 16);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 17);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 18);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 19);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 20);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 21);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 22);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostControllerTransport, 23);

public:
    IOBluetoothHCIController *  mBluetoothFamily;      // 136
    IOBluetoothHostController * mBluetoothController;  // 144
    IOService *                 mProvider;             // 152
    IOWorkLoop *                mWorkLoop;             // 160
    IOCommandGate *             mCommandGate;          // 168
#if __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_15
    IOWorkQueue *               mTransportWorkQueue;   ///176
#endif
    UInt16                      mControllerVendorType; // 176

    bool    mUSBControllerSupportsSuspend; // 178
    UInt8   mTerminateState;               // 179
    bool    mLMPLoggingEnabled;            // 180
    bool    mSystemOnTheWayToSleep;        // 181
    uint8_t reserved2;                     // 182
    uint8_t reserved3;                     // 183
    bool    mConfiguredPM;                 // 184
    UInt32  mSwitchBehavior;               // 188
    bool    mHardwareInitialized;          // 192
    UInt32  mHardwareStatus;               // 196
    UInt32  mTerminateCounter;             // 200
    bool    mLMPLoggingAvailable;          // 204
    void *  mRefCon;                       // 208

    IOBluetoothHCIControllerInternalPowerState mCurrentInternalPowerState; // 216
    IOBluetoothHCIControllerInternalPowerState mPendingInternalPowerState; // 220

    UInt16                   mConrollerTransportType;         // 224
    bool                     mSupportNewIdlePolicy;           // 226
    bool                     mBuiltIn;                        // 227
    UInt32                   mLocationID;                     // 228
    bool                     mSupportPowerOff;                // 232
    UInt32                   mSleepType;                      // 236
    bool                     mIsControllerActive;             // 240
    uint8_t                  reserved4;                       // 241
    UInt32                   unknown111;                      // 244
    UInt8                    mAlreadyOff;                     // 248
    UInt8                    reserved6;                       // 249
    bool                     mSupportWoBT;                    // 250
    UInt8                    mCurrentPMMethod;                // 251
    UInt64                   mTransportCounter;               // 256, retain/released in Retain/ReleaseTransport
    UInt16                   mTransportOutstandingCalls;      // 264
    bool                     reserved7;                       // 266
    IOTimerEventSource *     mBluetoothSleepTimerEventSource; // 272
    bool                     mBluetoothSleepTimerStarted;     // 280
    os_log_t                 mInternalOSLogObject;            // 288
    bool                     mBootFromROM;                    // 296
    UInt16                   mUARTProductID;                  // 298
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    IOBluetoothACPIMethods * mACPIMethods;                    // 304
#endif
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    bool                     reserved10;                      // 312
#endif

    struct ExpansionData
    {
        UInt16 reserved;
    };
    ExpansionData * mExpansionData; // 320
};

#endif
