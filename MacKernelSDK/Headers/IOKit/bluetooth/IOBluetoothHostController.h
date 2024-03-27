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

#ifndef _IOKIT_IOBLUETOOTHHOSTCONTROLLER_H
#define _IOKIT_IOBLUETOOTHHOSTCONTROLLER_H

#include <IOKit/IOReportTypes.h>
#include <IOKit/IOService.h>
#include <IOKit/bluetooth/Bluetooth.h>
#include <IOKit/bluetooth/IOBluetoothInternal.h>
#include <os/log.h>

#include <Availability.h>

#ifndef __MAC_OS_X_VERSION_MIN_REQUIRED
#error "Missing macOS target version"
#endif

class IOBluetoothHostControllerUserClient;
class IOBluetoothHostControllerTransport;
class IOBluetoothHCIController;
class IOBluetoothHCIRequest;
class IOWorkQueue;
class IOReportLegend;
class IOStateReporter;
class IOSimpleReporter;
class IOBluetoothDevice;
class IOBluetoothInactivityTimerEventSource;
class IOBluetoothACLMemoryDescriptor;
class IOBluetoothSCOMemoryDescriptorRetainer;

struct BluetoothDeviceReporter;
struct BluetoothBroadcomBFCReconnectData;
struct BluetoothBroadcomBFCParams;
struct BluetoothBroadcomBFCRemoteBPCSFeatures;
struct BluetoothBroadcomLocalFirmwareInfo;
struct BluetoothBroadcomSetEventMask;
struct BluetoothBroadcomBFCConnectionTBFCSuspendedInfo;
struct BluetoothBroadcomVerboseConfigVersionInfo;

enum
{
    kRadioPowerInitialState = 0,
    kRadioPoweringOff       = 1,
    kRadioPoweringOn        = 2,
    kRadioPoweredOff        = 3,
    kRadioPoweredOn         = 4,
    kRadioPowerError        = 5
};

typedef uint16_t DevicePublishNotificationStateType;
enum DevicePublishNotificationStateTypes
{
    kHasNotRegisteredForDevicePublishNotification = 0x0001,
    kHasRegisteredForDevicePublishNotification    = 0x0002,
    kDevicePublishNotificationCalled              = 0x0004,
    kDevicePublishNotificationProcessed           = 0x0008
};

typedef UInt32 HCIDataHandlerType;
enum HCIDataHandlerTypes
{
    kTransportDataTypeHCIEvents,
    kTransportDataTypeACL,
    kTransportDataTypeSCO
};

typedef struct HearingDeviceListType
{
    BluetoothDeviceAddress             mDeviceAddress;
    bool                               mRemoveDeviceCalled;
    DevicePublishNotificationStateType mDevicePublishNotificationState;
    HearingDeviceListType *            mNextDevice;
    HearingDeviceListType *            mPreviousDevice;
} HearingDeviceListType;

typedef struct LEDeviceListType
{
    BluetoothConnectionHandle mConnectionHandle;
    bool                      unknown;
    LEDeviceListType *        mNextDevice;
    LEDeviceListType *        mPreviousDevice;
} BluetoothLEDevice;

struct BluetoothHCIACLPacket
{
    IOMemoryDescriptor *    descriptor;
    IOBluetoothDevice *     device;
    BluetoothHCIACLPacket * nextPacket;
};

IOBLUETOOTH_EXPORT long PackData( void * inBuffer, UInt32 inDataSize, const char * inFormat, ... );
IOBLUETOOTH_EXPORT long PackDataList( void * inBuffer, UInt32 inDataSize, const char * inFormat, va_list inArgs );
IOBLUETOOTH_EXPORT long UnpackData( IOByteCount inBufferSize, const void * inBuffer, const char * inFormat, ... );
IOBLUETOOTH_EXPORT long UnpackDataList( IOByteCount inBufferSize, const void * inBuffer, const char * inFormat, va_list inArgs );

class IOBluetoothHostController : public IOService
{
	OSDeclareDefaultStructors(IOBluetoothHostController)

    typedef IOReturn (*IOBluetoothIncomingDataAction)(IOBluetoothHostController * hostController, UInt8 * inDataPtr, UInt32 inDataSize, UInt32 inSequenceNumber);

    friend class IOBluetoothHCIRequest;
    friend class IOBluetoothHCIController;
    friend class IOBluetoothHostControllerTransport;
    friend class IOBluetoothHostControllerUSBTransport;
    friend class IOBluetoothHostControllerUARTTransport;

public:
    virtual IOReturn newUserClient(task_t owningTask, void * securityID, UInt32 type, IOUserClient ** handler) APPLE_KEXT_OVERRIDE;
    virtual IOReturn AddUserClient(void *);
    virtual void     DetachUserClients();

    virtual bool InitializeVariables(IOBluetoothHostControllerTransport * transport);
    virtual bool init(IOBluetoothHCIController * family, IOBluetoothHostControllerTransport * transport);
    virtual void free() APPLE_KEXT_OVERRIDE;

    virtual bool SetVariables();
    virtual void Enable(IOBluetoothHostControllerTransport * transport);
    virtual void Disable();
    virtual void StopAndFreeVariables();

    virtual bool     start(IOService * provider) APPLE_KEXT_OVERRIDE;
    virtual void     stop(IOService * provider) APPLE_KEXT_OVERRIDE;
    static IOReturn  stopAction(OSObject * owner, void * arg1, void * arg2, void * arg3, void * arg4);
    virtual IOReturn stopWL(IOService * provider);

    virtual bool     terminate(IOOptionBits options = 0) APPLE_KEXT_OVERRIDE;
    static IOReturn  terminateAction(OSObject * owner, void * arg1, void * arg2, void * arg3, void * arg4);
    virtual IOReturn terminateWL(IOOptionBits options = 0);

    virtual IOReturn setProperties(OSObject * properties) APPLE_KEXT_OVERRIDE;
    static IOReturn  setPropertiesAction(OSObject * owner, void * arg1, void * arg2, void * arg3, void * arg4);
    virtual IOReturn setPropertiesWL(OSObject * properties);

    virtual bool setProperty(const OSSymbol * aKey, OSObject * anObject) APPLE_KEXT_OVERRIDE;
    virtual bool setProperty(const OSString * aKey, OSObject * anObject) APPLE_KEXT_OVERRIDE;
    virtual bool setProperty(const char * aKey, OSObject * anObject) APPLE_KEXT_OVERRIDE;
    virtual bool setProperty(const char * aKey, const char * aString) APPLE_KEXT_OVERRIDE;
    virtual bool setProperty(const char * aKey, bool aBoolean) APPLE_KEXT_OVERRIDE;
    virtual bool setProperty(const char * aKey, unsigned long long aValue, unsigned int aNumberOfBits) APPLE_KEXT_OVERRIDE;
    virtual bool setProperty(const char * aKey, void * bytes, unsigned int length) APPLE_KEXT_OVERRIDE;

    virtual bool SetBluetoothControllerProperty(const OSSymbol * aKey, OSObject * anObject);
    virtual bool SetBluetoothControllerProperty(const OSString * aKey, OSObject * anObject);
    virtual bool SetBluetoothControllerProperty(const char * aKey, OSObject * anObject);
    virtual bool SetBluetoothControllerProperty(const char * aKey, const char * aString);
    virtual bool SetBluetoothControllerProperty(const char * aKey, bool aBoolean);
    virtual bool SetBluetoothControllerProperty(const char * aKey, unsigned long long aValue, unsigned int aNumberOfBits);
    virtual bool SetBluetoothControllerProperty(const char * aKey, void * bytes, unsigned int length);

    virtual IOWorkLoop *    getWorkLoop() const APPLE_KEXT_OVERRIDE;
    virtual IOCommandGate * getCommandGate() const;

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    virtual void BroadcastEventNotification(BluetoothHCIRequestID inID, BluetoothHCIEventCode inEventCode, IOReturn eventStatus, UInt8 * inDataToSendPtr, IOByteCount inDataSize, BluetoothHCICommandOpCode inOpCode, bool, UInt8);
    virtual void BroadcastNotification(UInt32 type, IOBluetoothHCIControllerConfigState oldState, IOBluetoothHCIControllerConfigState newState);
#else
    virtual void BroadcastEventNotification(BluetoothHCIRequestID inID, BluetoothHCIEventCode inEventCode, IOReturn eventStatus, UInt8 * inDataToSendPtr, IOByteCount inDataSize, BluetoothHCICommandOpCode inOpCode);
    virtual void BroadcastConfigStateChangeNotification(IOBluetoothHCIControllerConfigState oldState, IOBluetoothHCIControllerConfigState newState);
    virtual void BroadcastWoBTReconnectionNotification();
    virtual void BroadcastCommandTimeoutNotification();
    virtual void BroadcastUSBPipeStallNotification();
#endif

    virtual void WaitForBluetoothd();
    virtual void FoundBluetoothd();

    virtual IOBluetoothHCIControllerFeatureFlags GetControllerFeatureFlags();
    virtual bool                                 SetControllerFeatureFlags(IOBluetoothHCIControllerFeatureFlags featureFlags);
    virtual void                                 setConfigState(IOBluetoothHCIControllerConfigState configState);

    virtual bool     InitializeController();
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    virtual IOReturn SetupController(bool *);
#else
    virtual IOReturn SetupController();
#endif
#if __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_15
    virtual IOReturn CallSetupController();
#endif
    virtual IOReturn SetupGeneralController();
    virtual IOReturn SetupCommonHardware();
    virtual bool     ControllerSetupIsComplete();
    virtual void     ControllerSetupComplete(int);
    virtual void     BluetoothHostControllerSetupCompleted();
    virtual bool     InitializeHostControllerVariables(bool);
    virtual IOReturn CallInitializeHostControllerVariables();
    virtual void     CompleteInitializeHostControllerVariables();

    virtual IOReturn configureReport(IOReportChannelList * channels, IOReportConfigureAction action, void * result, void * destination) APPLE_KEXT_OVERRIDE;
    virtual IOReturn updateReport(IOReportChannelList * channels, IOReportConfigureAction action, void * result, void * destination) APPLE_KEXT_OVERRIDE;

