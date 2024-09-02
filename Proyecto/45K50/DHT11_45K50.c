/*******************************************************************************
Company:
Microside Technology Inc.

File Name:
LCD.mcppi

Product Revision  :  1
Device            :  X-TRAINER
Driver Version    :  1.0
********************************************************************************/

/*******************************************************************************
Para usar el código con bootloader, configurar como lo indica MICROSIDE:
1) Tools> Options> Output> Output Settings> Marcar "Long HEX format"
2) Project> Edit Project> Insertar 48.000000 en "MCU Clock Frequency [MHz]" box
********************************************************************************/

#define DHT11_DIR TRISB2_bit
#define DHT11_IN RB2_bit
#define DHT11_OUT LATB.f2

#pragma orgall 0x1FFF         // Espacio reservado para bootloader
#include "dht_sensor.h"
#include "lcd.h"

float temperature = 0;
float humidity = 0;
int dht11_stat = 0;

void main() org 0x2000
{
   ANSELB = 0;                // Configura el puerto B como pines digitales
   TRISB1_bit = 0;            // Configura el puerto B1 como salida
   LATB.f1 = 0;               // Escribimos un 0 para definir el LCD como escritura
   C1ON_bit = 0;              // Deshabilita los comparadores

   C2ON_bit = 0;

   I2C1_Init( 100000 );       // Inicializa el BUS I2C

   Delay_ms( 2000 );

   lcd_i2c_init( 0x27 );      // Inicializa la pantalla LCD

   lcd_i2c_clear();
   lcd_i2c_setCursor( 3, 0 );
   lcd_i2c_write_str( "DHT11" );
   lcd_i2c_setCursor( 3, 1 );

   if ( dht11_init() != 0 ) { // Inicializa el DHT11
      lcd_i2c_write_str( "ERROR" );
      while ( 1 ) {
      }
   }

   lcd_i2c_write_str( "OK" );
   Delay_ms( 1000 );

   while ( 1 ) {
      Delay_ms( 2000 );
      // Lee el sensor DHT11
      dht11_stat = dht11_read( &temperature, &humidity );
      // Cualquier valor diferente a 0 significa error en la lectura
      if ( dht11_stat != 0 ) {
         lcd_i2c_clear();
         lcd_i2c_setCursor( 0, 0 );
         lcd_i2c_write_str( "DHT11 Error" );
         continue;
      }
      lcd_i2c_clear();
      lcd_i2c_setCursor( 0, 0 );
      lcd_i2c_write_str( "T: " );
      lcd_i2c_write_double( temperature, 2 );
      lcd_i2c_write_str( " C" );
      lcd_i2c_setCursor( 0, 1 );
      lcd_i2c_write_str( "H: " );
      lcd_i2c_write_double( humidity, 2 );
      lcd_i2c_write_str( " %" );
   }
}