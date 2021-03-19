// Librerias
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include <math.h>
#include <string.h>

// Definiciones
#define ADC0 0
#define ADC1 1
#define ADC2 2
#define ADC3 3
#define ADC4 4
#define ADC5 5

// Estructuras y uniones
struct ring_buffer
{
	uint8_t datos[256];
	
	uint8_t indice_lectura;
	uint8_t indice_escritura;
}typedef ring_buffer_t;

struct timer
{
	unsigned long us;
	unsigned long ms;
	
	bool enabled;
}typedef timer_t;

struct servo
{
	timer_t t_alto;
	timer_t t_bajo;
	
	uint16_t grados;
}typedef servo_t;

union datos
{
	uint8_t u8[4];
	uint16_t u16[2];
	uint32_t u32;
	
	int8_t i8[4];
	int16_t i16[2];
	int32_t i32;
}typedef datos_u;

struct serial_manager
{
	timer_t refresh;
	timer_t time_out;
	
	ring_buffer buffer_entrada;
	ring_buffer buffer_salida;
	
	uint8_t read_state;
	uint8_t payload_init;
	uint8_t payload_lengh;
	
	datos_u conversor;
}typedef serial_manager_t;

struct tunel_de_viento
{
	double gradosClapeta;
	double tempSalida;
	double tempSalidaSet;
	double caudalEntrada;
}typedef tunel_de_viento_t;

struct adc
{
	uint16_t tempSalida_value;
	uint16_t caudalEntrada_value;
	
	uint8_t indice_lectura;
}typedef adc_t;

// Funciones
void mcu_init();	// Funcion de inicio

void serial_transmit();	// Funcion para enviar lo almacenado en el buffer por el puerto serie
void serial_write(uint8_t cmd, uint8_t *payload, uint8_t lengh);	// Funcion para enviar una secuencia de datos

uint8_t checksum(uint8_t *data, uint8_t lengh);	// Funcion para calcular el checksum de un paquete de datos

// Variables
volatile servo_t clapeta;
volatile serial_manager_t serial;
volatile timer_t led;
volatile tunel_de_viento_t tunel;
volatile adc_t adc_manager;
volatile timer_t system_timer;
volatile timer_t clapeta_timer;

