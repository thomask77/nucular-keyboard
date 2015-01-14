/**
 * Nucular Keyboard - HID Interface
 * Copyright (C)2015 Thomas Kindler <mail_nucular@t-kindler.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "usbd_hid.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include <assert.h>


#define WORD(x)     (x & 255), (x >> 8)


// HID Keyboard report descriptor
//
static uint8_t KeyboardReportDesc[] = {
    0x05, 0x01,         // Usage Page (Generic Desktop)
    0x09, 0x06,         // Usage (Keyboard)
    0xA1, 0x01,         // Collection (Application)
    0x75, 0x01,         //     Report Size (1)
    0x95, 0x08,         //     Report Count (8)
    0x05, 0x07,         //     Usage Page (Keyboard/Keypad)
    0x19, 0xE0,         //     Usage Minimum (Keyboard Left Control)
    0x29, 0xE7,         //     Usage Maximum (Keyboard Right GUI)
    0x15, 0x00,         //     Logical Minimum (0)
    0x25, 0x01,         //     Logical Maximum (1)
    0x81, 0x02,         //     Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
    0x95, 0x01,         //     Report Count (1)
    0x75, 0x08,         //     Report Size (8)
    0x81, 0x01,         //     Input (Cnst,Ary,Abs)
    0x95, 0x05,         //     Report Count (5)
    0x75, 0x01,         //     Report Size (1)
    0x05, 0x08,         //     Usage Page (LEDs)
    0x19, 0x01,         //     Usage Minimum
    0x29, 0x05,         //     Usage Maximum
    0x91, 0x02,         //     Output (Data,Var,Abs,NWrp,Lin,Pref,NNul,NVol,Bit)
    0x95, 0x01,         //     Report Count (1)
    0x75, 0x03,         //     Report Size (3)
    0x91, 0x01,         //     Output (Cnst,Ary,Abs,NWrp,Lin,Pref,NNul,NVol,Bit)
    0x95, 0x06,         //     Report Count (6)
    0x75, 0x08,         //     Report Size (8)
    0x26, 0xFF, 0x00,   //     Logical Maximum (255)
    0x05, 0x07,         //     Usage Page (Keyboard/Keypad)
    0x19, 0x00,         //     Usage Minimum
    0x29, 0xFF,         //     Usage Maximum
    0x81, 0x00,         //     Input (Data,Ary,Abs)
    0xC0                // End Collection
};


// HID Mouse report descriptor
//
static uint8_t MouseReportDesc[] = {
    0x05, 0x01,         //  Usage Page (Generic Desktop)
    0x09, 0x02,         //  Usage (Mouse)
    0xA1, 0x01,         //  Collection (Application)
    0x09, 0x01,         //      Usage (Pointer)
    0xA1, 0x00,         //      Collection (Physical)
    0x05, 0x09,         //          Usage Page (Button)
    0x19, 0x01,         //          Usage Minimum (Button 1)
    0x29, 0x08,         //          Usage Maximum (Button 8)
    0x15, 0x00,         //          Logical Minimum (0)
    0x25, 0x01,         //          Logical Maximum (1)
    0x75, 0x01,         //          Report Size (1)
    0x95, 0x08,         //          Report Count (8)
    0x81, 0x02,         //          Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
    0x05, 0x01,         //          Usage Page (Generic Desktop)
    0x09, 0x30,         //          Usage (X)
    0x09, 0x31,         //          Usage (Y)
    0x09, 0x38,         //          Usage (Wheel)
    0x15, 0x81,         //          Logical Minimum (-127)
    0x25, 0x7F,         //          Logical Maximum (127)
    0x75, 0x08,         //          Report Size (8)
    0x95, 0x03,         //          Report Count (3)
    0x81, 0x06,         //          Input (Data,Var,Rel,NWrp,Lin,Pref,NNul,Bit)
    0x05, 0x0C,         //          Usage Page (Consumer Devices)
    0x0A, 0x38, 0x02,   //          Usage (AC Pan)
    0x95, 0x01,         //          Report Count (1)
    0x81, 0x06,         //          Input (Data,Var,Rel,NWrp,Lin,Pref,NNul,Bit)
    0xC0,               //      End Collection
    0xC0,               //  End Collection
};


// Extra HID reports
//
static uint8_t ExtraReportDesc[] = {
    // System control buttons
    //
    0x05, 0x01,         // Usage Page (Generic Desktop)
    0x09, 0x80,         // Usage (System Control)
    0xA1, 0x01,         // Collection (Application)
    0x85, 0x01,         //     Report ID (1)
    0x19, 0x81,         //     Usage Minimum (System Power Down)
    0x29, 0x83,         //     Usage Maximum (System Wake Up)
    0x09, 0xA8,         //     Usage (System Hibernate)
    0x15, 0x00,         //     Logical Minimum (0)
    0x25, 0x01,         //     Logical Maximum (1)
    0x95, 0x04,         //     Report Count (4)
    0x75, 0x01,         //     Report Size (1)
    0x81, 0x02,         //     Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
    0x95, 0x01,         //     Report Count (1)
    0x75, 0x04,         //     Report Size (4)
    0x81, 0x01,         //     Input (Cnst,Ary,Abs)
    0xC0,               // End Collection

    // Media control buttons
    //
    0x05, 0x0C,         // Usage Page (Consumer Devices)
    0x09, 0x01,         // Usage (Consumer Control)
    0xA1, 0x01,         // Collection (Application)
    0x85, 0x02,         //     Report ID (2)
    0x15, 0x00,         //     Logical Minimum (0)
    0x25, 0x01,         //     Logical Maximum (1)
    0x75, 0x01,         //     Report Size (1)
    0x95, 0x08,         //     Report Count (8)

    0x09, 0xE9,         //     Usage (Volume Increment)
    0x09, 0xEA,         //     Usage (Volume Decrement)
    0x09, 0xE2,         //     Usage (Mute)
    0x09, 0xCD,         //     Usage (Play/Pause)
    0x09, 0xB5,         //     Usage Minimum (Scan Next Track)
    0x09, 0xB6,         //     Usage Minimum (Scan Previous Track)
    0x09, 0xB7,         //     Usage Minimum (Stop)
    0x09, 0xB8,         //     Usage Maximum (Eject)
    0x81, 0x02,         //     Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)

    0x0A, 0x8A, 0x01,   //     Usage (AL Email Reader)
    0x0A, 0x21, 0x02,   //     Usage (AC Search)
    0x0A, 0x2A, 0x02,   //     Usage (AC Bookmarks)
    0x0A, 0x23, 0x02,   //     Usage (AC Home)
    0x0A, 0x24, 0x02,   //     Usage (AC_Back)
    0x0A, 0x25, 0x02,   //     Usage (AC Forward)
    0x0A, 0x26, 0x02,   //     Usage (AC Stop)
    0x0A, 0x27, 0x02,   //     Usage (AC Refresh)
    0x81, 0x02,         //     Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)

    0x0A, 0x83, 0x01,   //     Usage (AL Consumer Control Configuration)
    0x0A, 0x96, 0x01,   //     Usage (AL Internet Browser)
    0x0A, 0x92, 0x01,   //     Usage (AL Calculator)
    0x0A, 0x9E, 0x01,   //     Usage (AL Terminal Lock/Screensaver)
    0x0A, 0x94, 0x01,   //     Usage (AL Local Machine Browser)
    0x0A, 0x06, 0x02,   //     Usage (AC Minimize)
    0x09, 0x6F,         //     Usage (Brightness Increment, >= Win 8.1)
    0x09, 0x70,         //     Usage (Brightness Decrement, >= Win 8.1)
    0x81, 0x02,         //     Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
    0xC0,               // End Collection

    /*
    // Mute, Power, etc. LEDs
    //
    0x05, 0x08,         // Usage Page (LEDs)
    0x09, 0x01,         // Usage (Consumer Control)
    0xA1, 0x01,         // Collection (Application)
    0x85, 0x03,         //     Report ID (3)
    0x15, 0x00,         //     Logical Minimum (0)
    0x25, 0x01,         //     Logical Maximum (1)
    0x09, 0x09,         //     Usage (Mute)
    0x95, 0x01,         //     Report Count (1)
    0x75, 0x01,         //     Report Size (1)
    0x05, 0x08,         //     Usage Page (LEDs)
    0x91, 0x02,         //     Output (Data,Var,Abs,NWrp,Lin,Pref,NNul,NVol,Bit)
    0x95, 0x01,         //     Report Count (1)
    0x75, 0x07,         //     Report Size (7)
    0x91, 0x01,         //     Output (Cnst,Ary,Abs,NWrp,Lin,Pref,NNul,NVol,Bit)
    0xC0                // End Collection
    */

    // HID detach for Bootloader (see UM0412)
    // Does _not_ work in ST's DfuSe >= 3.0.3
    //
    0x06, 0x00, 0xFF,   // Vendor defined usage page - 0xFF00
    0x09, 0x55,         // Usage (HID Detach)
    0xA1, 0x01,         // Collection (Application)
    0x85, 0x80,         // REPORT_ID (128)
    0x09, 0x55,         // USAGE (HID Detach)
    0x15, 0x00,         // LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,   // LOGICAL_MAXIMUM (255)
    0x75, 0x08,         // REPORT_SIZE (8 bits)
    0x95, 0x01,         // REPORT_COUNT (1)
    0xB1, 0x82,         // FEATURE (Data,Var,Abs,Vol)
    0xC0,               // END_COLLECTION (Vendor defined)
};


