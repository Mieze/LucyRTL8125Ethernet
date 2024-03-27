/* iig(DriverKit-73.100.4) generated from IOUserUSBHostHIDDevice.iig */

/* IOUserUSBHostHIDDevice.iig:1-41 */
/*
 * Copyright (c) 1998-2006 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef _HIDDRIVERKIT_IOUSERUSBHOSTHIDDEVICE_H
#define _HIDDRIVERKIT_IOUSERUSBHOSTHIDDEVICE_H

#include <USBDriverKit/IOUSBHostInterface.h>  /* .iig include */
#include <USBDriverKit/IOUSBHostPipe.h>  /* .iig include */
#include <DriverKit/OSAction.h>  /* .iig include */
#include <DriverKit/IOService.h>  /* .iig include */
#include <DriverKit/IOTimerDispatchSource.h>  /* .iig include */
#include <HIDDriverKit/IOUserHIDDevice.h>  /* .iig include */

class  OSString;
struct IOUSBHostHIDDescriptorInfo;

typedef enum {
    USBIdlePolicyTypeInterface,
    USBIdlePolicyTypePipe,
} USBIdlePolicyType;

/* source class IOUserUSBHostHIDDevice IOUserUSBHostHIDDevice.iig:42-333 */

#if __DOCUMENTATION__
#define KERNEL IIG_KERNEL

class IOUserUSBHostHIDDevice : public IOUserHIDDevice
{
    
public:

    /*!
     * @function    init
     * @brief       init IOUserUSBHostHIDDevice object
     * @discussion  method can be use to initlialize object data.
     * @return      true if successful.
     */

    virtual bool init() override;
    
    /*!
     * @function    Start
     * @brief       Service start
     * @discussion  method called when service instantiated.
     *              concrete class required to call <code>RegisterService</code> to activate
                    the service. If <code>Start</code> method of concrete class returning error
                    it should not call <code>RegisterService</code> and should call <code>Stop</code>.
     * @param       provider The IOService that is provider for this object.
     * @return      KERN_SUCCESS is successful see IOReturn.h for error codes.
     */

    virtual kern_return_t Start(IOService * provider) override;

    /*!
     * @function    Stop
     * @brief       Service stop
     * @discussion  method called when device terminating.
     * @param       provider The IOService that is provider for this object.
     * @return      KERN_SUCCESS is successful see IOReturn.h for error codes.
     */
    
    virtual kern_return_t Stop (IOService * provider) override;


 
    /*!
     * @function    free
     * @brief       free service resources
     * @discussion  method called when last reference to object removed.
     */

    virtual void free() override;

    /*!
     * @function    handleStart
     * @brief       Handle service start
     * @discussion  method called from <code>Start</code> and can be use to initlialize device.
     * @param       provider The IOService that is provider for this object.
     * @return      true on success.
     */

    virtual bool handleStart (IOService * provider) override;

    /*!
     * @function newDeviceDescription
     * @abstract Create and return a new dicitonary that describe the device HID device
     * @discussion A subclass must override this pure virtual function, and
     * return a dictionary of key value pairs that describe device.
     * Supported keys:
     *  kIOHIDReportIntervalKey
     *  kIOHIDVendorIDKey
     *  kIOHIDProductIDKey
     *  kIOHIDTransportKey
     *  kIOHIDVersionNumberKey
     *  kIOHIDCountryCodeKey
     *  kIOHIDLocationIDKey
     *  kIOHIDManufacturerKey
     *  kIOHIDProductKey
     *  kIOHIDSerialNumberKey
     *  @param description Ponter to the description dictionary pointer
     *  @result OSDictionary that contains device deccription dicitonary.
     */
    
    virtual OSDictionary * newDeviceDescription () override;
   
    /*!
     * @function newReportDescriptor
     * @abstract Create and return a new memory descriptor that describes the
     * report descriptor for the HID device.
     * @discussion A subclass must override this pure virtual function, and
     * return a memory descriptor that describes the HID report descriptor as
     * defined by the USB Device Class Definition for Human Interface Devices
     * Version 1.1 specification.
     * @result OSData object that contains HID descriptor.
     */

    virtual OSData * newReportDescriptor() override;
    
    /*!
     @function getReport
     @abstract Get a report from the HID device.
     @discussion A completion parameter may be added in the future.
     @param report A memory descriptor that describes the memory to store
     the report read from the HID device.
     @param reportType The report type.
     @param options The lower 8 bits will represent the Report ID.  The
     other 24 bits are options to specify the request.
     @param completionTimeout Specifies an amount of time (in ms) after which
     the command will be aborted if the entire command has not been completed.
     @param completion Function to call when request completes. If omitted then
     getReport() executes synchronously, blocking until the request is complete.
     @result kIOReturnSuccess on success, or an error return otherwise.
     */
    
