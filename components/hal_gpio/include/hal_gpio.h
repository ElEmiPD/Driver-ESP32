// FileName:        hal_gpio.h                                                                                            
// Dependencies:    None                                                                                              
// Processor:       Tensilica Xtensa LX6 160 MHz                                                                                             
// Board:           ESP-WROOM-32                                                                                  
// Program version: 1.0                                                                                  
// Company:         Instituto Tecnologico de Chihuahua                                                                                 
// Description:     Abstracción sobre gpio_2026. No acceder
//                  a registros directamente. Funciones de inicialización, lectura y escritura GPIO.                               
// Autor:           Ana Paola Cardona Valenzuela
//                  Luis Adrian Anchondo Carreón
//                  Emiliano Perez Dyck 
// Updated:         02/06/2026

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "gpio_2026.h"

// Estructuras para abstraer LEDs y botones
typedef struct
{
    gpio_pin_t *pin;
} hal_led_t;

typedef struct
{
    gpio_pin_t *pin;
} hal_button_t;

// Prototipos de funciones

//Salidas
//Leds
void hal_led_init(hal_led_t *led, uint8_t gpio_pin);
void hal_led_on(hal_led_t *led);
void hal_led_off(hal_led_t *led);
void hal_led_toggle(hal_led_t *led);

//Relay
/*
void hal_relay_init(hal_led_t *relay, uint8_t gpio_pin);
void hal_relay_on(hal_led_t *relay);
void hal_relay_off(hal_led_t *relay);
void hal_relay_toggle(hal_led_t *relay);
*/

//Entradas
void hal_button_init(hal_button_t *button, uint8_t gpio_pin);
bool hal_button_pressed(hal_button_t *button);

#endif /* HAL_GPIO_H */