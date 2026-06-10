// FileName:        bsp_panther48.c
// Dependencies:    bsp_panther48.h
// Processor:       Tensilica Xtensa LX6 160 MHz
// Board:           ESP32 ITCH (Panther48)  REV 1.0
// Program version: 4.0
// Company:         Instituto Tecnologico de Chihuahua
// Description:     Implementación del BSP.
//                  Contiene TODO el conocimiento de la placa:
//                    - qué pin es cada componente
//                    - qué lógica activa tiene cada circuito
//                    - qué resistencia de pull necesita cada entrada
//                    - qué flanco dispara cada interrupción
//                  Las capas superiores (APP) no deben conocer nada de esto.
//
//                  Cambios v4.0 respecto a v3.0:
//                    + bsp_pwm_rgb_set_percent(): recibe brillo 0–100 % por
//                      canal. Invierte el porcentaje para compensar la lógica
//                      activa baja del LED RGB, luego llama a
//                      hal_pwm_set_duty_percent(). Esta función es más
//                      legible que bsp_pwm_rgb_set() para la APP porque
//                      el 0 siempre significa apagado y el 100 siempre
//                      significa máximo brillo, independientemente de la
//                      lógica del circuito.
//                    + bsp_pwm_rgb_set_ch_percent(): variante por canal
//                      en porcentaje de bsp_pwm_rgb_set_ch().
//
//                  Corrección de lógica activa baja en funciones de porcentaje:
//                    LED ánodo común -> cátodo al GPIO -> lógica invertida:
//                      brillo = 0   -> pin HIGH -> LED apagado -> duty_pct = 100
//                      brillo = 100 -> pin LOW  -> LED máximo  -> duty_pct = 0
//                    duty_pct_hw = 100 - brillo
//
// Autores:         Ana Paola Cardona Valenzuela
//                  Emiliano Perez Dyck
//                  Luis Adrian Anchondo Carreón
// Created:         04/06/2026
// Updated:         04/06/2026

#include "bsp_panther48.h"

// ===========================================================================
//  Tabla interna: pin físico de cada LED (índice 1-based)
// ===========================================================================
static const uint8_t led_pin[BOARD_LED_COUNT + 1] = {
    0,                 // [0] sin uso  índices 1-based
    BOARD_LED1_PIN,    // [1] IO2
    BOARD_LED2_PIN,    // [2] IO4
    BOARD_LED3_PIN,    // [3] IO16
    BOARD_LED4_PIN,    // [4] IO17
    BOARD_LED5_PIN,    // [5] IO5
};

// ===========================================================================
//  Helper: valida índice de LED semántico (1…BOARD_LED_COUNT)
// ===========================================================================
static inline bool led_idx_valid(uint8_t led)
{
    return (led >= 1u && led <= BOARD_LED_COUNT);
}

// ===========================================================================
//  Helper interno: convierte brillo % (0–100) a duty absoluto del LEDC
//
//  LED ánodo común -> lógica invertida:
//    brillo = 0   -> pin HIGH -> LED apagado -> duty = MAX
//    brillo = 100 -> pin LOW  -> LED máximo  -> duty = 0
//
//  duty = MAX_DUTY - (brillo * MAX_DUTY / 100)
// ===========================================================================
static inline uint32_t brillo_a_duty(uint8_t brillo)
{
    if (brillo > BSP_PWM_BRILLO_MAX) brillo = BSP_PWM_BRILLO_MAX;
    return HAL_PWM_MAX_DUTY - ((uint32_t)brillo * HAL_PWM_MAX_DUTY / BSP_PWM_BRILLO_MAX);
}

// ===========================================================================
//  Helper interno: invierte porcentaje para lógica activa baja
//
//  brillo_pct_invertido = 100 - brillo
//
//  Usado por las variantes _percent del PWM RGB. Al invertir el porcentaje
//  antes de pasarlo a hal_pwm_set_duty_percent() se logra que:
//    brillo = 0   -> duty_hw = 100 % -> GPIO alto -> LED apagado  ✓
//    brillo = 100 -> duty_hw =   0 % -> GPIO bajo -> LED encendido ✓
// ===========================================================================
static inline uint8_t invertir_pct(uint8_t brillo)
{
    if (brillo > 100U) brillo = 100U;
    return (uint8_t)(100U - brillo);
}

// ===========================================================================
//  bsp_init
// ===========================================================================
void bsp_init(void)
{
    hal_timer_init(HAL_TIMER_1); wdt_disable_all();

    for (uint8_t i = 1; i <= BOARD_LED_COUNT; i++)
    {
        hal_gpio_output_init(led_pin[i], BOARD_LED_LOGIC);
    }

    hal_gpio_output_init(BOARD_RGB_R_PIN, BOARD_RGB_LOGIC);
    hal_gpio_output_init(BOARD_RGB_G_PIN, BOARD_RGB_LOGIC);
    hal_gpio_output_init(BOARD_RGB_B_PIN, BOARD_RGB_LOGIC);

    hal_gpio_input_init(BOARD_BTN_SW1_PIN,  BOARD_BTN_PULL, BOARD_BTN_LOGIC);
    hal_gpio_input_init(BOARD_BTN_SW2_PIN,  BOARD_BTN_PULL, BOARD_BTN_LOGIC);
    hal_gpio_input_init(BOARD_BTN_BOOT_PIN, BOARD_BTN_PULL, BOARD_BTN_LOGIC);
}

