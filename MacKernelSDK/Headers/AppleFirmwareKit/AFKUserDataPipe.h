/* iig(DriverKit-248) generated from AFKUserDataPipe.iig */

/* AFKUserDataPipe.iig:1-38 */
/*
 * Copyright (c) 2021 Apple Inc.  All rights reserved.
 *
 * This document is the property of Apple Inc.
 * It is considered confidential and proprietary.
 *
 * This document may not be reproduced or transmitted in any form,
 * in whole or in part, without the express written permission of
 * Apple Inc.
 */

#if !__IIG
#if KERNEL
#include <AppleFirmwareKit/AFKUserDataPipe_kext.h>
#endif
#endif

#ifndef _AFKDriverKit_AFKUserDataPipe_h
#define _AFKDriverKit_AFKUserDataPipe_h

#include <AFKDriverKit/AFKDriverKitCommon.h>
#include <AFKDriverKit/AFKUserMemoryDescriptor.h>  /* .iig include */
#include <DriverKit/IODataQueueDispatchSource.h>  /* .iig include */
#include <DriverKit/IOService.h>  /* .iig include */
#include <DriverKit/OSAction.h>  /* .iig include */

/*! @define   AFKUserDataPipe
 *  @abstract An internal class that should not be used.
 *            This class is internal only, and will be subject
 *            to constant change.
 */

#if AFKDKUser
uint32_t _enqueue_report_opts(endpoint_options options);
uint32_t _enqueue_command_opts(endpoint_options options);
uint32_t _enqueue_response_opts(endpoint_options options);
#endif /* AFKDKUser */

/* source class AFKUserDataPipe AFKUserDataPipe.iig:39-77 */

#if __DOCUMENTATION__
#define KERNEL IIG_KERNEL

class KERNEL AFKUserDataPipe : public OSObject
{
    using super = OSObject;

    virtual AFKUserMemoryDescriptor * prepareBuffer(PayloadBuffer data) LOCALONLY;

public:
    virtual bool init() override;

    virtual void free() override;

    virtual bool open() LOCALONLY;

    virtual void close(OSActionAbortedHandler handler) LOCALONLY;

    virtual void setReportHandler(ReportBlock reportHandler) LOCALONLY;

    virtual void setCommandHandler(CommandBlock commandHandler) LOCALONLY;

    virtual void setResponseHandler(ResponseBlock responseHandler) LOCALONLY;

    virtual IOReturn enqueueReport(unsigned         packetType,
                                   uint64_t         timestamp,
                                   PayloadBuffer    reportBuffer,
                                   endpoint_options options = 0) LOCALONLY;

    virtual IOReturn enqueueCommand(void           * context,
                                    unsigned         packetType,
                                    uint64_t         timestamp,
                                    PayloadBuffer    commandBuffer,
                                    PayloadBuffer    responseBuffer,
                                    endpoint_options options = 0,
                                    IOService      * forClient = nullptr) LOCALONLY;

    virtual IOReturn enqueueResponse(CommandID        id,
                                     IOReturn         result,
                                     uint64_t         timestamp,
                                     PayloadBuffer    responseBuffer,
                                     endpoint_options options = 0) LOCALONLY;
};

#undef KERNEL
#else /* __DOCUMENTATION__ */

/* generated class AFKUserDataPipe AFKUserDataPipe.iig:39-77 */

