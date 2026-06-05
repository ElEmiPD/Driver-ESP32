// FileName:        hal_pwm.h
// Dependencies:    pwm_2026.h
// Processor:       Tensilica Xtensa LX6 160 MHz
// Board:           Independiente de placa (portable)
// Program version: 2.0
// Company:         Instituto Tecnologico de Chihuahua
// Description:     HAL de PWM portable y escalable.
//                  Abstrae OPERACIONES de periférico sobre el driver pwm_2026,
//                  sin conocer conceptos de aplicación como LED, servo o motor.
//
//                  Regla de capas:
//                    DRIVER  (pwm_2026)   acceso directo a registros LEDC
//                    HAL     (hal_pwm)    operaciones de periférico
//                    BSP     (bsp_x)      mapeo a componentes de placa
//                    APP     (main)       lógica de negocio
//
//                  El HAL expone canales numerados HAL_PWM_CH_1…HAL_PWM_CH_8
//                  que mapean internamente a los canales High Speed del LEDC.
//                  Para agregar soporte a otro MCU basta con reimplementar
//                  hal_pwm.c; hal_pwm.h y las capas superiores no cambian.
//
//                  Configuración fija de este HAL:
//                    Modo:        PWM_HIGH_SPEED
//                    Timer:       PWM_TIMER_0  (compartido por todos los canales)
//                    Resolución:  8 bits (256 pasos, 0–255)
//                    Frecuencia:  5000 Hz (configurable en HAL_PWM_FREQ_HZ)
//
//                  Cambios v2.0 respecto a v1.0:
//                    + hal_pwm_set_duty_percent(): control de duty en porcentaje
//                      La conversión % -> valor absoluto se hace internamente
//                      usando la resolución declarada en HAL_PWM_RESOLUTION,
//                      sin que el BSP ni la APP tengan que conocerla.
//
// Autores:         Ana Paola Cardona Valenzuela
//                  Emiliano Perez Dyck
//                  Luis Adrian Anchondo Carreón
// Created:         04/06/2026
// Updated:         04/06/2026

#ifndef HAL_PWM_H
#define HAL_PWM_H

#include <stdint.h>
#include <stdbool.h>
#include "pwm_2026.h"

// ===========================================================================
//  Parámetros del periférico — ajustar aquí si se cambia MCU o requisito
// ===========================================================================
#define HAL_PWM_MODE        PWM_HIGH_SPEED
#define HAL_PWM_TIMER       PWM_TIMER_0
#define HAL_PWM_RESOLUTION  PWM_RES_8_BIT
#define HAL_PWM_FREQ_HZ     5000u
#define HAL_PWM_MAX_DUTY    255u   // 2^8 - 1
#define HAL_PWM_NUM_CH      8u     // canales disponibles en HS

// ===========================================================================
//  Identificadores de canal HAL
//  Ocultan el enum pwm_channel_t del driver a las capas superiores
// ===========================================================================

/**
 * @brief Canal PWM desde el punto de vista del HAL.
 *        HAL_PWM_CH_1 … HAL_PWM_CH_8 mapean a PWM_CHANNEL_0 … PWM_CHANNEL_7
 */
typedef enum {
    HAL_PWM_CH_1 = 0,
    HAL_PWM_CH_2 = 1,
    HAL_PWM_CH_3 = 2,
    HAL_PWM_CH_4 = 3,
    HAL_PWM_CH_5 = 4,
    HAL_PWM_CH_6 = 5,
    HAL_PWM_CH_7 = 6,
    HAL_PWM_CH_8 = 7,
} hal_pwm_ch_t;

// ===========================================================================
//  Tabla interna de descriptores de canal
//  Registra si cada canal fue inicializado y a qué GPIO está ligado.
//  Declarada extern aquí; definida UNA SOLA VEZ en hal_pwm.c.
//  Las capas superiores no deben acceder a esta tabla directamente.
// ===========================================================================
typedef struct {
    bool    init;   // true si el canal fue inicializado
    uint8_t gpio;   // GPIO físico al que está conectado
} hal_pwm_ch_desc_t;

extern hal_pwm_ch_desc_t hal_pwm_ch_table[HAL_PWM_NUM_CH];

// ===========================================================================
//  Códigos de retorno
// ===========================================================================
typedef enum {
    HAL_PWM_OK       =  0,
    HAL_PWM_ERR_CH   = -1,   // Canal fuera de rango
    HAL_PWM_ERR_INIT = -2,   // Canal no inicializado
    HAL_PWM_ERR_DUTY = -3,   // Valor de duty fuera de rango
} hal_pwm_err_t;

// ===========================================================================
//  API — Inicialización del periférico
// ===========================================================================

