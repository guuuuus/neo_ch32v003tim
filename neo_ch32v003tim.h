// 2023 guus. neopixel/ws2811/12 lib using tim1 cc1. (GPIOC, pin 4)
// abuses tim1 pwm to generate long / short pulses. Timing is a little out of spec (300khz instead of 400),
// but seems to work with all the led i have laying around
#include <ch32v00x_rcc.h>
#include <ch32v00x_gpio.h>
#include <ch32v00x_tim.h>
#ifndef neo_ch32v003tim_h
#define neo_ch32v003tim_h
typedef enum NEO_STATE
{
    NEO_IDLE,
    NEO_DATA,
    NEO_LATCH,

} NEO_STATE;


typedef enum NEO_CONTINUES
{
    NEO_SINGLE,
    NEO_CONSTANT,

} NEO_CONTINUES;
// set up timer and gpio
void neo_begin();

// sets up timer and gpio, and starts them. 
void neo_beginBuff(unsigned char *p, unsigned short len, NEO_CONTINUES cont);

// starts the timer, either single burst or constant
unsigned char neo_start();

// starts the timer with the new buffer in new mode
unsigned char neo_startBuff(unsigned char *p, unsigned short len, NEO_CONTINUES cont);

// return if currently sending
unsigned char neo_sending();

extern void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

#endif