    virtual bool                      CreatePowerReporters();
    virtual IOReturn                  UpdatePowerReports(IOBluetoothHCIControllerInternalPowerState powerState);
    virtual IOReturn                  CallCreateDeviceReporter(IOBluetoothDevice * device);
    static IOReturn                   CreateDeviceReporterAction(OSObject * owner, void * arg1, void * arg2, void * arg3, void * arg4, void * arg5, void * arg6);
    virtual IOReturn                  CreateDeviceReporter(IOBluetoothDevice * device);
    virtual BluetoothDeviceReporter * FindDeviceReporter(const BluetoothDeviceAddress * address);
    virtual IOReturn                  ConvertClassOfDeviceToDeviceType(UInt32, bool, UInt16 *, char *, UInt64 *, char *, UInt8 *);
    virtual IOReturn                  AddDeviceReporter(BluetoothDeviceReporter * reporter);
    virtual IOReturn                  RemoveDeviceReporter(IOBluetoothDevice * device);
    virtual IOReturn                  UpdateDeviceReporter(IOBluetoothDevice * device, bool);
    virtual IOReturn                  GetDeviceCounterForName(UInt16, char *, char *, UInt8 *);
    virtual void                      PrintDeviceReporterList();
    virtual void                      UpdateLESetAdvertisingDataReporter(IOBluetoothHCIRequest * request);
    virtual IOReturn                  SetupPowerStateReporter();

    virtual void DesyncIncomingData(IOBluetoothIncomingDataAction action, UInt8 * inDataPtr, UInt32 inDataSize);
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_11_0
    static void  DesyncIncomingDataAction(IOBluetoothHostController * hostController, void * action, void * inDataPtr, UInt32 inDataSize, UInt32 sequenceNumber);
#else
    static void  DesyncIncomingDataAction(IOBluetoothHostController * hostController, IOBluetoothIncomingDataAction action, void * inDataPtr, UInt32 inDataSize, UInt32 sequenceNumber);
#endif
    virtual void SynchronizePacketSequence(UInt32 sequenceNumber);
    virtual void SynchronizeSCOPacketSequence(UInt32 sequenceNumber);

    virtual IOBluetoothDevice * FindDeviceWithHandle(BluetoothConnectionHandle inConnectionHandle);
    virtual IOBluetoothDevice * FindDeviceWithSCOHandle(BluetoothConnectionHandle inConnectionHandle);
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    virtual IOBluetoothDevice * FindDeviceWithAddress(const BluetoothDeviceAddress * inDeviceAddress, bool all);
#else
    virtual IOBluetoothDevice * FindDeviceWithAddress(const BluetoothDeviceAddress * inDeviceAddress);
#endif

    virtual HearingDeviceListType * FindHearingDeviceWithAddress(const BluetoothDeviceAddress * inDeviceAddress);
    virtual IOReturn                AddHearingDevice(IOBluetoothDevice * inDevice);
    virtual IOReturn                RemoveHearingDevice(IOBluetoothDevice * inDevice, bool all);

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    virtual LEDeviceListType * FindLEDeviceWithConnectionHandle(BluetoothConnectionHandle inConnectionHandle);
    virtual IOReturn           AddLEDevice(BluetoothConnectionHandle inConnectionHandle);
    virtual IOReturn           RemoveLEDevice(BluetoothConnectionHandle inConnectionHandle, bool all);
#endif

    virtual IOReturn AddDevice(IOBluetoothDevice * inDevice);
    virtual IOReturn RemoveDevice(IOBluetoothDevice * inDevice);
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    virtual void     PrintDeviceList();
#endif

    virtual IOReturn SetDevicePublishNotificationState(const BluetoothDeviceAddress *, UInt16);
    virtual UInt16   GetDevicePublishNotificationState(const BluetoothDeviceAddress *);
    virtual IOReturn CreateDeviceFromLEConnectionResults(BluetoothHCIEventLEConnectionCompleteResults * connectionResults);
    virtual IOReturn CreateDeviceFromConnectionResults(BluetoothHCIEventConnectionCompleteResults * connectionResults);
    virtual IOReturn CreateDeviceFromConnectionResults(BluetoothHCIEventConnectionCompleteResults * connectionResults, bool isInitiator);
    virtual IOReturn DestroyDeviceWithDisconnectionResults(BluetoothHCIEventDisconnectionCompleteResults * disconnectionResults);
    virtual IOReturn DestroyDevice(IOBluetoothDevice * inDevice);
    virtual IOReturn DestroyAllDevices();

    virtual bool terminateServiceNubs();
    virtual bool terminateServiceNubsComplete();

    virtual void FlushDeviceACLPackets(IOBluetoothDevice * inDevice);
    virtual void SaveNumOutstandingACLPackets();
    virtual void DecrementIdleTimerActivityCount(UInt16);
    virtual void IncrementIdleTimerActivityCount();
    virtual void DecrementOutstandingACLPackets(UInt16, UInt16, UInt16, UInt16, UInt16, UInt16);

    virtual void ProcessFlushOccurredEvent(BluetoothHCIEventFlushOccurredResults * inFlushResults);
    virtual void ProcessNumberOfCompletedPacketsEvent(UInt8 *);
    virtual void ProcessHCIControllerResetEvent();

    virtual IOReturn            DispatchIncomingACLData(UInt8 * inDataPtr, UInt32 inDataSize);
    virtual IOReturn            DispatchIncomingSCOData(UInt8 * inDataPtr, UInt32 inDataSize, UInt32 inMissingData, AbsoluteTime inTimestamp);
    virtual IOBluetoothDevice * OpenDeviceConnection(const BluetoothDeviceAddress * inDeviceAddress);
    static IOReturn             OpenDeviceConnectionAction(OSObject * owner, void * arg1, void * arg2, void * arg3, void * arg4);
    virtual IOBluetoothDevice * OpenDeviceConnectionWL(const BluetoothDeviceAddress * inDeviceAddress, UInt16, bool);
    virtual IOReturn            SendACLData(IOMemoryDescriptor * memDescriptor);
    static void                 ACLPacketTimerFired(OSObject * owner, IOTimerEventSource * timerEventSource);
    virtual IOReturn            SendACLPacket(IOBluetoothACLMemoryDescriptor * memDescriptor, IOBluetoothDevice * inTargetDevice = NULL);
    virtual IOReturn            TransferACLPacketToHW(IOMemoryDescriptor * memDescriptor);
    virtual void                handleACLPacketTimeout(IOBluetoothACLMemoryDescriptor *);
    virtual IOReturn            setUnackQueueCompletionCalled(void *);
    static IOReturn             setUnackQueueCompletionCalledAction(OSObject * onwer, void * arg1, void * arg2, void * arg3, void * arg4, void * arg5, void * arg6);

    virtual IOReturn SetACLPacketCompletion(void *);
    virtual IOReturn EnqueuePacket(IOMemoryDescriptor * memDescriptor, IOBluetoothDevice * inTargetDevice);
    virtual IOReturn DequeuePacket(UInt16, UInt16, UInt16, UInt16, UInt16, UInt16);
    virtual IOReturn RemovePacket(IOBluetoothACLMemoryDescriptor *);
    virtual UInt32   RemoveAllPacketsBelongingTo(IOBluetoothDevice *); // returns number of packets removed
    virtual IOReturn AdjustACLQueuesLimits(IOBluetoothDevice *);
    virtual IOReturn AdjustACLQueuesAllowance(IOBluetoothDevice *, bool);
    static IOReturn  AdjustACLQueuesAllowanceAction(OSObject * owner, void * arg1, void * arg2, void * arg3, void * arg4, void * arg5, void * arg6);
    virtual char *   CreateACLQueueDebugString(char *, UInt16);
    virtual void     SendSCOCompleted(IOBluetoothSCOMemoryDescriptorRetainer *, AbsoluteTime timestamp);
    static IOReturn  SendSCOCompletedAction(OSObject * owner, void * arg1, void * arg2, void * arg3, void * arg4, void * arg5, void * arg6);
    virtual IOReturn SendSCOData(IOMemoryDescriptor *, IOBluetoothSCOMemoryDescriptorRetainer *, int);
    virtual IOReturn SendSCOData(IOMemoryDescriptor *);
    static void      SCOPacketTimerFired(OSObject * owner, IOTimerEventSource * timerEventSource);
    virtual void     handleSCOPacketTimeout(IOBluetoothSCOMemoryDescriptorRetainer *);

    virtual void     SetNumSCOConnections(UInt8, UInt32);
    virtual IOReturn ToggleSCOConnection();
    virtual IOReturn ToggleeSCOEV3Connection();
    virtual IOReturn ToggleeSCOEV4Connection();
    virtual IOReturn ToggleeSCOEV5Connection();

    virtual UInt16 getSynchronousConnectionPacketTypeProperty();

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    virtual bool    NeedToSetupController();
    static IOReturn NeedToSetupControllerAction(IOBluetoothHostController * target);
    virtual bool    FullWake(char * calledByFunction);
#endif

    virtual void     ProcessLMPData(UInt8 * data, UInt32 dataSize);
    static IOReturn  ProcessLMPDataAction(IOBluetoothHostController * hostController, UInt8 * data, UInt32 dataSize);
    virtual void     ProcessLMPDataWL(UInt8 * data, UInt32 dataSize);

    virtual void    ProcessACLData(UInt8 * data, UInt32 dataSize);
    static IOReturn ProcessACLDataAction(IOBluetoothHostController * hostController, UInt8 * incomingDataPtr, UInt32 inDataSize, UInt32 inSequenceNumber);
    virtual void    ProcessACLDataWL(UInt8 * data, UInt32 dataSize, UInt32 sequenceNumber);

    virtual void    ProcessSCOData(UInt8 * data, UInt32 dataSize, UInt32 inMissingData, AbsoluteTime inTimestamp, Boolean copyData);
    static IOReturn ProcessSCODataAction(IOBluetoothHostController * hostController, UInt8 * incomingDataPtr, UInt32 inDataSize, UInt32 inMissingData, AbsoluteTime * inTimestamp, Boolean inCopyData);
    virtual void    ProcessSCODataWL(UInt8 * data, UInt32 dataSize, UInt32 inMissingData, AbsoluteTime inTimestamp, Boolean copyData);

