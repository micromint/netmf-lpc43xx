/*
 * LPC43XX/LPC18XX USB ROM header
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
 * Simplified version of NXP LPCUSBLib headers for USB ROM stack
 * available in LPC43XX, LPC18XX, LPC1347 and LPC11UXX microcontrollers
 * 05/15/13  Micromint USA <support@micromint.com>
 */

#ifndef __LPC43XX_USB_H
#define __LPC43XX_USB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define USB_DEVICE_ROM_DRIVER
/*
 * LPC43XX and LPC18XX MCUs without internal flash include a USB stack
 * in ROM. Drivers for CDC, DFU, HID and MSC devices are implemented.
 * Drivers for custom classes can call core and HW APIs directly. Check
 * the LPC43XX User's Manual and the LPCUsbLib documentation for details.
 */

#ifndef _TINYHAL_H_
  #define USB_CONFIGURATION_DESCRIPTOR USBD_CONFIGURATION_DESCRIPTOR
  #define USB_DEVICE_DESCRIPTOR        USBD_DEVICE_DESCRIPTOR
  #define USB_ENDPOINT_DESCRIPTOR      USBD_ENDPOINT_DESCRIPTOR
  #define USB_INTERFACE_DESCRIPTOR     USBD_INTERFACE_DESCRIPTOR
  #define USB_SETUP_PACKET             USBD_SETUP_PACKET
#endif
	
#ifndef ROM_FUNCTION_TABLE_PTR_ADDR
/* Pointers to the MCU ROM function tables */
typedef struct _ROM {
	uint32_t p_otp;
	uint32_t p_aes;
	uint32_t p_pwd;
	uint32_t p_clk;
	uint32_t p_ipc;
	uint32_t p_spifi;
	uint32_t p_usbd;
}  ROM_FUNCTION_TABLE;

#define ROM_FUNCTION_TABLE_PTR_ADDR  (0x10400104)
#define MAX_USB_CORE  2

#define ROM_USBD_PTR  (((ROM_FUNCTION_TABLE *) \
                        (ROM_FUNCTION_TABLE_PTR_ADDR))->p_usbd)

/* ---------- ERROR_H ---------- */
/* Error codes returned by Boot ROM drivers/library functions */
/*   16 MSB = peripheral code    16 LSB = error code number */
typedef enum
{
  /*0x00000000*/ LPC_OK=0, /* enum value returned on Success */
  /*0xFFFFFFFF*/ ERR_FAILED = -1, /* enum value returned on general failure */

  /* ISP related errors */
  ERR_ISP_BASE = 0x00000000,
  /*0x00000001*/ ERR_ISP_INVALID_COMMAND = ERR_ISP_BASE + 1,
  /*0x00000002*/ ERR_ISP_SRC_ADDR_ERROR, /* Source address not on word boundary */
  /*0x00000003*/ ERR_ISP_DST_ADDR_ERROR, /* Destination address not on word or 256 byte boundary */
  /*0x00000004*/ ERR_ISP_SRC_ADDR_NOT_MAPPED,
  /*0x00000005*/ ERR_ISP_DST_ADDR_NOT_MAPPED,
  /*0x00000006*/ ERR_ISP_COUNT_ERROR, /* Byte count is not multiple of 4 or is not a permitted value */
  /*0x00000007*/ ERR_ISP_INVALID_SECTOR,
  /*0x00000008*/ ERR_ISP_SECTOR_NOT_BLANK,
  /*0x00000009*/ ERR_ISP_SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION,
  /*0x0000000A*/ ERR_ISP_COMPARE_ERROR,
  /*0x0000000B*/ ERR_ISP_BUSY, /* Flash programming hardware interface is busy */
  /*0x0000000C*/ ERR_ISP_PARAM_ERROR, /* Insufficient number of parameters */
  /*0x0000000D*/ ERR_ISP_ADDR_ERROR, /* Address not on word boundary */
  /*0x0000000E*/ ERR_ISP_ADDR_NOT_MAPPED,
  /*0x0000000F*/ ERR_ISP_CMD_LOCKED, /* Command is locked */
  /*0x00000010*/ ERR_ISP_INVALID_CODE, /* Unlock code is invalid */
  /*0x00000011*/ ERR_ISP_INVALID_BAUD_RATE,
  /*0x00000012*/ ERR_ISP_INVALID_STOP_BIT,
  /*0x00000013*/ ERR_ISP_CODE_READ_PROTECTION_ENABLED,

  /* ROM API related errors */
  ERR_API_BASE = 0x00010000,
  /*0x00010001*/ ERR_API_INVALID_PARAMS = ERR_API_BASE + 1, /* Invalid parameters*/
  /*0x00010002*/ ERR_API_INVALID_PARAM1, /* PARAM1 is invalid */
  /*0x00010003*/ ERR_API_INVALID_PARAM2, /* PARAM2 is invalid */
  /*0x00010004*/ ERR_API_INVALID_PARAM3, /* PARAM3 is invalid */
  /*0x00010005*/ ERR_API_MOD_INIT, /* API is called before module init */

  /* SPIFI API related errors */
  ERR_SPIFI_BASE = 0x00020000,
  /*0x00020001*/ ERR_SPIFI_DEVICE_ERROR =ERR_SPIFI_BASE+1,
  /*0x00020002*/ ERR_SPIFI_INTERNAL_ERROR,
  /*0x00020003*/ ERR_SPIFI_TIMEOUT,
  /*0x00020004*/ ERR_SPIFI_OPERAND_ERROR,
  /*0x00020005*/ ERR_SPIFI_STATUS_PROBLEM,
  /*0x00020006*/ ERR_SPIFI_UNKNOWN_EXT,
  /*0x00020007*/ ERR_SPIFI_UNKNOWN_ID,
  /*0x00020008*/ ERR_SPIFI_UNKNOWN_TYPE,
  /*0x00020009*/ ERR_SPIFI_UNKNOWN_MFG,

  /* Security API related errors */
  ERR_SEC_BASE = 0x00030000,
  /*0x00030001*/	ERR_SEC_AES_WRONG_CMD=ERR_SEC_BASE+1,
  /*0x00030002*/	ERR_SEC_AES_NOT_SUPPORTED,
  /*0x00030003*/	ERR_SEC_AES_KEY_ALREADY_PROGRAMMED,

  /* USB device stack related errors */
  ERR_USBD_BASE = 0x00040000,
  /*0x00040001*/ ERR_USBD_INVALID_REQ = ERR_USBD_BASE + 1, /* invalid request */
  /*0x00040002*/ ERR_USBD_UNHANDLED, /* Callback did not process the event */
  /*0x00040003*/ ERR_USBD_STALL,     /* Stall the endpoint on which the call back is called */
  /*0x00040004*/ ERR_USBD_SEND_ZLP,  /* Send ZLP packet on the endpoint on which the call back is called */
  /*0x00040005*/ ERR_USBD_SEND_DATA, /* Send data packet on the endpoint on which the call back is called */
  /*0x00040006*/ ERR_USBD_BAD_DESC,  /* Bad descriptor*/
  /*0x00040007*/ ERR_USBD_BAD_CFG_DESC,/* Bad config descriptor*/
  /*0x00040009*/ ERR_USBD_BAD_INTF_DESC,/* Bad interface descriptor*/
  /*0x0004000a*/ ERR_USBD_BAD_EP_DESC,/* Bad endpoint descriptor*/
  /*0x0004000b*/ ERR_USBD_BAD_MEM_BUF, /* Bad alignment of buffer passed. */
  /*0x0004000c*/ ERR_USBD_TOO_MANY_CLASS_HDLR, /* Too many class handlers. */

  /* CGU  related errors */
  ERR_CGU_BASE = 0x00050000,
  /*0x00050001*/ ERR_CGU_NOT_IMPL=ERR_CGU_BASE+1,
  /*0x00050002*/ ERR_CGU_INVALID_PARAM,
  /*0x00050003*/ ERR_CGU_INVALID_SLICE,
  /*0x00050004*/ ERR_CGU_OUTPUT_GEN,
  /*0x00050005*/ ERR_CGU_DIV_SRC,
  /*0x00050006*/ ERR_CGU_DIV_VAL,
  /*0x00050007*/ ERR_CGU_SRC

} ErrorCode_t;

