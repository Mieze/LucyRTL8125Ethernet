/*
 * Copyright (c) 2019 Apple Inc. All rights reserved.
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

#ifndef _SKYWALK_OS_CHANNEL_EVENT_H_
#define _SKYWALK_OS_CHANNEL_EVENT_H_

#include <stdint.h>
#include <mach/vm_types.h>

typedef enum {
	CHANNEL_EVENT_PACKET_TRANSMIT_STATUS = 1,
	CHANNEL_EVENT_MIN    = CHANNEL_EVENT_PACKET_TRANSMIT_STATUS,
	CHANNEL_EVENT_MAX    = CHANNEL_EVENT_PACKET_TRANSMIT_STATUS,
} os_channel_event_type_t;

typedef enum {
	CHANNEL_EVENT_SUCCESS = 0,
	CHANNEL_EVENT_PKT_TRANSMIT_STATUS_ERR_FLUSH = 1,
	CHANNEL_EVENT_PKT_TRANSMIT_STATUS_ERR_RETRY_FAILED = 2,
} os_channel_event_error_t;

typedef struct os_channel_event_packet_transmit_status {
	packet_id_t    packet_id;
	int32_t        packet_status;
} os_channel_event_packet_transmit_status_t;

/*
 * opaque handles
 */
typedef uint64_t             os_channel_event_handle_t;
typedef mach_vm_address_t    os_channel_event_t;

struct os_channel_event_data {
	os_channel_event_type_t    event_type;
	boolean_t                  event_more;
	uint16_t                   event_data_length;
	uint8_t                    *event_data;
};

#endif /* !_SKYWALK_OS_CHANNEL_EVENT_H_ */
