import json
import os

# Define the terms to extract and group them by category
GLOSSARY_SCHEMA = {
    "数据对象类型 (Objects & Types)": [
        "Sound",
        "LongSound",
        "TextGrid",
        "Pitch",
        "Formant",
        "Intensity",
        "Harmonicity",
        "PointProcess",
        "Manipulation",
        "Spectrum",
        "Spectrogram",
        "WordList",
        "SpellingChecker",
        "PitchTier",
        "IntensityTier",
        "DurationTier",
        "AmplitudeTier",
        "Strings"
    ],
    "编辑器名称 (Editors)": [
        "SoundEditor",
        "LongSoundEditor",
        "TextGridEditor",
        "PitchTierEditor",
        "IntensityTierEditor",
        "ButtonEditor",
        "ScriptEditor",
        "NotebookEditor"
    ],
    "菜单与操作 (Menus & Actions)": [
        "View & Edit",
        "Play",
        "Mute",
        "Record",
        "Rename...",
        "Info",
        "Copy...",
        "Remove",
        "Inspect",
        "Save as text file...",
        "Save as short text file...",
        "Save as binary file...",
        "Read from file...",
        "Draw",
        "Query",
        "Extract sound selection",
        "Cut",
        "Copy selection to Sound clipboard",
        "Paste after selection",
        "Set selection to zero",
        "Reverse selection",
        "Undo",
        "Redo"
    ],
    "核心声学与分析概念 (Acoustic & Analysis Concepts)": [
        "pitch contour",
        "intensity contour",
        "formant contour",
        "spectral slice",
        "time domain",
        "time selection",
        "boundary",
        "point",
        "interval",
        "tier",
        "sinc-interpolated",
        "linear interpolation",
        "CD-quality stereo",
        "mono",
        "stereo",
        "ceiling",
        "pitch range",
        "spectrogram settings",
        "autocorrelation",
        "filtered autocorrelation"
    ]
}

# Pre-defined fallbacks or custom manual translation overrides to ensure absolute alignment
MANUAL_OVERRIDES = {
    "Sound": "Sound（声音）",
    "LongSound": "LongSound（长声音）",
    "TextGrid": "TextGrid（文本标注）",
    "Pitch": "Pitch（基频/音高）",
    "Formant": "Formant（共振峰）",
    "Intensity": "Intensity（音强）",
    "Harmonicity": "Harmonicity（谐噪比）",
    "PointProcess": "PointProcess（点过程）",
    "Manipulation": "Manipulation（操纵对象）",
    "Spectrum": "Spectrum（频谱）",
    "Spectrogram": "Spectrogram（声谱图）",
    "WordList": "WordList（单词列表）",
    "SpellingChecker": "SpellingChecker（拼写检查器）",
    "PitchTier": "PitchTier（音高层）",
    "IntensityTier": "IntensityTier（音强层）",
    "DurationTier": "DurationTier（时长层）",
    "AmplitudeTier": "AmplitudeTier（振幅层）",
    "Strings": "Strings（字符串列表）",
    
    "SoundEditor": "SoundEditor（声音编辑器）",
    "LongSoundEditor": "LongSoundEditor（长声音编辑器）",
    "TextGridEditor": "TextGridEditor（TextGrid 编辑器）",
    "PitchTierEditor": "PitchTierEditor（音高层编辑器）",
    "IntensityTierEditor": "IntensityTierEditor（音强层编辑器）",
    "ButtonEditor": "ButtonEditor（按钮编辑器）",
    "ScriptEditor": "ScriptEditor（脚本编辑器）",
    "NotebookEditor": "NotebookEditor（笔记本编辑器）",
    
    "View & Edit": "View & Edit（查看与编辑）",
    "Mute": "静音",
    "Rename...": "Rename...（重命名...）",
    "Info": "Info（信息）",
    "Copy...": "Copy...（复制...）",
    "Remove": "Remove（删除）",
    "Inspect": "Inspect（检查）",
    "Save as text file...": "Save as text file...（保存为文本文件...）",
    "Save as short text file...": "Save as short text file...（保存为简短文本文件...）",
    "Save as binary file...": "Save as binary file...（保存为二进制文件...）",
    "Read from file...": "Read from file...（从文件读取...）",
    "Extract sound selection": "Extract sound selection（提取所选声音）",
    
    "pitch contour": "音高曲线",
    "intensity contour": "音强曲线",
    "formant contour": "共振峰曲线",
    "spectral slice": "频谱切片",
    "time domain": "时间域",
    "time selection": "时间选区",
    "boundary": "边界",
    "point": "点（时间点/控制点，视上下文）",
    "interval": "区间",
    "tier": "层",
    "sinc-interpolated": "sinc 插值",
    "linear interpolation": "线性插值",
    "CD-quality stereo": "CD 音质立体声",
    "mono": "单声道",
    "stereo": "立体声/双声道",
    "ceiling": "上限（如最高频率）",
    "pitch range": "音高范围",
    "spectrogram settings": "声谱图设置",
    "autocorrelation": "自相关",
    "filtered autocorrelation": "滤波自相关"
}

from pathlib import Path

TOOLS_DIR = Path(__file__).resolve().parent

def load_ui_translations():
    try:
        with open(TOOLS_DIR / "praat_translations.json", "r", encoding="utf-8") as f:
            return json.load(f)
    except Exception:
        return {}

def main():
    ui_map = load_ui_translations()
    
    output_path = TOOLS_DIR / "praat_glossary.md"
    with open(output_path, "w", encoding="utf-8", newline="\n") as out:
        out.write("# Praat ZH 汉化术语对照表 (Terminology & Glossary)\n\n")
        out.write("本术语表由已翻译的界面词条以及核心帮助手册的译文整理而成，用于规范后续手册翻译与界面翻译的术语一致性。\n\n")
        
        for category, terms in GLOSSARY_SCHEMA.items():
            out.write(f"## {category}\n\n")
            out.write("| 英文原词 (English Term) | 推荐中文翻译 (Chinese Translation) | 来源说明/备忘 (Source/Note) |\n")
            out.write("| --- | --- | --- |\n")
            
            for term in terms:
                # 1. Check manual overrides (high priority for manual consistency)
                chinese = MANUAL_OVERRIDES.get(term)
                source = "帮助手册译法"
                
                # 2. Check UI translation map if not in overrides
                if not chinese:
                    chinese = ui_map.get(term)
                    source = "界面菜单译法"
                    
                # 3. Fallback to lowercase lookups or other forms
                if not chinese:
                    # try stripping ellipses
                    clean_term = term.replace("...", "")
                    chinese = ui_map.get(clean_term) or MANUAL_OVERRIDES.get(clean_term)
                    source = "自动匹配 (去省略号)"
                
                if not chinese:
                    chinese = term  # Fallback to English if not found
                    source = "未收录，暂保留原文"
                
                # Clean up formatting
                chinese_display = chinese.replace("\n", " ").strip()
                out.write(f"| `{term}` | **{chinese_display}** | {source} |\n")
                
            out.write("\n")
            
    print(f"Successfully generated glossary at {output_path}")

if __name__ == "__main__":
    main()
