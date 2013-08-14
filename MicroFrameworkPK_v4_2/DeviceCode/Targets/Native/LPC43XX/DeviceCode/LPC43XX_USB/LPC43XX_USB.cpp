////////////////////////////////////////////////////////////////////////////////
// LPC43XX_USB.cpp - USB ROM functions for NXP LPC43XX/LPC18XX
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
// Copyright (c) NXP Semiconductors, 2012
// Ported to NetMF by Micromint USA <support@micromint.com>
////////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h>
#include <pal\com\usb\USB.h>
#include "LPC43XX.h"

// Uses the USB stack in ROM on the LPC4350/30/20 and LPC185x/3x/2x.
// Logic may be portable to other USB stacks (LPCUSBLib/LUFA, RL-USB, etc)
//
// For USB ROM API details, see the corresponding MCU user's manual
// LPC43XX - http://www.nxp.com/documents/user_manual/UM10503.pdf
// LPC18XX - http://www.nxp.com/documents/user_manual/UM10430.pdf
//
#define USE_USB_ROM_STACK  1

#include "LPC43XX_USB.h"

// USB interrupt handlers
void USB0_IRQHandler(void* param);
void USB1_IRQHandler(void* param);

static LPC_USBHS_T * const USB_REG_BASE[] = {LPC_USB0, LPC_USB1};
static UINT32 const USB_IRQ[] = {USB0_IRQn, USB1_IRQn};
static HAL_CALLBACK_FPN const USB_ISR[] = {USB0_IRQHandler, USB1_IRQHandler};
//static UINT32 const USB_CLK[] = {CLK_MX_USB0, CLK_MX_USB1};
static UINT32 const USB_CLK[] = {CLK_BASE_USB0, CLK_BASE_USB1};

typedef enum {
    REQ_NONE,
    REQ_READ,
    REQ_WRITE,
    REQ_NO_DATA,
} REQ_STAT_T;

typedef struct
{
  USBD_HANDLE_T hCore;
  BOOL enabled;
  UINT8 flags;
  USB_CONTROLLER_STATE *pState;
  UINT8 prevState;
  REQ_STAT_T reqState;
} USB_CORE_T;

static USB_CORE_T usb_core[MAX_USB_CORE];
static USB_CONTROLLER_STATE usb_ctrl_state[MAX_USB_CORE];

#define USB_REG(core)      (USB_REG_BASE[core])
#define USB_FLAGS(core)    (usb_core[core].flags)
#define USB_STATE(core)    (usb_core[core].pState)
#define USB_HANDLE(core)   (usb_core[core].hCore)
#define USB_PREVSTAT(core) (usb_core[core].prevState)
#define USB_REQSTAT(core)  (usb_core[core].reqState)
#define USB_CORE(handle)   (handle == USB_HANDLE(0) ? 0 : 1)

Hal_Queue_KnownSize<USB_PACKET64,USB_QUEUE_PACKET_COUNT> QueueBuffers[USB_MAX_EP_NUM];

#define MAX_EP0_SIZE    64
#define MAX_EP_SIZE     512  // Bulk transfers
#define EP_BUFF_SIZE    MAX_EP_SIZE
//#define EP_BUFF_NUM     2    // For dual buffers

// Initial configuration for USB ROM
// ToDo: Get this from USB_FindRecord()
#include "usbrom_config.cpp"

// ---------------------------------------------------------------------------
// USB buffers and descriptors need to be in AHBSRAM for USB bus master access
#pragma arm section zidata = "SectionForDMA"

#define USBRAM_BUFFSIZE (4 * 1024)

ALIGNED(2048) UINT8 usb_ram[MAX_USB_CORE][USBRAM_BUFFSIZE];

// Endpoint buffers
typedef enum
{
    BUFFER_EMPTY,
    BUFFER_EMPTYING,
    BUFFER_FILLING,
    BUFFER_FILLED,
} BUFFER_STATUS;

typedef struct
{
	BUFFER_STATUS status;
	UINT32 size;
	UINT32 copied;
	UINT8 *data;
} EP_BUFFER;

ALIGNED(4) UINT8 ep_buff_data[MAX_USB_CORE][EP_BUFF_SIZE];
EP_BUFFER ep_buff[MAX_USB_CORE];

