/* iig(DriverKit-248) generated from AFKEndpointInterface.iig */

/* AFKEndpointInterface.iig:1-24 */
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
#include <AppleFirmwareKit/AFKEndpointInterface.h>
#endif
#endif

#ifndef _AFKDriverKit_AFKEndpointInterface_h
#define _AFKDriverKit_AFKEndpointInterface_h

#include <AFKDriverKit/AFKUserDataPipe.h>  /* .iig include */
#include <AFKDriverKit/AFKUserMemoryDescriptor.h>  /* .iig include */
#include <DriverKit/IOService.h>  /* .iig include */

/* source class AFKEndpointInterface AFKEndpointInterface.iig:25-185 */

#if __DOCUMENTATION__
#define KERNEL IIG_KERNEL

/*! @class       AFKEndpointInterface
 *  @brief       Interface for interacting with an AFKEndpoint.
 *  @classdesign Match this class as a client and open it to begin
 *               communication with AFKEndpoints.
 */
class KERNEL AFKEndpointInterface : public IOService
{
    using super = IOService;

public:
    // Overridden functions, see IOService documentation for usage
    virtual bool init() override;
    virtual void free() override;

    /*!
     * @function open
     * @abstract Open the interface, call before using the interface.
     * @param    forClient The IOService based client of the interface.
     * @param    options Options to specify the request. Currently the only
     *           supported option is kOpenOptions_Multi_Client, to indicate
     *           if exclusive access is requested.
     * @result   bool indicating if the open was successful.
     */
    virtual bool open(IOService * forClient,
                      uint32_t    options) LOCALONLY;

    /*!
     * @function close
     * @abstract Close the interface.
     *           This will need to be called, and the provided handler block
     *           will need to be invoked before it is safe to release the interface.
     * @param    forClient The IOService based client of the interface.
     * @param    handler The interface has asynchronous tasks that need
     *           to be completed after close. This handler will be invoked
     *           when those tasks are complete and it is safe to release
     *           the interface. The handler will always be invoked.
     * @param    options Options to specify the request. Currently there
     *           are no supported options.
     * @result   bool indicating if the close was successful.
     */
    virtual bool close(IOService            * forClient,
                       OSActionAbortedHandler handler,
                       uint32_t               options) LOCALONLY;

    /*!
     * @function allocateBuffer
     * @abstract Create a memory descriptor object and prepare it for use.
                 Prefer to use this method rather than creating a descriptor
     *           manually.
     * @param    options Options to specify the request. Currently no options
     *           are supported, pass 0 for this parameter.
     * @param    capacity The desired size of the buffer.
     * @result   The successfully created and initialized memory buffer
     *           or NULL if an issue was encountered.
     */
    virtual AFKUserMemoryDescriptor * allocateBuffer(buffer_options options,
                                                     uint64_t       capacity) LOCALONLY;

    /*!
     * @function setReportHandler
     * @abstract Set the callback invoked when a report is received.
     *           Callbacks can only be set when the device is not open.
     * @param    reportHandler The callback.
     */
    virtual void setReportHandler(ReportBlock reportHandler) LOCALONLY;

    /*!
     * @function setCommandHandler
     * @abstract Set the callback invoked when a command is received.
     *           Callbacks can only be set when the device is not open.
     * @param    commandHandler The callback.
     */
    virtual void setCommandHandler(CommandBlock commandHandler) LOCALONLY;

    /*!
     * @function setResponseHandler
     * @abstract Set the callback invoked when a response is received.
     *           Callbacks can only be set when the device is not open.
     * @param    responseHandler The callback.
     */
    virtual void setResponseHandler(ResponseBlock responseHandler) LOCALONLY;

    /*! @function enqueueReport
     *  @abstract Send a report via the endpoint interface.
     *            This method will send a report packet over the endpoint interface.
     *            A report is a message that does not require an acknowledgement.
     *            The packet will either be sent immediately (blocking), or it will
     *            be enqueued and sent asynchronously depending on the endpoint
     *            implementation.
     *  @param    packetType The type of packet, implementation specific.
     *            Values less than kIOPPacketTypePrivate are reserved for internal
     *            AFK use. See AFKPacket.h.
     *  @param    timestamp Timestamp for this enqueue action. Units and domain
     *            are implementation specific.
     *  @param    reportBuffer The report to send. Can be specified as an
     *            IOBufferMemoryDescriptor or a raw buffer and size.
     *  @param    options See endpoint_options for available options.
     *  @result   kIOReturnSuccess if the enqueue was successful,
     *            an error code otherwise.
     */
    virtual IOReturn enqueueReport(unsigned         packetType,
                                   uint64_t         timestamp,
                                   PayloadBuffer    reportBuffer,
                                   endpoint_options options = 0) LOCALONLY;

