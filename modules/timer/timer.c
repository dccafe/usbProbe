#include <msp430.h>
#include <stdint.h>
#include "../gpio/gpio.h"
#include "timer.h"

timer_t timer[3];

void timer_init()
{
    timer[0].ctl = (uint16_t *)TA0_BASE_ADDR;
    timer[1].ctl = (uint16_t *)TA1_BASE_ADDR;
    timer[2].ctl = (uint16_t *)TA2_BASE_ADDR;
}

uint8_t timer_get_free(timer_t ** t, uint8_t port, uint8_t pin)
{
    int i;
    for(i=0; i<3; i++)
    {
        if(timer[i].state == off)
        {
            *t = &timer[i];
            return TIMER_SUCCESS;
        }
    }
    return ERROR_TIMER_NOT_AVAILABLE;
}

uint8_t timer_get_active(timer_t ** t, uint8_t port, uint8_t pin)
{
    int i;
    for(i=0; i<3; i++)
    {
        if((timer[i].state != off ) &&
           (timer[i].port  == port) &&
           (timer[i].pin   == pin ))
        {
            *t = &timer[i];
            return TIMER_SUCCESS;
        }
    }
    return ERROR_TIMER_NOT_ACTIVE;
}

uint8_t timer_off (uint8_t port, uint8_t pin)
{
    timer_t * t = 0;
    if(timer_get_active(&t, port, pin))
        return ERROR_TIMER_NOT_ACTIVE;

    gpio_mode(port, pin, INPUT | PULLUP);
    t->state = off;
    t->port  = 0;
    t->pin   = 0;
    *(t->ctl + 0) = 0; // CTL: Stop timer
    *(t->ctl + 1) = 0; // CCTL0: Disable INTs
    *(t->ctl + 2) = 0; // CCTL1: Disable INTs

    return TIMER_SUCCESS;
}

uint8_t timer_read (uint8_t port, uint8_t pin, uint32_t * freq, uint16_t * dc)
{
    timer_t * t = 0;
    uint16_t * cctl;

    if ((port == 1) &&
        (pin  >= 1) &&
        (pin  <  6))
    {
        t = &timer[0];
        cctl = TA0_BASE_ADDR + pin;
    }
    else
    if ((port == 2) &&
        (pin  <  2))
    {
        t = &timer[1];
        cctl = TA1_BASE_ADDR + pin + 1;
    }
    else
    if ((port == 2) &&
        (pin  >= 3) &&
        (pin  <  6))
    {
        t = &timer[2];
        cctl = TA2_BASE_ADDR + pin - 2;
    }
    else
    {
        return ERROR_TIMER_NOT_AVAILABLE;
    }

    // if (t->state == pwm)
    //     return ERROR_TIMER_NOT_AVAILABLE;

    if (t->state == reading)
        return ERROR_TIMER_DATA_NOT_READY;

    if (t->state == data_ready)
    {
        uint32_t dt_avg  = 0;
        uint32_t ton_avg = 0;

        int i = 0;
        for(i = 0; i < 8; i++)
        {
            ton_avg += t->ton[i];
            dt_avg  += t->dt[i];
        }
        dt_avg  /= 8;
        ton_avg /= 8;

        t->freq = SMCLK_FREQ / dt_avg;
        t->dc   = (100 * ton_avg) / dt_avg;

        if (t->dc > 100)
            t->dc = 100;

        *freq = t->freq;
        *dc   = t->dc;
        return TIMER_SUCCESS;
    }

    *(t->ctl + 0) = MC__CONTINOUS | TASSEL__SMCLK;
    *cctl = CAP | CM_3 | CCIE;

    t->state = reading;
    t->port  = port;
    t->pin   = pin;
    t->freq  = 0;
    t->dc    = 0;

    gpio_mode(port, pin, INPUT | FUNC1);

    return ERROR_TIMER_DATA_NOT_READY;
}

uint8_t timer_write(uint8_t port, uint8_t pin, uint32_t freq, uint16_t dc)
{
    timer_t * t = 0;
    if (timer_get_active(&t, port, pin))
        if (timer_get_free(&t, port, pin))
            return ERROR_TIMER_NOT_AVAILABLE;

    // SMCLK = 16 MHz, ACLK = 32k
    gpio_mode(port, pin, OUTPUT);

    t->state = pwm;
    t->port  = port;
    t->pin   = pin;
    t->freq  = freq;
    t->dc    = dc;

    uint16_t cfg, div;
    if (freq < 500)
    {
        cfg = MC__UP | TASSEL__ACLK;
        div = 32768 / freq;
    }
    else
    {
        cfg = MC__UP | TASSEL__SMCLK;
        div = SMCLK_FREQ / freq;
    }
    dc *= div / 100;

    *(t->ctl +  0) = cfg;       // CTL
    *(t->ctl +  1) = CCIE;      // CCTL0
    *(t->ctl +  2) = CCIE;      // CCTL1
    *(t->ctl +  9) = div - 1;   // CCR0
    *(t->ctl + 10) = dc  - 1;   // CCR1

    return TIMER_SUCCESS;
}


#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_CCR0_ISR()
{
    static uint8_t i = 0;
    static uint16_t t1, t2, t3;

    if(timer[0].state == pwm)
        gpio_write(timer[0].port, timer[0].pin, 1);
    else
    {
        if (TA0CCTL0 & CCI)
        {
            // Rising edge
            t3 = t1;
            t1 = TA0CCR0;
            timer[0].dt[i]  = t3 - t1;
        }
        else
        {
            // Falling edge
            t2 = TA0CCR0;
            timer[0].ton[i] = t2 - t1;
            i += 1;
            if(i == 8)
            {
                timer[0].state = data_ready;
                i = 0;
            }
        }
    }
}

uint16_t history[8];

#pragma vector = TIMER0_A1_VECTOR
__interrupt void TA0_CCRN_ISR()
{
    static uint8_t i = 0;
    static uint16_t t1, t2, t3;

    uint16_t * cctl = ((uint16_t *)TA0_BASE_ADDR) + timer[0].pin;
    *cctl &= ~CCIFG;

    if(timer[0].state == pwm)
        gpio_write(timer[0].port, timer[0].pin, 0);
    else
    {
        uint16_t * ccr  = ((uint16_t *)TA0_BASE_ADDR) + timer[0].pin + 8;

        static int j = 0;
        history[j++] = *ccr;
        if(j == 8) j = 0;

        if (*cctl & CCI)
        {
            // Rising edge
            t1 = t3;
            t3 = *ccr;
            timer[0].dt[i]  = t3 - t1;
        }
        else
        {
            // Falling edge
            t2 = *ccr;
            timer[0].ton[i] = t2 - t3;
            i += 1;
            if(i == 8)
            {
                timer[0].state = data_ready;
                i = 0;
            }
        }
    }

}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_CCR0_ISR()
{
    gpio_write(timer[1].port, timer[1].pin, 1);
}
#pragma vector = TIMER1_A1_VECTOR
__interrupt void TA1_CCRN_ISR()
{
    gpio_write(timer[1].port, timer[1].pin, 0);
    TA0CCTL1 &= ~CCIFG;
}

#pragma vector = TIMER2_A0_VECTOR
__interrupt void TA2_CCR0_ISR()
{
    gpio_write(timer[2].port, timer[2].pin, 1);
}
#pragma vector = TIMER2_A1_VECTOR
__interrupt void TA2_CCRN_ISR()
{
    gpio_write(timer[2].port, timer[2].pin, 0);
    TA0CCTL1 &= ~CCIFG;
}