    virtual kern_return_t getReport(IOMemoryDescriptor      * report,
                                    IOHIDReportType         reportType,
                                    IOOptionBits            options,
                                    uint32_t                 completionTimeout,
                                    OSAction                * action) override;
    
    /*!
     * @function setReport
     * @abstract Send a report to the HID device.
     * @discussion A completion parameter may be added in the future.
     * @param report A memory descriptor that describes the report to send
     * to the HID device.
     * @param reportType The report type.
     * @param options The lower 8 bits will represent the Report ID.  The
     * other 24 bits are options to specify the request.
     * @param completionTimeout Specifies an amount of time (in ms) after which
     * the command will be aborted if the entire command has not been completed.
     * @param action Action to call CompleteReport when request completes. If omitted then
     * setReport() executes synchronously, blocking until the request is complete.
     * @result kIOReturnSuccess on success, or an error return otherwise.
     */
    
    virtual kern_return_t setReport(IOMemoryDescriptor      * report,
                                    IOHIDReportType         reportType,
                                    IOOptionBits            options,
                                    uint32_t                completionTimeout,
                                    OSAction                * action) override;

    /*!
     * @function setProtocol
     * @abstract Set protocol
     * @param protocol value.
     * @result kIOReturnSuccess on success, or an error return otherwise.
     */
    
    virtual kern_return_t  setProtocol (uint16_t protocol) LOCALONLY;

protected:

    /*!
     * @function initInputReport
     * @abstract initialize read of interrupt report
     * @result kIOReturnSuccess on success, or an error return otherwise.
     */
    
    virtual kern_return_t initInputReport () LOCALONLY;

    /*!
     * @function scheduleInputReportRetry
     * @abstract initialize logic that retries interrupt report init
     */
    
    virtual void scheduleInputReportRetry (kern_return_t reason) LOCALONLY;

    /*!
     * @function cancelInputReportRetry
     * @abstract cancel logic that retries interrupt report init
     */
    
    virtual void cancelInputReportRetry () LOCALONLY;


    /*!
     * @function CompleteInputReport
     * @abstract Complete input report read
     * @discussion Function call upon read completion of interrupt reports.
     * <code>action</code> reference carry <code>IOUSBHIDCompletion</code>
     * associated with read request
     * @param action completion action
     * @param status completion status
     * @param actualByteCount number of bytes read
     */

    virtual void CompleteInputReport (OSAction      * action TARGET,
                                      IOReturn      status,
                                      uint32_t      actualByteCount,
                                      uint64_t      completionTimestamp) TYPE(IOUSBHostPipe::CompleteAsyncIO);

    
    /*!
     * @function TimerOccurred
     * @abstract Timer handler
     * @discussion Hadler called to retry scheduling input report read
     * @param action timer action
     * @param time time
     */
    
    virtual void TimerOccurred  (OSAction       * action,
                                 uint64_t       time) TYPE(IOTimerDispatchSource::TimerOccurred);

    /*!
     * @function setIdle
     * @abstract Set idle time value on device.
     * @discussion The idle rate determines how often a device resends data that
     * has not changed since the last report. This is used to limit the reporting
     * frequency of an interrupt IN endpoint.
     * @param idleTimeMs Idle time in millisecond
     * @result kIOReturnSuccess on success, or an error return otherwise.
     */

    virtual kern_return_t setIdle (uint16_t idleTimeMs) LOCALONLY;

    
    /*!
     * @function setIdlePolicy
     * @abstract Set idle timeout value for interface/pipe.
     * @discussion Once the interface/pipe is considered idle, it will defer
     * electrical suspend of the device for the specified duration
     * @param idleTimeMs Idle time in millisecond
     * @result kIOReturnSuccess on success, or an error return otherwise.
     */
    
    virtual kern_return_t setIdlePolicy (USBIdlePolicyType type, uint16_t idleTimeMs) LOCALONLY;

    /*!
     * @function reset
     * @abstract Reset USB device
     * @discussion reset  USB device if fail to communicate
     */
    
    virtual void reset () LOCALONLY;

    /*!
     * @function getReport
     * @abstract Get a report from the HID device.
     * @param report A memory descriptor that describes the memory to store
     * the report read from the HID device.
     * @param reportType The report type.
     * @param options The lower 8 bits will represent the Report ID.  The
     * other 24 bits are options to specify the request.
     * @param completionTimeout Specifies an amount of time (in ms) after which
     * the command will be aborted if the entire command has not been completed.
     * @param completion Function to call when request completes. If omitted then
     * getReport() executes synchronously, blocking until the request is complete.
     * @result kIOReturnSuccess on success, or an error return otherwise.
     */