// HID Debug (compatible with PJRC's hid_listen)
//
static uint8_t DebugReportDesc[] = {
    0x06, 0x31, 0xff,   // Usage Page (Vendor-Defined)
    0x09, 0x74,         // Usage (Vendor-Defined)
    0xa1, 0x53,         // Collection
    0x75, 0x08,         //     Report Size (8)
    0x15, 0x00,         //     Logical Minimum (0)
    0x26, 0xFF, 0x00,   //     Logical Maximum (255)
    0x95, 0x3F,         //     Report Count (63)
    0x09, 0x75,         //     Usage (Vendor-Defined)
    0x81, 0x02,         //     Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit)
//    0x09, 0x75,         //     Usage (Vendor-Defined 1)
//    0x91, 0x02,         //     Output (Data,Var,Abs,NWrp,Lin,Pref,NNul,NVol,Bit)
    0xc0                // End Collection
};


/* USB HID device Configuration Descriptor */
static uint8_t USBD_HID_CfgDesc[USB_HID_CONFIG_DESC_SIZ] = {
    USB_LEN_CFG_DESC,         /* bLength: Configuration Descriptor size */
    USB_DESC_TYPE_CONFIGURATION, /* bDescriptorType: Configuration */
    WORD(USB_HID_CONFIG_DESC_SIZ),  /* wTotalLength: Bytes returned */
    0x04,         /*bNumInterfaces */
    0x01,         /*bConfigurationValue: Configuration value*/
    0x00,         /*iConfiguration: Index of string descriptor describing the configuration*/
    0xA0,         /*bmAttributes: bus powered and Support Remote Wake-up */
    0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/



    /************** Descriptor of Keyboard interface ****************/
    USB_LEN_IF_DESC,         /*bLength: Interface Descriptor size*/
    USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
    0x00,         /*bInterfaceNumber: Number of Interface*/
    0x00,         /*bAlternateSetting: Alternate setting*/
    0x01,         /*bNumEndpoints*/
    0x03,         /*bInterfaceClass: HID*/
    0x01,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x01,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0x00,         /*iInterface: Index of string descriptor*/

    /******************** Descriptor of Keyboard HID ********************/
    USB_LEN_HID_DESC,         /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
    0x11, 0x01,         /*bcdHID: HID Class Spec release number*/
    0x00,         /*bCountryCode: Hardware target country*/
    0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
    HID_REPORT_DESC,         /*bDescriptorType*/
    WORD(HID_KEYBOARD_REPORT_DESC_SIZE), /*wItemLength: Total length of Report descriptor*/

    /******************** Descriptor of Keyboard endpoint ********************/
    USB_LEN_EP_DESC,          /*bLength: Endpoint Descriptor size*/
    USB_DESC_TYPE_ENDPOINT, /*bDescriptorType:*/
    HID_KEYBOARD_EPIN_ADDR,          /* bEndpointAddress: Endpoint Address (IN) */
    0x03,                     /* bmAttributes: Interrupt endpoint */
    WORD(HID_KEYBOARD_EPIN_SIZE),               /* wMaxPacketSize: 8 Byte max */
    HID_KEYBOARD_POLLING_INTERVAL,       /* bInterval: Polling Interval (10 ms) */



    /************** Descriptor of Mouse interface ****************/
    USB_LEN_IF_DESC,         /*bLength: Interface Descriptor size*/
    USB_DESC_TYPE_INTERFACE,/*bDescriptorType: Interface descriptor type*/
    0x01,         /*bInterfaceNumber: Number of Interface*/
    0x00,         /*bAlternateSetting: Alternate setting*/
    0x01,         /*bNumEndpoints*/
    0x03,         /*bInterfaceClass: HID*/
    0x01,         /*bInterfaceSubClass : 1=BOOT, 0=no boot*/
    0x02,         /*nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse*/
    0x00,         /*iInterface: Index of string descriptor*/

    /******************** Descriptor of Mouse HID ********************/
    USB_LEN_HID_DESC,         /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
    0x11, 0x01,        /*bcdHID: HID Class Spec release number*/
    0x00,         /*bCountryCode: Hardware target country*/
    0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
    HID_REPORT_DESC,         /*bDescriptorType*/
    WORD(HID_MOUSE_REPORT_DESC_SIZE), /*wItemLength: Total length of Report descriptor*/

    /******************** Descriptor of Mouse endpoint ********************/
    USB_LEN_EP_DESC,              // bLength: Endpoint Descriptor size
    USB_DESC_TYPE_ENDPOINT,       // bDescriptorType:
    HID_MOUSE_EPIN_ADDR,          // bEndpointAddress: Endpoint Address (IN)
    0x03,                         // bmAttributes: Interrupt endpoint
    WORD(HID_MOUSE_EPIN_SIZE),    // wMaxPacketSize
    HID_MOUSE_POLLING_INTERVAL,   // bInterval


    // Extra function interface
    //
    USB_LEN_IF_DESC,                    // bLength: Interface Descriptor size
    USB_DESC_TYPE_INTERFACE,            // bDescriptorType: Interface descriptor type
    0x02,                               // bInterfaceNumber: Number of Interface
    0x00,                               // bAlternateSetting: Alternate setting
    0x01,                               // bNumEndpoints
    0x03,                               // bInterfaceClass: HID
    0x00,                               // bInterfaceSubClass : 1=BOOT, 0=no boot
    0x00,                               // nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse
    0x00,                               // iInterface: Index of string descriptor

    USB_LEN_HID_DESC,                   // bLength
    HID_DESCRIPTOR_TYPE,                // bDescriptorType
    0x11, 0x01,                         // bcdHID
    0x00,                               // bCountryCode
    0x01,                               // bNumDescriptors
    HID_REPORT_DESC,                    // bDescriptorType
    WORD(HID_EXTRA_REPORT_DESC_SIZE),   // wItemLength

    USB_LEN_EP_DESC,                    // bLength
    USB_DESC_TYPE_ENDPOINT,             // bDescriptorType
    HID_EXTRA_EPIN_ADDR,                // bEndpointAddress
    0x03,                               // bmAttributes: Interrupt endpoint
    WORD(HID_EXTRA_EPIN_SIZE),          // wMaxPacketSize
    HID_EXTRA_POLLING_INTERVAL,         // bInterval

    // HID Debug Interface
    //
    USB_LEN_IF_DESC,                    // bLength: Interface Descriptor size
    USB_DESC_TYPE_INTERFACE,            // bDescriptorType: Interface descriptor type
    0x03,                               // bInterfaceNumber: Number of Interface
    0x00,                               // bAlternateSetting: Alternate setting
    0x01,                               // bNumEndpoints
    0x03,                               // bInterfaceClass: HID
    0x00,                               // bInterfaceSubClass : 1=BOOT, 0=no boot
    0x00,                               // nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse
    0x00,                               // iInterface: Index of string descriptor

    USB_LEN_HID_DESC,                   // bLength
    HID_DESCRIPTOR_TYPE,                // bDescriptorType
    0x11, 0x01,                         // bcdHID
    0x00,                               // bCountryCode
    0x01,                               // bNumDescriptors
    HID_REPORT_DESC,                    // bDescriptorType
    WORD(HID_DEBUG_REPORT_DESC_SIZE),   // wItemLength

    USB_LEN_EP_DESC,                    // bLength
    USB_DESC_TYPE_ENDPOINT,             // bDescriptorType
    HID_DEBUG_EPIN_ADDR,                // bEndpointAddress
    0x03,                               // bmAttributes: Interrupt endpoint
    WORD(HID_DEBUG_EPIN_SIZE),          // wMaxPacketSize
    HID_DEBUG_POLLING_INTERVAL          // bInterval
};


