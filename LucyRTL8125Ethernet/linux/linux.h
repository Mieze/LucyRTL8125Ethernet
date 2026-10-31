/* linux.h -- Definitions to make the linux code compile under OS X.
 *
 * Copyright (c) 2014 Laura Müller <laura-mueller@uni-duesseldorf.de>
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * Driver Intel Gigabit PCIe ethernet controllers.
 *
 * This driver is based on Intel's E1000e driver for Linux.
 */

#ifndef ixgbe_linux_h
#define ixgbe_linux_h

#include <IOKit/IOLib.h>

/******************************************************************************/
#pragma mark -
#pragma mark Debugging
#pragma mark -
/******************************************************************************/

#define printk(args...) IOLog(args)
/******************************************************************************/
#pragma mark -
#pragma mark Bits and Bytes
#pragma mark -
/******************************************************************************/

#define HZ 1000 // Milliseconds.

#if defined(__LITTLE_ENDIAN__)
#define __LITTLE_ENDIAN 1234
#define __LITTLE_ENDIAN_BITFIELD

#elif defined(__BIG_ENDIAN__)
#define __BIG_ENDIAN 4321
#define __BIG_ENDIAN_BITFIELD

#endif // ENDIAN

#define u8      UInt8
#define u16     UInt16
#define u32     UInt32
#define u64     UInt64
#define s8      SInt8
#define s16     SInt16
#define s32     SInt32
#define s64     SInt64
#define __be16  SInt16
#define __be32  SInt32
#define __be64  SInt64
#define __le16  SInt16
#define __le32  SInt32
#define __le64  SInt64
#define __s8    SInt8
#define __s16   SInt16
#define __s32   SInt32
#define __s64   SInt64
#define __u8    UInt8
#define __u16   UInt16
#define __u32   UInt32
#define __u64   UInt64

#define __sum16 UInt16

#define ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#define ALIGN(x, a)         ALIGN_MASK(x, (typeof(x))(a) - 1)

#define cpu_to_le16(x) OSSwapHostToLittleInt16(x)
#define cpu_to_le32(x) OSSwapHostToLittleInt32(x)
#define cpu_to_le64(x) OSSwapHostToLittleInt64(x)
#define le16_to_cpu(x) OSSwapLittleToHostInt16(x)
#define le32_to_cpu(x) OSSwapLittleToHostInt32(x)
#define le64_to_cpu(x) OSSwapLittleToHostInt64(x)

#define cpu_to_be16(x) OSSwapHostToBigInt16(x)
#define cpu_to_be32(x) OSSwapHostToBigInt32(x)
#define cpu_to_be64(x) OSSwapHostToBigInt64(x)
#define be16_to_cpu(x) OSSwapBigToHostInt16(x)
#define be32_to_cpu(x) OSSwapBigToHostInt32(x)
#define be64_to_cpu(x) OSSwapBigToHostInt64(x)

#define le16_to_cpus(x) ((*x) = OSSwapLittleToHostInt16((*x)))
#define le32_to_cpus(x) ((*x) = OSSwapLittleToHostInt32((*x)))
#define le64_to_cpus(x) ((*x) = OSSwapLittleToHostInt64((*x)))

#define container_of(ptr, type, member) ({                                     \
const typeof( ((type *)0)->member ) *__mptr = (ptr);                       \
(type *)( (char *)__mptr - offsetof(type,member) );})

#define BITS_PER_LONG           LONG_BIT
#define BIT(nr)                 (1U << (nr))
#define BIT_ULL(nr)             (1ULL << (nr))
#define BIT_WORD(nr)            ((nr) / BITS_PER_LONG)
#define BITS_PER_BYTE           8
#define BITS_TO_LONGS(bits)     (((bits)+BITS_PER_LONG-1)/BITS_PER_LONG)
#define GENMASK_INPUT_CHECK(h, l) 0

#define ARRAY_SIZE(x)           (sizeof(x) / sizeof((x)[0]))

