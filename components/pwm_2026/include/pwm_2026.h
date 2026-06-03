// FileName:        pwm_2026.h
// Dependencies:    None
// Processor:       ESP32 (Tensilica Xtensa LX6)
// Board:           ESP-WROOM-32
// Program version: 1.0
// Company:         Instituto Tecnologico de Chihuahua
// Description:     Declaración de macros, direcciones de memoria y
//                  prototipos de funciones de bajo nivel para el periférico
//                  LEDC (LED PWM Controller) del ESP32
//
//                  Arquitectura del LEDC (TRM v4.6, capítulo 14):
//
//                  El LEDC tiene dos grupos de velocidad:
//                    - High Speed (HS): 8 canales, 4 timers  -> soportado aquí
//                    - Low  Speed (LS): 8 canales, 4 timers  -> soportado aquí
//
//                  Cada canal se asocia a un timer del mismo grupo
//                  El timer genera la base de tiempo (frecuencia + resolución)
//                  El canal usa esa base para producir el ciclo de trabajo
//
//                  Flujo de configuración:
//                    1. Configurar el timer (prescaler + resolución)
//                    2. Asociar canal -> timer
//                    3. Configurar hpoint y duty del canal
//                    4. Activar la actualización del canal
//                    5. Conectar la salida al GPIO vía GPIO Matrix
//
//                  Fórmula de frecuencia PWM:
//                    f_pwm = f_clk / (DIV / 256) / 2^RES
//                    Con APB_CLK = 80 MHz, DIV y RES configurables
//
// Authors:         Ana Paola Cardona Valenzuela
//                  Luis Adrian Anchondo Carreón
//                  Emiliano Perez Dyck
// Created:         02/06/2026
// Updated:         02/06/2026

#ifndef PWM_2026_H
#define PWM_2026_H

#include <stdint.h>

// ===========================================================================
//  Macro de acceso directo a memoria 
// ===========================================================================
#ifndef HWREG32
#define HWREG32(x) (*((volatile uint32_t *)(x)))
#endif

// ===========================================================================
//  Dirección base del periférico LEDC
// ===========================================================================
#define LEDC_BASE  0x3FF59000

// ===========================================================================
//  Identificadores de grupo de velocidad
// ===========================================================================
/**
 * @brief Selecciona el modo de operación del LEDC
 *        HIGH_SPEED los registros se actualizan en el siguiente ciclo de reloj
 *        APB (para señales rápidas).
 * 
 *        LOW_SPEED los registros se actualizan solo cuando el software escribe 
 *        el bit de confirmación
 */
typedef enum{
    PWM_HIGH_SPEED = 0, // Grupo HS con offset base 0x0000
    PWM_LOW_SPEED = 1   // Grupo LS con offset base 0x00A0
}pwm_speed_mode_t;

// ===========================================================================
//  Identificadores de timer LEDC
//  Cada grupo tiene 4 timers independientes
// ===========================================================================
/**
 * @brief Timer LEDC dentro de un grupo de velocidad
 *        Cada timer controla la frecuencia y la resolución del PWM
 */
typedef enum {
    PWM_TIMER_0 = 0,
    PWM_TIMER_1 = 1,
    PWM_TIMER_2 = 2,
    PWM_TIMER_3 = 3
} pwm_timer_t;

// ===========================================================================
//  Identificadores de canal LEDC
//  Cada grupo tiene 8 canales independientes
// ===========================================================================
/**
 * @brief Canal LEDC dentro de un grupo de velocidad
 *        Cada canal genera una señal PWM independiente en un GPIO
 */
typedef enum {
    PWM_CHANNEL_0 = 0,
    PWM_CHANNEL_1 = 1,
    PWM_CHANNEL_2 = 2,
    PWM_CHANNEL_3 = 3,
    PWM_CHANNEL_4 = 4,
    PWM_CHANNEL_5 = 5,
    PWM_CHANNEL_6 = 6,
    PWM_CHANNEL_7 = 7
} pwm_channel_t;

// ===========================================================================
//  Resolución del contador (bits)
//  Determina la granularidad del duty cycle: 2^RES pasos posibles
//  A mayor resolución → menor frecuencia máxima posible
// ===========================================================================

/**
 * @brief Número de bits del contador PWM (resolución de duty cycle)
 *
 *        Ejemplos con APB_CLK = 80 MHz y CLOCK_DIV = 256:
 *          f_pwm = 80_000_000 / (CLOCK_DIV/256) / 2^RES
 *
 *          PWM_RES_1_BIT  -> ~40  MHz (2 pasos de duty)
 *          PWM_RES_8_BIT  -> ~312 kHz (256 pasos de duty)
 *          PWM_RES_10_BIT -> ~78  kHz (1024 pasos de duty)
 *          PWM_RES_13_BIT -> ~9.8 kHz (8192 pasos de duty)
 *          PWM_RES_16_BIT -> ~1.2 kHz (65536 pasos de duty)
 */