/* USB HID device Configuration Descriptor */
static uint8_t USBD_HID_Desc_Mouse[USB_LEN_HID_DESC] = {
    USB_LEN_HID_DESC,         /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID */
    0x11, 0x01,   /*bcdHID: HID Class Spec release number*/
    0x00,         /*bCountryCode: Hardware target country*/
    0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
    HID_REPORT_DESC,         /*bDescriptorType*/
    WORD(HID_MOUSE_REPORT_DESC_SIZE) /*wItemLength: Total length of Report descriptor*/
};


static uint8_t USBD_HID_Desc_Keyboard[USB_LEN_HID_DESC] = {
    USB_LEN_HID_DESC,         /*bLength: HID Descriptor size*/
    HID_DESCRIPTOR_TYPE, /*bDescriptorType: HID*/
    0x11, 0x01,        /*bcdHID: HID Class Spec release number*/
    0x00,         /*bCountryCode: Hardware target country*/
    0x01,         /*bNumDescriptors: Number of HID class descriptors to follow*/
    HID_REPORT_DESC,         /*bDescriptorType*/
    WORD(HID_KEYBOARD_REPORT_DESC_SIZE) /*wItemLength: Total length of Report descriptor*/
};


static uint8_t USBD_HID_Desc_Extra[USB_LEN_HID_DESC] = {
    USB_LEN_HID_DESC,               // bLength
    HID_DESCRIPTOR_TYPE,            // bDescriptorType
    0x11, 0x01,                     // bcdHID
    0x00,                           // bCountryCode
    0x01,                           // bNumDescriptors
    HID_REPORT_DESC,                // bDescriptorType
    WORD(HID_EXTRA_REPORT_DESC_SIZE)  // wItemLength
};


