#!/usr/bin/env python3

# generate/whispercpp/convert-pyannote-embedding-to-ggml.py
# Anastasia Shchupak, 21 March 2026, 30 May 2026

# Convert pyannote/wespeaker-voxceleb-resnet34-LM embedding model from PyTorch to GGML
#
# Prerequisites:
#   pip install pyannote.audio
#
# Usage:
#   # Inspect the model:
#   python convert-pyannote-embedding-to-ggml.py --inspect path/to/pytorch_model.bin
#
#   # Convert to GGML:
#   python convert-pyannote-embedding-to-ggml.py path/to/pytorch_model.bin ./output-dir
#
# Download pytorch_model.bin from:
#   https://huggingface.co/pyannote/wespeaker-voxceleb-resnet34-LM
#
# Output format:
#   - magic ("ggml")
#   - hparams (architecture parameters)
#   - tensors (same format as segmentation converter)
#
# Architecture: WeSpeaker ResNet34
#   conv1(1->32, k=3, p=1) + bn1
#   layer1: 3x BasicBlock(32->32)
#   layer2: 4x BasicBlock(32->64, first stride=2, with shortcut)
#   layer3: 6x BasicBlock(64->128, first stride=2, with shortcut)
#   layer4: 3x BasicBlock(128->256, first stride=2, with shortcut)
#   TSTP pooling (no params)
#   seg_1: Linear(5120->256)
#

import os
import sys
import struct
import numpy as np
import torch
from pathlib import Path


# ============================================================================
# Inspect mode
# ============================================================================

def inspect_model(fname_inp):
    from pyannote.audio import Model

    print(f"Loading: {fname_inp}")
    print(f"File size: {os.path.getsize(fname_inp)} bytes ({os.path.getsize(fname_inp) / 1024 / 1024:.2f} MB)")
    print()

    model = Model.from_pretrained(fname_inp)
    state_dict = model.state_dict()

    print(f"Model class: {type(model).__name__}")
    print(f"Number of tensors: {len(state_dict)}")
    print()
    print(f"  {'Name':65s} {'Shape':25s} {'Dtype':10s} {'Elements':>12s} {'Size (KB)':>10s}")
    print("  " + "-" * 130)

    total_params = 0
    total_bytes = 0
    for name in state_dict.keys():
        tensor = state_dict[name]
        shape_str = str(list(tensor.shape))
        n_elements = tensor.numel()
        n_bytes = n_elements * tensor.element_size()
        total_params += n_elements
        total_bytes += n_bytes
        print(f"  {name:65s} {shape_str:25s} {str(tensor.dtype):10s} {n_elements:12d} {n_bytes/1024:10.1f}")

    print("  " + "-" * 130)
    print(f"  Total: {total_params:,} parameters, {total_bytes / 1024 / 1024:.2f} MB")
    print()

    print("Sample values (first 5 elements):")
    for name in state_dict.keys():
        tensor = state_dict[name]
        flat = tensor.detach().cpu().flatten()
        vals = flat[:min(5, len(flat))].numpy()
        vals_str = ", ".join(f"{v:.6f}" for v in vals)
        print(f"  {name:65s} [{vals_str}]")


# ============================================================================
# Convert mode
# ============================================================================

def convert_model(fname_inp, dir_out):
    from pyannote.audio import Model

    print(f"Loading: {fname_inp}")

    model = Model.from_pretrained(fname_inp)
    model.eval()
    list_vars = model.state_dict()

    print(f"Model class: {type(model).__name__}")
    print(f"Tensors: {len(list_vars)}")

    # Output file
    dir_out = Path(dir_out)
    dir_out.mkdir(parents=True, exist_ok=True)
    fname_out = dir_out / "ggml-embedding.bin"

    fout = fname_out.open("wb")

    # =========================================================================
    # Write magic: "ggml"
    # =========================================================================
    fout.write(struct.pack("i", 0x67676d6c))

    # =========================================================================
    # Write hparams -- derived from tensor shapes
    # =========================================================================

    # Base channels: from conv1 output channels
    m_channels = list_vars["resnet.conv1.weight"].shape[0]  # 32

    # Embed dim and feat dim from seg_1 weight
    embed_dim = list_vars["resnet.seg_1.weight"].shape[0]    # 256
    pool_out_dim = list_vars["resnet.seg_1.weight"].shape[1] # 5120
    # TSTP: pool_out = 2 * stats_dim (mean + std)
    # stats_dim = (feat_dim / 8) * m_channels * 8 = feat_dim * m_channels
    stats_dim = pool_out_dim // 2  # 2560
    feat_dim = stats_dim // m_channels  # 80

    # Count blocks per layer
    num_blocks = []
    for layer_idx in range(1, 5):
        n = 0
        while f"resnet.layer{layer_idx}.{n}.conv1.weight" in list_vars:
            n += 1
        num_blocks.append(n)

    # Check for two_emb_layer
    two_emb_layer = 1 if "resnet.seg_2.weight" in list_vars else 0

    print("hparams (derived from model):")
    print(f"  m_channels      = {m_channels}")
    print(f"  feat_dim        = {feat_dim}")
    print(f"  embed_dim       = {embed_dim}")
    print(f"  num_blocks      = {num_blocks}")
    print(f"  two_emb_layer   = {two_emb_layer}")

    fout.write(struct.pack("i", m_channels))       # 32
    fout.write(struct.pack("i", feat_dim))          # 80
    fout.write(struct.pack("i", embed_dim))         # 256
    fout.write(struct.pack("i", num_blocks[0]))     # 3
    fout.write(struct.pack("i", num_blocks[1]))     # 4
    fout.write(struct.pack("i", num_blocks[2]))     # 6
    fout.write(struct.pack("i", num_blocks[3]))     # 3
    fout.write(struct.pack("i", two_emb_layer))     # 0

    # =========================================================================
    # Write model tensors
    # =========================================================================
    n_written = 0
    n_skipped = 0

    for name in list_vars.keys():
        tensor = list_vars[name]

        # Skip num_batches_tracked -- not needed for inference
        if "num_batches_tracked" in name:
            n_skipped += 1
            continue

        # Don't squeeze 4D conv weights — they need to keep all dimensions
        # (e.g. conv1.weight [32,1,3,3] must NOT become [32,3,3])
        # Only squeeze truly scalar/1-element tensors
        if tensor.dim() <= 1:
            data = tensor.numpy().astype(np.float32)
        else:
            data = tensor.numpy().astype(np.float32)
        n_dims = len(data.shape)

        print(f"  Writing: {name:65s} shape={str(list(data.shape)):25s} ({data.size} elements)")

        ftype = 0  # f32

        # header
        str_ = name.encode('utf-8')
        fout.write(struct.pack("iii", n_dims, len(str_), ftype))
        for i in range(n_dims):
            fout.write(struct.pack("i", data.shape[n_dims - 1 - i]))
        fout.write(str_)

        # data
        data.tofile(fout)
        n_written += 1

    fout.close()

    print(f"\nWritten {n_written} tensors, skipped {n_skipped}")
    print(f"Output file: {fname_out}")
    print(f"Output size: {os.path.getsize(fname_out)} bytes ({os.path.getsize(fname_out) / 1024 / 1024:.2f} MB)")


# ============================================================================
# Main
# ============================================================================

if len(sys.argv) < 2:
    print("Usage:")
    print(f"  {sys.argv[0]} --inspect model.bin                # inspect model tensors")
    print(f"  {sys.argv[0]} model.bin dir-output               # convert to GGML")
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
    convert_model(sys.argv[1], sys.argv[2])