// Copy of descriptors in AHBSRAM
ALIGNED(4) UINT8 USBROM_DeviceDescriptor[sizeof(USB_DeviceDescriptor)];
ALIGNED(4) UINT8 USBROM_ConfigDescriptor[sizeof(USB_ConfigDescriptor)];
ALIGNED(4) UINT8 USBROM_FSConfigDescriptor[sizeof(USB_FSConfigDescriptor)];
ALIGNED(4) UINT8 USBROM_StringDescriptor[sizeof(USB_StringDescriptor)];
ALIGNED(4) UINT8 USBROM_DeviceQualifier[sizeof(USB_DeviceQualifier)];

#pragma arm section zidata, rodata, rwdata
// ---------------------------------------------------------------------------
// USB device event callbacks
ErrorCode_t USB_Reset_Event_Handler(USBD_HANDLE_T hUsb);
ErrorCode_t USB_Suspend_Event_Handler(USBD_HANDLE_T hUsb);
ErrorCode_t USB_Resume_Event_Handler(USBD_HANDLE_T hUsb);
// USB endpoint event callbacks
ErrorCode_t USB_EP0_Handler(USBD_HANDLE_T hUsb, void* data, UINT32 event);
ErrorCode_t USB_EP_In_Handler(USBD_HANDLE_T hUsb, void* data, UINT32 event);
ErrorCode_t USB_EP_Out_Handler(USBD_HANDLE_T hUsb, void* data, UINT32 event);
// Local functions
static ErrorCode_t USB_EP0_Setup(USBD_HANDLE_T hUsb);
static void USB_EP0_Write(USBD_HANDLE_T hUsb, UINT8* pData, UINT16 cnt);
static void USB_InitBuffers(int core);

#if 1
//#define dbg(format, arg...) debug_printf("%s: " format, __FILE__ , ## arg)
#define dbg(format, arg...) debug_printf(format, ## arg)
#else
#define dbg(format, arg...) do {} while (0)
#endif

// ---------------------------------------------------------------------------
ErrorCode_t USB_Suspend_Event_Handler(USBD_HANDLE_T hUsb)
{
    int core = USB_CORE(hUsb);
    USB_CONTROLLER_STATE *State;

    dbg("Suspend_Event\r\n");

    State = USB_STATE(core);
    USB_PREVSTAT(core) = State->DeviceState;
    State->DeviceState = USB_DEVICE_STATE_SUSPENDED;
    USB_StateCallback(State);
    return RET_OK;
}

// ---------------------------------------------------------------------------
ErrorCode_t USB_Resume_Event_Handler (USBD_HANDLE_T hUsb)
{
    int core = USB_CORE(hUsb);
    USB_CONTROLLER_STATE *State;

    dbg("Resume_Event\r\n");

    State = USB_STATE(core);
    State->DeviceState = USB_PREVSTAT(core);
    USB_PREVSTAT(core) = 0;
    USB_StateCallback(State);
    return RET_OK;
}

// ---------------------------------------------------------------------------
ErrorCode_t USB_Reset_Event_Handler(USBD_HANDLE_T hUsb)
{
    int core = USB_CORE(hUsb);
    USB_CONTROLLER_STATE *State = USB_STATE(core);

    dbg("Reset_Event\r\n");

    // Clear all flags
    USB_ClearEvent(0, USB_EVENT_ALL);

    State->FirstGetDescriptor = TRUE;
    State->DeviceState = USB_DEVICE_STATE_DEFAULT;
    State->Address = 0;
    USB_StateCallback(State);
    USB_FLAGS(core) = 0;
    USB_REQSTAT(core) = REQ_NONE;
    USB_InitBuffers(core);

    USBD_ClrStallEP(hUsb, USB_ENDPOINT_OUT(2)); // Temp fix?
    USBD_ClrStallEP(hUsb, USB_ENDPOINT_OUT(0));

    return RET_OK;
}

