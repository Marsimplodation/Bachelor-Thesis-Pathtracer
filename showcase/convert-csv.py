
import json
import csv

def json_to_csv(json_file, csv_file):
    # Step 1: Read the JSON file
    with open(json_file, 'r') as f:
        data = json.load(f)

    # Extract field names and data
    fieldnames = ["name",1,2,3,4,5]
    rows = []
    keys = iter(data.keys())
    for idx, entry in enumerate(data.values()):
        counter = 1
        entry_val = next(keys)+ ", "
        for item in entry:
            for key in (item.keys()):
                fieldnames[counter] = key
                counter += 1
                entry_val = entry_val + str(item[key])
                if counter < 6:
                    entry_val += ", "
        rows.append(entry_val)
    
    print(', '.join(fieldnames))
    for row in rows:
        print(row)

# Example usage
json_file = 'render/result_parameters_lucy.json'
csv_file = 'data.csv'
json_to_csv(json_file, csv_file)

