// FileName:        hal_timer.h                                                                                     
// Dependencies:    "timer_2026.h"                                                                                   
// Processor:       Tensilica Xtensa LX6 160 MHz                                                                                           
// Board:           ESP-WROOM-32                                                                                     
// Program version: 2.0                                                                                              
// Company:         Instituto Tecnologico de Chihuahua                                                               
// Description:     Abstracción sobre timer_2026
//                  Hace uso de los 4 timers de la tarjeta:
//                  (HAL_TIMER_1 ... HAL_TIMER_4) que mapean internamente a los
//                  Timer Groups y Timer Numbers del ESP32:
//                     HAL_TIMER_1 -> TIMER_GROUP_0, TIMER_0
//                     HAL_TIMER_2 -> TIMER_GROUP_0, TIMER_1
//                     HAL_TIMER_3 -> TIMER_GROUP_1, TIMER_0
//                     HAL_TIMER_4 -> TIMER_GROUP_1, TIMER_1
// Authors:         Ana Paola Cardona Valenzuela
//                  Luis Adrian Anchondo Carreón
//                  Emiliano Perez Dyck 
// Created:         01/06/2026
// Updated:         02/06/2026

#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include "timer_2026.h"

// ===========================================================================
//  Identificadores del timer
// ===========================================================================

/**
 * @brief Identificadores de los 4 timers disponibles en el ESP32
 *          HAL_TIMER_1 -> TIMER_GROUP_0 / TIMER_0
 *          HAL_TIMER_2 -> TIMER_GROUP_0 / TIMER_1
 *          HAL_TIMER_3 -> TIMER_GROUP_1 / TIMER_0
 *          HAL_TIMER_4 -> TIMER_GROUP_1 / TIMER_1
 */
typedef enum {
    HAL_TIMER_1 = 0,
    HAL_TIMER_2 = 1,
    HAL_TIMER_3 = 2,
    HAL_TIMER_4 = 3
} hal_timer_id_t;

// ===========================================================================
//  Mapeo de Timers
// ===========================================================================

/**
 * @brief Entrada de la tabla de mapeo 
 */
typedef struct {
    timer_group_t group;
    timer_num_t   num;
} hal_timer_map_t;

/**
 * @brief Declaración de la tabla de mapeo. (Definida en hal_timer.c)
 */
extern const hal_timer_map_t hal_timer_map[4];

// ===========================================================================
//  Extrae group y num de la tabla a partir del hal_timer_id_t
// ===========================================================================
#define HAL_GROUP(id) (hal_timer_map[(id)].group)
#define HAL_NUM(id)   (hal_timer_map[(id)].num)


/**
 * @brief Inicializa uno de los 4 timers del sistema
 *        Debe llamarse una vez por timer antes de usar cualquier otra función
 * @param id  Identificador del timer: HAL_TIMER_1 ... HAL_TIMER_4
 * @return void
 */
void hal_timer_init(hal_timer_id_t id);

/**
 * @brief Genera un retardo bloqueante en milisegundos
 * @param id  Identificador del timer a usar
 * @param ms  Tiempo de retardo en milisegundos
 * @return void
 */
void hal_timer_delay_ms(hal_timer_id_t id, uint32_t ms);

/**
 * @brief Genera un retardo bloqueante en microsegundos
 * @param id  Identificador del timer a usar
 * @param us  Tiempo de retardo en microsegundos
 * @return void
 */
void hal_timer_delay_us(hal_timer_id_t id, uint32_t us);

// ===========================================================================
//  Lectura de tiempo transcurrido
// ===========================================================================
/**
 * @brief Devuelve los microsegundos transcurridos desde hal_timer_init()
 * @param id  Identificador del timer a leer
 * @return uint64_t  Microsegundos desde la inicialización del timer
 */
uint64_t hal_timer_get_us(hal_timer_id_t id);

/**
 * @brief Devuelve los milisegundos transcurridos desde hal_timer_init()
 *        Internamente divide la lectura en µs entre 1000
 * @param id  Identificador del timer a leer
 * @return uint64_t  Milisegundos desde la inicialización del timer
 */
uint64_t hal_timer_get_ms(hal_timer_id_t id);

// ===========================================================================
//  Periodo cumplido
// ===========================================================================
/**
 * @brief Verifica si ya transcurrió un período en milisegundos (no bloqueante)
 *        Si el período se cumplió, actualiza automáticamente *ultimo_ms
 *        para preparar el siguiente ciclo
 *
 * @param id         Identificador  del timer a usar
 * @param ultimo_ms  Puntero a la última marca de tiempo registrada (en ms)
 * @param periodo_ms Período a verificar en milisegundos
 * @return bool  true si el período ya transcurrió, false si no
 */
bool hal_timer_periodo_cumplido(hal_timer_id_t id, uint64_t *ultimo_ms, uint32_t periodo_ms);

// ===========================================================================
//  Marca de tiempo y tiempo transcurrido
// ===========================================================================
/**
 * @brief Guarda el instante actual como punto de referencia (marca de inicio)
 * @param id     Identificador del timer a usar
 * @param marca  Puntero a la variable donde se guardará la marca (en µs)
 * @return void
 */
void hal_timer_marca_inicio(hal_timer_id_t id, uint64_t *marca);

/**
 * @brief Calcula los microsegundos transcurridos desde una marca de inicio
 * @param id     Identificador del timer a usar
 * @param marca  Puntero a la marca guardado con hal_timer_marca_inicio()
 * @return uint64_t  Microsegundos transcurridos desde la marca
 */
uint64_t hal_timer_elapsed_us(hal_timer_id_t id, uint64_t *marca);

#endif