    virtual void    ProcessEventData(UInt8 * inDataPtr, UInt32 inDataSize);
    static IOReturn ProcessEventDataAction(IOBluetoothHostController * hostController, UInt8 * incomingDataPtr, UInt32 inDataSize, UInt32 inSequenceNumber);
    virtual void    ProcessEventDataWL(UInt8 * inDataPtr, UInt32 inDataSize, UInt32 sequenceNumber);
    virtual void    NotifyDataClientsForEventData(IOBluetoothHCIRequest * hciRequest, UInt32, UInt8 *);

    virtual bool     GetCompleteCodeForCommand(BluetoothHCICommandOpCode inOpCode, BluetoothHCIEventCode * outEventCode);
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    virtual IOReturn GetOpCodeAndEventCode(UInt8 * inDataPtr, UInt32 inDataSize, BluetoothHCICommandOpCode * outOpCode, BluetoothHCIEventCode * eventCode, BluetoothHCIEventStatus * outStatus, UInt8 *, BluetoothDeviceAddress * outDeviceAddress, BluetoothConnectionHandle * outConnectionHandle, bool *);
#else
    virtual IOReturn GetOpCodeAndEventCode(UInt8 * inDataPtr, BluetoothHCICommandOpCode * outOpCode, BluetoothHCIEventCode * eventCode, BluetoothHCIEventStatus * outStatus, UInt8 *, BluetoothDeviceAddress * outDeviceAddress, BluetoothConnectionHandle * outConnectionHandle);
#endif

    virtual IOReturn FindConnectionCompleteType(BluetoothDeviceAddress * inDeviceAddress, BluetoothHCICommandOpCode * outOpCode);
    virtual IOReturn FindSynchronousConnectionCompleteType(BluetoothDeviceAddress * inDeviceAddress, BluetoothHCICommandOpCode * outOpCode);

    virtual IOReturn HandleSpecialOpcodes(BluetoothHCICommandOpCode opCode);
    virtual IOReturn HCIRequestCreate(BluetoothHCIRequestID * outRequestID, bool inDoAsyncNotify = false, UInt32 inTimeout = 5000, BluetoothHCIRequestCallbackInfo * inCallbackInfo = NULL, task_t inTaskID = NULL, UInt32 inControlFlags = 0);
    virtual IOReturn HCIRequestDelete(task_t inTask, BluetoothHCIRequestID inID);
    virtual IOReturn LookupRequest(BluetoothHCIRequestID inID, IOBluetoothHCIRequest ** outRequestPtr);
    virtual IOReturn PrepareRequestForNewCommand(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inDeviceAddress, BluetoothConnectionHandle inConnectionHandle);
    virtual IOReturn DisposeRequestsForTaskID(task_t inTaskID);
    virtual IOReturn CleanUpOrphanedHCIRequest();
    virtual void     CleanUpExpiredHCIRequest();
    virtual bool     HCIRequestQueuesAreEmpty();
    virtual void     RemoveAllHCIRequestBelongsToThisDevice(IOBluetoothDevice * thisDevice);
    virtual bool     HCIRequestIsSynchronous(UInt32, bool *);

    virtual bool SetHCIRequestRequireEvents(UInt16, IOBluetoothHCIRequest * request);
    virtual bool SetHCIRequestReceivedEvents(UInt8, IOBluetoothHCIRequest * request);
    virtual void ValidateHCIRequestReceivedEvents(UInt8, IOBluetoothHCIRequest * request);

    virtual IOReturn SendHCIRequestFormatted(BluetoothHCIRequestID inID, BluetoothHCICommandOpCode inOpCode, IOByteCount outResultsSize, void * outResultsPtr, const char * inFormat, ...);
    virtual IOReturn SendRawHCICommand(BluetoothHCIRequestID inID, char * buffer, IOByteCount bufferSize, UInt8 * outResultsPtr, IOByteCount outResultsSize);
    virtual IOReturn ProcessWaitingRequests(bool);
    virtual IOReturn SendingRequest(IOBluetoothHCIRequest * requestPtr);
    virtual IOReturn SendHCIRequestToTransport(UInt8 *, IOByteCount);

    virtual IOReturn EnqueueRequestForController(IOBluetoothHCIRequest * requestPtr);
    virtual IOReturn EnqueueRequest(IOBluetoothHCIRequest * inRequestPtr);
    virtual IOReturn DequeueRequest(IOBluetoothHCIRequest * inRequestPtr);
    virtual IOReturn FindQueuedRequest(BluetoothHCICommandOpCode opCode, BluetoothDeviceAddress * inDeviceAddress, BluetoothConnectionHandle inConnectionHandle, Boolean inUseAttributes,
                                       IOBluetoothHCIRequest ** outRequestPtr);
    virtual IOReturn EnqueueWaitRequest(IOBluetoothHCIRequest * inRequestPtr);
    virtual IOReturn DequeueWaitRequest(IOBluetoothHCIRequest * inRequestPtr);
    virtual IOReturn FindQueuedWaitRequest(BluetoothHCICommandOpCode opCode, IOBluetoothHCIRequest ** outRequestPtr);
    virtual IOReturn AbortRequest(IOBluetoothHCIRequest * inRequestPtr);
    virtual IOReturn AbortRequestAndSetTime(IOBluetoothHCIRequest *);
    virtual IOReturn KillAllPendingRequests(bool destroy, bool includeIdleRequests);
    virtual IOReturn CallKillAllPendingRequests(bool destroy, bool includeIdleRequests);

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_11_0
    virtual IOReturn IncrementHCICommandTimeOutCounter(UInt16 inOpCode);
#else
    virtual IOReturn IncrementHCICommandTimeOutCounter();
#endif
    virtual void     ResetHCICommandTimeOutCounter();

    virtual void IncrementActiveConnections();
    virtual void DecrementActiveConnections();
    virtual void ActiveConnectionsInProgress();
    virtual void AllConnectionsGone();
    virtual void MergeChannelDescription(OSDictionary * destination, OSDictionary * source);
    virtual void RemoveChannelRestrictions(OSDictionary * toChange, OSDictionary * removeRule, const char * key);
    virtual bool IsAllowedDevice(OSDictionary * description, IOBluetoothDevice * device);
    virtual void RemoveAllRules();

    virtual OSDictionary * GetIncomingL2CAPChannelDescription(OSNumber * psmNumber);
    virtual IOReturn       AddAllowedIncomingL2CAPChannel(OSDictionary * channelDescription);
    virtual void           RemoveAllowedIncomingL2CAPChannel(OSObject * channelID);

    virtual void AddAllowedIncomingL2CAPChannel(BluetoothL2CAPPSM incomingPSM);
    virtual void AddAllowedIncomingL2CAPChannel(OSNumber * psmNumber);
    virtual void RemoveAllowedIncomingL2CAPChannel(BluetoothL2CAPPSM incomingPSM);

    virtual OSDictionary * GetIncomingRFCOMMChannelDescription(OSNumber * channelIDNumber);
    virtual IOReturn       AddAllowedIncomingRFCOMMChannel(OSDictionary * channelDescription);
    virtual void           AddAllowedIncomingRFCOMMChannel(BluetoothRFCOMMChannelID incomingChannelID);
    virtual void           AddAllowedIncomingRFCOMMChannel(OSNumber * channelIDNumber);
    virtual void           AddSecureIncomingRFCOMMChannel(BluetoothRFCOMMChannelID incomingChannelID);
    virtual void           AddSecureIncomingRFCOMMChannel(OSNumber * channelIDNumber);
    virtual void           RemoveAllowedIncomingRFCOMMChannel(OSObject * channelID);
    virtual void           RemoveAllowedIncomingRFCOMMChannel(BluetoothRFCOMMChannelID incomingChannelID);
    virtual bool           IsSecureIncomingRFCOMMChannel(BluetoothRFCOMMChannelID incomingChannelID);
#if __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_15
    virtual bool           IsSecureIncomingRFCOMMChannel(OSNumber * incomingChannelID);
    virtual void           SetEnabledIncomingRFCOMMChannel(OSNumber * incomingChannelID, bool ShouldBeEnabled);
#endif
    virtual void           SetEnabledIncomingRFCOMMChannel(BluetoothRFCOMMChannelID incomingChannelID, bool ShouldBeEnabled);

    virtual bool IsAllowedIncomingL2CAPChannelForDevice(BluetoothL2CAPPSM incomingPSM, IOBluetoothDevice * device);
    virtual bool IsAllowedIncomingRFCOMMChannelForDevice(BluetoothRFCOMMChannelID incomingChannelID, IOBluetoothDevice * device);

    static void      idleTimerFired(OSObject * owner, IOTimerEventSource * timerEventSource);
    virtual void     StartIdleTimer(uint32_t milliseconds);
    virtual void     StopIdleTimer();
    virtual void     EvaluateHighPriorityConnections();
    virtual void     handleIdleTimeout();
    virtual IOReturn CallCancelTimeoutForIdleTimer();
    virtual IOReturn CallUpdateTimerForIdleTimer();
    virtual IOReturn CallResetTimerForIdleTimer();
    virtual IOReturn EnableIdleTimer();
    virtual IOReturn DisableIdleTimer();
    virtual void     ChangeIdleTimerTime(char * calledByFunction, UInt32 newTime);
    virtual IOReturn SetIdleTimerValue(UInt32 value);
    virtual IOReturn SetIdleTimerValueInNVRAM(UInt32 value);
    virtual IOReturn ClearIdleTimerValueInNVRAM();
    virtual void     WriteIdleTimerValueToNVRAM();
    virtual void     ReadIdleTimerValueFromNVRAM();
    virtual void     RemoveIdleTimerValueInNVRAM();

    virtual bool SetTransportIdlePolicyValue();
    virtual void WaitForAllDevicesToGetReadyToBeDestroyed();