static uint8_t USBD_HID_Desc_Debug[USB_LEN_HID_DESC] = {
    USB_LEN_HID_DESC,               // bLength
    HID_DESCRIPTOR_TYPE,            // bDescriptorType
    0x11, 0x01,                     // bcdHID
    0x00,                           // bCountryCode
    0x01,                           // bNumDescriptors
    HID_REPORT_DESC,                // bDescriptorType
    WORD(HID_DEBUG_REPORT_DESC_SIZE)  // wItemLength
};


/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_HID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    /* Open EP IN */
    USBD_LL_OpenEP(pdev, HID_KEYBOARD_EPIN_ADDR,  USBD_EP_TYPE_INTR, HID_KEYBOARD_EPIN_SIZE);
    USBD_LL_OpenEP(pdev, HID_MOUSE_EPIN_ADDR,     USBD_EP_TYPE_INTR, HID_MOUSE_EPIN_SIZE);
    USBD_LL_OpenEP(pdev, HID_EXTRA_EPIN_ADDR,     USBD_EP_TYPE_INTR, HID_EXTRA_EPIN_SIZE);
    USBD_LL_OpenEP(pdev, HID_DEBUG_EPIN_ADDR,     USBD_EP_TYPE_INTR, HID_DEBUG_EPIN_SIZE);

    static USBD_HID_HandleTypeDef mem;

    memset(&mem, 0, sizeof(mem));

    pdev->pClassData = &mem;

    return 0;
}


