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
