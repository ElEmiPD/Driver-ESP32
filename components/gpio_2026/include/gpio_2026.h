// FileName:        gpio_2026.h                                                                                            
// Dependencies:    None                                                                                              
// Processor:       Tensilica Xtensa LX6 160 MHz                                                                                             
// Board:           ESP-WROOM-32                                                                                  
// Program version: 2.0                                                                                  
// Company:         Instituto Tecnologico de Chihuahua                                                                                 
// Description:     Declaración de funciones de configuración
//                  control GPIO para ESP32. Construcción de estructura 
//                  gpio_pin_t y configuración hardware.                               
// Autor:           Ana Paola Cardona Valenzuela
//                  Luis Adrian Anchondo Carreón
//                  Emiliano Perez Dyck 
// Updated:         03/06/2026

#ifndef GPIO_2026_H
#define GPIO_2026_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "esp_attr.h"        // IRAM_ATTR
#include "esp_intr_alloc.h"  // esp_intr_alloc, intr_handle_t
#include "soc/interrupts.h"  // ETS_GPIO_INTR_SOURCE

// Macro registers access
#ifndef HWREG32
#define HWREG32(x) (*((volatile unsigned int *)(x)))
#endif

// Prototipos de macrofunciones
// Macro para gpio_2026

//Entradas
#define GPIO_INPUT(pin) \
    gpio_init2026(pin, false, GPIO_ACTIVE_HIGH, GPIO_DEFAULT, INT_DESHABILITADA)

#define GPIO_INPUT_PU(pin) \
    gpio_init2026(pin, false, GPIO_ACTIVE_LOW, GPIO_PULLUP, INT_DESHABILITADA)

#define GPIO_INPUT_PD(pin) \
    gpio_init2026(pin, false, GPIO_ACTIVE_HIGH, GPIO_PULLDOWN, INT_DESHABILITADA)

//Salidas
#define GPIO_OUTPUT_P(pin) \
    gpio_init2026(pin, true, GPIO_ACTIVE_HIGH, GPIO_PULLUP, INT_DESHABILITADA)
#define GPIO_OUTPUT_N(pin) \
    gpio_init2026(pin, true, GPIO_ACTIVE_LOW, GPIO_PULLDOWN, INT_DESHABILITADA)

// Lectura y escritura de pines GPIO
#define READ_PIN(gpio) \
    gpio_read((gpio))

#define WRITE_PIN(gpio, value) \
    gpio_write((gpio), (value))

// Interrupciones - configuración de pin
#define GPIO_INPUT_IT(pin, pull, intr) \
    gpio_init2026(pin, false, GPIO_ACTIVE_HIGH, pull, intr)

// Interrupciones - registro y control
#define IT_ATTACH(gpio, cb, arg) \
    gpio_enable_interrupt((gpio), (cb), (arg))

#define IT_DETACH(gpio) \
    gpio_disable_interrupt((gpio))

// Inicialización del servicio (se llama una vez en app_main)
#define IT_SERVICE_INIT() \
    gpio_isr_service_init()


// GPIO REGISTERS

// GPIO ENABLE
#define GPIO_ENABLE        HWREG32(0x3FF44020)  //Enable control for GPIO output

// GPIO OUTPUT CONTROL
#define GPIO_OUT_W1TS      HWREG32(0x3FF44008)  //Set
#define GPIO_OUT_W1TC      HWREG32(0x3FF4400C)  //Clear

// GPIO INPUT CONTROL
#define GPIO_IN            HWREG32(0x3FF4403C)  //Input status
#define GPIO_PIN_REG_BASE  0x3FF44088 // Base para GPIO_PINn_REG (interrupciones)

// Mascaras para configuración de pines de entrada
// Mascara al registro GPIO_MUX_REGS[pin]

#define FUN_IE              (1 << 9)    // Habilitar función de entrada
#define PULL_WPU            (1 << 8)    // Habilitar resistencia de pull-up
#define PULL_WPD            (1 << 7)    // Habilitar resistencia de pull-down
#define MUX_MCU_SEL_GPIO    (0x2 << 12) // Seleccionar función GPIO (MCU_SEL = 2)
#define MUX_MCU_SEL_MASK    (0x7 << 12) // Máscara para limpiar MCU_SEL

