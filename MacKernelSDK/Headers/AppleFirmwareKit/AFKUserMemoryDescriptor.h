/* iig(DriverKit-248) generated from AFKUserMemoryDescriptor.iig */

/* AFKUserMemoryDescriptor.iig:1-23 */
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
#include <AppleFirmwareKit/AFKUserMemoryDescriptor_kext.h>
#endif
#endif

#ifndef _AFKDriverKit_AFKUserMemoryDescriptor_h
#define _AFKDriverKit_AFKUserMemoryDescriptor_h

#include <AFKDriverKit/AFKDriverKitCommon.h>
#include <DriverKit/IOBufferMemoryDescriptor.h>  /* .iig include */

/* source class AFKUserMemoryDescriptor AFKUserMemoryDescriptor.iig:24-68 */

#if __DOCUMENTATION__
#define KERNEL IIG_KERNEL

/*! @class       AFKUserMemoryDescriptor
 *  @brief       Memory descriptor object for working with an AFKEndpointInterface.
 *  @classdesign This class is currently nothing but a wrapper for an IOBufferMemoryDescriptor.
 *               Functions may be altered in the future, or new functions may be added to
 *               provide better functionality and efficiency by leveraging the AFK framework.
 */
class KERNEL AFKUserMemoryDescriptor : public OSObject
{
    using super = OSObject;

public:
    // Overridden functions, see IOService documentation for usage
    virtual bool init() override;
    virtual void free() override;

    /*!
     * @function createBuffer
     * @abstract Create a buffer object.
     * @param    options Options to specify the request. Currently
     *           no options are supported. Pass 0 to this parameter.
     * @param    capacity The desired size of the buffer.
     * @result   The successfully created and initialized memory buffer
     *           or NULL if an issue was encountered.
     */
    static AFKUserMemoryDescriptor * createBuffer(buffer_options options,
                                                    uint64_t       capacity) LOCALONLY;

    /*!
     * @function GetAddressRange
     * @abstract Retrieve the memory address and length of the data.
     * @param    range An IOAddressSegment reference that will be filled in with
     *           the proper address and length of the backing data buffer.
     * @result   kIOReturnSuccess if successful, otherwise an error code.
     *           The range parameter should not be trusted if an error is returned.
     */
    IOReturn GetAddressRange(IOAddressSegment * range) LOCALONLY;

    /*!
     * @function SetLength
     * @abstract Set the valid length of the buffer.
     * @param    length The length to declare as currently valid. Must be less
     *           than the initially declared capacity.
     * @result   kIOReturnSuccess if successful, otherwise an error code.
     */
    virtual IOReturn SetLength(uint64_t length) LOCALONLY;
};

#undef KERNEL
#else /* __DOCUMENTATION__ */

/* generated class AFKUserMemoryDescriptor AFKUserMemoryDescriptor.iig:24-68 */

#define AFKUserMemoryDescriptor_RequestBackingBuffer_ID            0x9db46207a53a702aULL
#define AFKUserMemoryDescriptor_Create_ID            0xe1059fb75746385dULL

#define AFKUserMemoryDescriptor_RequestBackingBuffer_Args \
        IOBufferMemoryDescriptor ** backing

#define AFKUserMemoryDescriptor_Create_Args \
        uint64_t options, \
        uint64_t capacity, \
        AFKUserMemoryDescriptor ** buff

#define AFKUserMemoryDescriptor_Methods \
\
public:\
\
    virtual kern_return_t\
    Dispatch(const IORPC rpc) APPLE_KEXT_OVERRIDE;\
\
    static kern_return_t\
    _Dispatch(AFKUserMemoryDescriptor * self, const IORPC rpc);\
\
    IOReturn\
    RequestBackingBuffer(\
        IOBufferMemoryDescriptor ** backing,\
        OSDispatchMethod supermethod = NULL);\
\
    static IOReturn\
    Create(\
        uint64_t options,\
        uint64_t capacity,\
        AFKUserMemoryDescriptor ** buff);\
\
    static AFKUserMemoryDescriptor *\
    createBuffer(\
        buffer_options options,\
        uint64_t capacity);\
\
    IOReturn\
    GetAddressRange(\
        IOAddressSegment * range);\
\
\
protected:\
    /* _Impl methods */\
\
\
public:\
    /* _Invoke methods */\
\
    typedef IOReturn (*RequestBackingBuffer_Handler)(OSMetaClassBase * target, AFKUserMemoryDescriptor_RequestBackingBuffer_Args);\
    static kern_return_t\
    RequestBackingBuffer_Invoke(const IORPC rpc,\
        OSMetaClassBase * target,\
        RequestBackingBuffer_Handler func);\
\
    typedef IOReturn (*Create_Handler)(AFKUserMemoryDescriptor_Create_Args);\
    static kern_return_t\
    Create_Invoke(const IORPC rpc,\
        Create_Handler func);\
\


#define AFKUserMemoryDescriptor_KernelMethods \
\
protected:\
    /* _Impl methods */\
\
    IOReturn\
    RequestBackingBuffer_Impl(AFKUserMemoryDescriptor_RequestBackingBuffer_Args);\
\
    static IOReturn\
    Create_Impl(AFKUserMemoryDescriptor_Create_Args);\
\


#define AFKUserMemoryDescriptor_VirtualMethods \
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
    virtual IOReturn\
    SetLength(\
        uint64_t length) APPLE_KEXT_OVERRIDE;\
\


#if !KERNEL

extern OSMetaClass          * gAFKUserMemoryDescriptorMetaClass;
extern const OSClassLoadInformation AFKUserMemoryDescriptor_Class;

class AFKUserMemoryDescriptorMetaClass : public OSMetaClass
{
public:
    virtual kern_return_t
    New(OSObject * instance) override;
    virtual kern_return_t
    Dispatch(const IORPC rpc) override;
};

#endif /* !KERNEL */

#if !KERNEL

class AFKUserMemoryDescriptorInterface : public OSInterface
{
public:
    virtual IOReturn
    SetLength(uint64_t length) = 0;

    IOReturn
    SetLength_Call(uint64_t length)  { return SetLength(length); };\

};

struct AFKUserMemoryDescriptor_IVars;
struct AFKUserMemoryDescriptor_LocalIVars;

class AFKUserMemoryDescriptor : public OSObject, public AFKUserMemoryDescriptorInterface
{
#if !KERNEL
    friend class AFKUserMemoryDescriptorMetaClass;
#endif /* !KERNEL */

#if !KERNEL
public:
#ifdef AFKUserMemoryDescriptor_DECLARE_IVARS
AFKUserMemoryDescriptor_DECLARE_IVARS
#else /* AFKUserMemoryDescriptor_DECLARE_IVARS */
    union
    {
        AFKUserMemoryDescriptor_IVars * ivars;
        AFKUserMemoryDescriptor_LocalIVars * lvars;
    };
#endif /* AFKUserMemoryDescriptor_DECLARE_IVARS */
#endif /* !KERNEL */

#if !KERNEL
    static OSMetaClass *
    sGetMetaClass() { return gAFKUserMemoryDescriptorMetaClass; };
#endif /* KERNEL */

    using super = OSObject;

#if !KERNEL
    AFKUserMemoryDescriptor_Methods
    AFKUserMemoryDescriptor_VirtualMethods
#endif /* !KERNEL */

};
#endif /* !KERNEL */


#endif /* !__DOCUMENTATION__ */


/* AFKUserMemoryDescriptor.iig:79- */

#endif /* _AFKDriverKit_AFKUserMemoryDescriptor_h */
