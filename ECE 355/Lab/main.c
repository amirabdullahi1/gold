//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------
// School: University of Victoria, Canada.
// Course: ECE 355 "Microprocessor-Based Systems".
// This is template code for Part 2 of Introductory Lab.
//
// See "system/include/cmsis/stm32f051x8.h" for register/bit definitions.
// See "system/src/cmsis/vectors_stm32f051x8.c" for handler declarations.
// ----------------------------------------------------------------------------

#include <stdio.h>
#include "diag/Trace.h"
#include <string.h>

#include "cmsis/cmsis_device.h"

// ----------------------------------------------------------------------------
//
// STM32F0 empty sample (trace via $(trace)).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the $(trace) output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"


/* Definitions of registers and their bits are
given in system/include/cmsis/stm32f051x8.h */


/* Clock prescaler for TIM2 timer: no prescaling */
#define myTIM2_PRESCALER ((uint16_t)0x0000)
#define myTIM3_PRESCALER ((uint16_t)0xBB7F)
#define myTIM14_PRESCALER ((uint16_t)0xBB7F)
/* Maximum possible setting for overflow */
#define myTIM2_PERIOD ((uint32_t)0xFFFFFFFF)
#define myTIM3_PERIOD ((uint16_t)0x0064)
#define myTIM14_PERIOD ((uint16_t)0x0064)

void myGPIOA_Init(void);
void myGPIOB_Init(void);
void myTIM2_Init(void);
void myTIM3_Init(void);
void myTIM14_Init(void);
void myEXTI0_Init(void);
void myEXTI2_3_Init(void);
void myADC1_Init(void);
void myDAC_Init(void);
void mySPI2_Init(void);

void oled_Write(unsigned char);
void oled_Write_Cmd(unsigned char);
void oled_Write_Data(unsigned char);

void oled_config(void);

//void refresh_OLED(void);

SPI_HandleTypeDef SPI_Handle;


// Declare/initialize your global variables here...
// NOTE: You'll need at least one global variable
// (say, timerTriggered = 0 or 1) to indicate
// whether TIM2 has started counting or not.
unsigned int timerTriggered = 0;

unsigned int Freq = 0;  // Example: measured frequency value (global variable)
unsigned int Res = 0;   // Example: measured resistance value (global variable)


//
// LED Display initialization commands
//
unsigned char oled_init_cmds[] =
{
    0xAE,
    0x20, 0x00,
    0x40,
    0xA0 | 0x01,
    0xA8, 0x40 - 1,
    0xC0 | 0x08,
    0xD3, 0x00,
    0xDA, 0x32,
    0xD5, 0x80,
    0xD9, 0x22,
    0xDB, 0x30,
    0x81, 0xFF,
    0xA4,
    0xA6,
    0xAD, 0x30,
    0x8D, 0x10,
    0xAE | 0x01,
    0xC0,
    0xA0
};