//#define offsetof(s,m)   (int)&(((s *)0)->m)
#define COMPILE_TIME_ASSERT(pred)    switch(0){case 0:case pred:;}

#endif /* ROM_FUNCTION_TABLE_PTR_ADDR */

/* ---------- USBD_H ---------- */
#if defined(__ARMCC_VERSION)
  #define PRE_PACK	__packed
  #define POST_PACK
  #define ALIGNED(n)  __align(n)
#elif defined(__IAR_SYSTEMS_ICC__)
  #define PRE_PACK	__packed
  #define POST_PACK
#else /* defined(__GNUC__) and others */
  #define PRE_PACK
  #define POST_PACK	__attribute__((__packed__))
  #define ALIGNED(n)  __attribute__((aligned (n)))
#endif

/* Structure to pack lower and upper byte to form 16 bit word */
PRE_PACK struct POST_PACK _WB_T
{
  uint8_t L; /* lower byte */
  uint8_t H; /* upper byte */
};
/* Structure to pack lower and upper byte to form 16 bit word */
typedef struct _WB_T WB_T;

/* Union of @ref _WB_T struct and 16 bit word */
PRE_PACK union POST_PACK __WORD_BYTE
{
  PRE_PACK uint16_t POST_PACK W; /* data member to do 16 bit access */
  WB_T WB; /* data member to do 8 bit access */
} ;
/* Union of @ref _WB_T struct and 16 bit word */
typedef union __WORD_BYTE WORD_BYTE;

#define REQUEST_HOST_TO_DEVICE     0
#define REQUEST_DEVICE_TO_HOST     1

#define REQUEST_STANDARD           0
#define REQUEST_CLASS              1
#define REQUEST_VENDOR             2
#define REQUEST_RESERVED           3

#define REQUEST_TO_DEVICE          0
#define REQUEST_TO_INTERFACE       1
#define REQUEST_TO_ENDPOINT        2
#define REQUEST_TO_OTHER           3

/* Structure to define 8 bit USB request */
PRE_PACK struct POST_PACK _BM_T
{
  uint8_t Recipient :  5; /* Recipeint type */
  uint8_t Type      :  2; /* Request type.  */
  uint8_t Dir       :  1; /* Directtion type */
};
/* Structure to define 8 bit USB request */
typedef struct _BM_T BM_T;

/* Union of @ref _BM_T struct and 8 bit byte */
PRE_PACK union POST_PACK _REQUEST_TYPE
{
//  PRE_PACK uint8_t POST_PACK B; /* byte wide access memeber */
  uint8_t B;
  BM_T BM;   /* bitfield structure access memeber */
} ;
/* Union of @ref _BM_T struct and 8 bit byte */
typedef union _REQUEST_TYPE REQUEST_TYPE;

#define USB_REQUEST_GET_STATUS                 0
#define USB_REQUEST_CLEAR_FEATURE              1
#define USB_REQUEST_SET_FEATURE                3
#define USB_REQUEST_SET_ADDRESS                5
#define USB_REQUEST_GET_DESCRIPTOR             6
#define USB_REQUEST_SET_DESCRIPTOR             7
#define USB_REQUEST_GET_CONFIGURATION          8
#define USB_REQUEST_SET_CONFIGURATION          9
#define USB_REQUEST_GET_INTERFACE              10
#define USB_REQUEST_SET_INTERFACE              11
#define USB_REQUEST_SYNC_FRAME                 12

#define USB_GETSTATUS_SELF_POWERED             0x01
#define USB_GETSTATUS_REMOTE_WAKEUP            0x02
#define USB_GETSTATUS_ENDPOINT_STALL           0x01

#define USB_FEATURE_ENDPOINT_STALL             0
#define USB_FEATURE_REMOTE_WAKEUP              1
#define USB_FEATURE_TEST_MODE                  2

PRE_PACK struct POST_PACK _USB_SETUP_PACKET
{
  REQUEST_TYPE bmRequestType; /* Characteristics of the request */
  uint8_t      bRequest; /* Specifies the particular request */
  WORD_BYTE    wValue; /* Used to pass a parameter to the device */
  WORD_BYTE    wIndex; /* Used to pass a parameter to the device */
  uint16_t     wLength; /* Length of the data transferred */
} ;
/* USB Default Control Pipe Setup Packet*/
typedef struct _USB_SETUP_PACKET USBD_SETUP_PACKET;

#define USB_DEVICE_DESCRIPTOR_TYPE             1
#define USB_CONFIGURATION_DESCRIPTOR_TYPE      2
#define USB_STRING_DESCRIPTOR_TYPE             3
#define USB_INTERFACE_DESCRIPTOR_TYPE          4
#define USB_ENDPOINT_DESCRIPTOR_TYPE           5
#define USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE   6
#define USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE 7
#define USB_INTERFACE_POWER_DESCRIPTOR_TYPE    8
#define USB_OTG_DESCRIPTOR_TYPE                     9
#define USB_DEBUG_DESCRIPTOR_TYPE                  10
#define USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE  11

#define USB_DEVICE_CLASS_RESERVED              0x00
#define USB_DEVICE_CLASS_AUDIO                 0x01
#define USB_DEVICE_CLASS_COMMUNICATIONS        0x02
#define USB_DEVICE_CLASS_HUMAN_INTERFACE       0x03
#define USB_DEVICE_CLASS_MONITOR               0x04
#define USB_DEVICE_CLASS_PHYSICAL_INTERFACE    0x05
#define USB_DEVICE_CLASS_POWER                 0x06
#define USB_DEVICE_CLASS_PRINTER               0x07
#define USB_DEVICE_CLASS_STORAGE               0x08
#define USB_DEVICE_CLASS_HUB                   0x09
#define USB_DEVICE_CLASS_MISCELLANEOUS         0xEF
#define USB_DEVICE_CLASS_APP                   0xFE
#define USB_DEVICE_CLASS_VENDOR_SPECIFIC       0xFF