    virtual bool                                       UpdatePowerStateProperty(IOBluetoothHCIControllerInternalPowerState powerState, bool);
    virtual IOReturn                                   RequestPowerStateChange(IOBluetoothHCIControllerInternalPowerState newPowerState, char *);
    virtual IOReturn                                   RequestPowerStateChangeFromUserSpace(IOBluetoothHCIControllerInternalPowerState newPowerState);
    virtual bool                                       Power_State_Change_In_Progress();
    virtual bool                                       Power_State_Is_Off();
    virtual bool                                       Is_Powering_Down();
    virtual IOBluetoothHCIControllerInternalPowerState GetControllerPowerState();
    virtual IOReturn                                   GetTransportCurrentPowerState(IOBluetoothHCIControllerInternalPowerState * outPowerState);
    virtual IOReturn                                   GetTransportPendingPowerState(IOBluetoothHCIControllerInternalPowerState * outPowerState);
    virtual IOReturn                                   ChangeTransportPowerStateTo(unsigned long ordinal, bool willWait, IOBluetoothHCIControllerInternalPowerState powerState, char * name);
    virtual IOReturn                                   WaitForTransportPowerStateChange(IOBluetoothHCIControllerInternalPowerState, char *);
    virtual bool                                       SystemReadyForSleep();
    virtual void                                       SetChangingPowerState(bool);
    virtual void                                       SetControllerPowerOptions(IOBluetoothHCIControllerPowerOptions inPowerOptions);
    virtual IOBluetoothHCIControllerPowerOptions       GetControllerPowerOptions();
    virtual void                                       TransportIsReady(bool);
    virtual void                                       TransportIsGoingAway();
    virtual IOReturn                                   ChangeControllerStateForRestart();
    virtual IOReturn                                   CleanUpForPoweringOff();
    virtual IOReturn                                   CleanUpBeforeTransportTerminate(IOBluetoothHostControllerTransport * transport);
    virtual void                                       SetVendorSpecificEventMask(bool);
    virtual IOReturn                                   CleanUpForCompletePowerChangeFromSleepToOn();
    virtual IOReturn                                   CleanupForPowerChangeFromOnToSleep(bool, UInt32 *);

    virtual IOReturn PerformTaskForPowerManagementCalls(IOOptionBits options);
    virtual IOReturn disableHIDEmulation();
    virtual IOReturn SetHIDEmulation(UInt32, bool);
    virtual IOReturn CallSetHIDEmulation();
    virtual bool     ModuleIsInUHEMode(bool *);
    virtual IOReturn TransportRadioPowerOff(BluetoothHCICommandOpCode opCode, char * processName, int processID, IOBluetoothHCIRequest * request);
    virtual IOReturn GetTransportRadioPowerState(UInt8 *);
    virtual IOReturn SetTransportRadioPowerState(UInt8);
    virtual IOReturn CallPowerRadio(bool);
    virtual IOReturn SendBFCSetParams(UInt8, UInt8, UInt8);
    virtual bool     SetMaxPowerForConnection(UInt32, UInt8, UInt16);
    virtual IOReturn WriteDeviceAddress(BluetoothHCIRequestID inID, BluetoothDeviceAddress * inAddress);
    virtual IOReturn WriteLocalSupportedFeatures(UInt32, void *);
    virtual IOReturn WriteBufferSize();
    virtual IOReturn DisableScan();
#if __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_15
    virtual IOReturn EnableScan();
#endif
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_11_0
    virtual IOReturn CallBluetoothHCIReset(bool, char *);
#else
    virtual IOReturn CallBluetoothHCIReset(bool);
#endif

    virtual IOReturn IgnoreUSBReset(bool);

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    virtual IOReturn HardResetController(UInt16);
#else
    virtual IOReturn BluetoothResetDevice(UInt16);
    virtual IOReturn BluetoothResetController();
    virtual IOReturn BluetoothResetHub();
    virtual IOReturn DoReset();
#endif
    virtual bool     WillResetModule();
#if __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_15
    virtual void     ResetHardResetCounter();
#endif
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_11_0
    static IOReturn  HardResetControllerAction(OSObject * target, UInt16);
    virtual IOReturn EnqueueHardResetControllerAction(UInt16);
#endif
    virtual IOReturn ResetTransportHardwareStatus();
    virtual IOReturn GetTransportHardwareStatus(UInt32 *);
    virtual void     TransportTerminating(IOBluetoothHostControllerTransport *);
#if __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_15
    virtual IOReturn ForceHardwareReset();
#endif
    virtual IOReturn GetTransportInfo(BluetoothTransportInfo *);
    virtual bool     SupportWoBT();
    virtual bool     IsTBFCSupported();
    virtual bool     IsAnyDevicesTBFCPageCapable();
    virtual bool     SupportDeepIdle();
    virtual IOReturn SetIdlePolicy(bool);
    virtual bool     BluetoothRemoteWakeEnabled();
    virtual bool     FileVaultIsTurnedOn();
    virtual IOReturn ControllerCommandSleep(void *, UInt32, char *, bool);

    virtual UInt32   ConvertAddressToUInt32(void * inAddress);
    virtual IOReturn UpdateFirmware(UInt32);
    virtual bool     NeedToIncreaseInactivityTime();
    virtual void     DisableControllerWorkQueue();
    virtual void     DoNotTickleDisplay();
    virtual IOReturn ValidSleepTypeFromPowerManager(UInt32 *);
    virtual IOReturn ToggleLMPLogging();
    virtual IOReturn SetupLighthouse();
    virtual IOReturn SetLighthouseControl(UInt8);
    virtual IOReturn SetLighthouseParameters(UInt16, UInt16, UInt16, UInt8, UInt8, SInt8, UInt8);
    virtual IOReturn SetLighthouseDebugQuery(UInt8);
    virtual void     WakeUpLEConnectionCompleteOutOfSequenceThread();
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    virtual IOReturn RecoverX238EModule();
#endif
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_11_0
    virtual void     SetControllerSleepMode();
#endif

    virtual void                  TakeAHexDump(const void * inData, UInt32 inDataSize);
    virtual void                  PrintAllHCIRequests();
    virtual void                  PrintDeviceListAddress();
    virtual void                  PrintClassVariablesContent();
    virtual os_log_t              CreateOSLogObject();
    virtual BluetoothHCIRequestID GetMaxRequestID(); // return 500
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    virtual IOReturn              TestLEAdvertisingReportEvent(UInt8);
#endif
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    virtual bool                  IsClassicSecureConnectionsSupported();
#endif

    virtual IOReturn BluetoothHCIInquiry(BluetoothHCIRequestID inID, BluetoothLAP inLAP, BluetoothHCIInquiryLength inInquiryLength, BluetoothHCIResponseCount inMaxResponseCount,
                                         BluetoothHCIInquiryResults * outResults);
    virtual IOReturn BluetoothHCIInquiryCancel(BluetoothHCIRequestID inID);
    virtual IOReturn BluetoothHCIPeriodicInquiryMode(BluetoothHCIRequestID inID, BluetoothHCIInquiryLength inMaxPeriodLength, BluetoothHCIInquiryLength inMinPeriodLength, BluetoothLAP inLAP,
                                                     BluetoothHCIInquiryLength inInquiryLength, BluetoothHCIResponseCount inMaxResponses, BluetoothHCIInquiryResults * outResults);
    virtual IOReturn BluetoothHCIExitPeriodicInquiryMode(BluetoothHCIRequestID inID);
    virtual IOReturn BluetoothHCICreateConnection(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr, BluetoothPacketType inPacketType,
                                                  BluetoothPageScanRepetitionMode inPageScanRepetitionMode, BluetoothPageScanMode inPageScanMode, BluetoothClockOffset inClockOffset,
                                                  BluetoothAllowRoleSwitch inAllowRoleSwitch, BluetoothHCIEventConnectionCompleteResults * outConnectionHandle);
    virtual IOReturn BluetoothHCIDisconnect(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothReasonCode inReason,
                                            BluetoothHCIEventDisconnectionCompleteResults * outResults);
    virtual IOReturn BluetoothHCIAddSCOConnection(BluetoothHCIRequestID inID, BluetoothConnectionHandle inACLConnectionHandle, BluetoothPacketType inPacketType);
    virtual IOReturn BluetoothHCIAcceptConnectionRequest(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr, BluetoothRole inRole);
    virtual IOReturn BluetoothHCIRejectConnectionRequest(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr, BluetoothReasonCode inReason);
    virtual IOReturn BluetoothHCILinkKeyRequestReply(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr, const BluetoothKey * inKeyPtr, BluetoothDeviceAddress * outAddress);
    virtual IOReturn BluetoothHCILinkKeyRequestNegativeReply(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr, BluetoothDeviceAddress * outAddress);
    virtual IOReturn BluetoothHCIPINCodeRequestReply(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr, IOByteCount inPINCodeSize, const BluetoothPINCode * inPINCode,
                                                     BluetoothDeviceAddress * outAddress);
    virtual IOReturn BluetoothHCIPINCodeRequestNegativeReply(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr, BluetoothDeviceAddress * outAddress);
    virtual IOReturn BluetoothHCIChangeConnectionPacketType(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothPacketType inPacketType);
    virtual IOReturn BluetoothHCIAuthenticationRequested(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle);
    virtual IOReturn BluetoothHCISetEncryptionEnable(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothEncryptionEnable inEncryptionEnable);
    virtual IOReturn BluetoothHCIChangeConnectionLinkKey(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle);
    virtual IOReturn BluetoothHCIMasterLinkKey(BluetoothHCIRequestID inID, BluetoothKeyFlag inKeyFlag);
    virtual IOReturn BluetoothHCIRemoteNameRequest(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr, BluetoothPageScanRepetitionMode inPageScanRepetitionMode,
                                                   BluetoothPageScanMode inPageScanMode, BluetoothClockOffset inClockOffset, BluetoothHCIEventRemoteNameRequestResults * outName);
    virtual IOReturn BluetoothHCIReadRemoteSupportedFeatures(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle,
                                                             BluetoothHCIEventReadRemoteSupportedFeaturesResults * outFeatures);
    virtual IOReturn BluetoothHCIReadRemoteExtendedFeatures(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothHCIExtendedInquiryResponseDataType inDataType,
                                                            BluetoothHCIEventReadRemoteExtendedFeaturesResults * outFeatures);
    virtual IOReturn BluetoothHCIReadRemoteVersionInformation(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothHCIEventReadRemoteVersionInfoResults * outVersionInfo);
    virtual IOReturn BluetoothHCIReadClockOffset(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothClockOffset * outClockOffset);

