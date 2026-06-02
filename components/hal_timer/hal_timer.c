// FileName:        hal_timer.c                                                                                    
// Dependencies:    "hal_timer.h"                                                                                   
// Processor:       Tensilica Xtensa LX6 160 MHz                                                                                           
// Board:           ESP-WROOM-32                                                                                     
// Program version: 1.0                                                                                              
// Company:         Instituto Tecnologico de Chihuahua                                                               
// Description:     Implementación del HAL del timer
// Authors:         Ana Paola Cardona Valenzuela
//                  Luis Adrian Anchondo Carreón
//                  Emiliano Perez Dyck 
// Updated:         01/06/2026

#include "hal_timer.h"

/**
 * @brief Inicializa el hardware del timer
 * Llama a la capa de registros de bajo nivel para configurar el TIMG0
 */
void hal_timer_init(void)
{
    timer_init();
}

/**
 * @brief Genera un retardo bloqueante en milisegundos
 * * @param ms Tiempo de retardo deseado en milisegundos
 */
void hal_timer_delay_ms(uint32_t ms)
{
    timer_delay_ms(ms);
}

/**
 * @brief Genera un retardo bloqueante en microsegundos
 * * @param us Tiempo de retardo deseado en microsegundos
 */
void hal_timer_delay_us(uint32_t us)
{
    timer_delay_us(us);
}

/**
 * @brief Obtiene el valor absoluto y actual del contador del timer en microsegundos
 * * @return uint64_t Tiempo total transcurrido en microsegundos desde la inicialización
 */
uint64_t hal_timer_get_us(void)
{
    return timer_get_us();
}

/**
 * @brief Obtiene el valor absoluto y actual del contador del timer en milisegundos
 * Realiza la conversión dividiendo la lectura base de microsegundos entre 1000
 * * @return uint64_t Tiempo total transcurrido en milisegundos
 */
uint64_t hal_timer_get_ms(void)
{
    return timer_get_us() / 1000;
}

/**
 * @brief Evalúa si ha transcurrido un periodo de tiempo específico de forma no bloqueante
 * Si el periodo se ha cumplido, actualiza automáticamente la referencia de tiempo
 * * @param ultimo_ms Puntero a la variable que almacena la última marca de tiempo registrada (en ms)
 * @param periodo_ms Periodo de tiempo a verificar (en ms)
 * @return true Si la diferencia de tiempo es mayor o igual al periodo establecido.
 * @return false Si el periodo aún no se ha completado.
 */
bool hal_timer_periodo_cumplido(uint64_t *ultimo_ms, uint32_t periodo_ms)
{
    uint64_t ahora = hal_timer_get_ms();

    if ((ahora - *ultimo_ms) >= periodo_ms)
    {
        *ultimo_ms = ahora;
        return true;
    }
    return false;
}

/**
 * @brief Registra y almacena el valor actual del timer como una marca de tiempo inicial
 * * @param marca Puntero a la variable donde se guardará el tiempo actual (en us)
 */
void hal_timer_marca_inicio(uint64_t *marca)
{
    *marca = hal_timer_get_us();
}

/**
 * @brief Calcula la diferencia de tiempo transcurrido desde una marca de tiempo almacenada
 * * @param marca Puntero a la variable que contiene la marca de tiempo inicial (en us)
 * @return uint64_t Cantidad de microsegundos que han pasado desde la marca de inicio
 */
uint64_t hal_timer_elapsed_us(uint64_t *marca)
{
    return hal_timer_get_us() - *marca;
}