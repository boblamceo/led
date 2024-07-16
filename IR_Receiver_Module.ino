//www.elegoo.com
//2023.05.05

#include "IRremote.h"
#include "pitches.h"
#include "LedControl.h"
#include <Wire.h>
const int MPU_addr = 0x68;  // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;

int receiver = 11;  // Signal Pin of IR receiver to Arduino Digital Pin 11
LedControl lc = LedControl(12, 10, 9, 1);

unsigned long delaytime1 = 500;
unsigned long delaytime2 = 50;

/*-----( Declare objects )-----*/
IRrecv irrecv(receiver);  // create instance of 'irrecv'
//vairable uses to store the last decodedRawData
uint32_t last_decodedRawData = 0;
/*-----( Function )-----*/

bool controlled = false;

int currRow = 0;
int currCol = 0;

void move(char direction) {
  switch (direction) {
    case 'r':
      if (currCol < 7) {
        currCol += 1;
      }
      break;
    case 'l':
      if (currCol > 0) {
        currCol -= 1;
      }
      break;
    case 'u':
      if (currRow > 0) {
        currRow -= 1;
      }
      break;
    case 'd':
      if (currRow < 7) {
        currRow += 1;
      }
      break;
    default:
      Serial.println('move function error');
  }
}

void translateIR()  // takes action based on IR code received
{
  // Check if it is a repeat IR code
  if (irrecv.decodedIRData.flags) {
    //set the current decodedRawData to the last decodedRawData
    irrecv.decodedIRData.decodedRawData = last_decodedRawData;
    Serial.println("REPEAT!");
  } else {
    //output the IR code on the serial monitor
    Serial.print("IR code:0x");
    Serial.println(irrecv.decodedIRData.decodedRawData, HEX);
  }
  if (controlled) {
    //map the IR code to the remote key
    switch (irrecv.decodedIRData.decodedRawData) {
      case 0xBA45FF00:
        Serial.println("POWER");
        controlled = !controlled;
        break;
      case 0xB847FF00: Serial.println("FUNC/STOP"); break;
      case 0xB946FF00:
        Serial.println("VOL+");
        move('u');
        break;
      case 0xBB44FF00:
        Serial.println("FAST BACK");
        move('l');
        break;
      case 0xBF40FF00: Serial.println("PAUSE"); break;
      case 0xBC43FF00:
        Serial.println("FAST FORWARD");
        move('r');
        break;
      case 0xF807FF00: Serial.println("DOWN"); break;
      case 0xEA15FF00:
        Serial.println("VOL-");
        move('d');
        break;
      case 0xF609FF00: Serial.println("UP"); break;
      case 0xE916FF00: Serial.println("0"); break;
      case 0xE619FF00: Serial.println("EQ"); break;
      case 0xF20DFF00: Serial.println("ST/REPT"); break;
      case 0xF30CFF00: Serial.println("1"); break;
      case 0xE718FF00: Serial.println("2"); break;
      case 0xA15EFF00: Serial.println("3"); break;
      case 0xF708FF00: Serial.println("4"); break;
      case 0xE31CFF00: Serial.println("5"); break;
      case 0xA55AFF00: Serial.println("6"); break;
      case 0xBD42FF00: Serial.println("7"); break;
      case 0xAD52FF00: Serial.println("8"); break;
      case 0xB54AFF00: Serial.println("9"); break;
      default: Serial.println(" other button   ");
    }  // End Case
  } else {
    switch (irrecv.decodedIRData.decodedRawData) {
      case 0xBA45FF00:
        Serial.println("POWER");
        controlled = !controlled;
        break;
      default: Serial.println(" other button   ");
    }
  }
  //store the last decodedRawData
  last_decodedRawData = irrecv.decodedIRData.decodedRawData;
  delay(500);  // Do not get immediate repeat
}  //END translateIR

void setup() /*----( SETUP: RUNS ONCE )----*/
{
  Serial.begin(9600);
  Serial.println("IR Receiver Button Decode");
  irrecv.enableIRIn();  // Start the receiver
  lc.shutdown(0, false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0, 8);
  /* and clear the display */
  lc.clearDisplay(0);
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
} /*--(end setup )---*/

void loop() /*----( LOOP: RUNS CONSTANTLY )----*/
{

  lc.clearDisplay(0);
  lc.setLed(0, currRow, currCol, true);
  if (irrecv.decode())  // have we received an IR signal?
  {
    translateIR();
    irrecv.resume();  // receive the next value
  }
  if (!controlled) {
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr, 14, true);  // request a total of 14 registers
    AcX = Wire.read() << 8 | Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
    AcY = Wire.read() << 8 | Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ = Wire.read() << 8 | Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    if (AcX < 1000 && AcY < -8000) {
      currRow = 0;
      currCol = 3;
    } else if (AcX < 1000 && AcY > 8000) {
      currRow = 7;
      currCol = 3;
    }else if (AcX > 8000 && AcY < 1000) {
      currRow = 3;
      currCol = 0;
    } else if (AcX < -8000 && AcY < 1000) {
      currRow = 3;
      currCol = 7;
    }else{
      currRow = 3;
      currCol = 3;
    }
    Serial.println(AcX);
    Serial.println(AcY);
    delay(333);
  }
} /* --(end main loop )-- */
