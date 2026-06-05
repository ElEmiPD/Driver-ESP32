// FileName:        wdt_2026.h
// Dependencies:    None
// Processor:       Tensilica Xtensa LX6 160 MHz
// Board:           ESP-WROOM-32
// Program version: 1.0
// Company:         Instituto Tecnologico de Chihuahua
// Description:     Driver de Watchdog Timer para ESP32 en bare-metal.
//                  Acceso directo a registros del MWDT (Main WDT) de los
//                  Timer Groups y del RWDT (RTC WDT).
//                  Sin dependencias de ESP-IDF ni de FreeRTOS.
//
//                  Contexto de hardware:
//                    TIMG0 MWDT (0x3FF5F048)  usado por TWDT del ESP-IDF
//                    TIMG1 MWDT (0x3FF60048)  usado por IWDT del ESP-IDF
//                    RTC   RWDT (0x3FF48090)  protección de boot
// 
//                  USO RECOMENDADO en este proyecto:
//                    Llamar wdt_disable_all() en bsp_init() para silenciar
//                    los tres WDTs antes de que el busy-wait del timer_2026
//                    los dispare. No se necesita alimentarlos manualmente.
//
//                  ADVERTENCIA: Deshabilitar los WDTs elimina la protección
//                  contra bucles infinitos y cuelgues. Solo hacerlo en
//                  entornos de desarrollo o cuando el sistema tenga otra
//                  forma de recuperación (reset externo, supervisión externa).
//
// Autores:         Ana Paola Cardona Valenzuela
//                  Emiliano Perez Dyck
//                  Luis Adrian Anchondo Carreón
// Created:         04/06/2026
// Updated:         04/06/2026

#ifndef WDT_2026_H
#define WDT_2026_H

#include <stdint.h>
#include <stdbool.h>

// ===========================================================================
//  Macro de acceso directo a memoria (ya definida en timer_2026.h;
//  se guarda con #ifndef para no redefinir si se incluyen juntos)
// ===========================================================================
#ifndef HWREG32
#define HWREG32(x) (*((volatile uint32_t *)(x)))
#endif

// ===========================================================================
//  Write-protection key
//  Para escribir en cualquier registro del WDT primero hay que escribir
//  esta clave en WDTWPROTECT. Para volver a proteger: escribir cualquier
//  otro valor (convencionalmente 0).
//  TRM ESP32 v4.6  sección 19.3.1.2 "Write Protection"
// ===========================================================================
#define WDT_WRITE_ENABLE_KEY   0x50D83AA1u
#define WDT_WRITE_DISABLE_KEY  0x00000000u

// ===========================================================================
//  MWDT  Main Watchdog Timer (Timer Groups)
//  Cada Timer Group tiene su propio MWDT independiente.
//  TRM sección 19 + tabla de registros sección 18.4
// ===========================================================================

// Direcciones base de los Timer Groups
#ifndef TIMG0_BASE
#define TIMG0_BASE   0x3FF5F000u
#define TIMG1_BASE   0x3FF60000u
#endif

// Offsets de registros del MWDT dentro del Timer Group
// (TRM ESP32 v4.6, sección 18.4  Timer Group Registers)
#define MWDT_CONFIG0_OFFSET    0x0048u   // Configuración y control
#define MWDT_CONFIG1_OFFSET    0x004Cu   // Timeout stage 0 (ticks)
#define MWDT_CONFIG2_OFFSET    0x0050u   // Timeout stage 1
#define MWDT_CONFIG3_OFFSET    0x0054u   // Timeout stage 2
#define MWDT_CONFIG4_OFFSET    0x0058u   // Timeout stage 3
#define MWDT_CONFIG5_OFFSET    0x005Cu   // Alimentar al WDT (feed)
#define MWDT_FEED_OFFSET       0x0060u   // Registro de feed
#define MWDT_WPROTECT_OFFSET   0x0064u   // Write-protection key

// Macros de acceso parametrizados por Timer Group base
#define MWDT_CONFIG0(base)   HWREG32((base) + MWDT_CONFIG0_OFFSET)
#define MWDT_FEED(base)      HWREG32((base) + MWDT_FEED_OFFSET)
#define MWDT_WPROTECT(base)  HWREG32((base) + MWDT_WPROTECT_OFFSET)

