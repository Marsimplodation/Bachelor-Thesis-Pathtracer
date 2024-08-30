import subprocess
import json
import re
import os
import matplotlib.pyplot as plt
import numpy as np
# Function to parse output and convert to JSON


def parse_output_to_json_compare(output, jsonArr, name):
    all_data = []
    bvh_data = []
    pattern = re.compile(r'(\w+ \w+): (\d{2}:\d{2}:\d{2}|\d+)')

    lines = output.splitlines()
    for i, line in enumerate(lines):
        matches = pattern.findall(line)
        if matches:
            entry = {match[0]: match[1] for match in matches}
            if i < 5:  # First four lines go into 'bvh'
                bvh_data.append(entry)
            else:  # Remaining lines go into 'all'
                all_data.append(entry)

    jsonArr[name] = json.dumps({"bvh": bvh_data, "2plane": all_data}, indent=2)


def parse_output_to_json_2plane(output, jsonArr, name):
    all_data = []
    pattern = re.compile(r'(\w+ \w+): (\d{2}:\d{2}:\d{2}|\d+)')

    lines = output.splitlines()
    for i, line in enumerate(lines):
        matches = pattern.findall(line)
        if matches:
            entry = {match[0]: match[1] for match in matches}
            all_data.append(entry)

    jsonArr[name] = all_data


scenes = ["lucy"]


def comparisonTest():
    samples = [5, 10]
    # Execute the command and capture output
    out = {}
    for scene in scenes:
        command = f"../build/pathtracer scenes/{scene}.scene -S 10 -gs 8 -os 8 -m 50 --testing | tail -n 10"

        result = subprocess.run(
            command, shell=True, capture_output=True, text=True)
        entry = str(scene)
        parse_output_to_json_compare(result.stdout, out, entry)

    # Parse the output to JSON
    # Print the JSON output
    print(json.dumps(out))


def comparisonParam():
    gridSizes = [4,5,6,7,8,9,10]
    objectSizes = [4,5,6,7,8,9,10]
    maxTris = [30, 40, 50, 100]
    # Execute the command and capture output
    out = {}
    for gridSize in gridSizes:
        for max in maxTris:
            for scene in scenes:
                command = f"../build/pathtracer scenes/{scene}.scene -S 10 -gs {gridSize} -os {gridSize} -m {max} --testing --2plane_only | tail -n 5"

                result = subprocess.run(
                    command, shell=True, capture_output=True, text=True)
                entry = str(scene+"-"+str(gridSize)+"-"+str(max))
                parse_output_to_json_2plane(result.stdout, out, entry)

    # Parse the output to JSON
    # Print the JSON output
    print(json.dumps(out))


comparisonTest()
