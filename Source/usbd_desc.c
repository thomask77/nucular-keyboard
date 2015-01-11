#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_conf.h"

#define USBD_VID                    0x1D50
#define USBD_PID                    0xFF06
#define USBD_LANGID_STRING          1033
#define USBD_MFC_STRING             "t-kindler.de"
#define USBD_PRODUCT_STRING         "Nucular Keyboard"

#define USBD_MAX_STR_DESC_SIZ       256
#define DEVICE_ID1                  0x1FF80050
#define DEVICE_ID2                  0x1FF80054
#define DEVICE_ID3                  0x1FF80064

#define USB_SIZ_STRING_SERIAL       0x1A

static uint8_t USBD_StringSerial[USB_SIZ_STRING_SERIAL] = {
    USB_SIZ_STRING_SERIAL,
    USB_DESC_TYPE_STRING
};


/* USB Standard Device Descriptor */
static uint8_t USBD_FS_DeviceDesc[USB_LEN_DEV_DESC] = {
    USB_LEN_DEV_DESC,           // bLength
    USB_DESC_TYPE_DEVICE,       // bDescriptorType
    0x00, 0x02,                 // bcdUSB
    0x00,                       // bDeviceClass
    0x00,                       // bDeviceSubClass
    0x00,                       // bDeviceProtocol
    USB_MAX_EP0_SIZE,           // bMaxPacketSize
    LOBYTE(USBD_VID), HIBYTE(USBD_VID),   // idVendor
    LOBYTE(USBD_PID), HIBYTE(USBD_PID),   // idProduct
    0x00, 0x02,                 // bcdDevice rel. 2.00
    USBD_IDX_MFC_STR,           // iManufacturer
    USBD_IDX_PRODUCT_STR,       // iProduct
    USBD_IDX_SERIAL_STR,        // iSerialNumber
    USBD_MAX_NUM_CONFIGURATION  // bNumConfigurations
};


/* USB Standard Device Descriptor */
static uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] = {
    USB_LEN_LANGID_STR_DESC,
    USB_DESC_TYPE_STRING,
    LOBYTE(USBD_LANGID_STRING),
    HIBYTE(USBD_LANGID_STRING)
};


static uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ];


static uint8_t *USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    *length = sizeof(USBD_FS_DeviceDesc);
    return USBD_FS_DeviceDesc;
}


static uint8_t *USBD_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    *length = sizeof(USBD_LangIDDesc);
    return USBD_LangIDDesc;
}


static uint8_t *USBD_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    USBD_GetString(USBD_PRODUCT_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}


static uint8_t *USBD_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    USBD_GetString(USBD_MFC_STRING, USBD_StrDesc, length);
    return USBD_StrDesc;
}


/**
  * @brief  Convert Hex 32Bits value into char
  * @param  value: value to convert
  * @param  pbuf: pointer to the buffer
  * @param  len: buffer length
  * @retval None
  */
static void IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len)
{
    uint8_t idx = 0;

    for (idx = 0; idx < len; idx++) {
        if (((value >> 28)) < 0xA) {
            pbuf[2 * idx] = (value >> 28) + '0';
        }
        else {
            pbuf[2 * idx] = (value >> 28) + 'A' - 10;
        }

        value = value << 4;

        pbuf[2 * idx + 1] = 0;
    }
}


/**
  * @brief  Create the serial number string descriptor
  * @param  None
  * @retval None
  */
static void Get_SerialNum(void)
{
    uint32_t  deviceserial0 = *(uint32_t*)DEVICE_ID1;
    uint32_t  deviceserial1 = *(uint32_t*)DEVICE_ID2;
    uint32_t  deviceserial2 = *(uint32_t*)DEVICE_ID3;

    deviceserial0 += deviceserial2;

    IntToUnicode(deviceserial0, &USBD_StringSerial[2], 8);
    IntToUnicode(deviceserial1, &USBD_StringSerial[18], 4);
}


static uint8_t *USBD_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    *length = USB_SIZ_STRING_SERIAL;

    /* Update the serial number string descriptor with the data from the unique ID*/
    Get_SerialNum();

    return USBD_StringSerial;
}


USBD_DescriptorsTypeDef FS_Desc = {
    USBD_FS_DeviceDescriptor,
    USBD_FS_LangIDStrDescriptor,
    USBD_FS_ManufacturerStrDescriptor,
    USBD_FS_ProductStrDescriptor,
    USBD_FS_SerialStrDescriptor,
    NULL,
    NULL
};