typedef enum {
    PWM_RES_1_BIT  = 1,
    PWM_RES_2_BIT  = 2,
    PWM_RES_3_BIT  = 3,
    PWM_RES_4_BIT  = 4,
    PWM_RES_5_BIT  = 5,
    PWM_RES_6_BIT  = 6,
    PWM_RES_7_BIT  = 7,
    PWM_RES_8_BIT  = 8,
    PWM_RES_9_BIT  = 9,
    PWM_RES_10_BIT = 10,
    PWM_RES_11_BIT = 11,
    PWM_RES_12_BIT = 12,
    PWM_RES_13_BIT = 13,
    PWM_RES_14_BIT = 14,
    PWM_RES_15_BIT = 15,
    PWM_RES_16_BIT = 16,
    PWM_RES_17_BIT = 17,
    PWM_RES_18_BIT = 18,
    PWM_RES_19_BIT = 19,
    PWM_RES_20_BIT = 20
} pwm_resolution_t;

// ===========================================================================
//  Offsets de registros de TIMER dentro de cada grupo
//  Stride entre timers dentro del mismo grupo: 0x0008 bytes
//
//  Grupo High Speed (HS) — base de timers:  LEDC_BASE + 0x0140
//    HSTIMER0_CONF  = 0x0140   HSTIMER1_CONF  = 0x0148   (diff = 0x0008)
//    HSTIMER0_VALUE = 0x0144   HSTIMER1_VALUE = 0x014C
//
//  Grupo Low Speed  (LS) — base de timers:  LEDC_BASE + 0x0160
//    LSTIMER0_CONF  = 0x0160   LSTIMER1_CONF  = 0x0168   (diff = 0x0008)
//    LSTIMER0_VALUE = 0x0164   LSTIMER1_VALUE = 0x016C
// ===========================================================================

#define LEDC_TIMER_STRIDE 0x0008 // Separación entre timers consecutivos

// Bases de la sección de timers según grupo
#define LEDC_HS_TIMER_BASE 0x0140  // Inicio de timers HS
#define LEDC_LS_TIMER_BASE 0x0160  // Inicio de timers LS

// Offsets internos de cada timer (relativos al inicio de ese timer)
#define LEDC_TIMER_CONF  0x0000  // Registro de configuración del timer
#define LEDC_TIMER_VALUE 0x0004  // Registro de valor actual del contador

/**
 * @brief Obtiene la dirección base del bloque de registros de un timer
 */
#define LEDC_TIMER_BASE(mode, timer) \
    (LEDC_BASE + ((mode) == PWM_HIGH_SPEED ? LEDC_HS_TIMER_BASE : LEDC_LS_TIMER_BASE) + (LEDC_TIMER_STRIDE * (uint32_t)(timer)))

#define LEDC_TIMER_CONF_REG(mode, timer) HWREG32(LEDC_TIMER_BASE(mode, timer) + LEDC_TIMER_CONF) // Para la configuración
#define LEDC_TIMER_VALUE_REG(mode, timer) HWREG32(LEDC_TIMER_BASE(mode, timer) + LEDC_TIMER_VALUE) // Para leer el contador

// ===========================================================================
//  Bits del registro  LEDC_HSTIMERx_CONF_REG
//  [24:24]  TICK_SEL  -> fuente de reloj: 1 = APB_CLK (80 MHz), 0 = REF_TICK
//  [23:23]  TIMER_RST -> 1 = resetea el contador
//  [22:22]  PAUSE     -> 1 = pausa el contador sin resetear
//  [21:5]   CLOCK_DIV -> prescaler en formato Q10.8 (10 bits de entero y 8 de decimal)
//                         Valor efectivo = CLOCK_DIV / 256
//                         Para 80 MHz sin dividir: CLOCK_DIV = 256
//  [4:0]    DUTY_RES  -> resolución del contador en bits (1–20)
// ===========================================================================
#define LEDC_TIMER_TICK_SEL_BIT (1UL << 24)  // 1 = APB_CLK
#define LEDC_TIMER_RST_BIT      (1UL << 23)  // Reset del contador
#define LEDC_TIMER_PAUSE_BIT    (1UL << 22)  // Pausa sin reset
#define LEDC_TIMER_CLKDIV_SHIFT 5 // Campo CLOCK_DIV empieza en bit 5
#define LEDC_TIMER_CLKDIV_MASK (0x3FFFFUL << LEDC_TIMER_CLKDIV_SHIFT)
#define LEDC_TIMER_DUTY_RES_SHIFT 0  // Campo DUTY_RES empieza en bit 0
#define LEDC_TIMER_DUTY_RES_MASK 0x1FUL // Bits [4:0]

