// FileName:        hal_timer.c                                                                                    
// Dependencies:    "hal_timer.h"                                                                                   
// Processor:       Tensilica Xtensa LX6 160 MHz                                                                                           
// Board:           ESP-WROOM-32                                                                                     
// Program version: 2.0                                                                                              
// Company:         Instituto Tecnologico de Chihuahua                                                               
// Description:     Implementación del HAL del timer con
//                  lógica para múltiples timers
// Authors:         Ana Paola Cardona Valenzuela
//                  Luis Adrian Anchondo Carreón
//                  Emiliano Perez Dyck 
// Created          01/06/2026
// Updated:         02/06/2026

#include "hal_timer.h"

// ===========================================================================
//  Definición de la tabla de mapeo
// ===========================================================================
/**
 * @brief Tabla por hal_timer_id_t
 *        HAL_TIMER_1 (0) -> TIMER_GROUP_0, TIMER_0
 *        HAL_TIMER_2 (1) -> TIMER_GROUP_0, TIMER_1
 *        HAL_TIMER_3 (2) -> TIMER_GROUP_1, TIMER_0
 *        HAL_TIMER_4 (3) -> TIMER_GROUP_1, TIMER_1
 */
const hal_timer_map_t hal_timer_map[4] = {
    { TIMER_GROUP_0, TIMER_0 }, // HAL_TIMER_1
    { TIMER_GROUP_0, TIMER_1 }, // HAL_TIMER_2
    { TIMER_GROUP_1, TIMER_0 }, // HAL_TIMER_3
    { TIMER_GROUP_1, TIMER_1 }  // HAL_TIMER_4
};

// ===========================================================================
//  hal_timer_init
// ===========================================================================
void hal_timer_init(hal_timer_id_t id)
{
    timer_init(HAL_GROUP(id), HAL_NUM(id));
}

// ===========================================================================
//  hal_timer_delay_us
// ===========================================================================
void hal_timer_delay_us(hal_timer_id_t id, uint32_t us)
{
    timer_delay_us(HAL_GROUP(id), HAL_NUM(id), us);
}

// ===========================================================================
//  hal_timer_delay_ms
// ===========================================================================
void hal_timer_delay_ms(hal_timer_id_t id, uint32_t ms)
{
    timer_delay_ms(HAL_GROUP(id), HAL_NUM(id), ms);
}

// ===========================================================================
//  hal_timer_get_us
// ===========================================================================
uint64_t hal_timer_get_us(hal_timer_id_t id)
{
    return timer_get_us(HAL_GROUP(id), HAL_NUM(id));
}

// ===========================================================================
//  hal_timer_get_ms
// ===========================================================================
uint64_t hal_timer_get_ms(hal_timer_id_t id)
{
    return timer_get_us(HAL_GROUP(id), HAL_NUM(id)) / 1000;
}

// ===========================================================================
//  hal_timer_periodo_cumplido
// ===========================================================================
bool hal_timer_periodo_cumplido(hal_timer_id_t id, uint64_t *ultimo_ms, uint32_t periodo_ms)
{
    uint64_t ahora = hal_timer_get_ms(id);

    if ((ahora - *ultimo_ms) >= periodo_ms)
    {
        *ultimo_ms = ahora;
        return true;
    }
    return false;
}

// ===========================================================================
//  hal_timer_marca_inicio
// ===========================================================================
void hal_timer_marca_inicio(hal_timer_id_t id, uint64_t *marca)
{
    *marca = timer_get_us(HAL_GROUP(id), HAL_NUM(id));
}

// ===========================================================================
//  hal_timer_elapsed_us
// ===========================================================================
uint64_t hal_timer_elapsed_us(hal_timer_id_t id, uint64_t *marca)
{
    return timer_get_us(HAL_GROUP(id), HAL_NUM(id)) - *marca;
}