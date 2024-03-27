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

#ifndef __IOKIT_IOBLUETOOTHHCIREQUEST_H
#define __IOKIT_IOBLUETOOTHHCIREQUEST_H

#include <IOKit/IOCommandGate.h>
#include <IOKit/IOService.h>
#include <IOKit/IOTimerEventSource.h>
#include <IOKit/bluetooth/IOBluetoothHostController.h>
#import <IOKit/system.h>

// Forward declaration to avoid the need to include IOBluetoothHCIUserLibShared.h
typedef struct BluetoothHCINotificationMessage BluetoothHCINotificationMessage;

enum
{
    kMaxHCIBufferLength = 512
};

typedef UInt8 BluetoothHCIRequestState;

/*! @enum        BluetoothHCIRequestStates
     @abstract    States of Bluetooth HCI Requests
     @constant    kHCIRequestStateIdle              Doing nothing - neither waiting nor busy. usually waiting for deletion.
     @constant    kHCIRequestStateWaiting        On the wait queue - request has not been processed in any way.
     @constant    kHCIRequestStateBusy            On the busy queue - request is sent and is currently processing
*/

enum BluetoothHCIRequestStates
{
    kHCIRequestStateIdle    = 0,
    kHCIRequestStateWaiting = 1,
    kHCIRequestStateBusy    = 2,
    kHCIRequestStateEnd
};

IOBLUETOOTH_EXPORT IOReturn ParseHCIEvent(UInt8 * inData, UInt32 inDataSize, UInt8 * outData, UInt32 * outDataSize, UInt8 * outStatus);
IOBLUETOOTH_EXPORT IOReturn DecodeHCICommandResult(UInt16 opCode, UInt8 * inData, UInt32 inDataSize, UInt8 * outData, UInt32 * outDataSize, UInt8 * outStatus);
IOBLUETOOTH_EXPORT IOReturn ParseLinkControlGroupCommand(UInt16 ocf, UInt8 * inData, UInt32 inDataSize, UInt8 * outData, UInt32 * outDataSize, UInt8 * outStatus);
IOBLUETOOTH_EXPORT IOReturn ParseLinkPolicyGroupCommand(UInt16 ocf, UInt8 * inData, UInt32 inDataSize, UInt8 * outData, UInt32 * outDataSize, UInt8 * outStatus);
IOBLUETOOTH_EXPORT IOReturn ParseHostControllerGroupCommand(UInt16 ocf, UInt8 * inData, UInt32 inDataSize, UInt8 * outData, UInt32 * outDataSize, UInt8 * outStatus);
IOBLUETOOTH_EXPORT IOReturn ParseInformationalGroupCommand(UInt16 ocf, UInt8 * inData, UInt32 inDataSize, UInt8 * outData, UInt32 * outDataSize, UInt8 * outStatus);
IOBLUETOOTH_EXPORT IOReturn ParseStatusGroupCommand(UInt16 ocf, UInt8 * inData, UInt32 inDataSize, UInt8 * outData, UInt32 * outDataSize, UInt8 * outStatus);
IOBLUETOOTH_EXPORT IOReturn ParseTestingGroupCommand(UInt16 ocf, UInt8 * inData, UInt32 inDataSize, UInt8 * outData, UInt32 * outDataSize, UInt8 * outStatus);
IOBLUETOOTH_EXPORT IOReturn ParseVendorSpecificCommand(UInt16 ocf, UInt8 * inData, UInt32 inDataSize, UInt8 * outData, UInt32 * outDataSize, UInt8 * outStatus);

class IOBluetoothHCIRequest : public OSObject
{
    OSDeclareDefaultStructors(IOBluetoothHCIRequest)

public:
    static IOBluetoothHCIRequest * Create(IOCommandGate * commandGate, IOBluetoothHostController * hostController, bool async = true, UInt32 timeout = 5, UInt32 controlFlags = 0);
    static IOReturn                Dispose(IOBluetoothHCIRequest * inObject);
    IOReturn                       DisposeRequest();

    bool         init(IOCommandGate * commandGate, IOBluetoothHostController * hostController);
    void         InitializeRequest();
    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual void retain() const APPLE_KEXT_OVERRIDE;
    void         RetainRequest(char * name);
    virtual void release() const APPLE_KEXT_OVERRIDE;
    void         ReleaseRequest(char * name);

