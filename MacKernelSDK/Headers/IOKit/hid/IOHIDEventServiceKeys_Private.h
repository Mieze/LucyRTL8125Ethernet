/*
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * Copyright (c) 2019 Apple Computer, Inc.  All Rights Reserved.
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef IOHIDEventServiceKeys_Private_h
#define IOHIDEventServiceKeys_Private_h


/*!
 * @define kIOHIDMaxEventSizeKey
 *
 * @abstract
 * Number property that describes the max event size in bytes that the event
 * service will dispatch. If this property is not provided, the default max
 * size will be set to 4096 bytes.
 */
#define kIOHIDMaxEventSizeKey "MaxEventSize"

/*!
 * @define kIOHIDMaxReportSizeKey
 *
 * @abstract
 * Number property that describes the max report size in bytes that the device
 * will dispatch. If this property is not provided, the
 * kIOHIDMaxInputReportSizeKey key will be used.
 */
#define kIOHIDMaxReportSizeKey "MaxReportSize"

/*!
 * @define kIOHIDReportPoolSizeKey
 *
 * @abstract
 * Number property that describes the amount of report buffers that should
 * be created for handling reports. If this property is not provided, the
 * default value of 1 will be used.
 */
#define kIOHIDReportPoolSizeKey "ReportPoolSize"


#endif /* IOHIDEventServiceKeys_Private_h */
