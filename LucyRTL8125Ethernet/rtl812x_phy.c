//
//  rtl812x_phy.c
//  RTL812xLucy
//
//  Created by Laura Müller on 22.01.26.
//
// This driver is based on version 9.016.01 of Realtek's r8125 driver.

#include "rtl812x.h"
#include "linux/bitsperlong.h"
#include "linux/bitops.h"

#define bitmap_size(nbits)    (ALIGN(nbits, BITS_PER_LONG) / BITS_PER_BYTE)

static __always_inline void bitmap_zero(unsigned long *dst, unsigned int nbits)
{
    unsigned int len = bitmap_size(nbits);

    if (small_const_nbits(nbits))
        *dst = 0;
    else
        memset(dst, 0, len);
}

static inline void linkmode_zero(unsigned long *dst)
{
    bitmap_zero(dst, __ETHTOOL_LINK_MODE_MASK_NBITS);
}

void ethtool_convert_legacy_u32_to_link_mode(unsigned long *dst,
                         u32 legacy_u32)
{
    linkmode_zero(dst);
    dst[0] = legacy_u32;
}

void linkmode_mod_bit(unsigned int nbit, unsigned long *dst, u32 value)
{
    if (value)
        *dst |= (1 << nbit);
    else
        *dst &= ~(1 << nbit);
}

void linkmode_set_bit(unsigned int nbit, unsigned int *dst)
{
    *dst |= (1 << nbit);
}