#define READ_ONCE(x) (u8 __iomem *)(x)

#define min_t(type,x,y) \
({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })

#define max_t(type, x, y) \
({ type __x = (x); type __y = (y); __x > __y ? __x: __y; })

#if 0
enum bool_t
{
    false = 0,
    true = 1
};
typedef enum bool_t bool;
#endif

#define dma_addr_t  IOPhysicalAddress64

#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)

static inline int atomic_dec_and_test(volatile SInt32 * addr)
{
    return ((OSDecrementAtomic(addr) == 1) ? 1 : 0);
}

static inline int atomic_inc_and_test(volatile SInt32 * addr)
{
    return ((OSIncrementAtomic(addr) == -1) ? 1 : 0);
}

#define atomic_inc(v) OSIncrementAtomic(v)
#define atomic_dec(v) OSDecrementAtomic(v)

static inline bool
test_mask(unsigned int mask, const volatile unsigned int *addr)
{
    return ((mask & *addr) == mask);
}

static inline void set_mask(unsigned int mask, volatile unsigned int *addr)
{
    OSBitOrAtomic(mask, (volatile UInt32 *)addr);
}

static inline void clear_mask(unsigned int mask, const volatile unsigned int *addr)
{
    OSBitAndAtomic(~mask, (volatile UInt32 *)addr);
}

static inline int
test_bit(unsigned int nr, const volatile unsigned int *addr)
{
    return (*addr & (1 << nr));
}

static inline void set_bit(unsigned int nr, volatile unsigned int *addr)
{
    OSBitOrAtomic((1 << nr), (volatile UInt32 *)addr);
}

static inline void clear_bit(unsigned int nr, volatile unsigned int *addr)
{
    OSBitAndAtomic(~(1 << nr), (volatile UInt32 *)addr);
}

static inline int test_and_clear_bit(unsigned int nr, volatile unsigned int *addr)
{
    unsigned int mask = (1 << nr);
    
    return (mask & OSBitAndAtomic(~mask, (volatile UInt32 *)addr));
}

static inline int test_and_set_bit(unsigned int nr, volatile unsigned int *addr)
{
    unsigned int mask = (1 << nr);

    return (mask & OSBitOrAtomic(mask, (volatile UInt32 *)addr));
}

/******************************************************************************/
#pragma mark -
#pragma mark Read/Write Registers
#pragma mark -
/******************************************************************************/

OS_INLINE
void
_OSWriteInt8(
             volatile void               * base,
             uintptr_t                     byteOffset,
             uint16_t                      data
             )
{
    *(volatile uint8_t *)((uintptr_t)base + byteOffset) = data;
}

OS_INLINE
uint8_t
_OSReadInt8(
            const volatile void               * base,
            uintptr_t                     byteOffset
            )
{
    return *(volatile uint8_t *)((uintptr_t)base + byteOffset);
}

#define OSWriteLittleInt8(base, byteOffset, data) \
_OSWriteInt8((base), (byteOffset), (data))
#define OSReadLittleInt8(base, byteOffset) \
_OSReadInt8((base), (byteOffset))
/*
#define writew(reg, val16)     OSWriteLittleInt16((hw->hw_addr), (reg), (val16))
#define writel(val32, reg)     OSWriteLittleInt32((hw->hw_addr), (reg), (val32))

#define readw(hw, reg)      OSReadLittleInt16((hw->hw_addr), (reg))
#define readl(hw, reg)      OSReadLittleInt32((hw->hw_addr), (reg))
*/
/******************************************************************************/
#pragma mark -
#pragma mark Locks
#pragma mark -
/******************************************************************************/

#define spinlock_t  IOSimpleLock *
#define atomic_t    volatile SInt32

#define spin_lock(lock)

#define spin_unlock(lock)

#define spin_lock_irqsave(lock,flags)

#define spin_trylock_irqsave(lock,flags)