    virtual IOReturn BluetoothHCIIOCapabilityRequestReply(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr, UInt8, UInt8, UInt8);
    virtual IOReturn BluetoothHCIUserConfirmationRequestReply(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr);
    virtual IOReturn BluetoothHCIUserConfirmationRequestNegativeReply(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr);
    virtual IOReturn BluetoothHCIUserPasskeyRequestReply(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr, UInt32);
    virtual IOReturn BluetoothHCIUserPasskeyRequestNegativeReply(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr);
    virtual IOReturn BluetoothHCIRemoteOOBDataRequestReply(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr, BluetoothHCISimplePairingOOBData *,
                                                           BluetoothHCISimplePairingOOBData *);
    virtual IOReturn BluetoothHCIRemoteOOBDataRequestNegativeReply(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr);
    virtual IOReturn BluetoothHCIIOCapabilityRequestNegativeReply(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr, UInt8);
    virtual IOReturn BluetoothHCISetupSynchronousConnection(BluetoothHCIRequestID inID, BluetoothConnectionHandle inHandle, BluetoothHCISetupSynchronousConnectionParams * inParams);
    virtual IOReturn BluetoothHCIAcceptSynchronousConnectionRequest(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr, BluetoothHCIAcceptSynchronousConnectionRequestParams *);
    virtual IOReturn BluetoothHCIRejectSynchronousConnectionRequest(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr, UInt8);
    virtual IOReturn BluetoothHCIEnhancedSetupSynchronousConnection(BluetoothHCIRequestID inID, BluetoothConnectionHandle inHandle, BluetoothHCIEnhancedSetupSynchronousConnectionParams * inParams);
    virtual IOReturn BluetoothHCIEnhancedAcceptSynchronousConnectionRequest(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr,
                                                                            BluetoothHCIEnhancedAcceptSynchronousConnectionRequestParams *);

    virtual IOReturn BluetoothHCIHoldMode(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothHCIModeInterval inHoldModeMaxInterval,
                                          BluetoothHCIModeInterval inHoldModeMinInterval);
    virtual IOReturn BluetoothHCISniffMode(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothHCIModeInterval inSniffModeMaxInterval,
                                           BluetoothHCIModeInterval inSniffModeMinInterval, BluetoothHCISniffAttemptCount inSniffAttemptCount, BluetoothHCISniffTimeout inSniffModeTimeout);
    virtual IOReturn BluetoothHCIExitSniffMode(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle);
    virtual IOReturn BluetoothHCIParkMode(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothHCIParkModeBeaconInterval inMaxInterval,
                                          BluetoothHCIParkModeBeaconInterval inMinInterval);
    virtual IOReturn BluetoothHCIExitParkMode(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle);
    virtual IOReturn BluetoothHCIQualityOfServiceSetup(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothHCIQualityOfServiceSetupParams * inSetupPtr);
    virtual IOReturn BluetoothHCIRoleDiscovery(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothHCIRoleInfo * outRoleInfo);
    virtual IOReturn BluetoothHCISwitchRole(BluetoothHCIRequestID inID, BluetoothDeviceAddress * inAddressPtr, BluetoothHCIRole inNewRole);
    virtual IOReturn BluetoothHCIReadLinkPolicySettings(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothHCILinkPolicySettingsInfo * outSettingsInfo);
    virtual IOReturn BluetoothHCIReadDefaultLinkPolicySettings(BluetoothHCIRequestID inID, BluetoothHCILinkPolicySettingsInfo * outSettingsInfo);
    virtual IOReturn BluetoothHCIWriteLinkPolicySettings(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothHCILinkPolicySettings inSettings,
                                                         BluetoothConnectionHandle * outConnectionHandle);
    virtual IOReturn BluetoothHCIWriteDefaultLinkPolicySettings(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle);
    virtual IOReturn BluetoothHCISniffSubrating(BluetoothHCIRequestID inID, UInt16, UInt16, UInt16, UInt16, UInt16 *);
    virtual IOReturn BluetoothHCIFlowSpecification(BluetoothHCIRequestID inID, BluetoothHCIEventFlowSpecificationData *, BluetoothHCIEventFlowSpecificationData *);
    virtual IOReturn BluetoothHCISetEventMask(BluetoothHCIRequestID inID, BluetoothSetEventMask * inMask);
    virtual IOReturn BluetoothHCILESetEventMask(BluetoothHCIRequestID inID, BluetoothSetEventMask * inMask);
    virtual IOReturn BluetoothHCILEReadBufferSize(BluetoothHCIRequestID inID, BluetoothHCILEBufferSize * outSize);
    virtual IOReturn BluetoothHCILESetScanEnable(BluetoothHCIRequestID inID, UInt8, UInt8);
    virtual IOReturn BluetoothHCILESetAdvertiseEnable(BluetoothHCIRequestID inID, UInt8);
    virtual IOReturn BluetoothHCILEReadLocalSupportedFeatures(BluetoothHCIRequestID inID, BluetoothHCISupportedFeatures * outFeatures);
    virtual IOReturn BluetoothHCILEReadRemoteUsedFeatures(BluetoothHCIRequestID inID, UInt16, BluetoothHCISupportedFeatures * outFeatures);
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_11_0
    virtual IOReturn BluetoothHCIReset(BluetoothHCIRequestID inID, char * name);
#else
    virtual IOReturn BluetoothHCIReset(BluetoothHCIRequestID inID);
#endif
    virtual IOReturn BluetoothHCISetEventFilter(BluetoothHCIRequestID inID, UInt8, UInt8, BluetoothEventFilterCondition *);
    virtual IOReturn BluetoothHCIFlush(BluetoothHCIRequestID inID, BluetoothConnectionHandle inHandle);
    virtual IOReturn BluetoothHCIReadPINType(BluetoothHCIRequestID inID, BluetoothPINType * outType);
    virtual IOReturn BluetoothHCIWritePINType(BluetoothHCIRequestID inID, BluetoothPINType inType);

    virtual IOReturn BluetoothHCICreateNewUnitKey(BluetoothHCIRequestID inID);
    virtual IOReturn BluetoothHCIReadStoredLinkKey(BluetoothHCIRequestID inID, BluetoothDeviceAddress * targetDevice, BluetoothHCIReadStoredLinkKeysFlag * inFlags,
                                                   BluetoothHCIStoredLinkKeysInfo * outKeysInfo);
    virtual IOReturn BluetoothHCIWriteStoredLinkKey(BluetoothHCIRequestID inID, IOItemCount inNumKeysToWrite, BluetoothDeviceAddress inDeviceAddresses[], BluetoothKey inLinkKeys[],
                                                    BluetoothHCINumLinkKeysToWrite * outNumKeysWritten);
    virtual IOReturn BluetoothHCIDeleteStoredLinkKey(BluetoothHCIRequestID inID, BluetoothDeviceAddress * targetDevice, BluetoothHCIDeleteStoredLinkKeyFlag * inFlag,
                                                     BluetoothHCINumLinkKeysDeleted * outNumDeleted);

    virtual IOReturn BluetoothHCIChangeLocalName(BluetoothHCIRequestID inID, BluetoothDeviceName newName);
    virtual IOReturn BluetoothHCIReadLocalName(BluetoothHCIRequestID inID, BluetoothDeviceName name);
    virtual IOReturn BluetoothHCIReadConnectionAcceptTimeout(BluetoothHCIRequestID inID, BluetoothHCIConnectionAcceptTimeout * outTimeout);
    virtual IOReturn BluetoothHCIWriteConnectionAcceptTimeout(BluetoothHCIRequestID inID, BluetoothHCIConnectionAcceptTimeout inTimeout);
    virtual IOReturn BluetoothHCIReadPageTimeout(BluetoothHCIRequestID inID, BluetoothHCIPageTimeout * outDataPtr);
    virtual IOReturn BluetoothHCIWritePageTimeout(BluetoothHCIRequestID inID, BluetoothHCIPageTimeout inTimeout);
    virtual IOReturn BluetoothHCIReadScanEnable(BluetoothHCIRequestID inID, BluetoothHCIPageScanEnableState * outState);
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    virtual IOReturn BluetoothHCIWriteScanEnable(BluetoothHCIRequestID inID, BluetoothHCIPageScanEnableState inState, bool);
#else
    virtual IOReturn BluetoothHCIWriteScanEnable(BluetoothHCIRequestID inID, BluetoothHCIPageScanEnableState inState);
#endif
    virtual IOReturn BluetoothHCIReadPageScanActivity(BluetoothHCIRequestID inID, BluetoothHCIScanActivity * outActivityInfo);
    virtual IOReturn BluetoothHCIWritePageScanActivity(BluetoothHCIRequestID inID, BluetoothHCIScanActivity * inActivityInfo);
    virtual IOReturn BluetoothHCIReadInquiryScanActivity(BluetoothHCIRequestID inID, BluetoothHCIScanActivity * outActivityInfo);
    virtual IOReturn BluetoothHCIWriteInquiryScanActivity(BluetoothHCIRequestID inID, BluetoothHCIScanActivity * inActivityInfo);
    virtual IOReturn BluetoothHCIReadAuthenticationEnable(BluetoothHCIRequestID inID, BluetoothHCIAuthenticationEnable * outAuthenticationState);
    virtual IOReturn BluetoothHCIWriteAuthenticationEnable(BluetoothHCIRequestID inID, BluetoothHCIAuthenticationEnable inAuthenticationState);

