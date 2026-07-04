#include <Wire.h>
#include <SoftwareSerial.h>

// ---------------- BLUETOOTH ----------------
SoftwareSerial BT(11, 10);

// ---------------- MPU6050 ----------------
const int MPU_ADDR = 0x68;
int16_t accX, accY, accZ;
int16_t gyroX, gyroY, gyroZ;

// ---------------- CALIBRATION OFFSETS ----------------
// These are filled during calibrate() at startup
float flex_rest[5]   = {0};   // raw value when hand is flat
float gyroOffset[3]  = {0};   // gyro baseline
float accOffset[3]   = {0};   // acc baseline

// ---------------- SCALING FACTORS ----------------
// Converts your raw glove values → dataset scale
// Dataset flex range is roughly -100 to +200
// Your raw flex range is roughly 140 to 260 (120 units)
// Scale = dataset_range / your_range
float flex_scale[5] = {-1.58, -1.04, -1.34, -1.95, -1.39};
// MPU6050 raw to physical units
// Raw gyro → deg/s:  divide by 131.0  (±250 dps range)
// Raw acc  → g:      divide by 16384.0 (±2g range)
// Dataset ACC looks like m/s² so multiply g by 9.81
const float GYRO_SCALE = 1.0 / 131.0;
const float ACC_SCALE  = 9.81 / 16384.0;

// ---------------- FLEX ----------------
int flexPins[5] = {A7, A6, A3, A2, A1};
float flexSmooth[5] = {0};
float alpha = 0.3;

// ---------------- TIMING ----------------
unsigned long lastSend = 0;
const int sendInterval = 50;

// ============================================================
void readMPU() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 14, true);

  accX  = Wire.read() << 8 | Wire.read();
  accY  = Wire.read() << 8 | Wire.read();
  accZ  = Wire.read() << 8 | Wire.read();
  Wire.read(); Wire.read(); // skip temp
  gyroX = Wire.read() << 8 | Wire.read();
  gyroY = Wire.read() << 8 | Wire.read();
  gyroZ = Wire.read() << 8 | Wire.read();
}

// ============================================================
void calibrate() {
  Serial.println(">>> Calibrating... Hold hand FLAT and STILL! <<<");
  BT.println("CALIBRATING");
  delay(2000);

  long gxSum=0, gySum=0, gzSum=0;
  long axSum=0, aySum=0, azSum=0;
  long flexSum[5] = {0};
  int samples = 100;

  for (int i = 0; i < samples; i++) {
    readMPU();
    gxSum += gyroX;
    gySum += gyroY;
    gzSum += gyroZ;
    axSum += accX;
    aySum += accY;
    azSum += accZ;

    for (int j = 0; j < 5; j++) {
      flexSum[j] += analogRead(flexPins[j]);
    }
    delay(10);
  }

  // Store rest positions
  gyroOffset[0] = gxSum / samples;
  gyroOffset[1] = gySum / samples;
  gyroOffset[2] = gzSum / samples;
  accOffset[0]  = axSum / samples;
  accOffset[1]  = aySum / samples;
  accOffset[2]  = azSum / samples;

  for (int j = 0; j < 5; j++) {
    flex_rest[j] = flexSum[j] / samples;
  }

  Serial.println(">>> Calibration Done! <<<");
  Serial.print("Flex Rest: ");
  for (int j = 0; j < 5; j++) {
    Serial.print(flex_rest[j]); Serial.print(" ");
  }
  Serial.println();
}

// ============================================================
void setup() {
  Serial.begin(9600);
  BT.begin(9600);
  Wire.begin();

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  delay(200);
  calibrate();

  Serial.println("System Ready");
  BT.println("START");
}

// ============================================================
void loop() {
  if (millis() - lastSend < sendInterval) return;
  lastSend = millis();

  // ---------- FLEX ----------
  for (int i = 0; i < 5; i++) {
    int raw = analogRead(flexPins[i]);
    flexSmooth[i] = alpha * raw + (1 - alpha) * flexSmooth[i];
  }

  // ---------- MPU ----------
  readMPU();

  // ---------- SCALE FLEX to match dataset range ----------
  // Step 1: subtract rest → 0 when straight
  // Step 2: multiply by -scale → bent gives positive value
  // Step 3: matches dataset range
  float calFlex[5];
  for (int i = 0; i < 5; i++) {

// CORRECT
calFlex[i] = (flexSmooth[i] - flex_rest[i]) * flex_scale[i];  }

  // ---------- SCALE IMU to match dataset units ----------
  // Gyro → degrees per second (dataset uses deg/s small values)
  float calGyrX = (gyroX - gyroOffset[0]) * GYRO_SCALE;
  float calGyrY = (gyroY - gyroOffset[1]) * GYRO_SCALE;
  float calGyrZ = (gyroZ - gyroOffset[2]) * GYRO_SCALE;

  // Acc → m/s² (dataset ACCx mean=9.14 which is ~gravity)
  float calAccX = (accX - accOffset[0]) * ACC_SCALE;
  float calAccY = (accY - accOffset[1]) * ACC_SCALE;
  float calAccZ = (accZ - accOffset[2]) * ACC_SCALE;

  // ---------- SEND PACKET ----------
  BT.print("<");
  BT.print(calFlex[0],2); BT.print(",");
  BT.print(calFlex[1],2); BT.print(",");
  BT.print(calFlex[2],2); BT.print(",");
  BT.print(calFlex[3],2); BT.print(",");
  BT.print(calFlex[4],2); BT.print(",");
  BT.print(calGyrX,4);    BT.print(",");
  BT.print(calGyrY,4);    BT.print(",");
  BT.print(calGyrZ,4);    BT.print(",");
  BT.print(calAccX,2);    BT.print(",");
  BT.print(calAccY,2);    BT.print(",");
  BT.print(calAccZ,2);
  BT.println(">");

  // Debug
  Serial.print("<");
  Serial.print(calFlex[0],2); Serial.print(",");
  Serial.print(calFlex[1],2); Serial.print(",");
  Serial.print(calFlex[2],2); Serial.print(",");
  Serial.print(calFlex[3],2); Serial.print(",");
  Serial.print(calFlex[4],2); Serial.print(",");
  Serial.print(calGyrX,4);    Serial.print(",");
  Serial.print(calGyrY,4);    Serial.print(",");
  Serial.print(calGyrZ,4);    Serial.print(",");
  Serial.print(calAccX,2);    Serial.print(",");
  Serial.print(calAccY,2);    Serial.print(",");
  Serial.print(calAccZ,2);
  Serial.println(">");
}
