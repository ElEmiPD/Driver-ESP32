// FileName:        hal_pwm.c
// Dependencies:    hal_pwm.h, pwm_2026.h
// Processor:       Tensilica Xtensa LX6 160 MHz
// Board:           Independiente de placa (portable)
// Program version: 2.0
// Company:         Instituto Tecnologico de Chihuahua
// Description:     Implementación del HAL de PWM.
//                  Todo acceso al hardware pasa por pwm_2026.
//                  Para portar a otro MCU, solo se reemplaza este .c;
//                  hal_pwm.h y las capas superiores no cambian.
//
//                  Cambios v2.0 respecto a v1.0:
//                    + hal_pwm_set_duty_percent(): convierte porcentaje a
//                      valor absoluto usando HAL_PWM_MAX_DUTY como techo.
//                      Saturación a 100 para evitar overflow. No requiere
//                      tabla de resolución por timer porque este HAL usa
//                      un único timer con resolución fija (HAL_PWM_RESOLUTION).
//
// Autores:         Ana Paola Cardona Valenzuela
//                  Emiliano Perez Dyck
//                  Luis Adrian Anchondo Carreón
// Created:         04/06/2026
// Updated:         04/06/2026

#include "hal_pwm.h"

// ===========================================================================
//  Tabla interna de descriptores de canal
//  NULL-init: todos sin inicializar al arranque
// ===========================================================================
hal_pwm_ch_desc_t hal_pwm_ch_table[HAL_PWM_NUM_CH] = {
    {false, 0}, {false, 0}, {false, 0}, {false, 0},
    {false, 0}, {false, 0}, {false, 0}, {false, 0},
};

// Flag interno para evitar inicializar el reloj y el timer más de una vez
static bool hal_pwm_periph_init = false;

// ===========================================================================
//  Helpers internos de validación
// ===========================================================================

static hal_pwm_err_t validate_ch(hal_pwm_ch_t ch)
{
    if ((uint8_t)ch >= HAL_PWM_NUM_CH)
    {
        return HAL_PWM_ERR_CH;
    }
    return HAL_PWM_OK;
}

static hal_pwm_err_t validate_ch_init(hal_pwm_ch_t ch)
{
    hal_pwm_err_t err = validate_ch(ch);
    if (err != HAL_PWM_OK)
    {
        return err;
    }
    if (!hal_pwm_ch_table[ch].init)
    {
        return HAL_PWM_ERR_INIT;
    }
    return HAL_PWM_OK;
}

// ===========================================================================
//  hal_pwm_init
// ===========================================================================
void hal_pwm_init(void)
{
    if (hal_pwm_periph_init)
    {
        return;  // Idempotente: segunda llamada no tiene efecto
    }

    pwm_ledc_clock_enable();
    pwm_timer_init(HAL_PWM_MODE, HAL_PWM_TIMER, HAL_PWM_RESOLUTION, HAL_PWM_FREQ_HZ);

    hal_pwm_periph_init = true;
}

// ===========================================================================
//  hal_pwm_ch_init
// ===========================================================================
hal_pwm_err_t hal_pwm_ch_init(hal_pwm_ch_t ch, uint8_t gpio, uint32_t duty)
{
    hal_pwm_err_t err = validate_ch(ch);
    if (err != HAL_PWM_OK)
    {
        return err;
    }
    if (duty > HAL_PWM_MAX_DUTY)
    {
        return HAL_PWM_ERR_DUTY;
    }

    pwm_channel_t drv_ch = (pwm_channel_t)ch;

    pwm_channel_init(HAL_PWM_MODE, drv_ch, HAL_PWM_TIMER, duty);
    pwm_gpio_bind(HAL_PWM_MODE, drv_ch, gpio);

    hal_pwm_ch_table[ch].init = true;
    hal_pwm_ch_table[ch].gpio = gpio;

    return HAL_PWM_OK;
}

// ===========================================================================
//  hal_pwm_set_duty
// ===========================================================================
hal_pwm_err_t hal_pwm_set_duty(hal_pwm_ch_t ch, uint32_t duty)
{
    hal_pwm_err_t err = validate_ch_init(ch);
    if (err != HAL_PWM_OK)
    {
        return err;
    }
    if (duty > HAL_PWM_MAX_DUTY)
    {
        return HAL_PWM_ERR_DUTY;
    }

    pwm_set_duty(HAL_PWM_MODE, (pwm_channel_t)ch, duty);
    return HAL_PWM_OK;
}

// ===========================================================================
//  hal_pwm_set_duty_percent
// ===========================================================================
hal_pwm_err_t hal_pwm_set_duty_percent(hal_pwm_ch_t ch, uint8_t porcentaje)
{
    hal_pwm_err_t err = validate_ch_init(ch);
    if (err != HAL_PWM_OK)
    {
        return err;
    }

    // Saturar a 100 % para evitar overflow en la multiplicación
    if (porcentaje > 100U)
    {
        porcentaje = 100U;
    }

    // Conversión: duty = porcentaje * HAL_PWM_MAX_DUTY / 100
    //
    // Se usa uint32_t para la multiplicación intermedia. El máximo posible es
    // 100 * 255 = 25 500, que cabe holgadamente en uint32_t sin overflow.
    // HAL_PWM_MAX_DUTY se deriva de HAL_PWM_RESOLUTION definida en el .h;
    // si se cambia la resolución basta con actualizar ese #define y este
    // cálculo sigue siendo correcto sin tocar el BSP ni la APP.
    uint32_t duty = ((uint32_t)porcentaje * HAL_PWM_MAX_DUTY) / 100U;

    pwm_set_duty(HAL_PWM_MODE, (pwm_channel_t)ch, duty);
    return HAL_PWM_OK;
}

// ===========================================================================
//  hal_pwm_get_duty
// ===========================================================================
hal_pwm_err_t hal_pwm_get_duty(hal_pwm_ch_t ch, uint32_t *duty)
{
    hal_pwm_err_t err = validate_ch_init(ch);
    if (err != HAL_PWM_OK)
    {
        return err;
    }
    if (duty == NULL)
    {
        return HAL_PWM_ERR_DUTY;  // Puntero nulo equivale a argumento inválido
    }

    // LEDC_CH_DUTY_R_REG es el registro de solo lectura que refleja el duty
    // que el hardware está aplicando actualmente en el pin. Es distinto de
    // LEDC_CH_DUTY_REG, que es el buffer donde el software escribe el nuevo
    // valor pero que puede no haberse transferido al hardware todavía.
    // El registro almacena el valor en formato Q21.4 (4 bits fraccionarios),
    // por lo que se desplaza >> 4 para recuperar el entero original.
    *duty = LEDC_CH_DUTY_R_REG(HAL_PWM_MODE, (pwm_channel_t)ch) >> 4;
    return HAL_PWM_OK;
}

// ===========================================================================
//  hal_pwm_pause / hal_pwm_resume
// ===========================================================================
void hal_pwm_pause(void)
{
    pwm_timer_pause(HAL_PWM_MODE, HAL_PWM_TIMER);
}

void hal_pwm_resume(void)
{
    pwm_timer_resume(HAL_PWM_MODE, HAL_PWM_TIMER);
}

// ===========================================================================
//  hal_pwm_is_init
// ===========================================================================
bool hal_pwm_is_init(hal_pwm_ch_t ch)
{
    if ((uint8_t)ch >= HAL_PWM_NUM_CH)
    {
        return false;
    }
    return hal_pwm_ch_table[ch].init;
}