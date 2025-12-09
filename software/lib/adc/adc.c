/**************************************************************************/
/*!
@file     ADC.c
@brief    Simple ADC driver for AVR (single-ended, 10-bit)
@author   P. Horsky
@license  MIT

Initialization and reading of the AVR ADC.

Uses AVcc as reference and enables ADC with prescaler 128.
*/
/**************************************************************************/

#include <avr/io.h>
#define GP_ADC_CH   1 /*!< Default ADC channel used after conversion for frequenter measurements */

/**************************************************************************/
/*!
@brief  Initialize the AVR ADC

Sets AVcc as reference and enables ADC with a prescaler of 128.

@return None
*/
/**************************************************************************/

void adc_init(void) {
    ADMUX = (1 << REFS0);                                                //AVcc reference
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);   //ADC enable, prescaler 128
    
}

/**************************************************************************/
/*!
@brief  Read value from ADC channel

Performs a single 10-bit conversion on the selected ADC input channel.

@param[in] channel  ADC channel number (0–7)

@return 10-bit ADC conversion result (0–1023)
*/
/**************************************************************************/

uint16_t adc_read(uint8_t channel) {
    
    channel &= 0x07;                    // Ensure only channels 0–7, mask higher bits
    ADMUX = (ADMUX & 0xF0) | channel;   // Select ADC channel

    ADCSRA |= (1 << ADSC);              // Start conversion
    while (ADCSRA & (1 << ADSC));       // Wait until conversion is complete

    ADMUX = (ADMUX & 0xF0) | GP_ADC_CH;  // Restore default channel

    return ADC;                          // Return ADC result
}
