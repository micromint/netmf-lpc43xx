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
#include <WireProtocol.h> // for WP_Packet
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

typedef struct
{
  USBD_HANDLE_T hCore;
  USBD_HANDLE_T hCdc;
  BOOL enabled;
  UINT8 flags;
  USB_CONTROLLER_STATE *pState;
  UINT16 epStatus[USB_MAX_EP_NUM];
  UINT8 prevState;
} USB_CORE_T;

static USB_CORE_T usb_core[MAX_USB_CORE];
static USB_CONTROLLER_STATE usb_ctrl_state[MAX_USB_CORE];

#define USB_REG(core)      (USB_REG_BASE[core])
#define USB_HANDLE(core)   (usb_core[core].hCore)
#define USB_HCDC(core)     (usb_core[core].hCdc)
#define USB_ENABLED(core)  (usb_core[core].enabled)
#define USB_FLAGS(core)    (usb_core[core].flags)
#define USB_STATE(core)    (usb_core[core].pState)
#define USB_EPSTATUS(core) (usb_core[core].epStatus)
#define USB_PREVSTAT(core) (usb_core[core].prevState)
#define USB_CORE(handle)   ((handle == USB_HANDLE(0)) ? 0 : 1)

Hal_Queue_KnownSize<USB_PACKET64,USB_QUEUE_PACKET_COUNT> QueueBuffers[USB_MAX_EP_NUM];

#define MAX_EP0_SIZE    64  // 64 for HS
// Max bulk transfer size should be <= USB_MAX_DATA_PACKET_SIZE
#define MAX_EP_SIZE     USB_MAX_DATA_PACKET_SIZE  // Bulk transfers

// Initial USB config for USB ROM stack
// ToDo: Get descriptors using USB_FindRecord() to avoid duplicate structs
#include "usbrom_config.cpp"

// ---------------------------------------------------------------------------
// USB buffers and descriptors need to be in AHBSRAM for USB bus master access
#pragma arm section zidata = "SectionForDMA"

#define USBROM_MEM_SIZE (4 * 1024)
#define CDC_MEM_SIZE    (2 * 1024)
#define EP_MEM_SIZE     (2 * 1024)

ALIGNED(2048) static UINT8 usbrom_buff[MAX_USB_CORE][USBROM_MEM_SIZE];
ALIGNED(1024) static UINT8 cdc_buff[MAX_USB_CORE][CDC_MEM_SIZE];

// Endpoint buffers
ALIGNED(4) static UINT8 ep_out_data[MAX_USB_CORE][EP_MEM_SIZE];
ALIGNED(4) static UINT8 ep_in_data[MAX_USB_CORE][EP_MEM_SIZE];
ALIGNED(4) static UINT16 ep_out_count[MAX_USB_CORE];
ALIGNED(4) static UINT16 ep_in_count[MAX_USB_CORE];

// Copy of descriptors in AHBSRAM
ALIGNED(4) static UINT8 USBROM_DeviceDescriptor[sizeof(USB_DeviceDescriptor)];
ALIGNED(4) static UINT8 USBROM_ConfigDescriptor[sizeof(USB_ConfigDescriptor)];
ALIGNED(4) static UINT8 USBROM_FSConfigDescriptor[sizeof(USB_FSConfigDescriptor)];
ALIGNED(4) static UINT8 USBROM_StringDescriptor[sizeof(USB_StringDescriptor)];
ALIGNED(4) static UINT8 USBROM_DeviceQualifier[sizeof(USB_DeviceQualifier)];

#pragma arm section zidata

// ---------------------------------------------------------------------------
// Local functions
static ErrorCode_t USB_InitStack(int core);
// USB device event callbacks
static ErrorCode_t USB_Reset_Event_Handler(USBD_HANDLE_T hUsb);
// USB endpoint event callbacks
static ErrorCode_t USB_EP_In_Handler(USBD_HANDLE_T hUsb, void* data, UINT32 event);
static ErrorCode_t USB_EP_Out_Handler(USBD_HANDLE_T hUsb, void* data, UINT32 event);