/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_HID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    /* Close HID EPs */
    USBD_LL_CloseEP(pdev, HID_KEYBOARD_EPIN_ADDR);
    USBD_LL_CloseEP(pdev, HID_MOUSE_EPIN_ADDR);
    USBD_LL_CloseEP(pdev, HID_EXTRA_EPIN_ADDR);
    USBD_LL_CloseEP(pdev, HID_DEBUG_EPIN_ADDR);

    /* Free allocated memory */
    if (pdev->pClassData != NULL)
        pdev->pClassData = NULL;
    return USBD_OK;
}


/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_HID_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
    USBD_HID_HandleTypeDef *hhid = pdev->pClassData;

    switch (req->bmRequest & USB_REQ_TYPE_MASK) {
    case USB_REQ_TYPE_CLASS:
        switch (req->bRequest) {

        case HID_REQ_SET_PROTOCOL:
            hhid->Protocol = req->wValue;
            break;

        case HID_REQ_GET_PROTOCOL:
            USBD_CtlSendData(pdev, &hhid->Protocol, 1);
            break;

        case HID_REQ_SET_IDLE:
            hhid->IdleState = req->wValue >> 8;
            break;

        case HID_REQ_GET_IDLE:
            USBD_CtlSendData(pdev, &hhid->IdleState, 1);
            break;

        case HID_REQ_SET_REPORT:
            if (!hhid->ep0_out_req_ready) {
                hhid->ep0_out_req = *req;
                USBD_CtlPrepareRx(pdev, hhid->ep0_out_buf, req->wLength);
            }
            else {
                USBD_CtlError(pdev, req);
            }
            break;

        case HID_REQ_GET_REPORT:
            // Not (yet) supported, stall endpoint.
            //
            // But according to HID1_11, 7.2:
            //      This request is mandatory and must be supported by all devices.
            //
            USBD_CtlError(pdev, req);
            break;

        default:
            USBD_CtlError(pdev, req);
            return USBD_FAIL;
        }
        break;

    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest) {
        case USB_REQ_GET_DESCRIPTOR:
            if (req->wValue >> 8 == HID_REPORT_DESC) {
                switch(req->wIndex) {
                case 0: USBD_CtlSendData(pdev, KeyboardReportDesc,  MIN(HID_KEYBOARD_REPORT_DESC_SIZE, req->wLength));  break;
                case 1: USBD_CtlSendData(pdev, MouseReportDesc,     MIN(HID_MOUSE_REPORT_DESC_SIZE, req->wLength));     break;
                case 2: USBD_CtlSendData(pdev, ExtraReportDesc,     MIN(HID_EXTRA_REPORT_DESC_SIZE, req->wLength));     break;
                case 3: USBD_CtlSendData(pdev, DebugReportDesc,     MIN(HID_DEBUG_REPORT_DESC_SIZE, req->wLength));     break;
                }
            }
            else if (req->wValue >> 8 == HID_DESCRIPTOR_TYPE) {
                switch(req->wIndex) {
                case 0: USBD_CtlSendData(pdev, USBD_HID_Desc_Keyboard,  MIN(USB_LEN_HID_DESC, req->wLength));   break;
                case 1: USBD_CtlSendData(pdev, USBD_HID_Desc_Mouse,     MIN(USB_LEN_HID_DESC, req->wLength));   break;
                case 2: USBD_CtlSendData(pdev, USBD_HID_Desc_Extra,     MIN(USB_LEN_HID_DESC, req->wLength));   break;
                case 3: USBD_CtlSendData(pdev, USBD_HID_Desc_Debug,     MIN(USB_LEN_HID_DESC, req->wLength));   break;
                }
            }
            break;

        case USB_REQ_GET_INTERFACE:
            USBD_CtlSendData(pdev, &hhid->AltSetting, 1);
            break;

        case USB_REQ_SET_INTERFACE:
            hhid->AltSetting = req->wValue;
            break;
        }
    }
    return USBD_OK;
}


