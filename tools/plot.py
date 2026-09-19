import sys
import pandas as pd
import matplotlib.pyplot as plt

filename = sys.argv[1]
df = pd.read_csv(filename)

# optional second argument: the setpoint, drawn as a reference line
setpoint = float(sys.argv[2]) if len(sys.argv) > 2 else None

fig, ax = plt.subplots(2, 1, figsize=(10, 7), sharex=True)

# --- top: angle ---
ax[0].plot(df['time'], df['angle'], label='angle', color='#1f77b4')
if setpoint is not None:
    ax[0].axhline(setpoint, color='red', linestyle='--',
                  label=f'setpoint = {setpoint}')
ax[0].set_ylabel('Angle (degrees)')
ax[0].set_title(f'Step response — {filename}')
ax[0].grid(True, alpha=0.3)
ax[0].legend()

# --- bottom: rate ---
ax[1].plot(df['time'], df['rate'], label='rate', color='#ff7f0e')
ax[1].axhline(0, color='grey', linewidth=0.8)
ax[1].set_ylabel('Rate (degrees/second)')
ax[1].set_xlabel('Time (seconds)')
ax[1].grid(True, alpha=0.3)
ax[1].legend()

plt.tight_layout()
plt.savefig('plot.png', dpi=120)
print("saved plot.png")

# --- numbers, so you don't have to squint at the picture ---
print(f"  final angle : {df['angle'].iloc[-1]:.2f}")
print(f"  peak angle  : {df['angle'].max():.2f}")
if setpoint:
    overshoot = (df['angle'].max() - setpoint) / setpoint * 100
    print(f"  overshoot   : {overshoot:.1f}%")
