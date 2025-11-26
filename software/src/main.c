/*
 * Use USART unit and transmit data between ATmega328P and computer.
 * (c) 2018-2025 Tomas Fryza, MIT license
 *
 * Developed using PlatformIO and Atmel AVR platform.
 * Tested on Arduino Uno board and ATmega328P, 16 MHz.
 */

// -- Includes ---------------------------------------------
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <uart.h>           // Peter Fleury's UART library
#include <stdlib.h>         // C library. Needed for number conversions
#include "adc.h"
#include "mq135.h"


// -- Function definitions ---------------------------------
/*
 * Function: Main function where the program execution begins
 * Purpose:  Use Timer/Counter1 and transmit UART data.
 * Returns:  none
 */
int main(void)
{
   
    uint16_t value;
    char uart_msg[8];

    // Initialize USART to asynchronous, 8-N-1, 9600 Bd
    uart_init(UART_BAUD_SELECT(9600, F_CPU));

    // Configure 16-bit Timer/Counter1 to transmit UART data
    // Set prescaler to 1 sec and enable overflow interrupt
    tim1_ovf_1sec();
    tim1_ovf_enable();

    // WRITE YOUR CODE HERE

    adc_init();


    // Interrupts must be enabled, bacause of `uart_puts()` need interrupts
    sei();

    // Put strings to ringbuffer for transmitting via UART
    uart_puts("\r\n");  // New line only
    uart_puts("Start\t using Serial monitor...\r\n");
    // uart_puts("\x1b[4;31m");
    // uart_puts("Char\thaxa\tdeca\tbinary\r\n");
    // uart_puts("\x1b[0m");

    // Infinite loop
    while (1)
    {
        
}

    // Will never reach this
    return 0;
}


// -- Interrupt service routines ---------------------------
/*
 * Function: Timer/Counter1 overflow interrupt
 * Purpose:  Transmit UART data.
 */
ISR(TIMER1_OVF_vect)
{
    // WRITE YOUR CODE HERE
    char msg[40];
    uint16_t val = adc_read();

    float v_meas = (5* (float)val) / 1023.0f;
    float R = (5-v_meas)/(v_meas/20000);

        
    float rs = getResistance(5.0f, v_meas); //5.0f=Vcc

    float ppm = getPPM(rs);
    float ppm_corr = getCorrectedPPM(25.0f, 50.0f, rs); //default temp, edit
    
    sprintf(msg, "V=%.3f  Rs=%.1f  PPM=%.1f  corr=%.1f  rzero=%.1f  r/r0=%0.1f", v_meas, rs, ppm, ppm_corr, RZERO, rs/RZERO);
    uart_puts(msg);
    uart_puts("\r\n");

    
}