// Funcion principal del programa
int main()
{
	// Configuracion inicial del programa
	mcu_init();
	
	// Bucle principal
    while (1) 
    {
		// Reinicio del wachdog
		if (led.ms >= 500)
		{
			wdt_reset();	// Reincia el timer del wachdog
			
			PORTB ^= (1 << PORTB3);	// Blinkea el led de estado
			
			led.ms = 0;
		}
		
		// Control de tiempos de ejecucion
		if (serial.refresh.ms >= 100)
		{
			// Si hay algo para enviar por el puerto se envia
			serial_transmit();

			// Se reestablece el tiempo de refresco
			serial.refresh.ms = 0;
		}
		
		// Si hay algo para leer en el buffer se lee
		if (serial.buffer_entrada.indice_lectura != serial.buffer_entrada.indice_escritura)
		{
			switch (serial.read_state)
			{
				// Inicio de la cabecera
				case 0:
					if (serial.buffer_entrada.datos[serial.buffer_entrada.indice_lectura] == 'P')
					{
						serial.time_out.enabled = true;
						
						serial.read_state = 1;
					}
				
					break;
				
				case 1:
					if (serial.buffer_entrada.datos[serial.buffer_entrada.indice_lectura] == 'D')
					{
						serial.read_state = 2;
					}
				
					else
					{
						serial.read_state = 0;
					}
				
					break;
				
				case 2:
					if (serial.buffer_entrada.datos[serial.buffer_entrada.indice_lectura] == 'C')
					{
						serial.read_state = 3;
					}
				
					else
					{
						serial.read_state = 0;
					}
				
					break;
				
				case 3:
					if (serial.buffer_entrada.datos[serial.buffer_entrada.indice_lectura] == 'C')
					{
						serial.read_state = 4;
					}
				
					else
					{
						serial.read_state = 0;
					}
				
					break;
				
				case 4:
					serial.payload_lengh = serial.buffer_entrada.datos[serial.buffer_entrada.indice_lectura];
				
					serial.read_state = 5;
				
					break;
				
				case 5:
					if (serial.buffer_entrada.datos[serial.buffer_entrada.indice_lectura] == ':')
					{
						serial.read_state = 6;
					}
				
					else
					{
						serial.read_state = 0;
					}
				
					break;
				
				// Inicio de la parte de comando y control
				case 6:
					serial.payload_init = serial.buffer_entrada.indice_lectura + 1;
				
					serial.read_state = 7;
				
					break;
				
				case 7:
					// Si se terminaron de recibir todos los datos
					if (serial.buffer_entrada.indice_lectura == (serial.payload_init + serial.payload_lengh))
					{
						// Se comprueba la integridad de datos
						if (checksum((uint8_t *)(&serial.buffer_entrada.datos[serial.payload_init - 7]), serial.payload_lengh + 7)
							== serial.buffer_entrada.datos[serial.buffer_entrada.indice_lectura])
						{
							// Analisis del comando recibido
							switch (serial.buffer_entrada.datos[serial.payload_init - 1])
							{
								case 0x00:	// Pos clapeta
									serial.conversor.u16[0] = (uint16_t)(tunel.gradosClapeta / 180.0 * 1000.0);
								
									serial_write(0x00, (uint8_t *)(&serial.conversor.u8[0]), 2);
								
									break;
									
								case 0x01:
									serial.conversor.u8[0] = serial.buffer_entrada.datos[serial.payload_init];
									serial.conversor.u8[1] = serial.buffer_entrada.datos[serial.payload_init + 1];
									
									tunel.tempSalidaSet = (double)(serial.conversor.i16[0] / 10.0);
									
									serial_write(0x01, (uint8_t *)(&serial.conversor.u8[0]), 2);
								
									break;
									
								case 0x02:	// Temp salida
									serial.conversor.i16[0] = (int16_t)(tunel.tempSalida * 10.0);
									
									serial_write(0x02, (uint8_t *)(&serial.conversor.u8[0]), 2);
								
									break;
									
								case 0x03:	// Caudal entrada
									serial.conversor.i16[0] = (int16_t)(tunel.caudalEntrada * 10.0);
									
									serial_write(0x03, (uint8_t *)(&serial.conversor.u8[0]), 2);
									
									break;
									
								case 0x04:
									if (!clapeta_timer.enabled)
									{
										serial.conversor.u8[0] = serial.buffer_entrada.datos[serial.payload_init];
										serial.conversor.u8[1] = serial.buffer_entrada.datos[serial.payload_init + 1];
										
										clapeta.grados = serial.conversor.u16[0];
									}
									
									serial.conversor.u16[0] = clapeta.grados;
									
									serial_write(0x04, (uint8_t *)(&serial.conversor.u8[0]), 2);
									
									break;
									
								case 0x05:
									if (serial.buffer_entrada.datos[serial.payload_init] == 0xFF)
										clapeta_timer.enabled = true;
									
									else if (serial.buffer_entrada.datos[serial.payload_init] == 0x00)
										clapeta_timer.enabled = false;
									
									serial_write(0x05, (uint8_t *)(&serial.buffer_entrada.datos[serial.payload_init]), 1);
								
									break;
								
								case 0xFE:	// ACK
									serial_write(0xFE, NULL, 0);
							
									break;
							}
						}
					
						// Corrupcion de datos recibidos
						else
						{
							serial_write(0xFF, NULL, 0);
						}
					
						serial.time_out.enabled = false;
						serial.time_out.ms = 0;
						
						serial.read_state = 0;
					}
				
					break;
			}
			
			serial.buffer_entrada.indice_lectura++;
		}
		
		// Si se supero el tiempo de espera de los datos
		if (serial.time_out.ms >= 500)
		{
			serial.time_out.enabled = false;
			serial.time_out.ms = 0;
			
			serial.buffer_entrada.indice_lectura = serial.buffer_entrada.indice_escritura;
			
			serial.read_state = 0;
		}
		
		// Tiempo para procesar las variables de operacion
		if (system_timer.ms >= 100)
		{
			// Calculo de las variables del ADC
			tunel.gradosClapeta = clapeta.grados * 180.0 / 1000.0;
			tunel.tempSalida = 21.186 * (adc_manager.tempSalida_value / 1023.0 * 5.0) - 4.66;
			tunel.caudalEntrada = (adc_manager.caudalEntrada_value / 1023.0 * 5.0) * 20.0;
			
			// Disparo del ADC
			ADCSRA |= (1 << ADSC);
			
			system_timer.ms = 0;
		}
		
		// Control de la temperatura de salida
		if (clapeta_timer.ms >= 10000)
		{
			if ((tunel.tempSalida > tunel.tempSalidaSet) && (clapeta.grados <= 450))
			{
				clapeta.grados += 50;
			}
			
			else if ((tunel.tempSalida < tunel.tempSalidaSet) && (clapeta.grados >= 50))
			{
				clapeta.grados -= 50;
			}
			
			clapeta_timer.ms = 0;
		}
    }
}

