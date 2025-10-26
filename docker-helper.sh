#!/bin/bash

case "$1" in
  build)
    echo "Building dv-sissr Docker image..."
    docker build -t dv-sissr:latest .
    ;;
  usage)
    echo "Testing Docker build..."
    docker run --rm dv-sissr:latest --help
    ;;
  run)
    if [ -z "$2" ] || [ -z "$3" ] || [ -z "$4" ]; then
      echo "Error: <candidate-dir>, <initial-model>, and <output-dir> are required."
      echo "Usage: $0 run <candidate-dir> <initial-model> <output_dir> [additional args...]"
      exit 1
    fi
    CANDIDATE_DIR="$2"
    INITIAL_MODEL="$3"
    OUTPUT_DIR="$4"
    shift 4
    INITIAL_MODEL_DIR="$(dirname "$INITIAL_MODEL")"
    INITIAL_MODEL_FILE="$(basename "$INITIAL_MODEL")"
    mkdir -p "$OUTPUT_DIR"
    docker run --rm \
      --userns=keep-id \
      --user "$(id -u):$(id -g)" \
      -v "$CANDIDATE_DIR:/candidate:ro,z" \
      -v "$INITIAL_MODEL_DIR:/initial-model-dir:ro,z" \
      -v "$OUTPUT_DIR:/output:z" \
      dv-sissr:latest \
      --candidate-dir /candidate \
      --initial-model "/initial-model-dir/$INITIAL_MODEL_FILE" \
      --output-dir /output \
      "$@"
    ;;
  help)
    echo "Usage: $0 {build|test|run|help}"
    echo "  build - Build the dv-sissr Docker image"
    echo "  usage - Print the output of 'dv-sissr --help'"
    echo "  run   - Run the Docker image with mounted directories"
    echo "          Usage: $0 run <input_dir> <output_dir> [additional args...]"
    echo "  help  - Show this help message"
    ;;
  *)
    echo "Usage: $0 {build|test|run|help}"
    exit 1
    ;;
esac
