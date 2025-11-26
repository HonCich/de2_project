/*
 * The I2C (TWI) bus scanner tests all addresses and detects devices
 * that are connected to the SDA and SCL signals.
 * (c) 2023-2025 Tomas Fryza, MIT license
 *
 * Developed using PlatformIO and Atmel AVR platform.
 * Tested on Arduino Uno board and ATmega328P, 16 MHz.
 */

// -- Includes ---------------------------------------------
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <twi.h>            // I2C/TWI library for AVR-GCC
#include <uart.h>           // Peter Fleury's UART library
#include <stdio.h>          // C library. Needed for `sprintf`
#include <timer.h>
#include <oled.h>

#define DHT_ADR 0x5c
#define DHT_HUM_MEM 0
#define DHT_TEMP_MEM 2

volatile uint8_t flag_update_uart = 0;
volatile uint8_t dht12_values[4];
volatile float temp = 0.0;
volatile float hum = 0.0;

// -- Function definitions ---------------------------------
/*
 * Function: Main function where the program execution begins
 * Purpose:  Call function to test all I2C (TWI) combinations and send
 *           detected devices to UART.
 * Returns:  none
 */
int main(void)
{
    char str_temp[22];
    char str_hum[22];

    twi_init();

    // Initialize USART to asynchronous, 8-N-1, 115200 Bd
    // NOTE: Add `monitor_speed = 115200` to `platformio.ini`
    uart_init(UART_BAUD_SELECT(115200, F_CPU));

    sei();  // Needed for UART

    oled_init(OLED_DISP_ON);
    oled_clrscr();

    oled_charMode(NORMALSIZE);
    tim1_ovf_1sec();
    tim1_ovf_enable();

    // Infinite empty loop
    while (1)
    {
        if (flag_update_uart == 1)
        {
            temp = dht12_values[2]+0.1*dht12_values[3];
            hum = dht12_values[0]+0.1*dht12_values[1];
            sprintf(str_temp, "Teplota: %4.1f °C ", temp); 
            sprintf(str_hum, "Vlhkost: %4.1f %% \r\n", hum);
            
            uart_putc('a');

            oled_gotoxy(0, 2);
            oled_puts(str_temp);
            oled_display();
            oled_gotoxy(0, 4);
            oled_puts(str_hum);
            oled_display();

            // Do not print it again and wait for the new data
            flag_update_uart = 0;
        }
    }

    // Will never reach this
    return 0;
}

ISR(TIMER1_OVF_vect)
{
    twi_readfrom_mem_into(DHT_ADR, DHT_HUM_MEM, dht12_values, 4);
    flag_update_uart = 1;
}
