/*
 * Copyright (c) 2021 Apple Inc. All rights reserved.
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
 *   @header IOBluetoothHIDDriverTypes.h
 *   This header contains enumerations for IOBluetoothHIDDriver.
 */

#pragma once

// Vendor ID Sources
enum
{
    kVendorIDSourceBluetoothSIG = 0x1,
    kVendorIDSourceUSBIF        = 0x2
};

// Bluetooth HID Transaction Headers
enum
{
    kBluetoothHIDTransactionHandshakeHeader   = 0x00,
    kBluetoothHIDTransactionHIDControlHeader  = 0x10,
    kBluetoothHIDTransactionGetReportHeader   = 0x40,
    kBluetoothHIDTransactionSetReportHeader   = 0x50,
    kBluetoothHIDTransactionGetProtocolHeader = 0x60,
    kBluetoothHIDTransactionSetProtocolHeader = 0x70,
    kBluetoothHIDTransactionGetIdleHeader     = 0x80,
    kBluetoothHIDTransactionSetIdleHeader     = 0x90,
    kBluetoothHIDTransactionDataHeader        = 0xA0,
    kBluetoothHIDTransactionDatcHeader        = 0xB0
};

// Handshake Types
enum
{
    kBluetoothHandshakeTypeSuccessful         = 0x0,
    kBluetoothHandshakeTypeNotReady           = 0x1,
    kBluetoothHandshakeTypeInvalidReportID    = 0x2,
    kBluetoothHandshakeTypeUnsupportedRequest = 0x3,
    kBluetoothHandshakeTypeInvalidParameter   = 0x4,
    kBluetoothHandshakeTypeErrorUnknown       = 0xD,
    kBluetoothHandshakeTypeErrorFatal         = 0xF
};

// HID Control Types
enum
{
    kBluetoothHIDControlTypeNoOperation = 0x0,
    kBluetoothHIDControlTypeHardReset   = 0x1,
    kBluetoothHIDControlTypeSoftRest    = 0x2,
    kBluetoothHIDControlTypeSuspend     = 0x3,
    kBluetoothHIDControlTypeExitSuspend = 0x4,
    kBluetoothHIDControlTypeVCUnplug    = 0x5
};

// Report Types
enum
{
    kBluetoothReportTypeReserved = 0x0,
    kBluetoothReportTypeOther    = 0x0,
    kBluetoothReportTypeInput    = 0x1,
    kBluetoothReportTypeOutput   = 0x2,
    kBluetoothReportTypeFeature  = 0x3
};

// Protocol Types
enum
{
    kBluetoothProtocolTypeBoot   = 0x0,
    kBluetoothProtocolTypeReport = 0x1
};
