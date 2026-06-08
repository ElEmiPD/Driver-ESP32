// FileName:        board_pins.h
// Dependencies:    None
// Processor:       Tensilica Xtensa LX6 160 MHz
// Board:           ESP32 ITCH (Panther48)  REV 1.0
// Program version: 2.0
// Company:         Instituto Tecnologico de Chihuahua
// Description:     Mapeo COMPLETO y EXCLUSIVO de pines físicos de la tarjeta.
//                  Derivado directamente del esquemático REV 1.0
//                  (Drawn by: Miguel Maldonado, 2022-07-05).
//
//                  REGLA: Este es el ÚNICO archivo del proyecto que contiene
//                  números de GPIO en crudo. Cualquier capa superior
//                  (BSP, HAL, APP) referencia estos nombres, nunca los números.
//                  Si la tarjeta cambia de revisión, solo se edita este archivo.
//
// Autores:         Ana Paola Cardona Valenzuela
//                  Emiliano Perez Dyck
//                  Luis Adrian Anchondo Carreón
// Created:         02/06/2026
// Updated:         04/06/2026

#ifndef BOARD_PINS_H
#define BOARD_PINS_H

// ===========================================================================
//  LEDs individuales 
//  Circuito: 3.3V -> R(470Ω) -> LED -> GPIO
//  Lógica activa: LOW  (cátodo al GPIO -> GPIO=0 enciende)
//  Componentes: U5-U9 (0603), R9-R13 (470Ω)
// ===========================================================================
#define BOARD_LED1_PIN          2   // IO2  – U9
#define BOARD_LED2_PIN          4   // IO4  – U8
#define BOARD_LED3_PIN         16   // IO16 – U7
#define BOARD_LED4_PIN         17   // IO17 – U6
#define BOARD_LED5_PIN          5   // IO5  – LEDB (D803)

#define BOARD_LED_LOGIC         GPIO_ACTIVE_HIGH
#define BOARD_LED_COUNT         5

// ===========================================================================
//  LED RGB  (ánodo común a 3.3V, cátodos a GPIO)
//  Lógica activa: LOW  (GPIO=0 enciende ese canal)
//  Componente: RBG FM-B2020RGBA-HG
//  Resistencias: R14(470Ω)->IO14 R,  R15(470Ω)->IO12 B,  R16(470Ω)->IO13 G
//  NOTA: el silk de la tarjeta dice "RBG" (no RGB); el orden real es:
//        IO14 = Rojo, IO12 = Azul, IO13 = Verde
// ===========================================================================
#define BOARD_RGB_R_PIN        14   // IO14 – Canal Rojo
#define BOARD_RGB_G_PIN        13   // IO13 – Canal Verde
#define BOARD_RGB_B_PIN        12   // IO12 – Canal Azul

#define BOARD_RGB_LOGIC        GPIO_ACTIVE_LOW

// ===========================================================================
//  Botones / Pulsadores
//  Circuito: GPIO -> C(100nF) -> GND  +  pull-up externa a 3.3V
//  Al presionar: GPIO se conecta a GND -> nivel bajo
//  Lógica activa: LOW  (presionado = LOW = activo = true)
//  Componentes: TS-1109S-C-D
// ===========================================================================
#define BOARD_BTN_SW1_PIN      18   // IO18 – SW1
#define BOARD_BTN_SW2_PIN      19   // IO19 – SW2
#define BOARD_BTN_BOOT_PIN      0   // IO0  – BOOT (comparte función bootloader)

#define BOARD_BTN_PULL          GPIO_PULLUP
#define BOARD_BTN_LOGIC         GPIO_ACTIVE_LOW
#define BOARD_BTN_IRQ_EDGE      INT_FLANCO_NEG   // flanco de bajada al presionar

// ===========================================================================
//  Display OLED I2C 128×64
//  Componente: OLED1 (Oled I2C OLED 128×44 YG)
//  Conector: PZ254V-11-02P
//  Pull-ups de bus I2C: R17, R18 (10kΩ a 3.3V) ya en placa
// ===========================================================================
#define BOARD_OLED_SDA_PIN     21   // IO21 – I2C SDA
#define BOARD_OLED_SCL_PIN     22   // IO22 – I2C SCL

// ===========================================================================
//  Sensores analógicos  (ADC1  solo lectura, 12 bits, ref 3.3V)
//  Estos pines son input-only: no tienen resistencias de pull ni salida.
//  Conector: H1, H2 (PZ254V-11-02P headers)
// ===========================================================================
#define BOARD_NTC_PIN          36   // SENSOR_VP (IO36 / ADC1_CH0) – NTC 10k 3950
#define BOARD_LDR_PIN          39   // SENSOR_VN (IO39 / ADC1_CH3) – LDR GL5537-1

// ===========================================================================
//  Cristal externo  (RESERVADOS  no usar como GPIO en ningún caso)
//  Componente: K1 (cristal de cuarzo)
// ===========================================================================
#define BOARD_XTAL_N_PIN       32   // IO32 – XTAL_N  (reservado)
#define BOARD_XTAL_P_PIN       33   // IO33 – XTAL_P  (reservado)

// ===========================================================================
//  UART  (CH340C  USB ↔ Serial)
//  Velocidad típica de programación / debug: 115200 baud
// ===========================================================================
#define BOARD_UART_TX_PIN       1   // TXD0 – hacia CH340C RXD
#define BOARD_UART_RX_PIN       3   // RXD0 – desde CH340C TXD

// ===========================================================================
//  Pines de control del CH340C  (hardware reset / bootloader)
//  Gestionados automáticamente por el circuito Q1/Q2 + R1/R2
//  No manipular desde la aplicación.
// ===========================================================================
#define BOARD_EN_PIN           -1   // EN – controlado por Q1 (no usar)
#define BOARD_IO0_CTRL_PIN     -1   // IO0 ctrl – controlado por Q2 (no usar)

// ===========================================================================
//  Resumen de pines disponibles para el usuario en la tarjeta
//  (los que no están reservados por periféricos de placa)
// ===========================================================================
//  Salidas libres:   ninguna adicional en REV 1.0 (todos mapeados)
//  Entradas libres:  IO34, IO35 (input-only, sin pull interno)
//  IO32, IO33:       reservados para cristal
//  IO6-IO11:         reservados para flash SPI interna (no accesibles)

#endif /* BOARD_PINS_H */