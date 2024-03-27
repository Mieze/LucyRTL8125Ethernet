/*
 * Released under "The BSD 3-Clause License"
 *
 * Copyright (c) 2021 cjiang. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The names of its contributors may not be used to endorse or promote
 *    products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

 #ifndef _IO80211VIRTUALINTERFACENAMER_H
 #define _IO80211VIRTUALINTERFACENAMER_H

#include <IOKit/IOService.h>

class IO80211VirtualInterfaceNamer : public IOService
{
	OSDeclareDefaultStructors(IO80211VirtualInterfaceNamer)

public:
	virtual bool init() APPLE_KEXT_OVERRIDE;
	virtual void free() APPLE_KEXT_OVERRIDE;

	bool arrayContainsUnitNumber( OSArray * array, OSNumber * unitNumber );
	bool markInterfaceUnitUsed( char const * name, UInt32 unitNumber );
	void markInterfaceUnitUnused( char const * name, UInt32 unitNumber );
	UInt32 nextAvailableUnitNumberForName( char const * name, bool markUsed );

	IORecursiveLock * _lock;
	OSDictionary * _interfaceUnits; // all OSArrays
};

static IO80211VirtualInterfaceNamer * _interfaceNamer;

bool IO80211VirtualInterfaceNamerRetain();
void IO80211VirtualInterfaceNamerRelease();
bool IO80211VirtualInterfaceNamerMarkInterfaceUnitUsed( char const * name, UInt32 unitNumber );
void IO80211VirtualInterfaceNamerMarkInterfaceUnitUnused( char const * name, UInt32 unitNumber );
UInt32 IO80211VirtualInterfaceNamerNextAvailableUnitNumberForName( char const * name, bool markUsed );

#endif
