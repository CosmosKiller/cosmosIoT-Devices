SELECT soil_moisture_1, soil_moisture_2, soil_moisture_3, soil_moisture_4,
       water_level, temperature, humidity, pressure, air_quality,
       time
FROM homeassistant.autogen.sensor
WHERE time > now() - 30d
