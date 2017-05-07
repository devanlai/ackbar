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

#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include <stdint.h>

#define DEFAULT_BAUDRATE 115200

#define CONSOLE_SPLIT_USART 0
#define CONSOLE_USART USART2

#define CONSOLE_TX_BUFFER_SIZE 128
#define CONSOLE_RX_BUFFER_SIZE 128

#define CONSOLE_USART_GPIO_PORT GPIOA
#define CONSOLE_USART_GPIO_TX   GPIO2
#define CONSOLE_USART_GPIO_RX   GPIO3

#define CONSOLE_USART_MODE USART_MODE_TX_RX

#define CONSOLE_USART_CLOCK RCC_USART2

#define CONSOLE_USART_IRQ_NAME  usart2_isr
#define CONSOLE_USART_NVIC_LINE NVIC_USART2_IRQ

/* Word size for usart_recv and usart_send */
typedef uint16_t usart_word_t;

#define LED_OPEN_DRAIN          1
#define LED_GPIO_PORT           GPIOC
#define LED_GPIO_PIN            GPIO13

#define CAN_NVIC_LINE           NVIC_USB_LP_CAN_RX0_IRQ
#define CAN_IRQ_NAME            usb_lp_can_rx0_isr
#define CAN_PORT                CAN1
#define SWJ_AFIO_REMAP          AFIO_MAPR_SWJ_CFG_FULL_SWJ
#define CAN_AFIO_REMAP          AFIO_MAPR_CAN1_REMAP_PORTB
#define CAN_GPIO_PORT           GPIOB
#define CAN_RX_GPIO_PIN         GPIO8
#define CAN_TX_GPIO_PIN         GPIO9

#endif