    const char * RequestDescription(const char * name);

    IOReturn Start();    // Called when a request is started on a transport
    void     Complete(); // Called when a request is completed on a transport

    void SetState(BluetoothHCIRequestState inState);

    void SetCallbackInfo(BluetoothHCIRequestCallbackInfo * inInfo);
    void SetCallbackInfoToZero();

    void        SetResultsDataSize(IOByteCount inCount);
    void        SetResultsBufferPtrAndSize(UInt8 * resultsBuffer, IOByteCount inSize);
    void        CopyDataIntoResultsPtr(UInt8 * inDataPtr, IOByteCount inSize);
    UInt8 *     GetResultsBuffer();
    IOByteCount GetResultsBufferSize();

    BluetoothHCICommandOpCode GetCommandOpCode();

    void StartTimer();

    static void timerFired(OSObject * owner, IOTimerEventSource * sender);
    void        handleTimeout();

    UInt32 GetCurrentTime();
    void   SetStartTimeForDelete();

    bool CompareDeviceAddress(const BluetoothDeviceAddress * inDeviceAddress);

protected:
    UInt8                             mPrivateResultsBuffer[kMaxHCIBufferLength * 4]; // Just in case they didn't give a results ptr. 12
    IOByteCount                       mPrivateResultsSize;                            // Result size. 2064
    BluetoothHCITransportID           mTransportID;                                   // Transport ID to use for this request. 2072
    
public:
    BluetoothHCIRequestState          mState;                                         // Busy, waiting, idle. 2076
    bool                              mAsyncNotify;                                   // 2077
    task_t                            mOwningTaskID;                                  // 2080
    BluetoothHCIRequestCallbackInfo   mCallbackInfo;                                  // When this request is complete, call this. 2088
    BluetoothHCICommandOpCode         mOpCode;                                        // 2128
    BluetoothDeviceAddress            mDeviceAddress;                                 // 2130
    BluetoothConnectionHandle         mConnectionHandle;                              // 2136
    BluetoothHCINotificationMessage * mNotificationMessage;                           // 2144
    IOByteCount                       mNotificationMessageSize;                       // 2152

    IOBluetoothHCIRequest * mNextBusy;                           // Points to next request element on busy queue. 2160
    IOBluetoothHCIRequest * mNextWait;                           // Points to next request element on wait queue. 2168
    IOBluetoothHCIRequest * mNextAllocated;                      // Points to next allocated request element. 2176
    IOBluetoothHCIRequest * mPreviousAllocated;                  // Points to next allocated request element. 2184
    BluetoothHCIRequestID   mID;                                 // For internal identification. 2192
    UInt8                   mCommandBuffer[kMaxHCIBufferLength]; // Built-up HCI Command to send to the transport. 2196
    IOByteCount             mCommandBufferSize;                  // Size of command buffer. 2712

    UInt8 *     mResultsPtr;  // Result ptr, provided by object creator. 2720
    IOByteCount mResultsSize; // Result size. 2728

    IOCommandGate *             mCommandGate;    // 2736
    IOTimerEventSource *        mTimer;          // 2744
    IOBluetoothHostController * mHostController; // 2752

    IOReturn mStatus;                    // Success/failure code of request. 2760
    UInt32   mTimeout;                   // Timeout for request to complete, in milliseconds. 2764
    UInt32   mOriginalTimeout;           // 2768
    UInt32   mControlFlags;              // 2772
    int      mPID;                       // Creating Task 2776
    bool     mHCIRequestDeleteWasCalled; // Fixed rdar://problem/7044168 2780
    UInt32   mStartTimeForDelete;        // 2784

    // Require and Receive Events

    bool unknown; // 2788
    UInt32 mExpectedEvent;                          // 2792
    UInt8  mNumberOfExpectedExplicitCompleteEvents; // 2796
    UInt32 mExpectedExplicitCompleteEvents[5];      // 2800

    UInt32 mReceivedEvent;                      // 2820
    UInt8  mExplictCompleteEvent;               // 2824
    UInt32 mNumberOfPossibleIntermediateEvents; // 2828
    UInt32 mPossibleIntermediateEvents[5];      // 2832
    UInt32 mReceivedIntermediateEvents[5];      // 2852
};

#endif
