/*
 * Copyright (c) 2015, Devan Lai
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

#include <libopencm3/stm32/can.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>

#include <string.h>

#include "config.h"
#include "target.h"

#include "can.h"

#define IS_POW_OF_TWO(X) (((X) & ((X)-1)) == 0)
_Static_assert(IS_POW_OF_TWO(CAN_RX_BUFFER_SIZE),
               "Unmasked circular buffer size must be a power of two");
_Static_assert(CAN_RX_BUFFER_SIZE <= UINT8_MAX/2,
               "Buffer size too big for unmasked circular buffer");

static volatile CAN_Message can_rx_buffer[CAN_RX_BUFFER_SIZE];
static volatile uint8_t can_rx_head = 0;
static volatile uint8_t can_rx_tail = 0;

bool can_rx_buffer_empty(void) {
    return can_rx_head == can_rx_tail;
}

bool can_rx_buffer_full(void) {
    return (uint8_t)(can_rx_tail - can_rx_head) == CAN_RX_BUFFER_SIZE;
}

CAN_Message* can_rx_buffer_peek(void) {
    if (!can_rx_buffer_empty()) {
        return (CAN_Message*)(&can_rx_buffer[(can_rx_head % CAN_RX_BUFFER_SIZE)]);
    } else {
        return NULL;
    }
}

void can_rx_buffer_pop(void) {
    can_rx_head++;

    // Re-enable the ISR since we made space
    nvic_enable_irq(CAN_NVIC_LINE);
}

static CAN_Message* can_rx_buffer_tail(void) {
    return (CAN_Message*)&can_rx_buffer[(can_rx_tail % CAN_RX_BUFFER_SIZE)];
}

static void can_rx_buffer_extend(void) {
    can_rx_tail++;
}

void can_rx_buffer_put(const CAN_Message* msg) {
    memcpy((void*)&can_rx_buffer[(can_rx_tail % CAN_RX_BUFFER_SIZE)], (const void*)msg, sizeof(CAN_Message));
    can_rx_tail++;
}

void can_rx_buffer_get(CAN_Message* msg) {
    memcpy((void*)msg, (const void*)&can_rx_buffer[(can_rx_head % CAN_RX_BUFFER_SIZE)], sizeof(CAN_Message));
    can_rx_head++;

    // Re-enable the ISR since we made space
    nvic_enable_irq(CAN_NVIC_LINE);
}

bool can_reconfigure(uint32_t baudrate, CanMode mode) {
    nvic_disable_irq(CAN_NVIC_LINE);
    can_disable_irq(CAN_PORT, CAN_IER_FMPIE0);
    can_reset(CAN_PORT);

    if (mode == MODE_RESET) {
        // Just stop after resetting the CAN controller.
        return true;
    }

    /* Set appropriate bit timing */
    uint32_t sjw = CAN_BTR_SJW_1TQ;
    uint32_t ts1;
    uint32_t ts2;
    uint32_t brp;

    /* Timing from http://www.bittiming.can-wiki.info/
       using a sample point of 75% for mbed compatibility */
    if (rcc_apb1_frequency == 48000000UL) {
        if (baudrate == 1000000) {
            brp = 3;
            ts1 = CAN_BTR_TS1_11TQ;
            ts2 = CAN_BTR_TS2_4TQ;
        } else if (baudrate == 500000) {
            brp = 6;
            ts1 = CAN_BTR_TS1_11TQ;
            ts2 = CAN_BTR_TS2_4TQ;
        } else if (baudrate == 250000) {
            brp = 12;
            ts1 = CAN_BTR_TS1_11TQ;
            ts2 = CAN_BTR_TS2_4TQ;
        } else if (baudrate == 125000) {
            brp = 24;
            ts1 = CAN_BTR_TS1_11TQ;
            ts2 = CAN_BTR_TS2_4TQ;
        } else if (baudrate == 100000) {
            brp = 30;
            ts1 = CAN_BTR_TS1_11TQ;
            ts2 = CAN_BTR_TS2_4TQ;
        } else {
            return false;
        }
    } else if (rcc_apb1_frequency == 36000000UL) {
        if (baudrate == 1000000) {
            brp = 3;
            ts1 = CAN_BTR_TS1_8TQ;
            ts2 = CAN_BTR_TS2_3TQ;
        } else if (baudrate == 500000) {
            brp = 6;
            ts1 = CAN_BTR_TS1_8TQ;
            ts2 = CAN_BTR_TS2_3TQ;
        } else if (baudrate == 250000) {
            brp = 9;
            ts1 = CAN_BTR_TS1_11TQ;
            ts2 = CAN_BTR_TS2_4TQ;
        } else if (baudrate == 125000) {
            brp = 18;
            ts1 = CAN_BTR_TS1_11TQ;
            ts2 = CAN_BTR_TS2_4TQ;
        } else if (baudrate == 100000) {
            brp = 18;
            ts1 = CAN_BTR_TS1_14TQ;
            ts2 = CAN_BTR_TS2_5TQ;
        } else {
            return false;
        }
    } else {
        return false;
    }

    bool loopback = (mode == MODE_TEST_LOCAL || mode == MODE_TEST_SILENT);
    bool silent = (mode == MODE_SILENT || mode == MODE_TEST_SILENT);

    bool TTCM = false; /* TTCM: Time triggered comm mode? */
    bool ABOM = true;  /* ABOM: Automatic bus-off management? */
    bool AWUM = false; /* AWUM: Automatic wakeup mode? */
    bool NART = false; /* NART: No automatic retransmission? */
    bool RFLM = true;  /* RFLM: Receive FIFO locked mode? */
    bool TXFP = false; /* TXFP: Transmit FIFO priority? */

    /* CAN cell init. */
    if (can_init(CAN_PORT, TTCM, ABOM, AWUM, NART, RFLM, TXFP,
                 sjw, ts1, ts2,
                 brp,  /* BRP+1: Baud rate prescaler */
                 loopback,
                 silent) != 0) {
        return false;
    } else {
        can_filter_id_mask_32bit_init(CAN_PORT,
                                      0,     /* Filter ID */
                                      0,     /* CAN ID */
                                      0,     /* CAN ID mask */
                                      0,     /* FIFO assignment (here: FIFO0) */
                                      true); /* Enable the filter. */
    }

    can_enable_irq(CAN_PORT, CAN_IER_FMPIE0);
    nvic_enable_irq(CAN_NVIC_LINE);
    return true;
}