#define USB_CONFIG_POWERED_MASK                0x40
#define USB_CONFIG_BUS_POWERED                 0x80
#define USB_CONFIG_SELF_POWERED                0xC0
#define USB_CONFIG_REMOTE_WAKEUP               0x20

/* bMaxPower in Configuration Descriptor */
//#define USB_CONFIG_POWER_MA(mA)                ((mA)/2)

#define USB_ENDPOINT_DIRECTION_MASK            0x80
#define USB_ENDPOINT_OUT(addr)                 ((addr) | 0x00)
#define USB_ENDPOINT_IN(addr)                  ((addr) | 0x80)

#define USB_ENDPOINT_TYPE_MASK                 0x03
#define USB_ENDPOINT_TYPE_CONTROL              0x00
#define USB_ENDPOINT_TYPE_ISOCHRONOUS          0x01
#define USB_ENDPOINT_TYPE_BULK                 0x02
#define USB_ENDPOINT_TYPE_INTERRUPT            0x03
#define USB_ENDPOINT_SYNC_MASK                 0x0C
#define USB_ENDPOINT_SYNC_NO_SYNCHRONIZATION   0x00
#define USB_ENDPOINT_SYNC_ASYNCHRONOUS         0x04
#define USB_ENDPOINT_SYNC_ADAPTIVE             0x08
#define USB_ENDPOINT_SYNC_SYNCHRONOUS          0x0C
#define USB_ENDPOINT_USAGE_MASK                0x30
#define USB_ENDPOINT_USAGE_DATA                0x00
#define USB_ENDPOINT_USAGE_FEEDBACK            0x10
#define USB_ENDPOINT_USAGE_IMPLICIT_FEEDBACK   0x20
#define USB_ENDPOINT_USAGE_RESERVED            0x30

/* Control endopint EP0's maximum packet size in high-speed mode */
#define USB_ENDPOINT_0_HS_MAXP                 64
/* Control endopint EP0's maximum packet size in low-speed mode */
#define USB_ENDPOINT_0_LS_MAXP                 8
/* Bulk endopint's maximum packet size in high-speed mode */
#define USB_ENDPOINT_BULK_HS_MAXP              512

/* USB Standard Device Descriptor */
PRE_PACK struct POST_PACK _USB_DEVICE_DESCRIPTOR
{
  uint8_t  bLength;     /* Size of this descriptor in bytes */
  uint8_t  bDescriptorType; /* DEVICE Descriptor Type */
  uint16_t bcdUSB; /* BUSB Specification Release Number BCD (2.10 is 210H) */
  uint8_t  bDeviceClass; /* Class code (assigned by the USB-IF) */
  uint8_t  bDeviceSubClass; /* Subclass code (assigned by the USB-IF) */
  uint8_t  bDeviceProtocol; /* Protocol code (assigned by the USB-IF) */
  uint8_t  bMaxPacketSize0; /* Maximum packet size for endpoint zero */
  uint16_t idVendor; /* Vendor ID (assigned by the USB-IF) */
  uint16_t idProduct; /* Product ID (assigned by the manufacturer) */
  uint16_t bcdDevice; /* Device release number in binary-coded decimal */
  uint8_t  iManufacturer; /* Index of manufacturer string descriptor */
  uint8_t  iProduct; /* Index of product string descriptor */
  uint8_t  iSerialNumber; /* Index of serial number string descriptor */
  uint8_t  bNumConfigurations; /* Number of possible configurations */
} ;
/* USB Standard Device Descriptor */
typedef struct _USB_DEVICE_DESCRIPTOR USBD_DEVICE_DESCRIPTOR;

/* USB 2.0 Device Qualifier Descriptor */
PRE_PACK struct POST_PACK _USB_DEVICE_QUALIFIER_DESCRIPTOR
{
  uint8_t  bLength; /* Size of descriptor */
  uint8_t  bDescriptorType; /* Device Qualifier Type */
  uint16_t bcdUSB; /* USB specification version number (e.g., 0200H for V2.00) */
  uint8_t  bDeviceClass; /* Class Code */
  uint8_t  bDeviceSubClass; /* SubClass Code */
  uint8_t  bDeviceProtocol; /* Protocol Code */
  uint8_t  bMaxPacketSize0; /* Maximum packet size for other speed */
  uint8_t  bNumConfigurations; /* Number of Other-speed Configurations */
  uint8_t  bReserved; /* Reserved for future use, must be zero */
} ;
/* USB 2.0 Device Qualifier Descriptor */
typedef struct _USB_DEVICE_QUALIFIER_DESCRIPTOR USB_DEVICE_QUALIFIER_DESCRIPTOR;

/* USB Standard Configuration Descriptor */
PRE_PACK struct POST_PACK _USB_CONFIGURATION_DESCRIPTOR
{
  uint8_t  bLength; /* Size of this descriptor in bytes */
  uint8_t  bDescriptorType; /* CONFIGURATION Descriptor Type*/
  uint16_t wTotalLength; /* Total length of data for this configuration */
  uint8_t  bNumInterfaces; /* Number of interfaces supported */
  uint8_t  bConfigurationValue; /* Value to select this configuration */
  uint8_t  iConfiguration; /* Index of configuration string descriptor */
  uint8_t  bmAttributes; /* Configuration characteristics */
  uint8_t  bMaxPower; /* Maximum power consumption of the USB device */
} ;
/* USB Standard Configuration Descriptor */
typedef struct _USB_CONFIGURATION_DESCRIPTOR USBD_CONFIGURATION_DESCRIPTOR;

/* USB Standard Interface Descriptor */
PRE_PACK struct POST_PACK _USB_INTERFACE_DESCRIPTOR
{
  uint8_t  bLength; /* Size of this descriptor in bytes*/
  uint8_t  bDescriptorType; /* INTERFACE Descriptor Type*/
  uint8_t  bInterfaceNumber; /* Number of this interface (zero-based) */
  uint8_t  bAlternateSetting; /* Value used to select this alternate setting */
  uint8_t  bNumEndpoints; /* Number of endpoints used */
  uint8_t  bInterfaceClass; /* Class code (assigned by the USB-IF) */
  uint8_t  bInterfaceSubClass; /* Subclass code (assigned by the USB-IF) */
  uint8_t  bInterfaceProtocol; /* Protocol code (assigned by the USB) */
  uint8_t  iInterface; /* Index of string descriptor describing this interface*/
} ;
/* USB Standard Interface Descriptor */
typedef struct _USB_INTERFACE_DESCRIPTOR USBD_INTERFACE_DESCRIPTOR;

