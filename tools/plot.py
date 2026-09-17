import sys
import pandas as pd
import matplotlib.pyplot as plt

# 1. get the filename from the command line
filename = sys.argv[1]

# 2. load the CSV
df = pd.read_csv(filename)

# 3. make two stacked plots
fig, ax = plt.subplots(2, 1)

ax[0].plot(df['time'],df['angle'])
ax[1].plot(df['time'],df['rate'])

# 5. YOU: plot time vs rate on ax[1]

# 6. YOU: label the axes
#    hint: ax[0].set_ylabel('...')

# 7. save it
plt.savefig('plot.png')
print("saved plot.png")