#define spin_unlock_irqrestore(lock,flags)

#define usec_delay(x)           IODelay(x)
#define msec_delay(x)           IOSleep(x)
#define udelay(x)               IODelay((unsigned int)(x))
#define mdelay(x)               IODelay(1000*(x))
#define msleep(x)               IOSleep(x)

#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

static inline void usleep_range(unsigned long min, unsigned long max)
{
    uint64_t timeout;
    uint64_t now;
    
    nanoseconds_to_absolutetime(((min + max) * 500), &timeout);
    clock_get_uptime(&now);
    clock_delay_until(now + timeout);
}

#define USEC_PER_MSEC   1000L
#define TICK_NSEC       4000000UL
static const unsigned int max_slack_shift = 2;
#define USLEEP_RANGE_UPPER_BOUND    ((TICK_NSEC << max_slack_shift) / NSEC_PER_USEC)

/**
 * fsleep - flexible sleep which autoselects the best mechanism
 * @usecs:    requested sleep duration in microseconds
 *
 * flseep() selects the best mechanism that will provide maximum 25% slack
 * to the requested sleep duration. Therefore it uses:
 *
 * * udelay() loop for sleep durations <= 10 microseconds to avoid hrtimer
 *   overhead for really short sleep durations.
 * * usleep_range() for sleep durations which would lead with the usage of
 *   msleep() to a slack larger than 25%. This depends on the granularity of
 *   jiffies.
 * * msleep() for all other sleep durations.
 *
 * Note: When %CONFIG_HIGH_RES_TIMERS is not set, all sleeps are processed with
 * the granularity of jiffies and the slack might exceed 25% especially for
 * short sleep durations.
 */
static inline void fsleep(unsigned long usecs)
{
    if (usecs <= 10)
        udelay(usecs);
    else if (usecs < USLEEP_RANGE_UPPER_BOUND)
        usleep_range(usecs, usecs + (usecs >> max_slack_shift));
    else
        msleep(DIV_ROUND_UP((unsigned int)usecs, USEC_PER_MSEC));
}

enum
{
    GFP_KERNEL,
    GFP_ATOMIC,
};

#define __iomem volatile
#define __devinit

#define LINUX_VERSION_CODE 30000
#define KERNEL_VERSION(x,y,z) (x*10000+100*y+z)

#define irqreturn_t int

#define WARN_ON_ONCE(x)

#define __always_unused

#define DISABLED_CODE 0

struct pci_dev {
    UInt16 vendor;
    UInt16 device;
    UInt16 subsystem_vendor;
    UInt16 subsystem_device;
    UInt8 revision;
};

#define dev_err(x,y,z)

/**
 * is_zero_ether_addr - Determine if give Ethernet address is all zeros.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return true if the address is all zeroes.
 *
 * Please note: addr must be aligned to u16.
 */
static inline bool is_zero_ether_addr(const u8 *addr)
{
#if defined(CONFIG_HAVE_EFFICIENT_UNALIGNED_ACCESS)
	return ((*(const u32 *)addr) | (*(const u16 *)(addr + 4))) == 0;
#else
	return (*(const u16 *)(addr + 0) |
            *(const u16 *)(addr + 2) |
            *(const u16 *)(addr + 4)) == 0;
#endif
}

/**
 * is_multicast_ether_addr - Determine if the Ethernet address is a multicast.
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Return true if the address is a multicast address.
 * By definition the broadcast address is also a multicast address.
 */
static inline bool is_multicast_ether_addr(const u8 *addr)
{
	return 0x01 & addr[0];
}

/**
 * is_valid_ether_addr - Determine if the given Ethernet address is valid
 * @addr: Pointer to a six-byte array containing the Ethernet address
 *
 * Check that the Ethernet address (MAC) is not 00:00:00:00:00:00, is not
 * a multicast address, and is not FF:FF:FF:FF:FF:FF.
 *
 * Return true if the address is valid.
 *
 * Please note: addr must be aligned to u16.
 */