    kern_return_t getReport (IOMemoryDescriptor      * report,
                             IOHIDReportType         reportType,
                             IOOptionBits            options,
                             uint32_t                completionTimeout,
                             uint32_t                * bytesTransferred) LOCALONLY;
    
    /*!
     * @function setReport
     * @abstract Syncronous send a report to the HID device.
     * @param report A memory descriptor that describes the report to send
     * to the HID device.
     * @param reportType The report type.
     * @param options The lower 8 bits will represent the Report ID.  The
     * other 24 bits are options to specify the request.
     * @param completionTimeout Specifies an amount of time (in ms) after which
     * the command will be aborted if the entire command has not been completed.
     * @result kIOReturnSuccess on success, or an error return otherwise.
     */

    kern_return_t setReport (IOMemoryDescriptor      * report,
                             IOHIDReportType         reportType,
                             IOOptionBits            options,
                             uint32_t                completionTimeout) LOCALONLY;
    
    
    /*!
     * @function setProperty
     * @abstract Pass property from corresponding kernel object
     * @param key property key.
     * @param value property value.
     */

    virtual void setProperty (OSObject * key, OSObject * value) override;

private:
    
    kern_return_t initPipes () LOCALONLY;
    
    kern_return_t getHIDDescriptorInfo (uint8_t type, const IOUSBHostHIDDescriptorInfo ** info, uint8_t * index) LOCALONLY;
    
    OSString * copyStringAtIndex (uint8_t index, uint16_t lang) LOCALONLY;
    
    virtual void CompleteZLP (OSAction       * action TARGET,
                              IOReturn       status,
                              uint32_t       actualByteCount,
                              uint64_t       completionTimestamp) TYPE(IOUSBHostPipe::CompleteAsyncIO);

};

#undef KERNEL
#else /* __DOCUMENTATION__ */

/* generated class IOUserUSBHostHIDDevice IOUserUSBHostHIDDevice.iig:42-333 */

#define IOUserUSBHostHIDDevice_CompleteInputReport_ID            0x40e1addf867de565ULL
#define IOUserUSBHostHIDDevice_TimerOccurred_ID            0x2a91ee629fd82a2dULL
#define IOUserUSBHostHIDDevice_CompleteZLP_ID            0x6b7467ea496a70d2ULL

#define IOUserUSBHostHIDDevice_Start_Args \
        IOService * provider

#define IOUserUSBHostHIDDevice_Stop_Args \
        IOService * provider

#define IOUserUSBHostHIDDevice_CompleteInputReport_Args \
        OSAction * action, \
        IOReturn status, \
        uint32_t actualByteCount, \
        uint64_t completionTimestamp

#define IOUserUSBHostHIDDevice_TimerOccurred_Args \
        OSAction * action, \
        uint64_t time

#define IOUserUSBHostHIDDevice_CompleteZLP_Args \
        OSAction * action, \
        IOReturn status, \
        uint32_t actualByteCount, \
        uint64_t completionTimestamp

#define IOUserUSBHostHIDDevice_Methods \
\
public:\
\
    virtual kern_return_t\
    Dispatch(const IORPC rpc) APPLE_KEXT_OVERRIDE;\
\
    static kern_return_t\
    _Dispatch(IOUserUSBHostHIDDevice * self, const IORPC rpc);\
\
    kern_return_t\
    CreateActionCompleteInputReport(size_t referenceSize, OSAction ** action);\
\
    kern_return_t\
    CreateActionTimerOccurred(size_t referenceSize, OSAction ** action);\
\
    kern_return_t\
    getReport(\
        IOMemoryDescriptor * report,\
        IOHIDReportType reportType,\
        IOOptionBits options,\
        uint32_t completionTimeout,\
        uint32_t * bytesTransferred);\
\
    kern_return_t\
    setReport(\
        IOMemoryDescriptor * report,\
        IOHIDReportType reportType,\
        IOOptionBits options,\
        uint32_t completionTimeout);\
\
    kern_return_t\
    initPipes(\
);\
\
    kern_return_t\
    getHIDDescriptorInfo(\
        uint8_t type,\
        const IOUSBHostHIDDescriptorInfo ** info,\
        uint8_t * index);\
\
    OSString *\
    copyStringAtIndex(\
        uint8_t index,\
        uint16_t lang);\
\
    kern_return_t\
    CreateActionCompleteZLP(size_t referenceSize, OSAction ** action);\
\
\
protected:\
    /* _Impl methods */\
\
    kern_return_t\
    Start_Impl(IOService_Start_Args);\
\
    kern_return_t\
    Stop_Impl(IOService_Stop_Args);\