// ---------------------------------------------------------------------------
// Process requests for control endpoint (ep0)
ErrorCode_t USB_EP0_Handler(USBD_HANDLE_T hUsb, void* data, UINT32 event)
{
    USB_CORE_CTRL_T* pCtrl = (USB_CORE_CTRL_T*)hUsb;
    int core = USB_CORE(hUsb);

    dbg("ep0:    evt 0x%02x addr 0x%02x bRequest 0x%02x Type 0x%02x wValue 0x%04x wLength 0x%04x\r\n",
                        event,
                        pCtrl->device_addr,
                        pCtrl->SetupPacket.bRequest,
                        pCtrl->SetupPacket.bmRequestType.B,
                        pCtrl->SetupPacket.wValue,
                        pCtrl->SetupPacket.wLength);

    switch(event)
    {
        case USB_EVT_SETUP:
            // Handle SETUP events
            return USB_EP0_Setup(hUsb);

        case USB_EVT_OUT:
        case USB_EVT_OUT_NAK:
            // Handle OUT events (Rx <-- Host OUT)
            if (USB_REQSTAT(core) == REQ_WRITE) {
                if (pCtrl->EP0Data.Count) {  // Data to receive?
                    USBD_DataOutStage(hUsb);  // Receive data
                }
                if (pCtrl->EP0Data.Count == 0) { // Data complete?
                    // Process data on buffer
                    USBD_StatusInStage(hUsb);
                    USB_REQSTAT(core) = REQ_NONE;
                }
                return RET_OK;
            }
            break;

        case USB_EVT_IN:
            // Handle IN events (Tx -> Host IN)
            if (USB_REQSTAT(core) == REQ_READ) {
                if (pCtrl->EP0Data.Count) {
                    USBD_DataInStage(hUsb);
                } else {
                    USBD_StatusOutStage(hUsb);
                    USB_REQSTAT(core) = REQ_NONE;
                }
                return RET_OK;
            }
            break;

        default:
            break;
    }

    USB_REQSTAT(core) = REQ_NONE;
    return RET_UNHANDLED;
}

// ---------------------------------------------------------------------------
// Process setup requests for control endpoint (ep0)
static ErrorCode_t USB_EP0_Setup(USBD_HANDLE_T hUsb)
{
    int core = USB_CORE(hUsb);
    USB_CONTROLLER_STATE *State;
    USB_CORE_CTRL_T* pCtrl = (USB_CORE_CTRL_T*)hUsb;
    USB_SETUP_PACKET* Setup;
    BOOL shortDescriptor = FALSE;

    // Setup packet received
    State = USB_STATE(core);
    Setup = (USB_SETUP_PACKET*)&pCtrl->SetupPacket;

    // Special handling for the very first SETUP command
    // Getdescriptor[DeviceType], the host looks for 8 bytes data only
    if ((Setup->bRequest == USB_GET_DESCRIPTOR)
                && (((Setup->wValue & 0xFF00) >> 8) == USB_DEVICE_DESCRIPTOR_TYPE)
                && (Setup->wLength != 0x12))
            shortDescriptor = TRUE;

    // Send packet to upper layer
    State->Data = (UINT8*)Setup;
    State->DataSize = sizeof(USB_SETUP_PACKET);
    UINT8 result = USB_ControlCallback(State);
    dbg("ep0:         Setup result 0x%04x\r\n",
                        result);

    switch(result)
    {
        case USB_STATE_DATA:
            // Setup packet was handled and the upper layer has data to send
            break;

        case USB_STATE_ADDRESS:
            // Upper layer needs us to change the address 
            //USBD_SetAddress(hUsb, State->Address);
            return RET_UNHANDLED; // Let USB stack do it

        case USB_STATE_DONE:
            State->DataCallback = NULL;
            break;

        case USB_STATE_STALL:
            //
            // setup packet failed to process successfully
            // set stall condition on the default control endpoint
            //
            // Handled by USB stack
            //USBD_SetStallEP(hUsb, 0);
            return RET_UNHANDLED; // Let USB stack do it

        case USB_STATE_STATUS:
            // Handled by USB stack
            break;

        case USB_STATE_CONFIGURATION:
            break;

        case USB_STATE_REMOTE_WAKEUP:
            // It is not using currently as the device side won't go into SUSPEND mode unless
            // the PC is purposely to select it to SUSPEND, as there is always SOF in the bus
            // to keeping the device from SUSPEND.
            break;

        default:
            ASSERT(0);
            break;
            // Status change is only handled by USB stack
    }
            
    if (result != USB_STATE_STALL)
    {
        if (State->DataCallback)
        {
            /* this call can't fail */
            State->DataCallback(State);

            if (State->DataSize > 0)
            {
                UINT32 cnt = (shortDescriptor ? 8 : State->DataSize);
                pCtrl->EP0Data.Count = Setup->wLength;
                //USBD_WriteEP(hUsb, USB_ENDPOINT_IN(0), State->Data, cnt);
                USB_EP0_Write(hUsb, (UINT8*)State->Data, cnt);
    dbg("ep0:         WriteEP size 0x%04x 0x%02x\r\n",
                        cnt, State->Data[0]);
                // Special handling the USB driver set address test, cannot use 
                // first descriptor as ADDRESS state if handle is in hardware
                if (shortDescriptor) State->DataCallback = NULL;
            }
        }
    }

    // If port is now configured, output any queued data
    if (result == USB_STATE_CONFIGURATION)
    {
        for (int epNum = 1; epNum < State->EndpointCount; epNum++ )
        {
            if (State->Queues[epNum] && State->IsTxQueue[epNum])
                CPU_USB_StartOutput(State, epNum);
        }
    }
    return RET_OK;
}

