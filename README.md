# Driver-ESP32 · Firmware Bare-Metal en C

**Driver de hardware en capas para la tarjeta Panther48 (ESP-WROOM-32)**  
Proyecto integrador — Software Embebido · ITCH · Semestre enero–junio 2026

![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v6.0.1-red?logo=espressif)
![Plataforma](https://img.shields.io/badge/Plataforma-ESP32-blue?logo=espressif)
![Lenguaje](https://img.shields.io/badge/Lenguaje-C-lightgrey?logo=c)
![Licencia](https://img.shields.io/badge/Licencia-Acad%C3%A9mica-green)
![Estado](https://img.shields.io/badge/Estado-Completado-brightgreen)

---

## Descripción

Firmware bare-metal en C para la tarjeta de desarrollo **Panther48 REV 1.0** basada en el módulo ESP-WROOM-32. El proyecto implementa una arquitectura de software embebido en cuatro capas bien definidas, accediendo directamente a los registros físicos del microcontrolador mediante `HWREG32` sin depender de la API de alto nivel de Espressif.

Los periféricos implementados son **GPIO**, **Timer Groups (TIMG)** con Watchdog, y **PWM (LEDC)**. Cada uno cuenta con su capa de driver de bajo nivel y su HAL correspondiente, integrados en un BSP específico para la Panther48.

---

## Arquitectura

```
┌──────────────────────────────────┐
│         Aplicación (main.c)      │   Solo incluye bsp_panther48.h
├──────────────────────────────────┤
│        BSP · bsp_panther48       │   Mapeo de pines de la Panther48
├───────────┬──────────┬───────────┤
│ hal_gpio  │hal_timer │  hal_pwm  │   Interfaces uniformes por periférico
├───────────┼──────────┼───────────┤
│gpio_2026  │timer_2026│ pwm_2026  │   Acceso directo a registros (HWREG32)
├───────────┴──────────┴───────────┤
│          Hardware ESP32          │   IO_MUX · GPIO Matrix · TIMG · LEDC
└──────────────────────────────────┘
```

Cada capa únicamente se comunica con la inmediatamente inferior. No se saltan niveles ni se generan dependencias cruzadas entre módulos del mismo nivel.

---

## Módulos

### `gpio_2026` / `hal_gpio`
Driver bare-metal para el periférico GPIO del ESP32. Accede directamente al IO_MUX y la Matriz GPIO para configurar pines, leer entradas, escribir salidas y gestionar interrupciones. Implementa un dispatcher global de ISR que enruta los eventos al callback del pin correspondiente. La HAL hace una abstracción de configuraciones correspondientes a salidas, entradas e interrupciones con identificadores lógicos y soporte de lógica activa alta/baja.

### `timer_2026` / `hal_timer`
Driver para los cuatro Timer Groups (TIMG0/TIMG1, Timer 0/1) del ESP32. Configura el prescaler a 80 para obtener resolución de 1 µs directa. Incluye lectura atómica de 64 bits mediante el registro `UPDATE`, retardos bloqueantes y funciones de control del MWDT/RWDT con de escritura protegida. La HAL abstrae la dualidad `group/num` detrás de un identificador único (`HAL_TIMER_1`…`HAL_TIMER_4`) y agrega utilidades como `periodo_cumplido()` para tareas periódicas no bloqueantes.

### `pwm_2026` / `hal_pwm`
Driver para el periférico LEDC (LED PWM Controller) del ESP32. Calcula el divisor `CLOCK_DIV` en formato punto fijo Q10.8 para la frecuencia deseada, inicializa canales HS/LS y los vincula a pines físicos a través de la GPIO Matrix escribiendo el índice de señal en `GPIO_FUNCn_OUT_SEL_CFG`. La HAL centraliza la configuración (frecuencia 5 kHz, resolución 8 bits) y expone control de duty en valor absoluto y porcentaje, con manejo correcto de las diferencias HS/LS en la actualización de `DUTY_START`.

### `bsp_panther48`
Board Support Package específico para la Panther48 REV 1.0 (diseñada en el ITCH por los estudiantes Miguel Maldonado y Jesús Joshua Muñoz Pacheco en 2022). Centraliza el mapeo de todos los pines en `board_pins.h` y expone una API estilo Arduino al código de aplicación. Incluye corrección automática de duty cycle para el LED RGB de ánodo común (`duty_hw = MAX_DUTY - brillo`), macros de color predefinidas (`BSP_COLOR_CYAN`, etc.) y separación entre `bsp_init()` y `bsp_pwm_rgb_init()` para no cargar el subsistema LEDC en aplicaciones que no lo necesitan.

---

## Hardware soportado (Panther48 REV 1.0)

| Componente | GPIO | Notas |
|---|---|---|
| LED1–LED5 | IO2, IO4, IO16, IO17, IO5 | Lógica activa alta, R=470Ω |
| RGB Rojo | IO14 | Ánodo común, lógica activa baja |
| RGB Verde | IO13 | Ánodo común, lógica activa baja |
| RGB Azul | IO12 | Ánodo común, lógica activa baja |
| SW1 | IO18 | Pull-up externa, filtro 100 nF |
| SW2 | IO19 | Pull-up externa, filtro 100 nF |
| BOOT | IO0 | Comparte función con flasheo |
| OLED SDA/SCL | IO21 / IO22 | I2C, pull-ups de 10 kΩ en placa |
| NTC / LDR | IO36 / IO39 | Input-only, sin pull interno |

> **Nota:** IO6–IO11 están conectados a la flash SPI interna; IO32–IO33 al cristal externo (No usar como GPIO). Para esta aplicación no se implementa lectura de el NTC ni LDR, tampoco se tiene soporte para OLED.

<img width="921" height="563" alt="Panther48" src="https://github.com/user-attachments/assets/e68a5d9c-fa15-469d-b745-b62721982a52" />


---

## Estructura del repositorio

```
Driver-ESP32/
├── components/
│   ├── gpio_2026/          # Driver GPIO (IO_MUX + GPIO Matrix)
│   ├── hal_gpio/           # HAL GPIO
│   ├── timer_2026/         # Driver Timer Groups + WDT
│   ├── hal_timer/          # HAL Timer
│   ├── pwm_2026/           # Driver LEDC PWM
│   ├── hal_pwm/            # HAL PWM
│   └── bsp_panther48/      # Board Support Package Panther48
│       ├── board_pins.h    # Único archivo con números de GPIO en crudo
│       ├── bsp_panther48.h # Interfaz pública (único include para la APP)
│       └── bsp_panther48.c
├── main/
│   └── main.c              # Aplicación de demostración (menú + 3 demos)
├── .devcontainer/          # Dev Container para VS Code
├── .vscode/
├── .clangd
└── CMakeLists.txt
```

---

## Requisitos

| Herramienta | Versión |
|---|---|
| ESP-IDF | v5.x / v6.x |
| CMake | ≥ 3.16 |
| Python | ≥ 3.8 |
| Git | ≥ 2.30 |

---



## Uso desde la aplicación

La APP solo debe incluir un archivo:

```c
#include "bsp_panther48.h"

void app_main(void)
{
    // Inicialización obligatoria
    bsp_init();

    // Opcional: activar PWM para el LED RGB
    bsp_pwm_rgb_init();

    // LEDs individuales
    bsp_led_on(1);
    bsp_delay_ms(500);
    bsp_led_off(1);

    // LED RGB con colores predefinidos
    bsp_rgb_set(BSP_COLOR_CYAN);
    bsp_delay_ms(1000);

    // LED RGB con control PWM en porcentaje
    bsp_pwm_rgb_set_percent(100, 0, 50);   // Rojo 100%, Verde 0%, Azul 50%

    // Lectura de botones
    if (bsp_sw1_pressed()) {
        bsp_led_toggle(2);
    }

    // Interrupciones (callback debe llevar IRAM_ATTR)
    bsp_sw2_irq_attach(mi_isr, NULL);
}
```

> La capa de aplicación nunca debe referenciar números de GPIO, tipos de la HAL ni constantes de lógica activa. Todo ese conocimiento vive en el BSP.

---

## Aplicación de demostración

`main.c` implementa un menú navegable con SW1 (avanzar) y SW2 (seleccionar/salir):

| Demo | Descripción |
|---|---|
| **LEDs** | Barrido circular de LED1 al LED5. SW1 cicla entre velocidades: 200 / 100 / 50 ms |
| **RGB** | 7 colores fijos + modo *breath* (magenta, animación no bloqueante por timestamp) |
| **Rotas** | 3 patrones con LED1–LED4: IDA, PING-PONG, 2EN2. SW1 cambia patrón en tiempo real |

---

## Equipo

Proyecto desarrollado por estudiantes de Ingeniería Electrónica de la especialidad de Inteligencia Artificial en Sistemas Embebidos del ITCH.

| Integrante | Módulo asignado |
|---|---|
| Ana Cardona  | `timer_2026` · `hal_timer` · `pwm_2026` · `hal_pwm` |
| Emiliano Pérez | `gpio_2026` · `hal_gpio` |
| Luis Anchondo  |  `bsp_panther48` · `main.c` (aplicación) |

**Docente:** M.C. Alfredo Chacón Aldama  
**Materia:** Software Embebido · Semestre enero–junio 2026

---

## Referencia técnica

- ESP32 Technical Reference Manual v4.6 — Espressif Systems
- Oravatec, "Las 4 capas del software embebido"
- A. Chacón Aldama, *Modelos de Programación para Software Embebido*, ITCH 2026
- A. Chacón Aldama, *Control de Versiones con GIT*, ITCH 2021

---

<p align="center">
  Instituto Tecnológico de Chihuahua · Chihuahua, México · 2026
</p>