// ===========================================================================
//  Offsets de registros de CANAL dentro de cada grupo
//  Fuente: ESP32 TRM v5.7, sección 28.4
//
//  Stride entre canales dentro del mismo grupo: 0x0014 bytes
//
//  Grupo High Speed (HS) — base de canales: LEDC_BASE + 0x0000
//    HSCH0_CONF0 = 0x0000  HSCH1_CONF0 = 0x0014  (diff = 0x0014)
//    HSCH0_HPOINT= 0x0004  HSCH1_HPOINT= 0x0018
//    HSCH0_DUTY  = 0x0008  HSCH1_DUTY  = 0x001C
//    HSCH0_CONF1 = 0x000C  HSCH1_CONF1 = 0x0020
//    HSCH0_DUTY_R= 0x0010  HSCH1_DUTY_R= 0x0024
//
//  Grupo Low Speed (LS)  — base de canales: LEDC_BASE + 0x00A0
//    Mismos offsets internos, stride idéntico
// ===========================================================================

#define LEDC_CH_STRIDE      0x0014UL  // Separación entre canales consecutivos

// Bases de la sección de canales según grupo
#define LEDC_HS_CH_BASE     0x0000UL  // Inicio canales HS
#define LEDC_LS_CH_BASE     0x00A0UL  // Inicio canales LS

// Offsets internos de cada canal
#define LEDC_CH_CONF0_OFF   0x0000UL  // Config 0: timer_sel, idle_lv, sig_out_en
#define LEDC_CH_HPOINT_OFF  0x0004UL  // Punto de subida (flanco rise)
#define LEDC_CH_DUTY_OFF    0x0008UL  // Duty cycle a aplicar
#define LEDC_CH_CONF1_OFF   0x000CUL  // Config 1: duty_start, duty_inc, duty_num…
#define LEDC_CH_DUTY_R_OFF  0x0010UL  // Duty cycle leído (valor actual aplicado)

/**
 * @brief Obtiene la dirección base del bloque de registros de un canal
 */
#define LEDC_CH_BASE(mode, ch) \
    (LEDC_BASE \
     + ((mode) == PWM_HIGH_SPEED ? LEDC_HS_CH_BASE : LEDC_LS_CH_BASE) \
     + (LEDC_CH_STRIDE * (uint32_t)(ch)))

#define LEDC_CH_CONF0_REG(mode, ch)  HWREG32(LEDC_CH_BASE(mode, ch) + LEDC_CH_CONF0_OFF)
#define LEDC_CH_HPOINT_REG(mode, ch) HWREG32(LEDC_CH_BASE(mode, ch) + LEDC_CH_HPOINT_OFF)
#define LEDC_CH_DUTY_REG(mode, ch)   HWREG32(LEDC_CH_BASE(mode, ch) + LEDC_CH_DUTY_OFF)
#define LEDC_CH_CONF1_REG(mode, ch)  HWREG32(LEDC_CH_BASE(mode, ch) + LEDC_CH_CONF1_OFF)
#define LEDC_CH_DUTY_R_REG(mode, ch) HWREG32(LEDC_CH_BASE(mode, ch) + LEDC_CH_DUTY_R_OFF)

// ===========================================================================
//  Bits del registro LEDC_HSCHx_CONF0 / LEDC_LSCHx_CONF0
//  [3:2]  TIMER_SEL  → qué timer usa el canal (0–3)
//  [2:2]  IDLE_LV    → nivel de salida cuando el canal está inactivo (0 o 1)
//  [1:1]  SIG_OUT_EN → habilita la salida del canal
//  [0:0]  (LS only)  OVF_NUM_RST_EN
// ===========================================================================
#define LEDC_CH_TIMER_SEL_SHIFT  2            // Campo TIMER_SEL en bits [3:2]
#define LEDC_CH_TIMER_SEL_MASK   (0x3UL << LEDC_CH_TIMER_SEL_SHIFT)
#define LEDC_CH_IDLE_LV_BIT      (1UL << 3)  // Nivel en idle
#define LEDC_CH_SIG_OUT_EN_BIT   (1UL << 2)  // Enable salida