// ---------------------------------------------------------------------------
static void USB_EP0_Write(USBD_HANDLE_T hUsb, UINT8* pData, UINT16 cnt)
{
    USB_CORE_CTRL_T* pCtrl = (USB_CORE_CTRL_T*)hUsb;
    int core = USB_CORE(hUsb);

    pCtrl->EP0Data.pData = pData;
    if (pCtrl->EP0Data.Count > cnt) {
        pCtrl->EP0Data.Count = cnt;
    }
    USBD_DataInStage(hUsb);
    USB_REQSTAT(core) = REQ_READ;
}

// ---------------------------------------------------------------------------
// Receive buffer when handling data endpoint OUT event (Rx <-- Host OUT)
ErrorCode_t USB_EP_Out_Handler(USBD_HANDLE_T hUsb, void* data, UINT32 event)
{
    int epNum = (int)data;
    int core = USB_CORE(hUsb);
    USB_PACKET64* Packet64;
    USB_CONTROLLER_STATE *State = USB_STATE(core);
    BOOL full;
    UINT32 len;

    dbg("ep_out:  event 0x%02x endpoint 0x%02x\r\n",
                        event, epNum);

    // Handle OUT events

    if (State->Queues[epNum] == NULL || State->IsTxQueue[epNum])
        return RET_UNHANDLED;

    switch(event)
    {
        case USB_EVT_OUT_NAK:
            if (ep_buff[core].status == BUFFER_EMPTY)
            {
                // Check if we are ready to receive data. If yes give the buffer pointer stack
                ep_buff[core].status = BUFFER_FILLING;
                USBD_ReadReqEP(hUsb, USB_ENDPOINT_OUT(epNum), ep_buff[core].data, EP_BUFF_SIZE);
            }
            break;

        case USB_EVT_OUT:
            // Read data received via DMA
            len = USBD_ReadEP(hUsb, USB_ENDPOINT_OUT(epNum), ep_buff[core].data) & 0xFFFF;
            if (len > 0) {
                // The DMA transfer is complete. Mark the buffer as full and process
                ep_buff[core].status = BUFFER_FILLED;
                ep_buff[core].size = len;
                Packet64 = USB_RxEnqueue(State, epNum, full);
                Packet64->Size = len;
                memcpy(Packet64->Buffer, &ep_buff[core], Packet64->Size);
            }
            USB_InitBuffers(core);
            break;

        default:
            return RET_UNHANDLED;
    }

    return RET_OK;
}