static uint8_t *USBD_HID_GetCfgDesc(uint16_t *length)
{
    *length = sizeof(USBD_HID_CfgDesc);
    return USBD_HID_CfgDesc;
}


static uint8_t USBD_HID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
    USBD_HID_HandleTypeDef  *hhid = pdev->pClassData;

    // Ensure that the FIFO is empty before a new transfer, this condition could
    // be caused by  a new transfer before the end of the previous transfer
    //
    hhid->ep_in_state[epnum & 0x0F] = HID_IDLE;

    return USBD_OK;
}


/**
  * @brief  USBD_CUSTOM_HID_EP0_RxReady
  *         Handles control request data.
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_HID_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
    USBD_HID_HandleTypeDef  *hhid = pdev->pClassData;
    hhid->ep0_out_req_ready = 1;

    return USBD_OK;
}


USBD_ClassTypeDef  USBD_HID = {
    USBD_HID_Init,
    USBD_HID_DeInit,
    USBD_HID_Setup,
    NULL,                   // EP0_TxSent
    USBD_HID_EP0_RxReady,   // EP0_RxReady
    USBD_HID_DataIn,        // DataIn
    NULL,                   // DataOut
    NULL,                   // SOF
    NULL,                   // IsoINIncomplete
    NULL,                   // IsoOUTIncomplete
    NULL,                   // GetHSConfigDescriptor
    USBD_HID_GetCfgDesc,    // GetFSConfigDescriptor
    NULL,                   // GetOtherSpeedConfigDescriptor
    NULL                    // GetDeviceQualifierDescriptor
};


uint8_t USBD_HID_SendReport(USBD_HandleTypeDef *pdev, int ep, const void *report, int len)
{
    USBD_HID_HandleTypeDef *hhid = pdev->pClassData;

    if (pdev->dev_state != USBD_STATE_CONFIGURED)
        return USBD_FAIL;

    if (hhid->ep_in_state[ep & 0x0F] != HID_IDLE)
        return USBD_BUSY;

    hhid->ep_in_state[ep & 0x0F] = HID_BUSY;
    USBD_LL_Transmit(pdev, ep, (void*)report, len);

    return USBD_OK;
}