static inline bool is_valid_ether_addr(const u8 *addr)
{
	/* FF:FF:FF:FF:FF:FF is a multicast address so we don't need to
	 * explicitly check for it here. */
	return !is_multicast_ether_addr(addr) && !is_zero_ether_addr(addr);
}


#define hw_dbg(hw, format, arg...)
#define hw_err(hw, format, arg...)
#define e_dev_info(format, arg...)
#define e_dev_warn(format, arg...)
#define e_dev_err(format, arg...)
#define e_dev_notice(format, arg...)
#define e_info(msglvl, format, arg...)
#define e_err(msglvl, format, arg...)
#define e_warn(msglvl, format, arg...)
#define e_crit(msglvl, format, arg...)

#define	DEFINE_MUTEX(x)	void x##_dummy(){}
#define	mutex_lock(x)
#define	mutex_unlock(x)

#define net_device rtl8125_private
#define netdev_priv(x)  ((struct rtl8125_private *)x)

#define do_div(lat_ns, speed) \
(lat_ns) = (UInt64)(lat_ns) / (speed)

#define ether_crc_le(length, data) _kc_ether_crc_le(length, data)

static inline unsigned int _kc_ether_crc_le(int length, unsigned char *data)
{
	unsigned int crc = 0xffffffff;  /* Initial value. */
	while(--length >= 0) {
		unsigned char current_octet = *data++;
		int bit;
		for (bit = 8; --bit >= 0; current_octet >>= 1) {
			if ((crc ^ current_octet) & 1) {
				crc >>= 1;
				crc ^= 0xedb88320U;
			} else
				crc >>= 1;
		}
	}
	return crc;
}

struct list_head {
    struct list_head *next, *prev;
};

#define DEFINE_DMA_UNMAP_ADDR(x) u64 x
#define DEFINE_DMA_UNMAP_LEN(x) u32 x

#define VLAN_N_VID        4096
#define ____cacheline_internodealigned_in_smp
#define ____cacheline_aligned_in_smp
#define NET_IP_ALIGN    0
#define u64_stats_t u64
#define compat_u64 u64
#define __force 
#define __swab16(x) OSSwapInt16(x)
#define cpu_relax()
#define KTIME_MAX            ((s64)~((u64)1 << 63))
#define KTIME_MIN            (-KTIME_MAX - 1)
#define KTIME_SEC_MAX            (KTIME_MAX / NSEC_PER_SEC)
#define KTIME_SEC_MIN            (KTIME_MIN / NSEC_PER_SEC)
#define WARN_ON(x)
#define __always_inline
#define __must_check
#define MDIO_NAME_SIZE        32
#define UL(x)        (_UL(x))

#define wmb()    asm volatile("sfence" : : : "memory")

/* Maximum number of MCA banks per CPU. */
#define MAX_NR_BANKS 64

/**
 * __struct_group() - Create a mirrored named and anonyomous struct
 *
 * @TAG: The tag name for the named sub-struct (usually empty)
 * @NAME: The identifier name of the mirrored sub-struct
 * @ATTRS: Any struct attributes (usually empty)
 * @MEMBERS: The member declarations for the mirrored structs
 *
 * Used to create an anonymous union of two structs with identical layout
 * and size: one anonymous and one named. The former's members can be used
 * normally without sub-struct naming, and the latter can be used to
 * reason about the start, end, and size of the group of struct members.
 * The named struct can also be explicitly tagged for layer reuse, as well
 * as both having struct attributes appended.
 */
#define __struct_group(TAG, NAME, ATTRS, MEMBERS...) \
    union { \
        struct { MEMBERS } ATTRS; \
        struct TAG { MEMBERS } ATTRS NAME; \
    } ATTRS

#define DECLARE_BITMAP(name,bits) \
    unsigned long name[BITS_TO_LONGS(bits)]