// ---------------------------------------------------------------------------
// Transmit buffer when handling data endpoint IN event (Tx -> Host IN)
ErrorCode_t USB_EP_In_Handler(USBD_HANDLE_T hUsb, void* data, UINT32 event)
{
    int epNum = (int)data;

    dbg("ep_in:  event 0x%02x endpoint 0x%02x\r\n",
                        event, epNum);

    // Handle IN events
    if (event != USB_EVT_IN && event != USB_EVT_IN_NAK)
        return RET_UNHANDLED;

    int core = USB_CORE(hUsb);
    USB_CONTROLLER_STATE *State = USB_STATE(core);

    if (State->Queues[epNum] == NULL || !State->IsTxQueue[epNum])
        return RET_UNHANDLED;

    USB_PACKET64* Packet64;

    for(;;)
    {
        Packet64 = USB_TxDequeue(State, epNum, TRUE); // Queue Pop
        if (Packet64 == NULL) break;
        USBD_WriteEP(hUsb, USB_ENDPOINT_IN(epNum), Packet64->Buffer, Packet64->Size);
        dbg("ep_tx:      WriteEP size 0x%04x 0x%08x\r\n",
                            Packet64->Size, Packet64->Buffer[0]);
    }

    return RET_OK;
}

// ---------------------------------------------------------------------------
void USB0_IRQHandler(void* param)
{
    USBD_ISR(USB_HANDLE(0));
}

// ---------------------------------------------------------------------------
void USB1_IRQHandler(void* param)
{
    USBD_ISR(USB_HANDLE(1));
}

// ---------------------------------------------------------------------------
static void USB_InitBuffers(int core)
{
    ep_buff[core].status = BUFFER_EMPTY;
    ep_buff[core].size = EP_BUFF_SIZE;
    ep_buff[core].copied = 0;
    ep_buff[core].data = &ep_buff_data[core][0];
}

