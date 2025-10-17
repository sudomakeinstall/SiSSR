#!/bin/bash

# Build the dv-sissr Docker image
docker build -t dv-sissr:latest .

# Test the build by running help
echo "Testing Docker build..."
docker run --rm dv-sissr:latest --help