\
    void\
    CompleteInputReport_Impl(IOUserUSBHostHIDDevice_CompleteInputReport_Args);\
\
    void\
    TimerOccurred_Impl(IOUserUSBHostHIDDevice_TimerOccurred_Args);\
\
    void\
    CompleteZLP_Impl(IOUserUSBHostHIDDevice_CompleteZLP_Args);\
\
\
public:\
    /* _Invoke methods */\
\


#define IOUserUSBHostHIDDevice_KernelMethods \
\
protected:\
    /* _Impl methods */\
\


#define IOUserUSBHostHIDDevice_VirtualMethods \
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
    handleStart(\
        IOService * provider) APPLE_KEXT_OVERRIDE;\
\
    virtual OSDictionary *\
    newDeviceDescription(\
) APPLE_KEXT_OVERRIDE;\
\
    virtual OSData *\
    newReportDescriptor(\
) APPLE_KEXT_OVERRIDE;\
\
    virtual kern_return_t\
    getReport(\
        IOMemoryDescriptor * report,\
        IOHIDReportType reportType,\
        IOOptionBits options,\
        uint32_t completionTimeout,\
        OSAction * action) APPLE_KEXT_OVERRIDE;\
\
    virtual kern_return_t\
    setReport(\
        IOMemoryDescriptor * report,\
        IOHIDReportType reportType,\
        IOOptionBits options,\
        uint32_t completionTimeout,\
        OSAction * action) APPLE_KEXT_OVERRIDE;\
\
    virtual kern_return_t\
    setProtocol(\
        uint16_t protocol) APPLE_KEXT_OVERRIDE;\
\
    virtual kern_return_t\
    initInputReport(\
) APPLE_KEXT_OVERRIDE;\
\
    virtual void\
    scheduleInputReportRetry(\
        kern_return_t reason) APPLE_KEXT_OVERRIDE;\
\
    virtual void\
    cancelInputReportRetry(\
) APPLE_KEXT_OVERRIDE;\
\
    virtual kern_return_t\
    setIdle(\
        uint16_t idleTimeMs) APPLE_KEXT_OVERRIDE;\
\
    virtual kern_return_t\
    setIdlePolicy(\
        USBIdlePolicyType type,\
        uint16_t idleTimeMs) APPLE_KEXT_OVERRIDE;\
\
    virtual void\
    reset(\
) APPLE_KEXT_OVERRIDE;\
\
    virtual void\
    setProperty(\
        OSObject * key,\
        OSObject * value) APPLE_KEXT_OVERRIDE;\
\


#if !KERNEL

extern OSMetaClass          * gIOUserUSBHostHIDDeviceMetaClass;
extern const OSClassLoadInformation IOUserUSBHostHIDDevice_Class;

class IOUserUSBHostHIDDeviceMetaClass : public OSMetaClass
{
public:
    virtual kern_return_t
    New(OSObject * instance) override;
    virtual kern_return_t
    Dispatch(const IORPC rpc) override;
};

#endif /* !KERNEL */

#if !KERNEL

class IOUserUSBHostHIDDeviceInterface : public OSInterface
{
public:
    virtual kern_return_t
    setProtocol(uint16_t protocol) = 0;

    virtual kern_return_t
    initInputReport() = 0;

    virtual void
    scheduleInputReportRetry(kern_return_t reason) = 0;

    virtual void
    cancelInputReportRetry() = 0;

    virtual kern_return_t
    setIdle(uint16_t idleTimeMs) = 0;

    virtual kern_return_t
    setIdlePolicy(USBIdlePolicyType type,
        uint16_t idleTimeMs) = 0;

    virtual void
    reset() = 0;

};

struct IOUserUSBHostHIDDevice_IVars;
struct IOUserUSBHostHIDDevice_LocalIVars;

class IOUserUSBHostHIDDevice : public IOUserHIDDevice, public IOUserUSBHostHIDDeviceInterface
{
#if !KERNEL
    friend class IOUserUSBHostHIDDeviceMetaClass;
#endif /* !KERNEL */

#if !KERNEL
public:
    union
    {
        IOUserUSBHostHIDDevice_IVars * ivars;
        IOUserUSBHostHIDDevice_LocalIVars * lvars;
    };
#endif /* !KERNEL */

    using super = IOUserHIDDevice;

#if !KERNEL
    IOUserUSBHostHIDDevice_Methods
    IOUserUSBHostHIDDevice_VirtualMethods
#endif /* !KERNEL */

};
#endif /* !KERNEL */


#endif /* !__DOCUMENTATION__ */

/* IOUserUSBHostHIDDevice.iig:335- */

#endif	// _HIDDRIVERKIT_IOUSERUSBHOSTHIDDEVICE_H
