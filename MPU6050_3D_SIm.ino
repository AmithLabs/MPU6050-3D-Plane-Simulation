#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

MPU6050 mpu;

bool dmpReady = false;
uint16_t packetSize;
uint8_t fifoBuffer[64];

Quaternion q;
VectorFloat gravity;
float ypr[3];

// Offsets (replace with calibrated values if available)
float yaw_offset = 0;
float pitch_offset = 0;
float roll_offset = 0;

bool calibrated = false;
unsigned long startTime;

void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1);

  mpu.initialize();

  // Basic offsets (can improve later)
  mpu.setXAccelOffset(-2000);
  mpu.setYAccelOffset(-1000);
  mpu.setZAccelOffset(1000);
  mpu.setXGyroOffset(50);
  mpu.setYGyroOffset(20);
  mpu.setZGyroOffset(30);

  if (mpu.dmpInitialize() == 0) {
    mpu.setDMPEnabled(true);
    dmpReady = true;
    packetSize = mpu.dmpGetFIFOPacketSize();
  }

  startTime = millis();
}

void loop() {
  if (!dmpReady) return;

  if (mpu.getFIFOCount() >= packetSize) {

    mpu.getFIFOBytes(fifoBuffer, packetSize);

    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

    float yaw   = ypr[0] * 180 / PI;
    float pitch = ypr[1] * 180 / PI;
    float roll  = ypr[2] * 180 / PI;

    // 🟢 Auto zero calibration (first 2 seconds)
    if (!calibrated && millis() - startTime > 2000) {
      yaw_offset = yaw;
      pitch_offset = pitch;
      roll_offset = roll;
      calibrated = true;
    }

    // Apply offsets
    yaw   -= yaw_offset;
    pitch -= pitch_offset;
    roll  -= roll_offset;

    Serial.print(yaw);
    Serial.print(",");
    Serial.print(pitch);
    Serial.print(",");
    Serial.println(roll);
  }
}