#!/bin/bash

# generate/whispercpp/convert-pyannote-models-to-ggml.sh
# Anastasia Shchupak, 21 March 2026, 30 May 2026
#
# Shell script that converts one of the two models required by the
# pyannote/speaker-diarization-3.0 pipeline to ggml binary format, using the
# Python scripts convert-pyannote-segmentation-to-ggml.py and
# convert-pyannote-embedding-to-ggml.py.
#
# Which converter runs is decided from the model path:
#   a path containing "pyannote-segmentation.bin" -> segmentation model (-> ggml-segmentation.bin)
#   a path containing "pyannote-embedding.bin"    -> embedding model    (-> ggml-embedding.bin)
#
# The two models (both are downloaded from HuggingFace repos)
#   Segmentation:  pyannote/segmentation-3.0                  (pyannote-segmentation.bin)
#   Embedding:     pyannote/wespeaker-voxceleb-resnet34-LM    (pyannote-embedding.bin)
#
# Usage: convert-pyannote-models-to-ggml.sh <model-path> <output-dir> [--inspect]
# Example: convert-pyannote-models-to-ggml.sh ./models/pyannote-segmentation.bin ./models

set -e  # stop if any command fails

# We need this version as the newer Python (3.14) broke the import chain below.
PYTHON_BIN="python3.12"

usage() {
    printf "Usage: %s <model-path> <output-dir>\n" "$0"
    printf "or %s <model-path> --inspect\n" "$0"
    printf "Example: %s ./models/pyannote-segmentation.bin ./models\n" "$0"
    printf "\nThe converter is chosen from the model path:\n"
    printf "  path containing 'pyannote-segmentation.bin' -> segmentation model\n"
    printf "  path containing 'pyannote-embedding.bin' -> embedding model\n\n"
}

# check that the required arguments are provided
if [ -z "$1" ] || [ -z "$2" ]; then
    usage
    exit 1
fi

MODEL_PT="$1"
OUT_DIR="$2"

DO_INSPECT=0
if [ "$3" = "--inspect" ]; then
    DO_INSPECT=1
fi

# validate the model path and pick its converter
case "$MODEL_PT" in
    *pyannote-segmentation.bin*) CONVERTER="convert-pyannote-segmentation-to-ggml.py" ;;
    *pyannote-embedding.bin*)    CONVERTER="convert-pyannote-embedding-to-ggml.py" ;;
    *)  echo "ERROR: model name must be 'pyannote-segmentation.bin' or 'pyannote-embedding.bin'" >&2; usage 1 ;;
esac

echo "Creating virtual environment..."
"$PYTHON_BIN" -m venv venv

echo "Activating virtual environment..."
source venv/bin/activate

echo "Installing pyannote.audio and dependencies..."
#   The embedding path imports pyannote -> speaker_verification.py -> speechbrain, and speechbrain
#   calls torchaudio.list_audio_backends() at import time. Recent torchaudio removed that function,
#   so the import crashes; hence the earlier versions are specified below. Also, everything is
#   installed in one pip command for pip to complain if pyannote needs another version of torchaudio.
pip install \
    "torchaudio==2.8.0" \
    "torchcodec==0.7.0" \
    speechbrain \
    pyannote.audio

if [ "$DO_INSPECT" -eq 1 ]; then
    echo "Inspecting model: ${MODEL_PT}"
    python3 "$CONVERTER" --inspect "$MODEL_PT"
else
    mkdir -p "$OUT_DIR"
    echo "Converting model: ${MODEL_PT} -> ${OUT_DIR}/"
    python3 "$CONVERTER" "$MODEL_PT" "$OUT_DIR"
fi

echo "Deactivating virtual environment..."
deactivate

echo "Done."
