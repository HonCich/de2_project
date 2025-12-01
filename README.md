# Indoor air quality monitoring system  

## Zadání projektu  
Cílem je vytvořit měřicí zařízení, které monitoruje vlastnosti vzduchu pomocí několika senzorů...

---


## Měřené parametry  

- Teplota  
- Relativní vlhkost  
- Indikace kvality vzduchu (plynné složky)  
- Prachové částice PM (např. PM2.5 / PM10)  



## Hardwarové komponenty  

| Komponenta | Funkce | 
|------------|--------|
| **ATmega328 / Arduino Uno** | hlavní MCU |
| **DHT12** | měření teploty a vlhkosti |
| **MG135** | indikátor kvality vzduchu / CO₂ |
| **GP2Y10** | optický senzor prachu | 
| **OLED displej (I2C)** | zobrazování dat | 

---

## Popis senzorů  

### DHT12  
Digitální senzor poskytující údaje o teplotě a vlhkosti pomocí sb2rnice I2C...  

### MG135  
Analogový senzor pro měření kvality vzduchu...

### GP2Y10  


### OLED displej  
 