/* USB Standard Endpoint Descriptor */
PRE_PACK struct POST_PACK _USB_ENDPOINT_DESCRIPTOR
{
  uint8_t  bLength; /* Size of this descriptor in bytes*/
  uint8_t  bDescriptorType; /* ENDPOINT Descriptor Type*/
  uint8_t  bEndpointAddress; /* Endpoint address */
  uint8_t  bmAttributes; /* Endpoint’s attributes */
  uint16_t wMaxPacketSize; /* Maximum packet size supported by endpoint */
  uint8_t  bInterval; /* Interval for polling endpoint */
} ;
/* USB Standard Endpoint Descriptor */
typedef struct _USB_ENDPOINT_DESCRIPTOR USBD_ENDPOINT_DESCRIPTOR;

/* USB String Descriptor */
PRE_PACK struct POST_PACK _USB_STRING_DESCRIPTOR
{
  uint8_t  bLength; /* Size of this descriptor in bytes*/
  uint8_t  bDescriptorType; /* STRING Descriptor Type*/
  uint16_t bString/*[]*/; /* UNICODE encoded string */
}  ;
/* USB String Descriptor */
typedef struct _USB_STRING_DESCRIPTOR USB_STRING_DESCRIPTOR;

/* USB Common Descriptor */
PRE_PACK struct POST_PACK _USB_COMMON_DESCRIPTOR
{
  uint8_t  bLength; /* Size of this descriptor in bytes*/
  uint8_t  bDescriptorType; /* Descriptor Type*/
} ;
/* USB Common Descriptor */
typedef struct _USB_COMMON_DESCRIPTOR USB_COMMON_DESCRIPTOR;

/* USB Other Speed Configuration */
PRE_PACK struct POST_PACK _USB_OTHER_SPEED_CONFIGURATION
{
  uint8_t  bLength; /* Size of descriptor*/
  uint8_t  bDescriptorType; /* Other_speed_Configuration Type*/
  uint16_t wTotalLength; /* Total length of data returned*/
  uint8_t  bNumInterfaces; /* Number of interfaces supported by config */
  uint8_t  bConfigurationValue; /* Value to use to select configuration*/
  uint8_t  IConfiguration; /* Index of string descriptor*/
  uint8_t  bmAttributes; /* Same as Configuration descriptor*/
  uint8_t  bMaxPower; /* Same as Configuration descriptor*/
} ;
/* USB Other Speed Configuration */
typedef struct _USB_OTHER_SPEED_CONFIGURATION USB_OTHER_SPEED_CONFIGURATION;

/* USB device stack/module handle */
typedef void* USBD_HANDLE_T;

#define WBVAL(x) ((x) & 0xFF),(((x) >> 8) & 0xFF)
#define B3VAL(x) ((x) & 0xFF),(((x) >> 8) & 0xFF),(((x) >> 16) & 0xFF)

#define USB_DEVICE_DESC_SIZE        (sizeof(USBD_DEVICE_DESCRIPTOR))
#define USB_CONFIGURATION_DESC_SIZE (sizeof(USBD_CONFIGURATION_DESCRIPTOR))
#define USB_INTERFACE_DESC_SIZE     (sizeof(USBD_INTERFACE_DESCRIPTOR))
#define USB_ENDPOINT_DESC_SIZE      (sizeof(USBD_ENDPOINT_DESCRIPTOR))
#define USB_DEVICE_QUALI_SIZE       (sizeof(USB_DEVICE_QUALIFIER_DESCRIPTOR))
#define USB_OTHER_SPEED_CONF_SIZE   (sizeof(USB_OTHER_SPEED_CONFIGURATION))

uint32_t CALLBACK_UsbdRom_Register_DeviceDescriptor(void);
uint32_t CALLBACK_UsbdRom_Register_ConfigurationDescriptor(void);
uint32_t CALLBACK_UsbdRom_Register_StringDescriptor(void);
uint32_t CALLBACK_UsbdRom_Register_DeviceQualifierDescriptor(void);
uint8_t CALLBACK_UsbdRom_Register_ConfigureEndpoint(void);

/* ---------- USBD_CORE_H ---------- */
/* Function pointer types */
typedef ErrorCode_t (*USB_CB_T) (USBD_HANDLE_T hUsb);
typedef ErrorCode_t (*USB_PARAM_CB_T) (USBD_HANDLE_T hUsb, uint32_t param1);
typedef ErrorCode_t (*USB_EP_HANDLER_T)(USBD_HANDLE_T hUsb, void* data, uint32_t event);

/* USB descriptors data structure */
typedef struct _USB_CORE_DESCS_T
{
  uint8_t *device_desc; /* Pointer to USB device descriptor */
  uint8_t *string_desc; /* Pointer to array of USB string descriptors */
  uint8_t *full_speed_desc; /* Pointer to USB FS device config descriptor */
  uint8_t *high_speed_desc; /* Pointer to USB HS device config descriptor */
  uint8_t *device_qualifier; /* Pointer to USB device qualifier descriptor */
} USB_CORE_DESCS_T;

/* USB device stack initilization parameter data structure */
typedef struct USBD_API_INIT_PARAM
{
  uint32_t usb_reg_base; /* USB device controller's base register address. */
  uint32_t mem_base;  /* Base memory location */
  uint32_t mem_size;  /* Size of memory buffer for stack use */
  uint8_t max_num_ep; /* max number of endpoints supported */

  /* USB Device Events Callback Functions */
  USB_CB_T USB_Reset_Event;
  USB_CB_T USB_Suspend_Event;
  USB_CB_T USB_Resume_Event;
  USB_CB_T reserved_sbz;
  USB_CB_T USB_SOF_Event;
  USB_PARAM_CB_T USB_WakeUpCfg;
  USB_PARAM_CB_T USB_Power_Event;
  USB_PARAM_CB_T USB_Error_Event;

  USB_CB_T USB_Configure_Event;
  USB_CB_T USB_Interface_Event;
  USB_CB_T USB_Feature_Event;
  uint32_t (* virt_to_phys)(void* vaddr);
  void (* cache_flush)(uint32_t* start_adr, uint32_t* end_adr);
} USBD_API_INIT_PARAM_T;

/* Core API functions structure */
typedef struct USBD_CORE_API
{
  ErrorCode_t (*RegisterClassHandler)(USBD_HANDLE_T hUsb, USB_EP_HANDLER_T pfn,
               void* data);
  ErrorCode_t (*RegisterEpHandler)(USBD_HANDLE_T hUsb, uint32_t ep_index,
               USB_EP_HANDLER_T pfn, void* data);
  void (*SetupStage )(USBD_HANDLE_T hUsb);
  void (*DataInStage)(USBD_HANDLE_T hUsb);
  void (*DataOutStage)(USBD_HANDLE_T hUsb);
  void (*StatusInStage)(USBD_HANDLE_T hUsb);
  void (*StatusOutStage)(USBD_HANDLE_T hUsb);
  void (*StallEp0)(USBD_HANDLE_T hUsb);
} USBD_CORE_API_T;