bool can_setup(uint32_t baudrate, CanMode mode) {
    target_can_init();

    return can_reconfigure(baudrate, mode);
}

static uint8_t can_fifo_depth(void) {
    uint8_t fifo_depth = (CAN_RF0R(CAN_PORT) & CAN_RF0R_FMP0_MASK);
    // Account for one fifo entry possibly going away
    if (CAN_RF0R(CAN_PORT) & CAN_RF0R_RFOM0) {
        fifo_depth = fifo_depth > 0 ? (fifo_depth - 1) : 0;
    }

    return fifo_depth;
}

bool can_read(CAN_Message* msg) {
    bool success = false;

    if (can_fifo_depth() > 0) {
        // Wait for the previous message to be released
        while (CAN_RF0R(CAN_PORT) & CAN_RF0R_RFOM0);

        uint32_t fmi;
        bool ext, rtr;
        can_receive(CAN_PORT, 0, true, &msg->id, &ext, &rtr, &fmi, &msg->len, msg->data);
        msg->format = ext ? CANExtended : CANStandard;
        msg->type = rtr ? CANRemote : CANData;
        success = true;
    }

    return success;
}

bool can_read_buffer(CAN_Message* msg) {
    bool success = false;
    if (!can_rx_buffer_empty()) {
        can_rx_buffer_get(msg);
        success = true;
    }

    return success;
}

bool can_write(CAN_Message* msg) {
    bool ext = msg->format == CANExtended;
    bool rtr = msg->type == CANRemote;
    return (can_transmit(CAN_PORT, msg->id, ext, rtr, msg->len, msg->data) != -1);
}

void CAN_IRQ_NAME(void) {
    uint8_t messages_queued = 0;
    uint8_t fifo_depth = can_fifo_depth();
    while (!can_rx_buffer_full() && messages_queued < fifo_depth) {
        CAN_Message* msg = can_rx_buffer_tail();
        if (can_read(msg)) {
            can_rx_buffer_extend();
            messages_queued++;
        } else {
            break;
        }
    }

    // If the software buffer is full, disable the ISR so that
    // the main loop can drain the buffer over USB.
    if (messages_queued == 0) {
        nvic_disable_irq(CAN_NVIC_LINE);
    }
}
