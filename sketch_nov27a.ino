#include <Servo.h>
#include <SoftwareSerial.h>

// --- Configuración ESP-01 ---
SoftwareSerial esp8266(12, 13); // RX (Pin 12), TX (Pin 13)
String ssid     = "CAMPUS_EPN";
String password = "politecnica**";
String apiKey   = "UY1KCPELRUS4EUW3"; 
String host     = "api.thingspeak.com";

// Pines Sensores
const int pinFuego = A0;   
const int pinGas = A1;     
const int trigPin = 9;
const int echoPin = 10;

// Pines LEDs
const int ledAzul = 8, ledVerde = 3, ledAmarillo = 6, ledNaranja = 7, ledRojo = 4;
const int ledAlertaFuego = 2, buzzer = 5, pinServo = 11; 

Servo radarServo;
int angulo = 0, paso = 5; 
bool incrementando = true;
unsigned long ultimoEnvio = 0;

void setup() {
  Serial.begin(9600);
  
  // 1. Apagamos todo lo que consuma energía al inicio
  pinMode(ledAlertaFuego, OUTPUT);
  digitalWrite(ledAlertaFuego, LOW);
  noTone(buzzer);
  
  Serial.println("Esperando que el voltaje se estabilice...");
  delay(3000); // Pausa de 3 segundos para que los condensadores se carguen
  
  // 2. Iniciamos el ESP-01
  esp8266.begin(9600); // O 9600 según tu módulo
  setupWiFi(); 
  
  // 3. Hasta el final, activamos el servo
  radarServo.attach(pinServo);
  radarServo.write(0);
  Serial.println("Sistema listo.");
}

void loop() {
  long distancia = obtenerDistancia();
  int valorFuego = analogRead(pinFuego);
  int valorGas = analogRead(pinGas);

  // 1. Lógica de Radar y Alarma Local
  if (valorFuego < 300 || valorGas > 400) {
    ejecutarAlarmaGeneral();
  } else {
    noTone(buzzer);
    digitalWrite(ledAlertaFuego, LOW);
    actualizarSemaforo(distancia);
    radarServo.write(angulo);
    actualizarMovimientoRadar();
  }

  // 2. Envío a ThingSpeak y Debug en Terminal cada 20 segundos
  if (millis() - ultimoEnvio > 20000) {
    Serial.println("\n------------------------------------");
    Serial.println("PREPARANDO ENVIO A THINGSPEAK:");
    Serial.print("Distancia: "); Serial.print(distancia); Serial.println(" cm");
    Serial.print("Gas: "); Serial.println(valorGas);
    Serial.print("Fuego (IR): "); Serial.println(valorFuego);
    
    enviarThingSpeak(distancia, valorGas, valorFuego);
    
    Serial.println("------------------------------------\n");
    ultimoEnvio = millis();
  }

  delay(50); 
}

void actualizarMovimientoRadar() {
  if (incrementando) {
    angulo += paso;
    if (angulo >= 180) incrementando = false;
  } else {
    angulo -= paso;
    if (angulo <= 0) incrementando = true;
  }
}

void setupWiFi() {
  Serial.println("Estabilizando energia...");
  delay(2000); // Espera a que el voltaje se asiente
  
  enviarComando("AT+RST", 2000);
  enviarComando("AT+CWMODE=1", 1000);
  
  Serial.println("Intentando conectar a WiFi...");
  // Aumentamos el tiempo de espera para no saturar al procesador
  String cmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  enviarComando(cmd, 8000); 
  
  enviarComando("AT+CIFSR", 2000); 
}

void enviarThingSpeak(long d, int g, int f) {
  while(esp8266.available()) esp8266.read(); 

  // Construimos una petición HTTP 1.1 completa
  String peticion = "GET /update?api_key=" + apiKey + "&field1=" + String(d) + "&field2=" + String(g) + "&field3=" + String(f);
  peticion += " HTTP/1.1\r\n";
  peticion += "Host: api.thingspeak.com\r\n";
  peticion += "Connection: close\r\n\r\n"; // El doble salto de línea final es vital
  
  Serial.println(">> Conectando a TCP...");
  enviarComando("AT+CIPSTART=\"TCP\",\"" + host + "\",80", 3000);
  
  Serial.print(">> Enviando longitud: "); Serial.println(peticion.length());
  enviarComando("AT+CIPSEND=" + String(peticion.length()), 1500);
  
  // Enviamos la cadena completa
  esp8266.print(peticion);
  
  // Leemos la respuesta del servidor para ver el "Status 200"
  long t = millis();
  while(millis() - t < 2000){
    if(esp8266.available()) Serial.write(esp8266.read());
  }
}

String enviarComando(String cmd, int timeout) {
  String resp = "";
  esp8266.println(cmd);
  long t = millis();
  while ((millis() - t) < timeout) {
    while (esp8266.available()) {
      char c = (char)esp8266.read();
      resp += c;
    }
  }
  // Esto imprime la respuesta "cruda" del ESP-01 para saber si dio OK o ERROR
  if (resp.length() > 0) {
    Serial.print("Respuesta ESP: ");
    Serial.println(resp);
  }
  return resp;
}

void actualizarSemaforo(long d) {
  digitalWrite(ledAzul,     d > 40);
  digitalWrite(ledVerde,    d <= 40 && d > 30);
  digitalWrite(ledAmarillo, d <= 30 && d > 20);
  digitalWrite(ledNaranja,  d <= 20 && d > 10);
  digitalWrite(ledRojo,     d <= 10);
}

void ejecutarAlarmaGeneral() {
  digitalWrite(ledAlertaFuego, HIGH);
  tone(buzzer, 1200, 100); 
  digitalWrite(ledAzul, HIGH); digitalWrite(ledRojo, LOW);
  delay(100);
  digitalWrite(ledAzul, LOW); digitalWrite(ledRojo, HIGH);
}

long obtenerDistancia() {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duracion = pulseIn(echoPin, HIGH, 20000); 
  return (duracion == 0) ? 100 : duracion / 59;
}