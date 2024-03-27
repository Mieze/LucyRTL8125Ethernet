/*
 * Copyright (c) 2015-2021 Apple Inc. All rights reserved.
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

#ifndef _SKYWALK_OS_CHANNEL_H_
#define _SKYWALK_OS_CHANNEL_H_

#include <stdint.h>
#include <sys/types.h>
#include <sys/cdefs.h>
#include <uuid/uuid.h>
#include <mach/vm_types.h>
#include <skywalk/nexus/os_nexus.h>
#include <skywalk/packet/os_packet.h>
#include <skywalk/channel/os_channel_event.h>
#include <net/if_var.h>

/*
 * Indicates that channel supports "CHANNEL_ATTR_NUM_BUFFERS" attribute.
 * used by Libnetcore.
 */
#define OS_CHANNEL_HAS_NUM_BUFFERS_ATTR 1

/* Flow advisory table index */
typedef uint32_t flowadv_idx_t;
#define FLOWADV_IDX_NONE                ((flowadv_idx_t)-1)

/*
 * Channel ring direction.
 */
typedef enum {
	CHANNEL_DIR_TX_RX,      /* default: TX and RX ring(s) */
	CHANNEL_DIR_TX,         /* (monitor) only TX ring(s) */
	CHANNEL_DIR_RX          /* (monitor) only RX ring(s) */
} ring_dir_t;

/*
 * Channel ring ID.
 */
typedef uint32_t ring_id_t;
#define CHANNEL_RING_ID_ANY             ((ring_id_t)-1)

typedef enum {
	CHANNEL_FIRST_TX_RING,
	CHANNEL_LAST_TX_RING,
	CHANNEL_FIRST_RX_RING,
	CHANNEL_LAST_RX_RING
} ring_id_type_t;

/* Sync mode values */
typedef enum {
	CHANNEL_SYNC_TX,        /* synchronize TX ring(s) */
	CHANNEL_SYNC_RX,        /* synchronize RX ring(s) */
#if defined(LIBSYSCALL_INTERFACE) || defined(BSD_KERNEL_PRIVATE)
	CHANNEL_SYNC_UPP        /* synchronize packet pool rings only */
#endif /* LIBSYSCALL_INTERFACE || BSD_KERNEL_PRIVATE */
} sync_mode_t;

/* Sync flags */
typedef uint32_t sync_flags_t;
#if defined(LIBSYSCALL_INTERFACE) || defined(BSD_KERNEL_PRIVATE)
#define CHANNEL_SYNCF_ALLOC        0x1     /* synchronize alloc ring */
#define CHANNEL_SYNCF_FREE         0x2     /* synchronize free ring */
#define CHANNEL_SYNCF_PURGE        0x4     /* purge user packet pool */
#define CHANNEL_SYNCF_ALLOC_BUF    0x8     /* synchronize buflet alloc ring */
#endif /* LIBSYSCALL_INTERFACE || BSD_KERNEL_PRIVATE */

/*
 * Opaque handles.
 */
struct channel;
struct channel_ring_desc;
struct __slot_desc;
struct channel_attr;

typedef struct channel                  *channel_t;
typedef struct channel_ring_desc        *channel_ring_t;
typedef struct __slot_desc              *channel_slot_t;
typedef struct channel_attr             *channel_attr_t;

/*
 * Channel monitor types.
 */
typedef enum {
	CHANNEL_MONITOR_OFF,            /* default */
	CHANNEL_MONITOR_NO_COPY,        /* zero-copy (delayed) mode */
	CHANNEL_MONITOR_COPY            /* copy (immediate) mode */
} channel_monitor_type_t;

/*
 * Channel threshold unit types.
 */
typedef enum {
	CHANNEL_THRESHOLD_UNIT_SLOTS,   /* unit in slots (default) */
	CHANNEL_THRESHOLD_UNIT_BYTES,   /* unit in bytes */
} channel_threshold_unit_t;

/*
 * Channel attribute types gettable/settable via os_channel_attr_{get,set}.
 *
 *     g: retrievable at any time
 *     s: settable at any time
 *     S: settable once, only at creation time
 */