#define AFKUserDataPipe_EnqueueResponse_ID            0xc10141ce29d0a683ULL
#define AFKUserDataPipe_EnqueueCommand_ID            0x8ee8626f4b3f3021ULL
#define AFKUserDataPipe_EnqueueReport_ID            0xb49910bb998957cdULL
#define AFKUserDataPipe_SendHandleResponseAction_ID            0xb4d82ea5d3b827b7ULL
#define AFKUserDataPipe__HandleResponse_ID            0xdfa384c702f44f1fULL
#define AFKUserDataPipe_HandleResponse_ID            0xc92d5ba3b04310deULL
#define AFKUserDataPipe_SendHandleCommandAction_ID            0xd448141067d9ef46ULL
#define AFKUserDataPipe_CompleteHandleCommand_ID            0xbf61bb4c9a1471b8ULL
#define AFKUserDataPipe__HandleCommand_ID            0xafd924d4d6ea7e6eULL
#define AFKUserDataPipe_HandleCommand_ID            0xcfc1ddbce436d302ULL
#define AFKUserDataPipe_SendHandleReportAction_ID            0x9d480c65b28d49c9ULL
#define AFKUserDataPipe__HandleReport_ID            0xa085980707a86a8dULL
#define AFKUserDataPipe_HandleReport_ID            0xc5b95e2452c9d0a9ULL

#define AFKUserDataPipe_EnqueueResponse_Args \
        uint64_t context, \
        IOReturn result, \
        uint64_t timestamp, \
        AFKUserMemoryDescriptor * payload, \
        uint32_t options

#define AFKUserDataPipe_EnqueueCommand_Args \
        uint64_t context, \
        unsigned int packetType, \
        uint64_t timestamp, \
        AFKUserMemoryDescriptor * payload, \
        size_t outputPayloadSize, \
        uint32_t options, \
        IOService * forClient

#define AFKUserDataPipe_EnqueueReport_Args \
        unsigned int packetType, \
        uint64_t timestamp, \
        AFKUserMemoryDescriptor * payload, \
        uint32_t options

#define AFKUserDataPipe_SendHandleResponseAction_Args \
        OSAction * action

#define AFKUserDataPipe__HandleResponse_Args \
        uint64_t context, \
        IOReturn result, \
        uint64_t timestamp, \
        AFKUserMemoryDescriptor * payload, \
        OSAction * action

#define AFKUserDataPipe_HandleResponse_Args \
        uint64_t context, \
        IOReturn result, \
        uint64_t timestamp, \
        AFKUserMemoryDescriptor * payload, \
        OSAction * action

#define AFKUserDataPipe_SendHandleCommandAction_Args \
        OSAction * action

#define AFKUserDataPipe_CompleteHandleCommand_Args \
        uint64_t context, \
        IOReturn result, \
        OSAction * action

#define AFKUserDataPipe__HandleCommand_Args \
        uint64_t context, \
        uint32_t packetType, \
        uint64_t timestamp, \
        AFKUserMemoryDescriptor * payload, \
        uint32_t options, \
        OSAction * action

#define AFKUserDataPipe_HandleCommand_Args \
        uint64_t context, \
        uint32_t packetType, \
        uint64_t timestamp, \
        AFKUserMemoryDescriptor * payload, \
        uint32_t options, \
        OSAction * action

#define AFKUserDataPipe_SendHandleReportAction_Args \
        OSAction * action

#define AFKUserDataPipe__HandleReport_Args \
        uint32_t packetType, \
        uint64_t timestamp, \
        AFKUserMemoryDescriptor * payload, \
        uint32_t options, \
        OSAction * action

#define AFKUserDataPipe_HandleReport_Args \
        uint32_t packetType, \
        uint64_t timestamp, \
        AFKUserMemoryDescriptor * payload, \
        uint32_t options, \
        OSAction * action

#define AFKUserDataPipe_Methods \
\
public:\
\
    virtual kern_return_t\
    Dispatch(const IORPC rpc) APPLE_KEXT_OVERRIDE;\
\
    static kern_return_t\
    _Dispatch(AFKUserDataPipe * self, const IORPC rpc);\
\
    IOReturn\
    EnqueueResponse(\
        uint64_t context,\
        IOReturn result,\
        uint64_t timestamp,\
        AFKUserMemoryDescriptor * payload,\
        uint32_t options,\
        OSDispatchMethod supermethod = NULL);\
