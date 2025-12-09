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
#include <util/delay.h>  // Functions for busy-wait delay loops

#define DHT_ADR 0x5c
#define DHT_HUM_MEM 0
#define DHT_TEMP_MEM 2

#define MQ 0
#define MQ_D PD2

#define GP_LED_PIN  PB0
#define GP_ADC_CH   1 

volatile uint8_t flag_update_uart = 0;
volatile uint8_t dht12_values[4];
volatile float temp = 0.0;
volatile float hum = 0.0;
volatile uint16_t GP_read = 0;

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
    char str_CO2[20];
    char str_GP[22];

    adc_init();
    twi_init();

    gpio_mode_output(&DDRB, GP_LED_PIN);
    gpio_write_high(&PORTB, GP_LED_PIN);
    
    sei(); // Interrupts must be enabled, bacause of `uart_puts()` need interrupts

    // Initialize USART to asynchronous, 8-N-1, 115200 Bd
    // NOTE: Add `monitor_speed = 115200` to `platformio.ini`
    uart_init(UART_BAUD_SELECT(115200, F_CPU));
    oled_init(OLED_DISP_ON);
    oled_clrscr();

    oled_charMode(NORMALSIZE);
    tim1_ovf_1sec();
    tim1_ovf_enable();
 
    tim2_ovf_16ms();      
    tim2_ovf_enable();   

    // Infinite empty loop
    while (1)
    {
        if (flag_update_uart == 1)
        {
            twi_readfrom_mem_into(DHT_ADR, DHT_HUM_MEM, dht12_values, 4);
            temp = dht12_values[2]+0.1*dht12_values[3];
            hum = dht12_values[0]+0.1*dht12_values[1];

            /* Read MQ135 ADC value */
            val = adc_read(MQ);
            /* Convert ADC value to voltage (V) */
            float v_meas = (5 * (float)val) / 1023.0f;  
            /* Calculate sensor resistance in Ohms */
            float rs = getResistance(5.0f, v_meas); //5V supply
            /* Compute CO2 concentration corrected for temperature and humidity */
            float ppm_corr = getCorrectedPPM(temp, hum, rs);

            /* Read dust sensor and convert to voltage */
            float GP_U = GP_read * (5.0f / 1023.0f);
            /* Convert voltage to dust concentration (ug/m3) */
            float dust = 1000*(GP_U-0.1f) /5.8f;
            /* Prevent negative dust values */
            if (dust < 0) dust = 0;

            sprintf(str_temp, "Teplota: %4.1f °C ", temp); 
            sprintf(str_hum, "Vlhkost: %4.1f %% ", hum);
            
            sprintf(str_CO2, "CO2 = %.1f ppm    ", ppm_corr);
            //sprintf(str_CO2, "V=%.3f  Rs=%.1f  corr=%.1f ", v_meas, rs, ppm_corr);
                        
            sprintf(str_GP, "Dust = %4.2f ug/m3   ", dust);
            //sprintf(str_GP, "U=%5.3f, dust %4.2f", GP_U, dust);

            if( gpio_read(&PIND, MQ_D)==0)
            {
                oled_gotoxy(0, 5);
                oled_puts("CO2 ALERT!");
            }
            else
            {
                oled_gotoxy(0, 5);
                oled_puts("           ");
            }

            //uart_puts(str_temp);
            //uart_puts(str_hum);
            /*
            uart_puts(str_CO2);
            uart_puts("\r\n");
            uart_puts(str_GP);
            uart_puts("\r\n");
            uart_puts("\r\n"); */

            oled_gotoxy(0, 1);
            oled_puts(str_temp);
            oled_gotoxy(0, 2);
            oled_puts(str_hum);
            oled_gotoxy(0, 3);
            oled_puts(str_CO2);
            oled_gotoxy(0, 4);
            oled_puts(str_GP);
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


/**
 * @brief  Timer2 overflow interrupt for dust sensor sampling
 *
 * This ISR generates the LED control pulses required for the dust sensor.
 * On the first state it pulls the LED low and schedules a short delay.
 * On the second state it performs the ADC measurement and turns the LED off.
 */

ISR(TIMER2_OVF_vect)
{
    static uint8_t state = 0;
    if(state == 0){
        /* Turn LED on (active low) */
        gpio_write_low(&PORTB, GP_LED_PIN);

        /* Delay before sampling */
        TCNT2 = 252;      
        state = 1;
    }
    else{
        /* Sample dust sensor after LED-on interval */
        GP_read = adc_read(GP_ADC_CH);

        /* Turn LED off, adc_read() took some time */
        gpio_write_high(&PORTB, GP_LED_PIN);

        /* Delay before next LED cycle */
        TCNT2 = 118;      
        state = 0;
    }
}