// FileName:        hal_gpio.h
// Dependencies:    gpio_2026.h
// Processor:       Tensilica Xtensa LX6 160 MHz
// Board:           ESP-Panther48 (ESP32 ITCH REV 1.0)
// Program version: 3.0
// Company:         Instituto Tecnologico de Chihuahua
// Description:     HAL de GPIO portable y escalable.
//                  Abstrae operaciones de periférico (salida digital,
//                  entrada digital, interrupción), sin conocer conceptos
//                  de aplicación.
//
//                  Regla de capas:
//                    DRIVER  (gpio_2026)   acceso directo a registros
//                    HAL     (hal_gpio)    operaciones de periférico
//                    BSP     (bsp_x)       mapeo a componentes de placa
//                    APP     (main)        lógica de aplicación
//
//                  Para agregar soporte a otro MCU basta con reimplementar
//                  hal_gpio.c apuntando al driver de ese MCU; hal_gpio.h 
//                  y todas las capas superiores no cambian.
//
// Autores:         Ana Paola Cardona Valenzuela
//                  Emiliano Perez Dyck
//                  Luis Adrian Anchondo Carreón
// Created:         04/06/2026
// Updated:         04/06/2026

#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include <stdint.h>
#include <stdbool.h>
#include "gpio_2026.h"   // tipos: pull_mode_t, gpio_logic_t,
                         //        gpio_int_type_t, gpio_isr_callback_t

// ===========================================================================
//  Número máximo de pines soportados
//  Cambiar este valor para MCUs con más o menos GPIOs
// ===========================================================================
#define HAL_GPIO_MAX_PINS  39 // No me acuerdo cuantos son del ESP32

// ===========================================================================
//  Códigos de retorno imitando la filosofia de diseño de los Chinos.
//  Todas las funciones devuelven hal_gpio_err_t para que el llamador
//  pueda decidir si ignorar o manejar el error sin depender de errno.
// ===========================================================================
typedef enum {
    HAL_GPIO_OK        =  0,   // Operación exitosa
    HAL_GPIO_ERR_PIN   = -1,   // Número de pin inválido o inexistente
    HAL_GPIO_ERR_DIR   = -2,   // Dirección incorrecta (ej. escribir en entrada)
    HAL_GPIO_ERR_INIT  = -3,   // Pin no inicializado antes de usar
    HAL_GPIO_ERR_IRQ   = -4,   // Error en configuración de interrupción
} hal_gpio_err_t;

// ===========================================================================
//  Tabla interna de descriptores
//
//  Almacena el puntero a gpio_pin_t devuelto por el driver para cada pin.
//  El índice es el número de GPIO (0-39).
//  NULL indica pin no inicializado.
//
//  Se declara extern aquí; se define UNA SOLA VEZ en hal_gpio.c.
//  Las capas superiores (BSP, APP) no deben acceder a esta tabla
//  directamente; usar las funciones de la API.
// ===========================================================================
extern gpio_pin_t *hal_gpio_table[HAL_GPIO_MAX_PINS];

// ===========================================================================
//  API  Salidas digitales
//
//  Un output es cualquier actuador digital: LED, relay, buzzer,
//  transistor, etc. El HAL no sabe ni le importa qué hay
//  conectado; solo controla el pin.
// ===========================================================================

/**
 * @brief  Inicializa un pin como salida digital.
 *
 * @param  pin    Número de GPIO (0 ... HAL_GPIO_MAX_PINS-1)
 * @param  logic  GPIO_ACTIVE_HIGH o GPIO_ACTIVE_LOW
 *                Determina si "encendido" = nivel alto o bajo en el hardware.
 *
 * @return HAL_GPIO_OK         si el pin quedó configurado
 *         HAL_GPIO_ERR_PIN    si el pin no existe o está fuera de rango
 *
 * Ejemplo:
 *   hal_gpio_output_init(2,  GPIO_ACTIVE_LOW);   // LED con cátodo al GPIO
 *   hal_gpio_output_init(25, GPIO_ACTIVE_HIGH);  // Relay con bobina al GPIO
 */

hal_gpio_err_t hal_gpio_output_init(uint8_t pin, gpio_logic_t logic);

/**
 * @brief  Establece el estado lógico de una salida.
 *
 * @param  pin    Número de GPIO previamente inicializado con output_init
 * @param  state  true  = activo (ON  según la lógica configurada)
 *                false = inactivo (OFF según la lógica configurada)
 *
 * @return HAL_GPIO_OK        éxito
 *         HAL_GPIO_ERR_INIT  pin no inicializado
 *         HAL_GPIO_ERR_DIR   pin configurado como entrada
 */

hal_gpio_err_t hal_gpio_output_set(uint8_t pin, bool state);

/**
 * @brief  Invierte el estado lógico actual de una salida.
 *
 * @param  pin  Número de GPIO previamente inicializado con output_init
 *
 * @return HAL_GPIO_OK / HAL_GPIO_ERR_INIT / HAL_GPIO_ERR_DIR
 */

hal_gpio_err_t hal_gpio_output_toggle(uint8_t pin);

/**
 * @brief  Lee el estado lógico actual de una salida (lo que se escribió,
 *         no el nivel físico del pin).
 *
 * Útil para toggle sin variable auxiliar y para verificar estado sin
 * depender de la lógica activa.
 *
 * @param  pin    Número de GPIO
 * @param  state  Puntero donde se deposita el resultado
 *
 * @return HAL_GPIO_OK / HAL_GPIO_ERR_INIT / HAL_GPIO_ERR_DIR
 */