\
    IOReturn\
    EnqueueCommand(\
        uint64_t context,\
        unsigned int packetType,\
        uint64_t timestamp,\
        AFKUserMemoryDescriptor * payload,\
        size_t outputPayloadSize,\
        uint32_t options,\
        IOService * forClient,\
        OSDispatchMethod supermethod = NULL);\
\
    IOReturn\
    EnqueueReport(\
        unsigned int packetType,\
        uint64_t timestamp,\
        AFKUserMemoryDescriptor * payload,\
        uint32_t options,\
        OSDispatchMethod supermethod = NULL);\
\
    IOReturn\
    SendHandleResponseAction(\
        OSAction * action,\
        OSDispatchMethod supermethod = NULL);\
\
    kern_return_t\
    CreateAction_HandleResponse(size_t referenceSize, OSAction ** action);\
\
    void\
    HandleResponse(\
        uint64_t context,\
        IOReturn result,\
        uint64_t timestamp,\
        AFKUserMemoryDescriptor * payload,\
        OSAction * action,\
        OSDispatchMethod supermethod = NULL);\
\
    IOReturn\
    SendHandleCommandAction(\
        OSAction * action,\
        OSDispatchMethod supermethod = NULL);\
\
    IOReturn\
    CompleteHandleCommand(\
        uint64_t context,\
        IOReturn result,\
        OSAction * action,\
        OSDispatchMethod supermethod = NULL);\
\
    kern_return_t\
    CreateAction_HandleCommand(size_t referenceSize, OSAction ** action);\
\
    void\
    HandleCommand(\
        uint64_t context,\
        uint32_t packetType,\
        uint64_t timestamp,\
        AFKUserMemoryDescriptor * payload,\
        uint32_t options,\
        OSAction * action,\
        OSDispatchMethod supermethod = NULL);\
\
    IOReturn\
    SendHandleReportAction(\
        OSAction * action,\
        OSDispatchMethod supermethod = NULL);\
\
    kern_return_t\
    CreateAction_HandleReport(size_t referenceSize, OSAction ** action);\
\
    void\
    HandleReport(\
        uint32_t packetType,\
        uint64_t timestamp,\
        AFKUserMemoryDescriptor * payload,\
        uint32_t options,\
        OSAction * action,\
        OSDispatchMethod supermethod = NULL);\
\
\
protected:\
    /* _Impl methods */\
\
    void\
    _HandleResponse_Impl(AFKUserDataPipe__HandleResponse_Args);\
\
    void\
    _HandleCommand_Impl(AFKUserDataPipe__HandleCommand_Args);\
\
    void\
    _HandleReport_Impl(AFKUserDataPipe__HandleReport_Args);\
\
\
public:\
    /* _Invoke methods */\
\
    typedef IOReturn (*EnqueueResponse_Handler)(OSMetaClassBase * target, AFKUserDataPipe_EnqueueResponse_Args);\
    static kern_return_t\
    EnqueueResponse_Invoke(const IORPC rpc,\
        OSMetaClassBase * target,\
        EnqueueResponse_Handler func);\
\
    typedef IOReturn (*EnqueueCommand_Handler)(OSMetaClassBase * target, AFKUserDataPipe_EnqueueCommand_Args);\
    static kern_return_t\
    EnqueueCommand_Invoke(const IORPC rpc,\
        OSMetaClassBase * target,\
        EnqueueCommand_Handler func);\
\
    typedef IOReturn (*EnqueueReport_Handler)(OSMetaClassBase * target, AFKUserDataPipe_EnqueueReport_Args);\
    static kern_return_t\
    EnqueueReport_Invoke(const IORPC rpc,\
        OSMetaClassBase * target,\
        EnqueueReport_Handler func);\
\
    typedef IOReturn (*SendHandleResponseAction_Handler)(OSMetaClassBase * target, AFKUserDataPipe_SendHandleResponseAction_Args);\
    static kern_return_t\
    SendHandleResponseAction_Invoke(const IORPC rpc,\
        OSMetaClassBase * target,\
        SendHandleResponseAction_Handler func);\