// ===========================================================================
//  API GENÉRICA  bsp_pin_mode
// ===========================================================================
void bsp_pin_mode(uint8_t pin, bsp_pin_mode_t mode)
{
    switch (mode)
    {
        case BSP_OUTPUT:
            hal_gpio_output_init(pin, GPIO_ACTIVE_HIGH);
            break;

        case BSP_OUTPUT_INVERTED:
            hal_gpio_output_init(pin, GPIO_ACTIVE_LOW);
            break;

        case BSP_INPUT:
            hal_gpio_input_init(pin, GPIO_FLOATING, GPIO_ACTIVE_HIGH);
            break;

        case BSP_INPUT_PULLUP:
            hal_gpio_input_init(pin, GPIO_PULLUP, GPIO_ACTIVE_HIGH);
            break;

        case BSP_INPUT_PULLDOWN:
            hal_gpio_input_init(pin, GPIO_PULLDOWN, GPIO_ACTIVE_HIGH);
            break;

        default:
            break;
    }
}

// ===========================================================================
//  API GENÉRICA  bsp_digital_write
// ===========================================================================
void bsp_digital_write(uint8_t pin, bsp_pin_level_t level)
{
    hal_gpio_output_set(pin, (bool)level);
}

// ===========================================================================
//  API GENÉRICA  bsp_digital_read
// ===========================================================================
bsp_pin_level_t bsp_digital_read(uint8_t pin)
{
    bool state = false;
    hal_gpio_input_read(pin, &state);
    return state ? BSP_HIGH : BSP_LOW;
}

// ===========================================================================
//  API GENÉRICA  Interrupciones
// ===========================================================================
void bsp_attach_interrupt(uint8_t pin, gpio_int_type_t int_type,
                          gpio_isr_callback_t cb, void *arg)
{
    hal_gpio_irq_attach(pin, int_type, cb, arg);
}

void bsp_detach_interrupt(uint8_t pin)
{
    hal_gpio_irq_detach(pin);
}

// ===========================================================================
//  API SEMÁNTICA  LEDs
// ===========================================================================
void bsp_led_on(uint8_t led)
{
    if (!led_idx_valid(led)) return;
    hal_gpio_output_set(led_pin[led], true);
}

void bsp_led_off(uint8_t led)
{
    if (!led_idx_valid(led)) return;
    hal_gpio_output_set(led_pin[led], false);
}

void bsp_led_toggle(uint8_t led)
{
    if (!led_idx_valid(led)) return;
    hal_gpio_output_toggle(led_pin[led]);
}

void bsp_led_blink(uint8_t led, uint32_t ms)
{
    if (!led_idx_valid(led)) return;
    hal_gpio_output_set(led_pin[led], true);
    hal_timer_delay_ms(HAL_TIMER_1, ms);
    hal_gpio_output_set(led_pin[led], false);
}

void bsp_led_all_on(void)
{
    for (uint8_t i = 1u; i <= BOARD_LED_COUNT; i++)
        hal_gpio_output_set(led_pin[i], true);
}

void bsp_led_all_off(void)
{
    for (uint8_t i = 1u; i <= BOARD_LED_COUNT; i++)
        hal_gpio_output_set(led_pin[i], false);
}

// ===========================================================================
//  API SEMÁNTICA  LED RGB — digital
// ===========================================================================
void bsp_rgb_set(bool r, bool g, bool b)
{
    hal_gpio_output_set(BOARD_RGB_R_PIN, r);
    hal_gpio_output_set(BOARD_RGB_G_PIN, g);
    hal_gpio_output_set(BOARD_RGB_B_PIN, b);
}

void bsp_rgb_off(void)
{
    hal_gpio_output_set(BOARD_RGB_R_PIN, false);
    hal_gpio_output_set(BOARD_RGB_G_PIN, false);
    hal_gpio_output_set(BOARD_RGB_B_PIN, false);
}

// ===========================================================================
//  API SEMÁNTICA  Botones (polling)
// ===========================================================================
bool bsp_sw1_pressed(void)
{
    bool s = false;
    hal_gpio_input_read(BOARD_BTN_SW1_PIN, &s);
    return s;
}

bool bsp_sw2_pressed(void)
{
    bool s = false;
    hal_gpio_input_read(BOARD_BTN_SW2_PIN, &s);
    return s;
}

