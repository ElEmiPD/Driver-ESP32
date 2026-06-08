// FileName:        bsp_panther48.h
// Dependencies:    board_pins.h, hal_gpio.h, hal_timer.h, hal_pwm.h
// Processor:       Tensilica Xtensa LX6 160 MHz
// Board:           ESP32 ITCH (Panther48)  REV 1.0
// Program version: 4.0
// Company:         Instituto Tecnologico de Chihuahua
// Description:     Board Support Package completo para ESP32 ITCH.
//
//                  El BSP expone DOS estilos de API:
//
//                  1) API SEMÁNTICA: nombres de componente de la placa.
//                     La APP no conoce números de pin ni lógica activa.
//                     Ideal para código legible y de alto nivel:
//                       bsp_led_on(BSP_LED1);
//                       bsp_sw1_pressed();
//
//                  2) API GENÉRICA: el usuario puede operar cualquier pin
//                     con:
//                       bsp_pin_mode(BSP_IO5, BSP_OUTPUT);
//                       bsp_digital_write(BSP_IO5, BSP_HIGH);
//                       bsp_digital_read(BSP_IO18);
//                     Los alias BSP_IOx mapean directamente a los GPIO
//                     físicos; la lógica activa es siempre HIGH en este modo.
//
//                  La APP solo incluye este archivo.
//                  No debe incluir hal_pwm.h, hal_gpio.h, gpio_2026.h
//                  ni board_pins.h directamente.
//
//                  Cambios v4.0 respecto a v3.0:
//                    + bsp_pwm_rgb_set_percent(): variante de bsp_pwm_rgb_set()
//                      que acepta brillo en porcentaje (0–100) para cada canal.
//                      Delega a hal_pwm_set_duty_percent() internamente, con
//                      corrección de lógica activa baja (invierte el porcentaje).
//                    + bsp_pwm_rgb_set_ch_percent(): variante de
//                      bsp_pwm_rgb_set_ch() con argumento en porcentaje.
//
// Autores:         Ana Paola Cardona Valenzuela
//                  Emiliano Perez Dyck
//                  Luis Adrian Anchondo Carreón
// Created:         03/06/2026
// Updated:         04/06/2026

#ifndef BSP_PANTHER48_H
#define BSP_PANTHER48_H

#include <stdint.h>
#include <stdbool.h>
#include "board_pins.h"
#include "hal_gpio.h"
#include "hal_timer.h"
#include "hal_pwm.h"

// ===========================================================================
//  ALIAS DE PINES estilo Arduino
//  La APP usa BSP_IO2, BSP_IO5, etc.  nunca el número crudo.
// ===========================================================================
#define BSP_IO0     0
#define BSP_IO1     1
#define BSP_IO2     2
#define BSP_IO3     3
#define BSP_IO4     4
#define BSP_IO5     5
#define BSP_IO12   12
#define BSP_IO13   13
#define BSP_IO14   14
#define BSP_IO16   16
#define BSP_IO17   17
#define BSP_IO18   18
#define BSP_IO19   19
#define BSP_IO21   21
#define BSP_IO22   22
#define BSP_IO25   25
#define BSP_IO26   26
#define BSP_IO27   27
#define BSP_IO32   32
#define BSP_IO33   33
#define BSP_IO34   34
#define BSP_IO35   35
#define BSP_IO36   36
#define BSP_IO39   39

// Alias semánticos para los componentes de la placa
#define BSP_LED1    BOARD_LED1_PIN   //  2
#define BSP_LED2    BOARD_LED2_PIN   //  4
#define BSP_LED3    BOARD_LED3_PIN   // 16
#define BSP_LED4    BOARD_LED4_PIN   // 17
#define BSP_LED5    BOARD_LED5_PIN   //  5
#define BSP_RGB_R   BOARD_RGB_R_PIN  // 14
#define BSP_RGB_G   BOARD_RGB_G_PIN  // 13
#define BSP_RGB_B   BOARD_RGB_B_PIN  // 12
#define BSP_SW1     BOARD_BTN_SW1_PIN   // 18
#define BSP_SW2     BOARD_BTN_SW2_PIN   // 19
#define BSP_BOOT    BOARD_BTN_BOOT_PIN  //  0
#define BSP_SDA     BOARD_OLED_SDA_PIN  // 21
#define BSP_SCL     BOARD_OLED_SCL_PIN  // 22
#define BSP_NTC     BOARD_NTC_PIN       // 36
#define BSP_LDR     BOARD_LDR_PIN       // 39