\
    typedef void (*HandleResponse_Handler)(OSMetaClassBase * target, AFKUserDataPipe_HandleResponse_Args);\
    static kern_return_t\
    HandleResponse_Invoke(const IORPC rpc,\
        OSMetaClassBase * target,\
        HandleResponse_Handler func,\
        const OSMetaClass * targetActionClass);\
\
    static kern_return_t\
    HandleResponse_Invoke(const IORPC rpc,\
        OSMetaClassBase * target,\
        HandleResponse_Handler func);\
\
    typedef IOReturn (*SendHandleCommandAction_Handler)(OSMetaClassBase * target, AFKUserDataPipe_SendHandleCommandAction_Args);\
    static kern_return_t\
    SendHandleCommandAction_Invoke(const IORPC rpc,\
        OSMetaClassBase * target,\
        SendHandleCommandAction_Handler func);\
\
    typedef IOReturn (*CompleteHandleCommand_Handler)(OSMetaClassBase * target, AFKUserDataPipe_CompleteHandleCommand_Args);\
    static kern_return_t\
    CompleteHandleCommand_Invoke(const IORPC rpc,\
        OSMetaClassBase * target,\
        CompleteHandleCommand_Handler func);\
\
    typedef void (*HandleCommand_Handler)(OSMetaClassBase * target, AFKUserDataPipe_HandleCommand_Args);\
    static kern_return_t\
    HandleCommand_Invoke(const IORPC rpc,\
        OSMetaClassBase * target,\
        HandleCommand_Handler func,\
        const OSMetaClass * targetActionClass);\
\
    static kern_return_t\
    HandleCommand_Invoke(const IORPC rpc,\
        OSMetaClassBase * target,\
        HandleCommand_Handler func);\
\
    typedef IOReturn (*SendHandleReportAction_Handler)(OSMetaClassBase * target, AFKUserDataPipe_SendHandleReportAction_Args);\
    static kern_return_t\
    SendHandleReportAction_Invoke(const IORPC rpc,\
        OSMetaClassBase * target,\
        SendHandleReportAction_Handler func);\
\
    typedef void (*HandleReport_Handler)(OSMetaClassBase * target, AFKUserDataPipe_HandleReport_Args);\
    static kern_return_t\
    HandleReport_Invoke(const IORPC rpc,\
        OSMetaClassBase * target,\
        HandleReport_Handler func,\
        const OSMetaClass * targetActionClass);\
\
    static kern_return_t\
    HandleReport_Invoke(const IORPC rpc,\
        OSMetaClassBase * target,\
        HandleReport_Handler func);\
\


#define AFKUserDataPipe_KernelMethods \
\
protected:\
    /* _Impl methods */\
\
    IOReturn\
    EnqueueResponse_Impl(AFKUserDataPipe_EnqueueResponse_Args);\
\
    IOReturn\
    EnqueueCommand_Impl(AFKUserDataPipe_EnqueueCommand_Args);\
\
    IOReturn\
    EnqueueReport_Impl(AFKUserDataPipe_EnqueueReport_Args);\
\
    IOReturn\
    SendHandleResponseAction_Impl(AFKUserDataPipe_SendHandleResponseAction_Args);\
\
    IOReturn\
    SendHandleCommandAction_Impl(AFKUserDataPipe_SendHandleCommandAction_Args);\
\
    IOReturn\
    CompleteHandleCommand_Impl(AFKUserDataPipe_CompleteHandleCommand_Args);\
\
    IOReturn\
    SendHandleReportAction_Impl(AFKUserDataPipe_SendHandleReportAction_Args);\
\


#define AFKUserDataPipe_VirtualMethods \
\
public:\
\
    virtual AFKUserMemoryDescriptor *\
    prepareBuffer(\
        PayloadBuffer data) APPLE_KEXT_OVERRIDE;\
