// FileName:        gpio_2026.h                                                                                            
// Dependencies:    None                                                                                              
// Processor:       Tensilica Xtensa LX6 160 MHz                                                                                             
// Board:           ESP-WROOM-32                                                                                  
// Program version: 1.0                                                                                  
// Company:         Instituto Tecnologico de Chihuahua                                                                                 
// Description:     Declaración de funciones de configuración
//                  control GPIO para ESP32. Construcción de estructura 
//                  gpio_pin_t y configuración hardware.                               
// Autor:           Ana Paola Cardona Valenzuela
//                  Luis Adrian Anchondo Carreón
//                  Emiliano Perez Dyck 
// Updated:         31/05/2026

#ifndef GPIO_2026_H
#define GPIO_2026_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

// Macro registers access
#define HWREG32(x) (*((volatile unsigned int *)(x)))

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
#define READ_PIN(pin) \
    gpio_read(&gpio_table[pin])

#define WRITE_PIN(pin, value) \
    gpio_write(&gpio_table[pin], value)

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

// IOMUX REGISTERS GPIO0-39

// Tabla de direcciones de los registros de configuración de pines
extern const uint32_t GPIO_MUX_REGS[];

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

// Estructura para configuración de pin GPIO
typedef struct {
    uint8_t pin;
    bool is_output;
    gpio_logic_t logic;
    pull_mode_t pull_mode;
    gpio_int_type_t int_type;
} gpio_pin_t;

// Tabla de pines GPIO configurados
extern gpio_pin_t gpio_table[];

// -----------------------  Prototipos de funciones --------------------------

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

#endif /* GPIO_2026_H */