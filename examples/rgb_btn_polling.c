// FileName:        rgb_btn_polling.c
// Dependencies:    bsp_panther48.h
// Processor:       Tensilica Xtensa LX6 160 MHz
// Board:           ESP32 ITCH (Panther48)  REV 1.0
// Program version: 1.0
// Company:         Instituto Tecnologico de Chihuahua
// Description:     Ejemplo de polling de botones con respuesta en LED y RGB.
//                  SW1 enciende LED1 y el RGB en rojo; SW2 enciende LED2
//                  y el RGB en cian. Al soltar ambos botones, todos los
//                  periféricos se apagan. Retardo de 20 ms como debounce
//                  por software.
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
        if (bsp_sw1_pressed())
        {
            bsp_led_on(BSP_LED1);
            bsp_rgb_set(BSP_COLOR_RED);
        }
        else if (bsp_sw2_pressed())
        {
            bsp_led_on(BSP_LED2);
            bsp_rgb_set(BSP_COLOR_CYAN);
        }
        else
        {
            bsp_led_all_off();
            bsp_rgb_off();
        }

        bsp_delay_ms(20);
    }
}