    virtual IOReturn BluetoothHCIReadEncryptionMode(BluetoothHCIRequestID inID, BluetoothHCIEncryptionMode * outEncryptionState);
    virtual IOReturn BluetoothHCIWriteEncryptionMode(BluetoothHCIRequestID inID, BluetoothHCIEncryptionMode inEncryptionMode);
    virtual IOReturn BluetoothHCIReadClassOfDevice(BluetoothHCIRequestID inID, BluetoothClassOfDevice * outClassOfDevice);
    virtual IOReturn BluetoothHCIWriteClassOfDevice(BluetoothHCIRequestID inID, BluetoothClassOfDevice inClassOfDevice);
    virtual IOReturn BluetoothHCIReadVoiceSetting(BluetoothHCIRequestID inID, BluetoothHCIVoiceSetting * outVoiceSetting);
    virtual IOReturn BluetoothHCIWriteVoiceSetting(BluetoothHCIRequestID inID, BluetoothHCIVoiceSetting inVoiceSetting);
    virtual IOReturn BluetoothHCIReadAutomaticFlushTimeout(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle,
                                                           BluetoothHCIAutomaticFlushTimeoutInfo * outAutomaticFlushTimeoutInfo);
    virtual IOReturn BluetoothHCIWriteAutomaticFlushTimeout(BluetoothHCIRequestID inID, BluetoothHCIAutomaticFlushTimeoutInfo * inAutomaticFlushTimeoutInfo,
                                                            BluetoothConnectionHandle * outConnectionHandle);
    virtual IOReturn BluetoothHCIReadNumBroadcastRetransmissions(BluetoothHCIRequestID inID, BluetoothHCINumBroadcastRetransmissions * outNumRetrans);
    virtual IOReturn BluetoothHCIWriteNumBroadcastRetransmissions(BluetoothHCIRequestID inID, BluetoothHCINumBroadcastRetransmissions inNumRetrans);
    virtual IOReturn BluetoothHCIReadHoldModeActivity(BluetoothHCIRequestID inID, BluetoothHCIHoldModeActivity * outState);
    virtual IOReturn BluetoothHCIWriteHoldModeActivity(BluetoothHCIRequestID inID, BluetoothHCIHoldModeActivity inState);
    virtual IOReturn BluetoothHCIReadTransmitPowerLevel(BluetoothHCIRequestID inID, BluetoothConnectionHandle inHandle, BluetoothHCITransmitPowerLevelType inType,
                                                        BluetoothHCITransmitPowerLevelInfo * outLevelInfo);
    virtual IOReturn BluetoothHCIReadSCOFlowControlEnable(BluetoothHCIRequestID inID, BluetoothHCIFlowControlState * outState);
    virtual IOReturn BluetoothHCIWriteSCOFlowControlEnable(BluetoothHCIRequestID inID, BluetoothHCIFlowControlState inState);
    virtual IOReturn BluetoothHCISetHostControllerToHostFlowControl(BluetoothHCIRequestID inID, BluetoothHCIFlowControlState inState);
    virtual IOReturn BluetoothHCIHostBufferSize(BluetoothHCIRequestID inID, BluetoothHCIBufferSize * inSize);
    virtual IOReturn BluetoothHCIHostNumberOfCompletedPackets(BluetoothHCIRequestID inID, UInt8, UInt16 *, UInt16 *);
    virtual IOReturn BluetoothHCIReadLinkSupervisionTimeout(BluetoothHCIRequestID inID, BluetoothConnectionHandle inHandle, BluetoothHCILinkSupervisionTimeout * outInfo);
    virtual IOReturn BluetoothHCIWriteLinkSupervisionTimeout(BluetoothHCIRequestID inID, BluetoothHCILinkSupervisionTimeout * inInfo, BluetoothConnectionHandle * outHandle);
    virtual IOReturn BluetoothHCIReadNumberOfSupportedIAC(BluetoothHCIRequestID inID, BluetoothHCISupportedIAC * outNumSupported);
    virtual IOReturn BluetoothHCIReadCurrentIACLAP(BluetoothHCIRequestID inID, BluetoothHCICurrentInquiryAccessCodes * inAccessCodes);
    virtual IOReturn BluetoothHCIWriteCurrentIACLAP(BluetoothHCIRequestID inID, BluetoothHCICurrentInquiryAccessCodesForWrite * inAccessCodes);
    virtual IOReturn BluetoothHCIReadPageScanPeriodMode(BluetoothHCIRequestID inID, BluetoothHCIPageScanPeriodMode * outMode);
    virtual IOReturn BluetoothHCIWritePageScanPeriodMode(BluetoothHCIRequestID inID, BluetoothHCIPageScanPeriodMode inMode);
    virtual IOReturn BluetoothHCIReadPageScanMode(BluetoothHCIRequestID inID, BluetoothHCIPageScanMode * outMode);
    virtual IOReturn BluetoothHCIWritePageScanMode(BluetoothHCIRequestID inID, BluetoothHCIPageScanMode inMode);

    virtual IOReturn BluetoothHCIEnhancedFlush(BluetoothHCIRequestID inID, UInt16, UInt8, UInt16 *);
    virtual IOReturn BluetoothHCIReadExtendedInquiryResponse(BluetoothHCIRequestID inID, BluetoothHCIReadExtendedInquiryResponseResults *);
    virtual IOReturn BluetoothHCIWriteExtendedInquiryResponse(BluetoothHCIRequestID inID, UInt8, BluetoothHCIExtendedInquiryResponse *);
    virtual IOReturn BluetoothHCIRefreshEncryptionKey(BluetoothHCIRequestID inID, UInt16, BluetoothHCIEventEncryptionKeyRefreshCompleteResults *);
    virtual IOReturn BluetoothHCIReadInquiryMode(BluetoothHCIRequestID inID, BluetoothHCIInquiryMode *);
    virtual IOReturn BluetoothHCIWriteInquiryMode(BluetoothHCIRequestID inID, BluetoothHCIInquiryMode);
    virtual IOReturn BluetoothHCIReadSimplePairingMode(BluetoothHCIRequestID inID, BluetoothHCISimplePairingMode *);
    virtual IOReturn BluetoothHCIWriteSimplePairingMode(BluetoothHCIRequestID inID, BluetoothHCISimplePairingMode);
    virtual IOReturn BluetoothHCIReadLocalOOBData(BluetoothHCIRequestID inID, BluetoothHCIReadLocalOOBDataResults *);
    virtual IOReturn BluetoothHCIReadInquiryResponseTransmitPower(BluetoothHCIRequestID inID, BluetoothHCITransmitPowerLevel *);
    virtual IOReturn BluetoothHCIWriteInquiryResponseTransmitPower(BluetoothHCIRequestID inID, BluetoothHCITransmitPowerLevel);
    virtual IOReturn BluetoothHCISendKeypressNotification(BluetoothHCIRequestID inID, const BluetoothDeviceAddress * inAddressPtr, UInt8);
    virtual IOReturn BluetoothHCIReadDefaultErroneousDataReporting(BluetoothHCIRequestID inID, BluetoothHCIErroneousDataReporting *);
    virtual IOReturn BluetoothHCIWriteDefaultErroneousDataReporting(BluetoothHCIRequestID inID, BluetoothHCIErroneousDataReporting);
    virtual IOReturn BluetoothHCIReadAFHChannelAssessmentMode(BluetoothHCIRequestID inID, BluetoothHCIAFHChannelAssessmentMode *);
    virtual IOReturn BluetoothHCIWriteAFHChannelAssessmentMode(BluetoothHCIRequestID inID, BluetoothHCIAFHChannelAssessmentMode);
    virtual IOReturn BluetoothHCISetAFHHostChannelClassification(BluetoothHCIRequestID inID, BluetoothAFHHostChannelClassification *);
    virtual IOReturn BluetoothHCIReadInquiryScanType(BluetoothHCIRequestID inID, BluetoothHCIInquiryScanType *);
    virtual IOReturn BluetoothHCIWriteInquiryScanType(BluetoothHCIRequestID inID, BluetoothHCIInquiryScanType);
    virtual IOReturn BluetoothHCIReadPageScanType(BluetoothHCIRequestID inID, BluetoothHCIPageScanType *);
    virtual IOReturn BluetoothHCIWritePageScanType(BluetoothHCIRequestID inID, BluetoothHCIPageScanType);

    virtual IOReturn BluetoothHCIReadLocalVersionInformation(BluetoothHCIRequestID inID, BluetoothHCIVersionInfo * outVersionInfo);
    virtual IOReturn BluetoothHCIReadLocalSupportedCommands(BluetoothHCIRequestID inID, BluetoothHCISupportedCommands * outCommands);
    virtual IOReturn BluetoothHCIReadLocalSupportedFeatures(BluetoothHCIRequestID inID, BluetoothHCISupportedFeatures * outFeatures);
    virtual IOReturn BluetoothHCIReadLocalExtendedFeatures(BluetoothHCIRequestID inID, UInt8, BluetoothHCIExtendedFeaturesInfo * outFeatures);
    virtual IOReturn BluetoothHCIReadBufferSize(BluetoothHCIRequestID inID, BluetoothHCIBufferSize * outSize);
    virtual IOReturn BluetoothHCIReadCountryCode(BluetoothHCIRequestID inID, BluetoothHCICountryCode * outCountryCode);
    virtual IOReturn BluetoothHCIReadDeviceAddress(BluetoothHCIRequestID inID, BluetoothDeviceAddress * outAddress);
    virtual IOReturn BluetoothHCIReadFailedContactCounter(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothHCIFailedContactInfo * outFailedContactCount);
    virtual IOReturn BluetoothHCIResetFailedContactCounter(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle);
    virtual IOReturn BluetoothHCIGetLinkQuality(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothHCILinkQualityInfo * outLinkQualityInfo);
    virtual IOReturn BluetoothHCIReadRSSI(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothHCIRSSIInfo * outRSSIInfo);
    virtual IOReturn BluetoothHCIReadAFHChannelMap(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothAFHResults * outAFHResults);
    virtual IOReturn BluetoothHCIReadClock(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, UInt8, BluetoothReadClockInfo * outClockInfo);
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    virtual IOReturn BluetoothHCIReadEncryptionKeySize(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothHCIEncryptionKeySizeInfo * outEncryptionKeySizeInfo);
#endif
    virtual IOReturn BluetoothHCICreateConnectionCancel(BluetoothHCIRequestID inID, const BluetoothDeviceAddress *, BluetoothDeviceAddress *);
    virtual IOReturn BluetoothHCIRemoteNameRequestCancel(BluetoothHCIRequestID inID, const BluetoothDeviceAddress *, BluetoothDeviceAddress *);
    virtual IOReturn BluetoothHCIReadLMPHandle(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothHCIReadLMPHandleResults * outLMPHandleResults);