hal_gpio_err_t hal_gpio_output_get(uint8_t pin, bool *state);

// ===========================================================================
//  API  Entradas digitales
//
//  Un input es cualquier sensor digital.
// ===========================================================================

/**
 * @brief  Inicializa un pin como entrada digital.
 *
 * @param  pin   Número de GPIO
 * @param  pull  GPIO_FLOATING, GPIO_PULLUP o GPIO_PULLDOWN
 * @param  logic GPIO_ACTIVE_HIGH o GPIO_ACTIVE_LOW
 *               Define cuándo gpio_read devuelve true.
 *               Para botón con pull-up: GPIO_ACTIVE_LOW (presionado = LOW = true)
 *               Para sensor con pull-down: GPIO_ACTIVE_HIGH (activo = HIGH = true)
 *
 * @return HAL_GPIO_OK / HAL_GPIO_ERR_PIN
 *
 * Ejemplo:
 *   hal_gpio_input_init(18, GPIO_PULLUP,   GPIO_ACTIVE_LOW);   // Botón
 *   hal_gpio_input_init(34, GPIO_FLOATING, GPIO_ACTIVE_HIGH);  // Sensor 3.3V
 */

hal_gpio_err_t hal_gpio_input_init(uint8_t pin, pull_mode_t pull, gpio_logic_t logic);

/**
 * @brief  Lee el estado lógico de una entrada.
 *
 * @param  pin    Número de GPIO previamente inicializado con input_init
 * @param  state  Puntero donde se deposita el resultado:
 *                true  = señal activa (según lógica configurada)
 *                false = señal inactiva
 *
 * @return HAL_GPIO_OK / HAL_GPIO_ERR_INIT / HAL_GPIO_ERR_DIR
 *
 * Ejemplo:
 *   bool pressed;
 *   hal_gpio_input_read(18, &pressed);
 *   if (pressed) { ... }
 */

hal_gpio_err_t hal_gpio_input_read(uint8_t pin, bool *state);

// ===========================================================================
//  API  Interrupciones
//
//  Las interrupciones se configuran sobre pines de entrada ya inicializados
//  con hal_gpio_input_init(). El tipo de evento (flanco, nivel) se pasa
//  en hal_gpio_irq_attach(); esto permite reutilizar el pin con distintos
//  eventos sin reinicializarlo desde cero.
// ===========================================================================

/**
 * @brief  Configura y habilita una interrupción en un pin de entrada.
 *
 * Reinicializa el pin con el tipo de interrupción indicado y registra
 * el callback. Llama internamente a gpio_isr_service_init() si aún no
 * fue inicializado el servicio.
 *
 * @param  pin       Número de GPIO (debe estar ya inicializado como entrada)
 * @param  int_type  Tipo de evento: INT_FLANCO_POS, INT_FLANCO_NEG,
 *                   INT_CUALQUIER_FLANCO, INT_NIVEL_ALTO, INT_NIVEL_BAJO
 * @param  cb        Función callback  DEBE declararse con IRAM_ATTR.
 *                   Restricciones ISR: sin printf, malloc ni bloqueantes.
 * @param  arg       Argumento que se pasa a cb en cada invocación (puede ser NULL)
 *
 * @return HAL_GPIO_OK / HAL_GPIO_ERR_INIT / HAL_GPIO_ERR_DIR / HAL_GPIO_ERR_IRQ
 *
 * Ejemplo:
 *   void IRAM_ATTR mi_isr(void *arg) { ... }
 *
 *   hal_gpio_input_init(18, GPIO_PULLUP, GPIO_ACTIVE_LOW);
 *   hal_gpio_irq_attach(18, INT_FLANCO_NEG, mi_isr, NULL);
 */
hal_gpio_err_t hal_gpio_irq_attach(uint8_t           pin,
                                   gpio_int_type_t   int_type,
                                   gpio_isr_callback_t cb,
                                   void             *arg);

/**
 * @brief  Deshabilita la interrupción de un pin y elimina su callback.
 *
 * El pin sigue configurado como entrada; puede volver a habilitarse con
 * hal_gpio_irq_attach() sin necesidad de reinicializar.
 *
 * @param  pin  Número de GPIO
 *
 * @return HAL_GPIO_OK / HAL_GPIO_ERR_INIT
 */

hal_gpio_err_t hal_gpio_irq_detach(uint8_t pin);

// ===========================================================================
//  API  Utilidades
// ===========================================================================

/**
 * @brief  Devuelve true si el pin fue inicializado (como entrada o salida).
 *
 * Útil para verificar en el BSP antes de operar sin necesidad de manejar
 * el código de error completo.
 *
 * @param  pin  Número de GPIO
 * @return true si inicializado, false si no
 */

bool hal_gpio_is_init(uint8_t pin);

/**
 * @brief  Desinicializa un pin y libera su entrada en la tabla interna.
 *
 * Tras esta llamada, hal_gpio_is_init(pin) devuelve false y cualquier
 * operación sobre el pin retorna HAL_GPIO_ERR_INIT.
 * No modifica el hardware directamente; el pin queda en el último estado
 * físico hasta que otro módulo lo reconfigure.
 *
 * @param  pin  Número de GPIO
 *
 * @return HAL_GPIO_OK / HAL_GPIO_ERR_PIN
 */

hal_gpio_err_t hal_gpio_deinit(uint8_t pin);

#endif /* HAL_GPIO_H */