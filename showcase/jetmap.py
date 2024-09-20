import cv2
import numpy as np
import matplotlib.pyplot as plt
import os
import glob


# Step 1: Load the grayscale image
# Replace 'grayscale_image.png' with your actual grayscale image file
baseDir = "./"

def convertToJet(gray_image, file_path):
    # Step 3: Apply the 'jet' colormap
    jet_image = cv2.applyColorMap(gray_image, cv2.COLORMAP_JET)

    # Step 7: Save the jet-mapped image without the colorbar
    plt.figure(figsize=(8, 6))
    plt.imshow(cv2.cvtColor(jet_image, cv2.COLOR_BGR2RGB))
    plt.axis('off')  # Hide axis
    plt.savefig(file_path, bbox_inches='tight', pad_inches=0)
    plt.close()

    # Step 8: Create and save the colorbar as a separate image
    fig, ax = plt.subplots(figsize=(2, 6))  # Narrow figure for just the colorbar
    sm = plt.cm.ScalarMappable(cmap='jet', norm=plt.Normalize(vmin=0, vmax=cap))
    cbar = plt.colorbar(sm, ax=ax)
    cbar.set_label('Intersects')

    # Remove the plot axis for the colorbar
    ax.remove()

    plt.savefig('colorbar.png', bbox_inches='tight', pad_inches=0)
    plt.close()


# Use glob to find all .png files that contain 'heatmap' in the filename
# '**' ensures that subdirectories are searched as well
search_pattern = os.path.join(baseDir, '**', '**.png')

# Use glob with recursive=True to search through subdirectories
files = glob.glob(search_pattern, recursive=True)

# Loop through and process each file
for file_path in files:
    image = cv2.imread(file_path)
    cap = 1000
    # Step 2: Convert the image to grayscale, but give red regions a high intensity
    # Create a mask where red regions are detected
    lower_red = np.array([0, 0, 100])
    upper_red = np.array([0, 0, 255])
    red_mask = cv2.inRange(image, lower_red, upper_red)

    # Convert the non-red part of the image to grayscale
    gray_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

    # Assign maximum intensity to red areas in the grayscale image
    gray_image[red_mask > 0] = 255
    convertToJet(gray_image, file_path)
