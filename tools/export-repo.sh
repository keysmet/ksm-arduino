#!/bin/bash

# Get the current date in YYYY-MM-DD format
current_date=$(date +%F)

# Get the latest commit hash
commit_hash=$(git rev-parse --short HEAD)

# Define the output file name
output_file="ksm-arduino_${current_date}_${commit_hash}.zip"

# Export the repository to a zip file
git -C .. archive -o "$output_file" HEAD
