# M5Stick Object Detection over Bluetooth

Press a button on the M5Stick to capture a photo from your computer's camera, run it through Google Gemini vision, and display the result on the M5Stick screen: object, brand, country of origin, condition, price, and where to buy it.

Built as Lab 5 coursework (Python + embedded systems).

## How it works

1. `M5Labeller.py` scans for nearby Bluetooth LE devices and connects to the M5Stick.
2. Press **BtnA** on the M5Stick to capture a photo, **BtnB** to quit.
3. The photo is sent to the Gemini API, which returns short labels (object, brand, country, condition, price, store).
4. Labels are written back over BLE and shown on the M5Stick display.
5. Each button press also reports the M5Stick battery voltage to the Python console.

If nothing is detected, the M5Stick displays `GET OUT >:(`.

## Hardware

- M5StickC Plus (ESP32)
- Flash `Arduino_control_api_button/M5Label/M5Label.ino` with the Arduino IDE (requires the M5Unified and M5GFX libraries)

## Setup

```bash
pip install -r requirements.txt
export GEMINI_API_KEY="your-key-here"   # macOS / Linux
python M5Labeller.py
```

## Demo

https://youtube.com/shorts/ODMpL6lVuFM

## Tech

Python, Bleak (Bluetooth LE), OpenCV, Google Gemini API, Arduino (M5StickC Plus)
