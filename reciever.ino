#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CSN_GPIO        8
#define CE_GPIO         7
#define RIGHT_FORWARD   5
#define RIGHT_BACKWARD  4
#define LEFT_FORWARD    3
#define LEFT_BACKWARD   2

RF24 radio(CE_GPIO, CSN_GPIO);
const byte Address[6] = "00001";

unsigned char Received_Command = 0;
unsigned char Speed_index      = 0;
unsigned char Run_Stop_Mode    = 0;
unsigned char Rx_Array[2];
unsigned int  Run_Stop_Counter = 0;

void setup() {
  Serial.begin(115200);

  pinMode(RIGHT_FORWARD,  OUTPUT);
  pinMode(RIGHT_BACKWARD, OUTPUT);
  pinMode(LEFT_FORWARD,   OUTPUT);
  pinMode(LEFT_BACKWARD,  OUTPUT);

 // WITH this:
if (!radio.begin()) {
    Serial.println("WARNING: Radio init failed, trying anyway...");
    // continue anyway
}

  radio.setPALevel(RF24_PA_MIN);
  radio.setChannel(76);
  radio.setAutoAck(false);        // ✅ KEY FIX - must match transmitter
  radio.setDataRate(RF24_1MBPS);  // ✅ Must match transmitter
  radio.openReadingPipe(1, Address);
  radio.startListening();

  Serial.println("=== Receiver Ready ===");
}

void loop() {
  if (radio.available()) {
    radio.read(&Rx_Array, 2);
    Received_Command = Rx_Array[0];
    Speed_index      = Rx_Array[1];

    Serial.print("Command: "); Serial.print(Received_Command);
    Serial.print(" | Speed: "); Serial.print(Speed_index);
    Serial.print(" | Action: ");
    switch (Received_Command) {
      case 0: Serial.println("STOP");    break;
      case 1: Serial.println("FORWARD"); break;
      case 2: Serial.println("REVERSE"); break;
      case 3: Serial.println("LEFT");    break;
      case 4: Serial.println("RIGHT");   break;
      default: Serial.println("UNKNOWN"); break;
    }
  }

  if (Run_Stop_Mode == 0) {
    digitalWrite(RIGHT_FORWARD,  LOW);
    digitalWrite(RIGHT_BACKWARD, LOW);
    digitalWrite(LEFT_FORWARD,   LOW);
    digitalWrite(LEFT_BACKWARD,  LOW);
    Run_Stop_Counter++;
    if (Run_Stop_Counter >= ((5 - Speed_index) * 100)) {
      Run_Stop_Counter = 0;
      Run_Stop_Mode = 1;
    }
  }
  else {
    Run_Stop_Counter++;
    if (Run_Stop_Counter >= (Speed_index * 100)) {
      Run_Stop_Counter = 0;
      Run_Stop_Mode = 0;
    }
    switch (Received_Command) {
      case 1:
        digitalWrite(LEFT_FORWARD,   HIGH);
        digitalWrite(RIGHT_FORWARD,  HIGH);
        break;
      case 2:
        digitalWrite(LEFT_BACKWARD,  HIGH);
        digitalWrite(RIGHT_BACKWARD, HIGH);
        break;
      case 3:
        digitalWrite(LEFT_FORWARD,   HIGH);
        digitalWrite(RIGHT_BACKWARD, HIGH);
        break;
      case 4:
        digitalWrite(RIGHT_FORWARD,  HIGH);
        digitalWrite(LEFT_BACKWARD,  HIGH);
        break;
      default:
        digitalWrite(RIGHT_FORWARD,  LOW);
        digitalWrite(RIGHT_BACKWARD, LOW);
        digitalWrite(LEFT_FORWARD,   LOW);
        digitalWrite(LEFT_BACKWARD,  LOW);
        break;
    }
  }
}