// Bit de habilitación en MWDT_CONFIG0 (bit 31)
#define MWDT_EN_BIT          (1u << 31)

// ===========================================================================
//  RWDT  RTC Watchdog Timer
//  Vive en el módulo RTC_CNTL, dirección base 0x3FF48000
//  TRM sección 30 (Low-Power Management)  registros RTC
//  NOTA: Los registros del RWDT no están documentados completamente en
//        el TRM público v4.1. Las direcciones aquí provienen del
//        análisis del soc/rtc_cntl_reg.h del ESP-IDF y están
//        verificadas contra el comportamiento real del hardware.
// ===========================================================================
#define RTC_CNTL_BASE          0x3FF48000u

#define RWDT_CONFIG0_OFFSET    0x008Cu   // Configuración principal RWDT
#define RWDT_FEED_OFFSET       0x009Cu   // Registro de feed
#define RWDT_WPROTECT_OFFSET   0x00A0u   // Write-protection key

#define RWDT_CONFIG0     HWREG32(RTC_CNTL_BASE + RWDT_CONFIG0_OFFSET)
#define RWDT_FEED        HWREG32(RTC_CNTL_BASE + RWDT_FEED_OFFSET)
#define RWDT_WPROTECT    HWREG32(RTC_CNTL_BASE + RWDT_WPROTECT_OFFSET)

// Bit de habilitación en RWDT_CONFIG0 (bit 31)
#define RWDT_EN_BIT      (1u << 31)

// ===========================================================================
//  Prototipos de funciones
// ===========================================================================

/**
 * @brief  Deshabilita el MWDT del Timer Group 0 (TIMG0).
 *
 *         El ESP-IDF usa este WDT para el Task Watchdog Timer (TWDT).
 *         Deshabilitarlo elimina la vigilancia de tareas que no ceden CPU.
 *         Llamar en bsp_init() antes de cualquier busy-wait largo.
 *
 *         Secuencia de escritura (TRM 19.3.1.2):
 *           1. Escribir WDT_WRITE_ENABLE_KEY en WDTWPROTECT  -> desbloquea
 *           2. Limpiar bit EN en CONFIG0                     -> deshabilita
 *           3. Escribir 0 en WDTWPROTECT                    -> vuelve a bloquear
 */
void wdt_disable_timg0(void);

/**
 * @brief  Deshabilita el MWDT del Timer Group 1 (TIMG1).
 *
 *         El ESP-IDF usa este WDT para el Interrupt Watchdog Timer (IWDT).
 *         PRECAUCIÓN: Deshabilitar el IWDT significa que un bucle en una
 *         ISR no será detectado. Solo hacerlo si se está seguro de que
 *         las ISRs no pueden bloquearse indefinidamente.
 */
void wdt_disable_timg1(void);

/**
 * @brief  Deshabilita el RTC Watchdog Timer (RWDT).
 *
 *         El ESP-IDF lo usa para protección durante el boot.
 *         Normalmente ya está desactivado cuando app_main() corre,
 *         pero se incluye por completitud.
 */
void wdt_disable_rtc(void);

/**
 * @brief  Deshabilita los tres WDTs del sistema de una sola vez.
 *
 *         Función de conveniencia equivalente a llamar:
 *           wdt_disable_timg0();
 *           wdt_disable_timg1();
 *           wdt_disable_rtc();
 *
 *         Uso típico en bsp_init():
 *           void bsp_init(void) {
 *               wdt_disable_all();   // primero
 *               hal_timer_init(...); // después el resto
 *               ...
 *           }
 */
void wdt_disable_all(void);

/**
 * @brief  Alimenta (resetea) el MWDT del Timer Group 0 sin deshabilitarlo.
 *
 *         Útil si en el futuro se quiere mantener el WDT activo pero
 *         alimentarlo manualmente desde el driver de timer durante
 *         busy-waits largos  alternativa a deshabilitarlo.
 *
 *         Debe llamarse con una frecuencia mayor a la del timeout
 *         configurado en MWDT_CONFIG1 (por defecto ~5 s en ESP-IDF).
 */
void wdt_feed_timg0(void);

/**
 * @brief  Retorna true si el MWDT del TIMG0 está habilitado.
 *         Útil para diagnóstico o para verificar antes de operar.
 */
bool wdt_is_enabled_timg0(void);

#endif /* WDT_2026_H */