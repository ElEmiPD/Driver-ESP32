// FileName:        timer_2026.h                                                                                     
// Dependencies:    None                                                                         
// Processor:       ESP32                                                                                            
// Board:           ESP-WROOM-32                                                                                     
// Program version: 2.0                                                                                              
// Company:         Instituto Tecnologico de Chihuahua                                                               
// Description:     Declaración de macros, direcciones de memoria física y   
//                  prototipos de funciones a nivel hardware para el manejo  
//                  de los 4 timers de la ESP32:
//                     - Timer Group 0 (TIMG0): Timer 0 y Timer 1
//                     - Timer Group 1 (TIMG1): Timer 0 y Timer 1               
// Authors:         Ana Paola Cardona Valenzuela
//                  Luis Adrian Anchondo Carreón
//                  Emiliano Perez Dyck 
// Created:         31/05/2026
// Updated:         02/06/2026

#ifndef TIMER_2026_H
#define TIMER_2026_H

#include <stdint.h>
#include <stdbool.h>

// ===========================================================================
// Macro de acceso a directo a memoria
// ===========================================================================
#ifndef HWREG32
#define HWREG32(x) (*((volatile uint32_t *)(x)))
#endif

// ===========================================================================
// Identificadores de Timer Group y Timer Number
// ===========================================================================

/**
 * @brief Selecciona el Timer Group del ESP32
 */
typedef enum {
    TIMER_GROUP_0 = 0,
    TIMER_GROUP_1 = 1
}timer_group_t;

/**
 * @brief Selecciona cuál de los 2 timers dentro del grupo se usará
 *        Cada grupo tiene un Timer 0 y un Timer 1
 */
typedef enum {
    TIMER_0 = 0, // Primer timer del grupo
    TIMER_1 = 1  // Segundo timer del grupo
} timer_num_t;

// ===========================================================================
//  Direcciones base de cada Timer Group
// ===========================================================================
#define TIMG0_BASE 0x3FF5F000 // Direccióon base del Timer Group 0
#define TIMG1_BASE 0x3FF60000 // Dirección base del Timer Group 1

/**
 * @brief Obtiene la dirección base del Timer Group indicado
 *        Usado internamente por las macros de registros
 */
#define TIMG_BASE(group) ((group) == TIMER_GROUP_0 ? TIMG0_BASE : TIMG1_BASE)

// ===========================================================================
//  Offsets de registros por timer dentro de un grupo
//  Obtenidos de ESP32 TRM v4.6, sección 18 — Timer Group (TIMG)
//
//  El stride entre Timer 0 y Timer 1 es de 0x0024 bytes
//  Verificación con TIMG0:
//    T0CONFIG = 0x0000   T1CONFIG = 0x0024 -> diff = 0x0024 
//    T0LO     = 0x0004   T1LO     = 0x0028 -> diff = 0x0024 
//    T0HI     = 0x0008   T1HI     = 0x002C -> diff = 0x0024 
//    T0UPDATE = 0x000C   T1UPDATE = 0x0030 -> diff = 0x0024 
//    T0LOADLO = 0x0018   T1LOADLO = 0x003C -> diff = 0x0024 
//    T0LOAD   = 0x0020   T1LOAD   = 0x0044 -> diff = 0x0024 
// ===========================================================================

#define TIMER_STRIDE 0x0024 // Separación en bytes entre T0 y T1
#define TIMG_TxCONFIG 0x0000U // Registro de configuración
#define TIMG_TxLO 0x0004 // Contador: 32 bits bajos  (RO)
#define TIMG_TxHI 0x0008 // Contador: 32 bits altos  (RO)
#define TIMG_TxUPDATE 0x000C // Dispara captura del contador hacia LO/HI (WO)
#define TIMG_TxALARMLO 0x0010 // Valor de alarma: 32 bits bajos  (R/W)
#define TIMG_TxALARMHI 0x0014 // Valor de alarma: 32 bits altos  (R/W)
#define TIMG_TxLOADLO 0x0018 // Valor de recarga: 32 bits (único registro LOAD) (R/W)
#define TIMG_TxLOAD 0x0020 // Aplica LOADLO al contador interno (WO)

