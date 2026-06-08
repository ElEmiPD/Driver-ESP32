// FileName:        main.c
// Dependencies:    bsp_panther48.h
// Processor:       Tensilica Xtensa LX6 160 MHz
// Board:           ESP32 ITCH (Panther48)  REV 1.0
// Program version: 1.0
// Company:         Instituto Tecnologico de Chihuahua
// Description:     Demo de capacidades de la placa.
//                  Navegacion con SW1 y SW2 de la placa.
//
//                  SW1 (IO18)  avanza en el menu
//                  SW2 (IO19)  entra / sale
//
//                  Menu principal:
//                    [0] LEDs     parpadeo secuencial LED1-5
//                    [1] RGB      colores fijos y breath
//                    [2] Rotas    patrones en LEDs 1-4
//
// Autores:         Ana Paola Cardona Valenzuela
//                  Emiliano Perez Dyck
//                  Luis Adrian Anchondo Carreón
// Created:         08/06/2026

#include "bsp_panther48.h"
#include <stdio.h>

// ===========================================================================
//  SW1 por interrupcion con debounce por timestamp
// ===========================================================================
static volatile bool     sw1_flag    = false;  // la ISR avisa al loop
static volatile uint64_t sw1_last_ms = 0;      // para el debounce

#define SW1_DEBOUNCE_MS 200

typedef enum { 
    ROTA_IDA, 
    ROTA_PINGPONG, 
    ROTA_2EN2, 
    ROTA_COUNT 
} rota_t;

static void IRAM_ATTR sw1_isr(void *arg)
{
    uint64_t ahora = bsp_get_ms();
    if ((ahora - sw1_last_ms) >= SW1_DEBOUNCE_MS)
    {
        sw1_last_ms = ahora;
        sw1_flag = true;
    }
}

// Consume el flag  retorna true una sola vez por pulsacion
static bool sw1_pulsado(void)
{
    if (sw1_flag) 
    { 
        sw1_flag = false; 
        return true; 
    }
    return false;
}

//  SW2 por polling con debounce simple
// ===========================================================================
static bool sw2_pulsado(void)
{
    static bool prev = false;
    bool actual = bsp_sw2_pressed();
    if (actual && !prev) 
    { 
        bsp_delay_ms(30); 
        prev = true;  
        return bsp_sw2_pressed(); 
    }

    if (!actual)           
    {
        prev = false;
    }

    return false;
}

// ===========================================================================
//  Secuencia de arranque
//  Enciende cada LED uno a uno, luego muestra colores en el RGB
// ===========================================================================
static void arranque(void)
{
    // LEDs 1 al 5, uno a uno
    for (uint8_t i = 1; i <= 5; i++)
    {
        bsp_led_on(i);
        bsp_delay_ms(150);
        bsp_led_off(i);
    }

    // RGB: rojo, verde, azul, blanco, apagado
    bsp_rgb_set(true,  false, false);  
    bsp_delay_ms(300);
    bsp_rgb_set(false, true,  false);  
    bsp_delay_ms(300);
    bsp_rgb_set(false, false, true);   
    bsp_delay_ms(300);
    bsp_rgb_set(true,  true,  true);   
    bsp_delay_ms(300);
    bsp_rgb_off();
}

// ===========================================================================
//  DEMO LEDs  parpadeo secuencial
//  SW1 cambia velocidad, SW2 sale
// ===========================================================================
static void demo_leds(void)
{
    uint32_t velocidad = 200;
    uint8_t  led = 1;

    printf("  LEDs: SW1=velocidad  SW2=salir\n");

    while (!sw2_pulsado())
    {
        if (sw1_pulsado())
        {
            // Cicla entre 3 velocidades
            if(velocidad == 200) 
            {
                velocidad = 100;
            }
            else if (velocidad == 100) 
            {
                velocidad = 50;
            }
            else                       
            {
                velocidad = 200;
            }
            printf("  Velocidad: %lu ms\n", (unsigned long)velocidad);
        }

        bsp_led_all_off();
        bsp_led_on(led);
        led = (led % 5) + 1;
        bsp_delay_ms(velocidad);
    }

    bsp_led_all_off();
}

// ===========================================================================
//  DEMO RGB  colores fijos y breath
//  Todo usa LEDC (bsp_pwm_rgb_x) para evitar conflicto con GPIO
// ===========================================================================

// Breath: el brillo sube de 0 a 100 y luego baja de 100 a 0.
// Se usa una variable de direccion (subiendo/bajando) en vez de tabla.

