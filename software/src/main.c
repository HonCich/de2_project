// -- Includes ---------------------------------------------
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include <uart.h>           // Peter Fleury's UART library
#include <stdlib.h>         // C library. Needed for number conversions
#include "adc.h"            // Simple ADC driver for AVR (single-ended, 10-bit)
#include "mq135.h"          // Gas concentration sensor library
#include <twi.h>            // I2C/TWI library for AVR-GCC
#include <stdio.h>          // C library. Needed for `sprintf`
#include <oled.h>           // OLED display commands
#include "gpio.h"           // GPIO library for AVR-GCC
#include <util/delay.h>     // Functions for busy-wait delay loops

#define DHT_ADR 0x5c //I2C adress of DHT12 sensor
//Register adresses of humidity and temperature registers
#define DHT_HUM_MEM 0 
#define DHT_TEMP_MEM 2

#define MQ 0
#define MQ_D PD2

#define GP_LED_PIN  PB0
#define GP_ADC_CH   1 

volatile uint8_t flag_update_uart = 0; //Signal flag used to trigger update of displayed values
volatile uint16_t GP_read = 0; 

// -- Function definitions ---------------------------------
/**
 * @brief Main application function for the environmental monitoring system.
 * * @details This function initializes all necessary peripherals (ADC, TWI, UART, OLED, Timers)
 * and enters an infinite loop to manage sensor readings and display updates.
 * It reads data from the DHT12 (Temp/Hum), MQ-135 (CO2), and GP2Y1010AU0F (Dust)
 * sensors and displays the results on an OLED screen. Sensor readings and display
 * updates are triggered by a 1-second interval from Timer1 overflow interrupt.
 * * @note Global interrupts are enabled using sei() to allow Timer and UART operation.
 * * @param void No arguments expected.
 * @return int Returns 0, though this point is never reached in the infinite loop.
 */
int main(void)
{
    uint8_t dht12_values[4];
    float temp = 0.0;
    float hum = 0.0;
    uint16_t val = 0;

    //Storage of strings containing informations about each parameter (displayed strings)
    char str_temp[22];
    char str_hum[22];
    char str_CO2[20];
    char str_GP[22];

    adc_init(); //Initialization of adc for PM sensor reading
    twi_init(); //Initialization of I2C interface

    gpio_mode_output(&DDRB, GP_LED_PIN);
    gpio_write_high(&PORTB, GP_LED_PIN);
    
    sei(); // Interrupts enabled

    // Initialize USART to asynchronous, 8-N-1, 115200 Bd
    uart_init(UART_BAUD_SELECT(115200, F_CPU));
    //Intialize, clear and set font size of OLED display 
    oled_init(OLED_DISP_ON);
    oled_clrscr();
    oled_charMode(NORMALSIZE);
    
    //Enable timer 1 overflow and set prescaler for 1s timing
    tim1_ovf_1sec();
    tim1_ovf_enable();
 
    //Enable timer 2 overflow and set prescaler for 16ms timing
    tim2_ovf_16ms();      
    tim2_ovf_enable();   

    // Infinite loop
    while (1)
    {
        if (flag_update_uart == 1) //Trigered by overflow of timer 1 once every second
        {
            //Read data from DHT12 humidity and temperature resgisters over I2C
            twi_readfrom_mem_into(DHT_ADR, DHT_HUM_MEM, dht12_values, 4);
            //Covert values from 2 pairs of uint8 into 2 floats (one for temperature and one for humidity)
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

            //Turn all read values into displayable strings (2 commented options used for debug)
            sprintf(str_temp, "Teplota: %4.1f °C ", temp); 
            sprintf(str_hum, "Vlhkost: %4.1f %% ", hum);
            sprintf(str_CO2, "CO2 = %.1f ppm    ", ppm_corr);
            //sprintf(str_CO2, "V=%.3f  Rs=%.1f  corr=%.1f ", v_meas, rs, ppm_corr); 
            sprintf(str_GP, "Dust = %4.2f ug/m3   ", dust);
            //sprintf(str_GP, "U=%5.3f, dust %4.2f", GP_U, dust);

            //Display warning for high CO2 level on screen (warning level set by trimmer on MQ sensor)
            if( gpio_read(&PIND, MQ_D)==0)
            {
                oled_gotoxy(0, 5);
                oled_puts("CO2 ALERT!");
            }
            else
            {
                oled_gotoxy(0, 5);
                oled_puts("           ");//clear only warning line of display
            }

            //Printing all values over UART used only for debuging
            //uart_puts(str_temp);
            //uart_puts(str_hum);
            /*
            uart_puts(str_CO2);
            uart_puts("\r\n");
            uart_puts(str_GP);
            uart_puts("\r\n");
            uart_puts("\r\n"); */

            //Print all value strings on separate lines and display
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
/**
 * @brief Timer/Counter1 Overflow Interrupt Service Routine.
 * * @details This ISR is triggered every 1 second (configured by tim1_ovf_1sec()).
 * Its primary purpose is to set the @p flag_update_uart to 1, signaling the main loop
 * to read all sensor data and update the display/UART output.
 * * @param void
 * @return void
 */
ISR(TIMER1_OVF_vect)
{
    flag_update_uart = 1; 
}


/**
 * @brief Timer/Counter2 Overflow Interrupt Service Routine for Dust Sensor LED control.
 * * @details This ISR implements the required LED pulse timing logic for the GP2Y1010AU0F dust sensor.
 * It cycles through two states:
 * - **State 0:** Turns the dust sensor LED **ON** (active low) and sets a short TCNT2 delay 
 * (TCNT2=252) to allow the LED to stabilize.
 * - **State 1:** Takes the **ADC measurement** (`GP_read`) after the stabilization delay, 
 * turns the LED **OFF** (active high), and sets a longer TCNT2 delay (TCNT2=118) 
 * for the rest of the cycle before the next pulse.
 * * @param void
 * @return void
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