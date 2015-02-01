/**
 * Nucular Keyboard - Low-level keyboard driver
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
#include "kb_driver.h"
#include "ustime.h"
#include "util.h"
#include "gamma_tab.inc"
#include "stm32l0xx_hal.h"
#include <stdio.h>


// If you ever change the pinout, print this stuff out and tick it off with a pencil..
//
#define GPIOA_DRV_BITS      (GPIO_PIN_8  | GPIO_PIN_10)
#define GPIOB_DRV_BITS      (GPIO_PIN_0  | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15)
#define GPIOC_DRV_BITS      (GPIO_PIN_6  | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9  | GPIO_PIN_10 | GPIO_PIN_12)
#define GPIOD_DRV_BITS      (GPIO_PIN_2)

#define GPIOC_SENSE_BITS    (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_13 | GPIO_PIN_15)
#define GPIOH_SENSE_BITS    (GPIO_PIN_0 | GPIO_PIN_1)

#define GPIOA_SPECIAL_BITS  (GPIO_PIN_9 | GPIO_PIN_15)
#define GPIOB_SPECIAL_BITS  (GPIO_PIN_9)
#define GPIOC_SPECIAL_BITS  (GPIO_PIN_11 | GPIO_PIN_14)

#define GPIOA_ALL_BITS      (GPIOA_DRV_BITS | GPIOA_SPECIAL_BITS)
#define GPIOB_ALL_BITS      (GPIOB_DRV_BITS | GPIOB_SPECIAL_BITS)
#define GPIOC_ALL_BITS      (GPIOC_DRV_BITS | GPIOC_SPECIAL_BITS | GPIOC_SENSE_BITS)
#define GPIOD_ALL_BITS      (GPIOD_DRV_BITS)
#define GPIOH_ALL_BITS      (GPIOH_SENSE_BITS)


static void discharge(void)
{
    // set all lines high
    //
    GPIOA->BSRR = GPIOA_DRV_BITS;
    GPIOB->BSRR = GPIOB_DRV_BITS;
    GPIOC->BSRR = (GPIOC_DRV_BITS | GPIOC_SENSE_BITS);
    GPIOD->BSRR = GPIOD_DRV_BITS;
    GPIOH->BSRR = GPIOH_SENSE_BITS;

    // use push/pull to boost the rising edge
    //
    GPIOA->OTYPER &= ~GPIOA_DRV_BITS;
    GPIOB->OTYPER &= ~GPIOB_DRV_BITS;
    GPIOC->OTYPER &= ~(GPIOC_DRV_BITS | GPIOC_SENSE_BITS);
    GPIOD->OTYPER &= ~GPIOD_DRV_BITS;
    GPIOH->OTYPER &= ~GPIOH_SENSE_BITS;

    // switch back to open-drain
    //
    GPIOA->OTYPER |= GPIOA_DRV_BITS;
    GPIOB->OTYPER |= GPIOB_DRV_BITS;
    GPIOC->OTYPER |= (GPIOC_DRV_BITS | GPIOC_SENSE_BITS);
    GPIOD->OTYPER |= GPIOD_DRV_BITS;
    GPIOH->OTYPER |= GPIOH_SENSE_BITS;
}


static void set_drv(int d)
{
    // set active line low
    //
    switch (d) {
    case  0: GPIOC->BRR = GPIO_PIN_9;   break;
    case  1: GPIOC->BRR = GPIO_PIN_7;   break;
    case  2: GPIOB->BRR = GPIO_PIN_15;  break;
    case  3: GPIOB->BRR = GPIO_PIN_13;  break;
    case  4: GPIOB->BRR = GPIO_PIN_0;   break;
    case  5: GPIOB->BRR = GPIO_PIN_1;   break;
    case  6: GPIOB->BRR = GPIO_PIN_12;  break;
    case  7: GPIOB->BRR = GPIO_PIN_14;  break;
    case  8: GPIOB->BRR = GPIO_PIN_2;   break;
    case  9: GPIOC->BRR = GPIO_PIN_8;   break;
    case 10: GPIOC->BRR = GPIO_PIN_6;   break;
    case 11: GPIOA->BRR = GPIO_PIN_8;   break;
    case 12: GPIOC->BRR = GPIO_PIN_10;  break;
    case 13: GPIOD->BRR = GPIO_PIN_2;   break;
    case 14: GPIOA->BRR = GPIO_PIN_10;  break;
    case 15: GPIOC->BRR = GPIO_PIN_12;  break;
    }
}


static uint8_t get_sense(void)
{
    uint16_t H = GPIOH->IDR;
    uint16_t C = GPIOC->IDR;

    return  ((H & GPIO_PIN_0 ) ? 0 : (1 << 0) ) |
            ((C & GPIO_PIN_2 ) ? 0 : (1 << 1) ) |
            ((C & GPIO_PIN_0 ) ? 0 : (1 << 2) ) |
            ((H & GPIO_PIN_1 ) ? 0 : (1 << 3) ) |
            ((C & GPIO_PIN_1 ) ? 0 : (1 << 4) ) |
            ((C & GPIO_PIN_15) ? 0 : (1 << 5) ) |
            ((C & GPIO_PIN_3 ) ? 0 : (1 << 6) ) |
            ((C & GPIO_PIN_13) ? 0 : (1 << 7) ) ;
}


int kb_scan_matrix(uint8_t *matrix)
{
    // scan drive lines: ~180 us
    //
    for (int i=0; i<16; i++) {
        discharge();
        set_drv(i);
        delay_us(3);
        matrix[i] = get_sense();
    }

    // ghost detection: ~10us
    //
    for (int i=0; i<15; i++) {
        if ((matrix[i] - 1) & matrix[i]) {
            // if >= 2 keys are pressed .. (bit bashing trick)
            //
            for (int j = i + 1; j<16; j++) {
                // .. check other rows for shared keys
                //
                if (matrix[i] & matrix[j])
                    return 0;
            }
        }
    }

    return 1;
}


int kb_get_fn_key(void)
{
    return !(GPIOC->IDR & GPIO_PIN_14);
}


int kb_get_power_key(void)
{
    return !(GPIOB->IDR & GPIO_PIN_9);
}


int kb_get_id(void)
{
    return
        (GPIOA->IDR & GPIO_PIN_9  ? 1 : 0) +
        (GPIOA->IDR & GPIO_PIN_15 ? 2 : 0) +
        (GPIOC->IDR & GPIO_PIN_11 ? 4 : 0);
}


void kb_set_brightness(uint8_t level)
{
    int pwm = gamma_tab[level];

    TIM2->CCR1  = pwm;  // PA0    LED_MIC
    TIM2->CCR2  = pwm;  // PA1    LED_POWER
    TIM21->CCR1 = pwm;  // PA2    LED_MUTE
    TIM21->CCR2 = pwm;  // PA3    LED_CAPS
    TIM22->CCR1 = pwm;  // PA6    LED_NUM
    TIM22->CCR2 = pwm;  // PA7    LED_SCROLL
    TIM2->CCR3  = pwm;  // PB10   LED_AUX
}


void kb_set_thinklight(uint8_t level)
{
    int pwm = gamma_tab[level];
    TIM2->CCR4  = pwm;  // PB11   LED_LIGHT
}


void kb_set_leds(const struct kb_out_report *report)
{
    if (report->_01_num_lock)
        TIM22->CCER |= TIM_CCER_CC1E;
    else
        TIM22->CCER &= ~TIM_CCER_CC1E;

    if (report->_02_caps_lock)
        TIM21->CCER |= TIM_CCER_CC2E;
    else
        TIM21->CCER &= ~TIM_CCER_CC2E;

    if (report->_03_scroll_lock)
        TIM22->CCER |= TIM_CCER_CC2E;
    else
        TIM22->CCER &= ~TIM_CCER_CC2E;
}


void kb_init(void)
{
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
    RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
    RCC->IOPENR |= RCC_IOPENR_GPIOCEN;
    RCC->IOPENR |= RCC_IOPENR_GPIODEN;
    RCC->IOPENR |= RCC_IOPENR_GPIOHEN;

    // Initialize drive and sense lines
    //
    GPIOA->BSRR = GPIOA_ALL_BITS;
    GPIOB->BSRR = GPIOB_ALL_BITS;
    GPIOC->BSRR = GPIOC_ALL_BITS;
    GPIOD->BSRR = GPIOD_ALL_BITS;
    GPIOH->BSRR = GPIOH_ALL_BITS;

    GPIO_InitTypeDef  gpio_init =  {
        .Mode  = GPIO_MODE_OUTPUT_OD,
        .Pull  = GPIO_PULLUP,
        .Speed = GPIO_SPEED_LOW
    };

    gpio_init.Pin = GPIOA_ALL_BITS;  HAL_GPIO_Init(GPIOA, &gpio_init);
    gpio_init.Pin = GPIOB_ALL_BITS;  HAL_GPIO_Init(GPIOB, &gpio_init);
    gpio_init.Pin = GPIOC_ALL_BITS;  HAL_GPIO_Init(GPIOC, &gpio_init);
    gpio_init.Pin = GPIOD_ALL_BITS;  HAL_GPIO_Init(GPIOD, &gpio_init);
    gpio_init.Pin = GPIOH_ALL_BITS;  HAL_GPIO_Init(GPIOH, &gpio_init);

    // Initialize LED PWM outputs
    //
    // PA0      TIM2_CH1    LED_MIC
    // PA1      TIM2_CH2    LED_POWER
    // PB10     TIM2_CH3    LED_AUX
    // PB11     TIM2_CH4    LED_LIGHT
    // PA2      TIM21_CH1   LED_MUTE
    // PA3      TIM21_CH2   LED_CAPS
    // PA6      TIM22_CH1   LED_NUM
    // PA7      TIM22_CH2   LED_SCROLL
    //
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM21EN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM22EN;

    TIM_HandleTypeDef htim2 = { .Init = {
        .Prescaler = 0,
        .CounterMode = TIM_COUNTERMODE_UP,
        .Period = 3200,   // 10kHz
        .ClockDivision = TIM_CLOCKDIVISION_DIV1
    } };

    TIM_HandleTypeDef htim21 = htim2;
    TIM_HandleTypeDef htim22 = htim2;

    htim2.Instance = TIM2;
    htim21.Instance = TIM21;
    htim22.Instance = TIM22;

    HAL_TIM_PWM_Init(&htim2);
    HAL_TIM_PWM_Init(&htim21);
    HAL_TIM_PWM_Init(&htim22);

    TIM_OC_InitTypeDef sConfigOC = {
        .OCMode = TIM_OCMODE_PWM1,
        .OCPolarity = TIM_OCPOLARITY_LOW,
        .OCFastMode = TIM_OCFAST_DISABLE
    };

    HAL_TIM_PWM_ConfigChannel(&htim2,  &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim2,  &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&htim2,  &sConfigOC, TIM_CHANNEL_3);
    HAL_TIM_PWM_ConfigChannel(&htim2,  &sConfigOC, TIM_CHANNEL_4);
    HAL_TIM_PWM_ConfigChannel(&htim21, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim21, &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&htim22, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim22, &sConfigOC, TIM_CHANNEL_2);

    TIM2->CCER  |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;
    TIM21->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;
    TIM22->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;

    TIM2->CR1   |= TIM_CR1_CEN;
    TIM21->CR1  |= TIM_CR1_CEN;
    TIM22->CR1  |= TIM_CR1_CEN;

    gpio_init.Mode = GPIO_MODE_AF_OD;

    gpio_init.Pin = GPIO_PIN_0 | GPIO_PIN_1;
    gpio_init.Alternate = GPIO_AF2_TIM2;
    HAL_GPIO_Init(GPIOA, &gpio_init);

    gpio_init.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    gpio_init.Alternate = GPIO_AF2_TIM2;
    HAL_GPIO_Init(GPIOB, &gpio_init);

    gpio_init.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    gpio_init.Alternate = GPIO_AF0_TIM21;
    HAL_GPIO_Init(GPIOA, &gpio_init);

    gpio_init.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio_init.Alternate = GPIO_AF5_TIM22;
    HAL_GPIO_Init(GPIOA, &gpio_init);

    kb_set_brightness(128);
}
