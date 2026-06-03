// FileName:        hal_gpio.h                                                                                            
// Dependencies:    None                                                                                              
// Processor:       Tensilica Xtensa LX6 160 MHz                                                                                             
// Board:           ESP-WROOM-32                                                                                  
// Program version: 2.0                                                                                  
// Company:         Instituto Tecnologico de Chihuahua                                                                                 
// Description:     Abstracción sobre gpio_2026. No acceder
//                  a registros directamente. Funciones de inicialización, lectura y escritura GPIO.                               
// Autor:           Ana Paola Cardona Valenzuela
//                  Luis Adrian Anchondo Carreón
//                  Emiliano Perez Dyck 
// Updated:         03/06/2026

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "gpio_2026.h"
#include "hal_timer.h"

// Estructuras para abstraer LEDs y botones
// Estructura tentativa para mas atributos comunes a LEDs
/*
typedef struct
{
    gpio_pin_t *pin;
} hal_led_t;
*/
// Arreglo de punteros a estructuras gpio_pin_t para LEDs
extern gpio_pin_t *led_table[40];

// Estructura tentativa para mas atributos comunes a botones
/*
typedef struct
{
    gpio_pin_t *pin;
} hal_button_t;
*/
// Arreglo de punteros a estructuras gpio_pin_t para botones
extern gpio_pin_t *button_table[40];

// Prototipos de funciones

//Salidas
//Leds
void hal_led_init_p(uint8_t gpio_pin);
void hal_led_init_n(uint8_t gpio_pin);
void hal_led_on(uint8_t gpio_pin);
void hal_led_off(uint8_t gpio_pin);
void hal_led_toggle(uint8_t gpio_pin);
void hal_led_blink(uint8_t gpio_pin, uint32_t ms);

//Relay
/*
void hal_relay_init(hal_led_t *relay, uint8_t gpio_pin);
void hal_relay_on(hal_led_t *relay);
void hal_relay_off(hal_led_t *relay);
void hal_relay_toggle(hal_led_t *relay);
*/

//Entradas
void hal_button_init_pd(uint8_t gpio_pin);
void hal_button_init_pu(uint8_t gpio_pin);
void hal_button_init(uint8_t gpio_pin);
bool hal_button_pressed(uint8_t gpio_pin);

#endif /* HAL_GPIO_H */