/*-----------------------------------------------------------------------------
 *  Private functions & structures prototypes
 *-----------------------------------------------------------------------------*/
/* forward declaration */
struct _USB_CORE_CTRL_T;
typedef struct _USB_CORE_CTRL_T  USB_CORE_CTRL_T;

/* USB device Speed status defines */
#define USB_FULL_SPEED    0
#define USB_HIGH_SPEED    1

/* USB Endpoint Data Structure */
typedef struct _USB_EP_DATA
{
  uint8_t  *pData;
  uint16_t   Count;
} USB_EP_DATA;

#define USB_MAX_IF_NUM  8
#define USB_MAX_EP_NUM  6

/* USB core controller data structure */
struct _USB_CORE_CTRL_T
{
  /* overridable function pointers ~ c++ style virtual functions*/
  USB_CB_T USB_EvtSetupHandler;
  USB_CB_T USB_EvtOutHandler;
  USB_PARAM_CB_T USB_ReqVendor;
  USB_CB_T USB_ReqGetStatus;
  USB_CB_T USB_ReqGetDescriptor;
  USB_CB_T USB_ReqGetConfiguration;
  USB_CB_T USB_ReqSetConfiguration;
  USB_CB_T USB_ReqGetInterface;
  USB_CB_T USB_ReqSetInterface;
  USB_PARAM_CB_T USB_ReqSetClrFeature;

  /* USB Device Events Callback Functions */
  USB_CB_T USB_Reset_Event;
  USB_CB_T USB_Suspend_Event;
  USB_CB_T USB_Resume_Event;
  USB_CB_T USB_SOF_Event;
  USB_PARAM_CB_T USB_Power_Event;
  USB_PARAM_CB_T USB_Error_Event;
  USB_PARAM_CB_T USB_WakeUpCfg;

  /* USB Core Events Callback Functions */
  USB_CB_T USB_Configure_Event;
  USB_CB_T USB_Interface_Event;
  USB_CB_T USB_Feature_Event;

  /* cache and mmu translation functions */
  uint32_t (* virt_to_phys)(void* vaddr);
  void (* cache_flush)(uint32_t* start_adr, uint32_t* end_adr);

  /* event handlers for endpoints. */
  USB_EP_HANDLER_T  ep_event_hdlr[2 * USB_MAX_EP_NUM];
  void*  ep_hdlr_data[2 * USB_MAX_EP_NUM];

  /* USB class handlers */
  USB_EP_HANDLER_T  ep0_hdlr_cb[USB_MAX_IF_NUM];
  void*  ep0_cb_data[USB_MAX_IF_NUM];
  uint8_t num_ep0_hdlrs;
  /* USB Core data Variables */
  uint8_t max_num_ep; /* max number of endpoints supported by the HW */
  uint8_t device_speed;
  uint8_t  num_interfaces;
  uint8_t  device_addr;
  uint8_t  config_value;
  uint16_t device_status;
  uint8_t *device_desc;
  uint8_t *string_desc;
  uint8_t *full_speed_desc;
  uint8_t *high_speed_desc;
  uint8_t *device_qualifier;
  uint32_t ep_mask;
  uint32_t ep_halt;
  uint32_t ep_stall;
  uint8_t  alt_setting[USB_MAX_IF_NUM];
  /* HW driver data pointer */
  void* hw_data;

  /* USB Endpoint 0 Data Info */
  USB_EP_DATA EP0Data;

  /* USB Endpoint 0 Buffer */
  //ALIGNED(4)
  uint8_t  EP0Buf[64];

  /* USB Setup Packet */
  //ALIGNED(4)
  USBD_SETUP_PACKET SetupPacket;

};

static __INLINE void USB_SetSpeedMode(USB_CORE_CTRL_T* pCtrl, uint8_t mode)
{
  pCtrl->device_speed = mode;
}

/* ---------- USBD_HW_H ---------- */
/* USB Endpoint/class handler Callback Events */
enum USBD_EVENT_T {
  USB_EVT_SETUP =1,    /* 1   Setup Packet received */
  USB_EVT_OUT,         /* 2   OUT Packet received */
  USB_EVT_IN,          /* 3    IN Packet sent */
  USB_EVT_OUT_NAK,     /* 4   OUT Packet - Not Acknowledged */
  USB_EVT_IN_NAK,      /* 5    IN Packet - Not Acknowledged */
  USB_EVT_OUT_STALL,   /* 6   OUT Packet - Stalled */
  USB_EVT_IN_STALL,    /* 7    IN Packet - Stalled */
  USB_EVT_OUT_DMA_EOT, /* 8   DMA OUT EP - End of Transfer */
  USB_EVT_IN_DMA_EOT,  /* 9   DMA  IN EP - End of Transfer */
  USB_EVT_OUT_DMA_NDR, /* 10  DMA OUT EP - New Descriptor Request */
  USB_EVT_IN_DMA_NDR,  /* 11  DMA  IN EP - New Descriptor Request */
  USB_EVT_OUT_DMA_ERR, /* 12  DMA OUT EP - Error */
  USB_EVT_IN_DMA_ERR,  /* 13  DMA  IN EP - Error */
  USB_EVT_RESET,       /* 14  Reset event received */
  USB_EVT_SOF,         /* 15  Start of Frame event */
  USB_EVT_DEV_STATE,   /* 16  Device status events */
  USB_EVT_DEV_ERROR,   /* 16  Device error events */
};

/* Hardware API functions structure */
typedef struct USBD_HW_API
{
  uint32_t (*GetMemSize)(USBD_API_INIT_PARAM_T* param);
  ErrorCode_t (*Init)(USBD_HANDLE_T* phUsb, USB_CORE_DESCS_T* pDesc,
               USBD_API_INIT_PARAM_T* param);
  void (*Connect)(USBD_HANDLE_T hUsb, uint32_t con);
  void (*ISR)(USBD_HANDLE_T hUsb);
  void  (*Reset)(USBD_HANDLE_T hUsb);
  void  (*ForceFullSpeed )(USBD_HANDLE_T hUsb, uint32_t cfg);
  void  (*WakeUpCfg)(USBD_HANDLE_T hUsb, uint32_t  cfg);
  void  (*SetAddress)(USBD_HANDLE_T hUsb, uint32_t adr);
  void  (*Configure)(USBD_HANDLE_T hUsb, uint32_t  cfg);
  void  (*ConfigEP)(USBD_HANDLE_T hUsb, USBD_ENDPOINT_DESCRIPTOR *pEPD);
  void  (*DirCtrlEP)(USBD_HANDLE_T hUsb, uint32_t dir);
  void  (*EnableEP)(USBD_HANDLE_T hUsb, uint32_t EPNum);
  void  (*DisableEP)(USBD_HANDLE_T hUsb, uint32_t EPNum);
  void  (*ResetEP)(USBD_HANDLE_T hUsb, uint32_t EPNum);
  void  (*SetStallEP)(USBD_HANDLE_T hUsb, uint32_t EPNum);
  void  (*ClrStallEP)(USBD_HANDLE_T hUsb, uint32_t EPNum);
  ErrorCode_t (*SetTestMode)(USBD_HANDLE_T hUsb, uint8_t mode);
  uint32_t (*ReadEP)(USBD_HANDLE_T hUsb, uint32_t EPNum, uint8_t *pData);
  uint32_t (*ReadReqEP)(USBD_HANDLE_T hUsb, uint32_t EPNum,
            uint8_t *pData, uint32_t len);
  uint32_t (*ReadSetupPkt)(USBD_HANDLE_T hUsb, uint32_t EPNum, uint32_t *pData);
  uint32_t (*WriteEP)(USBD_HANDLE_T hUsb, uint32_t EPNum,
            uint8_t *pData, uint32_t cnt);
  void  (*WakeUp)(USBD_HANDLE_T hUsb);
  ErrorCode_t  (*EnableEvent)(USBD_HANDLE_T hUsb, uint32_t EPNum,
                uint32_t event_type, uint32_t enable);

} USBD_HW_API_T;

