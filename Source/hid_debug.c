#include "hid_debug.h"
#include "ringbuf.h"
#include "usbd_hid.h"
#include "stm32l0xx.h"

#define HID_DEBUG_TIMEOUT   (HID_DEBUG_POLLING_INTERVAL * 2)


struct hid_debug_report {
    // 64 does not seem to work with hid_listen
    //
    uint8_t data[63];
};


extern USBD_HandleTypeDef   hUsbDeviceFS;


static struct ringbuf  tx_buf = RINGBUF(256);


#undef putchar

int putchar(int c)
{
    uint32_t t0 = HAL_GetTick();

    for (;;) {
        // tx_buf needs locking if there are multiple writers
        //
        __disable_irq();
        int ret = rb_putchar(&tx_buf, c);
        __enable_irq();

        // don't block or flush in ISRs
        //
        if (__get_IPSR() & 0x3F)
            return ret;

        // flush on newline
        //
        if (c == '\n')
            hid_debug_flush();

        // return if successful ..
        //
        if (ret >= 0)
            return ret;

        // .. or flush and retry until timed out
        //
        if (HAL_GetTick() - t0 > HID_DEBUG_TIMEOUT)
            return -1;

        hid_debug_flush();
    }
}


void hid_debug_flush(void)
{
    struct hid_debug_report report;
    void *ptr1, *ptr2;
    unsigned len1, len2;

    int len = rb_get_pointers( &tx_buf,
        RB_READ, sizeof(report.data),
        &ptr1, &len1, &ptr2, &len2
    );

    if (!len)
        return;

    memcpy(report.data, ptr1, len1);
    memcpy(report.data + len1, ptr2, len2);
    memset(report.data + len,  0, sizeof(report.data) - len);

    if (USBD_HID_SendReport(&hUsbDeviceFS, HID_DEBUG_EPIN_ADDR, &report, sizeof(report)) == USBD_OK)
        rb_commit(&tx_buf, RB_READ, len);
}
