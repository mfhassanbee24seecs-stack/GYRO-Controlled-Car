#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "Wire.h"
#include "MPU6050.h"

#define CSN_GPIO      8
#define CE_GPIO       7
#define TX_LED        3

RF24 radio(CE_GPIO, CSN_GPIO);
MPU6050 accelgyro;

const byte Address[6] = "00001";

int16_t ax, ay, az;
int16_t gx, gy, gz;

unsigned char Tx_command = 0;
unsigned char Speed_index = 0;
unsigned char Tx_Array[2];

void setup() {
  Serial.begin(115200);
  pinMode(TX_LED, OUTPUT);
  digitalWrite(TX_LED, HIGH);

  if (!radio.begin()) {
    Serial.println("ERROR: Radio not found! Check wiring.");
    while (1);
  }

  radio.setPALevel(RF24_PA_MIN);
  radio.setChannel(76);
  radio.setAutoAck(false);        // ✅ KEY FIX
  radio.setDataRate(RF24_1MBPS);  // ✅ Explicit data rate
  radio.openWritingPipe(Address);
  radio.stopListening();

  Wire.begin();
  Serial.println("Initializing MPU6050...");
  accelgyro.initialize();
  Serial.println(accelgyro.testConnection() ? "MPU6050 OK" : "MPU6050 FAILED");
  Serial.println("=== Transmitter Ready ===");
}

void loop() {
  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  if (ay <= -4000) {
    Tx_command  = 1;
    Speed_index = (ay + 4000) / -2000 + 1;
    if (Speed_index > 5) Speed_index = 5;
  }
  else if (ay >= 4000) {
    Tx_command  = 2;
    Speed_index = (ay - 4000) / 2000 + 1;
    if (Speed_index > 5) Speed_index = 5;
  }
  else if (ax <= -4000) {
    Tx_command  = 4;
    Speed_index = (ax + 4000) / -2000 + 1;
    if (Speed_index > 5) Speed_index = 5;
  }
  else if (ax >= 4000) {
    Tx_command  = 3;
    Speed_index = (ax - 4000) / 2000 + 1;
    if (Speed_index > 5) Speed_index = 5;
  }
  else {
    Tx_command  = 0;
    Speed_index = 0;
  }

  Tx_Array[0] = Tx_command;
  Tx_Array[1] = Speed_index;
  radio.write(&Tx_Array, 2);  // No need to check return value now

  Serial.print("Command: "); Serial.print(Tx_command);
  Serial.print(" | Speed: "); Serial.print(Speed_index);
  Serial.print(" | Tilt Y: "); Serial.print(ay);
  Serial.print(" | Tilt X: "); Serial.println(ax);

  delay(100);
}
