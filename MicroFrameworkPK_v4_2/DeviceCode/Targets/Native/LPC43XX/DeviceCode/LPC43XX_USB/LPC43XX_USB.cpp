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
// For USB ROM API details, see the corresponding user's manual
// LPC43XX - http://www.nxp.com/documents/user_manual/UM10503.pdf
// LPC18XX - http://www.nxp.com/documents/user_manual/UM10430.pdf
//
#define USE_USB_ROM_STACK  1

#include "LPC43XX_USB.h"

#define USBD_Init                  USBD_API->hw->Init
#define USBD_Connect               USBD_API->hw->Connect
#define USBD_ISR                   USBD_API->hw->ISR
#define USBD_RegisterClassHandler  USBD_API->core->RegisterClassHandler
#define USBD_RegisterEpHandler     USBD_API->core->RegisterEpHandler
#define USBD_ReadEP                USBD_API->hw->ReadEP
#define USBD_WriteEP               USBD_API->hw->WriteEP
#define USBD_EnableEP              USBD_API->hw->EnableEP
#define USBD_ResetEP               USBD_API->hw->ResetEP
//#define USBD_SetAddress            USBD_API->hw->SetAddress
//#define USBD_SetStallEP            USBD_API->hw->SetStallEP

#define USBROM_BASEADDR 0x20008000
#define USBROM_BUFFSIZE 0x2000
#define USBROM_BUFFADDR(core)  (USBROM_BASEADDR + core * USBROM_BUFFSIZE)
#define USB_MAX_PACKET0 64

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
  BOOL enabled;
  USB_CONTROLLER_STATE *pState;
  UINT8 previousState;
} USB_CORE_T;

static USB_CORE_T usb_core[MAX_USB_CORE];
static USB_CONTROLLER_STATE usb_ctrl_state[MAX_USB_CORE];

#define USB_REG(core)    (USB_REG_BASE[core])
#define USB_STATE(core)  (usb_core[core].pState)
#define USB_HANDLE(core) (usb_core[core].hCore)

Hal_Queue_KnownSize<USB_PACKET64,USB_QUEUE_PACKET_COUNT> QueueBuffers[USB_MAX_EP_NUM];

// USB device event callbacks
ErrorCode_t USB_Reset_Event_Handler(USBD_HANDLE_T hUsb);
ErrorCode_t USB_Suspend_Event_Handler(USBD_HANDLE_T hUsb);
ErrorCode_t USB_Resume_Event_Handler(USBD_HANDLE_T hUsb);
// USB endpoint event callbacks
ErrorCode_t USB_EP_Class_Handler(USBD_HANDLE_T hUsb, void* data, uint32_t event);
ErrorCode_t USB_EP_In_Handler(USBD_HANDLE_T hUsb, void* data, uint32_t event);
ErrorCode_t USB_EP_Out_Handler(USBD_HANDLE_T hUsb, void* data, uint32_t event);
// Local functions
static ErrorCode_t USB_EP_Receive(USBD_HANDLE_T hUsb, int epNum);
static ErrorCode_t USB_EP_Transmit(USBD_HANDLE_T hUsb, int epNum);

// ---------------------------------------------------------------------------
ErrorCode_t USB_Suspend_Event_Handler(USBD_HANDLE_T hUsb)
{
    int core = (hUsb == USB_HANDLE(0)) ? 0 : 1;
    USB_CONTROLLER_STATE *State;

    State = USB_STATE(core);
    usb_core[core].previousState = State->DeviceState;
    State->DeviceState = USB_DEVICE_STATE_SUSPENDED;
    USB_StateCallback(State);
    return LPC_OK;
}

// ---------------------------------------------------------------------------
ErrorCode_t USB_Resume_Event_Handler (USBD_HANDLE_T hUsb)
{
    int core = (hUsb == USB_HANDLE(0) ? 0 : 1);
    USB_CONTROLLER_STATE *State;

    State = USB_STATE(core);
    State->DeviceState = usb_core[core].previousState;
    usb_core[core].previousState = 0;
    USB_StateCallback(State);
    return LPC_OK;
}