\
    virtual bool\
    init(\
) APPLE_KEXT_OVERRIDE;\
\
    virtual void\
    free(\
) APPLE_KEXT_OVERRIDE;\
\
    virtual bool\
    open(\
) APPLE_KEXT_OVERRIDE;\
\
    virtual void\
    close(\
        OSActionAbortedHandler handler) APPLE_KEXT_OVERRIDE;\
\
    virtual void\
    setReportHandler(\
        ReportBlock reportHandler) APPLE_KEXT_OVERRIDE;\
\
    virtual void\
    setCommandHandler(\
        CommandBlock commandHandler) APPLE_KEXT_OVERRIDE;\
\
    virtual void\
    setResponseHandler(\
        ResponseBlock responseHandler) APPLE_KEXT_OVERRIDE;\
\
    virtual IOReturn\
    enqueueReport(\
        unsigned int packetType,\
        uint64_t timestamp,\
        PayloadBuffer reportBuffer,\
        endpoint_options options = 0) APPLE_KEXT_OVERRIDE;\
\
    virtual IOReturn\
    enqueueCommand(\
        void * context,\
        unsigned int packetType,\
        uint64_t timestamp,\
        PayloadBuffer commandBuffer,\
        PayloadBuffer responseBuffer,\
        endpoint_options options = 0,\
        IOService * forClient = nullptr) APPLE_KEXT_OVERRIDE;\
\
    virtual IOReturn\
    enqueueResponse(\
        CommandID id,\
        IOReturn result,\
        uint64_t timestamp,\
        PayloadBuffer responseBuffer,\
        endpoint_options options = 0) APPLE_KEXT_OVERRIDE;\
\


#if !KERNEL

extern OSMetaClass          * gAFKUserDataPipeMetaClass;
extern const OSClassLoadInformation AFKUserDataPipe_Class;

class AFKUserDataPipeMetaClass : public OSMetaClass
{
public:
    virtual kern_return_t
    New(OSObject * instance) override;
    virtual kern_return_t
    Dispatch(const IORPC rpc) override;
};

#endif /* !KERNEL */

#if !KERNEL

class AFKUserDataPipeInterface : public OSInterface
{
public:
    virtual AFKUserMemoryDescriptor *
    prepareBuffer(PayloadBuffer data) = 0;

    virtual bool
    open() = 0;

    virtual void
    close(OSActionAbortedHandler handler) = 0;

    virtual void
    setReportHandler(ReportBlock reportHandler) = 0;

    virtual void
    setCommandHandler(CommandBlock commandHandler) = 0;

    virtual void
    setResponseHandler(ResponseBlock responseHandler) = 0;

    virtual IOReturn
    enqueueReport(unsigned int packetType,
        uint64_t timestamp,
        PayloadBuffer reportBuffer,
        endpoint_options options) = 0;

    virtual IOReturn
    enqueueCommand(void * context,
        unsigned int packetType,
        uint64_t timestamp,
        PayloadBuffer commandBuffer,
        PayloadBuffer responseBuffer,
        endpoint_options options,
        IOService * forClient) = 0;

    virtual IOReturn
    enqueueResponse(CommandID id,
        IOReturn result,
        uint64_t timestamp,
        PayloadBuffer responseBuffer,
        endpoint_options options) = 0;

    AFKUserMemoryDescriptor *
    prepareBuffer_Call(PayloadBuffer data)  { return prepareBuffer(data); };\

    bool
    open_Call()  { return open(); };\

    void
    close_Call(OSActionAbortedHandler handler)  { return close(handler); };\

    void
    setReportHandler_Call(ReportBlock reportHandler)  { return setReportHandler(reportHandler); };\

    void
    setCommandHandler_Call(CommandBlock commandHandler)  { return setCommandHandler(commandHandler); };\

    void
    setResponseHandler_Call(ResponseBlock responseHandler)  { return setResponseHandler(responseHandler); };\

