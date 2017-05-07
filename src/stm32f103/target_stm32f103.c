/*
 * Copyright (c) 2017, Devan Lai
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice
 * appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "target.h"
#include "config.h"

/* Set STM32 to 72 MHz. */
void clock_setup(void) {
    rcc_clock_setup_in_hse_8mhz_out_72mhz();
}

void gpio_setup(void) {
    /* Enable GPIOA, GPIOB, and GPIOC clocks. */
    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);

    /* Setup LED */
    const uint8_t mode = GPIO_MODE_OUTPUT_10_MHZ;
    const uint8_t conf = (LED_OPEN_DRAIN ? GPIO_CNF_OUTPUT_OPENDRAIN
                                         : GPIO_CNF_OUTPUT_PUSHPULL);
    gpio_set_mode(LED_GPIO_PORT, mode, conf, LED_GPIO_PIN);
}

void target_console_init(void){
    /* Enable UART clocks */
    rcc_periph_clock_enable(CONSOLE_USART_CLOCK);

    /* Setup GPIO pins */
    gpio_set_mode(CONSOLE_USART_GPIO_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, CONSOLE_USART_GPIO_TX);
    gpio_set_mode(CONSOLE_USART_GPIO_PORT, GPIO_MODE_INPUT,
                  GPIO_CNF_INPUT_FLOAT, CONSOLE_USART_GPIO_RX);
}

void target_can_init(void) {
    /* Enable CAN clock */
    rcc_periph_clock_enable(RCC_CAN);

#ifdef CAN_AFIO_REMAP
    rcc_periph_clock_enable(RCC_AFIO);
    gpio_primary_remap(SWJ_AFIO_REMAP, CAN_AFIO_REMAP);
#endif
    
    /* Setup CAN pins */
    gpio_set_mode(CAN_GPIO_PORT, GPIO_MODE_INPUT,
                  GPIO_CNF_INPUT_PULL_UPDOWN, CAN_RX_GPIO_PIN);
    gpio_set(CAN_GPIO_PORT, CAN_RX_GPIO_PIN);
    gpio_set_mode(CAN_GPIO_PORT, GPIO_MODE_OUTPUT_50_MHZ,
                  GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, CAN_TX_GPIO_PIN);
}

bool led_get(void) {
    if (LED_OPEN_DRAIN) {
        return gpio_get(LED_GPIO_PORT, LED_GPIO_PIN) == 0;
    } else {
        return gpio_get(LED_GPIO_PORT, LED_GPIO_PIN) != 0;
    }
}

void led_set(bool on) {
    if (on ^ LED_OPEN_DRAIN) {
        gpio_set(LED_GPIO_PORT, LED_GPIO_PIN);
    } else {
        gpio_clear(LED_GPIO_PORT, LED_GPIO_PIN);
    }
}
