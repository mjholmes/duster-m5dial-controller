// M5Dial v1.0 pin allocation (SPEC 3.2).
//
// Sources, cross-checked 2026-09-27:
//  [1] M5Unified 0.2.23, src/M5Unified.inl pin tables for board_M5Dial
//      (Ex I2C / Port A SCL G15 SDA G13, Port B G1/G2, in-I2C SCL G12 SDA G11,
//      hold G46, buttons G42 and G0, speaker G3).
//  [2] M5Stack M5Dial v1.0 schematic, Sch_M5Dial.pdf, and the pin tables on
//      https://docs.m5stack.com/en/core/M5Dial (LCD, touch, encoder, Grove ports).
//  [3] M5Stack Unit CAN docs, https://docs.m5stack.com/en/unit/can
//      (Grove: yellow = CAN_TX, white = CAN_RX, MCU-side labels).
//
// Port A (G13/G15) belongs to TWAI and Port B (G1/G2) to the RA60 UART.
// Nothing else may configure them.
#pragma once

#include "driver/gpio.h"

// Port A (Grove): M5Stack CAN Unit. Dial Port A is yellow G13, white G15 [2],
// CAN Unit yellow = CAN_TX, white = CAN_RX [3]. Confirm on the bench.
#define PIN_CAN_TX          GPIO_NUM_13
#define PIN_CAN_RX          GPIO_NUM_15

// Port B (Grove): RA60 UART via auxbeam-light-controller. Dial Port B is
// yellow G2, white G1 [2]. Proposed per SPEC 3.2; confirm against the cable.
#define PIN_AUX_UART_TX     GPIO_NUM_2
#define PIN_AUX_UART_RX     GPIO_NUM_1

// Encoder. Wiring taken from the tested POC (M5Dial-LVGL Encoder::setup()
// defaults: pin_a = G40, pin_b = G41; positive = clockwise on hardware).
// Note: [2] labels G40 as encoder "B" and G41 as "A". The names below follow
// the POC decoder's roles, not the silkscreen; behaviour is what was tested.
#define PIN_ENC_A           GPIO_NUM_40
#define PIN_ENC_B           GPIO_NUM_41

// Front button (encoder push). Read by M5Unified as BtnA [1][2].
#define PIN_BUTTON          GPIO_NUM_42

// Power hold: drive high early in boot to stay on when on battery [1][2].
#define PIN_HOLD            GPIO_NUM_46

// Owned by M5Unified / M5GFX, listed for reference only [1][2]:
//   LCD GC9A01: MOSI G5, SCK G6, CS G7, RST G8, RS/DC G4, BL G9
//   Internal I2C (touch FT3267 0x38, RTC, RFID): SDA G11, SCL G12
//   Touch INT G14, RFID INT G10 (unused), buzzer G3 (unused), USB G19/G20