/* ---------- USBD_MSCUSER_H ---------- */
/* Mass Storage Class (MSC) Function Driver */

/* MSC function driver initilization parameter data structure */
typedef struct USBD_MSC_INIT_PARAM
{
  /* memory allocation params */
  uint32_t mem_base;
  uint32_t mem_size;
  /* mass storage params */
  uint8_t*  InquiryStr; /* Pointer to response to the SCSI Inquiry command */
  uint32_t  BlockCount; /* Number of blocks present in device */
  uint32_t  BlockSize; /* Block size in bytes */
  uint32_t  MemorySize; /* Memory size in bytes */
  uint8_t* intf_desc;

  /* user defined functions */
  void (*MSC_Write)( uint32_t offset, uint8_t** src, uint32_t length);
  void (*MSC_Read)( uint32_t offset, uint8_t** dst, uint32_t length);
  ErrorCode_t (*MSC_Verify)( uint32_t offset, uint8_t buf[], uint32_t length);
  void (*MSC_GetWriteBuf)( uint32_t offset, uint8_t** buff_adr, uint32_t length);
  ErrorCode_t (*MSC_Ep0_Hdlr) (USBD_HANDLE_T hUsb, void* data, uint32_t event);
} USBD_MSC_INIT_PARAM_T;

/* MSC class API functions structure */
typedef struct USBD_MSC_API
{
  uint32_t (*GetMemSize)(USBD_MSC_INIT_PARAM_T* param);
  ErrorCode_t (*init)(USBD_HANDLE_T hUsb, USBD_MSC_INIT_PARAM_T* param);
} USBD_MSC_API_T;

/* Bulk-only Command Block Wrapper */
PRE_PACK struct POST_PACK _MSC_CBW
{
  uint32_t dSignature;
  uint32_t dTag;
  uint32_t dDataLength;
  uint8_t  bmFlags;
  uint8_t  bLUN;
  uint8_t  bCBLength;
  uint8_t  CB[16];
} ;
typedef struct _MSC_CBW MSC_CBW;

/* Bulk-only Command Status Wrapper */
PRE_PACK struct POST_PACK _MSC_CSW
{
  uint32_t dSignature;
  uint32_t dTag;
  uint32_t dDataResidue;
  uint8_t  bStatus;
} ;
typedef struct _MSC_CSW MSC_CSW;

/*-----------------------------------------------------------------------------
 *  Private functions & structures prototypes
 *-----------------------------------------------------------------------------*/
typedef struct _MSC_CTRL_T
{
  /* If it's a USB HS, the max packet is 512, if it's USB FS,
  the max packet is 64. Use 512 for both HS and FS. */
  /*ALIGNED(4)*/ uint8_t  BulkBuf[512]; /* Bulk In/Out Buffer */
  /*ALIGNED(4)*/MSC_CBW CBW;                   /* Command Block Wrapper */
  /*ALIGNED(4)*/MSC_CSW CSW;                   /* Command Status Wrapper */

  USB_CORE_CTRL_T*  pUsbCtrl;

  uint32_t Offset;                  /* R/W Offset */
  uint32_t Length;                  /* R/W Length */
  uint32_t BulkLen;                 /* Bulk In/Out Length */
  uint8_t* rx_buf;

  uint8_t BulkStage;               /* Bulk Stage */
  uint8_t if_num;                  /* interface number */
  uint8_t epin_num;                /* BULK IN endpoint number */
  uint8_t epout_num;               /* BULK OUT endpoint number */
  uint32_t MemOK;                  /* Memory OK */

  uint8_t*  InquiryStr;
  uint32_t  BlockCount;
  uint32_t  BlockSize;
  uint32_t  MemorySize;

  /* user defined functions */
  void (*MSC_Write)( uint32_t offset, uint8_t** src, uint32_t length);
  void (*MSC_Read)( uint32_t offset, uint8_t** dst, uint32_t length);
  ErrorCode_t (*MSC_Verify)( uint32_t offset, uint8_t src[], uint32_t length);
  /* optional call back for MSC_Write optimization */
  void (*MSC_GetWriteBuf)( uint32_t offset, uint8_t** buff_adr, uint32_t length);
}USB_MSC_CTRL_T;

/* ---------- USBD_DFUUSER_H ---------- */
/* Firmware Upgrade (DFU) Class Function Driver */

/* USB DFU descriptors data structure */
typedef struct USBD_DFU_INIT_PARAM
{
  /* memory allocation params */
  uint32_t mem_base;
  uint32_t mem_size;
  /* DFU paramas */
  uint16_t wTransferSize; /* DFU transfer block size in bytes */
  uint8_t* intf_desc;

  /* user defined functions */
  uint8_t (*DFU_Write)( uint32_t block_num, uint8_t** src, uint32_t length,
                        uint8_t* bwPollTimeout);
  uint32_t (*DFU_Read)( uint32_t block_num, uint8_t** dst, uint32_t length);
  void (*DFU_Done)(void);
  void (*DFU_Detach)(USBD_HANDLE_T hUsb);
  ErrorCode_t (*DFU_Ep0_Hdlr) (USBD_HANDLE_T hUsb, void* data, uint32_t event);
} USBD_DFU_INIT_PARAM_T;


/* DFU class API functions structure */
typedef struct USBD_DFU_API
{
  uint32_t (*GetMemSize)(USBD_DFU_INIT_PARAM_T* param);
  ErrorCode_t (*init)(USBD_HANDLE_T hUsb, USBD_DFU_INIT_PARAM_T* param,
               uint32_t init_state);
} USBD_DFU_API_T;

PRE_PACK struct POST_PACK _DFU_STATUS {
  uint8_t bStatus;
  uint8_t bwPollTimeout[3];
  uint8_t bState;
  uint8_t iString;
};
typedef struct _DFU_STATUS DFU_STATUS_T;