typedef enum {
	CHANNEL_ATTR_TX_RINGS,          /* (g) # of transmit rings */
	CHANNEL_ATTR_RX_RINGS,          /* (g) # of receive rings */
	CHANNEL_ATTR_TX_SLOTS,          /* (g) # of slots per transmit ring */
	CHANNEL_ATTR_RX_SLOTS,          /* (g) # of slots per receive ring */
	CHANNEL_ATTR_SLOT_BUF_SIZE,     /* (g) buffer per slot (bytes) */
	CHANNEL_ATTR_SLOT_META_SIZE,    /* (g) metadata per slot (bytes) */
	CHANNEL_ATTR_EXCLUSIVE,         /* (g/s) bool: exclusive open */
	CHANNEL_ATTR_NO_AUTO_SYNC,      /* (g/s) bool: will do explicit sync */
	CHANNEL_ATTR_MONITOR,           /* (g/s) see channel_monitor_type_t */
	CHANNEL_ATTR_TX_LOWAT_UNIT,     /* (g/s) see channel_threshold_unit_t */
	CHANNEL_ATTR_TX_LOWAT_VALUE,    /* (g/s) transmit low-watermark */
	CHANNEL_ATTR_RX_LOWAT_UNIT,     /* (g/s) see channel_threshold_unit_t */
	CHANNEL_ATTR_RX_LOWAT_VALUE,    /* (g/s) receive low-watermark */
	CHANNEL_ATTR_NEXUS_TYPE,        /* (g) nexus type */
	CHANNEL_ATTR_NEXUS_EXTENSIONS,  /* (g) nexus extension(s) */
	CHANNEL_ATTR_NEXUS_MHINTS,      /* (g) nexus memory usage hints */
	CHANNEL_ATTR_TX_HOST_RINGS,     /* (g) # of transmit host rings */
	CHANNEL_ATTR_RX_HOST_RINGS,     /* (g) # of receive host rings */
	CHANNEL_ATTR_NEXUS_IFINDEX,     /* (g) nexus network interface index */
	CHANNEL_ATTR_NEXUS_STATS_SIZE,  /* (g) nexus statistics region size */
	CHANNEL_ATTR_NEXUS_FLOWADV_MAX, /* (g) # of flow advisory entries */
	CHANNEL_ATTR_NEXUS_META_TYPE,   /* (g) nexus metadata type */
	CHANNEL_ATTR_NEXUS_META_SUBTYPE, /* (g) nexus metadata subtype */
	CHANNEL_ATTR_NEXUS_CHECKSUM_OFFLOAD, /* (g) nexus checksum offload */
	CHANNEL_ATTR_USER_PACKET_POOL,  /* (g/s) bool: use user packet pool */
	CHANNEL_ATTR_NEXUS_ADV_SIZE,    /* (g) nexus advisory region size */
	CHANNEL_ATTR_NEXUS_DEFUNCT_OK,  /* (g/s) bool: allow defunct */
	CHANNEL_ATTR_FILTER,            /* (g/s) bool: filter mode */
	CHANNEL_ATTR_EVENT_RING,        /* (g/s) bool: enable event ring */
	CHANNEL_ATTR_MAX_FRAGS,         /* (g) max length of buflet chain */
	CHANNEL_ATTR_NUM_BUFFERS,       /* (g) # of buffers in user pool */
	CHANNEL_ATTR_LOW_LATENCY,       /* (g/s) bool: low latency channel */
} channel_attr_type_t;

/*
 * Channel nexus metadata type.
 */
typedef enum {
	CHANNEL_NEXUS_META_TYPE_INVALID = 0,
	CHANNEL_NEXUS_META_TYPE_QUANTUM, /* OK for os_packet quantum APIs */
	CHANNEL_NEXUS_META_TYPE_PACKET,  /* OK for all os_packet APIs */
} channel_nexus_meta_type_t;

/*
 * Channel nexus metadata subtype.
 */
typedef enum {
	CHANNEL_NEXUS_META_SUBTYPE_INVALID = 0,
	CHANNEL_NEXUS_META_SUBTYPE_PAYLOAD,
	CHANNEL_NEXUS_META_SUBTYPE_RAW,
} channel_nexus_meta_subtype_t;

/*
 * Valid values for CHANNEL_ATTR_NEXUS_CHECKSUM_OFFLOAD
 */
#define CHANNEL_NEXUS_CHECKSUM_PARTIAL  0x1     /* partial checksum */

/*
 * Channel statistics ID.
 */
typedef enum {
	CHANNEL_STATS_ID_IP = 0,        /* struct ip_stats */
	CHANNEL_STATS_ID_IP6,           /* struct ip6_stats */
	CHANNEL_STATS_ID_TCP,           /* struct tcp_stats */
	CHANNEL_STATS_ID_UDP,           /* struct udp_stats */
	CHANNEL_STATS_ID_QUIC,          /* struct quic_stats */
} channel_stats_id_t;

/*
 * Slot properties.  Structure is aligned to allow for efficient copy.
 *
 * Fields except for sp_{flags,len} are immutables (I).  The system will
 * verify for correctness during os_channel_put() across the immutable
 * fields, and will abort the process if it detects inconsistencies.
 * This is meant to help with debugging, since it indicates bugs and/or
 * memory corruption.
 */
typedef struct slot_prop {
	uint16_t sp_flags;              /* private flags */
	uint16_t sp_len;                /* length for this slot */
	uint32_t sp_idx;                /* (I) slot index */
	mach_vm_address_t sp_ext_ptr;   /* (I) pointer for indirect buffer */
	mach_vm_address_t sp_buf_ptr;   /* (I) pointer for native buffer */
	mach_vm_address_t sp_mdata_ptr; /* (I) pointer for metadata */
	uint32_t _sp_pad[8];            /* reserved */
} slot_prop_t __attribute__((aligned(sizeof(uint64_t))));

/*
 * Kernel APIs.
 */