// ===========================================================================
//  Bits del registro LEDC_HSCHx_CONF1 / LEDC_LSCHx_CONF1
//  [31]  DUTY_START → escribe 1 para confirmar la actualización del duty
//  (Low Speed requiere además escribir LEDC_LSTIMER_PARA_UP_BIT en CONF)
// ===========================================================================
#define LEDC_CH_DUTY_START_BIT   (1UL << 31)  // Activa la actualización del duty

// ===========================================================================
//  Registro global de configuración del LEDC
//  Fuente: ESP32 TRM v5.7, sección 28.4 — dirección 0x3FF59190
//
//  LEDC_CONF_REG controla la fuente de reloj del grupo Low Speed (LS).
//  Los timers HS siempre usan APB_CLK (80 MHz) y no dependen de este registro.
//  Los timers LS pueden usar dos fuentes, seleccionadas con el bit [0]:
//
//    Bit [0]  LEDC_APB_CLK_SEL:
//      0 → RTC8M_CLK (~8 MHz) — funciona en modo low-power / light sleep
//      1 → APB_CLK   (80 MHz) — máxima resolución y frecuencia, uso normal
//
//  Para uso normal (sin low-power) se debe escribir 1 en este bit antes de
//  inicializar cualquier timer LS. Si solo se usan timers HS, este registro
//  no es estrictamente necesario, pero escribirlo garantiza un estado conocido.
// ===========================================================================
#define LEDC_CONF_REG           HWREG32(0x3FF59190UL)
#define LEDC_APB_CLK_SEL_BIT    (1UL << 0)   // 1 = APB_CLK (80 MHz) para timers LS

// ===========================================================================
//  Registro de habilitación del reloj del periférico LEDC
//  Fuente: ESP32 TRM v5.7, sección 12 — DPort / Peripheral Clock Gating
//
//  DPORT_PERIP_CLK_EN_REG  = 0x3FF000C0
//  DPORT_PERIP_RST_EN_REG  = 0x3FF000C4
//  Bit 11 → LEDC_CLK_EN / LEDC_RST
// ===========================================================================
#define DPORT_PERIP_CLK_EN_REG  HWREG32(0x3FF000C0UL)
#define DPORT_PERIP_RST_EN_REG  HWREG32(0x3FF000C4UL)
#define LEDC_CLK_EN_BIT         (1UL << 11)

// ===========================================================================
//  Registro GPIO Matrix para ruteo de la señal LEDC hacia un pin físico
//  Fuente: ESP32 TRM v5.7, sección 6 — IO MUX / GPIO Matrix
//
//  GPIO_FUNCx_OUT_SEL_CFG_REG — un registro por cada GPIO (0–39)
//    Base:   0x3FF44530
//    Stride: 0x0004 bytes por GPIO
//    Bits [8:0] → índice de la señal periférica que se ruteará al pin
//
//  Señales LEDC de salida (signal index en la GPIO Matrix):
//    HS Channel 0–7: índices 71–78
//    LS Channel 0–7: índices 79–86
// ===========================================================================
#define GPIO_FUNC_OUT_SEL_BASE  0x3FF44530UL
#define GPIO_FUNC_OUT_REG(gpio) HWREG32(GPIO_FUNC_OUT_SEL_BASE + (0x0004UL * (uint32_t)(gpio)))

// Índices de señal en la GPIO Matrix para salidas LEDC (TRM tabla 6-2)
#define LEDC_HS_SIG_BASE  71U   // HS_CH0 = 71, HS_CH1 = 72 … HS_CH7 = 78
#define LEDC_LS_SIG_BASE  79U   // LS_CH0 = 79, LS_CH1 = 80 … LS_CH7 = 86

// ===========================================================================
//  Registro GPIO_ENABLE_REG — habilita el pin como salida
//  GPIO 0–31:  GPIO_ENABLE_REG   = 0x3FF44020
//  GPIO 32–39: GPIO_ENABLE1_REG  = 0x3FF44024
// ===========================================================================
#define GPIO_ENABLE_REG     HWREG32(0x3FF44020UL)
#define GPIO_ENABLE1_REG    HWREG32(0x3FF44024UL)

// ===========================================================================
//  Registro IO MUX para cada pin — selecciona función GPIO (función 2)
//  Fuente: ESP32 TRM v5.7, sección 6.10 — IO MUX Pin List
//
//  Cada pin tiene un registro propio; aquí solo se declara la macro de acceso.
//  La lista completa de direcciones físicas se maneja dentro del .c mediante
//  una tabla indexada por número de GPIO.
// ===========================================================================
#define IOMUX_PIN_FUNC_GPIO  2U  // Función 2 = GPIO Matrix (ruta al periférico)

