// FileName:        rgb_pwm.c
// Dependencies:    bsp_panther48.h
// Processor:       Tensilica Xtensa LX6 160 MHz
// Board:           ESP32 ITCH (Panther48)  REV 1.0
// Program version: 1.0
// Company:         Instituto Tecnologico de Chihuahua
// Description:     Ejemplo de control PWM del LED RGB con API de porcentaje.
//                  Ejecuta un ciclo continuo de transiciones suaves:
//                  azul creciente, cruce azul->verde, cruce verde->rojo,
//                  seguido de un apagado total. Usa bsp_pwm_rgb_set_percent()
//                  con corrección de lógica activa baja gestionada por el BSP.
//
// Autores:         Ana Paola Cardona Valenzuela
//                  Emiliano Perez Dyck
//                  Luis Adrian Anchondo Carreón
// Created:         09/06/2026
// Updated:         09/06/2026

#include "bsp_panther48.h"

void app_main(void)
{
    bsp_init();
    bsp_pwm_rgb_init();

    while (1)
    {
        for (uint8_t b = 0; b <= 100; b += 5)
        {
            bsp_pwm_rgb_set_percent(0, 0, b);
            bsp_delay_ms(30);
        }
        for (uint8_t g = 0; g <= 100; g += 5)
        {
            bsp_pwm_rgb_set_percent(0, g, 100 - g);
            bsp_delay_ms(30);
        }
        for (uint8_t r = 0; r <= 100; r += 5)
        {
            bsp_pwm_rgb_set_percent(r, 100 - r, 0);
            bsp_delay_ms(30);
        }
        bsp_pwm_rgb_off();
        bsp_delay_ms(300);
    }
}