// ===========================================================================
//  Constantes de modo y nivel  (estilo Arduino)
// ===========================================================================
typedef enum {
    BSP_INPUT           = 0,
    BSP_INPUT_PULLUP    = 1,
    BSP_INPUT_PULLDOWN  = 2,
    BSP_OUTPUT          = 3,
    BSP_OUTPUT_INVERTED = 4,
} bsp_pin_mode_t;

typedef enum {
    BSP_LOW  = 0,
    BSP_HIGH = 1,
} bsp_pin_level_t;

// ===========================================================================
//  COLORES RGB predefinidos
// ===========================================================================
#define BSP_COLOR_RED       true,  false, false
#define BSP_COLOR_GREEN     false, true,  false
#define BSP_COLOR_BLUE      false, false, true
#define BSP_COLOR_YELLOW    true,  true,  false
#define BSP_COLOR_CYAN      false, true,  true
#define BSP_COLOR_MAGENTA   true,  false, true
#define BSP_COLOR_WHITE     true,  true,  true
#define BSP_COLOR_OFF       false, false, false

// ===========================================================================
//  INICIALIZACIÓN
// ===========================================================================

/**
 * @brief  Inicializa TODOS los periféricos de la tarjeta ESP32 ITCH.
 *         Debe ser la primera llamada en app_main().
 */
void bsp_init(void);

// ===========================================================================
//  API GENÉRICA  (estilo Arduino)
// ===========================================================================

/**
 * @brief  Configura la dirección y modo de un pin.
 *
 * @param  pin   Alias del pin: BSP_IO2, BSP_LED1, BSP_SW1, etc.
 * @param  mode  BSP_INPUT | BSP_INPUT_PULLUP | BSP_INPUT_PULLDOWN |
 *               BSP_OUTPUT | BSP_OUTPUT_INVERTED
 */
void bsp_pin_mode(uint8_t pin, bsp_pin_mode_t mode);

/**
 * @brief  Escribe un nivel lógico en un pin de salida.
 *
 * @param  pin    Alias del pin (configurado con BSP_OUTPUT)
 * @param  level  BSP_HIGH o BSP_LOW
 */
void bsp_digital_write(uint8_t pin, bsp_pin_level_t level);

/**
 * @brief  Lee el nivel lógico de un pin de entrada.
 *
 * @param  pin  Alias del pin (configurado con BSP_INPUT*)
 * @return BSP_HIGH / BSP_LOW
 */
bsp_pin_level_t bsp_digital_read(uint8_t pin);

/**
 * @brief  Registra una ISR en un pin de entrada ya configurado.
 *
 * @param  pin       Alias del pin
 * @param  int_type  INT_FLANCO_POS | INT_FLANCO_NEG | INT_CUALQUIER_FLANCO |
 *                   INT_NIVEL_ALTO | INT_NIVEL_BAJO
 * @param  cb        Callback (debe declararse con IRAM_ATTR)
 * @param  arg       Argumento para el callback (puede ser NULL)
 */
void bsp_attach_interrupt(uint8_t pin, gpio_int_type_t int_type,
                          gpio_isr_callback_t cb, void *arg);

/**
 * @brief  Deshabilita la ISR de un pin.
 * @param  pin  Alias del pin
 */
void bsp_detach_interrupt(uint8_t pin);

// ===========================================================================
//  API SEMÁNTICA  LEDs individuales
// ===========================================================================
void bsp_led_on(uint8_t led);
void bsp_led_off(uint8_t led);
void bsp_led_toggle(uint8_t led);
void bsp_led_blink(uint8_t led, uint32_t ms);
void bsp_led_all_on(void);
void bsp_led_all_off(void);

// ===========================================================================
//  API SEMÁNTICA  LED RGB — control digital (ON/OFF)
// ===========================================================================

/**
 * @brief  Establece color del RGB con macros BSP_COLOR_*:
 *           bsp_rgb_set(BSP_COLOR_CYAN);
 */
void bsp_rgb_set(bool r, bool g, bool b);
void bsp_rgb_off(void);

// ===========================================================================
//  API SEMÁNTICA  Botones (polling)
// ===========================================================================
bool bsp_sw1_pressed(void);
bool bsp_sw2_pressed(void);
bool bsp_boot_pressed(void);

// ===========================================================================
//  API SEMÁNTICA  Botones (interrupción)
// ===========================================================================
void bsp_sw1_irq_attach(gpio_isr_callback_t cb, void *arg);
void bsp_sw1_irq_detach(void);
void bsp_sw2_irq_attach(gpio_isr_callback_t cb, void *arg);
void bsp_sw2_irq_detach(void);

// ===========================================================================
//  Retardos y tiempo
// ===========================================================================
void     bsp_delay_ms(uint32_t ms);
void     bsp_delay_us(uint32_t us);
uint64_t bsp_get_ms(void);
uint64_t bsp_get_us(void);

