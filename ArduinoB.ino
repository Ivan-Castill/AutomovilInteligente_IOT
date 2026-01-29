#include <Servo.h>
#include <SoftwareSerial.h>

// --- Configuración ESP-01 ---
SoftwareSerial esp8266(12, 13); 
String ssid = "CAMPUS_EPN";
String password = "politecnica**";
String apiKey = "UY1KCPELRUS4EUW3"; 
String host = "api.thingspeak.com";

// Pines Sensores
const int pinFuego = A0;
const int pinGas = A1;
const int trigPin = 9;
const int echoPin = 10; 

// Pines LEDs
const int ledVerdeGas = 2, ledRojoGas = 3;
const int ledVerdeFuego = 4, ledRojoFuego = 5;
const int ledAmarillo = 6, ledNaranja = 7, ledAzul = 8;
const int buzzer = A2; 
const int pinServo = A3; // Servo en A3

Servo radarServo;
int angulo = 0, paso = 2;
unsigned long ultimoMovimientoRadar = 0; 
const int velocidadRadar = 40; 
bool incrementando = true;
unsigned long ultimoEnvio = 0;
unsigned long ultimaMedicionDist = 0;

void setup() {
  Serial.begin(9600);
  
  
  for(int i=2; i<=8; i++) pinMode(i, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  esp8266.begin(9600);
  setupWiFi(); 
  
  radarServo.attach(pinServo);
  radarServo.write(90);
  Serial.println("Sistema Iniciado - Servo en A3");
}

void loop() {
  //LEER SENSORES ANALÓGICOS (Primero leemos para tener los datos)
  int valorFuego = analogRead(pinFuego);
  int valorGas = analogRead(pinGas);
  
  //MEDIR DISTANCIA (Cada 150ms)
  static long distancia = 0; // Static guarda el valor entre vueltas
  
  if (millis() - ultimaMedicionDist > 150) {
    distancia = obtenerDistanciaBlindada();
    ultimaMedicionDist = millis();
    
    //imprimir datos
    Serial.print("Dist: "); Serial.print(distancia);
    Serial.print(" cm | Gas: "); Serial.print(valorGas);
    Serial.print(" | Fuego: "); Serial.println(valorFuego);

    actualizarSemaforo(distancia);
  }

  //LÓGICA DE ALERTA
  bool hayGas = (valorGas > 400);
  bool hayFuego = (valorFuego < 300);

  //Control LEDs
  digitalWrite(ledRojoGas, hayGas);
  digitalWrite(ledVerdeGas, !hayGas);
  digitalWrite(ledRojoFuego, hayFuego);
  digitalWrite(ledVerdeFuego, !hayFuego);

  // Acción
  if (hayGas || hayFuego) {
    ejecutarAlarmaGeneral();
  } else {
    noTone(buzzer);
    actualizarMovimientoRadar();
    radarServo.write(angulo);
  }

  //ENVIAR A INTERNET (Cada 20 seg)
  if (millis() - ultimoEnvio > 20000) {
    enviarThingSpeak(distancia, valorGas, valorFuego);
    ultimoEnvio = millis();
  }
  
  delay(10); 
}


long obtenerDistanciaBlindada() {
  //Silenciamos WiFi para medir bien
  esp8266.stopListening(); 
  
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duracion = pulseIn(echoPin, HIGH, 25000); 
  
  //Reactivamos WiFi
  esp8266.listen(); 
  
  if (duracion == 0) return 268; // Si falla, devolvemos valor seguro (lejos)
  return duracion / 59;
}



void actualizarSemaforo(long d) {
  digitalWrite(ledAzul, d > 30);
  digitalWrite(ledAmarillo, d <= 30 && d > 15);
  digitalWrite(ledNaranja, d <= 15);
}

void ejecutarAlarmaGeneral() {
  tone(buzzer, 1200);
  digitalWrite(ledAzul, (millis() / 200) % 2); 
}

void actualizarMovimientoRadar() {
  if (millis() - ultimoMovimientoRadar > velocidadRadar) {
    
    ultimoMovimientoRadar = millis();
  
    if (incrementando) {
      angulo += paso;
      if (angulo >= 180) incrementando = false;
    } else {
      angulo -= paso;
      if (angulo <= 0) incrementando = true;
    }
  }
}



void setupWiFi() {
  Serial.println("Configurando WiFi...");
  enviarComando("AT+RST", 2000);
  enviarComando("AT+CWMODE=1", 1000);
  String cmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  enviarComando(cmd, 8000); 
  enviarComando("AT+CIFSR", 2000); 
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
  if (resp.length() > 0) {
    // Solo descomentar si quieres ver la basura del ESP en pantalla
    // Serial.print(resp); 
  }
  return resp;
}

void enviarThingSpeak(long d, int g, int f) {
  // Aseguramos que estamos escuchando al ESP
  esp8266.listen();
  while(esp8266.available()) esp8266.read(); 

  String peticion = "GET /update?api_key=" + apiKey + "&field1=" + String(d) + "&field2=" + String(g) + "&field3=" + String(f);
  peticion += " HTTP/1.1\r\n";
  peticion += "Host: " + host + "\r\n";
  peticion += "Connection: close\r\n\r\n";
  
  Serial.println(">> Enviando datos a la nube...");
  enviarComando("AT+CIPSTART=\"TCP\",\"" + host + "\",80", 4000);
  enviarComando("AT+CIPSEND=" + String(peticion.length()), 2000);
  esp8266.print(peticion);
  
  long t = millis();
  while(millis() - t < 3000){
    if(esp8266.available()) Serial.write(esp8266.read());
  }
  Serial.println("\n>> Envio finalizado.");
}