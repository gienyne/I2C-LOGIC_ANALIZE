/*
 * micro9.c
 *
 * Created: 15.01.2025 14:34:06
 * Author : Ntofeu Nyatcha dimitry
 */ 

#define F_CPU 4000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include "I2C_LCD.h"
#include <util/delay.h>
#include <stdio.h>

#define TCS34725_ADDRESS (0x29) // I2C-Adresse des TCS34725-Sensors
#define ENABLE_REGISTER (0x00) // Registeradresse zum Aktivieren des Sensors
#define ATIME_REGISTER (0x01)  // Registeradresse für die Integrationszeit
//#define CONTROL_REGISTER (0x0F)
#define WTIME_REGISTER (0x03) // Registeradresse für die Wartezeit
#define CMD_BIT (0x80) // Command-Bit zur Aktivierung des Zugriffs auf ein Register

// Variablen zum Speichern der RGBC-Werte
uint16_t red, green, blue, clear = 0;
uint8_t data[8]; // Puffer für die RGBC-Daten

// Funktion zur Konfiguration des TCS34725-Sensors
void I2C_einstellung(){
	
	uint8_t enable_cmd[] = {CMD_BIT | ENABLE_REGISTER, 0x03}; // PON und AEN im Register Enable aktivieren
    i2c_write(TCS34725_ADDRESS, enable_cmd, 2); // Befehl senden
	_delay_ms(10);
	uint8_t atime_cmd[] = {CMD_BIT | ATIME_REGISTER, 0xD5}; // Integrationszeit (circa 101ms)
	i2c_write(TCS34725_ADDRESS, atime_cmd, 2);	
	_delay_ms(10);
	
	//uint8_t control_cmd[] = {CMD_BIT | CONTROL_REGISTER, 0x10}; // gain konfigurieren (*16) für mehr empfindlichkeit
	//i2c_write(TCS34725_ADDRESS, control_cmd, 2);
	
	uint8_t wtime_cmd[] = {CMD_BIT | WTIME_REGISTER, 0xFF};
	i2c_write(TCS34725_ADDRESS, wtime_cmd, 2); 
	_delay_ms(10);
	
}

uint16_t fusion_low_und_high_byte(uint8_t low, uint8_t high){
	return (uint16_t)(low | (high << 8));
}

void TCS34725_read_RGBC_werte(){
	uint8_t start_adrr = (CMD_BIT | 0x14);  // Startadresse für die RGBC-Daten nämlich ab CDATA bis BDATAH
	i2c_write(TCS34725_ADDRESS, &start_adrr, 1); // Leseoperation starten
	_delay_ms(10);
	i2c_read(TCS34725_ADDRESS, data, 8);  // 8 Bytes lesen (je 2 Bytes pro Wert)
	_delay_ms(10);
	
	 // RGBC-Werte aus dem Datenpuffer extrahieren
	clear = fusion_low_und_high_byte(data[0], data[1]);
	red   = fusion_low_und_high_byte(data[2], data[3]);
	green = fusion_low_und_high_byte(data[4], data[5]);
	blue  = fusion_low_und_high_byte(data[6], data[7]);
}

void formatage(const char* color_name, uint16_t value) {
	char buffer[16];
	snprintf(buffer, sizeof(buffer), "%s:%u", color_name, value);
	lcd_putString(buffer);
}

void affiche_color(){
	
	lcd_clear();
	
	lcd_moveCursor(0, 0);
	formatage("R", red);
	lcd_moveCursor(9, 0);
	formatage("G", green);
	lcd_moveCursor(0, 1);
	formatage("B", blue);
	lcd_moveCursor(9, 1);
	formatage("C", clear);
	
}

int main(void)
{
    lcd_init();
	lcd_enable(1);
	i2c_init();
	I2C_einstellung();
	
    while (1) 
    {
		TCS34725_read_RGBC_werte();
		affiche_color();
		_delay_ms(500);
    }
}

