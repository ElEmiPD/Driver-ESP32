// FileName:        hal_gpio.c                                                                                         
// Dependencies:    None                                                                                              
// Processor:       Tensilica Xtensa LX6 160 MHz                                                                                             
// Board:           ESP-WROOM-32                                                                                  
// Program version: 1.0                                                                                  
// Company:         Instituto Tecnologico de Chihuahua                                                                                 
// Description:     Definición de abstracciones sobre gpio_2026. No acceder
//                  a registros directamente. Funciones de inicialización, lectura y escritura GPIO.                               
// Autor:           Ana Paola Cardona Valenzuela
//                  Luis Adrian Anchondo Carreón
//                  Emiliano Perez Dyck 
// Updated:         02/06/2026

#include "hal_gpio.h"

void hal_led_init(hal_led_t *led, uint8_t gpio_pin)
{

}

void hal_led_on(hal_led_t *led)
{
    // Implementation for turning LED on
}

void hal_led_off(hal_led_t *led)
{
    // Implementation for turning LED off
}

void hal_led_toggle(hal_led_t *led)
{
    // Implementation for toggling LED
}

void hal_button_init(hal_button_t *button, uint8_t gpio_pin)
{
    // Implementation for initializing button
}

bool hal_button_pressed(hal_button_t *button)
{
    // Implementation for checking if button is pressed
    return false; // Placeholder return value
}