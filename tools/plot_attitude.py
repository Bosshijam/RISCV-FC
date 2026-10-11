import sys, csv
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

path = sys.argv[1] if len(sys.argv) > 1 else "/tmp/att.csv"
t, tr, g, a, f = [], [], [], [], []
with open(path) as fh:
    for row in csv.DictReader(fh):
        t.append(float(row["time"]));      tr.append(float(row["true"]))
        g.append(float(row["gyro_only"])); a.append(float(row["accel_only"]))
        f.append(float(row["filtered"]))
err = [x - y for x, y in zip(f, tr)]

fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 8), sharex=True)
ax1.plot(t, a,  color="tab:gray", lw=0.5, alpha=0.6, label="accelerometer only")
ax1.plot(t, g,  color="tab:red",  lw=1.2, label="gyro only")
ax1.plot(t, tr, color="black",    lw=2.0, label="true angle")
ax1.plot(t, f,  color="tab:blue", lw=1.2, label="complementary filter")
for s, e in [(70, 72), (90, 92)]:
    for ax in (ax1, ax2):
        ax.axvspan(s, e, color="orange", alpha=0.2)
ax1.set_ylabel("Roll (degrees)")
ax1.set_ylim(-40, 70)
ax1.legend(loc="upper left")
ax1.grid(alpha=0.3)
ax1.set_title("Attitude estimate (shaded = fake acceleration bursts)")

ax2.plot(t, err, color="tab:blue", lw=0.8)
ax2.axhline(0, color="black", lw=0.8)
ax2.set_ylabel("Filter error (degrees)")
ax2.set_xlabel("Time (seconds)")
ax2.grid(alpha=0.3)

plt.tight_layout()
plt.savefig("attitude.png", dpi=100)
print("saved attitude.png")
