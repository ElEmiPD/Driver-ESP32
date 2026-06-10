// FileName:        pwm_2026.c
// Dependencies:    "pwm_2026.h"
// Processor:       ESP32 (Tensilica Xtensa LX6)
// Board:           ESP-WROOM-32
// Program version: 1.0
// Company:         Instituto Tecnologico de Chihuahua
// Description:     Implementación de bajo nivel del periférico LEDC del ESP32
//                  Accede directamente a registros de hardware, siguiendo el 
//                  mismo estilo que el driver de timers del proyecto
//
//                  Secuencia de inicialización:
//                    1. pwm_ledc_clock_enable()  habilita el periférico
//                    2. pwm_timer_init(...)   configura frecuencia + resolución
//                    3. pwm_channel_init(...)  asocia canal a timer + duty
//                    4. pwm_gpio_bind(...)  ruteado a pin físico
//
// Authors:         Ana Paola Cardona Valenzuela
//                  Emiliano Perez Dyck
//                  Luis Adrian Anchondo Carreón
// Created:         02/06/2026
// Updated:         05/06/2026

#include "pwm_2026.h"

// ===========================================================================
//  pwm_ledc_clock_enable
// ===========================================================================
void pwm_ledc_clock_enable(void)
{
    // Paso 1: liberar el reset del periférico LEDC
    // Si estuviera en reset, habilitar el reloj no tendría efecto
    DPORT_PERIP_RST_EN_REG &= ~LEDC_CLK_EN_BIT;

    // Paso 2: habilitar el reloj del periférico
    // Sin este paso todos los registros LEDC ignoran las escrituras
    DPORT_PERIP_CLK_EN_REG |= LEDC_CLK_EN_BIT;

    // Paso 3: seleccionar APB_CLK (80 MHz) como fuente para los timers LS
    //
    // LEDC_CONF_REG bit [0] = LEDC_APB_CLK_SEL:
    //   1 -> APB_CLK (80 MHz)  requerido para uso normal y máxima resolución
    //   0 -> RTC8M_CLK (~8 MHz)  solo necesario para operar en light sleep

    LEDC_CONF_REG |= LEDC_APB_CLK_SEL_BIT;
}

// ===========================================================================
//  pwm_timer_init
// ===========================================================================
void pwm_timer_init(pwm_speed_mode_t mode, pwm_timer_t timer,
                    pwm_resolution_t resolution, uint32_t freq_hz)
{
    // -----------------------------------------------------------------------
    // Calcular el prescaler CLOCK_DIV
    //
    // El campo CLOCK_DIV es de 18 bits en formato Q10.8:
    //   divisor efectivo = CLOCK_DIV / 256
    //
    // Frecuencia PWM: f = APB_CLK / (CLOCK_DIV/256) / 2^RES
    // Despejando:     CLOCK_DIV = (APB_CLK / f / 2^RES) * 256
    // -----------------------------------------------------------------------
    uint64_t div_q8 = ((uint64_t)LEDC_APB_CLK_HZ << 8) / ((uint64_t)freq_hz * ((uint64_t)1UL << (uint32_t)resolution));

    // Saturar al rango permitido por el campo de 18 bits en caso de que se solicite uan frecuencia muy baja
    if (div_q8 > 0x3FFFFUL) div_q8 = 0x3FFFFUL;
    if (div_q8 < 1UL) div_q8 = 1UL;

    // -----------------------------------------------------------------------
    // Construir y escribir el registro CONF del timer
    //
    // Se resetea el contador (TIMER_RST) para garantizar que arranque desde 0
    // En High Speed el bit de reset se auto-limpia; en Low Speed persiste un
    // ciclo adicional, pero el efecto es el mismo
    // -----------------------------------------------------------------------
    uint32_t conf = 0;
    conf |= LEDC_TIMER_TICK_SEL_BIT; // Fuente: APB_CLK
    conf |= ((uint32_t)div_q8 << LEDC_TIMER_CLKDIV_SHIFT); // Prescaler calculado
    conf |= ((uint32_t)resolution & LEDC_TIMER_DUTY_RES_MASK);// Resolución en bits
    conf |= LEDC_TIMER_RST_BIT; // Reset del contador

    LEDC_TIMER_CONF_REG(mode, timer) = conf;

    // Quitar el bit de reset para que el contador comience a contar
    LEDC_TIMER_CONF_REG(mode, timer) &= ~LEDC_TIMER_RST_BIT;
}

