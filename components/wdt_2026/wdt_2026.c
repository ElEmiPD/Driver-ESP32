// FileName:        wdt_2026.c
// Dependencies:    wdt_2026.h
// Processor:       Tensilica Xtensa LX6 160 MHz
// Board:           ESP-WROOM-32
// Program version: 1.0
// Company:         Instituto Tecnologico de Chihuahua
// Description:     Implementación del driver de Watchdog Timer.
//                  Acceso directo a registros  sin ESP-IDF ni FreeRTOS.
//
//                  Protocolo de escritura protegida (TRM 19.3.1.2):
//                    Todos los registros del WDT tienen write-protection.
//                    Para modificarlos:
//                      1. Escribir 0x50D83AA1 en WDTWPROTECT  (unlock)
//                      2. Modificar el registro deseado
//                      3. Escribir 0x00000000 en WDTWPROTECT  (lock)
//                    Si se omite el unlock, la escritura es ignorada
//                    silenciosamente (el registro no cambia).
//
// Autores:         Ana Paola Cardona Valenzuela
//                  Emiliano Perez Dyck
//                  Luis Adrian Anchondo Carreón
// Created:         04/06/2026
// Updated:         04/06/2026

#include "wdt_2026.h"

// ===========================================================================
//  Helper interno: deshabilita el MWDT de cualquier Timer Group
//  Parametrizado por dirección base para no duplicar código.
// ===========================================================================
static void mwdt_disable(uint32_t timg_base)
{
    // Desbloquear write-protection
    MWDT_WPROTECT(timg_base) = WDT_WRITE_ENABLE_KEY;

    // Limpiar bit EN (bit 31) en CONFIG0  deshabilita el WDT
    //  Se conservan los demás bits para no alterar la configuración
    MWDT_CONFIG0(timg_base) &= ~MWDT_EN_BIT;

    // 3. Volver a bloquear 
    MWDT_WPROTECT(timg_base) = WDT_WRITE_DISABLE_KEY;
}

// ===========================================================================
//  Helper interno: alimenta el MWDT de cualquier Timer Group
// ===========================================================================
static void mwdt_feed(uint32_t timg_base)
{
    MWDT_WPROTECT(timg_base) = WDT_WRITE_ENABLE_KEY;
    // Escribir cualquier valor en el registro FEED reinicia el contador
    MWDT_FEED(timg_base) = 1u;
    MWDT_WPROTECT(timg_base) = WDT_WRITE_DISABLE_KEY;
}

// ===========================================================================
//  wdt_disable_timg0
//  Deshabilita el TWDT (Task Watchdog) del ESP-IDF
// ===========================================================================
void wdt_disable_timg0(void)
{
    mwdt_disable(TIMG0_BASE);
}

// ===========================================================================
//  wdt_disable_timg1
//  Deshabilita el IWDT (Interrupt Watchdog) del ESP-IDF
// ===========================================================================
void wdt_disable_timg1(void)
{
    mwdt_disable(TIMG1_BASE);
}

// ===========================================================================
//  wdt_disable_rtc
//  Deshabilita el RTC Watchdog
// ===========================================================================
void wdt_disable_rtc(void)
{
    // Desbloquear
    RWDT_WPROTECT = WDT_WRITE_ENABLE_KEY;

    // Deshabilitar
    RWDT_CONFIG0 &= ~RWDT_EN_BIT;

    // Volver a bloquear
    RWDT_WPROTECT = WDT_WRITE_DISABLE_KEY;
}

// ===========================================================================
//  wdt_disable_all
// ===========================================================================
void wdt_disable_all(void)
{
    wdt_disable_timg0();
    wdt_disable_timg1();
    wdt_disable_rtc();
}

// ===========================================================================
//  wdt_feed_timg0
// ===========================================================================
void wdt_feed_timg0(void)
{
    mwdt_feed(TIMG0_BASE);
}

// ===========================================================================
//  wdt_is_enabled_timg0
// ===========================================================================
bool wdt_is_enabled_timg0(void)
{
    return (MWDT_CONFIG0(TIMG0_BASE) & MWDT_EN_BIT) != 0u;
}