    IOReturn
    enqueueReport_Call(unsigned int packetType,
        uint64_t timestamp,
        PayloadBuffer reportBuffer,
        endpoint_options options)  { return enqueueReport(packetType, timestamp, reportBuffer, options); };\

    IOReturn
    enqueueCommand_Call(void * context,
        unsigned int packetType,
        uint64_t timestamp,
        PayloadBuffer commandBuffer,
        PayloadBuffer responseBuffer,
        endpoint_options options,
        IOService * forClient)  { return enqueueCommand(context, packetType, timestamp, commandBuffer, responseBuffer, options, forClient); };\

    IOReturn
    enqueueResponse_Call(CommandID id,
        IOReturn result,
        uint64_t timestamp,
        PayloadBuffer responseBuffer,
        endpoint_options options)  { return enqueueResponse(id, result, timestamp, responseBuffer, options); };\

};

struct AFKUserDataPipe_IVars;
struct AFKUserDataPipe_LocalIVars;

class AFKUserDataPipe : public OSObject, public AFKUserDataPipeInterface
{
#if !KERNEL
    friend class AFKUserDataPipeMetaClass;
#endif /* !KERNEL */

#if !KERNEL
public:
#ifdef AFKUserDataPipe_DECLARE_IVARS
AFKUserDataPipe_DECLARE_IVARS
#else /* AFKUserDataPipe_DECLARE_IVARS */
    union
    {
        AFKUserDataPipe_IVars * ivars;
        AFKUserDataPipe_LocalIVars * lvars;
    };
#endif /* AFKUserDataPipe_DECLARE_IVARS */
#endif /* !KERNEL */

#if !KERNEL
    static OSMetaClass *
    sGetMetaClass() { return gAFKUserDataPipeMetaClass; };
#endif /* KERNEL */

    using super = OSObject;

#if !KERNEL
    AFKUserDataPipe_Methods
    AFKUserDataPipe_VirtualMethods
#endif /* !KERNEL */

};
#endif /* !KERNEL */


#define OSAction_AFKUserDataPipe__HandleResponse_Methods \
\
public:\
\
    virtual kern_return_t\
    Dispatch(const IORPC rpc) APPLE_KEXT_OVERRIDE;\
\
    static kern_return_t\
    _Dispatch(OSAction_AFKUserDataPipe__HandleResponse * self, const IORPC rpc);\
\
\
protected:\
    /* _Impl methods */\
\
\
public:\
    /* _Invoke methods */\
\


#define OSAction_AFKUserDataPipe__HandleResponse_KernelMethods \
\
protected:\
    /* _Impl methods */\
\


#define OSAction_AFKUserDataPipe__HandleResponse_VirtualMethods \
\
public:\
\


#if !KERNEL

extern OSMetaClass          * gOSAction_AFKUserDataPipe__HandleResponseMetaClass;
extern const OSClassLoadInformation OSAction_AFKUserDataPipe__HandleResponse_Class;

class OSAction_AFKUserDataPipe__HandleResponseMetaClass : public OSMetaClass
{
public:
    virtual kern_return_t
    New(OSObject * instance) override;
    virtual kern_return_t
    Dispatch(const IORPC rpc) override;
};

#endif /* !KERNEL */

class OSAction_AFKUserDataPipe__HandleResponseInterface : public OSInterface
{
public:
};

struct OSAction_AFKUserDataPipe__HandleResponse_IVars;
struct OSAction_AFKUserDataPipe__HandleResponse_LocalIVars;

