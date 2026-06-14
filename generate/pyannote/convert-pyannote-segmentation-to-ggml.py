#!/usr/bin/env python3

# generate/whispercpp/convert-pyannote-embedding-to-ggml.py
# Anastasia Shchupak, 21 March 2026, 30 May 2026

# Convert pyannote segmentation-3.0 (PyanNet) model from PyTorch to ggml format
#
# Prerequisites:
#   pip install pyannote.audio
#
# Usage:
#   # Inspect the model (see all tensor names and shapes):
#   python convert-pyannote-segmentation-to-ggml.py --inspect path/to/pytorch_model.bin
#
#   # Convert to GGML (default: f16 for large tensors):
#   python convert-pyannote-segmentation-to-ggml.py path/to/pytorch_model.bin ./output-dir
#
#   # Convert to GGML (all f32):
#   python convert-pyannote-segmentation-to-ggml.py path/to/pytorch_model.bin ./output-dir use-f32
#
# You can download pytorch_model.bin from:
#   https://huggingface.co/pyannote/segmentation-3.0
#   (requires accepting user conditions + HF token)
#
# This script loads the model via pyannote.audio, extracts the state_dict,
# and writes it in ggml format. The output is a single binary file containing:
#
#  - hparams
#  - model variables
#
# For each variable, write the following:
#
#  - Number of dimensions (int)
#  - Name length (int)
#  - Dimensions (int[n_dims])  -- in reverse order per ggml convention
#  - Name (char[name_length])
#  - Data (float[n_dims])
#

import io
import os
import sys
import struct
import torch
import numpy as np
from pathlib import Path

# ============================================================================
# Inspect mode: load and print all tensors
# ============================================================================

def inspect_model(fname_inp):
    """Load the PyTorch model via pyannote and print its full tensor inventory."""

    from pyannote.audio import Model

    print(f"Loading: {fname_inp}")
    print(f"File size: {os.path.getsize(fname_inp)} bytes ({os.path.getsize(fname_inp) / 1024 / 1024:.2f} MB)")
    print()

    model = Model.from_pretrained(fname_inp)
    state_dict = model.state_dict()

    print(f"Model class: {type(model).__name__}")
    print(f"Number of tensors: {len(state_dict)}")
    print()
    print(f"  {'Name':55s} {'Shape':25s} {'Dtype':10s} {'Elements':>12s} {'Size (KB)':>10s}")
    print("  " + "-" * 118)

    total_params = 0
    total_bytes = 0
    for name in sorted(state_dict.keys()):
        tensor = state_dict[name]
        shape_str = str(list(tensor.shape))
        n_elements = tensor.numel()
        n_bytes = n_elements * tensor.element_size()
        total_params += n_elements
        total_bytes += n_bytes
        print(f"  {name:55s} {shape_str:25s} {str(tensor.dtype):10s} {n_elements:12d} {n_bytes/1024:10.1f}")

    print("  " + "-" * 118)
    print(f"  Total: {total_params:,} parameters, {total_bytes / 1024 / 1024:.2f} MB")
    print()

    # Print first few values of each tensor for later verification against C++ loader
    print("Sample values (first 5 elements of each tensor):")
    for name in sorted(state_dict.keys()):
        tensor = state_dict[name]
        flat = tensor.detach().cpu().flatten()
        vals = flat[:min(5, len(flat))].numpy()
        vals_str = ", ".join(f"{v:.6f}" for v in vals)
        print(f"  {name:55s} [{vals_str}]")


# ============================================================================
# Convert mode
# ============================================================================

