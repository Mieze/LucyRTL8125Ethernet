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

#ifndef _IOKIT_IOBLUETOOTHSERIALMANAGER_H
#define _IOKIT_IOBLUETOOTHSERIALMANAGER_H

#include <IOKit/IOService.h>
#include <IOKit/bluetooth/Bluetooth.h>
#include <IOKit/bluetooth/serial/IOBluetoothSerialClient.h>

class IOBluetoothRFCOMMChannel;
class IOBluetoothL2CAPChannel;
class IOBluetoothHCIController;
class IOBluetoothRFCOMMConnection;

class IOBluetoothSerialManager : public IOService
{
    OSDeclareDefaultStructors(IOBluetoothSerialManager)

    typedef struct SecurityParameters
    {
        bool  authenticationRequired;
        UInt8 encryptionType;
    } SecurityParameters;

protected:
    static bool     staticChannelGoesAway(void * target, void * refCon, IOService * newService, IONotifier * notifier);
    static bool     staticNewChannelShowsUp(void * target, void * refCon, IOService * newService, IONotifier * notifier);
    static void     newRFCOMMChannelMain(void *, void *);
    static IOReturn staticNewChannelShowsUpAction(OSObject *, void *, void *, void *, void *);
    static bool     staticHCIControllerGoesAway(void * target, void * refCon, IOService * newService, IONotifier * notifier);
    static void     byeHCIControllerMain(void *, void *);
    static IOReturn staticHCIControllerGoesAwayAction(OSObject *, void *, void *, void *, void *);
    static bool     staticHCIControllerShowsUp(void * target, void * refCon, IOService * newService, IONotifier * notifier);
    static void     newHCIControllerMain(void *, void *);
    static IOReturn staticHCIControllerShowsUpAction(OSObject *, void *, void *, void *, void *);
    static bool     staticControllerTransportGoesAway(void * target, void * refCon, IOService * newService, IONotifier * notifier);
    static bool     staticControllerTransportShowsUp(void * target, void * refCon, IOService * newService, IONotifier * notifier);
    static IOReturn staticOpenForClient(OSObject * target, void *, void *, void *, void *);
    static IOReturn staticOpenL2CAPChannel(OSObject *, void *, void *, void *, void *);
    static IOReturn staticCloseDeviceForService(OSObject *, void *, void *, void *, void *);

    virtual IOReturn protectedSetProperties(OSDictionary * dictionary);
    virtual IOReturn protectedNewChannelShowsUp(IOBluetoothRFCOMMChannel * channel);
    virtual IOReturn protectedHCIControllerGoesAway(IOBluetoothHCIController * controller);
    virtual IOReturn protectedHCIControllerShowsUp(IOBluetoothHCIController * controller);
    virtual IOReturn protectedOpenL2CAPChannel(BluetoothDeviceAddress * deviceAddress, SecurityParameters * params, UInt16, IOBluetoothL2CAPChannel ** outChannel);
    virtual IOReturn protectedOpenChannelForClient(IOBluetoothSerialClient * client);
    virtual IOReturn protectedCloseDeviceForService(IOService * service);

public:
    virtual bool     init(OSDictionary * dictionary = NULL) APPLE_KEXT_OVERRIDE;
    virtual bool     start(IOService * provider) APPLE_KEXT_OVERRIDE;
    virtual void     stop(IOService * provider) APPLE_KEXT_OVERRIDE;
    virtual IOReturn setProperties(OSObject * properties) APPLE_KEXT_OVERRIDE;

    virtual IOReturn                  capture(IOBluetoothRFCOMMConnection *, IOBluetoothSerialClient *);
    virtual bool                      localRemovePort(OSString *);
    virtual IOBluetoothSerialClient * localFindPort(OSString *);
    virtual bool                      channelBelongsTo(IOBluetoothRFCOMMChannel *, UInt8 *);
    virtual bool                      setProperties(OSDictionary * dictionary, IOBluetoothSerialClient *);
    virtual bool                      copyProperty(const OSSymbol *, OSDictionary *, IOService *);
    virtual bool                      handleSerialDictionary(OSDictionary *);
    virtual bool                      terminateSerial(OSObject *);
    virtual IOReturn                  addToAllowedPorts(IOBluetoothSerialClient *, bool);
    IOReturn                          removeFromAllowedPorts(IOBluetoothSerialClient *);
    virtual IOReturn                  openChannelForClient(IOBluetoothSerialClient *);
    virtual bool                      closeChannelForClient(IOBluetoothSerialClient *);
    virtual IOReturn                  closeDeviceForClientRFCOMM(IOBluetoothRFCOMMChannel *);
    virtual bool                      serialMustReturnWithConnection(IOBluetoothSerialClient *);
    virtual UInt8                     serialAttachesToChannel(IOBluetoothSerialClient *);

    virtual bool            serialNeedsBDAddress(IOBluetoothSerialClient *, BluetoothDeviceAddress *);
    virtual bool            serialIsEnabled(IOBluetoothSerialClient *);
    virtual IOCommandGate * getBluetoothCommandGate() const;

protected:
    OSSymbol * mBTName;                   // 136
    OSSymbol * mBTTTYName;                // 144
    OSSymbol * mBTPSM;                    // 152
    OSSymbol * mBTRFCOMMChannel;          // 160
    OSSymbol * mBTAddress;                // 168
    OSSymbol * mBTAuthenticationRequired; // 176
    OSSymbol * mBTEncryptionType;         // 184
    OSSymbol * mBTSerialConnectionType;   // 192
    OSSymbol * mBTEanbledState;           // 200

    IONotifier *                  mChInNotification;         // 208
    IONotifier *                  mChOutNotification;        // 216
    thread_call_t                 mNewChThreadCall;          // 224
    IONotifier *                  mHCIInNotification;        // 232
    IONotifier *                  mHCIOutNotification;       // 240
    IONotifier *                  mTransportInNotification;  // 248
    IONotifier *                  mTransportOutNotification; // 256
    thread_call_t                 mNewHCIThreadCall;         // 264
    thread_call_t                 mByeHCIThreadCall;         // 272
    IOWorkLoop *                  mWorkLoop;                 // 280
    IOCommandGate *               mCommandGate;              // 288
    IOWorkLoop *                  mBluetoothWorkLoop;        // 296, workLoop of IOBluetoothHCIController
    IOCommandGate *               mBluetoothCommandGate;     // 304
    IOBluetoothHCIController *    mHCIController;            // 312
    uint8_t                       __reserved;                // 320, never used
    IOBluetoothRFCOMMConnection * mRFCOMMConnection;         // 328
    IOBluetoothL2CAPChannel *     mL2CAPChannel;             // 336
    bool                          mHasControllerTransport;   // 344
};

#endif