// ===========================================================================
//  pwm_channel_init
// ===========================================================================
void pwm_channel_init(pwm_speed_mode_t mode, pwm_channel_t channel,
                      pwm_timer_t timer, uint32_t duty)
{
    // ----------------------------------------------------------------------- 
    // CONF0: asociar el canal al timer y habilitar la salida
    //
    // Bits [1:0] TIMER_SEL -> índice del timer (0–3)
    // Bit  [2]   SIG_OUT_EN -> habilita la salida del canal
    // Bit  [3]   IDLE_LV  -> nivel en idle = 0 (Low)
    // -----------------------------------------------------------------------
    uint32_t conf0 = 0;
    conf0 |= ((uint32_t)timer << LEDC_CH_TIMER_SEL_SHIFT) & LEDC_CH_TIMER_SEL_MASK;
    conf0 |= LEDC_CH_SIG_OUT_EN_BIT;
    // IDLE_LV = 0 (nivel bajo en idle)  valor por defecto tras la máscara

    LEDC_CH_CONF0_REG(mode, channel) = conf0;

    // -----------------------------------------------------------------------
    // HPOINT: punto de subida del pulso PWM
    //
    // El pulso sube cuando el contador llega a HPOINT
    // Poniéndolo en 0 el flanco sube al inicio de cada ciclo, que es
    // el comportamiento más sencillo y predecible
    // -----------------------------------------------------------------------
    LEDC_CH_HPOINT_REG(mode, channel) = 0;

    // -----------------------------------------------------------------------
    // DUTY: ciclo de trabajo
    //
    // El campo DUTY en el registro es de 25 bits; los 4 bits menos
    // significativos son fraccionarios (Q21.4). Para simplificar el driver
    // se trabaja en entero: se desplaza el valor 4 posiciones a la izquierda
    // -----------------------------------------------------------------------
    LEDC_CH_DUTY_REG(mode, channel) = (duty << 4);

    // -----------------------------------------------------------------------
    // CONF1: activar la actualización del duty
    //
    // DUTY_START (bit 31) ordena al hardware aplicar el valor de DUTY al
    // siguiente ciclo del contador. En High Speed se auto-limpia; en
    // Low Speed el driver debe limpiarlo manualmente (se hace aquí mismo)
    // -----------------------------------------------------------------------
    LEDC_CH_CONF1_REG(mode, channel) = LEDC_CH_DUTY_START_BIT;

    // Para Low Speed: limpiar el bit de confirmación después de escribirlo
    // (el hardware lo necesita en flanco, no en nivel)
    if (mode == PWM_LOW_SPEED) {
        LEDC_CH_CONF1_REG(mode, channel) &= ~LEDC_CH_DUTY_START_BIT;
    }
}

// ===========================================================================
//  pwm_gpio_bind
// ===========================================================================
void pwm_gpio_bind(pwm_speed_mode_t mode, pwm_channel_t channel, uint8_t gpio)
{
    // -----------------------------------------------------------------------
    // Pasos 1 y 2: configurar IO MUX en función GPIO Matrix y habilitar salida
    //
    // gpio_config_out() de gpio_2026 realiza exactamente estas dos operaciones:
    //   - Escribe MUX_MCU_SEL_GPIO (función 2) en GPIO_MUX_REGS[pin]
    //   - Habilita el pin como salida en GPIO_ENABLE
    //   - Valida que el pin exista y sea <= 33 (solo salida)
    //
    // Se construye un gpio_pin_t temporal con los campos mínimos que necesita
    // gpio_config_out(): pin, is_output y logic (los demás no se usan)
    // -----------------------------------------------------------------------
    gpio_pin_t pin_cfg = {
        .pin       = gpio,
        .is_output = true,
        .logic     = GPIO_ACTIVE_HIGH,
        .pull_mode = GPIO_FLOATING,
        .int_type  = INT_DESHABILITADA
    };
    gpio_config_out(&pin_cfg);

    // -----------------------------------------------------------------------
    // Paso 3: rutear la señal LEDC al GPIO en la GPIO Matrix
    //
    // Este paso es exclusivo del driver PWM  gpio_2026 no lo hace porque
    // es específico del periférico LEDC. Se escribe el índice de señal en
    // GPIO_FUNCx_OUT_SEL_CFG_REG para que la GPIO Matrix conecte la salida
    // del canal LEDC al pin físico
    //
    // Índices de señal:
    //   HS Channel n -> 71 + n  (HS_CH0=71, HS_CH1=72 ... HS_CH7=78)
    //   LS Channel n -> 79 + n  (LS_CH0=79, LS_CH1=80 ... LS_CH7=86)
    // -----------------------------------------------------------------------
    uint32_t sig_idx = (mode == PWM_HIGH_SPEED)
                       ? (LEDC_HS_SIG_BASE + (uint32_t)channel)
                       : (LEDC_LS_SIG_BASE + (uint32_t)channel);

    GPIO_FUNC_OUT_REG(gpio) = sig_idx;
}

// ===========================================================================
//  pwm_set_duty
// ===========================================================================
void pwm_set_duty(pwm_speed_mode_t mode, pwm_channel_t channel, uint32_t duty)
{
    // Actualizar el registro de duty (formato Q21.4: desplazar 4 bits)
    LEDC_CH_DUTY_REG(mode, channel) = (duty << 4);

    // Activar la actualización. En HS el hardware lo auto-limpia;
    // en LS hay que limpiarlo manualmente tras escritura.
    LEDC_CH_CONF1_REG(mode, channel) |= LEDC_CH_DUTY_START_BIT;

    if (mode == PWM_LOW_SPEED) {
        LEDC_CH_CONF1_REG(mode, channel) &= ~LEDC_CH_DUTY_START_BIT;
    }
}

// ===========================================================================
//  pwm_timer_pause
// ===========================================================================
void pwm_timer_pause(pwm_speed_mode_t mode, pwm_timer_t timer)
{
    // El bit PAUSE congela el contador sin reiniciarlo. El nivel de salida
    // del canal queda en el estado en que estaba cuando se pausó
    LEDC_TIMER_CONF_REG(mode, timer) |= LEDC_TIMER_PAUSE_BIT;
}

// ===========================================================================
//  pwm_timer_resume
// ===========================================================================
void pwm_timer_resume(pwm_speed_mode_t mode, pwm_timer_t timer)
{
    LEDC_TIMER_CONF_REG(mode, timer) &= ~LEDC_TIMER_PAUSE_BIT;
}

// ===========================================================================
//  pwm_timer_get_value
// ===========================================================================
uint32_t pwm_timer_get_value(pwm_speed_mode_t mode, pwm_timer_t timer)
{
    // El registro VALUE refleja el valor actual del contador sin necesidad
    // de disparar una captura  a diferencia del timer general (TIMG)
    return LEDC_TIMER_VALUE_REG(mode, timer);
}