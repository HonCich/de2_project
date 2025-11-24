#include <avr/io.h>

void adc_init(void) {
    ADMUX = (1 << REFS0);   //AVcc
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); 
}

uint16_t adc_read(void) {
    ADMUX = (ADMUX & 0xF0);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));
    return ADC;
}
