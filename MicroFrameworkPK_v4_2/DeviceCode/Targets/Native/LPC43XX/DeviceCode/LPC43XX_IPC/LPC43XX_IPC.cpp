/*
 * LPC43XX_IPC.cpp - IPC functions for NXP LPC43XX
 *
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 *
 * Simplified version of NXP IPC APIs
 * 05/15/13  Micromint USA <support@micromint.com>
 */
#include <tinyhal.h>
#include "LPC43XX.h"
#include "LPC43XX_IPC.h"

// IPC messages allow two way communications between the
// master M4 core and the slave M0 core.
// Helper function allows loading binaries in the slave core

#define IPC_QUEUE_SZ          64

#define 	SHARED_MEM_M0   0x20000000
#define 	SHARED_MEM_M4   0x20000020

#define SHMEMM0     LOCATE_AT(SHARED_MEM_M0)
#define SHMEMM4     LOCATE_AT(SHARED_MEM_M4)

SHMEMM0 static struct ipc_queue queue_m0 = {0};
SHMEMM4 static struct ipc_queue queue_m4 = {0};

#define IPC_IRQ_Priority    IRQ_PRIO_IPC

static struct ipc_queue *qrd = &queue_m0;
static struct ipc_queue *qwr = &queue_m4;

#define IPC_IRQHandler MX_CORE_IRQHandler
#define ClearTXEvent   Chip_CREG_ClearM0Event
#define IPC_IRQn       M0CORE_IRQn

typedef struct __ipc0_msg {
	uint32_t id;
	uint32_t data;
} ipc0_msg_t;

static ipc0_msg_t ipc0_queue[IPC_QUEUE_SZ];

// Local functions
static void ipc_send_signal(void);

// ---------------------------------------------------------------------------
static void ipc_send_signal(void)
{
	__DSB();
	__SEV();
}

// ---------------------------------------------------------------------------
// Initialize IPC driver
BOOL IPC_Initialize()
{
    IPC_initMsgQueue(ipc0_queue, sizeof(ipc0_msg_t), IPC_QUEUE_SZ);

    return TRUE;
}

// ---------------------------------------------------------------------------
// Terminate IPC driver
BOOL IPC_Uninitialize()
{
    return TRUE;
}

// ---------------------------------------------------------------------------
// Initialize the IPC message queue
void IPC_initMsgQueue(void *data, int size, int count)
{
    // Sanity Check
    if (!size || !count || !data)
    {
        //DEBUGSTR("ERROR:IPC Queue size invalid parameters\r\n");
        while (1) {}
    }

    // Check if size is a power of 2
    if (count & (count - 1))
    {
        //DEBUGSTR("ERROR:IPC Queue size not power of 2\r\n");
        while (1) {	// BUG: Size must always be power of 2
        }
    }

    memset(qwr, 0, sizeof(*qwr));
    //ipc_misc_init();
    qwr->count = count;
    qwr->size = size;
    qwr->data = (uint8_t*)data;
    qwr->valid = QUEUE_MAGIC_VALID;
    NVIC_SetPriority(IPC_IRQn, IPC_IRQ_Priority);
    NVIC_EnableIRQ(IPC_IRQn);
}

// ---------------------------------------------------------------------------
// Push a message into queue with timeout
int IPC_pushMsgTout(const void *data, int tout)
{
    // Check if write queue is initialized
    if (!QUEUE_IS_VALID(qwr)) {
        return QUEUE_ERROR;
    }

    if (tout == 0) {
        // Check if queue is full
        if (QUEUE_IS_FULL(qwr)) {
            return QUEUE_FULL;
        }
    }
    else if (tout < 0) {
        // Wait for read queue to have some data
        //ipc_wait_event(QUEUE_IS_FULL(qwr), event_tx);
    }
    else {
        // Wait for read queue to have some data
        //ipc_wait_event_tout(QUEUE_IS_FULL(qwr), tout, event_tx);
        if (tout == 0) {
            return QUEUE_TIMEOUT;
        }
    }

    memcpy(qwr->data + ((qwr->head & (qwr->count - 1)) * qwr->size), data, qwr->size);
    qwr->head++;
    ipc_send_signal();

    return QUEUE_INSERT;
}

// ---------------------------------------------------------------------------
// Read a message from queue with timeout
int IPC_popMsgTout(void *data, int tout)
{
#ifdef EVENT_ON_RX
    int raise_event = QUEUE_IS_FULL(qrd);
#endif

    if (!QUEUE_IS_VALID(qrd)) {
        return QUEUE_ERROR;
    }

    if (tout == 0) {
        // Check if read queue is empty
        if (QUEUE_IS_EMPTY(qrd)) {
            return QUEUE_EMPTY;
        }
    }
    else if (tout < 0) {
        // Wait for read queue to have some data
        //ipc_wait_event(QUEUE_IS_EMPTY(qrd), event_rx);
    }
    else {
        // Wait for event or timeout
        //ipc_wait_event_tout(QUEUE_IS_EMPTY(qrd), tout, event_rx);
        if (tout == 0) {
          return QUEUE_TIMEOUT;
        }
    }

    // Pop the queue Item
    memcpy(data, qrd->data + ((qrd->tail & (qrd->count - 1)) * qrd->size), qrd->size);
    qrd->tail++;

#ifdef EVENT_ON_RX
    if (raise_event) {
        ipc_send_signal();
    }
#endif
    return QUEUE_VALID;
}

// ---------------------------------------------------------------------------
// Get number of pending items in queue
int IPC_msgPending(int queue_write)
{
    struct ipc_queue *q = queue_write ? qwr : qrd;
    if (!QUEUE_IS_VALID(q))
        return QUEUE_ERROR;

    return QUEUE_DATA_COUNT(q);
}

// ---------------------------------------------------------------------------
// Send notificaton interrupt
void IPC_msgNotify(void)
{
    ipc_send_signal();
}

// ---------------------------------------------------------------------------
// Load image on slave core
int IPC_Load(uint32_t image_addr)
{
    // Make sure the alignment is OK
    if (image_addr & 0xFFF) {
        return -1;
    }

#if 0
    // Check the validity of images
    if (CheckImages(image_addr, &__M4Signature) != 0) {
        return -1;
    }

    // Make sure the M0 core is being held in reset via the RGU
    Chip_RGU_TriggerReset(RGU_M0APP_RST);

    Chip_Clock_Enable(CLK_M4_M0APP);

    // Keep in mind the M0 image must be aligned on a 4K boundary
    Chip_CREG_SetM0AppMemMap(image_addr);

    Chip_RGU_ClearReset(RGU_M0APP_RST);
#endif
    return 0;
}
