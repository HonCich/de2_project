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
#include "gpio.h"


#define MQ_D PD2

#define GP_LED_PIN  PB0
#define GP_ADC_CH   1 

volatile uint16_t GP_read = 0;
volatile uint8_t flag_update_uart = 0;


// -- Function definitions ---------------------------------
/*
 * Function: Main function where the program execution begins
 * Purpose:  Use Timer/Counter1 and transmit UART data.
 * Returns:  none
 */
int main(void)
{


    // Initialize USART to asynchronous, 8-N-1, 9600 Bd
    uart_init(UART_BAUD_SELECT(9600, F_CPU));

    // Configure 16-bit Timer/Counter1 to transmit UART data
    // Set prescaler to 1 sec and enable overflow interrupt
    tim1_ovf_1sec();
    tim1_ovf_enable();

    tim2_ovf_16ms();      
    tim2_ovf_enable(); 

    // WRITE YOUR CODE HERE

    adc_init();

    gpio_mode_output(&DDRB, GP_LED_PIN);
    gpio_write_high(&PORTB, GP_LED_PIN);

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
        if (flag_update_uart == 1)
        {

            char msg[40];
            char msg2[40];
            uint16_t val = adc_read(0);

            float v_meas = (5* (float)val) / 1023.0f;
            // float R = (5-v_meas)/(v_meas/20000);

                
            float rs = getResistance(5.0f, v_meas); //5.0f=Vcc

            float ppm = getPPM(rs);
            float ppm_corr = getCorrectedPPM(25.0f, 50.0f, rs); //default temp, edit
            
            //uint16_t val2 = adc_read(1);

            float v_meas2 = (5* (float)GP_read) / 1023.0f;
            float dust = 1000*(v_meas2-0.1f) /5.8f;
            if( dust < 0) dust = 0;


            if( gpio_read(&PIND, MQ_D)==0)
            {
                uart_puts("CO2 ALERT!\r\n");
            }

            sprintf(msg, "V=%.3f  Rs=%.1f  PPM=%.1f  corr=%.1f  rzero=%.1f  r/r0=%0.1f", v_meas, rs, ppm, ppm_corr, RZERO, rs/RZERO);
            uart_puts(msg);
            uart_puts("\r\n");
            sprintf(msg2, "V=%.3f, dust %.3f", v_meas2,dust);
            uart_puts(msg2);
            uart_puts("\r\n");

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
    // WRITE YOUR CODE HERE
//     char msg[40];
//     char msg2[40];
//     uint16_t val = adc_read(0);

//     float v_meas = (5* (float)val) / 1023.0f;
//     // float R = (5-v_meas)/(v_meas/20000);

        
//     float rs = getResistance(5.0f, v_meas); //5.0f=Vcc

//     float ppm = getPPM(rs);
//     float ppm_corr = getCorrectedPPM(25.0f, 50.0f, rs); //default temp, edit
    
// uint16_t val2 = adc_read(1);

//     float v_meas2 = (5* (float)val2) / 1023.0f;


//     sprintf(msg, "V=%.3f  Rs=%.1f  PPM=%.1f  corr=%.1f  rzero=%.1f  r/r0=%0.1f", v_meas, rs, ppm, ppm_corr, RZERO, rs/RZERO);
//     uart_puts(msg);
//     uart_puts("\r\n");
//     sprintf(msg2, "V=%.3f ", v_meas2);
//     uart_puts(msg2);
//     uart_puts("\r\n");

    
}

/*
 * Function: Timer/Counter2 overflow interrupt
 * Purpose:  Measure GP2Y1010AU0F sensor.
 */

ISR(TIMER2_OVF_vect)
{
    // //citaci natvrdo nastavim hodnotu pro cas 280us - dodoelat
    // static uint8_t count = 0;
    // gpio_write_low(&PORTB, GP_LED_PIN);

    // if(count == 0)
    // {
    //     count = 1;
    // }
    // else
    // {
    //     count = 0;
    // }

    // if(count == 0)        // po 280us
    // {
    //     GP_read = adc_read(GP_ADC_CH);
    //     gpio_write_high(&PORTB, GP_LED_PIN);
    // }

    // //citaci natvrdo nastavim hodnotu pro cas 680us - dodoelat
 
    static uint8_t state = 0;

    if(state == 0){
            
            gpio_write_low(&PORTB, GP_LED_PIN);

            
            TCNT2 = 252;      
            state = 1;
    }
    else{
            // 280 us — změřit ADC
            GP_read = adc_read(GP_ADC_CH);

            gpio_write_high(&PORTB, GP_LED_PIN);
            TCNT2 = 118;      
            state = 0;
           
    
    }

}