// Funcion de inicio
void mcu_init()
{
	// Configuracion de los pines
	DDRB |= (1 << DDB5) | (1 << DDB3) | (1 << DDB0);
	
	PORTB &= ~(1 << PORTB5) & ~(1 << PORTB3) & ~(1 << PORTB0);
	
	DDRC &= ~(1 << DDC0) & ~(1 << DDC1);
	
	PORTC &= ~(1 << PORTC0) & ~(1 << PORTC1);
	
	// Configuracion de los perifericos
	cli();
	
	// Configuracion del wachdog
	wdt_reset();
	
	WDTCSR |= (1 << WDCE) | (1 << WDE);	// Se habilita el cambio de prescaler
	WDTCSR = (1 << WDE) | (1 << WDP3);
	
	// Configuracion del timer para el control de servos
	OCR0A = 49;	// Para lograr un desborde de 25us
	
	TCCR0A = 0b00000010;	// Modo CTC
	TCCR0B = 0b10000010;	// Prescaler de 8, comparacion con el registro A
	
	TIMSK0 = 0b00000010;	// Interrupcion por desborde
	
	// Configuracion del timer para uso general no prioritario
	OCR1A = 250;	// Para lograr un desborde de 1ms
	
	TCCR1B = 0b00001011;	// Modo CTC con prescaler de 64
	TCCR1C = 0b10000000;	// Comparador con OCR1A
	
	TIMSK1 = 0b00000010;	// Interrupcion por desborde
	
	// Configuracion de puerto serie
	UCSR0B = 0b11011000;	// Activado el puerto con interrupcion de recepcion y transmision
	UCSR0C = 0b00000110;	// 8 bits
	
	UBRR0 = 51;	// 19200 baudios
	
	// Configuracion del ADC
	ADMUX = 0b01000000;	// Referencia a 5V justificacion a la derecha
	ADCSRA = 0b10001111;	// Prescaler de 128 e interrupcion
	ADCSRB = 0b00000000;	// Modo free runing, cuando se termine una conversion se iniciara la proxima
	DIDR0 = 0b00111111;	// Se desactiva el pin digital de los adc
	
	sei();
	
	// Configuracion de dispositivos
	// Configuracion de la clapeta
	clapeta.t_alto.us = 0;
	clapeta.t_alto.enabled = true;
	
	clapeta.t_bajo.us = 0;
	clapeta.t_bajo.enabled = false;
	
	clapeta.grados = 250;
	
	// Variables del tunel
	tunel.gradosClapeta = clapeta.grados * 180.0 / 1000.0;
	tunel.tempSalida = 40.0;
	tunel.tempSalidaSet = 40.0;
	tunel.caudalEntrada = 50.0;
	
	// Configuracion del timer para manejar eventos del puerto serie
	serial.refresh.ms = 0;
	serial.refresh.enabled = true;
	
	serial.time_out.ms = 0;
	serial.time_out.enabled = false;
	
	// Configuracion de los buffers
	serial.buffer_entrada.indice_lectura = 0;
	serial.buffer_entrada.indice_escritura = 0;
	
	serial.buffer_salida.indice_lectura = 0;
	serial.buffer_salida.indice_escritura = 0;
	
	serial.read_state = 0;
	
	// Configuracion del led de estado
	led.ms = 0;
	led.enabled = true;
	
	// Configuracion del adc
	adc_manager.indice_lectura = ADC0;
	
	// Configuracion del timer del sistema de control
	system_timer.ms = 0;
	system_timer.enabled = true;
	
	// Timer de respuesta del la clapeta
	clapeta_timer.ms = 0;
	clapeta_timer.enabled = false;
}

// Funcion para enviar lo que hay en el buffer por el puerto serie
void serial_transmit()
{
	if ((serial.buffer_salida.indice_lectura != serial.buffer_salida.indice_escritura) && (UCSR0A & (1 << UDRE0)))
	{
		UDR0 = serial.buffer_salida.datos[serial.buffer_salida.indice_lectura];
		serial.buffer_salida.indice_lectura++;
	}
}

