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
