
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Read GPS measurements
df = pd.read_csv("gps_measurements.csv")

# Keep only valid GPS measurements
df = df[df["GPS_state"] == "FRESH"].copy()

df["PC_timestamp"] = pd.to_datetime(df["PC_timestamp"])

# Convert coordinates to numeric
df["Latitude"] = pd.to_numeric(df["Latitude"])
df["Longitude"] = pd.to_numeric(df["Longitude"])
df["HDOP"] = pd.to_numeric(df["HDOP"])
df["Satellites"] = pd.to_numeric(df["Satellites"])

# Time relative to first measurement
df["Time_s"] = (
    df["PC_timestamp"] - df["PC_timestamp"].iloc[0]
).dt.total_seconds()

# Calculate average position
mean_lat = df["Latitude"].mean()
mean_lon = df["Longitude"].mean()

# Convert position differences approximately to meters
df["East_m"] = (
    (df["Longitude"] - mean_lon)
    * 111320 * np.cos(np.radians(mean_lat))
)

df["North_m"] = (
    (df["Latitude"] - mean_lat) * 110540
)

df["Distance_m"] = np.sqrt(
    df["East_m"]**2 + df["North_m"]**2
)

# Print statistics
print("\n===== GPS ANALYSIS =====")
print("Samples:", len(df))
print(f"Mean latitude: {mean_lat:.7f}")
print(f"Mean longitude: {mean_lon:.7f}")
print(f"Mean HDOP: {df['HDOP'].mean():.2f}")
print(f"Mean satellites: {df['Satellites'].mean():.1f}")
print(f"Maximum distance from mean: {df['Distance_m'].max():.2f} m")
print(f"Mean distance from mean: {df['Distance_m'].mean():.2f} m")
print(f"East standard deviation: {df['East_m'].std():.2f} m")
print(f"North standard deviation: {df['North_m'].std():.2f} m")

# Create graphs
fig, axes = plt.subplots(2, 2, figsize=(13, 9))

# 1. Latitude and longitude
axes[0, 0].plot(
    df["Longitude"], df["Latitude"], "b.-"
)
axes[0, 0].set_title("GPS Position")
axes[0, 0].set_xlabel("Longitude")
axes[0, 0].set_ylabel("Latitude")
axes[0, 0].grid(True)

# 2. HDOP over time
axes[0, 1].plot(
    df["Time_s"], df["HDOP"], "g.-"
)
axes[0, 1].set_title("HDOP vs Time")
axes[0, 1].set_xlabel("Time (seconds)")
axes[0, 1].set_ylabel("HDOP")
axes[0, 1].grid(True)

# 3. Satellites over time
axes[1, 0].plot(
    df["Time_s"], df["Satellites"], "r.-"
)
axes[1, 0].set_title("Satellites vs Time")
axes[1, 0].set_xlabel("Time (seconds)")
axes[1, 0].set_ylabel("Number of satellites")
axes[1, 0].grid(True)

# 4. Position dispersion around mean
axes[1, 1].scatter(
    df["East_m"], df["North_m"],
    c=df["Time_s"], cmap="viridis"
)
axes[1, 1].scatter(
    0, 0, color="red", marker="x",
    s=100, label="Mean position"
)
axes[1, 1].set_title("GPS Position Dispersion")
axes[1, 1].set_xlabel("East (meters)")
axes[1, 1].set_ylabel("North (meters)")
axes[1, 1].axis("equal")
axes[1, 1].grid(True)
axes[1, 1].legend()

plt.tight_layout()

plt.savefig("gps_analysis.png", dpi=300)
plt.show()

print("\nGraph saved as gps_analysis.png")
