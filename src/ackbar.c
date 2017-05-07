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

#include <stdio.h>
#include <string.h>

#include "config.h"
#include "target.h"

#include "tick.h"
#include "retarget.h"
#include "console.h"

#include "CAN/can.h"

static inline uint32_t millis(void) {
    return get_ticks();
}

static inline void wait_ms(uint32_t duration_ms) {
    uint32_t now = millis();
    uint32_t end = now + duration_ms;
    if (end < now) {
        end = 0xFFFFFFFFU - end;
        while (millis() >= now) {
            __asm__("NOP");
        }
    }

    while (millis() < end) {
        __asm__("NOP");
    }
}

int main(void) {
    clock_setup();
    tick_setup(1000);
    gpio_setup();
    led_set(0);

    console_setup(DEFAULT_BAUDRATE);
    retarget(STDOUT_FILENO, CONSOLE_USART);
    retarget(STDERR_FILENO, CONSOLE_USART);

    tick_start();

    can_setup(500000, MODE_NORMAL);

    while (1) {
        unsigned msg_count = 0;
        CAN_Message msg;
        while (can_read_buffer(&msg)) {
            msg_count++;
        }

        if (msg_count > 0) {
            led_set(1);
            wait_ms(msg_count*10);
            led_set(0);
        } else {
            led_set(0);
        }
    }

    return 0;
}