// ---------------------------------------------------------------------------
ErrorCode_t USB_Reset_Event_Handler(USBD_HANDLE_T hUsb)
{
    int core = (hUsb == USB_HANDLE(0) ? 0 : 1);
    USB_CONTROLLER_STATE *State;

    State = USB_STATE(core);
#if 0
    // Reset endpoints
    for (int i = 0; i < State->EndpointCount; i++)
    {
        USBD_ResetEP(hUsb, i);
        //USBD_EnableEP(hUsb, i);  // Re-enable endpoint
    }

    // Enable interrupts
    CPU_INTC_ActivateInterrupt(USB_IRQ[core], USB_ISR[core], 0);
#endif
    // Clear all flags
    USB_ClearEvent(0, USB_EVENT_ALL);

    State->FirstGetDescriptor = TRUE;
    State->DeviceState = USB_DEVICE_STATE_DEFAULT;
    State->Address = 0;
    USB_StateCallback(State);
    return LPC_OK;
}

// ---------------------------------------------------------------------------
// Process requests for control endpoint (ep0)
ErrorCode_t USB_EP_Class_Handler(USBD_HANDLE_T hUsb, void* data, uint32_t event)
{
    int core = (hUsb == USB_HANDLE(0)) ? 0 : 1;
    USB_CONTROLLER_STATE *State;
    USB_CORE_CTRL_T* pCtrl = (USB_CORE_CTRL_T*)hUsb;
    USB_SETUP_PACKET* Setup;
    BOOL firstPacket = FALSE;

    // Only handle control events
//    if (event != USB_EVT_SETUP && event != USB_EVT_IN && event != USB_EVT_OUT)
//          return ERR_USBD_UNHANDLED;

    // Packet received
    State = USB_STATE(core);
    Setup = (USB_SETUP_PACKET*)&pCtrl->SetupPacket;

    // Special handling for the very first SETUP request
    //   Getdescriptor[DeviceType], the host looks for 8 bytes data only            
    if ((Setup->bRequest == USB_GET_DESCRIPTOR) &&
          (((Setup->wValue & 0xFF00) >> 8) == USB_DEVICE_DESCRIPTOR_TYPE) &&
          (Setup->wLength != 0x12))
        firstPacket = TRUE;

    // Receive data and send it to the upper layer
    State->DataSize = USBD_ReadEP(hUsb, 0, State->Data);
    UINT8 result = USB_ControlCallback(State);

    switch(result)
    {
        case USB_STATE_DATA:
            // Setup packet was handled and the upper layer has data to send
            break;

        case USB_STATE_ADDRESS:
            // Upper layer needs us to change the address 
            // Handled by USB stack
            //USBD_SetAddress(hUsb, State->Address);
            break;

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
            break;

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
                USBD_WriteEP(hUsb, 0, State->Data, State->DataSize);
                // Special handling the USB driver set address test
                if (firstPacket) State->DataCallback = NULL;
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
    return LPC_OK;
}

// ---------------------------------------------------------------------------
// Receive buffer when handling data endpoint OUT event (Rx <-- Host OUT)
ErrorCode_t USB_EP_Out_Handler(USBD_HANDLE_T hUsb, void* data, uint32_t event)
{
    USB_ENDPOINT_DESCRIPTOR *epDesc = (USB_ENDPOINT_DESCRIPTOR*)data;
    int epNum;

    // Only handle OUT events
    if (event != USB_EVT_OUT) return ERR_USBD_UNHANDLED;

    epNum = epDesc->bEndpointAddress & 0x7F;

    return USB_EP_Receive(hUsb, epNum);
}

// ---------------------------------------------------------------------------
// Transmit buffer when handling data endpoint IN event (Tx -> Host IN)
ErrorCode_t USB_EP_In_Handler(USBD_HANDLE_T hUsb, void* data, uint32_t event)
{
    USB_ENDPOINT_DESCRIPTOR *epDesc = (USB_ENDPOINT_DESCRIPTOR*)data;
    int epNum;

    // Only handle IN events
    if (event != USB_EVT_IN) return ERR_USBD_UNHANDLED;

    epNum = epDesc->bEndpointAddress & 0x7F;

    return USB_EP_Transmit(hUsb, epNum);
}

// ---------------------------------------------------------------------------
static ErrorCode_t USB_EP_Receive(USBD_HANDLE_T hUsb, int epNum)
{
    int core = (hUsb == USB_HANDLE(0)) ? 0 : 1;
    USB_CONTROLLER_STATE *State;
    USB_PACKET64* Packet64;
    BOOL full;

    State = USB_STATE(core);

    // Return if this is not a valid receive endpoint
    if (State->Queues[epNum] == NULL || State->IsTxQueue[epNum])
        return ERR_USBD_UNHANDLED;

    Packet64 = USB_RxEnqueue(State, epNum, full);
    Packet64->Size = USBD_ReadEP(hUsb, epNum, Packet64->Buffer);

    return LPC_OK;
}

// ---------------------------------------------------------------------------
static ErrorCode_t USB_EP_Transmit(USBD_HANDLE_T hUsb, int epNum)
{
    int core = (hUsb == USB_HANDLE(0)) ? 0 : 1;
    USB_CONTROLLER_STATE *State;
    USB_PACKET64* Packet64;

    State = USB_STATE(core);

    // Return if this is not a valid transmit endpoint
    if (State->Queues[epNum] == NULL || !State->IsTxQueue[epNum])
        return ERR_USBD_UNHANDLED;

    for(;;)
    {
        Packet64 = USB_TxDequeue(State, epNum, FALSE); // Queue Peek
        USBD_WriteEP(hUsb, (0x80 | epNum), Packet64->Buffer, Packet64->Size);

        Packet64 = USB_TxDequeue(State, epNum, TRUE); // Queue Pop
        if(Packet64 == NULL || Packet64->Size > 0) break;
    }

    return LPC_OK;
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
USB_CONTROLLER_STATE *CPU_USB_GetState(int Controller)
{
    if (Controller >= MAX_USB_CORE) return NULL;
    return &usb_ctrl_state[Controller];
}

// ---------------------------------------------------------------------------
HRESULT CPU_USB_Initialize(int Controller)
{
    if (Controller >= MAX_USB_CORE || usb_core[Controller].enabled) return S_FALSE;

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
		if (!usb_core[1 - Controller].enabled)  // No other USB controllers enabled?
    {
        LPC_CGU->PLL[CGU_USB_PLL].PLL_CTRL &= ~1; // Enable USB PLL
        while (!(LPC_CGU->PLL[CGU_USB_PLL].PLL_STAT & 1)); // Wait for USB PLL to lock
		}
    //LPC_CCU1->CLKCCU[USB_CLK[Controller]].CFG |= 1; // Enable USBx base clock
    LPC_CGU->BASE_CLK[USB_CLK[Controller]] &= ~1;  // Enable USBx base clock
    LPC_CREG->CREG0 &= ~(1 << 5); // Enable USB0 PHY
		if (Controller == 1) // Special init for USB1
    {
        // Enable USB1_DP and USB1_DN on chip FS phy
        LPC_SCU->SFSUSB = 0x12; // USB device mode (0x16 for host mode)
        LPC_USB1->PORTSC1_D |= (1 << 24);
		}

    // Initilize USB parameter structure
    memset((void *) &usb_param, 0, sizeof(USBD_API_INIT_PARAM_T));
    // Configure USB memory
    usb_param.usb_reg_base = (uint32_t) USB_REG(Controller);
    usb_param.max_num_ep = 6;
    usb_param.mem_base = USBROM_BUFFADDR(Controller);
    usb_param.mem_size = USBROM_BUFFSIZE;
    usb_param.mem_size = USBROM_BUFFSIZE;
    // Configure USB callbacks
    usb_param.USB_Suspend_Event = &USB_Suspend_Event_Handler;
    usb_param.USB_Resume_Event = &USB_Resume_Event_Handler;
    usb_param.USB_Reset_Event = &USB_Reset_Event_Handler;

    // Set USB state
    State = CPU_USB_GetState(Controller);
    USB_STATE(Controller) = State;

    // Initialize USB stack with first device, configuration and string descriptors
    // ep0 handler will handle USB dynamic configuration
    USB_DEVICE_DESCRIPTOR* dev_desc = (USB_DEVICE_DESCRIPTOR*)USB_FindRecord(State, USB_DEVICE_DESCRIPTOR_MARKER, 0);
    core_desc.device_desc = (uint8_t *)&(dev_desc->bLength);
    USB_CONFIGURATION_DESCRIPTOR* hs_desc = (USB_CONFIGURATION_DESCRIPTOR*)USB_FindRecord(State, USB_CONFIGURATION_DESCRIPTOR_MARKER, 0);
    core_desc.high_speed_desc = (uint8_t *)&(hs_desc->bLength);
    core_desc.full_speed_desc = core_desc.high_speed_desc;
    core_desc.string_desc = (uint8_t *)USB_FindRecord(State, USB_STRING_DESCRIPTOR_MARKER, 0);
    core_desc.device_qualifier = NULL;

    State->EndpointCount = USB_MAX_EP_NUM;
    State->PacketSize = dev_desc ? dev_desc->bMaxPacketSize0 : USB_MAX_PACKET0;

    // Initialize USB stack
    ret = USBD_Init(&(USB_HANDLE(Controller)), &core_desc, &usb_param);
    if (ret != LPC_OK) return S_FALSE; // Exit if not succesful
    // Register endpoint class handler
    USBD_RegisterClassHandler(USB_HANDLE(Controller), USB_EP_Class_Handler, NULL);

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
        if (endpointSize != 8 && endpointSize != 16 && endpointSize != 32 && endpointSize != 64)
            return S_FALSE;
        State->MaxPacketSize[epNum] = endpointSize;

        // Assign queues
        QueueBuffers[queueId].Initialize();           // Clear queue before use
        State->Queues[epNum] = &QueueBuffers[queueId];  // Attach queue to endpoint
        queueId++;

        // Isochronous endpoints are currently not supported
        if ((epDesc->bmAttributes & 3) == USB_ENDPOINT_ATTRIBUTE_ISOCHRONOUS) return FALSE;

        // Register endpoint out and in handlers
        if ((epDesc->bEndpointAddress & 0x80) == USB_ENDPOINT_DIRECTION_OUT)
            USBD_RegisterEpHandler(USB_HANDLE(Controller),
                                   (2 * epNum ), USB_EP_Out_Handler, &epDesc);
        else
            USBD_RegisterEpHandler(USB_HANDLE(Controller),
                                   (2 * epNum + 1), USB_EP_In_Handler, &epDesc);
    }

    // Enable interrupts and make soft connect
    CPU_INTC_ActivateInterrupt(USB_IRQ[Controller], USB_ISR[Controller], 0);
    USBD_Connect(USB_HANDLE(Controller), 1); // Connect
    usb_core[Controller].enabled = TRUE;

    return S_OK;
}

// ---------------------------------------------------------------------------
HRESULT CPU_USB_Uninitialize( int Controller )
{
    if (Controller >= MAX_USB_CORE || !usb_core[Controller].enabled) return S_FALSE;

    // Make soft disconnect and disable interrupts
    USBD_Connect(USB_HANDLE(Controller), 0); // Disconnect
    CPU_INTC_DeactivateInterrupt(USB_IRQ[Controller]);

    // Uninitialize physical layer
    //LPC_CCU1->CLKCCU[USB_CLK[Controller]].CFG &= ~1;  // Disable USBx base clock
    LPC_CGU->BASE_CLK[USB_CLK[Controller]] |= 1;  // Disable USBx base clock
		if (usb_core[1 - Controller].enabled)  // No other USB controllers enabled?
    {
      LPC_CREG->CREG0 |= (1 << 5);; // Disable USB0 PHY
      LPC_CGU->PLL[CGU_USB_PLL].PLL_CTRL |= 1; // Disable USB PLL
		}
    USB_STATE(Controller) = NULL;
    usb_core[Controller].enabled = FALSE;

    return S_OK;
}

// ---------------------------------------------------------------------------
BOOL CPU_USB_StartOutput(USB_CONTROLLER_STATE* State, int epNum)
{
    USBD_HANDLE_T hUsb = (State == USB_STATE(0)) ? USB_HANDLE(0) : USB_HANDLE(1);

    if (State == NULL || epNum >= State->EndpointCount) return FALSE;

    GLOBAL_LOCK(irq);

    // If endpoint is not an output
    if (State->Queues[epNum] == NULL || !State->IsTxQueue[epNum])
        return FALSE;

    // If the halt feature for this endpoint is set, then just clear all the characters
    if (State->EndpointStatus[epNum] & USB_STATUS_ENDPOINT_HALT)
    {
        if (State->Queues[epNum] != NULL) State->Queues[epNum]->Initialize();
        return TRUE;
    }

    USB_EP_Transmit(hUsb, epNum);

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
    USBD_EnableEP(USB_HANDLE(State->ControllerNum), epNum);

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
BOOL CPU_USB_ProtectPins(int Controller, BOOL On)
{
    USB_CONTROLLER_STATE *State;

    GLOBAL_LOCK(irq);

    if (Controller >= MAX_USB_CORE) return FALSE;
    State = USB_STATE(Controller);

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
