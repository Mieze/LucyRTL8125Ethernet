/*
 * Copyright (c) 2021 Apple Inc. All rights reserved.
 * Copyright (c) 2021 cjiang.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */
/*!
 *   @header IOBluetoothInternal.h
 *   This header contains internal types and macros needed by public IOBluetooth kernel APIs.
 */

#pragma once

typedef enum IOBluetoothHCIControllerConfigState
{
    kIOBluetoothHCIControllerConfigStateOnline,                       // Controller is configured and ready for clients
    kIOBluetoothHCIControllerConfigStateKernelSetupPending,           // New controller - kernel setup in progress
    kIOBluetoothHCIControllerConfigStateDaemonSetupPending,           // New controller - daemon setup in progress
    kIOBluetoothHCIControllerConfigStateDaemonSetupComplete,          // New controller - daemon setup complete
    kIOBluetoothHCIControllerConfigStateResetPending,                 // HCI reset has been issued
    kIOBluetoothHCIControllerConfigStateKernelPostResetSetupPending,  // HCI reset complete - kernel setup in progress
    kIOBluetoothHCIControllerConfigStateDaemonPostResetSetupPending,  // HCI reset complete - daemon setup in progress
    kIOBluetoothHCIControllerConfigStateDaemonPostResetSetupComplete, // HCI reset complete - daemon setup complete
    kIOBluetoothHCIControllerConfigStateUninitialized
} IOBluetoothHCIControllerConfigState;

typedef UInt32 IOBluetoothHCIControllerPowerOptions;
enum
{
    kIOBluetoothHCIControllerPowerOptionPowerOffSupported                   = 0x00000001,
    kIOBluetoothHCIControllerPowerOptionWakeFromExistingConnectionSupported = 0x00000002,
    kIOBluetoothHCIControllerPowerOptionWakeFromNewConnectionSupported      = 0x00000004,
    kIOBluetoothHCIControllerPowerOptionIdleWithConnectionSupported         = 0x00000008,
    kIOBluetoothHCIControllerPowerOptionIdleWhenInternalPower               = 0x00000010
};

enum IOBluetoothHCIControllerSleepOptions
{
    kIOBluetoothHCIControllerAllowWakeFromExistingConnection = 0x00000001,
    kIOBluetoothHCIControllerAllowWakeFromNewConnection      = 0x00000002,
    kIOBluetoothHCIControllerTemporaryOnFromIdle             = 0x00000004
};

#define HCI_CONTROLLER_POWER_OFF_SUPPORTED(_controllerPowerOptions)                                                                                                                                    \
    ((_controllerPowerOptions & kIOBluetoothHCIControllerPowerOptionPowerOffSupported) == kIOBluetoothHCIControllerPowerOptionPowerOffSupported)
#define HCI_CONTROLLER_SLEEP_SUPPORTED(_controllerPowerOptions)                                                                                                                                        \
    ((_controllerPowerOptions & (kIOBluetoothHCIControllerPowerOptionWakeFromExistingConnectionSupported | kIOBluetoothHCIControllerPowerOptionWakeFromNewConnectionSupported)))

typedef enum IOBluetoothHCIControllerInternalPowerState
{
    kIOBluetoothHCIControllerInternalPowerStateOff   = 0,
    kIOBluetoothHCIControllerInternalPowerStateOn    = 1,
    kIOBluetoothHCIControllerInternalPowerStateSleep = 2,
    kIOBluetoothHCIControllerInternalPowerStateIdle  = 3
} IOBluetoothHCIControllerInternalPowerState;

enum IOBluetoothHCIControllerPowerStateOrdinal
{
    kIOBluetoothHCIControllerPowerStateOrdinalOff  = 0,
    kIOBluetoothHCIControllerPowerStateOrdinalIdle = 1,
    kIOBluetoothHCIControllerPowerStateOrdinalOn   = 2,
	kIOBluetoothHCIControllerPowerStateOrdinalCount
};

typedef UInt32 IOBluetoothHCIControllerFeatureFlags;
enum
{
    kIOBluetoothHCIControllerFeatureFlagVendorCommandFlowControl = 0x00000001,
    kIOBluetoothHCIControllerFeatureFlagSCOSupported             = 0x00000002,
    kIOBluetoothHCIControllerFeatureFlagSerializeCommands        = 0x10000000
};

typedef struct IOBluetoothHCIUnifiedInquiryResult
{
    BluetoothHCIInquiryResult originalInquiryResult;

    /*! @var eventCode
     *   This value should be kBluetoothHCIEventInquiryResult, kBluetoothHCIEventInquiryResultWithRSSI, or kBluetoothHCIEventExtendedInquiryResult. */

    BluetoothHCIEventCode eventCode;

    /*! @var extendedInquiryResponse
     *   This value should be kBluetoothHCIEventExtendedInquiryResult only. */

    BluetoothHCIExtendedInquiryResponse extendedInquiryResponse;

    UInt8 reserved;

    /*! @var RSSIValue
     *   This value should be kBluetoothHCIEventExtendedInquiryResult or kBluetoothHCIEventInquiryResultWithRSSI only. */

    BluetoothHCIRSSIValue RSSIValue;
} IOBluetoothHCIUnifiedInquiryResult;

enum BluetoothHCIExtendedInquiryResponseDataTypesAppleSpecificInfo
{
    kBluetoothHCIExtendedInquiryResponseDataTypeAppleSpecificInfoReserved              = 0x00,
    kBluetoothHCIExtendedInquiryResponseDataTypeAppleSpecificInfoModelIdentifier       = 0x01, /* UTF8String char data */
    kBluetoothHCIExtendedInquiryResponseDataTypeAppleSpecificInfoThirdPartyAdvertising = 0x02,
};

#define REQUIRE(err) os_log(OS_LOG_DEFAULT, "REQUIRE failure: %s - file: %s:%d\n", err, __FILE__, __LINE__)
#define CHECK(err) os_log(OS_LOG_DEFAULT, "CHECK failure: %s - file: %s:%d\n", err, __FILE__, __LINE__)
#define REQUIRE_NO_ERR(err) os_log(OS_LOG_DEFAULT, "REQUIRE_NO_ERR failure: 0x%x - file: %s:%d\n", err, __FILE__, __LINE__)

#define BluetoothFamilyLogPacket(family, type, format, ...) do {                                \
    _Static_assert(__builtin_constant_p(format), "format string must be constant");             \
    char * _pkt_log = (char *) IOMalloc(0x1FF);                                                 \
    if ( _pkt_log ) {                                                                           \
        bzero(_pkt_log, 0x1FF);                                                                 \
        snprintf(_pkt_log, 0x1FF, format, ##__VA_ARGS__);                                       \
        if (family) {                                                                           \
            family->LogPacket(type, _pkt_log, strlen(_pkt_log)); }                              \
        IOFree(_pkt_log, 0x1FF);                                                                \
    }                                                                                           \
    __asm__(""); /* avoid tailcall */                                                           \
} while (0)

#define OSLogAndLogPacket(log, family, type, format, ...) do {                                  \
    os_log(log, format, ##__VA_ARGS__);                                                         \
    BluetoothFamilyLogPacket(family, type, format, ##__VA_ARGS__);                              \
} while (0)
