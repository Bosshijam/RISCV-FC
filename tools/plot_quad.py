import sys, csv
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

path = sys.argv[1] if len(sys.argv) > 1 else "/tmp/quad.csv"
rows = list(csv.DictReader(open(path)))
c = lambda k: [float(r[k]) for r in rows]
t = c("time")

fig, ax = plt.subplots(4, 1, figsize=(12, 11), sharex=True)

ax[0].plot(t, c("roll_sp"), "r--", label="setpoint")
ax[0].plot(t, c("roll"), color="black", lw=1.6, label="true")
ax[0].plot(t, c("roll_est"), color="tab:blue", lw=0.8, label="estimate")
ax[0].set_ylabel("Roll (deg)")
ax[0].set_title("Three-axis quad: calibration -> filter -> angle loops -> rate PIDs -> mixer")

ax[1].plot(t, c("pitch_sp"), "r--", label="setpoint")
ax[1].plot(t, c("pitch"), color="black", lw=1.6, label="true")
ax[1].plot(t, c("pitch_est"), color="tab:blue", lw=0.8, label="estimate")
ax[1].set_ylabel("Pitch (deg)")

ax[2].plot(t, c("yawrate_sp"), "r--", label="setpoint")
ax[2].plot(t, c("yaw_rate"), color="tab:purple", label="true")
ax[2].set_ylabel("Yaw rate (deg/s)")

for k, name in zip(("m1", "m2", "m3", "m4"),
                   ("M1 rear-R CW", "M2 front-R CCW", "M3 rear-L CCW", "M4 front-L CW")):
    ax[3].plot(t, c(k), lw=0.9, label=name)
ax[3].set_ylabel("Motor command (0-1)")
ax[3].set_xlabel("Time (s)")
ax[3].legend(loc="upper left", ncol=4, fontsize=8)

for a in ax[:3]:
    a.legend(loc="upper right", fontsize=8)
for a in ax:
    a.axvspan(10.0, 10.2, color="orange", alpha=0.3)
    a.grid(alpha=0.3)
ax[0].text(10.25, ax[0].get_ylim()[1] * 0.75, "gust", color="darkorange")

plt.tight_layout()
plt.savefig("quad.png", dpi=100)
print("saved quad.png")
