/**
* Digital Voice Modem - DSP Firmware
* GPLv2 Open Source. Use is subject to license terms.
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* @package DVM / DSP Firmware
*
*/
//
// Based on code from Mark Carter (https://mcturra2000.wordpress.com/2022/03/10/sending-data-over-i2c-using-bit-banging/)
// And also https://github.com/PascalPolygon/stm32_bitbang_i2c/
// Licensed under the GPLv2 License (https://opensource.org/licenses/GPL-2.0)
//
/*
*   Copyright (C) 2023 by Patrick McDonnell, W3AXL
*
*   This library is free software; you can redistribute it and/or
*   modify it under the terms of the GNU Library General Public
*   License as published by the Free Software Foundation; either
*   version 2 of the License, or (at your option) any later version.
*
*   This library is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   Library General Public License for more details.
*
*   You should have received a copy of the GNU Library General Public
*   License along with this library; if not, write to the
*   Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
*   Boston, MA  02110-1301, USA.
*
*/

#include "sw_i2c.h"
#include "DWT_delay.h"

#if (!defined(__SAM3X8E__) && !defined(__MK20DX256__) && !defined(__MK64FX512__) && !defined(__MK66FX1M0__)) && !defined(ARDUINO_SAM_DUE)

void scl(bool on)
{
    GPIO_WriteBit(PORT_SCL, PIN_SCL, on ? Bit_SET : Bit_RESET);
}

void sda(bool on)
{
    GPIO_WriteBit(PORT_SDA, PIN_SDA, on ? Bit_SET : Bit_RESET);
}

void i2c_delay() {
    DWT_Delay_us(5);
}

void gen_start() {
    scl(1);
    sda(1);
    i2c_delay();
    sda(0);
    i2c_delay();
    scl(0);
    i2c_delay();
}

void gen_stop() {
    sda(0);
    i2c_delay();
    scl(1);
    i2c_delay();
    sda(1);
    i2c_delay();
}

uint8_t read_sda() {
    if (GPIO_ReadInputDataBit(PORT_SDA, PIN_SDA) == Bit_SET) return 1; else return 0;
}

uint8_t read_bit() {
    uint8_t b;
    sda(1);
    i2c_delay();
    scl(1);
    i2c_delay();
    b = read_sda();
    scl(0);
    return b;
}

void write_bit(bool bit) {
    sda(bit);
    i2c_delay();
    scl(1);
    i2c_delay();
    scl(0);
}

bool write_byte(uint8_t B, bool start, bool stop) {
    uint8_t ack = 0;
    // Generate start sequence if needed
    if (start) gen_start();
    // Send the bits of the byte
    for (uint8_t i = 0; i < 8; i++) {
        write_bit(B & 0x80);
        B <<= 1;
    }
    // Check for ACK
    ack = read_bit();
    // Send stop pattern if needed
    if (stop) gen_stop();
    // Return ack (0 = ack, 1 = nack)
    return !ack;
}

uint8_t read_byte(bool ack, bool stop) {
    uint8_t B = 0;

    for (uint8_t i = 0; i < 8; i++) {
        B <<= 1;
        B |= read_bit();
    }

    if (ack) write_bit(0); else write_bit(1);

    if (stop) gen_stop();

    return B;
}

bool send_byte(uint8_t address, uint8_t data) {
    if (write_byte(address, true, false)) {
        if (write_byte(data, false, true)) {
            return true;
        }
    }
    // stop if we didn't get an ack
    gen_stop();
    return false;
}

uint8_t recv_byte(uint8_t address) {
    if (write_byte((address << 1) | 0x01, true, false)) {
        return read_byte(false, true);
    }
    return 0;
}

void sw_i2c_init() {
    DWT_Delay_Init();
    scl(1);
    sda(1);
}

bool sw_i2c_write(uint8_t address, uint8_t *data, uint8_t size) {
    if (write_byte(address, true, false)) {
        for (uint8_t i = 0; i < size; i++) {
            if (i == size - 1) {
                if (write_byte(data[i], false, true)) return true;
            } else {
                if (!write_byte(data[i], false, false)) break;
            }
        }
    }
    gen_stop();
    return false;
}

#endif