void func(void);// FileName:        bsp_panther48.h
// Dependencies:    board_pins.h, hal_gpio.h, hal_timer.h
// Processor:       Tensilica Xtensa LX6 160 MHz
// Board:           ESP32 ITCH (Panther48)  REV 1.0 
// Program version: 3.0
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
//                  2) API GENÉRICA: el usuario puede
//                     operar cualquier pin de la tarjeta con:
//                       bsp_pin_mode(BSP_IO5, BSP_OUTPUT);
//                       bsp_digital_write(BSP_IO5, BSP_HIGH);
//                       bsp_digital_read(BSP_IO18);
//                     Los alias BSP_IOx mapean directamente a los GPIO
//                     físicos; la lógica activa es siempre HIGH en este modo.
//
//                  La APP solo incluye este archivo.
//                  No debe incluir hal_gpio.h, gpio_2026.h ni board_pins.h.
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
//  Estos alias son los mismos que los de board_pins.h pero con nombre
//  uniforme para la API genérica.
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

// Alias semánticos para los pines de la placa (equivalentes a los anteriores)
// Debido a que la placa fisicamente tiene puentes removibles, el usuario puede usar cualquiera de los dos estilos de alias
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
    BSP_INPUT           = 0,   // Entrada sin resistencia
    BSP_INPUT_PULLUP    = 1,   // Entrada con pull-up
    BSP_INPUT_PULLDOWN  = 2,   // Entrada con pull-down
    BSP_OUTPUT          = 3,   // Salida activa-alta (HIGH enciende)
    BSP_OUTPUT_INVERTED = 4,   // Salida activa-baja (LOW enciende)
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
 *
 *         Configura internamente:
 *           - Timer del sistema (HAL_TIMER_1)
 *           - LEDs 1-5 como salidas activa-baja, apagados
 *           - LED RGB (R,G,B) como salidas activa-baja, apagados
 *           - SW1, SW2, BOOT como entradas con pull-up
 */
void bsp_init(void);

// ===========================================================================
//  API GENÉRICA  (estilo Arduino)
//  Opera sobre cualquier pin de la tarjeta usando BSP_IOx o BSP_LEDx, etc.
//  La lógica activa en este modo es siempre ACTIVE_HIGH, excepto en
//  BSP_OUTPUT_INVERTED que es ACTIVE_LOW.
// ===========================================================================

/**
 * @brief  Configura la dirección y modo de un pin.
 *
 * @param  pin   Alias del pin: BSP_IO2, BSP_LED1, BSP_SW1, etc.
 * @param  mode  BSP_INPUT | BSP_INPUT_PULLUP | BSP_INPUT_PULLDOWN |
 *               BSP_OUTPUT | BSP_OUTPUT_INVERTED
 *
 * Ejemplo (igual que Arduino):
 *   bsp_pin_mode(BSP_IO5,  BSP_OUTPUT);
 *   bsp_pin_mode(BSP_IO18, BSP_INPUT_PULLUP);
 */
void bsp_pin_mode(uint8_t pin, bsp_pin_mode_t mode);

/**
 * @brief  Escribe un nivel lógico en un pin de salida.
 *
 * @param  pin    Alias del pin (debe haber sido configurado con BSP_OUTPUT)
 * @param  level  BSP_HIGH o BSP_LOW
 *
 * Ejemplo:
 *   bsp_digital_write(BSP_IO5, BSP_HIGH);   // enciende LED en IO5
 *   bsp_digital_write(BSP_LED3, BSP_LOW);   // apaga LED3 (IO16)
 */
void bsp_digital_write(uint8_t pin, bsp_pin_level_t level);

/**
 * @brief  Lee el nivel lógico de un pin de entrada.
 *
 * @param  pin  Alias del pin (debe haber sido configurado con BSP_INPUT*)
 * @return BSP_HIGH si el pin está en nivel alto, BSP_LOW si está en nivel bajo
 *
 * Ejemplo:
 *   if (bsp_digital_read(BSP_SW1) == BSP_LOW) { ... }  // botón presionado
 */
bsp_pin_level_t bsp_digital_read(uint8_t pin);

