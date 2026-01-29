# Automóvil Inteligente IoT — README

Proyecto: Automóvil Inteligente con Arquitectura de 2 Bloques (Tracción y Sensores/Acción)  
Descripción: Vehículo dividido en dos bloques: Bloque A (Tracción) controla motores y recibe comandos por Bluetooth (HC-05). Bloque B (Sensores/Acción) gestiona sensores, servos, relé, indicadores y envía telemetría al ESP-01 para subir datos a la nube o servidor local.

## Contenido
- Propósito
- Bloques y responsabilidades
- Esquema de alimentación y tierra común
- Conexiones (pines sugeridos)
- Comunicación y protocolo
- Ejemplos de código (Arduino A, Arduino B, ESP-01)
- Lista de materiales (BOM)
- Recomendaciones de montaje y seguridad
- Solución de problemas rápida

---

## Propósito
Este proyecto demuestra un automóvil IoT modular con:
- Bloque A: control de movimiento por Bluetooth.
- Bloque B: adquisición de sensores, control de actuadores secundarios y envío de telemetría vía ESP-01.

---

## Bloques y responsabilidades

1. Bloque de Tracción (Arduino A)
- Alimentación: 9V / 1A (motores por L298N).
- Arduino A controla L298N y HC-05.
- Actuadores: 2 motores reductores conectados al L298N.
- Comunicación: HC-05 (Bluetooth) para recibir comandos del móvil.

2. Bloque de Sensores y Acción (Arduino B)
- Alimentación: 5V / 2A (servos, ESP, sensores).
- Sensores: HC-SR04 (sonar), sensor infrarrojo (obstáculo/línea), sensor de gas (MQ series).
- Actuadores: Servomotor SG90, módulo relé (bomba), buzzer, 5 LEDs (indicadores).
- Comunicación: ESP-01 (ESP8266) para envío de telemetría a servidor/IoT broker.

---

## Esquema de alimentación y tierra común
- Compartir GND entre todas las fuentes (motor 9V, lógica 5V, regulador 3.3V).
- Separar alimentación de motores (VMOT en L298N) de la lógica (5V).
- ESP-01 requiere 3.3V estable y capaz de picos ~300–400 mA (usar regulador con decoupling).
- No alimentar ESP-01 con 5V.

---

## Pines sugeridos (usar estos si no especificas otros)

Arduino A (Tracción)
- IN1 -> D8
- IN2 -> D9
- IN3 -> D10
- IN4 -> D11
- ENA -> D5 (PWM)
- ENB -> D6 (PWM)
- Bluetooth HC-05: RX -> D2 (SoftwareSerial RX), TX -> D3 (SoftwareSerial TX)
- Motor power -> L298N VMOT (9V)

Arduino B (Sensores/Acción)
- HC-SR04: TRIG -> D7, ECHO -> D6
- Sensor IR (obstáculo/línea): OUT -> D4
- Sensor MQ: AOUT -> A0
- Servomotor SG90: signal -> D3, Vcc -> 5V separado, GND común
- Relay (bomba): IN -> D2
- Buzzer -> D13
- LEDs -> A1..A5 (cada uno con resistor 220Ω)
- ESP-01 <-> Arduino B: Serial por SoftwareSerial en D10 (RX) / D11 (TX). IMPORTANTE: usa conversor de niveles o divisores para RX del ESP.

---

## Comunicación y protocolo sugerido

Bluetooth (HC-05)
- Baud por defecto: 9600
- Comandos simples (ejemplo):
  - 'F' -> adelante
  - 'B' -> atrás
  - 'L' -> izquierda
  - 'R' -> derecha
  - 'S' -> stop
  - Opcional: "V:150" -> velocidad PWM

ESP-01 (Telemetría)
- Recomendado: PROGRAMAR el ESP-01 con un sketch que reciba líneas por Serial desde Arduino B y las publique a tu servidor vía HTTP o MQTT.
- Alternativa: usar AT commands desde Arduino B (menos fiable, SoftwareSerial y firmware AT pueden complicar).
- Formato de telemetría sugerido (línea por \n):
  DIST:xxx;MQ:yyy;IR:0/1;SERVOPOS:nn

---

## Ejemplos de sketches incluidos
- ArduinoA.ino (control motores + HC-05)
- ArduinoB.ino (lectura sensores, control servo/relay/leds, envía telemetría por Serial al ESP)
- ESP01_gateway.ino (programar en el ESP-01: recibe telemetría por Serial y hace HTTP POST a tu servidor)

---

## Lista de materiales (BOM)
- 2x Arduino Uno/Nano (A y B)
- 1x L298N driver
- 2x Motores DC reductores
- 1x HC-05 Bluetooth
- 1x ESP-01 (ESP8266)
- 1x HC-SR04
- 1x Sensor infrarrojo (obstáculo/línea)
- 1x Sensor MQ (MQ-2, MQ-5, etc.)
- 1x Servomotor SG90
- 1x Módulo relay 5V
- 1x Buzzer
- 5x LEDs + resistencias
- Fuentes: 9V/1A (motores), 5V/2A (lógica/servos), 3.3V regulada (ESP-01 ≥500 mA recomendado)
- Cableado, convertidores de nivel, condensadores electrolíticos

---

## Recomendaciones y seguridad
- Nunca conectar ESP-01 a 5V.
- Mantén GND común.
- Usa reguladores con capacidad suficiente y condensadores cerca de los módulos.
- Aísla la bomba con el relé y respeta normas eléctricas.

---

## Solución de problemas rápida
- ESP reinicia al conectar WiFi: fuente 3.3V insuficiente.
- Motor no gira: revisar VMOT y GND, probar motores con batería directa.
- Lecturas MQ inestables: dejar precalentar y calibrar.
- HC-05 no empareja: revisar alimentación y PIN 1234/0000.

---

## Siguientes pasos que puedo hacer por ti
- Ajustar pines a tu preferencia.
- Generar diagramas de conexión (Fritzing).
- Generar versiones del código con comentarios ampliados.
- Preparar un ejemplo de backend HTTP/MQTT (servidor local o endpoint).