    virtual IOReturn BluetoothHCIReadLoopbackMode(BluetoothHCIRequestID inID, BluetoothHCILoopbackMode * inLoopbackMode);
    virtual IOReturn BluetoothHCIWriteLoopbackMode(BluetoothHCIRequestID inID, BluetoothHCILoopbackMode inLoopbackMode);
    virtual IOReturn BluetoothHCIEnableDeviceUnderTestMode(BluetoothHCIRequestID inID);
    virtual IOReturn BluetoothHCIWriteSimplePairingDebugMode(BluetoothHCIRequestID inID, BluetoothSimplePairingDebugMode inSimplePairingDebugMode);

    virtual IOReturn VendorCommand(BluetoothHCIRequestID inID, BluetoothHCIVendorCommandSelector inSelector, UInt8 * inCommandData, IOByteCount inCommandDataSize, IOByteCount outBufferSize,
                                   UInt8 * outBuffer);

    virtual IOReturn BluetoothHCIBroadcomReadRawRSSI(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, BluetoothHCIRSSIInfo * outRSSIInfo);
    virtual IOReturn BluetoothHCIBroadcomBFCSuspend(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle);
    virtual IOReturn BluetoothHCIBroadcomBFCResume(BluetoothHCIRequestID inID, BluetoothConnectionHandle inConnectionHandle, const BluetoothDeviceAddress * inAddressPtr,
                                                   BluetoothBroadcomBFCReconnectData *);
    virtual IOReturn BluetoothHCIBroadcomBFCReadParams(UInt32, BluetoothBroadcomBFCParams *);
    virtual IOReturn BluetoothHCIBroadcomBFCSetParams(UInt32, BluetoothBroadcomBFCParams *);
    virtual IOReturn BluetoothHCIBroadcomSetTransmitPower(UInt32, UInt16, SInt8);
    virtual IOReturn BluetoothHCIBroadcomBFCReadRemoteBPCSFeatures(UInt32, UInt16, BluetoothBroadcomBFCRemoteBPCSFeatures *);
    virtual IOReturn BluetoothHCIBroadcomBFCWriteScanEnable(UInt32, UInt8);
    virtual IOReturn BluetoothHCIBroadcomBFCCreateConnection(UInt32, const BluetoothDeviceAddress *, UInt16, BluetoothHCIEventConnectionCompleteResults *);
    virtual IOReturn BluetoothHCIBroadcomSetEventMask(UInt32, BluetoothBroadcomSetEventMask *);
    virtual IOReturn BluetoothHCIBroadcomReadLocalFirmwareInfo(UInt32, UInt8, BluetoothBroadcomLocalFirmwareInfo *);
    virtual IOReturn BluetoothHCIBroadcomBFCReadScanEnable(UInt32, UInt8 *);
    virtual IOReturn BluetoothHCIBroadcomBFCIsConnectionTBFCSuspended(UInt32, UInt16, BluetoothBroadcomBFCConnectionTBFCSuspendedInfo *);
    virtual IOReturn BluetoothHCIBroadcomSetUSBAutoResume(UInt32, UInt16);
    virtual IOReturn BluetoothHCIBroadcomChangeLNAGainCoexsECI(UInt32, UInt8);
    virtual IOReturn BluetoothHCIBroadcomTurnOFFDynamicPowerControl(UInt32, UInt8, const BluetoothDeviceAddress *);
    virtual IOReturn BluetoothHCIBroadcomIncreaseDecreasePowerLevel(UInt32, const BluetoothDeviceAddress *, UInt8);
    virtual IOReturn BluetoothHCIBroadcomARMMemoryPoke(UInt32, UInt32, UInt32, UInt8 *);
    virtual IOReturn BluetoothHCIBroadcomARMMemoryPeek(UInt32, UInt32, UInt8 *);
    virtual IOReturn BluetoothHCIBroadcomUARTSetSleepmodeParam(UInt32, UInt8, UInt8, UInt8, UInt8, UInt8, UInt8, UInt8, UInt32, UInt8);
    virtual IOReturn BluetoothHCIBroadcomReadVerboseConfigVersionInfo(UInt32, BluetoothBroadcomVerboseConfigVersionInfo *);
    virtual IOReturn BluetoothHCIBroadcomGetBasicRateACLConnectionStats();
    virtual IOReturn BluetoothHCIBroadcomResetBasicRateACLConnectionStats();
    virtual IOReturn BluetoothHCIBroadcomGetEDRACLConnectionStats();
    virtual IOReturn BluetoothHCIBroadcomIgnoreUSBReset(UInt32, UInt8, UInt8 *);
    virtual IOReturn BluetoothHCIBroadcomLighthouseControl(UInt32, UInt8);
    virtual IOReturn BluetoothHCIBroadcomLighthouseSetParameters(UInt32, UInt16, UInt16, UInt16, UInt8, UInt8, SInt8, UInt8);
    virtual IOReturn BluetoothHCIBroadcomLighthouseDebugQuery(UInt32, UInt8);
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    virtual IOReturn BluetoothHCIBroadcomMasterSkipSniffMode(UInt16, UInt8, UInt8, UInt16, UInt16);
    virtual IOReturn BluetoothHCIBroadcomLoadPwrRegulatoryFile(UInt8 *, UInt8);
#endif
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_11_0
    virtual IOReturn BluetoothWriteLocalAddressFromRegistry();
#endif

private:
    static bool     windowServerDidAppear(void * target, void * refCon, IOService * newService, IONotifier * notifier);
    static IOReturn windowServerDidAppearAction(OSObject * owner, void * arg0, void * arg1, void * arg2, void * arg3, void * arg4, void * arg5);

    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 0);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 1);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 2);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 3);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 4);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 5);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 6);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 7);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 8);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 9);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 10);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 11);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 12);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 13);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 14);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 15);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 16);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 17);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 18);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 19);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 20);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 21);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 22);
    OSMetaClassDeclareReservedUnused(IOBluetoothHostController, 23);

public:
    OSSet *                    mReporterSet;                  // 136
    IOReportLegend *           mReportLegend;                 // 144
    IOStateReporter *          mPowerStateReporter;           // 152
    BluetoothDeviceReporter ** mDeviceReporterList;           // 160
    BluetoothDeviceReporter *  mActiveDeviceReporter;         // 168
    IOSimpleReporter *         mAppleBTLEAdvertisingReporter; // 176
    IOSimpleReporter *         mLESetAdvertisingDataReporter; // 184
    IONotifier *               mWindowServerNotifier;         // 192
    IOWorkLoop *               mWorkLoop;                     // 200
    IOCommandGate *            mCommandGate;                  // 208
    IOBluetoothHCIRequest *    mBusyQueueHead;                // 216
    IOBluetoothHCIRequest *    mWaitQueueHead;                // 224
    IOBluetoothHCIRequest **   mHCIRequestList;               // 232
    IOByteCount                mHCIRequestListSize;           // 240
    BluetoothHCIRequestID      mCurrentRequestID;             // 248

    IOBluetoothDevice *      mDeviceListHead;               // 256
    UInt8 *                  mEventDataBuffer;              // 264, size is 2048
    IOByteCount              mEventDataBufferSize;          // 272
    UInt8                    mNumberOfLowPriorityDevice;    // 280
    UInt8                    mNumberOfMidPriorityDevice;    // 281
    UInt8                    mNumberOfHighPriorityDevice;   // 282
    UInt8                    mNumberOfLowPriorityLEDevice;  // 283
    UInt8                    mNumberOfMidPriorityLEDevice;  // 284
    UInt8                    mNumberOfHighPriorityLEDevice; // 285
    BluetoothHCIBufferSize   mACLSCOBufferSize;             // 286
    BluetoothHCILEBufferSize mLEACLBufferSize;              // 294

    UInt16 mPreviousNumberOfOutstandingLowPriorityACLPackets;  // 298
    UInt16 mPreviousNumberOfOutstandingMidPriorityACLPackets;  // 300
    UInt16 mPreviousNumberOfOutstandingHighPriorityACLPackets; // 302
    UInt16 mTotalNumberOfPreviousOutstandingACLPackets;        // 304, never used

    UInt16 mNumberOfOutstandingLowPriorityACLPackets;  // 306
    UInt16 mNumberOfOutstandingMidPriorityACLPackets;  // 308
    UInt16 mNumberOfOutstandingHighPriorityACLPackets; // 310
    UInt16 mTotalNumberOfOutstandingACLPackets;        // 312

    UInt32 mNumberOfLowPriorityACLPacketsInQueue;  // 316
    UInt32 mNumberOfMidPriorityACLPacketsInQueue;  // 320
    UInt32 mNumberOfHighPriorityACLPacketsInQueue; // 324
    UInt32 mTotalNumberOfACLPacketsInAllQueues;    // 328

    UInt16 mNumberOfAllowedLowPriorityACLDataPackets;  // 332
    UInt16 mNumberOfAllowedMidPriorityACLDataPackets;  // 334
    UInt16 mNumberOfAllowedHighPriorityACLDataPackets; // 336

    BluetoothHCIACLPacket * mHighPriorityACLPacketsHead; // 344
    BluetoothHCIACLPacket * mHighPriorityACLPacketsTail; // 352
    BluetoothHCIACLPacket * mMidriorityACLPacketsHead;   // 360
    BluetoothHCIACLPacket * mMidPriorityACLPacketsTail;  // 368
    BluetoothHCIACLPacket * mLowPriorityACLPacketsHead;  // 376
    BluetoothHCIACLPacket * mLowPriorityACLPacketsTail;  // 384

    UInt16 mPreviousNumberOfOutstandingLowPriorityLEACLPackets;  // 392
    UInt16 mPreviousNumberOfOutstandingMidPriorityLEACLPackets;  // 394
    UInt16 mPreviousNumberOfOutstandingHighPriorityLEACLPackets; // 396
    UInt16 mPreviousTotalNumberOfOutstandingLEACLPackets;        // 398

    UInt16 mNumberOfOutstandingLowPriorityLEACLPackets;  // 400
    UInt16 mNumberOfOutstandingMidPriorityLEACLPackets;  // 402
    UInt16 mNumberOfOutstandingHighPriorityLEACLPackets; // 404
    UInt16 mTotalNumberOfOutstandingLEACLPackets;        // 406

    UInt32 mNumberOfLowPriorityLEACLPacketsInQueue;  // 408
    UInt32 mNumberOfMidPriorityLEACLPacketsInQueue;  // 412
    UInt32 mNumberOfHighPriorityLEACLPacketsInQueue; // 416
    UInt32 mTotalNumberOfLEACLPacketsInAllQueues;    // 420

    UInt16 mNumberOfAllowedLowPriorityLEACLDataPackets;  // 424
    UInt16 mNumberOfAllowedMidPriorityLEACLDataPackets;  // 426
    UInt16 mNumberOfAllowedHighPriorityLEACLDataPackets; // 428

    BluetoothHCIACLPacket * mHighPriorityLEACLPacketsHead; // 432
    BluetoothHCIACLPacket * mHighPriorityLEACLPacketsTail; // 440
    BluetoothHCIACLPacket * mMidriorityLEACLPacketsHead;   // 448
    BluetoothHCIACLPacket * mMidPriorityLEACLPacketsTail;  // 456
    BluetoothHCIACLPacket * mLowPriorityLEACLPacketsHead;  // 464
    BluetoothHCIACLPacket * mLowPriorityLEACLPacketsTail;  // 472

    OSArray *                               mAllowedIncomingL2CAPChannels;        // 480
    UInt32                                  mDesyncIncomingDataCounter;           // 488
    UInt32                                  mCurrentlyExecutingSequenceNumber;    // 492
    OSArray *                               mAllowedIncomingRFCOMMChannels;       // 496
    IOBluetoothHCIControllerConfigState     mPreviousControllerConfigState;       // 504
    UInt32                                  unknown2;                             // 508
    UInt32                                  unknown3;                             // 512
    UInt16                                  unknown4;                             // 516
    bool                                    unknown5;                             // 518
    UInt8 *                                 mSCOPacketBuffer;                     // 520
    UInt16                                  mNumBufferedSCOBytes;                 // 528
    AbsoluteTime                            mBufferedSCOPacketTimestamp;          // 536
    IOBluetoothInactivityTimerEventSource * mIdleTimer;                           // 544
    UInt32                                  mNumSCODataInQueue;                   // 552
    UInt32                                  mCurrentlyExecutingSCOSequenceNumber; // 556
    bool                                    mNeedToCleanUpWaitForAckQueue;        // 560
    UInt16                                  mSynchronousConnectionPacketType;     // 562

    HearingDeviceListType * mConnectedHearingDeviceListHead; // 568
    HearingDeviceListType * mConnectedHearingDeviceListTail; // 576

    BluetoothHCISetupSynchronousConnectionParams mSetupSynchronousConnectionParams;        // 584
    BluetoothConnectionHandle                    mSetupSynchronousConnectionHandle;        // 600
    IOBluetoothDevice *                          mSetupSynchronousConnectionDevice;        // 608
    BluetoothDeviceAddress *                     mSetupSynchronousConnectionDeviceAddress; // 616

    BluetoothHCIEnhancedSetupSynchronousConnectionParams mEnhancedSetupSynchronousConnectionParams;        // 624
    BluetoothConnectionHandle                            mEnhancedSetupSynchronousConnectionHandle;        // 704
    IOBluetoothDevice *                                  mEnhancedSetupSynchronousConnectionDevice;        // 712
    BluetoothDeviceAddress *                             mEnhancedSetupSynchronousConnectionDeviceAddress; // 720

    IOByteCount mTotalDataBytesSent;           // 728
    IOByteCount mTotalDataBytesReceived;       // 736
    IOByteCount mTotalSCOBytesReceived;        // 744
    bool        mKillAllPendingRequestsCalled; // 752
    bool        mScanEnabled;                  // 753
	uint8_t 	__reserved0[6];				   // 754

    BluetoothSetEventMask mSetEventMask;            // 760
    BluetoothSetEventMask mPreviousSetEventMask;    // 768
    BluetoothSetEventMask mLESetEventMask;          // 776
    BluetoothSetEventMask mPreviousLESetEventMask;  // 784
    bool                  mLESetAdvertisingEnabled; // 792
    bool                  mLESetScanEnabled;        // 793

    OSSet *                              mHostControllerClients; // 800
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    LEDeviceListType *                   mLEDeviceListHead;      // 808
    LEDeviceListType *                   mLEDeviceListTail;      // 816
