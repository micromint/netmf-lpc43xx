/*
 * LPC43XX IPC header
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
 * Simplified version of NXP IPC headers
 * 05/15/13  Micromint USA <support@micromint.com>
 */

#ifndef __LPC43XX_IPC_H
#define __LPC43XX_IPC_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__IAR_SYSTEMS_ICC__)
  #define LOCATE_ATX(x)      _Pragma(#x)
  #define LOCATE_ATXX(x)     LOCATE_ATX(location=x)
  #define LOCATE_AT(x)       LOCATE_ATXX(x)
#elif defined(__ARMCC_VERSION)
  #define LOCATE_AT(x)     __attribute__ ((at(x)))
#elif (defined(__CODE_RED))
  #define LOCATE_ATX(x)     __attribute__((section(".shmem_"#x ",\"aw\",%nobits@")))
  #define LOCATE_AT(x) LOCATE_ATX(x)
#elif (defined(__CODE_RED))
  #define LOCATE_AT(x) LOCATE_ATX(x)
#endif

#define 	IRQ_PRIO_IPC   7

/* Queue status macros */
#define QUEUE_DATA_COUNT(q) ((uint32_t) ((q)->head - (q)->tail))
#define QUEUE_IS_FULL(q)    (QUEUE_DATA_COUNT(q) >= (q)->count)
#define QUEUE_IS_EMPTY(q)   ((q)->head == (q)->tail)
#define QUEUE_IS_VALID(q)   ((q)->valid == QUEUE_MAGIC_VALID)

/* IPC Queue Structure used for sync between M0 and M4 */
struct ipc_queue {
	int32_t size;				/*!< Size of a single item in queue */
	int32_t count;				/*!< Toal number of elements that can be stored in the queue */
	volatile uint32_t head;		/*!< Head index of the queue */
	volatile uint32_t tail;		/*!< Tail index of the queue */
	uint8_t *data;				/*!< Pointer to the data */
	uint32_t valid;             /*!< Queue is valid only if this is #QUEUE_MAGIC_VALID */
	uint32_t reserved[2];		/*!< Reserved entry to keep the structure aligned */
};

/* IPC Function return values */
#define QUEUE_VALID      1
#define QUEUE_INSERT     0
#define QUEUE_FULL      -1
#define QUEUE_EMPTY     -2
#define QUEUE_ERROR     -3
#define QUEUE_TIMEOUT   -4
#define QUEUE_MAGIC_VALID   0xCAB51053

/* IPC functions */
BOOL IPC_Initialize();
BOOL IPC_Uninitialize();
void IPC_initMsgQueue(void *data, int size, int count);
int IPC_pushMsgTout(const void *data, int tout);
int IPC_popMsgTout(void *data, int tout);
int IPC_msgPending(int queue_write);
void IPC_msgNotify(void);
int IPC_load(uint32_t image_addr);

#ifdef __cplusplus
}
#endif

#endif /* __LPC43XX_IPC_H */