class __attribute__((availability(driverkit,introduced=20,message="Type-safe OSAction factory methods are available in DriverKit 20 and newer"))) OSAction_AFKUserDataPipe__HandleResponse : public OSAction, public OSAction_AFKUserDataPipe__HandleResponseInterface
{
#if KERNEL
    OSDeclareDefaultStructorsWithDispatch(OSAction_AFKUserDataPipe__HandleResponse);
#endif /* KERNEL */

#if !KERNEL
    friend class OSAction_AFKUserDataPipe__HandleResponseMetaClass;
#endif /* !KERNEL */

public:
#ifdef OSAction_AFKUserDataPipe__HandleResponse_DECLARE_IVARS
OSAction_AFKUserDataPipe__HandleResponse_DECLARE_IVARS
#else /* OSAction_AFKUserDataPipe__HandleResponse_DECLARE_IVARS */
    union
    {
        OSAction_AFKUserDataPipe__HandleResponse_IVars * ivars;
        OSAction_AFKUserDataPipe__HandleResponse_LocalIVars * lvars;
    };
#endif /* OSAction_AFKUserDataPipe__HandleResponse_DECLARE_IVARS */
#if !KERNEL
    static OSMetaClass *
    sGetMetaClass() { return gOSAction_AFKUserDataPipe__HandleResponseMetaClass; };
    virtual const OSMetaClass *
    getMetaClass() const APPLE_KEXT_OVERRIDE { return gOSAction_AFKUserDataPipe__HandleResponseMetaClass; };
#endif /* KERNEL */

    using super = OSAction;

#if !KERNEL
    OSAction_AFKUserDataPipe__HandleResponse_Methods
#endif /* !KERNEL */

    OSAction_AFKUserDataPipe__HandleResponse_VirtualMethods
};

#define OSAction_AFKUserDataPipe__HandleCommand_Methods \
\
public:\
\
    virtual kern_return_t\
    Dispatch(const IORPC rpc) APPLE_KEXT_OVERRIDE;\
\
    static kern_return_t\
    _Dispatch(OSAction_AFKUserDataPipe__HandleCommand * self, const IORPC rpc);\
\
\
protected:\
    /* _Impl methods */\
\
\
public:\
    /* _Invoke methods */\
\


#define OSAction_AFKUserDataPipe__HandleCommand_KernelMethods \
\
protected:\
    /* _Impl methods */\
\


#define OSAction_AFKUserDataPipe__HandleCommand_VirtualMethods \
\
public:\
\


#if !KERNEL

extern OSMetaClass          * gOSAction_AFKUserDataPipe__HandleCommandMetaClass;
extern const OSClassLoadInformation OSAction_AFKUserDataPipe__HandleCommand_Class;

class OSAction_AFKUserDataPipe__HandleCommandMetaClass : public OSMetaClass
{
public:
    virtual kern_return_t
    New(OSObject * instance) override;
    virtual kern_return_t
    Dispatch(const IORPC rpc) override;
};

#endif /* !KERNEL */

class OSAction_AFKUserDataPipe__HandleCommandInterface : public OSInterface
{
public:
};

struct OSAction_AFKUserDataPipe__HandleCommand_IVars;
struct OSAction_AFKUserDataPipe__HandleCommand_LocalIVars;

class __attribute__((availability(driverkit,introduced=20,message="Type-safe OSAction factory methods are available in DriverKit 20 and newer"))) OSAction_AFKUserDataPipe__HandleCommand : public OSAction, public OSAction_AFKUserDataPipe__HandleCommandInterface
{
#if KERNEL
    OSDeclareDefaultStructorsWithDispatch(OSAction_AFKUserDataPipe__HandleCommand);
#endif /* KERNEL */

#if !KERNEL
    friend class OSAction_AFKUserDataPipe__HandleCommandMetaClass;
#endif /* !KERNEL */

public:
#ifdef OSAction_AFKUserDataPipe__HandleCommand_DECLARE_IVARS
OSAction_AFKUserDataPipe__HandleCommand_DECLARE_IVARS
#else /* OSAction_AFKUserDataPipe__HandleCommand_DECLARE_IVARS */
    union
    {
        OSAction_AFKUserDataPipe__HandleCommand_IVars * ivars;
        OSAction_AFKUserDataPipe__HandleCommand_LocalIVars * lvars;
    };
#endif /* OSAction_AFKUserDataPipe__HandleCommand_DECLARE_IVARS */
#if !KERNEL
    static OSMetaClass *
    sGetMetaClass() { return gOSAction_AFKUserDataPipe__HandleCommandMetaClass; };
    virtual const OSMetaClass *
    getMetaClass() const APPLE_KEXT_OVERRIDE { return gOSAction_AFKUserDataPipe__HandleCommandMetaClass; };
#endif /* KERNEL */

