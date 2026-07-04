# Silent Voice — ASL Sensor Glove

A wearable glove that translates American Sign Language (ASL) hand gestures into text and speech in real time, using flex sensors, motion sensing, and a machine learning model. Built as a Final Year Project (FYP) to support communication for the deaf community.

**⚠️ TODO before this is final:** confirm the two flagged items below (marked with 🔶) using your project report, then remove this line.

## What It Does

- The glove detects hand and finger movements as the user signs.
- Sensor data is sent via Bluetooth to a mobile app.
- A trained machine learning model interprets the sensor data as ASL gestures.
- The app outputs the result as both **text and speech**.
- 🔶 Scope of recognition (fingerspelling only, or full word/phrase prediction) — needs confirmation from the project report before stating this as fact.

## Hardware

| Component | Role |
|---|---|
| 5x Flex Sensors | One per finger, measures finger bend |
| MPU6050 IMU | Wrist-mounted, measures hand orientation and movement |
| HC-05 Bluetooth Module | Sends sensor data to the mobile app |
| Arduino UNO | Reads sensors and controls data transmission |

Full wiring details and pin mapping are documented separately in the Arduino folder.

## Software / Tech Stack

- **Embedded:** Arduino (C/C++)
- **Machine Learning:** TensorFlow Lite (trained model included as `.h5` and `.tflite`)
- **Mobile App:** Android Studio
- 🔶 Model accuracy — filename suggests 75%, unconfirmed. Verify in the report and update this line with the real number.

## Repository Structure

```
Silent-Voice-FYP/
├── Android_App/     # Mobile application source
├── Arduino/         # Microcontroller code + wiring reference
├── Dataset/         # Training data (zipped)
├── ML_Model/        # Trained model files + training notebook
├── Report/          # Full project report (PDF)
└── Video/           # Demo recordings
```

## Demo

See the `Video/` folder for demo recordings of the glove in use.

## Author

Sadia Muzaffar
B.Sc. Computer Engineering Technology, International Islamic University Islamabad

---
*Final Year Project — in development*