static USB_EP_HANDLER_T CDC_Default_Handler; // Default CDC EP0 handler
static CDC_LINE_CODING* cdc_coding;

// CDC functions
static ErrorCode_t CDC_Init(int core);
static ErrorCode_t CDC_EP0_Handler(USBD_HANDLE_T hUsb, void* data, UINT32 event);
static ErrorCode_t CDC_SetLineCode(USBD_HANDLE_T hCDC, CDC_LINE_CODING *lineCoding);

// ---------------------------------------------------------------------------
static ErrorCode_t USB_InitStack(int core)
{
    if (USB_HANDLE(core)) return RET_OK; // Only init once

    USBD_API_INIT_PARAM_T usb_param;
    USB_CORE_DESCS_T core_desc;

    // Initialize USB parameter structure
    memset((void *)&usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
    // Configure USB memory
    usb_param.usb_reg_base = (UINT32)USB_REG(core);
    usb_param.max_num_ep = USB_MAX_EP_NUM;
    usb_param.mem_base = (UINT32)&usbrom_buff[core];
    usb_param.mem_size = USBROM_MEM_SIZE;
    // Configure USB callbacks
    usb_param.USB_Reset_Event = USB_Reset_Event_Handler;

    // Configure stack with initial device, configuration and string descriptors
    // ep0 class handler can manage USB dynamic configuration afterwards
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

    return USBD_Init(&(USB_HANDLE(core)), &core_desc, &usb_param);
}

// ---------------------------------------------------------------------------
static ErrorCode_t CDC_Init(int core)
{
    if (USB_HCDC(core)) return RET_OK; // Only init once

    USBD_CDC_INIT_PARAM_T cdc_param;
    USB_CORE_CTRL_T* pCtrl;
    USB_INTERFACE_DESCRIPTOR *pIfDesc;
    USB_INTERFACE_DESCRIPTOR *pDataIfDesc;
    USBD_HANDLE_T hUsb = USB_HANDLE(core);
    ErrorCode_t ret;

    pIfDesc = (USB_INTERFACE_DESCRIPTOR*)((UINT32)USBROM_ConfigDescriptor + USB_CONFIGURATION_DESCRIPTOR_LENGTH);
    pDataIfDesc = (USB_INTERFACE_DESCRIPTOR*)((UINT32)pIfDesc + sizeof(USB_INTERFACE_DESCRIPTOR)
                    + 0x0013 + sizeof(USB_ENDPOINT_DESCRIPTOR));

    // Set CDC parameters
    memset((void*)&cdc_param, 0, sizeof(USBD_CDC_INIT_PARAM_T));
    cdc_param.mem_base = (UINT32)&cdc_buff[core];
    cdc_param.mem_size = CDC_MEM_SIZE;
    cdc_param.cif_intf_desc = (UINT8*)pIfDesc;
    cdc_param.dif_intf_desc = (UINT8*)pDataIfDesc;
    cdc_param.SetLineCode = CDC_SetLineCode;

    USBD_RegisterEpHandler(hUsb, USB_EP_INDEX_IN(BULK_IN_EP), USB_EP_In_Handler, (void*)BULK_IN_EP);
    USBD_RegisterEpHandler(hUsb, USB_EP_INDEX_OUT(BULK_OUT_EP), USB_EP_Out_Handler, (void*)BULK_OUT_EP);
    ret = USBD_API->cdc->init(hUsb, &cdc_param, &USB_HCDC(core));

    // Patch from LPC43XX errata - Save default CDC handler and replace it
    pCtrl = (USB_CORE_CTRL_T*)hUsb;
    if (pCtrl->ep0_hdlr_cb[pCtrl->num_ep0_hdlrs - 1] != CDC_EP0_Handler)
    {
        CDC_Default_Handler = pCtrl->ep0_hdlr_cb[pCtrl->num_ep0_hdlrs - 1];
        pCtrl->ep0_hdlr_cb[pCtrl->num_ep0_hdlrs - 1] = CDC_EP0_Handler;
    }

    cdc_coding = &(((USB_CDC_CTRL_T*)USB_HCDC(core))->line_coding);

    return ret;
}

// ---------------------------------------------------------------------------
static ErrorCode_t CDC_EP0_Handler(USBD_HANDLE_T hUsb, void* data, UINT32 event)
{
    USB_CORE_CTRL_T* pCtrl = (USB_CORE_CTRL_T*)hUsb;
    USB_CDC_CTRL_T* pCdcCtrl = (USB_CDC_CTRL_T*)data;
    ErrorCode_t ret = ERR_USBD_UNHANDLED;

    // Patch from LPC43XX errata - Send acknowledge after SET_LINE_CONFIG
    if((event == USB_EVT_OUT) &&
            (pCtrl->SetupPacket.bmRequestType.BM.Type == REQUEST_CLASS) &&
            (pCtrl->SetupPacket.bmRequestType.BM.Recipient == REQUEST_TO_INTERFACE) &&
            ((pCtrl->SetupPacket.wIndex.WB.L == pCdcCtrl->cif_num) || // IF number correct?
            (pCtrl->SetupPacket.wIndex.WB.L == pCdcCtrl->dif_num)))
    {
        pCtrl->EP0Data.pData -= pCtrl->SetupPacket.wLength;
        ret = pCdcCtrl->CIC_SetRequest(pCdcCtrl, &pCtrl->SetupPacket,
                                        &pCtrl->EP0Data.pData,
                                        pCtrl->SetupPacket.wLength);
        if (ret == LPC_OK)
        {
            USBD_API->core->StatusInStage(pCtrl); // Send Acknowledge
        }
    } else {
        ret = CDC_Default_Handler(hUsb, data, event);
    }

    return ret;
}

// ---------------------------------------------------------------------------
static ErrorCode_t CDC_SetLineCode(USBD_HANDLE_T hCDC, CDC_LINE_CODING *lineCoding)
{
    memcpy(cdc_coding, lineCoding, sizeof(CDC_LINE_CODING));
    return LPC_OK;
}

// ---------------------------------------------------------------------------
static ErrorCode_t USB_Reset_Event_Handler(USBD_HANDLE_T hUsb)
{
    int core = USB_CORE(hUsb);
    USB_CONTROLLER_STATE *State = USB_STATE(core);

    // Clear all flags
    USB_ClearEvent(0, USB_EVENT_ALL);

    State->FirstGetDescriptor = TRUE;
    //State->Address = 0;
    //State->DeviceState = USB_DEVICE_STATE_DEFAULT;
    //USB_StateCallback(State);
    USB_FLAGS(core) = 0;

    // Reset endpoints
    USBD_ResetEP(hUsb, USB_ENDPOINT_IN(INT_IN_EP));
    USBD_ResetEP(hUsb, USB_ENDPOINT_IN(BULK_IN_EP));
    USBD_ResetEP(hUsb, USB_ENDPOINT_OUT(BULK_OUT_EP));
    ep_out_count[core] = 0;
    ep_in_count[core] = 0;

    return RET_OK;
}

// ---------------------------------------------------------------------------
// Receive buffer when handling data endpoint OUT event (Rx <-- Host OUT)
static ErrorCode_t USB_EP_Out_Handler(USBD_HANDLE_T hUsb, void* data, UINT32 event)
{
    int epNum = (int)data;
    int core = USB_CORE(hUsb);
    USB_PACKET64* Packet64;
    USB_CONTROLLER_STATE *State = USB_STATE(core);
    BOOL full;
    UINT32 len;

    // Handle OUT events
    if (State->Queues[epNum] == NULL || State->IsTxQueue[epNum])
        return RET_UNHANDLED;

    switch(event)
    {
        case USB_EVT_OUT_NAK:
            // If we are ready to receive data, send buffer pointer to USB stack
            USBD_ReadReqEP(hUsb, USB_ENDPOINT_OUT(epNum), ep_out_data[core], MAX_EP_SIZE);
            break;

        case USB_EVT_OUT:
            // Read data received via DMA
            len = USBD_ReadEP(hUsb, USB_ENDPOINT_OUT(epNum), ep_out_data[core]) & 0xFFFF;
            if (len > 0)
            {
                // The DMA transfer is complete. Process buffer.
                Packet64 = USB_RxEnqueue(State, epNum, full);
                if (Packet64 != NULL)
                {
                    Packet64->Size = len;
                    memcpy(Packet64->Buffer, ep_out_data[core], Packet64->Size);
                    //Events_Set(SYSTEM_EVENT_FLAG_USB_IN);
                }
                //if (full) USBD_DisableEP(hUsb, USB_ENDPOINT_OUT(epNum));
            }
            break;

        default:
            return RET_UNHANDLED;
    }

    return RET_OK;
}

// ---------------------------------------------------------------------------
// Transmit buffer when handling data endpoint IN event (Tx -> Host IN)
static ErrorCode_t USB_EP_In_Handler(USBD_HANDLE_T hUsb, void* data, UINT32 event)
{
    // Exit if not an IN event
    if (event != USB_EVT_IN) return RET_UNHANDLED;

    int epNum = (int)data;
    int core = USB_CORE(hUsb);

    // Exit if buffer empty
    if (ep_in_count[core] == 0) return RET_UNHANDLED;

    // Send buffer
    USBD_WriteEP(hUsb, USB_ENDPOINT_IN(epNum), ep_in_data[core], ep_in_count[core]);
    ep_in_count[core] = 0;

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
HRESULT CPU_USB_Initialize(int core)
{
    if (core >= MAX_USB_CORE || USB_ENABLED(core)) return S_FALSE;

    USB_CONTROLLER_STATE *State = CPU_USB_GetState(core);
    const USB_ENDPOINT_DESCRIPTOR  *pEpDesc;
    const USB_INTERFACE_DESCRIPTOR *pIfDesc;
    int epNum, ret;
    UINT32 queueId = 0;

    GLOBAL_LOCK(irq);

    // Initialize physical layer
    if (!USB_ENABLED(1 - core))  // No other USB controllers enabled?
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

    // Set USB state
    USB_STATE(core) = State;
    USB_FLAGS(core) = 0;
    ep_out_count[core] = 0;
    ep_in_count[core] = 0;

    State->EndpointStatus = USB_EPSTATUS(core);
    State->EndpointCount = USB_MAX_EP_NUM;
    State->PacketSize = MAX_EP0_SIZE;

    // Initialize USB stack
    ret = USB_InitStack(core);
    if (ret != RET_OK) return S_FALSE; // Exit if not succesful

    // Set defaults for unused endpoints
    for (epNum = 1; epNum < State->EndpointCount; epNum++)
    {
        State->IsTxQueue[epNum] = FALSE;
        State->MaxPacketSize[epNum] = MAX_EP_SIZE;
    }

    // Get endpoint configuration
    while (USB_NextEndpoint(State, pEpDesc, pIfDesc))
    {
        // Figure out which endpoint we are initializing
        epNum = pEpDesc->bEndpointAddress & 0x7F;

        // Check interface and endpoint numbers against hardware capability
        if (epNum >= State->EndpointCount || pIfDesc->bInterfaceNumber > 3)
            return S_FALSE;

        if (pEpDesc->bEndpointAddress & 0x80) State->IsTxQueue[epNum] = TRUE;

        // Set the maximum size of the endpoint hardware FIFO
        int endpointSize = pEpDesc->wMaxPacketSize;
        // Exit if the endpoint maximum size in the configuration list is bogus
        // or greater than USB_MAX_DATA_PACKET_SIZE (default=64)
        if ((endpointSize != 8 && endpointSize != 16 && endpointSize != 32 && endpointSize != 64
                && endpointSize != 128 && endpointSize != 256 && endpointSize != 512)
				|| endpointSize > USB_MAX_DATA_PACKET_SIZE)
            return S_FALSE;
        State->MaxPacketSize[epNum] = endpointSize;

        // Assign queues
        QueueBuffers[queueId].Initialize();           // Clear queue before use
        State->Queues[epNum] = &QueueBuffers[queueId];  // Attach queue to endpoint
        queueId++;

        // Isochronous endpoints are currently not supported
        if ((pEpDesc->bmAttributes & 3) == USB_ENDPOINT_ATTRIBUTE_ISOCHRONOUS) return FALSE;
    }

    // Configure CDC driver
    ret = CDC_Init(core);
    if (ret != RET_OK) return S_FALSE; // Exit if not succesful

    // Connect and enable interrupts
    CPU_USB_ProtectPins(core, FALSE);
    CPU_INTC_ActivateInterrupt(USB_IRQ[core], USB_ISR[core], 0);
    USB_ENABLED(core) = TRUE;
    State->DeviceState = USB_DEVICE_STATE_CONFIGURED; // Config done by ROM stack
    USB_StateCallback(State);

    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT CPU_USB_Uninitialize(int core)
{
    if (core >= MAX_USB_CORE || !USB_ENABLED(core)) return S_FALSE;

    // With CDC driver should keep COM port open, don't unitialize

    USB_ENABLED(core) = FALSE;

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
    if (State == NULL || epNum >= State->EndpointCount)
        return FALSE;

    GLOBAL_LOCK(irq);

    // Exit if endpoint has no output queue
    if (State->Queues[epNum] == NULL || !State->IsTxQueue[epNum])
        return FALSE;

    // If endpoint status is halt, clear all queues
    if (State->EndpointStatus[epNum] & USB_STATUS_ENDPOINT_HALT)
    {
        while (USB_TxDequeue(State, epNum, TRUE) != NULL); // Clear TX queue
        return TRUE;
    }

    USBD_HANDLE_T hUsb = (State == USB_STATE(0)) ? USB_HANDLE(0) : USB_HANDLE(1);
    int core = USB_CORE(hUsb);
    USB_PACKET64* Packet64;
    UINT8* dst = ep_in_data[core];
    UINT32 cnt = 0;

    // Hold if buffer pending
    if (ep_in_count[core] != 0) return FALSE;

    // If payload missing, hold until flush to avoid host timeout at HS
    Packet64 = USB_TxDequeue(State, epNum, FALSE);
    if (Packet64->Size == sizeof(WP_Packet)
            && ((memcmp(Packet64->Buffer, MARKER_DEBUGGER_V1, 7) == 0)
                    || (memcmp(Packet64->Buffer, MARKER_PACKET_V1, 7) == 0)))
    {
        memcpy(dst, Packet64->Buffer, Packet64->Size);
        int missing = ((WP_Packet*)dst)->m_size + sizeof(WP_Packet);
        for (int i = 0; i < State->Queues[epNum]->NumberOfElements(); i++) {
            missing -= (*State->Queues[epNum])[i]->Size;
        }
        if (missing != 0) return FALSE;
    }

    // Copy packets to endpoint buffer
    cnt = 0;
    while ((Packet64 = USB_TxDequeue(State, epNum, FALSE)) != NULL) // Peek
    {
        if ((cnt + Packet64->Size) > EP_MEM_SIZE) break;
        Packet64 = USB_TxDequeue(State, epNum, TRUE); // Pop
        memcpy(dst, Packet64->Buffer, Packet64->Size);
        dst += Packet64->Size;
        cnt += Packet64->Size;
    }
    ep_in_count[core] = cnt;

    // Transmit buffer
    USB_EP_In_Handler(hUsb, (void*) epNum, USB_EVT_IN);

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
    USBD_HANDLE_T hUsb = USB_HANDLE(core);

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
#if 0 // Don't disconnect on CDC
        // Make soft disconnect and set detached state
        USBD_Connect(hUsb, 0); // Disconnect
        State->DeviceState = USB_DEVICE_STATE_DETACHED;
        USB_StateCallback(State);
#endif
    } else {
        // Make soft connect and set attached state
        USBD_Connect(hUsb, 1); // Connect
        State->DeviceState = USB_DEVICE_STATE_ATTACHED;
        USB_StateCallback(State);
    }

    return TRUE;
}