// ---------------------------------------------------------------------------
HRESULT CPU_USB_Initialize(int core)
{
    if (core >= MAX_USB_CORE || usb_core[core].enabled) return S_FALSE;

    USBD_API_INIT_PARAM_T usb_param;
    USB_CORE_DESCS_T core_desc;
    ErrorCode_t ret;
    const USB_ENDPOINT_DESCRIPTOR  *epDesc = NULL;
    const USB_INTERFACE_DESCRIPTOR *intfDesc = NULL;
    USB_CONTROLLER_STATE *State;
    int epNum;
    UINT32 queueId = 0;

    GLOBAL_LOCK(irq);

    // Initialize physical layer
        if (!usb_core[1 - core].enabled)  // No other USB controllers enabled?
    {
        LPC_CGU->PLL[CGU_USB_PLL].PLL_CTRL &= ~1; // Enable USB PLL
        while (!(LPC_CGU->PLL[CGU_USB_PLL].PLL_STAT & 1)); // Wait for USB PLL to lock
        }
    LPC_CGU->BASE_CLK[USB_CLK[core]] &= ~1;  // Enable USBx base clock
    LPC_CREG->CREG0 &= ~(1 << 5); // Enable USB0 PHY
    if (core == 1) // Special init for USB1
    {
        // Enable USB1_DP and USB1_DN on chip FS phy
        LPC_SCU->SFSUSB = 0x12; // USB device mode (0x16 for host mode)
        LPC_USB1->PORTSC1_D |= (1 << 24);
    }

    // Initilize USB parameter structure
    memset((void *) &usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
    // Configure USB memory
    usb_param.usb_reg_base = (UINT32) USB_REG(core);
    usb_param.max_num_ep = 6;
    usb_param.mem_base = (UINT32)&usb_ram[core];
    usb_param.mem_size = USBRAM_BUFFSIZE;
    // Configure USB callbacks
    usb_param.USB_Suspend_Event = USB_Suspend_Event_Handler;
    usb_param.USB_Resume_Event = USB_Resume_Event_Handler;
    usb_param.USB_Reset_Event = USB_Reset_Event_Handler;

    // Set USB state
    State = CPU_USB_GetState(core);
    USB_STATE(core) = State;
    USB_FLAGS(core) = 0;
    USB_REQSTAT(core) = REQ_NONE;
    USB_InitBuffers(core);

    // Initialize USB stack with first device, configuration and string descriptors
    // ep0 class handler will manage USB dynamic configuration afterwards

    //USB_DEVICE_DESCRIPTOR* dev_desc = (USB_DEVICE_DESCRIPTOR*)USB_FindRecord(State, USB_DEVICE_DESCRIPTOR_MARKER, 0);
    //core_desc.device_desc = (uint8_t *)&(dev_desc->bLength);
    //USB_CONFIGURATION_DESCRIPTOR* hs_desc = (USB_CONFIGURATION_DESCRIPTOR*)USB_FindRecord(State, USB_CONFIGURATION_DESCRIPTOR_MARKER, 0);
    //core_desc.high_speed_desc = (uint8_t *)&(hs_desc->bLength);
    //core_desc.full_speed_desc = core_desc.high_speed_desc;
    //core_desc.string_desc = (uint8_t *)USB_FindRecord(State, USB_STRING_DESCRIPTOR_MARKER, 0);

#if 1
    memcpy(USBROM_DeviceDescriptor, USB_DeviceDescriptor, sizeof(USB_DeviceDescriptor));
    memcpy(USBROM_ConfigDescriptor, USB_ConfigDescriptor, sizeof(USB_ConfigDescriptor));
    memcpy(USBROM_FSConfigDescriptor, USB_FSConfigDescriptor, sizeof(USB_FSConfigDescriptor));
    memcpy(USBROM_StringDescriptor, USB_StringDescriptor, sizeof(USB_StringDescriptor));
    memcpy(USBROM_DeviceQualifier, USB_DeviceQualifier, sizeof(USB_DeviceQualifier));

    core_desc.device_desc = (UINT8*)USBROM_DeviceDescriptor;
    core_desc.high_speed_desc = (UINT8*)USBROM_ConfigDescriptor;
    core_desc.full_speed_desc = (UINT8*)USBROM_FSConfigDescriptor;
    core_desc.string_desc = (UINT8*)USBROM_StringDescriptor;
    core_desc.device_qualifier = (UINT8*)USBROM_DeviceQualifier;
#else
    core_desc.device_desc = (UINT8*)USB_DeviceDescriptor;
    core_desc.high_speed_desc = (UINT8*)USB_ConfigDescriptor;
    core_desc.full_speed_desc = (UINT8*)USB_FSConfigDescriptor;
    core_desc.string_desc = (UINT8*)USB_StringDescriptor;
    core_desc.device_qualifier = (UINT8*)USB_DeviceQualifier;
#endif
    State->EndpointCount = USB_MAX_EP_NUM;
    State->PacketSize = MAX_EP0_SIZE;

    // Initialize USB stack
    USBD_HANDLE_T *phUsb = &(USB_HANDLE(core));
    ret = USBD_Init(phUsb, &core_desc, &usb_param);
    dbg("USBD_Init 0x%02x\r\n", ret);
    if (ret != RET_OK) return S_FALSE; // Exit if not succesful
    // Register endpoint class handler
    USBD_RegisterClassHandler(*phUsb, USB_EP0_Handler, NULL);

    // Set defaults for unused endpoints
    for (epNum = 1; epNum < State->EndpointCount; epNum++)
    {
        State->IsTxQueue[epNum] = FALSE;
        State->MaxPacketSize[epNum] = USB_MAX_DATA_PACKET_SIZE;
    }

    // Get endpoint configuration
    while (USB_NextEndpoint(State, epDesc, intfDesc))
    {
        // Figure out which endpoint we are initializing
        epNum = epDesc->bEndpointAddress & 0x7F;

        // Check interface and endpoint numbers against hardware capability
        if (epNum >= State->EndpointCount || intfDesc->bInterfaceNumber > 3)
            return S_FALSE;

        if (epDesc->bEndpointAddress & 0x80) State->IsTxQueue[epNum] = TRUE;

        // Set the maximum size of the endpoint hardware FIFO
        int endpointSize = epDesc->wMaxPacketSize;
        // If the endpoint maximum size in the configuration list is bogus
        if (endpointSize != 8 && endpointSize != 16 && endpointSize != 32 && endpointSize != 64
                && endpointSize != 128 && endpointSize != 256 && endpointSize != 512)
            return S_FALSE;
        State->MaxPacketSize[epNum] = endpointSize;

        // Assign queues
        QueueBuffers[queueId].Initialize();           // Clear queue before use
        State->Queues[epNum] = &QueueBuffers[queueId];  // Attach queue to endpoint
        queueId++;

        // Isochronous endpoints are currently not supported
        if ((epDesc->bmAttributes & 3) == USB_ENDPOINT_ATTRIBUTE_ISOCHRONOUS) return FALSE;

        // Configure endpoint and register handler
        if ((epDesc->bEndpointAddress & 0x80) == USB_ENDPOINT_DIRECTION_OUT) {
            USBD_RegisterEpHandler(*phUsb,
                                   (2 * epNum ), USB_EP_Out_Handler, (void*)epNum);
        }
        else {
            USBD_RegisterEpHandler(*phUsb,
                                   (2 * epNum + 1), USB_EP_In_Handler, (void*)epNum);
        }
    }

    // Enable interrupts and make soft connect
    CPU_INTC_ActivateInterrupt(USB_IRQ[core], USB_ISR[core], 0);
    USBD_Connect(*phUsb, 1); // Connect
    usb_core[core].enabled = TRUE;

    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT CPU_USB_Uninitialize(int core)
{
    if (core >= MAX_USB_CORE || !usb_core[core].enabled) return S_FALSE;

    // Make soft disconnect and disable interrupts
    USBD_Connect(USB_HANDLE(core), 0); // Disconnect
    CPU_INTC_DeactivateInterrupt(USB_IRQ[core]);

    // Uninitialize physical layer
    LPC_CGU->BASE_CLK[USB_CLK[core]] |= 1;  // Disable USBx base clock
        if (usb_core[1 - core].enabled)  // No other USB controllers enabled?
    {
      LPC_CREG->CREG0 |= (1 << 5);; // Disable USB0 PHY
      LPC_CGU->PLL[CGU_USB_PLL].PLL_CTRL |= 1; // Disable USB PLL
        }
    USB_STATE(core) = NULL;
    usb_core[core].enabled = FALSE;

    return S_OK;
}

// ---------------------------------------------------------------------------
USB_CONTROLLER_STATE *CPU_USB_GetState(int core)
{
    if (core >= MAX_USB_CORE) return NULL;
    return &usb_ctrl_state[core];
}

// ---------------------------------------------------------------------------
BOOL CPU_USB_StartOutput(USB_CONTROLLER_STATE* State, int epNum)
{
    USBD_HANDLE_T hUsb = (State == USB_STATE(0)) ? USB_HANDLE(0) : USB_HANDLE(1);

    if (State == NULL || epNum >= State->EndpointCount) return FALSE;

    GLOBAL_LOCK(irq);

    dbg("USBD_StartOutput 0x%02x\r\n", epNum);
    // If endpoint is not an output
    if (State->Queues[epNum] == NULL || !State->IsTxQueue[epNum])
        return FALSE;

    // If the halt feature for this endpoint is set, then just clear all the characters
    if (State->EndpointStatus[epNum] & USB_STATUS_ENDPOINT_HALT)
    {
        if (State->Queues[epNum] != NULL) State->Queues[epNum]->Initialize();
        return TRUE;
    }

    USB_EP_In_Handler(hUsb, (void*) epNum, USB_EVT_IN); // ?
    dbg("StartOutput 0x%02x\r\n", epNum);

    return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CPU_USB_RxEnable(USB_CONTROLLER_STATE* State, int epNum)
{
    // If this is not a legal Rx queue
    if (State == NULL || State->Queues[epNum] == NULL || State->IsTxQueue[epNum])
        return FALSE;

    GLOBAL_LOCK(irq);

    // Enable endpoint
    USBD_EnableEP(USB_HANDLE(State->ControllerNum), USB_ENDPOINT_OUT(epNum));
    dbg("RxEnable 0x%02x\r\n", epNum);

    return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CPU_USB_GetInterruptState()
{
    if (CPU_INTC_InterruptState(USB_IRQ[0])
        || CPU_INTC_InterruptState(USB_IRQ[1])) return TRUE;

    return FALSE;
}

// ---------------------------------------------------------------------------
BOOL CPU_USB_ProtectPins(int core, BOOL On)
{
    USB_CONTROLLER_STATE *State;

    GLOBAL_LOCK(irq);

    if (core >= MAX_USB_CORE) return FALSE;
    State = USB_STATE(core);

    if (On)
    {
        // Clear queues
        for (int epNum = 1; epNum < State->EndpointCount; epNum++)
        {
            if (State->Queues[epNum] != NULL && State->IsTxQueue[epNum])
                State->Queues[epNum]->Initialize();                  
        }
    }

    return TRUE;
}
