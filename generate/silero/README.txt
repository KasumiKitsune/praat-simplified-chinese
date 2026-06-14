generate/silero/README.TXT
Anastasia Shchupak, 16 March, 30 May 2026

This folder documents how the file external/whispercpp/ggml-silero-vad-model-data.h was generated. This C header
contains the embedded Silero VAD (Voice Activity Detection) v6.2.0 model weights used by Praat's speech recognition.

Files in this folder
--------------------
README.txt
    This file.

ggml-silero-v6.2.0.bin
    The Silero VAD model (version 6.2.0) in the ggml format. This is the file from which the C header was generated.

convert-silero-vad-to-ggml.py
    Slightly adapted (to match the output file name) Python script from whisper.cpp that converts
    the original Silero VAD PyTorch model to the ggml format. Requires Python 3 with the silero-vad and numpy packages.

convert-silero-vad-to-ggml.sh
    Shell script that installs an original Silero VAD v6.2.0 model and converts it to the ggml format,
    using the python script convert-silero-vad-to-ggml.py. Included here to document how to obtain
    ggml-silero-v6.2.0.bin from the original Silero VAD model.

How to obtain ggml-silero-vad-model-data.h from the included ggml-silero-v6.2.0.bin file
----------------------------------------------------------------------------------------
Run the following command which produces ggml-silero-vad-model-data.h in the current folder,
which then needs to be copied to the external/whispercpp folder:
```
    xxd -i -n ggml_silero_bin ggml-silero-v6.2.0.bin > ggml-silero-vad-model-data.h
```

How to obtain the ggml-silero-v6.2.0.bin file
---------------------------------------------
You can download it from here: `https://huggingface.co/ggml-org/whisper-vad`.
If you need to recreate ggml-silero-v6.2.0.bin from scratch, then use the following command:
```
    ./convert-silero-vad-to-ggml.sh v6.2.0
```