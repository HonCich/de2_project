// -- Includes ---------------------------------------------
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <uart.h>           // Peter Fleury's UART library
#include <stdlib.h>         // C library. Needed for number conversions
#include "adc.h"
#include "mq135.h"
#include <twi.h>            // I2C/TWI library for AVR-GCC
#include <stdio.h>          // C library. Needed for `sprintf`
#include <oled.h>
#include "gpio.h"

#define DHT_ADR 0x5c
#define DHT_HUM_MEM 0
#define DHT_TEMP_MEM 2

#define MQ 1
#define MQ_D PD2 

volatile uint8_t flag_update_uart = 0;
volatile uint8_t dht12_values[4];
volatile float temp = 0.0;
volatile float hum = 0.0;

// -- Function definitions ---------------------------------
/*
 * Function: Main function where the program execution begins
 * Purpose:  Use Timer/Counter1 and transmit UART data.
 * Purpose:  Call function to test all I2C (TWI) combinations and send
 *           detected devices to UART.
 * Returns:  none
 */
int main(void)
{
    uint16_t val = 0;
    char str_temp[22];
    char str_hum[22];
    char str_CO2[30];

    adc_init();
    twi_init();
    
    sei(); // Interrupts must be enabled, bacause of `uart_puts()` need interrupts

    // Initialize USART to asynchronous, 8-N-1, 115200 Bd
    // NOTE: Add `monitor_speed = 115200` to `platformio.ini`
    uart_init(UART_BAUD_SELECT(115200, F_CPU));
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
            twi_readfrom_mem_into(DHT_ADR, DHT_HUM_MEM, dht12_values, 4);
            temp = dht12_values[2]+0.1*dht12_values[3];
            hum = dht12_values[0]+0.1*dht12_values[1];

            val = adc_read(MQ);
            float v_meas = (5 * (float)val) / 1023.0f;
            float rs = getResistance(5.0f, v_meas); //5.0f=Vcc
            float ppm_corr = getCorrectedPPM(25.0f, 50.0f, rs); //default temp, edit

            

            //sprintf(str_temp, "Teplota: %4.1f °C", temp); 
            //sprintf(str_hum, "Vlhkost: %4.1f %%", hum);
            if (gpio_read(&PIND, MQ_D) == 0)
                {
                sprintf(str_CO2, "rs %6.1f, v_meas %4.2f, %4d , %4.1f ppm\tCO2!\r\n", rs,v_meas,val, ppm_corr);
                }
            else{
                sprintf(str_CO2, "rs %6.1f, v_meas %4.2f, %4d , %4.1f ppm\r\n", rs,v_meas,val, ppm_corr);
                }
            //uart_puts(str_temp);
            //uart_puts(str_hum);
            uart_puts(str_CO2);

            oled_gotoxy(0, 1);
            oled_puts(str_temp);
            oled_display();
            oled_gotoxy(0, 3);
            oled_puts(str_hum);
            oled_display();
            oled_gotoxy(0, 4);
            oled_puts(str_CO2);
            oled_display();

            // Do not print it again and wait for the new data
            flag_update_uart = 0;
        }
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
    flag_update_uart = 1;
}
