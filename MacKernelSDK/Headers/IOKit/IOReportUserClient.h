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

#ifndef _IOKIT_IOREPORTUSERCLIENT_H
#define _IOKIT_IOREPORTUSERCLIENT_H

#include <IOKit/IOUserClient.h>
#include <IOKit/IOReportHub.h>

class IOReportUserClient : public IOUserClient
{
    OSDeclareDefaultStructors(IOReportUserClient)
    
public:
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_5
    virtual IOReturn externalMethod(uint32_t selector, IOExternalMethodArguments * arguments, IOExternalMethodDispatch * dispatch = NULL, OSObject * target = NULL, void * reference = NULL) APPLE_KEXT_OVERRIDE;
#endif
    
    virtual bool init( OSDictionary * dictionary ) APPLE_KEXT_OVERRIDE;
    virtual bool initWithTask( task_t owningTask, void * securityToken, UInt32 type ) APPLE_KEXT_OVERRIDE;
    virtual void free() APPLE_KEXT_OVERRIDE;
    virtual bool start( IOService * provider ) APPLE_KEXT_OVERRIDE;
    virtual bool stop( IOService * provider ) APPLE_KEXT_OVERRIDE;
    
    virtual IOReturn clientClose() APPLE_KEXT_OVERRIDE;
    virtual IOService * getService() APPLE_KEXT_OVERRIDE;
    virtual IOReturn clientMemoryForType( UInt32 type, IOOptionBits * options, IOMemoryDescriptor ** memory ) APPLE_KEXT_OVERRIDE;
    
    virtual IOReturn open( IOOptionBits options );
    virtual IOReturn close();
    
    static IOReturn _open( IOReportUserClient *, void *, IOExternalMethodArguments * );
    static IOReturn _close( IOReportUserClient *, void *, IOExternalMethodArguments * );
    static IOReturn goto_configureInterests( IOReportUserClient *, void *, IOExternalMethodArguments * );
    static IOReturn goto_getValues( IOReportUserClient *, void *, IOExternalMethodArguments * );
    IOReturn _configureInterests( IOReportInterestList *, IOByteCount * );
    IOReturn _getInterests( IOByteCount, OSDictionary **, IOBufferMemoryDescriptor ** );
    IOReturn _getValues( IOByteCount );

protected:
    IOOptionBits mHubOptions;
    task_t mOwningTask;
    IOReportHub * mHub;
    IOLock * mHubLock;
    OSArray * mReportDictionaryArray;
    OSArray * mSubscriptionBufferArray;
};

#endif
