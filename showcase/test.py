import subprocess
import json
import re
import os

# Function to parse output and convert to JSON
def parse_output_to_json(output):
    bvh_data = []
    all_data = []
    pattern = re.compile(r'(\w+ \w+): (\d{2}:\d{2}:\d{2}|\d+)')

    lines = output.splitlines()
    for i, line in enumerate(lines):
        matches = pattern.findall(line)
        if matches:
            entry = {match[0]: match[1] for match in matches}
            if i < 4:  # First four lines go into 'bvh'
                bvh_data.append(entry)
            else:  # Remaining lines go into 'all'
                all_data.append(entry)

    return json.dumps({"bvh": bvh_data, "2plane": all_data}, indent=2)


scenes = ["cornel", "cornel_Glass", "s"]
# Execute the command and capture output
out = {}
for scene in scenes:
    command = f"../build/pathtracer scenes/{scene}.scene test | tail -n 8"

    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    json_output = parse_output_to_json(result.stdout)
    out[scene] = json.loads(json_output)
    
    dir_path = os.path.join("render", scene)
    subprocess.run(f"mkdir -p {dir_path}", shell=True)
    
    source_pattern = os.path.join("render", "*.png")
    subprocess.run(f"mv {source_pattern} {dir_path}", shell=True)

# Parse the output to JSON

# Print the JSON output
print(json.dumps(out))
