/**
 * Nucular Keyboard - ThinkPad USB Adapter
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
#include "hid_debug.h"
#include "kb_driver.h"
#include "keyboard.h"
#include "ps2_host.h"
#include "trackpoint.h"
#include "ustime.h"
#include "ansi.h"
#include "usbd_desc.h"
#include "usbd_hid.h"
#include "stm32l0xx.h"


static IWDG_HandleTypeDef hiwdg;
USBD_HandleTypeDef hUsbDeviceFS;
extern PCD_HandleTypeDef hpcd_USB_FS;


void USB_IRQHandler(void)
{
    HAL_NVIC_ClearPendingIRQ(USB_IRQn);
    HAL_PCD_IRQHandler(&hpcd_USB_FS);
}


void SysTick_Handler(void)
{
    HAL_IncTick();
    HAL_SYSTICK_IRQHandler();
}


void SystemClock_Config(void)
{
    RCC_CRSInitTypeDef RCC_CRSInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInit;
    RCC_OscInitTypeDef RCC_OscInitStruct;

    __PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSI48;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = 16;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON;
    RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
    RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1);

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_USB;
    PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

    __SYSCFG_CLK_ENABLE();

    __CRS_CLK_ENABLE();

    RCC_CRSInitStruct.Prescaler = RCC_CRS_SYNC_DIV1;
    RCC_CRSInitStruct.Source = RCC_CRS_SYNC_SOURCE_USB;
    RCC_CRSInitStruct.Polarity = RCC_CRS_SYNC_POLARITY_RISING;
    RCC_CRSInitStruct.ReloadValue = __HAL_RCC_CRS_CALCULATE_RELOADVALUE(48000000, 1000);
    RCC_CRSInitStruct.ErrorLimitValue = 34;
    RCC_CRSInitStruct.HSI48CalibrationValue = 32;
    HAL_RCCEx_CRSConfig(&RCC_CRSInitStruct);
}


static void MX_IWDG_Init(void)
{
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_4;
    hiwdg.Init.Reload = 50;
    HAL_IWDG_Init(&hiwdg);
}


void MX_GPIO_Init(void)
{
    /* GPIO Ports Clock Enable */
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
}


void MX_USB_DEVICE_Init(void)
{
    USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);
    USBD_RegisterClass(&hUsbDeviceFS, &USBD_HID);
    USBD_Start(&hUsbDeviceFS);
}


void assert_failed(uint8_t* file, uint32_t line)
{
    printf("assert failed: %s:%lu\n", file, line);
}


void handle_out_requests(void)
{
    USBD_HID_HandleTypeDef *hhid = hUsbDeviceFS.pClassData;

    if (!hhid->ep0_out_req_ready)
        return;

    uint8_t report_type = hhid->ep0_out_req.wValue >> 8;
    uint8_t report_id = hhid->ep0_out_req.wValue & 0xff;

    switch (hhid->ep0_out_req.wIndex) {
    case 0:
        if (report_type == 2 && report_id == 0)
            kb_set_led_report((struct kb_out_report *)hhid->ep0_out_buf);
        break;

    case 2:
        if (report_type == 3 && report_id == 0x80)
            enter_bootloader();
        break;
    }

    hhid->ep0_out_req_ready = 0;
}


int main(void)
{
    SCB->VTOR = 0x8004000;  // Relocate IRQ table

    HAL_Init();
    SystemClock_Config();
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);

    MX_GPIO_Init();
    // MX_IWDG_Init();
    MX_USB_DEVICE_Init();

    // PA4  Debug output
    // PA5  Debug output
    //
    HAL_GPIO_Init(GPIOA, &(GPIO_InitTypeDef) {
        .Pin   = GPIO_PIN_4 | GPIO_PIN_5,
        .Mode  = GPIO_MODE_OUTPUT_PP,
        .Pull  = GPIO_PULLUP,
        .Speed = GPIO_SPEED_LOW
    } );

    init_us_timer();

    printf("Initializing keyboard interface..\n");
    kb_init();

    printf("Initializing TrackPoint interface..\n");
    tp_init();

    static struct tp_mouse_report     mr_old;
    static struct kb_in_report        kr_old;
    static struct kb_sysctrl_report   sr_old;
    static struct kb_consumer_report  cr_old;

    for (;;) {
        kb_update();
        tp_update();

        // Send keyboard reports on state change only
        //
        if (memcmp(&kb_in_report, &kr_old, sizeof(kb_in_report))) {
            if (USBD_HID_SendReport(&hUsbDeviceFS, HID_KEYBOARD_EPIN_ADDR, &kb_in_report, sizeof(kb_in_report)) == USBD_OK)
                kr_old = kb_in_report;
        }

        if (memcmp(&kb_sysctrl_report, &sr_old, sizeof(kb_sysctrl_report))) {
            if (USBD_HID_SendReport(&hUsbDeviceFS, HID_EXTRA_EPIN_ADDR, &kb_sysctrl_report, sizeof(kb_sysctrl_report)) == USBD_OK)
                sr_old = kb_sysctrl_report;
        }

        if (memcmp(&kb_consumer_report, &cr_old, sizeof(kb_consumer_report))) {
            if (USBD_HID_SendReport(&hUsbDeviceFS, HID_EXTRA_EPIN_ADDR, &kb_consumer_report, sizeof(kb_consumer_report)) == USBD_OK)
                cr_old = kb_consumer_report;
        }

        // Send mouse reports while moving and on button change
        //
        if (tp_mouse_report.dx      || tp_mouse_report.dy   ||
            tp_mouse_report.dwheel  || tp_mouse_report.dpan ||
            tp_mouse_report.buttons != mr_old.buttons)
        {
            if (USBD_HID_SendReport(&hUsbDeviceFS, HID_MOUSE_EPIN_ADDR, &tp_mouse_report, sizeof(tp_mouse_report)) == USBD_OK) {
                tp_clear_mouse_report();
                mr_old = tp_mouse_report;
            }
        }

        hid_debug_flush();

        handle_out_requests();
    }
}