// Interrupciones GPIO
// Registro de habilitación de interrupciones GPIO
#define GPIO_INT_ENA_REG (*(volatile uint32_t *)0x3FF44148)  // APP_CPU (CPU1)
// Si usas PRO_CPU (CPU0): 0x3FF44144

// Direcciones base
#define GPIO_STATUS_ADDR      0x3FF4404C
#define GPIO_STATUS_W1TC_ADDR 0x3FF44054
#define GPIO_STATUS1_ADDR     0x3FF44050
#define GPIO_STATUS1_W1TC_ADDR 0x3FF44058

// Macros de acceso
#define GPIO_STATUS_REG       (*(volatile uint32_t *)GPIO_STATUS_ADDR)
#define GPIO_STATUS_W1TC_REG  (*(volatile uint32_t *)GPIO_STATUS_W1TC_ADDR)
#define GPIO_STATUS1_REG      (*(volatile uint32_t *)GPIO_STATUS1_ADDR)
#define GPIO_STATUS1_W1TC_REG (*(volatile uint32_t *)GPIO_STATUS1_W1TC_ADDR)

// Tabla de direcciones de los registros de configuración de pines
extern const uint32_t GPIO_MUX_REGS[];

/**
 * @brief Handle de la ISR compartida del periférico GPIO.
 *
 * Almacena el identificador retornado por esp_intr_alloc() al registrar
 * gpio_dispatcher_isr como manejador de la fuente ETS_GPIO_INTR_SOURCE.
 *
 * Es utilizado internamente por gpio_isr_service_init() para evitar
 * registros duplicados. No debe ser manipulado directamente fuera del
 * módulo gpio_2026.c.
 */
static intr_handle_t gpio_isr_handle = NULL;

// Enumeracion para pines GPIO
/*
enum {
    P0,  P1,  P2,  P3,  P4,  P5,  P6,  P7,  P8,  P9,
    P10, P11, P12, P13, P14, P15, P16, P17, P18, P19,
    P20, P21, P22, P23, P24, P25, P26, P27, P28, P29,
    P30, P31, P32, P33, P34, P35, P36, P37, P38, P39
};
*/

// Modos de pull-up/pull-down
typedef enum {
    GPIO_DEFAULT = -1,
    GPIO_FLOATING,
    GPIO_PULLUP,
    GPIO_PULLDOWN
} pull_mode_t;

// Logica activa del pin GPIO
typedef enum {
    GPIO_ACTIVE_LOW,
    GPIO_ACTIVE_HIGH
} gpio_logic_t;

// Tipo de interrupción del pin
typedef enum {
    INT_DESHABILITADA = 0, // Sin interrupción
    INT_FLANCO_POS = 1, // Flanco de subida (LOW -> HIGH)
    INT_FLANCO_NEG = 2, // Flanco de bajada (HIGH -> LOW)
    INT_CUALQUIER_FLANCO = 3, // Cualquier cambio de estado
    INT_NIVEL_BAJO = 4, // Nivel bajo
    INT_NIVEL_ALTO = 5, // Nivel alto
} gpio_int_type_t;

/**
 * @brief Tipo de dato para funciones callback asociadas a interrupciones GPIO.
 *
 * Define un puntero a función que puede ser registrado por el usuario y
 * ejecutado automáticamente cuando ocurra una interrupción en un pin GPIO.
 *
 * La función callback no devuelve ningún valor y recibe un puntero genérico
 * (void *arg), permitiendo pasar información adicional o contexto al momento
 * de su ejecución.
 */
typedef void (*gpio_isr_callback_t)(void *arg);

/**
 * @brief Estructura de configuración y control de un pin GPIO.
 *
 * Almacena los parámetros de configuración del GPIO, incluyendo dirección,
 * lógica activa, resistencias de polarización e interrupciones. También
 * contiene los elementos necesarios para el manejo de callbacks asociados
 * a interrupciones externas.
 *
 * callback:
 *      Puntero a la función que será ejecutada cuando se genere una
 *      interrupción en el GPIO.
 *
 * cb_arg:
 *      Puntero genérico a datos definidos por el usuario. Este valor será
 *      pasado como argumento al callback cuando la interrupción ocurra,
 *      permitiendo compartir información o contexto sin necesidad de
 *      utilizar variables globales.
 */