// ===========================================================================
//  Macros de acceso a registros parametrizados por group y num
//
//  Ejemplo de uso (solo dentro del driver .c):
//    TIMG_CONFIG_REG(TIMER_GROUP_0, TIMER_1) |= TIMER_EN_BIT;
// ===========================================================================
#define TIMG_REG(group, num, offset) \
HWREG32(TIMG_BASE(group) + (TIMER_STRIDE * (num)) + (offset))

#define TIMG_CONFIG_REG(group, num) TIMG_REG(group, num, TIMG_TxCONFIG)
#define TIMG_LO_REG(group, num) TIMG_REG(group, num, TIMG_TxLO)
#define TIMG_HI_REG(group, num) TIMG_REG(group, num, TIMG_TxHI)
#define TIMG_UPDATE_REG(group, num) TIMG_REG(group, num, TIMG_TxUPDATE)
#define TIMG_ALARMLO_REG(group, num) TIMG_REG(group, num, TIMG_TxALARMLO)
#define TIMG_ALARMHI_REG(group, num) TIMG_REG(group, num, TIMG_TxALARMHI)
#define TIMG_LOADLO_REG(group, num) TIMG_REG(group, num, TIMG_TxLOADLO)
#define TIMG_LOAD_REG(group, num) TIMG_REG(group, num, TIMG_TxLOAD)

// ===========================================================================
//  Bits de control del registro TIMGn_Tx_CONFIG
// ===========================================================================
#define TIMER_EN_BIT (1 << 31) // Habilita el contador del timer TIMGn_Tx_EN
#define TIMER_INCREASE_BIT  (1 << 30) // Configura el conteo de forma ascendente
#define TIMER_DIVIDER_SHIFT 13 // Bits [28:13], campo del prescaler

/* * El reloj base típico de los periféricos del ESP32 (APB clock) funciona a 80 MHz
 *   Al usar un prescaler de 80, dividimos el reloj: 80,000,000 / 80 = 1,000,000 Hz
 *   Esto significa que el timer incrementará 1 vez cada microsegundo (1 tick = 1 µs)
 */
#define TIMER_PRESCALER 80

// ===========================================================================
//  Prototipos de funciones 
// ===========================================================================

/**
 * @brief  Inicializa el timer indicado del grupo indicado
 *         Configura el prescaler para obtener una base de tiempo de 1 µs/tick,
 *         establece el modo de conteo ascendente, resetea el contador a 0
 *         y habilita el timer
 *
 * @param group Timer Group a inicializar: TIMER_GROUP_0 o TIMER_GROUP_1
 * @param num  Timer a inicializar dentro del grupo: TIMER_0 o TIMER_1
 * @return void
 */
void timer_init(timer_group_t group, timer_num_t num);

/**
 * @brief  Lee el valor actual del contador del timer especificado
 *         Fuerza la captura del contador hacia los registros de lectura
 *         (mediante el registro UPDATE) y combina las dos mitades de 32 bits
 *         en un valor de 64 bits.
 *
 * @param  group  Timer Group: TIMER_GROUP_0 o TIMER_GROUP_1
 * @param  num    Timer dentro del grupo: TIMER_0 o TIMER_1
 * @return uint64_t  Microsegundos transcurridos desde la inicialización del timer
 */
uint64_t timer_get_us(timer_group_t group, timer_num_t num);

/**
 * @brief  Genera un retardo bloqueante en microsegundos usando el timer indicado
 *         El timer debe haber sido inicializado con timer_init() antes de llamar
 *         a esta función
 *
 * @param  group  Timer Group: TIMER_GROUP_0 o TIMER_GROUP_1
 * @param  num    Timer dentro del grupo: TIMER_0 o TIMER_1
 * @param  us     Tiempo de retardo en microsegundos
 * @return void
 */
void timer_delay_us(timer_group_t group, timer_num_t num, uint32_t us);

/**
 * @brief  Genera un retardo bloqueante en milisegundos usando el timer indicado
 *         Internamente convierte ms a µs y llama a timer_delay_us()
 *
 * @param  group  Timer Group: TIMER_GROUP_0 o TIMER_GROUP_1
 * @param  num    Timer dentro del grupo: TIMER_0 o TIMER_1
 * @param  ms     Tiempo de retardo en milisegundos
 * @return void
 */
void timer_delay_ms(timer_group_t group, timer_num_t num, uint32_t ms);

#endif