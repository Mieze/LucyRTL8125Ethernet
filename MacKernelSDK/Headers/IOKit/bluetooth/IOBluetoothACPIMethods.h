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
/*!
 *   @header IOBluetoothACPIMethods.h
 *   This header contains the definition of the IOBluetoothACPIMethods class.
 */

#ifndef _IOKIT_IOBLUETOOTHACPIMETHODS_H
#define _IOKIT_IOBLUETOOTHACPIMETHODS_H

#include <IOKit/IOService.h>
#include <os/log.h>

class IOBluetoothHCIController;
class IOACPIPlatformDevice;

class IOBluetoothACPIMethods : public IOService
{
    OSDeclareDefaultStructors(IOBluetoothACPIMethods)

    friend class IOBluetoothHCIController;
    friend class IOBluetoothHostControllerTransport;

public:
    bool         init(IOService * provider);
    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual bool start(IOService * provider) APPLE_KEXT_OVERRIDE;
    virtual void stop(IOService * provider) APPLE_KEXT_OVERRIDE;

    /*! @function SetBluetoothFamily
     *   @abstract Sets the mBluetoothFamily member variable.
     *   @param controller mBluetoothFamily is set to this IOBluetoothHCIController instance. */

    virtual void SetBluetoothFamily(IOBluetoothHCIController * controller);

    virtual void     CheckSpecialGPIO();
    virtual IOReturn SetBTRS();
    virtual IOReturn SetBTPU();
    virtual IOReturn SetBTPD();
    virtual IOReturn SetBTRB(bool, bool *);
    virtual IOReturn SetBTLP(bool);

    /*! @function ResetMethodIsAvailable
     *   @abstract Determines if the Bluetooth ACPI reset method (BTRS) is available.
     *   @discussion This function is implemented by returning mResetMethodAvailable.
     *   @result mResetMethodAvailable is returned. */

    virtual bool ResetMethodIsAvailable();

    /*! @function ROMBootMethodIsAvailable
     *   @abstract Determines if the Bluetooth ACPI ROM boot method (BTRB) is available.
     *   @discussion This function is implemented by returning mROMBootMethodAvailable.
     *   @result mROMBootMethodAvailable is returned. */

    virtual bool ROMBootMethodIsAvailable();

    /*! @function PowerUpMethodIsAvailable
     *   @abstract Determines if the Bluetooth ACPI power up method (BTPU) is available.
     *   @discussion This function is implemented by returning mPowerUpMethodAvailable.
     *   @result mPowerUpMethodAvailable is returned. */

    virtual bool PowerUpMethodIsAvailable();

    /*! @function PowerUpMethodIsAvailable
     *   @abstract Determines if the Bluetooth ACPI power down method (BTPD) is available.
     *   @discussion This function is implemented by returning mPowerDownMethodAvailable.
     *   @result mPowerDownMethodAvailable is returned. */

    virtual bool PowerDownMethodIsAvailable();

    /*! @function LowPowerMethodIsAvailable
     *   @abstract Determines if the Bluetooth ACPI low power method (BTLP) is available.
     *   @discussion This function is implemented by returning mLowPowerMethodAvailable.
     *   @result mLowPowerMethodAvailable is returned. */

    virtual bool LowPowerMethodIsAvailable();

    virtual UInt32 GetCurrentTime();

    virtual void ConvertErrorCodeToString(UInt32 errorCode, char * outStringLong, char * outStringShort);

protected:
    IOBluetoothHCIController * mBluetoothFamily; // 136
    IOACPIPlatformDevice *     mACPIDevice;      // 144
    IOService *                mProvider;        // 152

    bool     mResetMethodAvailable;     // 160 BTRS
    bool     mROMBootMethodAvailable;   // 161 BTRB
    bool     mPowerUpMethodAvailable;   // 162 BTPU
    bool     mPowerDownMethodAvailable; // 163 BTPD
    bool     mLowPowerMethodAvailable;  // 164 BTLP
    os_log_t mInternalOSLogObject;      // 168
};

#endif
