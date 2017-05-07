/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef MBED_CAN_HELPER_H
#define MBED_CAN_HELPER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum CANFormat {
    CANStandard = 0,
    CANExtended = 1,
    CANAny = 2
};
typedef enum CANFormat CANFormat;

enum CANType {
    CANData   = 0,
    CANRemote = 1
};
typedef enum CANType CANType;

struct CAN_Message {
    uint32_t  id;                 // 29 bit identifier
    uint8_t   data[8];            // Data field
    uint8_t   len;                // Length of data field in bytes
    CANFormat format;             // 0 - STANDARD, 1- EXTENDED IDENTIFIER
    CANType   type;               // 0 - DATA FRAME, 1 - REMOTE FRAME
};
typedef struct CAN_Message CAN_Message;

typedef enum {
    MODE_RESET,
    MODE_NORMAL,
    MODE_SILENT,
    MODE_TEST_LOCAL,
    MODE_TEST_SILENT
} CanMode;

#ifdef __cplusplus
};
#endif

#endif // MBED_CAN_HELPER_H
