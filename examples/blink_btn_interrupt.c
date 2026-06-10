// FileName:        blink_btn_interrupt.c
// Dependencies:    bsp_panther48.h
// Processor:       Tensilica Xtensa LX6 160 MHz
// Board:           ESP32 ITCH (Panther48)  REV 1.0
// Program version: 1.0
// Company:         Instituto Tecnologico de Chihuahua
// Description:     Ejemplo de interrupción por flanco en SW1.
//                  Cada pulsación incrementa un contador (1–5) en la ISR.
//                  El loop principal detecta cambios en el contador y
//                  enciende secuencialmente tantos LEDs como indica su valor
//                  usando bsp_led_blink(). La ISR se declara con IRAM_ATTR
//                  conforme a los requisitos del ESP-IDF.
//
// Autores:         Ana Paola Cardona Valenzuela
//                  Emiliano Perez Dyck
//                  Luis Adrian Anchondo Carreón
// Created:         09/06/2026
// Updated:         09/06/2026

#include "bsp_panther48.h"

static volatile uint8_t contador = 0;

static void IRAM_ATTR isr_sw1(void *arg)
{
    contador++;
    if (contador > 5) contador = 1;
}

void app_main(void)
{
    bsp_init();
    bsp_sw1_irq_attach(isr_sw1, NULL);

    uint8_t ultimo = 0;

    while (1)
    {
        if (contador != ultimo)
        {
            ultimo = contador;
            bsp_led_all_off();

            for (uint8_t i = 1; i <= ultimo; i++)
                bsp_led_blink(i, 200);

            bsp_delay_ms(100);
        }
        bsp_delay_ms(10);
    }
}