typedef DECLARE_BITMAP(mce_banks_t, MAX_NR_BANKS);

/**
 * struct_group() - Wrap a set of declarations in a mirrored struct
 *
 * @NAME: The identifier name of the mirrored sub-struct
 * @MEMBERS: The member declarations for the mirrored structs
 *
 * Used to create an anonymous union of two structs with identical
 * layout and size: one anonymous and one named. The former can be
 * used normally without sub-struct naming, and the latter can be
 * used to reason about the start, end, and size of the group of
 * struct members.
 */
#define struct_group(NAME, MEMBERS...)    \
    __struct_group(/* no tag */, NAME, /* no attrs */, MEMBERS)

/**
 * read_poll_timeout - Periodically poll an address until a condition is
 *            met or a timeout occurs
 * @op: accessor function (takes @args as its arguments)
 * @val: Variable to read the value into
 * @cond: Break condition (usually involving @val)
 * @sleep_us: Maximum time to sleep between reads in us (0
 *            tight-loops).  Should be less than ~20ms since usleep_range
 *            is used (see Documentation/timers/timers-howto.rst).
 * @timeout_us: Timeout in us, 0 means never timeout
 * @sleep_before_read: if it is true, sleep @sleep_us before read.
 * @args: arguments for @op poll
 *
 * Returns 0 on success and -ETIMEDOUT upon a timeout. In either
 * case, the last read value at @args is stored in @val. Must not
 * be called from atomic context if sleep_us or timeout_us are used.
 *
 * When available, you'll probably want to use one of the specialized
 * macros defined below rather than this macro directly.
 */
#define read_poll_timeout(op, val, cond, sleep_us, timeout_us, \
                sleep_before_read, args...) \
({ \
    u64 __timeout_us = (timeout_us); \
    unsigned long __sleep_us = (sleep_us); \
    ktime_t __timeout = ktime_add_us(ktime_get(), __timeout_us); \
    might_sleep_if((__sleep_us) != 0); \
    if (sleep_before_read && __sleep_us) \
        usleep_range((__sleep_us >> 2) + 1, __sleep_us); \
    for (;;) { \
        (val) = op(args); \
        if (cond) \
            break; \
        if (__timeout_us && \
            ktime_compare(ktime_get(), __timeout) > 0) { \
            (val) = op(args); \
            break; \
        } \
        if (__sleep_us) \
            usleep_range((__sleep_us >> 2) + 1, __sleep_us); \
        cpu_relax(); \
    } \
    (cond) ? 0 : -ETIMEDOUT; \
})

struct hlist_node {
    struct hlist_node *next, **pprev;
};

struct u64_stats_sync {
    int x;
};

/**
 * sizeof_field() - Report the size of a struct field in bytes
 *
 * @TYPE: The structure containing the field of interest
 * @MEMBER: The field to return the size of
 */
#define sizeof_field(TYPE, MEMBER) sizeof((((TYPE *)0)->MEMBER))

#endif

#define __WARN_printf(arg...)   

#define WARN(condition, format...) ({        \
    int __ret_warn_on = !!(condition);    \
    if (unlikely(__ret_warn_on))        \
        __WARN_printf(format);        \
    unlikely(__ret_warn_on);        \
})

#define RTL_W8(tp, reg, val8)   _OSWriteInt8((tp)->mmio_addr, (reg), (val8))
#define RTL_W16(tp, reg, val16) OSWriteLittleInt16((tp)->mmio_addr, (reg), (val16))
#define RTL_W32(tp, reg, val32) OSWriteLittleInt32((tp)->mmio_addr, (reg), (val32))
#define RTL_R8(tp, reg)         _OSReadInt8((tp)->mmio_addr, (reg))
#define RTL_R16(tp, reg)        OSReadLittleInt16((tp)->mmio_addr, (reg))
#define RTL_R32(tp, reg)        OSReadLittleInt32((tp)->mmio_addr, (reg))