/**
 * @brief  Registra una ISR en un pin de entrada ya configurado.
 *         El tipo de flanco (NEG para botones con pull-up) se pasa explícito
 *         para que la API genérica sea flexible.
 *
 * @param  pin       Alias del pin
 * @param  int_type  INT_FLANCO_POS | INT_FLANCO_NEG | INT_CUALQUIER_FLANCO |
 *                   INT_NIVEL_ALTO | INT_NIVEL_BAJO
 * @param  cb        Callback  DEBE declararse con IRAM_ATTR
 * @param  arg       Argumento para el callback (puede ser NULL)
 *
 * Ejemplo:
 *   void IRAM_ATTR mi_isr(void *arg) { ... }
 *   bsp_attach_interrupt(BSP_IO18, INT_FLANCO_NEG, mi_isr, NULL);
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
//  La APP no conoce pines ni lógica activa; solo habla de LEDs 1-5.
// ===========================================================================
void bsp_led_on(uint8_t led);          // led: 1…5  o BSP_LED1…BSP_LED5
void bsp_led_off(uint8_t led);
void bsp_led_toggle(uint8_t led);
void bsp_led_blink(uint8_t led, uint32_t ms);  // ON ms -> OFF ms, bloqueante
void bsp_led_all_on(void);
void bsp_led_all_off(void);

// ===========================================================================
//  API SEMÁNTICA  LED RGB
// ===========================================================================

/**
 * @brief  Establece color del RGB. Usar macros BSP_COLOR_*:
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
//  El BSP encapsula el tipo de flanco (NEG con pull-up).
//  La APP solo pasa el callback.
// ===========================================================================
void bsp_sw1_irq_attach(gpio_isr_callback_t cb, void *arg);
void bsp_sw1_irq_detach(void);
void bsp_sw2_irq_attach(gpio_isr_callback_t cb, void *arg);
void bsp_sw2_irq_detach(void);

// ===========================================================================
//  Retardos y tiempo (HAL_TIMER_1 reservado para el BSP)
// ===========================================================================
void     bsp_delay_ms(uint32_t ms);
void     bsp_delay_us(uint32_t us);
uint64_t bsp_get_ms(void);
uint64_t bsp_get_us(void);

// ===========================================================================
//  Mapeo canal HAL ↔ color ↔ GPIO
//  Cambia aquí si la placa cambia de revisión; la APP no se modifica.
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
//  API
// ===========================================================================
 
/**
 * @brief  Inicializa el PWM para el LED RGB.
 *         Llama a hal_pwm_init() internamente (idempotente).
 *         Configura los tres canales R, G, B apagados.
 *         Debe llamarse una sola vez, típicamente en bsp_init() o al inicio
 *         de app_main() antes de cualquier otra función de este módulo.
 *
 * @return void
 */
void bsp_pwm_rgb_init(void);
 
/**
 * @brief  Establece el brillo individual de cada canal (0–100 %).
 *         El valor se convierte internamente al duty correcto considerando
 *         la lógica activa baja del LED RGB de la tarjeta.
 *
 * @param  r  Brillo canal Rojo  (0 = apagado, 100 = máximo)
 * @param  g  Brillo canal Verde
 * @param  b  Brillo canal Azul
 *
 * Ejemplo:
 *   bsp_pwm_rgb_set(100, 0, 0);    // rojo puro
 *   bsp_pwm_rgb_set(0,   0, 50);   // azul al 50 %
 *   bsp_pwm_rgb_set(0,   0, 0);    // apagado
 */
void bsp_pwm_rgb_set(uint8_t r, uint8_t g, uint8_t b);
 
/**
 * @brief  Apaga el LED RGB (equivalente a bsp_pwm_rgb_set(0, 0, 0)).
 */
void bsp_pwm_rgb_off(void);
 
/**
 * @brief  Establece el brillo de un solo canal por su identificador HAL.
 *         Útil cuando la APP quiere animar un canal sin tocar los otros.
 *
 * @param  ch      BSP_PWM_CH_R, BSP_PWM_CH_G o BSP_PWM_CH_B
 * @param  brillo  0–100 %
 */
void bsp_pwm_rgb_set_ch(hal_pwm_ch_t ch, uint8_t brillo);
 
/**
 * @brief  Pausa el timer PWM (todos los canales se congelan).
 */
void bsp_pwm_rgb_pause(void);
 
/**
 * @brief  Reanuda el timer PWM previamente pausado.
 */
void bsp_pwm_rgb_resume(void);

#endif /* BSP_PANTHER48_H */