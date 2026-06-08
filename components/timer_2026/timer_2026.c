// FileName:        timer_2026.c                                                                                     
// Dependencies:    "timer_2026.h"                                                                                   
// Processor:       ESP32                                                                                            
// Board:           ESP-WROOM-32                                                                                     
// Program version: 3.0                                                                                             
// Company:         Instituto Tecnologico de Chihuahua                                                               
// Description:     Implementación de la lógica de bajo nivel para la        
//                  inicialización, lectura dividida de 64 bits y generación 
//                  de retardos con los 4 timers del ESP32:
//                     - Timer Group 0 (TIMG0): Timer 0 y Timer 1
//                     - Timer Group 1 (TIMG1): Timer 0 y Timer 1 
// Authors:           Ana Paola Cardona Valenzuela
//                  Luis Adrian Anchondo Carreón
//                  Emiliano Perez Dyck 
// Created:         31/05/2026
// Updated:         07/06/2026

#include "timer_2026.h"

// ===========================================================================
//  Funciones de ayuda interna para WDT
//  Parametrizados por dirección base para no duplicar código entre TIMG0/1
// ===========================================================================

static void mwdt_disable(uint32_t timg_base)
{
    // Desbloquear write-protection
    MWDT_WPROTECT(timg_base) = WDT_WRITE_ENABLE_KEY;
    // Limpiar bit EN (bit 31), deshabilita el WDT conservando el resto de bits
    MWDT_CONFIG0(timg_base) &= ~MWDT_EN_BIT;
    // Volver a bloquear
    MWDT_WPROTECT(timg_base) = WDT_WRITE_DISABLE_KEY;
}

static void mwdt_feed(uint32_t timg_base)
{
    MWDT_WPROTECT(timg_base) = WDT_WRITE_ENABLE_KEY;
    // Cualquier escritura en FEED reinicia el contador del WDT
    MWDT_FEED(timg_base) = 1;
    MWDT_WPROTECT(timg_base) = WDT_WRITE_DISABLE_KEY;
}

// ===========================================================================
//  timer_init
// ===========================================================================
void timer_init(timer_group_t group, timer_num_t num)
{
    // Deshabilitar el timer antes de configurarlo para evitar comportamientos
    // erráticos
    TIMG_CONFIG_REG(group, num) &= ~TIMER_EN_BIT;

    // Configurar el prescaler y el modo de conteo ascendente
    // Escribimos todo el registro de una vez para evitar estados intermedios
    // inconsistentes (el timer ya está deshabilitado en este punto)
    TIMG_CONFIG_REG(group, num) = ((uint32_t)TIMER_PRESCALER << TIMER_DIVIDER_SHIFT)
                                 | TIMER_INCREASE_BIT;

    // Preparar el reinicio del contador a 0
    // El registro de recarga es de 32 bits 
    TIMG_LOADLO_REG(group, num) = 0;   // Valor de recarga 32 bits

    // Disparar la carga, el hardware toma los valores de LOADLO/LOADHI
    // y los aplica al contador interno. Escribir cualquier valor distinto de 0
    // es suficiente para activar el disparo
    TIMG_LOAD_REG(group, num) = 1;

    // Habilitar el timer para que comience a contar desde 0
    TIMG_CONFIG_REG(group, num) |= TIMER_EN_BIT;
}

// ===========================================================================
//  timer_get_us
// ===========================================================================
uint64_t timer_get_us(timer_group_t group, timer_num_t num)
{
    // Enviar cualquier valor para actualizar
    // Esto obliga al hardware a congelar y copiar el valor actual del contador
    // hacia los registros de lectura (LO y HI), garantizando una lectura
    // atómica de los 64 bits aunque se realice en dos lecturas de 32 bits
    TIMG_UPDATE_REG(group, num) = 1;

    // Leer las dos mitades de 32 bits
    // Se castea a uint64_t antes de desplazar para evitar overflow
    uint64_t lo = (uint64_t)TIMG_LO_REG(group, num);
    uint64_t hi = (uint64_t)TIMG_HI_REG(group, num);

    // Combinar ambas mitades en una sola variable de 64 bits
    // La parte alta se desplaza 32 bits a la izquierda y se une con OR a la baja
    return (hi << 32) | lo;
}

// ===========================================================================
//  timer_delay_us
//
//  Alimenta el MWDT del TIMG0 cada WDT_FEED_INTERVAL_US durante el
//  busy-wait para que el watchdog no se dispare en retardos largos
//  En retardos cortos (< WDT_FEED_INTERVAL_US) el feed nunca ocurre
// ===========================================================================
void timer_delay_us(timer_group_t group, timer_num_t num, uint32_t us)
{
    uint64_t inicio = timer_get_us(group, num);
    uint64_t next_feed_us = inicio + WDT_FEED_INTERVAL_US;

    while ((timer_get_us(group, num) - inicio) < (uint64_t)us)
    {
        uint64_t ahora = timer_get_us(group, num);
        if (ahora >= next_feed_us)
        {
            mwdt_feed(TIMG0_BASE); // TWDT
            mwdt_feed(TIMG1_BASE); // IWDT
            next_feed_us = ahora + WDT_FEED_INTERVAL_US;
        }
    }
}


// ===========================================================================
//  timer_delay_ms
// ===========================================================================
void timer_delay_ms(timer_group_t group, timer_num_t num, uint32_t ms)
{
    // Reutilizamos timer_delay_us() multiplicando por 1000
    timer_delay_us(group, num, ms * 1000);
}

// ===========================================================================
//  Funciones del watchdog timer
// ===========================================================================
void wdt_disable_timg0(void)
{
    mwdt_disable(TIMG0_BASE);
}

void wdt_disable_timg1(void)
{
    mwdt_disable(TIMG1_BASE);
}

void wdt_disable_rtc(void)
{
    RWDT_WPROTECT = WDT_WRITE_ENABLE_KEY;
    RWDT_CONFIG0 &= ~RWDT_EN_BIT;
    RWDT_WPROTECT = WDT_WRITE_DISABLE_KEY;
}

void wdt_disable_all(void)
{
    wdt_disable_timg0();
    wdt_disable_timg1();
    wdt_disable_rtc();
}

void wdt_feed_timg0(void)
{
    mwdt_feed(TIMG0_BASE);
}

bool wdt_is_enabled_timg0(void)
{
    return (MWDT_CONFIG0(TIMG0_BASE) & MWDT_EN_BIT) != 0u;
}
