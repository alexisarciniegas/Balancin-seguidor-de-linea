/*
 * config.h — Configuración del robot para el sketch de identificación de motores
 * Control Inteligente II (168418) · Universidad de Pamplona · Ing. Jeisson Martínez Flórez
 *
 * ESTE ES EL ÚNICO ARCHIVO QUE CADA GRUPO MODIFICA.
 * Copie aquí los pines de SU PCB. Los valores de abajo son los de mi robot (del docente).
 *
 * Convención de signos (la misma para todo el curso):
 *   - PWM positivo  = la rueda gira hacia ADELANTE.
 *   - Cuentas positivas = la rueda giró hacia ADELANTE.
 * Si el comando "d" (prueba de sentido) dice lo contrario, cambie el SIGNO correspondiente.
 */
#pragma once

// ============================================================================
// 1. MOTORES — driver TB6612FNG
//    Rueda izquierda (L) y rueda derecha (R). Escriba el número de GPIO.
// ============================================================================
#define MOTOR_L_PWM   17    // PWMB en el robot del docente
#define MOTOR_L_IN1   15    // BIN1
#define MOTOR_L_IN2   16    // BIN2

#define MOTOR_R_PWM    4    // PWMA
#define MOTOR_R_IN1    6    // AIN1
#define MOTOR_R_IN2    5    // AIN2

#define PIN_STBY      7    // Pin STBY del TB6612. Si STBY va fijo a 3.3 V, escriba -1

#define MOTOR_L_SIGNO -1    // +1 o -1: hace que PWM positivo mueva la rueda hacia adelante
#define MOTOR_R_SIGNO -1

// ============================================================================
// 2. ENCODERS — cuadratura A/B
//    Si un canal B está dañado o no existe, escriba -1: el sketch cuenta solo
//    con el canal A (x2) y toma el sentido del PWM aplicado.
// ============================================================================
#define ENC_L_A       1
#define ENC_L_B       2    // Robot del docente: el sensor B izquierdo está dañado
#define ENC_R_A       42
#define ENC_R_B       41    // GPIO 46 es pin de arranque: desconéctelo para cargar el programa

#define ENC_L_SIGNO   -1    // +1 o -1: hace que avanzar cuente positivo
#define ENC_R_SIGNO   +1

// Cuentas por vuelta de la RUEDA (no del eje del motor). Mídalas con el comando "c".
#define CUENTAS_VUELTA_L  3290.9f
#define CUENTAS_VUELTA_R  3290.9f

// ============================================================================
// 3. PWM
// ============================================================================
#define PWM_FREQ_HZ   20000   // 20 kHz: fuera del rango audible
#define PWM_BITS      8       // Resolución: 0..255
#define PWM_MAX       255     // Tope de seguridad para todos los ensayos

// ============================================================================
// 4. ENSAYOS
// ============================================================================
#define SERIAL_BAUD     115200
#define MAX_MUESTRAS    4000      // 16 bytes por muestra: 64 kB de RAM
#define PWM_SENTIDO     80        // PWM de la prueba de sentido ("d")

// Escalón: tramos en ms y periodo de muestreo
#define ESCALON_ANTES_MS     300
#define ESCALON_DURANTE_MS   1500
#define ESCALON_DESPUES_MS   700
#define ESCALON_TS_US        2000  // 2 ms

// Escalera: duración de cada peldaño y periodo de muestreo
#define ESCALERA_PELDANO_MS  400
#define ESCALERA_TS_US       5000  // 5 ms
