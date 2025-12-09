/**************************************************************************/
/*!
@file     ADC.h
@brief    Header for AVR ADC driver
*/
/**************************************************************************/

#ifndef ADC_H
#define ADC_H

#include <stdint.h>

/**************************************************************************/
/*!
@brief  Initialize the AVR ADC

Sets:

- AVcc as reference voltage  
- ADC enabled  
- Prescaler = 128  

@return None
*/
/**************************************************************************/

void adc_init(void);

/**************************************************************************/
/*!
@brief  Read value from ADC channel

@param[in] channel  ADC channel (0–7)
@return 10-bit ADC result (0–1023)
*/
/**************************************************************************/

uint16_t adc_read(uint8_t channel);

#endif
