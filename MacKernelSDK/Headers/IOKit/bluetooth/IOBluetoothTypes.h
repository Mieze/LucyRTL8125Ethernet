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
 *   @header IOBluetoothTypes.h
 *   This header defines IOReturn values specific to IOBluetooth.
 */

#pragma once

#import <IOKit/IOReturn.h>

// Error returns
#ifndef sub_iokit_bluetooth
#define sub_iokit_bluetooth err_sub(8)
#endif

#define iokit_bluetooth_err(return ) (sys_iokit | sub_iokit_bluetooth | return )

#define kIOBluetoothDeviceResetError           iokit_bluetooth_err(1) // Device reset interrupted pending operation
#define kIOBluetoothConnectionAlreadyExists    iokit_bluetooth_err(2) // Attempting to open a connection that already exists
#define kIOBluetoothNoHCIController            iokit_bluetooth_err(3) // No HCI controller is present
#define kIOBluetoothHCIPowerStatesNotSupported iokit_bluetooth_err(4) // HCI controller does not support changing power states
