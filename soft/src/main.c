/**
 * @file main.c
 *
 * @author Kazuki Saita <saita@kinoshita-lab.com>
 *
 * Copyright (c) 2024 Kinoshita Lab. All rights reserved.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ch32v003_GPIO_branchless.h"
#include "ch32v003fun.h"

#define WS2812BSIMPLE_IMPLEMENTATION
#include "ws2812b_simple.h"

#include "debug.h"

// #define DEBUG_PRINT
#ifndef DEBUG_PRINT
#define printf(...)
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define LED_NUM 15
#define LED_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 4)

uint8_t r = 0;
uint8_t g = 0;
uint8_t b = 0;
uint8_t colorData[3 * LED_NUM];

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} RGBStruct;

uint32_t counter = 0;

void init_rcc(void)
{
    RCC->APB2PCENR |=
        RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;
}

void init_gpio()
{
    GPIO_port_enable(GPIO_port_C);
    GPIO_port_enable(GPIO_port_D);
    GPIO_pinMode(LED_PIN, GPIO_pinMode_O_pushPull, GPIO_Speed_50MHz);
    GPIO_digitalWrite_0(LED_PIN);
    Delay_Ms(100);
}

// init ADC Ch1 6 = PD6
void adc_init(void)
{
    // ADCCLK = 24 MHz => RCC_ADCPRE = 0: divide by 2
    RCC->CFGR0 &= ~(0x1F << 11);

    // Enable GPIOD and ADC
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_ADC1;

    // PD6 is analog input chl 6
    GPIOD->CFGLR &= ~(0xf << (6 * 4)); // CNF6 = 00: Analog, MODE6 = 00: Input

    // Reset the ADC to init all regs
    RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
    RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

    // Set up single conversion on chl
    ADC1->RSQR1 = 0;
    ADC1->RSQR2 = 0;
    ADC1->RSQR3 = 6; // 0-9 for 8 ext inputs and two internals

    // set sampling time for chl 7
    ADC1->SAMPTR2 &= ~(ADC_SMP0 << (3 * 6));
    ADC1->SAMPTR2 |= 7 << (3 * 6); // 0:7 => 3/9/15/30/43/57/73/241 cycles

    // turn on ADC and set rule group to sw trig
    ADC1->CTLR2 |= ADC_ADON | ADC_EXTSEL;

    // Reset calibration
    ADC1->CTLR2 |= ADC_RSTCAL;
    while (ADC1->CTLR2 & ADC_RSTCAL)
        ;

    // Calibrate
    ADC1->CTLR2 |= ADC_CAL;
    while (ADC1->CTLR2 & ADC_CAL)
        ;

    // should be ready for SW conversion now
}

uint16_t adc_get(void)
{
    // start sw conversion (auto clears)
    ADC1->CTLR2 |= ADC_SWSTART;

    // wait for conversion complete
    while (!(ADC1->STATR & ADC_EOC))
        ;

    // get result
    return ADC1->RDATAR;
}

void setBufferRGB(uint8_t r, uint8_t g, uint8_t b, uint8_t* buffer,
                  uint32_t offset)
{
    offset *= 3;

    buffer[offset]     = g;
    buffer[offset + 1] = r;
    buffer[offset + 2] = b;
}

RGBStruct hsv2rgb(float H, float S, float V)
{
    float r, g, b;

    float h = H / 360;
    float s = S / 100;
    float v = V / 100;

    int i   = (int)(h * 6);
    float f = h * 6 - i;
    float p = v * (1 - s);
    float q = v * (1 - f * s);
    float t = v * (1 - (1 - f) * s);

    switch (i % 6) {
    case 0:
        r = v, g = t, b = p;
        break;
    case 1:
        r = q, g = v, b = p;
        break;
    case 2:
        r = p, g = v, b = t;
        break;
    case 3:
        r = p, g = q, b = v;
        break;
    case 4:
        r = t, g = p, b = v;
        break;
    case 5:
        r = v, g = p, b = q;
        break;
    }

    RGBStruct color;
    color.r = r * 255;
    color.g = g * 255;
    color.b = b * 255;

    return color;
}

uint32_t xorshift()
{
    static uint32_t v = 2463534242;
    v ^= v << 13;
    v ^= v >> 17;
    v ^= v << 5;
    return v;
}

int main()
{

    SystemInit();
    init_rcc();
    init_gpio();
    adc_init();
    SDI_Printf_Enable();

    while (1) {
        for (uint8_t i = 0; i < LED_NUM; i++) {
            counter += 1;
            uint8_t val = xorshift() & 0x01;

            RGBStruct rgb = hsv2rgb(counter, 100, 100);
            if (val) {
                setBufferRGB(rgb.r, rgb.g, rgb.b, colorData, i);
            } else {
                setBufferRGB(16, 16, 16, colorData, i);
            }
        }

        WS2812BSimpleSend(GPIOC, 4, colorData, 3 * LED_NUM);

        const uint16_t bpm   = MAX(30, adc_get() >> 2);
        const uint16_t delay = 60000 / bpm / 4;
        Delay_Ms(delay);
    }
}