/*
 * Opaque handles.
 */
struct kern_channel;
struct __kern_channel_ring;

typedef struct kern_channel             *kern_channel_t;
typedef struct __kern_channel_ring      *kern_channel_ring_t;
typedef struct __slot_desc              *kern_channel_slot_t;

/*
 * Slot properties (deprecated).
 */
struct kern_slot_prop {
	uint32_t _sp_pad[16];           /* reserved */
} __attribute__((aligned(sizeof(uint64_t))));

/*
 * @struct kern_channel_ring_stat_increment
 * @abstract Structure used to increment the per ring statistic counters.
 * @field kcrsi_slots_transferred  number of slots transferred
 * @filed kcrsi_bytes_transferred  number of bytes transferred
 */
struct kern_channel_ring_stat_increment {
	uint32_t        kcrsi_slots_transferred;
	uint32_t        kcrsi_bytes_transferred;
};

/*
 * Data Movement APIs.
 *
 * See block comment above for userland data movement APIs for general
 * concepts.  The main differences here are the kern_channel_notify()
 * and kern_channel_reclaim() calls that aren't available for userland.
 * These calls are typically invoked within the TX and RX sync callbacks
 * implemented by the nexus provider.
 *
 * For TX sync, kern_channel_reclaim() is normally called after the
 * provider has finished reclaiming slots that have been "transmitted".
 * In this case, this call is simply a way to indicate to the system
 * that such condition has happened.
 *
 * For RX sync, kern_channel_reclaim() must be called at the beginning
 * of the callback in order to reclaim user-released slots, and to
 * ensure that subsequent calls to kern_channel_available_slot_count()
 * or kern_channel_get_next_slot() operates on the most recent state.
 *
 * The kern_channel_notify() is used to post notifications to indicate
 * slot availability; this may result in the kernel event subsystem
 * posting readable and writable events.
 */
__BEGIN_DECLS
extern uint32_t kern_channel_notify(const kern_channel_ring_t, uint32_t flags);
extern uint32_t kern_channel_available_slot_count(
	const kern_channel_ring_t ring);
/*
 * NOTE: kern_channel_set_slot_properties(), kern_channel_get_next_slot(),
 * kern_channel_reclaim() and kern_channel_advance_slot() require that the
 * caller invokes them from within the sync callback context; they will
 * assert otherwise.
 */
extern void kern_channel_set_slot_properties(const kern_channel_ring_t,
    const kern_channel_slot_t slot, const struct kern_slot_prop *prop);
extern kern_channel_slot_t kern_channel_get_next_slot(
	const kern_channel_ring_t kring, const kern_channel_slot_t slot,
	struct kern_slot_prop *slot_prop);
extern uint32_t kern_channel_reclaim(const kern_channel_ring_t);
extern void kern_channel_advance_slot(const kern_channel_ring_t kring,
    kern_channel_slot_t slot);

/*
 * Packet.
 */
extern kern_packet_t kern_channel_slot_get_packet(
	const kern_channel_ring_t ring, const kern_channel_slot_t slot);

/*
 * NOTE: kern_channel_slot_attach_packet(), kern_channel_slot_detach_packet()
 * and kern_channel_ring_get_container() require that the caller invokes them
 * from within the sync callback context; they will assert otherwise.
 */
extern errno_t kern_channel_slot_attach_packet(const kern_channel_ring_t ring,
    const kern_channel_slot_t slot, kern_packet_t packet);
extern errno_t kern_channel_slot_detach_packet(const kern_channel_ring_t ring,
    const kern_channel_slot_t slot, kern_packet_t packet);
extern errno_t kern_channel_ring_get_container(const kern_channel_ring_t ring,
    kern_packet_t **array, uint32_t *count);
extern errno_t kern_channel_tx_refill(const kern_channel_ring_t ring,
    uint32_t pkt_limit, uint32_t byte_limit, boolean_t tx_doorbell_ctxt,
    boolean_t *pkts_pending);
extern errno_t kern_channel_get_service_class(const kern_channel_ring_t ring,
    kern_packet_svc_class_t *svc);
extern errno_t kern_netif_queue_get_service_class(kern_netif_queue_t,
    kern_packet_svc_class_t *);

/*
 * Misc.
 */
extern void *kern_channel_get_context(const kern_channel_t channel);
extern void *kern_channel_ring_get_context(const kern_channel_ring_t ring);
extern void *kern_channel_slot_get_context(const kern_channel_ring_t ring,
    const kern_channel_slot_t slot);

/*
 * NOTE: kern_channel_increment_ring_{net}_stats() requires
 * that the caller invokes it from within the sync callback context;
 * it will assert otherwise.
 */
extern void kern_channel_increment_ring_stats(kern_channel_ring_t ring,
    struct kern_channel_ring_stat_increment *stats);
extern void kern_channel_increment_ring_net_stats(kern_channel_ring_t ring,
    ifnet_t, struct kern_channel_ring_stat_increment *stats);

__END_DECLS
#endif /* !_SKYWALK_OS_CHANNEL_H_ */
