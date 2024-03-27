/* iig(DriverKit-73.140.1) generated from IODispatchQueue.iig */

/* IODispatchQueue.iig:1-41 */
/*
 * Copyright (c) 2019-2019 Apple Inc. All rights reserved.
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

#ifndef _IOKIT_UIODISPATCHQUEUE_H
#define _IOKIT_UIODISPATCHQUEUE_H

#include <DriverKit/OSObject.h>  /* .iig include */
#include <DriverKit/OSAction.h>  /* .iig include */
#include <DriverKit/IODispatchSource.h>  /* .iig include */

typedef int (*IODispatchLogFunction)(const char *format, ...);
typedef void (^IODispatchBlock)(void);
typedef void (*IODispatchFunction)(void * context);
typedef void (^IODispatchQueueCancelHandler)(void);


/* source class IODispatchQueue IODispatchQueue.iig:42-134 */

#if __DOCUMENTATION__
#define KERNEL IIG_KERNEL

/*!
 * @class IODispatchQueue
 *
 * @abstract
 * IODispatchQueue provides a queue for ordered execution of blocks.
 *
 * @discussion
 * All blocks submitted to dispatch queues are dequeued in FIFO order.
 * By default the queue is serial and will execute one block at a time.
 */

class NATIVE KERNEL IODispatchQueue : public OSObject
{
public:
    /*!
     * @brief       Creates a new dispatch queue object.
     * @discussion  Creates a new dispatch queue object. All queues are currently serial, executing one block at time
     *              FIFO order. The new object has retain count 1 and should be released by the caller.
     * @param       options No options are currently defined, pass zero.
     * @param       priority No priorities are currently defined, pass zero.
     * @return      kIOReturnSuccess on success. See IOReturn.h for error codes.
     */
	static kern_return_t
	Create(
		const IODispatchQueueName name,
		uint64_t                  options,
		uint64_t                  priority,
		IODispatchQueue        ** queue) LOCAL;

	virtual bool
	init() override;

	virtual void
	free() override;

    /*!
     * @brief       Determines if the current thread is running on the queue.
     * @discussion  Determines if the current thread is running on the queue, including if the queue invoked a
     *              second queue (ie. OnQueue can return true for more than one queue in a given context.)
     * @return      bool true if current thread is running on this queue.
     */
	bool
	OnQueue() LOCALONLY;

    /*!
     * @brief       Return the name the queue was created with.
     * @discussion  Returns a pointer to the queues name. Only valid while the queue is retained.
     * @return      C-string pointer in the queues internal storage.
     */
	const char *
	GetName() LOCALONLY;

    /*!
     * @brief       Stop the queue from executing futher work.
     * @discussion  Stops the queue from dequeuing work, and on completion of any block currently being executed,
     *              invokes a callback block. Canceling is asynchronous.
     * @param       handler Block that will executed when the queue has completed any inflight work 
     *              and will not execute further work.
     * @return      C-string pointer in the queues internal storage.
     */
	kern_return_t
	Cancel(IODispatchQueueCancelHandler handler) LOCALONLY;

    /*!
     * @brief       Schedule a block to be executed on the queue asynchronously.
     * @discussion  Schedules work to be done on the queue without waiting for it to complete. The queue will be
     *              retained until the block completes.
     * @param       block Block that will executed on the queue, not in the context of the caller.
     */
	void
	DispatchAsync(IODispatchBlock block) LOCALONLY;

    /*!
     * @brief       C-function callback version of DispatchAsync.
	 */
	void
	DispatchAsync_f(void * context, IODispatchFunction function) LOCALONLY;

	void
	DispatchSync(IODispatchBlock block) LOCALONLY;

    /*!
     * @brief       C-function callback version of DispatchSync.
	 */
	void
	DispatchSync_f(void * context, IODispatchFunction function) LOCALONLY;

    /*!
     * @brief       Log the current execution context with respect to any queues the current thread holds.
     * @param       output printf like output function. The address of IOLog is suitable to be used.
	 */
	static void
	Log(const char * message, IODispatchLogFunction output) LOCALONLY;
};

#undef KERNEL
#else /* __DOCUMENTATION__ */

/* generated class IODispatchQueue IODispatchQueue.iig:42-134 */

#define IODispatchQueue_SetPort_ID            0xc437e970b5609767ULL
#define IODispatchQueue_Create_ID            0xac000428df2a91d0ULL

#define IODispatchQueue_SetPort_Args \
        mach_port_t port

#define IODispatchQueue_Create_Args \
        const char * name, \
        uint64_t options, \
        uint64_t priority, \
        IODispatchQueue ** queue

#define IODispatchQueue_Methods \
\
public:\
\
    virtual kern_return_t\
    Dispatch(const IORPC rpc) APPLE_KEXT_OVERRIDE;\
\
    static kern_return_t\
    _Dispatch(IODispatchQueue * self, const IORPC rpc);\
\
    kern_return_t\
    SetPort(\
        mach_port_t port,\
        OSDispatchMethod supermethod = NULL);\
\
    static kern_return_t\
    Create(\
        const IODispatchQueueName name,\
        uint64_t options,\
        uint64_t priority,\
        IODispatchQueue ** queue);\
\
    bool\
    OnQueue(\
);\
\
    const char *\
    GetName(\
);\
\
    kern_return_t\
    Cancel(\
        IODispatchQueueCancelHandler handler);\
\
    void\
    DispatchAsync(\
        IODispatchBlock block);\
\
    void\
    DispatchAsync_f(\
        void * context,\
        IODispatchFunction function);\
\
    void\
    DispatchSync(\
        IODispatchBlock block);\
\
    void\
    DispatchSync_f(\
        void * context,\
        IODispatchFunction function);\
\
    static void\
    Log(\
        const char * message,\
        IODispatchLogFunction output);\
\
\
protected:\
    /* _Impl methods */\
\
    static kern_return_t\
    Create_Call(IODispatchQueue_Create_Args);\
\
\
public:\
    /* _Invoke methods */\
\
    typedef kern_return_t (*SetPort_Handler)(OSMetaClassBase * target, IODispatchQueue_SetPort_Args);\
    static kern_return_t\
    SetPort_Invoke(const IORPC rpc,\
        OSMetaClassBase * target,\
        SetPort_Handler func);\
\
    typedef kern_return_t (*Create_Handler)(IODispatchQueue_Create_Args);\
    static kern_return_t\
    Create_Invoke(const IORPC rpc,\
        Create_Handler func);\
\


#define IODispatchQueue_KernelMethods \
\
protected:\
    /* _Impl methods */\
\
    kern_return_t\
    SetPort_Impl(IODispatchQueue_SetPort_Args);\
\
    static kern_return_t\
    Create_Impl(IODispatchQueue_Create_Args);\
\


#define IODispatchQueue_VirtualMethods \
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



class IODispatchQueueInterface : public OSInterface
{
public:
};

struct IODispatchQueue_IVars;
struct IODispatchQueue_LocalIVars;

class IODispatchQueue : public OSObject, public IODispatchQueueInterface
{
    OSDeclareDefaultStructorsWithDispatch(IODispatchQueue);


public:
    union
    {
        IODispatchQueue_IVars * ivars;
        IODispatchQueue_LocalIVars * lvars;
    };

    using super = OSObject;


    IODispatchQueue_VirtualMethods
};

#endif /* !__DOCUMENTATION__ */

/* IODispatchQueue.iig:136-137 */

#if DRIVERKIT_PRIVATE
/* IODispatchQueue.iig:144- */
#endif

#endif /* ! _IOKIT_UIODISPATCH_H */
