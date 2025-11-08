Awesome 🚀 let’s sketch a **starter ML model** you can train from your HA/InfluxDB logs.

---

# 🌱 First AI Model for Smart Irrigation

We’ll keep it  **simple but practical** :

Use a **Random Forest Classifier** to predict **“Water Now” (1) vs “Don’t Water” (0)** based on sensor data.

---

## 🔹 1. Collect Data from InfluxDB

Your HA → InfluxDB already stores time-series.

Export to CSV (example query for soil + BME680):

```sql
SELECT soil_moisture_1, soil_moisture_2, soil_moisture_3, soil_moisture_4,
       water_level, temperature, humidity, pressure, air_quality,
       time
FROM homeassistant.autogen.sensor
WHERE time > now() - 30d
```

Then label each row:

* `1` if soil moisture < 30% **and** water level > min
* `0` otherwise.

(At first you can manually set rules → later replace with real irrigation logs for supervised learning).

---

## 🔹 2. Train Model in Python

```python
import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
import joblib

# Load dataset
df = pd.read_csv("irrigation_data.csv")

# Features (X) and target (y)
X = df[["soil_moisture_1", "soil_moisture_2", "soil_moisture_3", "soil_moisture_4",
        "water_level", "temperature", "humidity", "pressure", "air_quality"]]
y = df["needs_watering"]   # 1 = yes, 0 = no

# Split
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# Train
model = RandomForestClassifier(n_estimators=100, random_state=42)
model.fit(X_train, y_train)

# Evaluate
print("Accuracy:", model.score(X_test, y_test))

# Save model
joblib.dump(model, "irrigation_model.pkl")
```

---

## 🔹 3. Deploy in Home Assistant

* Copy `irrigation_model.pkl` into HA’s `config/python_scripts/`.
* Use a Python integration (`pyscript` or `python_script` + `joblib`).

Example HA `python_script/irrigation_ai.py`:

```python
import joblib
import numpy as np

model = joblib.load("/config/python_scripts/irrigation_model.pkl")

# Get states
soil = [float(hass.states.get(f"sensor.soil_moisture_{i}").state) for i in range(1,5)]
water_level = float(hass.states.get("sensor.water_level").state)
temp = float(hass.states.get("sensor.bme680_temperature").state)
hum = float(hass.states.get("sensor.bme680_humidity").state)
press = float(hass.states.get("sensor.bme680_pressure").state)
aq = float(hass.states.get("sensor.bme680_air_quality").state)

X = np.array([soil + [water_level, temp, hum, press, aq]])

prediction = model.predict(X)[0]

if prediction == 1:
    hass.services.call("switch", "turn_on", {"entity_id": "switch.irrigation_pump"})
else:
    hass.services.call("switch", "turn_off", {"entity_id": "switch.irrigation_pump"})
```

---

## 🔹 4. Automate the Execution

Trigger every 30 min, or whenever new soil readings arrive:

```yaml
automation:
  - alias: "AI Irrigation Decision"
    trigger:
      - platform: time_pattern
        minutes: "/30"
    action:
      - service: python_script.irrigation_ai
```

---

## 🔹 5. Iterate

* Start with rule-based labels → train → deploy.
* Later: use **real irrigation logs** (“when pump was on/off and results”) → retrain with real-world outcomes.
* Upgrade to **regression (predict soil moisture X hours ahead)** for predictive watering.

---

✅ With this, you have a  **working AI irrigation loop** :

* Matter sensors → HA → InfluxDB → Random Forest AI → Pump ON/OFF decisions.
