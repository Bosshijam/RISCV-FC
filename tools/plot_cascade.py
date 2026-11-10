import sys, csv
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

path = sys.argv[1] if len(sys.argv) > 1 else "/tmp/cascade.csv"
rows = list(csv.DictReader(open(path)))
col = lambda k: [float(r[k]) for r in rows]
t = col("time")

fig, ax = plt.subplots(3, 1, figsize=(11, 9), sharex=True)

ax[0].plot(t, col("angle_sp"), "r--", label="angle setpoint")
ax[0].plot(t, col("angle"), color="tab:blue", label="angle")
ax[0].set_ylabel("Angle (deg)")
ax[0].set_title("Cascade: angle loop -> rate PID")

ax[1].plot(t, col("rate_sp"), "r--", label="rate setpoint (from angle loop)")
ax[1].plot(t, col("rate"), color="tab:orange", label="rate")
ax[1].set_ylabel("Rate (deg/s)")

ax[2].plot(t, col("torque"), color="tab:green", label="torque command")
ax[2].set_ylabel("Torque")
ax[2].set_xlabel("Time (s)")

for a in ax:
    a.axvspan(3.5, t[-1], color="grey", alpha=0.08)
    a.grid(alpha=0.3)
    a.legend(loc="upper right")
ax[0].text(3.55, ax[0].get_ylim()[1] * 0.85, "disturbance on", color="grey")

plt.tight_layout()
plt.savefig("cascade.png", dpi=100)
print("saved cascade.png")
