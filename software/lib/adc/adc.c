#include <avr/io.h>
#define GP_ADC_CH   1 

void adc_init(void) {
    ADMUX = (1 << REFS0);   //AVcc
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); //ADC enable, 128
    
}

uint16_t adc_read(uint8_t channel) {
    
    channel &= 0x07;
    ADMUX = (ADMUX & 0xF0) | channel;   //spodni 4 kanaly

    ADCSRA |= (1 << ADSC);  //spusteni
    while (ADCSRA & (1 << ADSC));

ADMUX = (ADMUX & 0xF0) | GP_ADC_CH;   //spodni 4 kanaly

    return ADC;
}