// Funcion para enviar datos
void serial_write(uint8_t cmd, uint8_t *payload, uint8_t lengh)
{	
	// Cabecera
	serial.buffer_salida.datos[serial.buffer_salida.indice_escritura] = 'P';
	serial.buffer_salida.indice_escritura++;
	serial.buffer_salida.datos[serial.buffer_salida.indice_escritura] = 'D';
	serial.buffer_salida.indice_escritura++;
	serial.buffer_salida.datos[serial.buffer_salida.indice_escritura] = 'C';
	serial.buffer_salida.indice_escritura++;
	serial.buffer_salida.datos[serial.buffer_salida.indice_escritura] = 'C';
	serial.buffer_salida.indice_escritura++;
	serial.buffer_salida.datos[serial.buffer_salida.indice_escritura] = lengh;
	serial.buffer_salida.indice_escritura++;
	serial.buffer_salida.datos[serial.buffer_salida.indice_escritura] = ':';
	serial.buffer_salida.indice_escritura++;
	
	// Datos
	serial.buffer_salida.datos[serial.buffer_salida.indice_escritura] = cmd;
	serial.buffer_salida.indice_escritura++;
	
	for (uint8_t i = 0 ; i < lengh ; i++)
	{
		serial.buffer_salida.datos[serial.buffer_salida.indice_escritura] = payload[i];
		serial.buffer_salida.indice_escritura++;
	}
	
	serial.buffer_salida.datos[serial.buffer_salida.indice_escritura] = 
		checksum((uint8_t *)(&serial.buffer_salida.datos[serial.buffer_salida.indice_escritura - lengh - 7]), lengh + 7);
	serial.buffer_salida.indice_escritura++;
}

// Funcion para calcular el checksum de un paquete de datos
uint8_t checksum(uint8_t *data, uint8_t lengh)
{
	uint8_t suma = 0x00;
	
	for (uint8_t i = 0 ; i < lengh ; i++)
	{
		suma ^= data[i];
	}
	
	return suma;
}

// Interrupcion cada 25us para el control presiso de los servomotores
ISR(TIMER0_COMPA_vect)
{
	// Aumento de tiempo de la clapeta
	if (clapeta.t_alto.enabled)
	{
		clapeta.t_alto.us += 25;
		
		if (clapeta.t_alto.us >= 20000)
		{
			PORTB |= (1 << PORTB0);
			
			clapeta.t_alto.us = 0;
			
			clapeta.t_bajo.enabled = true;
		}
	}
	
	if (clapeta.t_bajo.enabled)
	{
		clapeta.t_bajo.us += 25;
		
		if ((clapeta.t_bajo.us) >= (clapeta.grados + 1000))
		{
			PORTB &= ~(1 << PORTB0);
			
			clapeta.t_bajo.us = 0;
			
			clapeta.t_bajo.enabled = false;
		}
	}
}

// Interrupcion cada 1ms para el control de los timers
ISR(TIMER1_COMPA_vect)
{
	// Aumento de tiempo del timer serial manager
	if (serial.refresh.enabled)
		serial.refresh.ms++;
	
	if (serial.time_out.enabled)
		serial.time_out.ms++;
	
	// Aumento de tiempo del led de estado
	if (led.enabled)
		led.ms++;
	
	// Aunmento de tiempo del timer de control del sistema
	if (system_timer.enabled)
		system_timer.ms++;
		
	// Aunmento de tiempo del timer de control de la clapeta
	if (clapeta_timer.enabled)
		clapeta_timer.ms++;
}

// Interrupcion cuando se recibe un dato
ISR(USART_RX_vect)
{
	serial.buffer_entrada.datos[serial.buffer_entrada.indice_escritura] = UDR0;
	serial.buffer_entrada.indice_escritura++;
}

// Interrupcion cuando se termina de enviar un dato
ISR(USART_TX_vect)
{
	serial_transmit();
}

// Interrupcion de conversion completa
ISR(ADC_vect)
{
	switch (adc_manager.indice_lectura)
	{
		case ADC0:
			adc_manager.tempSalida_value = ADC;
			
			break;
		
		case ADC1:
			adc_manager.caudalEntrada_value = ADC;
			
			break;
	}
	
	ADMUX &= ~(1 << MUX3) & ~(1 << MUX2) & ~(1 << MUX1) & ~(1 << MUX0);
	
	adc_manager.indice_lectura++;
	
	if (adc_manager.indice_lectura > 1)
	{
		adc_manager.indice_lectura = 0;
	}
	
	ADMUX |= adc_manager.indice_lectura;
}