/**
 * @brief  Inicializa el periférico LEDC (reloj + timer compartido).
 *         Debe llamarse UNA sola vez antes de cualquier hal_pwm_ch_init().
 *         Llamadas adicionales no tienen efecto (flag interno).
 *
 * @return void
 */
void hal_pwm_init(void);

// ===========================================================================
//  API — Inicialización de canal
// ===========================================================================

/**
 * @brief  Inicializa un canal PWM y lo conecta a un pin GPIO.
 *         hal_pwm_init() debe haberse llamado previamente.
 *
 * @param  ch    Canal: HAL_PWM_CH_1 … HAL_PWM_CH_8
 * @param  gpio  Pin GPIO destino (0–33)
 * @param  duty  Duty inicial en valor absoluto (0 … HAL_PWM_MAX_DUTY)
 *
 * @return HAL_PWM_OK / HAL_PWM_ERR_CH / HAL_PWM_ERR_DUTY
 *
 * Ejemplo:
 *   hal_pwm_init();
 *   hal_pwm_ch_init(HAL_PWM_CH_1, 14, 0);   // canal 1 en IO14, duty=0
 */
hal_pwm_err_t hal_pwm_ch_init(hal_pwm_ch_t ch, uint8_t gpio, uint32_t duty);

// ===========================================================================
//  API — Control de duty cycle
// ===========================================================================

/**
 * @brief  Establece el duty cycle de un canal ya inicializado en valor absoluto.
 *         El cambio se aplica al siguiente ciclo PWM sin interrumpir la señal.
 *
 * @param  ch    Canal previamente inicializado
 * @param  duty  Nuevo duty (0 … HAL_PWM_MAX_DUTY)
 *
 * @return HAL_PWM_OK / HAL_PWM_ERR_CH / HAL_PWM_ERR_INIT / HAL_PWM_ERR_DUTY
 *
 * Ejemplo:
 *   hal_pwm_set_duty(HAL_PWM_CH_1, 128);   // ~50 %
 */
hal_pwm_err_t hal_pwm_set_duty(hal_pwm_ch_t ch, uint32_t duty);

/**
 * @brief  Establece el duty cycle de un canal en porcentaje (0–100 %).
 *         Convierte internamente el porcentaje al valor absoluto correcto
 *         usando HAL_PWM_RESOLUTION, sin que la capa superior necesite
 *         conocer la resolución del timer.
 *
 *         100 % equivale a HAL_PWM_MAX_DUTY (2^HAL_PWM_RESOLUTION - 1).
 *         Valores superiores a 100 se saturan a 100.
 *
 * @param  ch         Canal previamente inicializado
 * @param  porcentaje Duty cycle en porcentaje (0–100)
 *
 * @return HAL_PWM_OK / HAL_PWM_ERR_CH / HAL_PWM_ERR_INIT
 *
 * Ejemplo:
 *   hal_pwm_set_duty_percent(HAL_PWM_CH_1, 75);   // 75 %
 */
hal_pwm_err_t hal_pwm_set_duty_percent(hal_pwm_ch_t ch, uint8_t porcentaje);

/**
 * @brief  Lee el duty cycle que el hardware está aplicando actualmente.
 *         Usa el registro DUTY_R (solo lectura) que refleja el valor
 *         activo en el pin, no el buffer de escritura pendiente.
 *
 * @param  ch    Canal
 * @param  duty  Puntero donde se deposita el valor (0 … HAL_PWM_MAX_DUTY)
 *
 * @return HAL_PWM_OK / HAL_PWM_ERR_CH / HAL_PWM_ERR_INIT / HAL_PWM_ERR_DUTY
 *         (HAL_PWM_ERR_DUTY si duty == NULL)
 */
hal_pwm_err_t hal_pwm_get_duty(hal_pwm_ch_t ch, uint32_t *duty);

// ===========================================================================
//  API — Control de timer
// ===========================================================================

/**
 * @brief  Pausa el timer compartido (congela todos los canales).
 *         El nivel de salida de cada canal queda en el estado actual.
 *         Reanudar con hal_pwm_resume().
 *
 * @return void
 */
void hal_pwm_pause(void);

/**
 * @brief  Reanuda el timer compartido previamente pausado.
 *
 * @return void
 */
void hal_pwm_resume(void);

// ===========================================================================
//  API — Consulta de estado
// ===========================================================================

/**
 * @brief  Devuelve true si el canal fue inicializado con hal_pwm_ch_init().
 *
 * @param  ch  Canal
 * @return bool
 */
bool hal_pwm_is_init(hal_pwm_ch_t ch);

#endif /* HAL_PWM_H */