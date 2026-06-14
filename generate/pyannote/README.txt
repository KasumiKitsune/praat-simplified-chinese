generate/pyannote/README.TXT
Anastasia Shchupak, 30 May 2026

This folder documents how the files
    external/whispercpp/model-ggml-segmentation.cpp and
    external/whispercpp/model-ggml-embedding.cpp
were generated. They contain the model weights for the segmentation and embedding models
used by pyannote/speaker-diarization-3.0 pipeline, which is implemented in Praat and used for speaker diarization.

The HuggingFace repository from which the original models were downloaded is gated,
so a HuggingFace account is necessary for downloading.

1. Files in this folder
=======================
README.txt
    This file.

convert-pyannote-segmentation-to-ggml.py
    Python script that converts the original pyannote/segmentation-3.0 PyTorch model to ggml binary format.
    Requires Python 3 and the pyannote.audio package.

convert-pyannote-embedding-to-ggml.py
    Python script that converts the original pyannote/wespeaker-voxceleb-resnet34-LM PyTorch model to ggml binary format.
    Requires Python 3 and the pyannote.audio package.

convert-pyannote-models-to-ggml.sh
    Shell script that converts the two models required by the pyannote/speaker-diarization-3.0 pipeline
    to ggml binary format, using the Python scripts convert-pyannote-segmentation-to-ggml.py
    and convert-pyannote-embedding-to-ggml.py.

2. How to obtain the model files
================================
2a. Segmentation model
----------------------
Download the original pyannote/segmentation-3.0 model from HuggingFace:
`https://huggingface.co/pyannote/segmentation-3.0/blob/main/pytorch_model.bin`
and rename it from pytorch_model.bin to pyannote-segmentation.bin.
Then run the following command to convert it to the ggml format:
```
    ./convert-pyannote-models-to-ggml.sh ./pyannote-segmentation.bin ./
```
This is the file from which the .cpp model file was generated.

2b. Embedding model
-------------------
Download the original pyannote/wespeaker-voxceleb-resnet34-LM model, which is a wrapper around WeSpeaker
wespeaker-voxceleb-resnet34-LM pretrained speaker embedding model:
`https://huggingface.co/pyannote/wespeaker-voxceleb-resnet34-LM/blob/main/pytorch_model.bin`.
and rename it from pytorch_model.bin to pyannote-embedding.bin.
Then run the following command to convert it to the ggml format:
```
    ./convert-pyannote-models-to-ggml.sh ./pyannote-embedding.bin ./
```
This is the file from which the .cpp model file was generated.
