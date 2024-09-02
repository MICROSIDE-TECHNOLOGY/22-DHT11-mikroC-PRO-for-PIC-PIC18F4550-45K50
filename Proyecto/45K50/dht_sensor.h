#include <stdint.h> //Permite el uso de variables tipo uint8_t

/*******************************************************************************
Tipos de variables para la lectura del sensor
********************************************************************************/

typedef uint8_t dht11_pulse_counter_t;

static dht11_pulse_counter_t __dht11_high_time = 20;

/*******************************************************************************
Instrucciones que controlan el sensor
********************************************************************************/

/*******************************************************************************
 Funcion: dht11_send_start
 -----------------------
 Señal de inicio al sensor

 retorna: no parametros
********************************************************************************/
static inline void dht11_send_start()
{
   DHT11_OUT = 0;
   DHT11_DIR = 0;

   Delay_ms( 20 ); // Se?al de inicio

   DHT11_DIR = 1;

   Delay_us( 1 );
}

int dht11_init();
int dht11_read( float *temp, float *hum );

/*******************************************************************************
 Funcion: dht11_init
 -----------------------
 Configura I/O para el Bus de comunicación OW, verifica que el sensor se encuentre
 conectado y funcionando correctamente

 retorna: no parametros
********************************************************************************/
int dht11_init()
{
   static unsigned interrupt_status = 0;
   double tmp;
   dht11_pulse_counter_t pulse_lenght = 1;

   DHT11_OUT = 0;
   DHT11_DIR = 1;

   interrupt_status = GIE_bit;
   GIE_bit = 0;

   dht11_send_start();

   while ( DHT11_IN ) {
      if ( !pulse_lenght++ ) {
         GIE_bit = interrupt_status;
         return -1;
      }
   }
   pulse_lenght = 1;
   while ( !DHT11_IN ) {
      if ( !pulse_lenght++ ) {
         GIE_bit = interrupt_status;
         return -1;
      }
   }

   GIE_bit = interrupt_status;

   // La respuesta del sensor es de 80uS, dividimos la cantidad
   // de incrementos del contador entre este valor para estimar
   // 1uS
   tmp = pulse_lenght / 80.0;

   // 70uS = 1, 30 uS = 0, entonces pulsos > 50uS se consideran 1
   tmp = tmp * 20;

   __dht11_high_time = (dht11_pulse_counter_t)tmp;

   return 0;
}

/*******************************************************************************
 Funcion: dht11_read
 -----------------------
 Envia una petición de lectura al DHT11, interpreta la respuesta y convierte el valor
 en su representación con punto flotante.

 @param temp: Variable de temperatura
 @param hum: Variable de humedad

 retorna:
 @return 0: Operación exitosa
 @return -1: Tiempo de espera agotado
 @return -2: Error de suma de verificaci?n
********************************************************************************/
int dht11_read( float *temp, float *hum )
{
   unsigned interrupt_status = 0;

   uint8_t buffer[5] = { 0 };
   dht11_pulse_counter_t pulse_train[40] = { 0 };

   dht11_pulse_counter_t pulse_lenght = 1;

   int i = 0;
   int j = 0;

   // Deshabilita las interrupciones para calcular los intervalos de tiempo
   interrupt_status = GIE_bit;
   GIE_bit = 0;

   dht11_send_start();

   // Respuesta del sensor
   while ( DHT11_IN ) {
      if ( !pulse_lenght++ )
         goto timeout_error;
   }
   pulse_lenght = 1;
   while ( !DHT11_IN ) {
      if ( !pulse_lenght++ )
         goto timeout_error;
   }
   pulse_lenght = 1;
   while ( DHT11_IN ) {
      if ( !pulse_lenght++ )
         goto timeout_error;
   }
   pulse_lenght = 1;
   while ( !DHT11_IN ) {
      if ( !pulse_lenght++ )
         goto timeout_error;
   }

   // Capturar el tren de pulsos
   for ( i = 0; i < 40; i++ ) {
      pulse_train[i] = 1;
      pulse_lenght = 1;
      while ( DHT11_IN ) {
         if ( !pulse_train[i]++ )
            goto timeout_error;
      }
      while ( !DHT11_IN ) {
         if ( !pulse_lenght++ )
            goto timeout_error;
      }
   }

   // Decodificar el tren de pulsos
   for ( i = 0; i < 40; i += 8 ) {
      for ( j = 0; j < 8; j++ ) {
         if ( pulse_train[i + j] > __dht11_high_time ) {
            buffer[i / 8] |= 1 << ( 7 - j );
         }
      }
   }

   // Restablecer las interrupciones
   GIE_bit = interrupt_status;

   *hum = (float)( buffer[0] + ( buffer[1] * 0.1 ) );
   *temp = (float)( buffer[2] + ( buffer[3] * 0.1 ) );

   if ( buffer[4] != ( buffer[0] + buffer[1] + buffer[2] + buffer[3] ) )
      return buffer[4];

   return 0;

timeout_error:
   GIE_bit = interrupt_status;
   return -1;
}