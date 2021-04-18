/* linux.h -- Definitions to make the linux code compile under OS X.
 *
 * Copyright (c) 2013 Laura Müller <laura-mueller@uni-duesseldorf.de>
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
 * Driver for Realtek RTL8111x PCIe ethernet controllers.
 *
 * This driver is based on Realtek's r8125 Linux driver (9.03.04).
 */

#ifndef LucyRTL8125_linux_h
#define LucyRTL8125_linux_h

#include <IOKit/IOLib.h>

/******************************************************************************/
#pragma mark -
#pragma mark Debugging
#pragma mark -
/******************************************************************************/

#if defined(RTL8125_DEBUG)

// Levels 1 to 6 are used, in order of verbosity.
// 5 or above will usually produce a LOT of log output for every packet.
#define dprintk(format,args...)                                        \
do                                                                             \
{                                                                              \
IOLog(format, ##args);                             \
} while (0)

#define dev_dbg(dev,format,args...)                                            \
IOLog(format, ##args);

#else // Disable debugging.

#define dprintk(args...)

#endif // Disable debugging.

#define printk(args...) 
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
#define BIT(nr)                 (1UL << (nr))
#define BIT_MASK(nr)            (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)            ((nr) / BITS_PER_LONG)
#define BITS_PER_BYTE           8
#define BITS_TO_LONGS(bits)     (((bits)+BITS_PER_LONG-1)/BITS_PER_LONG)

#define ARRAY_SIZE(x)           (sizeof(x) / sizeof((x)[0]))

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

static inline int
test_bit(int nr, const volatile unsigned long *addr)
{
    return (OSAddAtomic(0, addr) & (1 << nr)) != 0;
}

static inline void
set_bit(unsigned int nr, volatile unsigned long *addr)
{
    OSTestAndSet(nr, (volatile UInt8 *)addr);
}

static inline void
clear_bit(unsigned int nr, volatile unsigned long *addr)
{
    OSTestAndClear(nr, (volatile UInt8 *)addr);
}

static inline int
test_and_clear_bit(unsigned int nr, volatile unsigned long *addr)
{
    return !OSTestAndClear(nr, (volatile UInt8 *)addr);
}

static inline int
test_and_set_bit(unsigned int nr, volatile unsigned long *addr)
{
    return OSTestAndSet(nr, (volatile UInt8 *)addr);
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

#define RTL_W8(tp, reg, val8)      _OSWriteInt8(tp->mmio_addr, (reg), (val8))
#define RTL_W16(tp, reg, val16)    OSWriteLittleInt16(tp->mmio_addr, (reg), (val16))
#define RTL_W32(tp, reg, val32)    OSWriteLittleInt32(tp->mmio_addr, (reg), (val32))

#define RTL_R8(tp, reg)             _OSReadInt8(tp->mmio_addr, (reg))
#define RTL_R16(tp, reg)            OSReadLittleInt16(tp->mmio_addr, (reg))
#define RTL_R32(tp, reg)            OSReadLittleInt32(tp->mmio_addr, (reg))

#define wmb() OSSynchronizeIO()

/******************************************************************************/
#pragma mark -
#pragma mark Locks
#pragma mark -
/******************************************************************************/

#define spinlock_t  IOSimpleLock *
#define atomic_t    volatile SInt32


#define spin_lock_init(slock)                           \
do                                                      \
{                                                       \
if (*slock == NULL)                                   \
{                                                     \
*(slock) = IOSimpleLockAlloc();                     \
}                                                     \
} while (0)

#define spin_lock(lock)

#define spin_unlock(lock)

#define spin_lock_irqsave(lock,flags)

#define spin_trylock_irqsave(lock,flags)

#define spin_unlock_irqrestore(lock,flags)

#define usec_delay(x)           IODelay(x)
#define msec_delay(x)           IOSleep(x)
#define udelay(x)               IODelay(x)
#define mdelay(x)               IODelay(1000*(x))
#define msleep(x)               IOSleep(x)

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

#define net_device rtl8125_private
#define netdev_priv(x)  ((struct rtl8125_private *)x)
#define netif_msg_link(x) 0
#define KERN_ERR

#define DISABLED_CODE 0

struct pci_dev {
    UInt16 vendor;
    UInt16 device;
    UInt16 subsystem_vendor;
    UInt16 subsystem_device;
};

#define aspm tp->configASPM
#define s0_magic_packet tp->s0MagicPacket

#define BMCR_SPEED10    0x0000

/* The additional bytes required by VLAN
 * (in addition to the Ethernet header)
 */
#define VLAN_HLEN    4

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

#endif
