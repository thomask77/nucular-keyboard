#pragma once

#include <stdint.h>
#include "usbd_ioreq.h"
#include "keyboard.h"

#define HID_KEYBOARD_EPIN_ADDR          0x81
#define HID_KEYBOARD_EPIN_SIZE          8

#define HID_MOUSE_EPIN_ADDR             0x82
#define HID_MOUSE_EPIN_SIZE             5

#define HID_EXTRA_EPIN_ADDR             0x83
#define HID_EXTRA_EPIN_SIZE             4

#define HID_DEBUG_EPIN_ADDR             0x84
#define HID_DEBUG_EPIN_SIZE             64

#define USB_HID_CONFIG_DESC_SIZ         109
#define HID_KEYBOARD_REPORT_DESC_SIZE   sizeof(KeyboardReportDesc)
#define HID_MOUSE_REPORT_DESC_SIZE      sizeof(MouseReportDesc)
#define HID_EXTRA_REPORT_DESC_SIZE      sizeof(ExtraReportDesc)
#define HID_DEBUG_REPORT_DESC_SIZE      sizeof(DebugReportDesc)

#define HID_MOUSE_POLLING_INTERVAL      10
#define HID_KEYBOARD_POLLING_INTERVAL   10
#define HID_EXTRA_POLLING_INTERVAL      10
#define HID_DEBUG_POLLING_INTERVAL      1

#define USB_LEN_HID_DESC                9

#define HID_DESCRIPTOR_TYPE             0x21
#define HID_REPORT_DESC                 0x22

#define HID_REQ_SET_PROTOCOL            0x0B
#define HID_REQ_GET_PROTOCOL            0x03

#define HID_REQ_SET_IDLE                0x0A
#define HID_REQ_GET_IDLE                0x02

#define HID_REQ_SET_REPORT              0x09
#define HID_REQ_GET_REPORT              0x01


typedef enum {
    HID_IDLE,
    HID_BUSY
} HID_StateTypeDef;


typedef struct {
    HID_StateTypeDef ep_in_state[8];

    uint8_t ep0_out_buf[USB_MAX_EP0_SIZE];
    struct  usb_setup_req ep0_out_req;
    uint8_t ep0_out_req_ready;

    uint8_t Protocol;
    uint8_t IdleState;
    uint8_t AltSetting;

} USBD_HID_HandleTypeDef;


extern USBD_ClassTypeDef USBD_HID;

void enter_bootloader(void);

uint8_t USBD_HID_SendReport(USBD_HandleTypeDef *pdev, int ep, const void *report, int len);
