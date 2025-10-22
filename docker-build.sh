#!/bin/bash

case "$1" in
  build)
    echo "Building dv-sissr Docker image..."
    docker build -t dv-sissr:latest .
    ;;
  test)
    echo "Testing Docker build..."
    docker run --rm dv-sissr:latest --help
    ;;
  help)
    echo "Usage: $0 {build|test|help}"
    echo "  build - Build the dv-sissr Docker image"
    echo "  test  - Test the Docker image by running --help"
    echo "  help  - Show this help message"
    ;;
  *)
    echo "Usage: $0 {build|test|help}"
    exit 1
    ;;
esac