bool bsp_boot_pressed(void)
{
    bool s = false;
    hal_gpio_input_read(BOARD_BTN_BOOT_PIN, &s);
    return s;
}

// ===========================================================================
//  API SEMÁNTICA  Botones (interrupción)
// ===========================================================================
void bsp_sw1_irq_attach(gpio_isr_callback_t cb, void *arg)
{
    hal_gpio_irq_attach(BOARD_BTN_SW1_PIN, BOARD_BTN_IRQ_EDGE, cb, arg);
}

void bsp_sw1_irq_detach(void)
{
    hal_gpio_irq_detach(BOARD_BTN_SW1_PIN);
}

void bsp_sw2_irq_attach(gpio_isr_callback_t cb, void *arg)
{
    hal_gpio_irq_attach(BOARD_BTN_SW2_PIN, BOARD_BTN_IRQ_EDGE, cb, arg);
}

void bsp_sw2_irq_detach(void)
{
    hal_gpio_irq_detach(BOARD_BTN_SW2_PIN);
}

// ===========================================================================
//  Retardos y tiempo
// ===========================================================================
void bsp_delay_ms(uint32_t ms)      { hal_timer_delay_ms(HAL_TIMER_1, ms); }
void bsp_delay_us(uint32_t us)      { hal_timer_delay_us(HAL_TIMER_1, us); }
uint64_t bsp_get_ms(void)           { return hal_timer_get_ms(HAL_TIMER_1); }
uint64_t bsp_get_us(void)           { return hal_timer_get_us(HAL_TIMER_1); }

// ===========================================================================
//  bsp_pwm_rgb_init
// ===========================================================================
void bsp_pwm_rgb_init(void)
{
    hal_pwm_init();  // Idempotente

    // Inicializar los tres canales apagados (duty = MAX por lógica invertida)
    hal_pwm_ch_init(BSP_PWM_CH_R, BOARD_RGB_R_PIN, HAL_PWM_MAX_DUTY);
    hal_pwm_ch_init(BSP_PWM_CH_G, BOARD_RGB_G_PIN, HAL_PWM_MAX_DUTY);
    hal_pwm_ch_init(BSP_PWM_CH_B, BOARD_RGB_B_PIN, HAL_PWM_MAX_DUTY);
}

// ===========================================================================
//  bsp_pwm_rgb_set  (valor absoluto, API v3.0 — sin cambios)
// ===========================================================================
void bsp_pwm_rgb_set(uint8_t r, uint8_t g, uint8_t b)
{
    hal_pwm_set_duty(BSP_PWM_CH_R, brillo_a_duty(r));
    hal_pwm_set_duty(BSP_PWM_CH_G, brillo_a_duty(g));
    hal_pwm_set_duty(BSP_PWM_CH_B, brillo_a_duty(b));
}

// ===========================================================================
//  bsp_pwm_rgb_off
// ===========================================================================
void bsp_pwm_rgb_off(void)
{
    bsp_pwm_rgb_set(0, 0, 0);
}

// ===========================================================================
//  bsp_pwm_rgb_set_ch  (valor absoluto, API v3.0 — sin cambios)
// ===========================================================================
void bsp_pwm_rgb_set_ch(hal_pwm_ch_t ch, uint8_t brillo)
{
    hal_pwm_set_duty(ch, brillo_a_duty(brillo));
}

// ===========================================================================
//  bsp_pwm_rgb_set_percent  (NUEVO v4.0)
//
//  Recibe brillo 0–100 % por canal.
//  Invierte el porcentaje para compensar la lógica activa baja antes
//  de delegar a hal_pwm_set_duty_percent().
//  La APP ve siempre 0 = apagado y 100 = máximo brillo.
// ===========================================================================
void bsp_pwm_rgb_set_percent(uint8_t r, uint8_t g, uint8_t b)
{
    hal_pwm_set_duty_percent(BSP_PWM_CH_R, invertir_pct(r));
    hal_pwm_set_duty_percent(BSP_PWM_CH_G, invertir_pct(g));
    hal_pwm_set_duty_percent(BSP_PWM_CH_B, invertir_pct(b));
}

// ===========================================================================
//  bsp_pwm_rgb_set_ch_percent  (NUEVO v4.0)
//
//  Variante de bsp_pwm_rgb_set_ch() con brillo en porcentaje.
//  Aplica la misma corrección de lógica activa baja.
// ===========================================================================
void bsp_pwm_rgb_set_ch_percent(hal_pwm_ch_t ch, uint8_t brillo)
{
    hal_pwm_set_duty_percent(ch, invertir_pct(brillo));
}

// ===========================================================================
//  bsp_pwm_rgb_pause / bsp_pwm_rgb_resume
// ===========================================================================
void bsp_pwm_rgb_pause(void)  { hal_pwm_pause();  }
void bsp_pwm_rgb_resume(void) { hal_pwm_resume(); }