typedef struct {
    uint8_t             pin;        // Número de GPIO (0-39)
    bool                is_output;  // Dirección del pin
    gpio_logic_t        logic;      // Lógica activa del GPIO
    pull_mode_t         pull_mode;  // Configuración de pull-up/pull-down
    gpio_int_type_t     int_type;   // Tipo de interrupción configurada

    // Manejo de interrupciones mediante callbacks
    gpio_isr_callback_t callback;   // Función registrada por el usuario
    void               *cb_arg;     // Argumento opcional para el callback

} gpio_pin_t;

// Tabla de pines GPIO configurados
extern gpio_pin_t gpio_table[];

// -----------------------  Prototipos de funciones --------------------------

// ------------------------------ GPIO Normal --------------------------------
/**
* @brief    Inicializa el pin GPIO con la configuración especificada. 
*           Configura el pin como entrada o salida,
*           establece la lógica activa (HIGH o LOW) 
*           y el modo de pull-up/pull-down. 
*           Devuelve una estructura gpio_pin_t

* @param  uint8_t pin: Número del pin GPIO a configurar (0-39)
* @param  bool is_output: true para configurar como salida, false para entrada
* @param  gpio_logic_t logic: GPIO_ACTIVE_LOW o GPIO_ACTIVE_HIGH 
          para la lógica activa del pin
* @param  pull_mode_t pull_mode: GPIO_FLOATING, GPIO_PULLUP o GPIO_PULLDOWN 
          para la resistencia de pu o pd
* @param  gpio_int_type_t int_type: Tipo de interrupción para el pin

* @return gpio_pin_t *gpio: Puntero a la estructura gpio_pin_t que contiene la configuración del pin GPIO
*/
gpio_pin_t *gpio_init2026(
    uint8_t pin,
    bool is_output,
    gpio_logic_t logic,
    pull_mode_t pull_mode,
    gpio_int_type_t int_type
);


/** 
* @brief Configura un pin GPIO como entrada.
*        Deshabilita la función de salida, habilita la función de entrada
*        y configura la resistencia de pull-up o pull-down según el modo especificado.
*
* @param gpio_pin_t *gpio: Puntero a la estructura gpio_pin_t 
*        que contiene la configuración del pin GPIO a configurar como entrada
*
* @return void
*/
void gpio_config_in(gpio_pin_t *gpio);


/** 
* @brief Configura un pin GPIO como salida.
*        Habilita la función de salida y configura la lógica activa.
*
* @param gpio_pin_t *gpio: Puntero a la estructura gpio_pin_t 
*        que contiene la configuración del pin GPIO a configurar como salida
*
* @return void
*/
void gpio_config_out(gpio_pin_t *gpio);

/** 
* @brief Lee el estado de un pin GPIO.
* @param gpio_pin_t *gpio: Puntero a la estructura gpio_pin_t 
*        que contiene la configuración del pin GPIO a leer
* @return bool: true si el pin está en nivel alto, false si está en nivel bajo
*/
bool gpio_read(gpio_pin_t *gpio);

/** 
* @brief Escribe un valor en un pin GPIO.
* @param gpio_pin_t *gpio: Puntero a la estructura gpio_pin_t 
*        que contiene la configuración del pin GPIO en el que escribir
* @param bool value: true para establecer el pin en nivel alto, false para nivel bajo
* @return void
*/
void gpio_write(gpio_pin_t *gpio, bool value);

// ------------------------------ Interrupciones --------------------------------

