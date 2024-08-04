import subprocess
import json
import re
import os
import sys
import matplotlib.pyplot as plt
import numpy as np

input_text = sys.stdin.read().strip()
print(f"Received input: {input_text}")
out = json.loads(input_text)
memory = {}
intersections = {}
time = {}
for key in out.keys():
    data = out[key]
    for key2 in data.keys():
        memory[key + key2] = int(data[key2][4]['Memory Consumption'])
        intersections[key + key2] = int(data[key2][3]['Total Intersections'])
        time[key + key2] = int(data[key2][0]['Rendering Time'])

# Extract keys and values from the dictionary
keys = list(memory.keys())
values = list(memory.values())

# Create a bar chart
plt.figure(figsize=(8, 6))
plt.bar(keys, values, color='skyblue')

# Add title and labels
plt.title('Bar Chart of Memory Dictionary')
plt.xlabel('Keys')
plt.ylabel('Values')
plt.ylim(0, int(max(values)) + 1)  # +1 to give some space above the highest bar

# Extract keys and values from the dictionary
keys = list(intersections.keys())
values = list(intersections.values())

# Create a bar chart
plt.figure(figsize=(8, 6))
plt.bar(keys, values, color='skyblue')

# Add title and labels
plt.title('Bar Chart of Intersections Dictionary')
plt.xlabel('Keys')
plt.ylabel('Values')
plt.ylim(0, int(max(values)) + 1)  # +1 to give some space above the highest bar

# Extract keys and values from the dictionary
keys = list(time.keys())
values = list(time.values())

# Create a bar chart
plt.figure(figsize=(8, 6))
plt.bar(keys, values, color='skyblue')

# Add title and labels
plt.title('Bar Chart of Time Dictionary')
plt.xlabel('Keys')
plt.ylabel('Values')
plt.ylim(0, int(max(values)) + 1)  # +1 to give some space above the highest bar

# Display the plot
plt.show()
