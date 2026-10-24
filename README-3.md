# Microbial Fuel Cell Voltage Monitor

An Arduino experiment for observing small amounts of electricity produced by microbial fuel cells (MFCs), soil batteries and other low-voltage sources. The Arduino measures a voltage on analogue input **A0**, plots its recent changes on a **128 × 64 OLED screen**, and displays the current reading in **millivolts (mV)**.

The aim is to make small, changing electrical outputs visible rather than treating electricity as an invisible or instantly available resource.

## What it does

- **Measures voltage:** reads the signal connected to Arduino pin **A0**.
- **Shows a live graph:** draws the recent voltage history on the OLED, automatically adjusting the graph's vertical scale.
- **Displays a number:** shows the latest smoothed voltage reading in **mV**.
- **Reduces noise:** averages eight analogue readings and smooths the result.
- **Handles near-zero input:** readings below **50 mV** are treated as zero. This is a threshold, not a reliable electronic connection detector.
- **Flashes two LEDs:** pin **13** changes state every **500 ms**; pin **6** changes state at random intervals of **150–1,200 ms**. The LEDs do **not** currently respond to the measured voltage.
- **Outputs serial data:** sends a voltage reading to the Arduino Serial Monitor at **9600 baud**.

The Arduino takes a new measurement approximately every **160 ms**. Its 124-point graph covers roughly the previous **20 seconds** of measurements.

## Files

- `moss-code2.ino` — the Arduino sketch.

## Hardware

- A 5 V Arduino board with a 10-bit analogue-to-digital converter (for example, a conventional Arduino Uno or Nano), or an appropriately adapted board.
- A 128 × 64 I²C OLED display, configured for **SH1106** by default.
- A microbial fuel cell, soil battery or another voltage source within the Arduino's safe analogue-input range.
- A **1 MΩ resistor** between A0 and GND, to pull a disconnected input towards zero.
- Optional external LED on digital pin **6**, with a suitable current-limiting resistor.
- USB cable for uploading code and powering the Arduino.

Many Arduino Uno/Nano boards have a built-in LED on pin **13**. The OLED and Arduino need their own suitable power supply; **this sketch measures the fuel-cell voltage, but does not power the Arduino from the fuel cell**.

## Connections

| Component | Arduino connection |
| --- | --- |
| Fuel cell positive (+) | A0 |
| Fuel cell negative (−) | GND |
| 1 MΩ pull-down resistor | Between A0 and GND |
| OLED SDA | Board's I²C SDA (A4 on a conventional Uno) |
| OLED SCL | Board's I²C SCL (A5 on a conventional Uno) |
| OLED VCC and GND | Appropriate supply for the **specific display module**, and GND |
| Optional second LED | Digital pin 6 through a current-limiting resistor, then to GND |

**Important:** Check the voltage rating of your particular Arduino and OLED before connecting anything. Never apply a negative voltage or a voltage above the board's permitted analogue-input range to A0. The 1 MΩ resistor also loads the measured source slightly, which may matter when measuring particularly weak microbial cells.

## Getting started

1. Open `moss-code2.ino` in the Arduino IDE.
2. Install the **U8g2** library via the Arduino IDE Library Manager.
3. Connect the OLED, voltage source and optional LED as described above.
4. Select the correct Arduino board and port, then upload the sketch.
5. Watch the OLED for the graph and voltage reading. To see the numbers on your computer, open **Serial Monitor** at **9600 baud**.

The sketch is configured for an SH1106 OLED. If yours uses an SSD1306 controller, comment out the SH1106 display constructor near the top of the sketch and uncomment the SSD1306 alternative. Ensure your board has sufficient memory for the selected display buffer.

## Settings you can change

Near the top of the sketch:

| Setting | Default | Effect |
| --- | --- | --- |
| `ADC_REFERENCE_MV` | `5000` | Assumed analogue reference, in mV |
| `ADC_MAX_VALUE` | `1023` | Maximum 10-bit ADC reading |
| `SENSOR_DISCONNECTED_BELOW_MV` | `50` | Below this voltage, display zero |
| `ADC_AVERAGE_SAMPLES` | `8` | Number of readings averaged each time |
| `VOLTAGE_SMOOTHING_ALPHA` | `0.50` | Higher = more responsive; lower = smoother |
| `SAMPLE_INTERVAL_MS` | `160` | Time between screen/serial updates |
| `LED1_TOGGLE_MS` | `500` | Regular LED toggle interval |

The random LED interval is set in the code with `random(150, 1200)`.

**Calibration note:** The code assumes a **0–5 V, 10-bit ADC**. On boards with a different reference voltage or ADC resolution (such as many ESP32 boards), readings will be incorrect unless the sketch and input wiring are adapted. The code has not been calibrated against a reference meter.

## Creative and research context

This is a small experimental instrument for exploring the electricity produced by living and environmental processes. Displaying changing millivolt readings provides a way of attending to slow, variable and sometimes barely perceptible electrical activity. It may be used as part of a research, workshop or installation process.

## AI-generated code credit

**Arduino code generated with AI assistance (OpenAI ChatGPT), through iterative prompting, testing and adjustment.** The project concept, experimental context and creative direction are supplied by the project author.

This is experimental code provided as a starting point for further adaptation, not a calibrated scientific measuring instrument.
