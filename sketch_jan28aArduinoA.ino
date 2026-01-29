
#include <SoftwareSerial.h>

SoftwareSerial BT(12, 13); // RX, TX

// Pines que funcionaban con tu Joystick
const int IN1 = 8; 
const int IN2 = 9;
const int IN3 = 10;
const int IN4 = 11;

void setup() {
  Serial.begin(9600);
  BT.begin(9600);
  
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  
  Serial.println("Sistema corregido. Esperando Bluetooth...");
}

void loop() {
  if (BT.available() > 0) {
    int dato = BT.read(); 
    Serial.print("Dato recibido: ");
    Serial.println(dato); // Esto te ayuda a ver en la PC qué llega
    
    if (dato == 70 || dato == 'F') { 
      adelante();
    } 
    else if (dato == 66 || dato == 'B') { 
      atras(); 
    }
    else if (dato == 76 || dato == 'L') { // 76 es 'L' (Left)
      izquierda();
    }
    else if (dato == 82 || dato == 'R') { // 82 es 'R' (Right)
      derecha();
    }
    else if (dato == 83 || dato == 'S') { 
      parar();
    }
  }
}

// --- AQUÍ DECLARAMOS LAS FUNCIONES PARA QUE NO DEN ERROR ---

void izquierda() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH); // Motor A atrás
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);  // Motor B adelante
  Serial.println("Giro Izquierda");
}

void derecha() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);  // Motor A adelante
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH); // Motor B atrás
  Serial.println("Giro Derecha");
}

void adelante() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  Serial.println("Moviendo: Adelante");
}

void atras() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  Serial.println("Moviendo: Atras");
}

void parar() {
  digitalWrite(IN1, LOW);  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, LOW);
  Serial.println("Moviendo: Parar");
}