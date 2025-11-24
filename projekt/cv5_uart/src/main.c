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
    uart_puts("\x1b[4;31m");
    uart_puts("Char\thaxa\tdeca\tbinary\r\n");
    uart_puts("\x1b[0m");

    // Infinite loop
    while (1)
    {
        /* Get received data from UART
        value = uart_getc();
        if ((value & 0xff00) == 0)  // If successfully received data from UART
        {
            // Transmit the received character back via UART
            if (value == '1')
            {
                    uart_puts("\r\n\t");
                    itoa(TCNT1, uart_msg, 10);
                    uart_puts(uart_msg);
                }
            else
            
            {
            uart_putc(value);
            uart_puts("\t");
            itoa(value, uart_msg, 16);
            uart_puts("0x");
            uart_puts(uart_msg);

            uart_puts("\t");
            uart_puts("\x1b[1;32m");
            itoa(value, uart_msg, 10);
            uart_puts(uart_msg);
            uart_puts("\x1b[0m");

            uart_puts("\x1b[7;34m");
            itoa(value, uart_msg, 2);
            uart_puts("\t");
            uart_puts("0b");
            uart_puts(uart_msg);
            uart_puts("\x1b[0m");

            // New line
            uart_puts("\r\n");
    }
}*/
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
    char msg[20];
    uint16_t val = adc_read();

    float v_meas = (5* (float)val) / 1023.0f;
    float R = (5-v_meas)/(v_meas/20000);

    sprintf(msg, "%f, %f", v_meas, R);
    uart_puts(msg);
    uart_puts("\r\n");
}