// ===========================================================================
//  Mapeo canal HAL ↔ color ↔ GPIO
//  Cambiar aquí si la placa cambia de revisión; la APP no se modifica.
// ===========================================================================
#define BSP_PWM_CH_R   HAL_PWM_CH_1   // Canal HAL para Rojo   -> IO14
#define BSP_PWM_CH_G   HAL_PWM_CH_2   // Canal HAL para Verde  -> IO13
#define BSP_PWM_CH_B   HAL_PWM_CH_3   // Canal HAL para Azul   -> IO12

// ===========================================================================
//  Rango de brillo visible para la APP (0–100 %)
//  Internamente se convierte a duty 0–255 con corrección de lógica activa.
// ===========================================================================
#define BSP_PWM_BRILLO_MIN   0u
#define BSP_PWM_BRILLO_MAX   100u

// ===========================================================================
//  API SEMÁNTICA  LED RGB — control PWM con duty en valor absoluto
// ===========================================================================

/**
 * @brief  Inicializa el PWM para el LED RGB.
 *         Llama a hal_pwm_init() internamente (idempotente).
 *         Configura los tres canales R, G, B apagados.
 *         Debe llamarse una sola vez, típicamente en bsp_init().
 */
void bsp_pwm_rgb_init(void);

/**
 * @brief  Establece el brillo de cada canal en valor absoluto (0–HAL_PWM_MAX_DUTY).
 *         La APP usa este nivel para manipulación precisa canal a canal.
 *         Corrección de lógica activa baja incluida internamente.
 *
 * @param  r  Duty canal Rojo  (0 = apagado, HAL_PWM_MAX_DUTY = máximo brillo)
 * @param  g  Duty canal Verde
 * @param  b  Duty canal Azul
 *
 * Ejemplo:
 *   bsp_pwm_rgb_set(255, 0, 0);    // rojo puro al máximo
 *   bsp_pwm_rgb_set(0,   0, 128);  // azul al ~50 %
 */
void bsp_pwm_rgb_set(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief  Apaga el LED RGB (equivalente a bsp_pwm_rgb_set(0, 0, 0)).
 */
void bsp_pwm_rgb_off(void);

/**
 * @brief  Establece el brillo de un solo canal en valor absoluto.
 *         Útil para animar un canal sin tocar los otros.
 *
 * @param  ch      BSP_PWM_CH_R, BSP_PWM_CH_G o BSP_PWM_CH_B
 * @param  brillo  Valor absoluto (0 … HAL_PWM_MAX_DUTY)
 */
void bsp_pwm_rgb_set_ch(hal_pwm_ch_t ch, uint8_t brillo);

// ===========================================================================
//  API SEMÁNTICA  LED RGB — control PWM con duty en porcentaje (NUEVO v4.0)
// ===========================================================================

/**
 * @brief  Establece el brillo de cada canal en porcentaje (0–100 %).
 *         Más legible que bsp_pwm_rgb_set() para la APP, y no requiere
 *         conocer HAL_PWM_MAX_DUTY ni la resolución del timer.
 *         Corrección de lógica activa baja incluida internamente.
 *
 * @param  r  Brillo canal Rojo  (0 = apagado, 100 = máximo)
 * @param  g  Brillo canal Verde
 * @param  b  Brillo canal Azul
 *
 * Ejemplo:
 *   bsp_pwm_rgb_set_percent(100, 0,  0);   // rojo puro
 *   bsp_pwm_rgb_set_percent(0,   0, 50);   // azul al 50 %
 *   bsp_pwm_rgb_set_percent(0,   0,  0);   // apagado
 */
void bsp_pwm_rgb_set_percent(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief  Establece el brillo de un solo canal en porcentaje (0–100 %).
 *         Variante de bsp_pwm_rgb_set_ch() para la APP que prefiere %.
 *
 * @param  ch      BSP_PWM_CH_R, BSP_PWM_CH_G o BSP_PWM_CH_B
 * @param  brillo  Brillo en porcentaje (0–100)
 *
 * Ejemplo:
 *   bsp_pwm_rgb_set_ch_percent(BSP_PWM_CH_B, 75);  // azul al 75 %
 */
void bsp_pwm_rgb_set_ch_percent(hal_pwm_ch_t ch, uint8_t brillo);

// ===========================================================================
//  API SEMÁNTICA  LED RGB — control de timer
// ===========================================================================

/**
 * @brief  Pausa el timer PWM (todos los canales se congelan).
 */
void bsp_pwm_rgb_pause(void);

/**
 * @brief  Reanuda el timer PWM previamente pausado.
 */
void bsp_pwm_rgb_resume(void);

#endif /* BSP_PANTHER48_H */