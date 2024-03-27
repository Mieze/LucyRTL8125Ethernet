/*
 * Copyright (c) 2019-2022 Apple, Inc. All rights reserved.
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

#ifndef _IOSKYWALKTYPES_H
#define _IOSKYWALKTYPES_H

#include <IOKit/IOLib.h>

typedef uint32_t IOSkywalkNetworkLinkStatus;
enum
{
    kIOSkywalkNetworkLinkStatusInvalid         = 0x00000000,
    kIOSkywalkNetworkLinkStatusInactive        = 0x00000001,
    kIOSkywalkNetworkLinkStatusActive          = 0x00000003,
    kIOSkywalkNetworkLinkStatusWakeSameNet     = 0x00000004
};

typedef uint32_t IOSkywalkNetworkLinkQuality;
enum
{
    kIOSkywalkNetworkLinkQualityOff            = (-2),
    kIOSkywalkNetworkLinkQualityUnknown        = (-1),
    kIOSkywalkNetworkLinkQualityBad            = 10,
    kIOSkywalkNetworkLinkQualityPoor           = 50,
    kIOSkywalkNetworkLinkQualityGood           = 100
};

enum {
    kIOSkywalkNetworkMediaOptionFullDuplex     = 0x00100000,
    kIOSkywalkNetworkMediaOptionHalfDuplex     = 0x00200000,
    kIOSkywalkNetworkMediaOptionFlowControl    = 0x00400000,
    kIOSkywalkNetworkMediaOptionEEE            = 0x00800000,
    kIOSkywalkNetworkMediaOptionLoopback       = 0x08000000
};

typedef uint32_t IOSkywalkNetworkMediaType;
enum
{
    kIOSkywalkNetworkMediaEthernetAuto             = 0x00000020,
    kIOSkywalkNetworkMediaEthernetManual           = 0x00000021,
    kIOSkywalkNetworkMediaEthernetNone             = 0x00000022,
    kIOSkywalkNetworkMediaEthernet10BaseT          = 0x00000023,
    kIOSkywalkNetworkMediaEthernet10Base2          = 0x00000024,
    kIOSkywalkNetworkMediaEthernet10Base5          = 0x00000025,
    kIOSkywalkNetworkMediaEthernet100BaseTX        = 0x00000026,
    kIOSkywalkNetworkMediaEthernet100BaseFX        = 0x00000027,
    kIOSkywalkNetworkMediaEthernet100BaseT4        = 0x00000028,
    kIOSkywalkNetworkMediaEthernet100BaseVG        = 0x00000029,
    kIOSkywalkNetworkMediaEthernet100BaseT2        = 0x0000002a,
    kIOSkywalkNetworkMediaEthernet1000BaseSX       = 0x0000002b,
    kIOSkywalkNetworkMediaEthernet10BaseSTP        = 0x0000002c,
    kIOSkywalkNetworkMediaEthernet10BaseFL         = 0x0000002d,
    kIOSkywalkNetworkMediaEthernet1000BaseLX       = 0x0000002e,
    kIOSkywalkNetworkMediaEthernet1000BaseCX       = 0x0000002f,
    kIOSkywalkNetworkMediaEthernet1000BaseT        = 0x00000030,
    kIOSkywalkNetworkMediaEthernetHomePNA1         = 0x00000031,
    kIOSkywalkNetworkMediaEthernet10GBaseSR        = 0x00000032,
    kIOSkywalkNetworkMediaEthernet10GBaseLR        = 0x00000033,
    kIOSkywalkNetworkMediaEthernet10GBaseCX4       = 0x00000034,
    kIOSkywalkNetworkMediaEthernet10GBaseT         = 0x00000035,
    kIOSkywalkNetworkMediaEthernet2500BaseT        = 0x00000036,
    kIOSkywalkNetworkMediaEthernet5000BaseT        = 0x00000037,
    kIOSkywalkNetworkMediaEthernet1000BaseCX_SGMII = 0x00000038,
    kIOSkywalkNetworkMediaEthernet1000BaseKX       = 0x00000039,
    kIOSkywalkNetworkMediaEthernet10GBaseKX4       = 0x0000003a,
    kIOSkywalkNetworkMediaEthernet10GBaseKR        = 0x0000003b,
    kIOSkywalkNetworkMediaEthernet10GBaseCR1       = 0x0000003c,
    kIOSkywalkNetworkMediaEthernet10GBaseER        = 0x0000003d,
    kIOSkywalkNetworkMediaEthernet20GBaseKR2       = 0x0000003e,
    kIOSkywalkNetworkMediaEthernet25GBaseCR        = 0x0000003f,
    kIOSkywalkNetworkMediaEthernet25GBaseKR        = 0x00000820,
    kIOSkywalkNetworkMediaEthernet25GBaseSR        = 0x00000821,
    kIOSkywalkNetworkMediaEthernet25GBaseLR        = 0x00000822,
    kIOSkywalkNetworkMediaEthernet40GBaseCR4       = 0x00000823,
    kIOSkywalkNetworkMediaEthernet40GBaseSR4       = 0x00000824,
    kIOSkywalkNetworkMediaEthernet40GBaseLR4       = 0x00000825,
    kIOSkywalkNetworkMediaEthernet40GBaseKR4       = 0x00000826,
    kIOSkywalkNetworkMediaEthernet50GBaseCR2       = 0x00000827,
    kIOSkywalkNetworkMediaEthernet50GBaseKR2       = 0x00000828,
    kIOSkywalkNetworkMediaEthernet50GBaseSR2       = 0x00000829,
    kIOSkywalkNetworkMediaEthernet50GBaseLR2       = 0x0000082a,
    kIOSkywalkNetworkMediaEthernet56GBaseR4        = 0x0000082b,
    kIOSkywalkNetworkMediaEthernet100GBaseCR4      = 0x0000082c,
    kIOSkywalkNetworkMediaEthernet100GBaseSR4      = 0x0000082d,
    kIOSkywalkNetworkMediaEthernet100GBaseKR4      = 0x0000082e,
    kIOSkywalkNetworkMediaEthernet100GBaseLR4      = 0x0000082f,
    kIOSkywalkNetworkMediaEthernetMask             = 0x00000fff
};

enum
{
    kIOSkywalkNetworkFeatureFlagSoftwareVlan       = 0x00020000,
    kIOSkywalkNetworkFeatureFlagWOMP               = 0x04000000,
    kIOSkywalkNetworkFeatureFlagNicProxy           = 0x08000000
};
 
#endif
