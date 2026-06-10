// FileName:        led_seq.c
// Dependencies:    bsp_panther48.h
// Processor:       Tensilica Xtensa LX6 160 MHz
// Board:           ESP32 ITCH (Panther48)  REV 1.0
// Program version: 1.0
// Company:         Instituto Tecnologico de Chihuahua
// Description:     Ejemplo de uso de la API semántica de LEDs individuales.
//                  Recorre los 5 LEDs de forma secuencial con un retardo
//                  de 150 ms entre cada uno, seguido de un destello
//                  simultáneo de todos (bsp_led_all_on / bsp_led_all_off).
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

    while (1)
    {
        for (uint8_t i = 1; i <= 5; i++)
        {
            bsp_led_on(i);
            bsp_delay_ms(150);
            bsp_led_off(i);
        }
        bsp_led_all_on();
        bsp_delay_ms(300);
        bsp_led_all_off();
        bsp_delay_ms(300);
    }
}