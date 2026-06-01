// FileName:        timer_2026.h                                                                                     
// Dependencies:    None                                                                         
// Processor:       ESP32                                                                                            
// Board:           ESP-WROOM-32                                                                                     
// Program version: 1.0                                                                                              
// Company:         Instituto Tecnologico de Chihuahua                                                               
// Description:     Declaración de macros, direcciones de memoria física y   
//                  prototipos de funciones a nivel hardware para el manejo  
//                  del Timer Group 0 (TIMG0) del ESP32.                     
// Autor:           Ana Paola Cardona Valenzuela
//                  Luis Adrian Anchondo Carreón
//                  Emiliano Perez Dyck 
// Updated:         31/05/2026
#ifndef TIMER_2026_H
#define TIMER_2026_H

#include <stdint.h>
#include <stdbool.h>

#define HWREG32(x) (*((volatile uint32_t *)(x)))

// Registros TIMG0 del ESP32
#define TIMG0_BASE 0x3FF5F000 // Direccióon base del registro del timer0
#define TIMG0_T0CONFIG_REG  HWREG32(TIMG0_BASE + 0x0000) // Registro de configuración del timer0
#define TIMG0_T0UPDATE_REG  HWREG32(TIMG0_BASE + 0x000C) // Registro para forzar la actualización del timer
// Registros de lectura del contador (dividido en Low y High por ser de 64 bits)
#define TIMG0_T0LO_REG      HWREG32(TIMG0_BASE + 0x0004) // Valor actual (32 bits menos significativos)
#define TIMG0_T0HI_REG      HWREG32(TIMG0_BASE + 0x0008) // Valor actual (32 bits más significativos)
// Registros de recarga del timer 0
#define TIMG0_T0LOADLO_REG  HWREG32(TIMG0_BASE + 0x0018) // Valor actual (32 bits menos significativos)
#define TIMG0_T0LOADHI_REG  HWREG32(TIMG0_BASE + 0x001C) // Valor actual (32 bits más significativos)
#define TIMG0_T0LOAD_REG    HWREG32(TIMG0_BASE + 0x0020) // Copia el valor de recarga en el contador del temporizador

#define TIMER_EN_BIT        (1 << 31) // Habilita el contador del timer TIMGn_Tx_EN
#define TIMER_INCREASE_BIT  (1 << 30) // Configura el conteo de forma ascendente
#define TIMER_DIVIDER_SHIFT 13

/* * El reloj base típico de los periféricos del ESP32 (APB clock) funciona a 80 MHz
 * Al usar un prescaler de 80, dividimos el reloj: 80,000,000 / 80 = 1,000,000 Hz
 * Esto significa que el timer incrementará 1 vez cada microsegundo (1 tick = 1 µs).
 */
#define TIMER_PRESCALER 80
// -----------------------  Prototipos de funciones --------------------------
/**
 * @brief Inicializa el Timer0 (TIMG0)
 * * Configura el prescaler (divisor) para establecer la base de tiempo,
 * * el modo de conteo de forma ascendente y habilita el contador
 * * @param void
 * @return void
 */
void timer_init(void);

 /**
 * @brief Genera un retardo bloqueante en milisegundos.
 * * @param ms Tiempo de retardo deseado expresado en milisegundos
 * @return void
 */
void timer_delay_ms(uint32_t ms);

/**
 * @brief Genera un retardo bloqueante en microsegundos.
 * * @param us Tiempo de retardo deseado expresado en microsegundos
 * @return void
 */
void timer_delay_us(uint32_t us);

/**
 * @brief Obtiene el valor actual del contador del timer
 * * Realiza la lectura de los registros T0LO y T0HI (conformando los 64 bits)
 * forzando la actualización con el registro UPDATE, y devuelve el tiempo equivalente
 * * @param void
 * @return uint64_t Tiempo transcurrido desde el inicio del timer, en microsegundos
 */
uint64_t timer_get_us(void);

#endif