    /*! @function enqueueCommand
     *  @abstract Send a command via the endpoint interface.
     *            This method will send a command packet over the endpoint
     *            interface. A command is a message that requires an
     *            acknowledgement. The packet will either be sent immediately
     *            (blocking), or it will be enqueued and sent asynchronously
     *            depending on the endpoint implementation.
     *  @param    context Caller defined context, passed to the response
     *            handler when a response is received for this command.
     *  @param    packetType The type of packet, implementation specific.
     *            Values less than kIOPPacketTypePrivate are reserved for
     *            internal AFK use. See AFKPacket.h.
     *  @param    timestamp Timestamp for this enqueue action. Units and
     *            domain are implementation-specific.
     *  @param    commandBuffer The command to send. Can be specified
     *            as an IOBufferMemoryDescriptor or a raw buffer and size.
     *  @param    responseBuffer The size field should contain the expected
     *            (maximum) response size, other fields are not used.
     *  @param    options See endpoint_options for available options.
     *  @param    forClient Reference to the calling IOSerivce, required if
     *            the interface has been opened with kOpenOptions_Multi_Client.
     *  @result   kIOReturnSuccess if the enqueue was successful,
     *            an error code otherwise.
     */
    virtual IOReturn enqueueCommand(void           * context,
                                    unsigned         packetType,
                                    uint64_t         timestamp,
                                    PayloadBuffer    commandBuffer,
                                    PayloadBuffer    responseBuffer,
                                    endpoint_options options = 0,
                                    IOService      * forClient = nullptr) LOCALONLY;

    /*! @function enqueueResponse
     *  @abstract Send a response via the endpoint interface.
     *            This method will send a response packet over the endpoint
     *            interface. A response is a message sent in response to a
     *            command received via the command handler. The packet
     *            will either be sent immediately (blocking), or it will be
     *            enqueued and sent asynchronously depending on the
     *            endpoint implementation.
     *  @param    id CommandID that was previously passed
     *            to the command handler.
     *  @param    result Result of the associated command.
     *  @param    timestamp Timestamp for this enqueue action. Units and
     *            domain are implementation-specific.
     *  @param    responseBuffer The response to send. Can be specified as an
     *            IOBufferMemoryDescriptor or a raw buffer and size.
     *  @param    options See endpoint_options for available options.
     *  @return   kIOReturnSuccess if the enqueue was successful,
     *            an error code otherwise.
     */
    virtual IOReturn enqueueResponse(CommandID        id,
                                     IOReturn         result,
                                     uint64_t         timestamp,
                                     PayloadBuffer    responseBuffer,
                                     endpoint_options options = 0) LOCALONLY;
};

#undef KERNEL
#else /* __DOCUMENTATION__ */

/* generated class AFKEndpointInterface AFKEndpointInterface.iig:25-185 */

#define AFKEndpointInterface_Close_ID            0x8b3103b608452ec4ULL
#define AFKEndpointInterface_Open_ID            0xabcd77e378998de1ULL

#define AFKEndpointInterface_Close_Args \
        IOService * forClient, \
        uint32_t options

#define AFKEndpointInterface_Open_Args \
        IOService * forClient, \
        AFKUserDataPipe ** pipe, \
        uint32_t options

#define AFKEndpointInterface_Methods \
\
public:\
\
    virtual kern_return_t\
    Dispatch(const IORPC rpc) APPLE_KEXT_OVERRIDE;\
\
    static kern_return_t\
    _Dispatch(AFKEndpointInterface * self, const IORPC rpc);\
\
    kern_return_t\
    Close(\
        IOService * forClient,\
        uint32_t options,\
        OSDispatchMethod supermethod = NULL);\