PRE_PACK struct POST_PACK _USB_DFU_FUNC_DESCRIPTOR {
  uint8_t   bLength;
  uint8_t   bDescriptorType;
  uint8_t   bmAttributes;
  uint16_t  wDetachTimeOut;
  uint16_t  wTransferSize;
  uint16_t  bcdDFUVersion;
};
typedef struct _USB_DFU_FUNC_DESCRIPTOR USB_DFU_FUNC_DESCRIPTOR;

/*-----------------------------------------------------------------------------
 *  Private functions & structures prototypes
 *-----------------------------------------------------------------------------*/
typedef struct _USBD_DFU_CTRL_T
{
  /*ALIGNED(4)*/ DFU_STATUS_T dfu_req_get_status;
  uint8_t dfu_state;
  uint8_t dfu_status;
  uint8_t download_done;
  uint8_t if_num;                  /* interface number */

  uint8_t* xfr_buf;
  USB_DFU_FUNC_DESCRIPTOR* dfu_desc;

  USB_CORE_CTRL_T*  pUsbCtrl;
  /* user defined functions */
  /* return DFU_STATUS_ values defined in mw_usbd_dfu.h */
  uint8_t (*DFU_Write)( uint32_t block_num, uint8_t** src, uint32_t length, uint8_t* bwPollTimeout);
  /* return
  * DFU_STATUS_ : values defined in mw_usbd_dfu.h in case of errors
  * 0 : If end of memory reached
  * length : Amount of data copied to destination buffer
  */
  uint32_t (*DFU_Read)( uint32_t block_num, uint8_t** dst, uint32_t length);
  /* callback called after download is finished */
  void (*DFU_Done)(void);
  /* callback called after USB_REQ_DFU_DETACH is recived */
  void (*DFU_Detach)(USBD_HANDLE_T hUsb);

} USBD_DFU_CTRL_T;

/* ---------- USBD_HIDUSER_H ---------- */
/* HID Class Function Driver */

/* HID report descriptor data structure */
typedef struct _HID_REPORT_T {
  uint16_t len; /* Size of the report descriptor in bytes. */
  uint8_t idle_time;
  uint8_t __pad; /* Padding space. */
  uint8_t* desc; /* Report descriptor. */
} USB_HID_REPORT_T;

/* USB HID descriptors data structure */
typedef struct USBD_HID_INIT_PARAM
{
  /* memory allocation params */
  uint32_t mem_base;
  uint32_t mem_size;
  /* HID params */
  uint8_t max_reports;
  uint8_t* intf_desc;
  USB_HID_REPORT_T* report_data;

  /* user defined functions */
  /* required functions */
  ErrorCode_t (*HID_GetReport)( USBD_HANDLE_T hHid, USBD_SETUP_PACKET* pSetup,
               uint8_t** pBuffer, uint16_t* length);
  ErrorCode_t (*HID_SetReport)( USBD_HANDLE_T hHid, USBD_SETUP_PACKET* pSetup,
               uint8_t** pBuffer, uint16_t length);
  /* optional functions */
  ErrorCode_t (*HID_GetPhysDesc)( USBD_HANDLE_T hHid, USBD_SETUP_PACKET* pSetup,
               uint8_t** pBuf, uint16_t* length);
  ErrorCode_t (*HID_SetIdle)( USBD_HANDLE_T hHid, USBD_SETUP_PACKET* pSetup,
               uint8_t idleTime);
  ErrorCode_t (*HID_SetProtocol)( USBD_HANDLE_T hHid, USBD_SETUP_PACKET* pSetup,
               uint8_t protocol);

  ErrorCode_t (*HID_EpIn_Hdlr) (USBD_HANDLE_T hUsb, void* data, uint32_t event);
  ErrorCode_t (*HID_EpOut_Hdlr) (USBD_HANDLE_T hUsb, void* data, uint32_t event);
  /* user overridable function */
  ErrorCode_t (*HID_GetReportDesc)(USBD_HANDLE_T hHid, USBD_SETUP_PACKET* pSetup,
               uint8_t** pBuf, uint16_t* length);
  ErrorCode_t (*HID_Ep0_Hdlr) (USBD_HANDLE_T hUsb, void* data, uint32_t event);
} USBD_HID_INIT_PARAM_T;

/* HID class API functions structure */
typedef struct USBD_HID_API
{
  uint32_t (*GetMemSize)(USBD_HID_INIT_PARAM_T* param);
  ErrorCode_t (*init)(USBD_HANDLE_T hUsb, USBD_HID_INIT_PARAM_T* param);
} USBD_HID_API_T;

/*-----------------------------------------------------------------------------
 *  Private functions & structures prototypes
 *-----------------------------------------------------------------------------*/
typedef struct _HID_CTRL_T {
  /* pointer to controller */
  USB_CORE_CTRL_T*  pUsbCtrl;
  /* descriptor pointers */
  uint8_t* hid_desc;
  USB_HID_REPORT_T* report_data;

  uint8_t protocol;
  uint8_t if_num;                  /* interface number */
  uint8_t epin_adr;                /* IN interrupt endpoint */
  uint8_t epout_adr;               /* OUT interrupt endpoint */

  /* user defined functions */
  ErrorCode_t (*HID_GetReport)( USBD_HANDLE_T hHid, USBD_SETUP_PACKET* pSetup,
               uint8_t** pBuffer, uint16_t* length);
  ErrorCode_t (*HID_SetReport)( USBD_HANDLE_T hHid, USBD_SETUP_PACKET* pSetup,
               uint8_t** pBuffer, uint16_t length);
  ErrorCode_t (*HID_GetPhysDesc)( USBD_HANDLE_T hHid, USBD_SETUP_PACKET* pSetup,
               uint8_t** pBuf, uint16_t* length);
  ErrorCode_t (*HID_SetIdle)( USBD_HANDLE_T hHid, USBD_SETUP_PACKET* pSetup,
               uint8_t idleTime);
  ErrorCode_t (*HID_SetProtocol)( USBD_HANDLE_T hHid, USBD_SETUP_PACKET* pSetup,
               uint8_t protocol);

  /* virtual overridable functions */
  ErrorCode_t (*HID_GetReportDesc)(USBD_HANDLE_T hHid, USBD_SETUP_PACKET* pSetup,
               uint8_t** pBuf, uint16_t* length);
}USB_HID_CTRL_T;

/* ---------- USBD_CDCUSER_H ---------- */
/* Communication Device Class (CDC) Function Driver */

/* Buffer for incomming data on USB port */
/* Buffer masks */
#define CDC_BUF_SIZE               (128)  /* Output buffer in bytes (power 2) */
                                          /* large enough for file transfer */
#define CDC_BUF_MASK               (CDC_BUF_SIZE-1ul)

/* Line coding structure */
PRE_PACK struct POST_PACK _CDC_LINE_CODING {
  uint32_t dwDTERate;                            /* Data terminal rate in bits per second */
  uint8_t  bCharFormat;                          /* Number of stop bits */
  uint8_t  bParityType;                          /* Parity bit type */
  uint8_t  bDataBits;                            /* Number of data bits */
};
typedef struct _CDC_LINE_CODING CDC_LINE_CODING;