//
// Character specifications for LED Display (1 row = 8 bytes = 1 ASCII character)
// Example: to display '4', retrieve 8 data bytes stored in Characters[52][X] row
//          (where X = 0, 1, ..., 7) and send them one by one to LED Display.
// Row number = character ASCII code (e.g., ASCII code of '4' is 0x34 = 52)
//
unsigned char Characters[][8] = {
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // SPACE
    {0b00000000, 0b00000000, 0b01011111, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // !
    {0b00000000, 0b00000111, 0b00000000, 0b00000111, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // "
    {0b00010100, 0b01111111, 0b00010100, 0b01111111, 0b00010100,0b00000000, 0b00000000, 0b00000000},  // #
    {0b00100100, 0b00101010, 0b01111111, 0b00101010, 0b00010010,0b00000000, 0b00000000, 0b00000000},  // $
    {0b00100011, 0b00010011, 0b00001000, 0b01100100, 0b01100010,0b00000000, 0b00000000, 0b00000000},  // %
    {0b00110110, 0b01001001, 0b01010101, 0b00100010, 0b01010000,0b00000000, 0b00000000, 0b00000000},  // &
    {0b00000000, 0b00000101, 0b00000011, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // '
    {0b00000000, 0b00011100, 0b00100010, 0b01000001, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // (
    {0b00000000, 0b01000001, 0b00100010, 0b00011100, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // )
    {0b00010100, 0b00001000, 0b00111110, 0b00001000, 0b00010100,0b00000000, 0b00000000, 0b00000000},  // *
    {0b00001000, 0b00001000, 0b00111110, 0b00001000, 0b00001000,0b00000000, 0b00000000, 0b00000000},  // +
    {0b00000000, 0b01010000, 0b00110000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // ,
    {0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000,0b00000000, 0b00000000, 0b00000000},  // -
    {0b00000000, 0b01100000, 0b01100000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // .
    {0b00100000, 0b00010000, 0b00001000, 0b00000100, 0b00000010,0b00000000, 0b00000000, 0b00000000},  // /
    {0b00111110, 0b01010001, 0b01001001, 0b01000101, 0b00111110,0b00000000, 0b00000000, 0b00000000},  // 0
    {0b00000000, 0b01000010, 0b01111111, 0b01000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // 1
    {0b01000010, 0b01100001, 0b01010001, 0b01001001, 0b01000110,0b00000000, 0b00000000, 0b00000000},  // 2
    {0b00100001, 0b01000001, 0b01000101, 0b01001011, 0b00110001,0b00000000, 0b00000000, 0b00000000},  // 3
    {0b00011000, 0b00010100, 0b00010010, 0b01111111, 0b00010000,0b00000000, 0b00000000, 0b00000000},  // 4
    {0b00100111, 0b01000101, 0b01000101, 0b01000101, 0b00111001,0b00000000, 0b00000000, 0b00000000},  // 5
    {0b00111100, 0b01001010, 0b01001001, 0b01001001, 0b00110000,0b00000000, 0b00000000, 0b00000000},  // 6
    {0b00000011, 0b00000001, 0b01110001, 0b00001001, 0b00000111,0b00000000, 0b00000000, 0b00000000},  // 7
    {0b00110110, 0b01001001, 0b01001001, 0b01001001, 0b00110110,0b00000000, 0b00000000, 0b00000000},  // 8
    {0b00000110, 0b01001001, 0b01001001, 0b00101001, 0b00011110,0b00000000, 0b00000000, 0b00000000},  // 9
    {0b00000000, 0b00110110, 0b00110110, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // :
    {0b00000000, 0b01010110, 0b00110110, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // ;
    {0b00001000, 0b00010100, 0b00100010, 0b01000001, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // <
    {0b00010100, 0b00010100, 0b00010100, 0b00010100, 0b00010100,0b00000000, 0b00000000, 0b00000000},  // =
    {0b00000000, 0b01000001, 0b00100010, 0b00010100, 0b00001000,0b00000000, 0b00000000, 0b00000000},  // >
    {0b00000010, 0b00000001, 0b01010001, 0b00001001, 0b00000110,0b00000000, 0b00000000, 0b00000000},  // ?
    {0b00110010, 0b01001001, 0b01111001, 0b01000001, 0b00111110,0b00000000, 0b00000000, 0b00000000},  // @
    {0b01111110, 0b00010001, 0b00010001, 0b00010001, 0b01111110,0b00000000, 0b00000000, 0b00000000},  // A
    {0b01111111, 0b01001001, 0b01001001, 0b01001001, 0b00110110,0b00000000, 0b00000000, 0b00000000},  // B
    {0b00111110, 0b01000001, 0b01000001, 0b01000001, 0b00100010,0b00000000, 0b00000000, 0b00000000},  // C
    {0b01111111, 0b01000001, 0b01000001, 0b00100010, 0b00011100,0b00000000, 0b00000000, 0b00000000},  // D
    {0b01111111, 0b01001001, 0b01001001, 0b01001001, 0b01000001,0b00000000, 0b00000000, 0b00000000},  // E
    {0b01111111, 0b00001001, 0b00001001, 0b00001001, 0b00000001,0b00000000, 0b00000000, 0b00000000},  // F
    {0b00111110, 0b01000001, 0b01001001, 0b01001001, 0b01111010,0b00000000, 0b00000000, 0b00000000},  // G
    {0b01111111, 0b00001000, 0b00001000, 0b00001000, 0b01111111,0b00000000, 0b00000000, 0b00000000},  // H
    {0b01000000, 0b01000001, 0b01111111, 0b01000001, 0b01000000,0b00000000, 0b00000000, 0b00000000},  // I
    {0b00100000, 0b01000000, 0b01000001, 0b00111111, 0b00000001,0b00000000, 0b00000000, 0b00000000},  // J
    {0b01111111, 0b00001000, 0b00010100, 0b00100010, 0b01000001,0b00000000, 0b00000000, 0b00000000},  // K
    {0b01111111, 0b01000000, 0b01000000, 0b01000000, 0b01000000,0b00000000, 0b00000000, 0b00000000},  // L
    {0b01111111, 0b00000010, 0b00001100, 0b00000010, 0b01111111,0b00000000, 0b00000000, 0b00000000},  // M
    {0b01111111, 0b00000100, 0b00001000, 0b00010000, 0b01111111,0b00000000, 0b00000000, 0b00000000},  // N
    {0b00111110, 0b01000001, 0b01000001, 0b01000001, 0b00111110,0b00000000, 0b00000000, 0b00000000},  // O
    {0b01111111, 0b00001001, 0b00001001, 0b00001001, 0b00000110,0b00000000, 0b00000000, 0b00000000},  // P
    {0b00111110, 0b01000001, 0b01010001, 0b00100001, 0b01011110,0b00000000, 0b00000000, 0b00000000},  // Q
    {0b01111111, 0b00001001, 0b00011001, 0b00101001, 0b01000110,0b00000000, 0b00000000, 0b00000000},  // R
    {0b01000110, 0b01001001, 0b01001001, 0b01001001, 0b00110001,0b00000000, 0b00000000, 0b00000000},  // S
    {0b00000001, 0b00000001, 0b01111111, 0b00000001, 0b00000001,0b00000000, 0b00000000, 0b00000000},  // T
    {0b00111111, 0b01000000, 0b01000000, 0b01000000, 0b00111111,0b00000000, 0b00000000, 0b00000000},  // U
    {0b00011111, 0b00100000, 0b01000000, 0b00100000, 0b00011111,0b00000000, 0b00000000, 0b00000000},  // V
    {0b00111111, 0b01000000, 0b00111000, 0b01000000, 0b00111111,0b00000000, 0b00000000, 0b00000000},  // W
    {0b01100011, 0b00010100, 0b00001000, 0b00010100, 0b01100011,0b00000000, 0b00000000, 0b00000000},  // X
    {0b00000111, 0b00001000, 0b01110000, 0b00001000, 0b00000111,0b00000000, 0b00000000, 0b00000000},  // Y
    {0b01100001, 0b01010001, 0b01001001, 0b01000101, 0b01000011,0b00000000, 0b00000000, 0b00000000},  // Z
    {0b01111111, 0b01000001, 0b00000000, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // [
    {0b00010101, 0b00010110, 0b01111100, 0b00010110, 0b00010101,0b00000000, 0b00000000, 0b00000000},  // back slash
    {0b00000000, 0b00000000, 0b00000000, 0b01000001, 0b01111111,0b00000000, 0b00000000, 0b00000000},  // ]
    {0b00000100, 0b00000010, 0b00000001, 0b00000010, 0b00000100,0b00000000, 0b00000000, 0b00000000},  // ^
    {0b01000000, 0b01000000, 0b01000000, 0b01000000, 0b01000000,0b00000000, 0b00000000, 0b00000000},  // _
    {0b00000000, 0b00000001, 0b00000010, 0b00000100, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // `
    {0b00100000, 0b01010100, 0b01010100, 0b01010100, 0b01111000,0b00000000, 0b00000000, 0b00000000},  // a
    {0b01111111, 0b01001000, 0b01000100, 0b01000100, 0b00111000,0b00000000, 0b00000000, 0b00000000},  // b
    {0b00111000, 0b01000100, 0b01000100, 0b01000100, 0b00100000,0b00000000, 0b00000000, 0b00000000},  // c
    {0b00111000, 0b01000100, 0b01000100, 0b01001000, 0b01111111,0b00000000, 0b00000000, 0b00000000},  // d
    {0b00111000, 0b01010100, 0b01010100, 0b01010100, 0b00011000,0b00000000, 0b00000000, 0b00000000},  // e
    {0b00001000, 0b01111110, 0b00001001, 0b00000001, 0b00000010,0b00000000, 0b00000000, 0b00000000},  // f
    {0b00001100, 0b01010010, 0b01010010, 0b01010010, 0b00111110,0b00000000, 0b00000000, 0b00000000},  // g
    {0b01111111, 0b00001000, 0b00000100, 0b00000100, 0b01111000,0b00000000, 0b00000000, 0b00000000},  // h
    {0b00000000, 0b01000100, 0b01111101, 0b01000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // i
    {0b00100000, 0b01000000, 0b01000100, 0b00111101, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // j
    {0b01111111, 0b00010000, 0b00101000, 0b01000100, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // k
    {0b00000000, 0b01000001, 0b01111111, 0b01000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // l
    {0b01111100, 0b00000100, 0b00011000, 0b00000100, 0b01111000,0b00000000, 0b00000000, 0b00000000},  // m
    {0b01111100, 0b00001000, 0b00000100, 0b00000100, 0b01111000,0b00000000, 0b00000000, 0b00000000},  // n
    {0b00111000, 0b01000100, 0b01000100, 0b01000100, 0b00111000,0b00000000, 0b00000000, 0b00000000},  // o
    {0b01111100, 0b00010100, 0b00010100, 0b00010100, 0b00001000,0b00000000, 0b00000000, 0b00000000},  // p
    {0b00001000, 0b00010100, 0b00010100, 0b00011000, 0b01111100,0b00000000, 0b00000000, 0b00000000},  // q
    {0b01111100, 0b00001000, 0b00000100, 0b00000100, 0b00001000,0b00000000, 0b00000000, 0b00000000},  // r
    {0b01001000, 0b01010100, 0b01010100, 0b01010100, 0b00100000,0b00000000, 0b00000000, 0b00000000},  // s
    {0b00000100, 0b00111111, 0b01000100, 0b01000000, 0b00100000,0b00000000, 0b00000000, 0b00000000},  // t
    {0b00111100, 0b01000000, 0b01000000, 0b00100000, 0b01111100,0b00000000, 0b00000000, 0b00000000},  // u
    {0b00011100, 0b00100000, 0b01000000, 0b00100000, 0b00011100,0b00000000, 0b00000000, 0b00000000},  // v
    {0b00111100, 0b01000000, 0b00111000, 0b01000000, 0b00111100,0b00000000, 0b00000000, 0b00000000},  // w
    {0b01000100, 0b00101000, 0b00010000, 0b00101000, 0b01000100,0b00000000, 0b00000000, 0b00000000},  // x
    {0b00001100, 0b01010000, 0b01010000, 0b01010000, 0b00111100,0b00000000, 0b00000000, 0b00000000},  // y
    {0b01000100, 0b01100100, 0b01010100, 0b01001100, 0b01000100,0b00000000, 0b00000000, 0b00000000},  // z
    {0b00000000, 0b00001000, 0b00110110, 0b01000001, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // {
    {0b00000000, 0b00000000, 0b01111111, 0b00000000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // |
    {0b00000000, 0b01000001, 0b00110110, 0b00001000, 0b00000000,0b00000000, 0b00000000, 0b00000000},  // }
    {0b00001000, 0b00001000, 0b00101010, 0b00011100, 0b00001000,0b00000000, 0b00000000, 0b00000000},  // ~
    {0b00001000, 0b00011100, 0b00101010, 0b00001000, 0b00001000,0b00000000, 0b00000000, 0b00000000}   // <-
};

/*** Call this function to boost the STM32F0xx clock to 48 MHz ***/

void SystemClock48MHz( void )
{
	//
	// Disable the PLL
	//
	RCC->CR &= ~(RCC_CR_PLLON);
	//
	// Wait for the PLL to unlock
	//
	while (( RCC->CR & RCC_CR_PLLRDY ) != 0 );
	//
	// Configure the PLL for 48-MHz system clock
	//
	RCC->CFGR = 0x00280000;
	//
	// Enable the PLL
	//
	RCC->CR |= RCC_CR_PLLON;
	//
	// Wait for the PLL to lock
	//
	while (( RCC->CR & RCC_CR_PLLRDY ) != RCC_CR_PLLRDY );
	//
	// Switch the processor to the PLL clock source
	//
	RCC->CFGR = ( RCC->CFGR & (~RCC_CFGR_SW_Msk)) | RCC_CFGR_SW_PLL;
	//
	// Update the system with the new clock frequency
	//
	SystemCoreClockUpdate();

}

/*****************************************************************/


int main(int argc, char* argv[])
{
	SystemClock48MHz();

	trace_printf("This is the Lab...\n");
	trace_printf("System clock: %u Hz\n", SystemCoreClock);

	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN; /* Enable SYSCFG clock */

	myGPIOA_Init(); 	/* Initialize I/O ports PA */
	myGPIOB_Init(); 	/* Initialize I/O ports PB */
	myTIM2_Init(); 		/* Initialize timer TIM2 */
	myTIM3_Init(); 		/* Initialize timer TIM3 */
	myTIM14_Init(); 	/* Initialize timer TIM14 */
	myEXTI0_Init(); 	/* Initialize EXTI0 */
	myEXTI2_3_Init(); 	/* Initialize EXTI2 and EXTI3 */
	myADC1_Init(); 		/* Initialize ADC1 */
	myDAC_Init(); 		/* Initialize DAC */
	mySPI2_Init(); 		/* Initialize SPI2 */

	oled_config();	/* Configure OLED */

	uint16_t ADC_val;
	uint16_t DAC_DOR;

	double DAC_out;
	double voltage_DDA = 3.3f;

	uint16_t voltage_V;
	uint16_t voltage_mV;
	uint16_t resistance_Ohms;

	while (1)
	{
		/* Wait for conversion */
		while ((ADC1->ISR & ADC_ISR_EOC) == 0);

		/* Store converted value */
		ADC_val = ADC1->DR & 0x0FFF;

		/* Use converted value */
		DAC->DHR12R1 = ADC_val;

		/* Store converted value */
		DAC_DOR = DAC->DOR1 & 0x0FFF;

		/* Store converted value voltage */
		DAC_out = DAC_DOR / 4095.0f * voltage_DDA;
		voltage_V = (uint16_t)DAC_out;
		voltage_mV = (uint16_t)((DAC_out - voltage_V) * 1000);

		/* Store converted value resistance */
		resistance_Ohms = (voltage_DDA - DAC_out) / voltage_DDA * 5000.0f;

		// Nothing is going on here...
		// trace_printf("This is the ADC_val: %u\n", ADC_val);
		// trace_printf("This is the resistance: %u Ohms\n", resistance_Ohms);
		// trace_printf("This is the voltage: %u.%03u V\n", voltage_V, voltage_mV);

		// refresh_OLED();		/* Refresh OLED */
	}

	return 0;

}


void myGPIOA_Init()
{
	/* Enable clock for GPIOA peripheral */
	// Relevant register: RCC->AHBENR
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

	/* Configure PA0 as input */
	// Relevant register: GPIOA->MODER
	GPIOA->MODER &= ~(GPIO_MODER_MODER0);

	/* Configure PA1 as analog */
	// Relevant register: GPIOA->MODER
	GPIOA->MODER |= GPIO_MODER_MODER1;

	/* Configure PA4 as analog */
	// Relevant register: GPIOA->MODER
	GPIOA->MODER |= GPIO_MODER_MODER4;

	/* Ensure no pull-up/pull-down for PA0, PA1 and PA4 */
	// Relevant register: GPIOA->PUPDR
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR0);
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR1);
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPDR4);
}

void myGPIOB_Init()
{
	/* Enable clock for GPIOB peripheral */
	// Relevant register: RCC->AHBENR
	RCC->AHBENR |= RCC_AHBENR_GPIOBEN;

	/* Configure PB2 and PB3 as input */
	// Relevant register: GPIOB->MODER
	GPIOB->MODER &= ~(GPIO_MODER_MODER2);
	GPIOB->MODER &= ~(GPIO_MODER_MODER3);

	/* Configure PB8, PB9 and PB11 as output */
	// Relevant register: GPIOB->MODER
	GPIOB->MODER |= GPIO_MODER_MODER8;
	GPIOB->MODER |= GPIO_MODER_MODER9;
	GPIOB->MODER |= GPIO_MODER_MODER11;

	/* Ensure no pull-up/pull-down for PB2 and PB3 */
	// Relevant register: GPIOB->PUPDR
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR2);
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPDR3);

	/* Configure PB13 as alternate function */
	// Relevant register: GPIOB->MODER
	GPIOB->MODER &= ~(GPIO_MODER_MODER13_0);
	GPIOB->MODER |= GPIO_MODER_MODER13_1;

	/* Configure PB15 as alternate function */
	// Relevant register: GPIOB->MODER
	GPIOB->MODER &= ~(GPIO_MODER_MODER15_0);
	GPIOB->MODER |= GPIO_MODER_MODER15_1;

	/* Configure PB13 and PB15 as AF0 */
	GPIOB->AFR[1] &= ~(GPIO_AFRH_AFSEL13);
	GPIOB->AFR[1] &= ~(GPIO_AFRH_AFSEL15);
}

void myTIM2_Init()
{
	/* Enable clock for TIM2 peripheral */
	// Relevant register: RCC->APB1ENR
	RCC->APB1ENR |= (RCC_APB1ENR_TIM2EN);

	/* Configure TIM2: buffer auto-reload, count up, stop on overflow,
	* enable update events, interrupt on overflow only */
	// Relevant register: TIM2->CR1
	TIM2->CR1 = ((uint16_t)0x008C);

	/* Set myTIM2_PRESCALER clock prescaler value */
	TIM2->PSC = myTIM2_PRESCALER;
	/* Set myTIM2_PERIOD auto-reloaded delay */
	TIM2->ARR = myTIM2_PERIOD;

	/* Update timer registers */
	// Relevant register: TIM2->EGR
	TIM2->EGR |= TIM_EGR_UG;

	/* Assign TIM2 interrupt priority = 0 in NVIC */
	// Relevant register: NVIC->IP[3], or use NVIC_SetPriority
	NVIC_SetPriority(TIM2_IRQn, 0);

	/* Enable TIM2 interrupts in NVIC */
	// Relevant register: NVIC->ISER[0], or use NVIC_EnableIRQ
	NVIC_EnableIRQ(TIM2_IRQn);

	/* Enable update interrupt generation */
	// Relevant register: TIM2->DIER
	TIM2->DIER |= TIM_DIER_UIE;
}

void myTIM3_Init()
{
	/* Enable clock for TIM3 peripheral */
	// Relevant register: RCC->APB1ENR
	RCC->APB1ENR |= (RCC_APB1ENR_TIM3EN);

	/* Configure TIM3: buffer auto-reload, count up
	* enable update events, interrupt on overflow only */
	// Relevant register: TIM3->CR1
	TIM3->CR1 = ((uint16_t)0x0084);

	/* Set myTIM3_PRESCALER clock prescaler value */
	TIM3->PSC = myTIM3_PRESCALER;
	/* Set myTIM3_PERIOD auto-reloaded delay */
	TIM3->ARR = myTIM3_PERIOD;

	/* Update timer registers */
	// Relevant register: TIM3->EGR
	TIM3->EGR |= TIM_EGR_UG;

	// - Start timer (TIM3->CR1).
	TIM3->CR1 |= TIM_CR1_CEN;
}

void myTIM14_Init()
{
	/* Enable clock for TIM14 peripheral */
	// Relevant register: RCC->APB1ENR
	RCC->APB1ENR |= (RCC_APB1ENR_TIM14EN);

	/* Configure TIM14: buffer auto-reload, count up, stop on overflow,
	* enable update events, interrupt on overflow only */
	// Relevant register: TIM14->CR1
	TIM14->CR1 = ((uint16_t)0x008C);

	/* Set myTIM14_PRESCALER clock prescaler value */
	TIM14->PSC = myTIM14_PRESCALER;
	/* Set myTIM14_PERIOD auto-reloaded delay */
	TIM14->ARR = myTIM14_PERIOD;

	/* Update timer registers */
	// Relevant register: TIM14->EGR
	TIM14->EGR |= TIM_EGR_UG;

	/* Assign TIM14 interrupt priority = 1 in NVIC */
	// Use NVIC_SetPriority
	NVIC_SetPriority(TIM14_IRQn, 1);

	/* Enable TIM14 interrupts in NVIC */
	// Use NVIC_EnableIRQ
	NVIC_EnableIRQ(TIM14_IRQn);

	/* Enable update interrupt generation */
	// Relevant register: TIM14->DIER
	TIM14->DIER |= TIM_DIER_UIE;
}

void myEXTI0_Init()
{
	/* Map EXTI0 line to PA0 */
	// Relevant register: SYSCFG->EXTICR[0]
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI0_PA;

	/* EXTI0 line interrupts: set falling-edge trigger */
	// Relevant register: EXTI->FTSR
	EXTI->FTSR |= EXTI_FTSR_TR0;

	/* Unmask interrupts from EXTI0 line */
	// Relevant register: EXTI->IMR
	EXTI->IMR |= EXTI_IMR_MR0;

	/* Assign EXTI0 interrupt priority = 0 in NVIC */
	// Use NVIC_SetPriority
	NVIC_SetPriority(EXTI0_1_IRQn, 0);

	/* Enable EXTI0 interrupts in NVIC */
	// Use NVIC_EnableIRQ
	NVIC_EnableIRQ(EXTI0_1_IRQn);
}

void myEXTI2_3_Init()
{
	/* Map EXTI2 line to PB2 */
	// Relevant register: SYSCFG->EXTICR[0]
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PB;

	/* Map EXTI3 line to PB3 */
	// Relevant register: SYSCFG->EXTICR[0]
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PB;

	/* EXTI2 line interrupts: set rising-edge trigger */
	// Relevant register: EXTI->RTSR
	EXTI->RTSR |= EXTI_RTSR_TR2;

	/* EXTI3 line interrupts: set rising-edge trigger */
	// Relevant register: EXTI->RTSR
	EXTI->RTSR |= EXTI_RTSR_TR3;

	/* Unmask interrupts from EXTI2 line */
	// Relevant register: EXTI->IMR
	EXTI->IMR |= EXTI_IMR_MR2;

	/* Mask interrupts from EXTI3 line */
	// Relevant register: EXTI->IMR
	EXTI->IMR &= ~(EXTI_IMR_MR3);

	/* Assign EXTI2 and EXTI3 interrupt priority = 1 in NVIC */
	// Relevant register: NVIC->IP[2], or use NVIC_SetPriority
	NVIC_SetPriority(EXTI2_3_IRQn, 1);

	/* Enable EXTI2 and EXTI3 interrupts in NVIC */
	// Relevant register: NVIC->ISER[0], or use NVIC_EnableIRQ
	NVIC_EnableIRQ(EXTI2_3_IRQn);
}

void myADC1_Init()
{
	/* Enable clock for ADC1 peripheral */
	RCC->APB2ENR |= RCC_APB2ENR_ADCEN;

	/* Enable clock for HSI */
	RCC->CR2 |= RCC_CR2_HSI14ON;

	/* Wait for the HSI clock to be ready */
	while ((RCC->CR2 & RCC_CR2_HSI14RDY) == 0);

	/* Configure ADC1 data resolution */
	ADC1->CFGR1 &= ~(ADC_CFGR1_RES);

	/* Configure ADC1 data alignment */
	ADC1->CFGR1 &= ~(ADC_CFGR1_ALIGN);

	/* Configure ADC1 overrun management mode */
	ADC1->CFGR1 |= ADC_CFGR1_OVRMOD;

	/* Configure ADC1 continuous conversion mode */
	ADC1->CFGR1 |= ADC_CFGR1_CONT;

	/* Configure ADC1 channel selection register */
	ADC1->CHSELR |= ADC_CHSELR_CHSEL1;

	/* Configure ADC1 sampling time register  */
	ADC1->SMPR |= ADC_SMPR_SMP;

	/* Enable ADC */
	ADC1->CR |= ADC_CR_ADEN;

	/* Wait for the ISR to be ready */
	while ((ADC1->ISR & ADC_ISR_ADRDY) == 0);

	/* Start ADC */
	ADC1->CR |= ADC_CR_ADSTART;
}

void myDAC_Init() {
	/* Enable clock for ADC1 peripheral */
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;

	/* Configure channel1 tri-state buffer disable */
	DAC->CR &= ~(DAC_CR_BOFF1);

	/* Configure channel1 trigger enable */
	DAC->CR &= ~(DAC_CR_TEN1);

	/* Enable DAC */
	DAC->CR |= DAC_CR_EN1;
}

void mySPI2_Init() {
	/* Enable clock for SPI2 peripheral */
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
}

/* This handler is declared in system/src/cmsis/vectors_stm32f051x8.c */
void EXTI0_1_IRQHandler()
{
	/* Mask interrupts from EXTI0 line */
	EXTI->IMR &= ~(EXTI_IMR_MR0);

	/* Check if EXTI0 interrupt pending flag is indeed set */
	if ((EXTI->PR & EXTI_PR_PR0) != 0) {
		// - Clear count register (TIM14->CNT).
		TIM14->CNT = 0U;
		// - Start timer (TIM14->CR1).
		TIM14->CR1 |= TIM_CR1_CEN;
		if ((EXTI->IMR & EXTI_IMR_MR2) == 0) {
			// Clear EXTI2 interrupt pending flag (EXTI->PR).
			EXTI->PR |= EXTI_PR_PR2;
			/* Mask interrupts from EXTI3 line */
			EXTI->IMR &= ~(EXTI_IMR_MR3);
			/* Unmask interrupts from EXTI2 line */
			EXTI->IMR |= EXTI_IMR_MR2;
		} else {
			// Clear EXTI3 interrupt pending flag (EXTI->PR).
			EXTI->PR |= EXTI_PR_PR3;
			/* Mask interrupts from EXTI2 line */
			EXTI->IMR &= ~(EXTI_IMR_MR2);
			/* Unmask interrupts from EXTI3 line */
			EXTI->IMR |= EXTI_IMR_MR3;
		}

		timerTriggered = 0;
		// Clear EXTI0 interrupt pending flag (EXTI->PR).
		EXTI->PR |= EXTI_PR_PR0;
		trace_printf("User Button Used\n");
	}
}

/* This handler is declared in system/src/cmsis/vectors_stm32f051x8.c */
void TIM2_IRQHandler()
{
	/* Check if update interrupt flag is indeed set */
	if ((TIM2->SR & TIM_SR_UIF) != 0)
	{
		trace_printf("\n*** Overflow! ***\n");

		/* Clear update interrupt flag */
		// Relevant register: TIM2->SR
		TIM2->SR &= ~(TIM_SR_UIF);

		/* Restart stopped timer */
		// Relevant register: TIM2->CR1
		TIM2->CR1 |= TIM_CR1_CEN;
	}
}

/* This handler is declared in system/src/cmsis/vectors_stm32f051x8.c */
void TIM14_IRQHandler()
{
	/* Check if update interrupt flag is indeed set */
	if ((TIM14->SR & TIM_SR_UIF) != 0)
	{
		/* Clear update interrupt flag */
		// Relevant register: TIM14->SR
		TIM14->SR &= ~(TIM_SR_UIF);

		// Clear EXTI0 interrupt pending flag (EXTI->PR).
		EXTI->PR |= EXTI_PR_PR0;
		/* Unmask interrupts from EXTI0 line */
		EXTI->IMR |= EXTI_IMR_MR0;
	}
}

/* This handler is declared in system/src/cmsis/vectors_stm32f051x8.c */
void EXTI2_3_IRQHandler()
{
	// Declare/initialize your local variables here...
	unsigned int read_out = 0;
	double period = 0;
	double frequency_Hz = 0;
	double frequency_mHz = 0;


	/* Check if EXTI2 interrupt pending flag is indeed set */
	if ((EXTI->PR & EXTI_PR_PR2) != 0 || (EXTI->PR & EXTI_PR_PR3) != 0)
	{
		//
		// 1. If this is the first edge:
		if (!timerTriggered) {
			// - Clear count register (TIM2->CNT).
			TIM2->CNT = 0U;
			// - Start timer (TIM2->CR1).
			TIM2->CR1 |= TIM_CR1_CEN;
			// - indicate that TIM2 has started counting (timerTriggered).
			timerTriggered = 1;
		}
		// Else (this is the second edge):
		else {
			// - Stop timer (TIM2->CR1).
			TIM2->CR1 &= ~(TIM_CR1_CEN);
			// - Read out count register (TIM2->CNT).
			read_out = TIM2->CNT;
			if (read_out > 0U) {
				// - Calculate signal period (s) and frequency.
				period = (double)read_out / 48000000.0;
				frequency_Hz = 48000000.0 / (double)read_out;
				frequency_mHz = 48000000000.0 / (double)read_out;
				Freq = frequency_Hz;
				// - Print calculated values to the console.
				// Cannot detect too low frequencies (<100 mHz)
				// -> TIM2 will time out between edges (overflow)
				// Cannot detect too high frequencies accurately (>220 kHz)
				// -> CPU gets overloaded and cannot process all interrupts
				// -> frequency gets inaccurate since CPU misses interrupts
				trace_printf("signal period %u us\n", (unsigned int)(period * 1000000));
				trace_printf("signal frequency %u Hz %u mHz\n", (unsigned int)frequency_Hz, (unsigned int)frequency_mHz);
				// NOTE: Function trace_printf does not work
				// with floating-point numbers: you must use
				// "unsigned int" type to print your signal
				// period and frequency.
				// - indicate that TIM2 has stopped counting (timerTriggered).
				timerTriggered = 0;
			}
		}
		// 2. Clear EXTI2 and EXTI3 interrupt pending flag (EXTI->PR).
		// NOTE: A pending register (PR) bit is cleared
		// by writing 1 to it.
		EXTI->PR |= EXTI_PR_PR2;
		EXTI->PR |= EXTI_PR_PR3;
	}
}


//
// LED Display Functions (See https://www.ece.uvic.ca/~ece355/lab/project_tips.html)
//


void refresh_OLED( void )
{
    // Buffer size = at most 16 characters per PAGE + terminating '\0'
    unsigned char Buffer[17];

    snprintf( Buffer, sizeof( Buffer ), "R: %5u Ohms", Res );
    /* Buffer now contains your character ASCII codes for LED Display
       - select PAGE (LED Display line) and set starting SEG (column)
       - for each c = ASCII code = Buffer[0], Buffer[1], ...,
           send 8 bytes in Characters[c][0-7] to LED Display
    */

    // ...


    snprintf( Buffer, sizeof( Buffer ), "F: %5u Hz", Freq );
    /* Buffer now contains your character ASCII codes for LED Display
       - select PAGE (LED Display line) and set starting SEG (column)
       - for each c = ASCII code = Buffer[0], Buffer[1], ...,
           send 8 bytes in Characters[c][0-7] to LED Display
    */

    // ...


	/* Wait for ~100 ms (for example) to get ~10 frames/sec refresh rate
       - Use TIM3 to implement this delay (e.g., via polling)
    */
	while((TIM3->SR & TIM_SR_UIF) == 0);

	// Clear TIM3 update interrupt flag (TIM3->SR).
	TIM3->SR &= ~(TIM_SR_UIF);
}


void oled_Write_Cmd( unsigned char cmd )
{
    GPIOB->BSRR = GPIO_BSRR_BS_8; 	// make PB8 = CS# = 1
    GPIOB->BSRR = GPIO_BSRR_BR_9; 	// make PB9 = D/C# = 0
    GPIOB->BSRR = GPIO_BSRR_BR_8; 	// make PB8 = CS# = 0
    oled_Write( cmd );
    GPIOB->BSRR = GPIO_BSRR_BS_8; 	// make PB8 = CS# = 1
}

void oled_Write_Data( unsigned char data )
{
	GPIOB->BSRR = GPIO_BSRR_BS_8; 	// make PB8 = CS# = 1
	GPIOB->BSRR = GPIO_BSRR_BS_9; 	// make PB9 = D/C# = 1
    GPIOB->BSRR = GPIO_BSRR_BR_8; 	// make PB8 = CS# = 0
    oled_Write( data );
    GPIOB->BSRR = GPIO_BSRR_BS_8; 	// make PB8 = CS# = 1
}


void oled_Write( unsigned char Value )
{

    /* Wait until SPI2 is ready for writing (TXE = 1 in SPI2_SR) */

    // ...

    /* Send one 8-bit character:
       - This function also sets BIDIOE = 1 in SPI2_CR1
    */
    HAL_SPI_Transmit( &SPI_Handle, &Value, 1, HAL_MAX_DELAY );


    /* Wait until transmission is complete (TXE = 1 in SPI2_SR) */

    // ...

}


void oled_config( void )
{

// Don't forget to enable GPIOB clock in RCC	- X
// Don't forget to configure PB13/PB15 as AF0	- X
// Don't forget to enable SPI2 clock in RCC		- X

    SPI_Handle.Instance = SPI2;

    SPI_Handle.Init.Direction = SPI_DIRECTION_1LINE;
    SPI_Handle.Init.Mode = SPI_MODE_MASTER;
    SPI_Handle.Init.DataSize = SPI_DATASIZE_8BIT;
    SPI_Handle.Init.CLKPolarity = SPI_POLARITY_LOW;
    SPI_Handle.Init.CLKPhase = SPI_PHASE_1EDGE;
    SPI_Handle.Init.NSS = SPI_NSS_SOFT;
    SPI_Handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    SPI_Handle.Init.FirstBit = SPI_FIRSTBIT_MSB;
    SPI_Handle.Init.CRCPolynomial = 7;

//
// Initialize the SPI interface
//
    HAL_SPI_Init( &SPI_Handle );

//
// Enable the SPI
//
    __HAL_SPI_ENABLE( &SPI_Handle );


    /* Reset LED Display (RES# = PB11):
       - make pin PB11 = 0, wait for a few ms
       - make pin PB11 = 1, wait for a few ms
    */
    GPIOB->BSRR = GPIO_BSRR_BS_11;
    while((TIM3->SR & TIM_SR_UIF) == 0);
    TIM3->SR &= ~(TIM_SR_UIF);
    GPIOB->BSRR = GPIO_BSRR_BR_11;
    while((TIM3->SR & TIM_SR_UIF) == 0);
    TIM3->SR &= ~(TIM_SR_UIF);


//
// Send initialization commands to LED Display
//
    for ( unsigned int i = 0; i < sizeof( oled_init_cmds ); i++ )
    {
        oled_Write_Cmd( oled_init_cmds[i] );
    }


    /* Fill LED Display data memory (GDDRAM) with zeros:
       - for each PAGE = 0, 1, ..., 7
           set starting SEG = 0
           call oled_Write_Data( 0x00 ) 128 times
    */

    for ( unsigned int pg = 0; pg < 8; pg++ )
    {
		oled_Write_Cmd(0xB0 + pg);
		oled_Write_Cmd(0x00);
		oled_Write_Cmd(0x10);
    	for ( unsigned int seg = 0; seg < 128; seg++ ) {
    		oled_Write_Data( 0x00 );
    	}
    }

}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