#endif
    IOBluetoothHCIController *           mBluetoothFamily;       // 824
    IOBluetoothHostControllerTransport * mBluetoothTransport;    // 832
    IOWorkQueue *                        mControllerWorkQueue;   // 840
    IOWorkQueue *                        mReporterWorkQueue;     // 848
    UInt16                               __reserved1;            // 856
    UInt16                               mVendorID;              // 858
    UInt16                               mProductID;             // 860

    UInt32                 mLocationID;                         // 864
    BluetoothDeviceAddress mDeviceAddress;                      // 868
    bool                   mBuiltIn;                            // 874
    bool                   mHostControllerVariablesInitialized; // 875
    bool                   mControllerSetupIsComplete;          // 876
    bool                   mTransportIsReady;                   // 877
    bool                   mTransportIsGoingAway;               // 878
    UInt8                  mControllerSetupState;               // 879
    UInt16                 mActiveControllerType;               // 880
    UInt8                  unknown6;                            // 882
    UInt8                  unknown7;                            // 883
    UInt8                  unknown8;                            // 884
    bool                   mNeedToWaitForPowerStateChangeToOn;  // 885
    bool                   mSupportNewIdlePolicy;               // 886
    bool                   mSupportPowerOff;                    // 887
    bool                   mSupportConcurrentCreateConnection;  // 888
    UInt8                  mNumberOfTimedOutHCICommands;        // 889
    UInt32                 mTransportSleepType;                 // 892

    bool mCanDoHardReset;   // 896
    bool mTransportPowerOn; // 897

    IOBluetoothHCIControllerPowerOptions mControllerPowerOptions;            // 900
    IOBluetoothHCIControllerConfigState  mControllerConfigState;             // 904
    IOBluetoothHCIControllerFeatureFlags mControllerFeatureFlags;            // 908
    UInt8                                mNumberOfCommandsAllowedByHardware; // 912
    bool                                 mWaitingForACLPackets;              // 913
    UInt8                                mNumSCOConnections;                 // 914
    UInt64                               mTotalSCOBytesSent;                 // 920
    UInt16                               mActiveConnections;                 // 928
    UInt8                                mNextNewSynchronousConnectionType;  // 930

    bool                    mNeedToGetCurrentUSBIsochFrameNumber; // 931
    UInt16                  unknown9;                             // 932, setted to 256
    UInt16                  mMaxPower;                            // 934
    UInt16                  unknownb;                             // 936
    UInt16                  unknownc;                             // 938
    BluetoothHCIVersionInfo mLocalVersionInfo;                    // 940

	UInt32 mVendorCommandSelector;  // 952
    UInt8 mNumConfiguredHIDDevices; // 956
    UInt8 unknownd;                 // 957
    bool  mSupportWoBT;             // 958
    bool  unknowne;                 // 959

    UInt32 unknownf;                               // 960
    UInt8  unknown10;                              // 964
    UInt8  unknown11;                              // 965
    UInt8  unknown12;                              // 966
    UInt8  unknown13;                              // 967
    bool   mIgnoreUSBResetCmdSent;                 // 968
    UInt8  unknown14;                              // 969
    bool   mSupportDeepIdle;                       // 970
    UInt8  unknown15;                              // 971
    bool   mWaitingForCompletedHCICommandsToSleep; // 972 calls transport completepowerstatechange

    int64_t mAppleBTLEAdvertisingReport[15];               // 976
    int64_t mTotalNumberofBTLEAdvertisingReportsReceived;  // 1096
    int64_t mTotalNumberOfTimesBluetoothIdleTimerExpired;  // 1104
    bool    mPowerReportersCreated;                        // 1112
    int64_t mLESetAdvertisingData[15];                     // 1120
    int64_t mTotalNumberOfLESetAdvertisingDataCommandSent; // 1240

    // Idle timer
    bool   mIdleTimerDisabled;        // 1248
    bool   mPowerStateChanging;       // 1249
    bool   mHandlingIdleTimeout;      // 1250
    UInt32 mPendingIdleTimerValue;    // 1252
    UInt32 mIdleTimerValue;           // 1256
    bool   mIdleTimerValueSet;        // 1260
    UInt32 mIdleTimerTime;            // 1264, the one functions actually use
    UInt32 mNVRAMIdleTimerValue;      // 1268
    bool   mSetNVRAMIdleTimerValue;   // 1272
	bool   mClearNVRAMIdleTimerValue; // 1273
    UInt32 mTransportIdleTimerValue;  // 1276
    bool   mSupportIdleTimer;         // 1280

    bool     mUpdatingFirmware;                   // 1281
    bool     mPendingRequestsState;               // 1282, processing = 1, kill = 0
    bool     mLEConnectionCompleteOutOfSequence;  // 1283
    UInt16   mControllerOutstandingCalls;         // 1284
    bool     mSystemNotReadyForSleep;             // 1286
    bool     mSupportDPLE;                        // 1287
    os_log_t mInternalOSLogObject;                // 1288
    bool     mAutoResumeSet;                      // 1296
    bool     mACLPacketCausedFullWake;            // 1297
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_11_0
    bool     mHardResetPerformed;                 // 1298
#endif
    UInt32   mHardResetCounter;                   // 1300
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_11_0
    bool     mHardResetDuringBoot;                // 1304
    bool     mSupportLighthouseFeature;           // 1305
    bool     mSendExitHIDSuspendPacket;           // 1306
    UInt8    mAllowedNumberOfTimedOutHCICommands; // 1307
    UInt32   mCreateLEDeviceCallTime;             // 1308
    bool     mBluetoothdNotFound;                 // 1312
#else
    bool     mHardResetPerformed;                 /// 1304
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_14
    UInt32   mCreateLEDeviceCallTime;             /// 1308
    bool     mHardResetDuringBoot;                /// 1312
#endif
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_15
    bool     mSupportLighthouseFeature;           /// 1313
    bool     mSendExitHIDSuspendPacket;           /// 1314
#endif
#endif

    struct ExpansionData
    {
        void * refCon;
    };
    ExpansionData * mIOBluetoothHostControllerExpansionData; // 1320
};

#endif
