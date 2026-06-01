#ifndef HAL_TIMER_H
#define HAL_TIMER_H

// FileName:        hal_timer.h                                                                                     
// Dependencies:    "timer_2026.h"                                                                                   
// Processor:       Tensilica Xtensa LX6 160 MHz                                                                                           
// Board:           ESP-WROOM-32                                                                                     
// Program version: 1.0                                                                                              
// Company:         Instituto Tecnologico de Chihuahua                                                               
// Description:     Abstracción sobre timer_2026. No acceder
//                  a registros directamente.
// Authors:           Ana Paola Cardona Valenzuela
//                  Luis Adrian Anchondo Carreón
//                  Emiliano Perez Dyck 
// Updated:         01/06/2026

#include "timer_2026.h"

/**
 * @brief Inicializa el timer del sistema
 *        Debe de llamarse una vez al inicio en gpio_init2026()
 */
void hal_timer_init(void);

/**
 * @brief Retardo en milisegundos
 * @param ms Tiempo a esperar en ms
 */
void hal_timer_delay_ms(uint32_t ms);

/**
 * @brief Retardo en microsegundos
 * @param us Tiempo a esperar en µs
 */
void hal_timer_delay_us(uint32_t us);

/**
 * @brief Devuelve el tiempo transcurrido desde el inicio en ms
 * @return Milisegundos desde el hal_timer_init()
 */
uint64_t hal_timer_get_ms(void);

/**
 * @brief Devuelve el tiempo transcurrido desde el inicio en µs
 * @return Microsegundos desde el hal_timer_init()
 */
uint64_t hal_timer_get_us(void);

/**
 * @brief Verifica si ya transcurrió un período 
 * @param ultimo_ms  Puntero a la marca de tiempo del último disparo
 * @param periodo_ms Período deseado en ms
 * @return true si ya pasó el período, actualiza *ultimo_ms automáticamente
 */
bool hal_timer_periodo_cumplido(uint64_t *ultimo_ms, uint32_t periodo_ms);

/**
 * @brief Guarda el tiempo actual como punto de inicio
 * @param marca Puntero donde se guarda la marca de tiempo
 */
void hal_timer_marca_inicio(uint64_t *marca);

/**
 * @brief Calcula cuánto tiempo pasó desde la marca
 * @param marca Timestamp guardado con hal_timer_marca_inicio()
 * @return Microsegundos transcurridos
 */
uint64_t hal_timer_elapsed_us(uint64_t *marca);

#endif