// ===========================================================================
//  Constante de reloj fuente
// ===========================================================================
#define LEDC_APB_CLK_HZ  80000000UL  // APB_CLK = 80 MHz (fuente estándar LEDC)

// ===========================================================================
//  Prototipos de funciones
// ===========================================================================

/**
 * @brief  Habilita el reloj del periférico LEDC y libera el reset de hardware
 *         Debe llamarse una única vez antes de cualquier otra función PWM.
 *         Sin este paso el periférico no responde.
 *
 * @return void
 */
void pwm_ledc_clock_enable(void);

/**
 * @brief  Configura un timer LEDC con la frecuencia y resolución indicadas
 *
 *         Calcula automáticamente el prescaler (CLOCK_DIV) a partir de:
 *           CLOCK_DIV = (APB_CLK / freq_hz) / 2^resolution  * 256
 *         Selecciona APB_CLK (80 MHz) como fuente, resetea el contador
 *         y lo pone en marcha.
 *
 * @param  mode        Grupo de velocidad: PWM_HIGH_SPEED o PWM_LOW_SPEED
 * @param  timer       Timer a configurar: PWM_TIMER_0 … PWM_TIMER_3
 * @param  resolution  Resolución del contador en bits: PWM_RES_1_BIT … PWM_RES_20_BIT
 * @param  freq_hz     Frecuencia PWM deseada en Hz (ej. 1000 para 1 kHz)
 * @return void
 */
void pwm_timer_init(pwm_speed_mode_t mode, pwm_timer_t timer,
                    pwm_resolution_t resolution, uint32_t freq_hz);

/**
 * @brief  Asocia un canal a un timer y configura su ciclo de trabajo
 *
 *         Escribe CONF0 (timer_sel + sig_out_en), HPOINT y DUTY,
 *         luego activa la actualización con DUTY_START en CONF1.
 *         El canal queda listo para generar la señal PWM; falta solo
 *         conectarlo a un GPIO con pwm_gpio_bind().
 *
 * @param  mode    Grupo de velocidad: PWM_HIGH_SPEED o PWM_LOW_SPEED
 * @param  channel Canal a configurar: PWM_CHANNEL_0 … PWM_CHANNEL_7
 * @param  timer   Timer al que se asocia el canal
 * @param  duty    Valor de duty cycle (0 = 0 %, 2^resolution - 1 = 100 %)
 * @return void
 */
void pwm_channel_init(pwm_speed_mode_t mode, pwm_channel_t channel,
                      pwm_timer_t timer, uint32_t duty);

/**
 * @brief  Conecta la salida de un canal LEDC a un pin GPIO físico
 *
 *         Configura el IO MUX del pin en función GPIO y programa la
 *         GPIO Matrix para rutear la señal LEDC correspondiente.
 *
 * @param  mode    Grupo de velocidad del canal
 * @param  channel Canal LEDC cuya señal se llevará al GPIO
 * @param  gpio    Número de pin GPIO destino (0–33 para salida digital)
 * @return void
 */
void pwm_gpio_bind(pwm_speed_mode_t mode, pwm_channel_t channel, uint8_t gpio);

/**
 * @brief  Actualiza el ciclo de trabajo de un canal ya inicializado
 *
 *         Escribe el nuevo valor en DUTY y activa DUTY_START para que
 *         el hardware aplique el cambio al siguiente ciclo PWM.
 *
 * @param  mode    Grupo de velocidad del canal
 * @param  channel Canal a actualizar
 * @param  duty    Nuevo valor de duty cycle
 * @return void
 */
void pwm_set_duty(pwm_speed_mode_t mode, pwm_channel_t channel, uint32_t duty);

/**
 * @brief  Pausa el timer indicado (mantiene el nivel de salida actual)
 *         El contador se detiene sin reiniciarse; reanudar con pwm_timer_resume()
 *
 * @param  mode   Grupo de velocidad
 * @param  timer  Timer a pausar
 * @return void
 */
void pwm_timer_pause(pwm_speed_mode_t mode, pwm_timer_t timer);

/**
 * @brief  Reanuda un timer previamente pausado con pwm_timer_pause()
 *
 * @param  mode   Grupo de velocidad
 * @param  timer  Timer a reanudar
 * @return void
 */
void pwm_timer_resume(pwm_speed_mode_t mode, pwm_timer_t timer);

/**
 * @brief  Lee el valor actual del contador de un timer (solo lectura)
 *
 * @param  mode   Grupo de velocidad
 * @param  timer  Timer a leer
 * @return uint32_t  Valor del contador en el momento de la lectura
 */
uint32_t pwm_timer_get_value(pwm_speed_mode_t mode, pwm_timer_t timer);

#endif