def convert_model(fname_inp, dir_out):
    """Convert the segmentation model to GGML format."""

    from pyannote.audio import Model

    print(f"Loading: {fname_inp}")

    model = Model.from_pretrained(fname_inp)
    list_vars = model.state_dict()

    print(f"Model class: {type(model).__name__}")
    print(f"Tensors: {len(list_vars)}")

    # output file
    dir_out = Path(dir_out)
    dir_out.mkdir(parents=True, exist_ok=True)
    fname_out = dir_out / "ggml-segmentation.bin"

    fout = fname_out.open("wb")

    # =========================================================================
    # Write magic: "ggml"
    # =========================================================================
    fout.write(struct.pack("i", 0x67676d6c))

    # =========================================================================
    # Write hparams
    # =========================================================================
    # Modeled after whisper converter lines 268-279.
    # The C++ loader will read these in exactly this order.
    #
    # IMPORTANT: We derive values from the actual tensor shapes rather than
    # hardcoding, because the pretrained model may differ from source code
    # defaults. For example, segmentation-3.0 has 4 LSTM layers (not 2),
    # and the SincNet filterbank has 40 learned parameter pairs (not 80).

    # --- Derive architecture from tensor shapes ---

    # SincNet: n_filters_0 = output channels of first conv block
    # ParamSincFB stores low_hz_ [n_params, 1] — the actual output is 2*n_params
    sincnet_n_params   = list_vars["sincnet.conv1d.0.filterbank.low_hz_"].shape[0]  # 40
    sincnet_filters_0  = sincnet_n_params * 2  # 80 (ParamSincFB doubles the filters)
    sincnet_kernel_0   = list_vars["sincnet.conv1d.0.filterbank.window_"].shape[0] * 2 + 1  # 125*2+1=251
    sincnet_filters_1  = list_vars["sincnet.conv1d.1.weight"].shape[0]  # 60
    sincnet_kernel_1   = list_vars["sincnet.conv1d.1.weight"].shape[2]  # 5
    sincnet_filters_2  = list_vars["sincnet.conv1d.2.weight"].shape[0]  # 60
    sincnet_kernel_2   = list_vars["sincnet.conv1d.2.weight"].shape[2]  # 5

    # LSTM: count layers by looking for weight_ih_l{N} keys
    lstm_layers = 0
    while f"lstm.weight_ih_l{lstm_layers}" in list_vars:
        lstm_layers += 1
    lstm_input_size  = list_vars["lstm.weight_ih_l0"].shape[1]          # 60
    lstm_hidden_size = list_vars["lstm.weight_hh_l0"].shape[1]          # 128
    lstm_bidir = 1 if "lstm.weight_ih_l0_reverse" in list_vars else 0   # 1

    # Linear layers: count by looking for linear.{N}.weight keys
    linear_layers = 0
    while f"linear.{linear_layers}.weight" in list_vars:
        linear_layers += 1
    linear_hidden = list_vars["linear.0.weight"].shape[0]               # 128

    # Classifier
    n_classes = list_vars["classifier.weight"].shape[0]                 # 7

    # Stride of SincConv — not stored in any tensor, this is an architecture
    # parameter. PyanNet.SINCNET_DEFAULTS = {"stride": 10} for segmentation-3.0.
    # If a different model uses a different stride, change this here.
    sincnet_stride_0 = 10

    print("hparams (derived from model):")
    print(f"  sincnet_filters_0  = {sincnet_filters_0}")
    print(f"  sincnet_kernel_0   = {sincnet_kernel_0}")
    print(f"  sincnet_stride_0   = {sincnet_stride_0}")
    print(f"  sincnet_filters_1  = {sincnet_filters_1}")
    print(f"  sincnet_kernel_1   = {sincnet_kernel_1}")
    print(f"  sincnet_filters_2  = {sincnet_filters_2}")
    print(f"  sincnet_kernel_2   = {sincnet_kernel_2}")
    print(f"  lstm_input_size    = {lstm_input_size}")
    print(f"  lstm_hidden_size   = {lstm_hidden_size}")
    print(f"  lstm_layers        = {lstm_layers}")
    print(f"  lstm_bidirectional = {lstm_bidir}")
    print(f"  linear_hidden      = {linear_hidden}")
    print(f"  linear_layers      = {linear_layers}")
    print(f"  n_classes          = {n_classes}")

    fout.write(struct.pack("i", sincnet_filters_0))   # 80
    fout.write(struct.pack("i", sincnet_kernel_0))    # 251
    fout.write(struct.pack("i", sincnet_stride_0))    # 10
    fout.write(struct.pack("i", sincnet_filters_1))   # 60
    fout.write(struct.pack("i", sincnet_kernel_1))    # 5
    fout.write(struct.pack("i", sincnet_filters_2))   # 60
    fout.write(struct.pack("i", sincnet_kernel_2))    # 5
    fout.write(struct.pack("i", lstm_input_size))     # 60
    fout.write(struct.pack("i", lstm_hidden_size))    # 128
    fout.write(struct.pack("i", lstm_layers))         # 4
    fout.write(struct.pack("i", lstm_bidir))          # 1
    fout.write(struct.pack("i", linear_hidden))       # 128
    fout.write(struct.pack("i", linear_layers))       # 2
    fout.write(struct.pack("i", n_classes))           # 7

    # =========================================================================
    # Write model tensors
    # =========================================================================
    # Following the exact whisper converter pattern (lines 295-337):
    #
    #   for name in list_vars.keys():
    #       data = list_vars[name].squeeze().numpy()
    #       n_dims = len(data.shape)
    #       ftype = 1
    #       if use_f16:
    #           if n_dims < 2 or name == ...:
    #               data = data.astype(np.float32)
    #               ftype = 0
    #       str_ = name.encode('utf-8')
    #       fout.write(struct.pack("iii", n_dims, len(str_), ftype))
    #       for i in range(n_dims):
    #           fout.write(struct.pack("i", data.shape[n_dims - 1 - i]))
    #       fout.write(str_)
    #       data.tofile(fout)

    for name in list_vars.keys():
        data = list_vars[name].squeeze().numpy()
        print("Processing variable: " , name ,  " with shape: ", data.shape)

        n_dims = len(data.shape)

        # Write data as-is (f32), same as whisper converter which just writes
        # whatever dtype the checkpoint already has. The pyannote checkpoint is
        # all float32. We can add f16 quantization later as an optimization.
        data = data.astype(np.float32)
        ftype = 0

        # header
        str_ = name.encode('utf-8')
        fout.write(struct.pack("iii", n_dims, len(str_), ftype))
        for i in range(n_dims):
            fout.write(struct.pack("i", data.shape[n_dims - 1 - i]))
        fout.write(str_)

        # data
        data.tofile(fout)

    fout.close()

    print(f"\nDone. Output file: {fname_out}")
    print(f"Output size: {os.path.getsize(fname_out)} bytes ({os.path.getsize(fname_out) / 1024 / 1024:.2f} MB)")


# ============================================================================
# Main
# ============================================================================

if len(sys.argv) < 2:
    print("Usage:")
    print(f"  {sys.argv[0]} --inspect model.bin                # inspect model tensors")
    print(f"  {sys.argv[0]} model.bin dir-output [use-f32]     # convert to GGML")
    print()
    print("Prerequisites: pip install pyannote.audio")
    sys.exit(1)

if sys.argv[1] == "--inspect":
    if len(sys.argv) < 3:
        print("Usage: --inspect model.bin")
        sys.exit(1)
    inspect_model(sys.argv[2])
else:
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} model.bin dir-output")
        sys.exit(1)
    fname_inp = sys.argv[1]
    dir_out = sys.argv[2]
    convert_model(fname_inp, dir_out)