static void demo_rgb(void)
{
    // Colores disponibles: r, g, b en porcentaje (0-100)
    static const struct { uint8_t r; uint8_t g; uint8_t b; const char *nombre; } colores[] = {
        { 100,   0,   0, "ROJO"     },
        {   0, 100,   0, "VERDE"    },
        {   0,   0, 100, "AZUL"     },
        { 100, 100,   0, "AMARILLO" },
        {   0, 100, 100, "CYAN"     },
        { 100,   0, 100, "MAGENTA"  },
        { 100, 100, 100, "BLANCO"   },
    };
    const uint8_t n_colores = 7;
    uint8_t idx = 0;
    bool modo_breath = false;
    int8_t  brillo    = 0;    // valor actual 0-100
    int8_t  dir_breath = 1;   // +1 sube, -1 baja
    uint64_t t_breath = bsp_get_ms();

    bsp_pwm_rgb_init();
    bsp_pwm_rgb_set_percent(colores[0].r, colores[0].g, colores[0].b);
    printf("  RGB: SW1=siguiente color / breath  SW2=salir\n");
    printf("  Color: %s\n", colores[0].nombre);

    while (!sw2_pulsado())
    {
        if (sw1_pulsado())
        {
            idx = (idx + 1) % (n_colores + 1); // +1 para el modo breath

            if (idx < n_colores)
            {
                modo_breath = false;
                bsp_pwm_rgb_set_percent(colores[idx].r, colores[idx].g, colores[idx].b);
                printf("  Color: %s\n", colores[idx].nombre);
            }
            else
            {
                modo_breath = true;
                //paso = 0;
                printf("  Modo: BREATH\n");
            }
        }

        // Animacion breath (no bloqueante)
        // Cada 30 ms se incrementa o decrementa el brillo.
        // Al llegar a 100 invierte la direccion, al llegar a 0 vuelve a subir.
        if (modo_breath && (bsp_get_ms() - t_breath) >= 30)
        {
            t_breath = bsp_get_ms();
            brillo += dir_breath;
            if (brillo >= 100) 
            { 
                brillo = 100; dir_breath = -1; 
            }
            if (brillo <=   0) 
            { 
                brillo =   0; dir_breath =  1; 
            }
            bsp_pwm_rgb_set_percent((uint8_t)brillo, 0, (uint8_t)brillo); // magenta
        }

        bsp_delay_ms(10);
    }

    bsp_pwm_rgb_off();
}

// ===========================================================================
//  DEMO ROTAS  patrones en LEDs 1-4
//  SW1 cambia patron, SW2 sale
// ===========================================================================
static void demo_rotas(void)
{

    static const char *nombres[] = { "IDA", "PING-PONG", "2 EN 2" };

    rota_t patron = ROTA_IDA;
    int8_t pos = 0, dir = 1;
    bool grupo = false;

    printf("  Rotas: SW1=patron  SW2=salir\n");
    printf("  Patron: %s\n", nombres[patron]);

    while (!sw2_pulsado())
    {
        if (sw1_pulsado())
        {
            patron = (rota_t)((patron + 1) % ROTA_COUNT);
            pos = 0; dir = 1; grupo = false;
            printf("  Patron: %s\n", nombres[patron]);
        }

        // Apagar LEDs 1-4
        for (uint8_t i = 1; i <= 4; i++) bsp_led_off(i);

        // Ejecutar patron activo
        if (patron == ROTA_IDA)
        {
            bsp_led_on(pos + 1);
            pos = (pos + 1) % 4;
        }
        else if (patron == ROTA_PINGPONG)
        {
            bsp_led_on((uint8_t)(pos + 1));
            pos += dir;
            if (pos >= 3) { pos = 3; dir = -1; }
            if (pos <= 0) { pos = 0; dir =  1; }
        }
        else // ROTA_2EN2
        {
            if (grupo) 
            { 
                bsp_led_on(1); bsp_led_on(2); 
            }
            else        
            { 
                bsp_led_on(3); bsp_led_on(4); 
            }
            grupo = !grupo;
        }

        bsp_delay_ms(180);
    }

    for (uint8_t i = 1; i <= 4; i++) bsp_led_off(i);
}

// ===========================================================================
//  Imprime el menu en consola
// ===========================================================================
static void menu_print(uint8_t sel)
{
    static const char *items[] = { "LEDs", "RGB", "Rotas" };
    printf("\n-- MENU (SW1=siguiente  SW2=entrar) --\n");
    for (uint8_t i = 0; i < 3; i++)
        printf("  %s %s\n", (i == sel) ? "[>]" : "[ ]", items[i]);
}

// ===========================================================================
//  app_main
// ===========================================================================
void app_main(void)
{
    bsp_init();
    bsp_sw1_irq_attach(sw1_isr, NULL);

    arranque();

    uint8_t sel = 0;
    menu_print(sel);

    while (1)
    {
        if (sw1_pulsado())
        {
            sel = (sel + 1) % 3;
            menu_print(sel);
        }

        if (sw2_pulsado())
        {
            if      (sel == 0) 
            {
                demo_leds();
            }
            else if (sel == 1) 
            {
                demo_rgb();
            }
            else 
            {
                demo_rotas();
            }
            menu_print(sel);
        }

        bsp_delay_ms(10);
    }
}