/**
 * @brief Inicializa el servicio global de interrupciones GPIO.
 *
 * Registra la ISR compartida del periférico GPIO en el controlador de
 * interrupciones del ESP32. Debe ser invocada una única vez antes de
 * habilitar interrupciones en cualquier pin con gpio_enable_interrupt().
 *
 * Internamente, asigna gpio_dispatcher_isr como manejador de la fuente
 * ETS_GPIO_INTR_SOURCE. Esta ISR es compartida por todos los pines GPIO
 * que tengan interrupciones habilitadas; el dispatcher determina en tiempo
 * de ejecución cuál pin generó el evento y ejecuta el callback registrado.
 *
 * Llamadas subsecuentes no tienen efecto si el servicio ya fue inicializado.
 *
 * @note    Debe llamarse antes de cualquier gpio_enable_interrupt().
 * @note    Compatible con gpio_init2026() para pines configurados con
 *          int_type distinto de GPIO_INTR_DISABLE.
 *
 * Ejemplo:
 *      void app_main(void)
 *      {
 *          gpio_isr_service_init();
 *          // A partir de aquí es válido registrar callbacks
 *      }
 */
void gpio_isr_service_init(void);

/**
 * @brief Habilita la interrupción de un pin GPIO y registra su callback.
 *
 * Asocia una función callback al pin indicado y activa su máscara de
 * interrupción en el registro GPIO_INT_ENA, permitiendo que el dispatcher
 * la invoque cuando ocurra el evento configurado en gpio_init2026().
 *
 * El tipo de evento que dispara la interrupción (flanco de subida, bajada,
 * cualquier flanco, nivel alto o nivel bajo) queda determinado por el
 * parámetro int_type proporcionado previamente a gpio_init2026(). Esta
 * función únicamente activa la máscara y almacena el callback; no modifica
 * la configuración del evento.
 *
 * @param[in]   gpio    Puntero a la estructura gpio_pin_t del pin objetivo,
 *                      previamente inicializada con gpio_init2026().
 * @param[in]   cb      Puntero a la función callback que será ejecutada
 *                      cuando ocurra la interrupción. Debe estar declarada
 *                      con el atributo IRAM_ATTR. Dentro de esta función
 *                      están prohibidos: printf(), malloc(), vTaskDelay()
 *                      y cualquier llamada que pueda bloquear. Para
 *                      comunicación con tareas usar xTaskNotifyFromISR()
 *                      o similar.
 * @param[in]   arg     Puntero genérico almacenado en gpio->cb_arg. Se
 *                      pasa directamente a cb en cada invocación. Puede
 *                      ser NULL si el callback no requiere contexto.
 *
 * @note    gpio_isr_service_init() debe haberse llamado previamente.
 * @note    El pin debe estar configurado como entrada con un int_type
 *          distinto de GPIO_INTR_DISABLE.
 *
 * Ejemplo:
 *      void IRAM_ATTR boton_isr(void *arg)
 *      {
 *          gpio_pin_t *pin = (gpio_pin_t *)arg;
 *          // Notificar tarea, togglear flag, etc.
 *      }
 *
 *      gpio_pin_t *boton = gpio_init2026(0, false, GPIO_ACTIVE_LOW,
 *                                         GPIO_PULLUP, GPIO_INTR_NEGEDGE);
 *      gpio_enable_interrupt(boton, boton_isr, boton);
 */
void gpio_enable_interrupt(gpio_pin_t *gpio, gpio_isr_callback_t cb, void *arg);

/**
 * @brief Deshabilita la interrupción de un pin GPIO y elimina su callback.
 *
 * Limpia la máscara de interrupción del pin en GPIO_INT_ENA y establece
 * gpio->callback y gpio->cb_arg en NULL, deteniendo cualquier invocación
 * futura del dispatcher para ese pin.
 *
 * No modifica la configuración del evento (int_type) almacenada en
 * GPIO_PINn_REG, por lo que una llamada posterior a gpio_enable_interrupt()
 * sobre el mismo pin restaura el comportamiento sin necesidad de
 * reinicializar el pin.
 *
 * @param[in]   gpio    Puntero a la estructura gpio_pin_t del pin objetivo.
 *
 * Ejemplo:
 *      // Deshabilitar temporalmente durante sección crítica
 *      gpio_disable_interrupt(boton);
 *      // ... operación crítica ...
 *      gpio_enable_interrupt(boton, boton_isr, boton);
 */
void gpio_disable_interrupt(gpio_pin_t *gpio);

#endif /* GPIO_2026_H */