\
    kern_return_t\
    Open(\
        IOService * forClient,\
        AFKUserDataPipe ** pipe,\
        uint32_t options,\
        OSDispatchMethod supermethod = NULL);\
\
\
protected:\
    /* _Impl methods */\
\
\
public:\
    /* _Invoke methods */\
\
    typedef kern_return_t (*Close_Handler)(OSMetaClassBase * target, AFKEndpointInterface_Close_Args);\
    static kern_return_t\
    Close_Invoke(const IORPC rpc,\
        OSMetaClassBase * target,\
        Close_Handler func);\
\
    typedef kern_return_t (*Open_Handler)(OSMetaClassBase * target, AFKEndpointInterface_Open_Args);\
    static kern_return_t\
    Open_Invoke(const IORPC rpc,\
        OSMetaClassBase * target,\
        Open_Handler func);\
\


#define AFKEndpointInterface_KernelMethods \
\
protected:\
    /* _Impl methods */\
\
    kern_return_t\
    Close_Impl(AFKEndpointInterface_Close_Args);\
\
    kern_return_t\
    Open_Impl(AFKEndpointInterface_Open_Args);\
\


#define AFKEndpointInterface_VirtualMethods \
\
public:\
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
        IOService * forClient,\
        uint32_t options) APPLE_KEXT_OVERRIDE;\
\
    virtual bool\
    close(\
        IOService * forClient,\
        OSActionAbortedHandler handler,\
        uint32_t options) APPLE_KEXT_OVERRIDE;\
\
    virtual AFKUserMemoryDescriptor *\
    allocateBuffer(\
        buffer_options options,\
        uint64_t capacity) APPLE_KEXT_OVERRIDE;\
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

extern OSMetaClass          * gAFKEndpointInterfaceMetaClass;
extern const OSClassLoadInformation AFKEndpointInterface_Class;

class AFKEndpointInterfaceMetaClass : public OSMetaClass
{
public:
    virtual kern_return_t
    New(OSObject * instance) override;
    virtual kern_return_t
    Dispatch(const IORPC rpc) override;
};

#endif /* !KERNEL */

#if !KERNEL

class AFKEndpointInterfaceInterface : public OSInterface
{
public:
    virtual bool
    open(IOService * forClient,
        uint32_t options) = 0;

    virtual bool
    close(IOService * forClient,
        OSActionAbortedHandler handler,
        uint32_t options) = 0;

    virtual AFKUserMemoryDescriptor *
    allocateBuffer(buffer_options options,
        uint64_t capacity) = 0;

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

    bool
    open_Call(IOService * forClient,
        uint32_t options)  { return open(forClient, options); };\

    bool
    close_Call(IOService * forClient,
        OSActionAbortedHandler handler,
        uint32_t options)  { return close(forClient, handler, options); };\

    AFKUserMemoryDescriptor *
    allocateBuffer_Call(buffer_options options,
        uint64_t capacity)  { return allocateBuffer(options, capacity); };\

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

struct AFKEndpointInterface_IVars;
struct AFKEndpointInterface_LocalIVars;

class AFKEndpointInterface : public IOService, public AFKEndpointInterfaceInterface
{
#if !KERNEL
    friend class AFKEndpointInterfaceMetaClass;
#endif /* !KERNEL */

#if !KERNEL
public:
#ifdef AFKEndpointInterface_DECLARE_IVARS
AFKEndpointInterface_DECLARE_IVARS
#else /* AFKEndpointInterface_DECLARE_IVARS */
    union
    {
        AFKEndpointInterface_IVars * ivars;
        AFKEndpointInterface_LocalIVars * lvars;
    };
#endif /* AFKEndpointInterface_DECLARE_IVARS */
#endif /* !KERNEL */

#if !KERNEL
    static OSMetaClass *
    sGetMetaClass() { return gAFKEndpointInterfaceMetaClass; };
#endif /* KERNEL */

    using super = IOService;

#if !KERNEL
    AFKEndpointInterface_Methods
    AFKEndpointInterface_VirtualMethods
#endif /* !KERNEL */

};
#endif /* !KERNEL */


#endif /* !__DOCUMENTATION__ */

/* AFKEndpointInterface.iig:187-188 */

// Internally used private functions
/* AFKEndpointInterface.iig:198- */

#endif /* _AFKDriverKit_AFKEndpointInterface_h */