    using super = OSAction;

#if !KERNEL
    OSAction_AFKUserDataPipe__HandleCommand_Methods
#endif /* !KERNEL */

    OSAction_AFKUserDataPipe__HandleCommand_VirtualMethods
};

#define OSAction_AFKUserDataPipe__HandleReport_Methods \
\
public:\
\
    virtual kern_return_t\
    Dispatch(const IORPC rpc) APPLE_KEXT_OVERRIDE;\
\
    static kern_return_t\
    _Dispatch(OSAction_AFKUserDataPipe__HandleReport * self, const IORPC rpc);\
\
\
protected:\
    /* _Impl methods */\
\
\
public:\
    /* _Invoke methods */\
\


#define OSAction_AFKUserDataPipe__HandleReport_KernelMethods \
\
protected:\
    /* _Impl methods */\
\


#define OSAction_AFKUserDataPipe__HandleReport_VirtualMethods \
\
public:\
\


#if !KERNEL

extern OSMetaClass          * gOSAction_AFKUserDataPipe__HandleReportMetaClass;
extern const OSClassLoadInformation OSAction_AFKUserDataPipe__HandleReport_Class;

class OSAction_AFKUserDataPipe__HandleReportMetaClass : public OSMetaClass
{
public:
    virtual kern_return_t
    New(OSObject * instance) override;
    virtual kern_return_t
    Dispatch(const IORPC rpc) override;
};

#endif /* !KERNEL */

class OSAction_AFKUserDataPipe__HandleReportInterface : public OSInterface
{
public:
};

struct OSAction_AFKUserDataPipe__HandleReport_IVars;
struct OSAction_AFKUserDataPipe__HandleReport_LocalIVars;

class __attribute__((availability(driverkit,introduced=20,message="Type-safe OSAction factory methods are available in DriverKit 20 and newer"))) OSAction_AFKUserDataPipe__HandleReport : public OSAction, public OSAction_AFKUserDataPipe__HandleReportInterface
{
#if KERNEL
    OSDeclareDefaultStructorsWithDispatch(OSAction_AFKUserDataPipe__HandleReport);
#endif /* KERNEL */

#if !KERNEL
    friend class OSAction_AFKUserDataPipe__HandleReportMetaClass;
#endif /* !KERNEL */

public:
#ifdef OSAction_AFKUserDataPipe__HandleReport_DECLARE_IVARS
OSAction_AFKUserDataPipe__HandleReport_DECLARE_IVARS
#else /* OSAction_AFKUserDataPipe__HandleReport_DECLARE_IVARS */
    union
    {
        OSAction_AFKUserDataPipe__HandleReport_IVars * ivars;
        OSAction_AFKUserDataPipe__HandleReport_LocalIVars * lvars;
    };
#endif /* OSAction_AFKUserDataPipe__HandleReport_DECLARE_IVARS */
#if !KERNEL
    static OSMetaClass *
    sGetMetaClass() { return gOSAction_AFKUserDataPipe__HandleReportMetaClass; };
    virtual const OSMetaClass *
    getMetaClass() const APPLE_KEXT_OVERRIDE { return gOSAction_AFKUserDataPipe__HandleReportMetaClass; };
#endif /* KERNEL */

    using super = OSAction;

#if !KERNEL
    OSAction_AFKUserDataPipe__HandleReport_Methods
#endif /* !KERNEL */

    OSAction_AFKUserDataPipe__HandleReport_VirtualMethods
};

#endif /* !__DOCUMENTATION__ */


/* AFKUserDataPipe.iig:152- */

#endif /* _AFKDriverKit_AFKUserDataPipe_h */
