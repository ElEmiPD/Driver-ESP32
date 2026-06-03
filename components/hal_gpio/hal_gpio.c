// FileName:        hal_gpio.c                                                                                         
// Dependencies:    None                                                                                              
// Processor:       Tensilica Xtensa LX6 160 MHz                                                                                             
// Board:           ESP-WROOM-32                                                                                  
// Program version: 2.0                                                                                  
// Company:         Instituto Tecnologico de Chihuahua                                                                                 
// Description:     Definición de abstracciones sobre gpio_2026. No acceder
//                  a registros directamente. Funciones de inicialización, lectura y escritura GPIO.                               
// Autor:           Ana Paola Cardona Valenzuela
//                  Luis Adrian Anchondo Carreón
//                  Emiliano Perez Dyck 
// Updated:         03/06/2026

#include "hal_gpio.h"

void hal_led_init_p(uint8_t gpio_pin)
{
    GPIO_OUTPUT_P(gpio_pin);
    led_table[gpio_pin] = &gpio_table[gpio_pin];
}

void hal_led_init_n(uint8_t gpio_pin)
{
    GPIO_OUTPUT_N(gpio_pin);
}

void hal_led_on(uint8_t gpio_pin)
{
    WRITE_PIN(led_table[gpio_pin], 1);
}

void hal_led_off(uint8_t gpio_pin)
{
    WRITE_PIN(led_table[gpio_pin], 0);
}

void hal_led_toggle(uint8_t gpio_pin)
{
    bool current_state = READ_PIN(led_table[gpio_pin]);
    WRITE_PIN(led_table[gpio_pin], !current_state);
}

void hal_led_blink(uint8_t gpio_pin, uint32_t ms)
{
    hal_timer_init(HAL_TIMER_1);
    hal_led_toggle(gpio_pin);
    hal_timer_delay_ms(HAL_TIMER_1, ms);
    hal_led_toggle(gpio_pin);
}

void hal_button_init_pd(uint8_t gpio_pin)
{
    // Implementation for initializing button with pull-down resistor
    GPIO_INPUT_PD(gpio_pin);
}

void hal_button_init_pu(uint8_t gpio_pin)
{
    // Implementation for initializing button with pull-up resistor
    GPIO_INPUT_PU(gpio_pin);
}

void hal_button_init(uint8_t gpio_pin)
{
    // Implementation for initializing button
    GPIO_INPUT(gpio_pin);
}

bool hal_button_pressed(uint8_t gpio_pin)
{
    // Implementation for checking if button is pressed
    return READ_PIN(button_table[gpio_pin]); // Placeholder return value
}