/* CDC function driver initilization parameter data structure */
typedef struct USBD_CDC_INIT_PARAM
{
  /* memory allocation params */
  uint32_t mem_base;
  uint32_t mem_size;
  uint8_t* cif_intf_desc;
  uint8_t* dif_intf_desc;

  /* user defined functions */
  /* required functions */
  ErrorCode_t (*CIC_GetRequest)( USBD_HANDLE_T hHid, USBD_SETUP_PACKET* pSetup,
               uint8_t** pBuffer, uint16_t* length); 
  ErrorCode_t (*CIC_SetRequest)( USBD_HANDLE_T hCdc, USBD_SETUP_PACKET* pSetup,
               uint8_t** pBuffer, uint16_t length);
  ErrorCode_t (*CDC_BulkIN_Hdlr) (USBD_HANDLE_T hUsb, void* data, uint32_t event);
  ErrorCode_t (*CDC_BulkOUT_Hdlr) (USBD_HANDLE_T hUsb, void* data,
               uint32_t event);
  ErrorCode_t (*SendEncpsCmd) (USBD_HANDLE_T hCDC, uint8_t* buffer, uint16_t len);
  ErrorCode_t (*GetEncpsResp) (USBD_HANDLE_T hCDC, uint8_t** buffer,
               uint16_t* len);
  ErrorCode_t (*SetCommFeature) (USBD_HANDLE_T hCDC, uint16_t feature,
               uint8_t* buffer, uint16_t len);
  ErrorCode_t (*GetCommFeature) (USBD_HANDLE_T hCDC, uint16_t feature,
               uint8_t** pBuffer, uint16_t* len);
  ErrorCode_t (*ClrCommFeature) (USBD_HANDLE_T hCDC, uint16_t feature);
  ErrorCode_t (*SetCtrlLineState) (USBD_HANDLE_T hCDC, uint16_t state);
  ErrorCode_t (*SendBreak) (USBD_HANDLE_T hCDC, uint16_t mstime);
  ErrorCode_t (*SetLineCode) (USBD_HANDLE_T hCDC, CDC_LINE_CODING* line_coding);
  /* optional functions */
  ErrorCode_t (*CDC_InterruptEP_Hdlr) (USBD_HANDLE_T hUsb, void* data,
               uint32_t event);
  ErrorCode_t (*CDC_Ep0_Hdlr) (USBD_HANDLE_T hUsb, void* data, uint32_t event);
} USBD_CDC_INIT_PARAM_T;

/* CDC class API functions structure */
typedef struct USBD_CDC_API
{
  uint32_t (*GetMemSize)(USBD_CDC_INIT_PARAM_T* param);
  ErrorCode_t (*init)(USBD_HANDLE_T hUsb, USBD_CDC_INIT_PARAM_T* param,
               USBD_HANDLE_T* phCDC);
  ErrorCode_t (*SendNotification)(USBD_HANDLE_T hCdc, uint8_t bNotification,
               uint16_t data);
} USBD_CDC_API_T;

/*-----------------------------------------------------------------------------
 *  Private functions & structures prototypes
 *-----------------------------------------------------------------------------*/
typedef struct _CDC_CTRL_T
{
  USB_CORE_CTRL_T*  pUsbCtrl;
  /* notification buffer */
  uint8_t notice_buf[12];
  CDC_LINE_CODING line_coding;
 
  uint8_t cif_num;                 /* control interface number */
  uint8_t dif_num;                 /* data interface number */
  uint8_t epin_num;                /* BULK IN endpoint number */
  uint8_t epout_num;               /* BULK OUT endpoint number */
  uint8_t epint_num;               /* Interrupt IN endpoint number */

  /* user defined functions */
  ErrorCode_t (*SendEncpsCmd) (USBD_HANDLE_T hCDC, uint8_t* buffer, uint16_t len);
  ErrorCode_t (*GetEncpsResp) (USBD_HANDLE_T hCDC, uint8_t** buffer, uint16_t* len);
  ErrorCode_t (*SetCommFeature) (USBD_HANDLE_T hCDC, uint16_t feature, uint8_t* buffer, uint16_t len);
  ErrorCode_t (*GetCommFeature) (USBD_HANDLE_T hCDC, uint16_t feature, uint8_t** pBuffer, uint16_t* len);
  ErrorCode_t (*ClrCommFeature) (USBD_HANDLE_T hCDC, uint16_t feature);
  ErrorCode_t (*SetCtrlLineState) (USBD_HANDLE_T hCDC, uint16_t state);
  ErrorCode_t (*SendBreak) (USBD_HANDLE_T hCDC, uint16_t state);
  ErrorCode_t (*SetLineCode) (USBD_HANDLE_T hCDC, CDC_LINE_CODING* line_coding);

  /* virtual functions */
  ErrorCode_t (*CIC_GetRequest)( USBD_HANDLE_T hCdc, USBD_SETUP_PACKET* pSetup, uint8_t** pBuffer, uint16_t* length);
  ErrorCode_t (*CIC_SetRequest)( USBD_HANDLE_T hCdc, USBD_SETUP_PACKET* pSetup, uint8_t** pBuffer, uint16_t length);

}USB_CDC_CTRL_T;

void UsbdCdc_IO_Buffer_Sync_Task(void);
void UsbdCdc_SendData(uint8_t* buffer, uint32_t cnt);
uint32_t UsbdCdc_RecvData(uint8_t* buffer, uint32_t len);

/* ---------- USBD_ROM_API_H ---------- */
/* USBD API functions structure */
typedef struct USBD_API
{ /* Pointer to function tables */
  USBD_HW_API_T* hw; /* USB device stack's core layer */
  USBD_CORE_API_T* core; /* USB device controller hardware */
  USBD_MSC_API_T* msc; /* MSC function driver module */
  USBD_DFU_API_T* dfu; /* DFU function driver module */
  USBD_HID_API_T* hid; /* HID function driver module */
  USBD_CDC_API_T* cdc; /* CDC-ACM function driver module */
  uint32_t* reserved6; /* Reserved for future function driver module */
  uint32_t version; /* Version identifier of USB ROM stack */
} USBD_API_T;

#define USBD_API (((USBD_API_T*)(ROM_USBD_PTR)))

#define USBD_HW    USBD_API->hw
#define USBD_CORE  USBD_API->core
#define USBD_MSC   USBD_API->msc
#define USBD_DFU   USBD_API->dfu
#define USBD_HID   USBD_API->hid
#define USBD_CDC   USBD_API->cdc

extern USBD_HANDLE_T UsbHandle;

/* USBD core functions */
void UsbdRom_Init(uint8_t corenum);
void UsbdRom_IrqHandler(void);
/* USBD MSC functions */
void UsbdMsc_Init(void);
/* USBD HID functions */
void UsbdHid_Init(void);
/* USBD CDC functions */
void UsbdCdc_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __LPC43XX_USB_H */
