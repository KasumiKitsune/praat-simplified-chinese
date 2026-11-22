#!/usr/bin/env python3
"""
Extract lossless localization assets for Praat_ZH Phase 3:
  1. locales/zh_CN.tsv         (context \t en_source \t zh_CN)
  2. locales/zh_CN_aliases.tsv (zh_alias \t en_canonical)
  3. locales/zh_CN_manuals.tsv (page_title \t par_index \t en_hash \t zh_CN)

Run from repository root while manual_*.cpp and sys/praat_translate.cpp still
hold the current translated baseline.
"""

from __future__ import annotations

import difflib
import re
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LOCALES_DIR = ROOT / "locales"

MANUAL_FILES = [
    "EEG/manual_EEG.cpp",
    "fon/manual_Fon.cpp",
    "fon/manual_Manual.cpp",
    "fon/manual_Picture.cpp",
    "fon/manual_SpeechRecognizer.cpp",
    "fon/manual_annotation.cpp",
    "fon/manual_formulas.cpp",
    "fon/manual_glossary.cpp",
    "fon/manual_pitch.cpp",
    "fon/manual_pointProcess.cpp",
    "fon/manual_scripting.cpp",
    "fon/manual_sound.cpp",
    "fon/manual_soundFiles.cpp",
    "fon/manual_spectrum.cpp",
    "fon/manual_tutorials.cpp",
    "fon/manual_voice.cpp",
    "gram/manual_gram.cpp",
]

PAR_MACROS = {
    "INTRO", "ENTRY", "NORMAL", "LIST_ITEM", "LIST_ITEM1", "LIST_ITEM2", "LIST_ITEM3",
    "TERM", "TERM1", "TERM2", "TERM3",
    "DEFINITION", "DEFINITION1", "DEFINITION2", "DEFINITION3",
    "CODE", "CODE1", "CODE2", "CODE3", "CODE4", "CODE5",
    "PROTOTYPE", "EQUATION", "CAPTION", "QUOTE", "QUOTE1", "QUOTE2", "QUOTE3",
    "SUBHEADER", "PICTURE", "SCRIPT",
    "TAG", "TAG1", "TAG2", "TAG3", "FORMULA",
}

# Additional UI strings promoted from hardcoded g_language_choice branches
EXTRA_UI_ENTRIES: list[tuple[str, str, str]] = [
    ("", "Total duration %f seconds", "总时长 %f 秒"),
    ("", "Visible part %f seconds", "可见部分 %f 秒"),
    ("", "%f (%.3f / s)", "%f (%.3f / 秒)"),
    ("", "Total bandwidth %.2f Hz", "总带宽 %.2f 赫兹"),
    ("", "Visible part %.2f Hz", "可见部分 %.2f 赫兹"),
    ("", "Best matches", "最佳匹配"),
    ("", "The best matches to your query seem to be:", "与您的查询最匹配的页面如下："),
    ("", "Links to this page", "指向此页面的链接"),
    ("", "Copy text", "复制全文"),
    ("", "Copy page text", "复制页面全文"),
    ("", "Open official English manual (Web)...", "打开官方英文手册 (网页)..."),
    ("", "Whisper Model Download & Setup Guide", "Whisper 模型下载与配置向导"),
    ("", "Open Folder", "打开模型文件夹"),
    ("", "Refresh", "刷新状态"),
    ("", "Download base model", "下载 base 模型"),
    ("", "Re-download base model", "重新下载 base 模型"),
    ("", "All Models...", "浏览所有模型..."),
    ("", "No Whisper speech recognition models found (.bin)", "未检测到 Whisper 语音识别模型 (.bin)"),
    ("", "Praat includes Whisper.cpp engine, but requires at least one model weight file.", "Praat 已内置 Whisper.cpp 语音识别引擎，但需要至少一个模型权重文件以执行转写。"),
    ("", "Recommended model: ggml-base.bin (~142 MB, good balance of speed and accuracy).", "推荐模型：ggml-base.bin（约 142 MB，速度与准确率平衡，支持中文等多语言识别）。"),
    ("", "Place models in models\\whispercpp\\ or user preferences directory.", "支持便携目录：可直接放置于 Praat 程序同级的 models\\whispercpp\\ 或用户偏好目录。"),
    ("", "Speech recognition is ready! Create a SpeechRecognizer or transcribe from TextGrid window.", "语音识别功能已就绪！您可在对象列表创建 SpeechRecognizer，或在 TextGrid 窗口直接转写。"),
    ("", "Models location: models\\whispercpp\\ or preferences folder. Use buttons below to manage models.", "模型存放目录：软件同级 models\\whispercpp\\ 或用户偏好目录。可通过下方按钮继续下载其他模型。"),
    ("", "No Whisper speech recognition model detected.\nThe model download & setup guide has been opened. You can also place ggml-*.bin models directly into the models/whispercpp folder next to the executable.",
         "未检测到 Whisper 语音识别模型。\n已为您打开模型下载与配置向导，您也可以直接将 ggml-*.bin 模型放置于软件同级目录的 models/whispercpp 文件夹中。"),
    ("", "International network connection detected (official source). Downloading recommended model ggml-base.bin (~142 MB) using the default browser.\n\nThe target models folder has been opened for you. After the download finishes, place ggml-base.bin into that folder.",
         "已检测到国际网络连接正常（官方源），正在使用默认浏览器下载推荐模型 ggml-base.bin (约 142 MB)。\n\n已为您自动打开目标模型文件夹，待浏览器下载完成后，请将 ggml-base.bin 放入该文件夹即可。"),
    ("", "No international network connection detected; switched to mirror source (HF-Mirror). Downloading recommended model ggml-base.bin (~142 MB) using the default browser.\n\nThe target models folder has been opened for you. After the download finishes, place ggml-base.bin into that folder.",
         "未检测到国际网络连接，已自动切换为国内镜像源 (HF-Mirror)，正在使用默认浏览器下载推荐模型 ggml-base.bin (约 142 MB)。\n\n已为您自动打开目标模型文件夹，待浏览器下载完成后，请将 ggml-base.bin 放入该文件夹即可。"),
    ("", "class part \"", "类部分 \""),
    ("", "About Chinese Edition...", "关于中文版..."),
]


def fnv1a_u32(text: str) -> int:
    h = 14695981039346656037
    mask = 0xFFFFFFFFFFFFFFFF
    for ch in text:
        h ^= ord(ch)
        h = (h * 1099511628211) & mask
    return h if h != 0 else 1


def tsv_escape(s: str) -> str:
    return (
        s.replace("\\", "\\\\")
         .replace("\t", "\\t")
         .replace("\r", "\\r")
         .replace("\n", "\\n")
    )


def unescape_cpp_str(s: str) -> str:
    out: list[str] = []
    i = 0
    n = len(s)
    while i < n:
        if s[i] == '"':
            i += 1
            while i < n and s[i] != '"':
                if s[i] == "\\" and i + 1 < n:
                    nxt = s[i + 1]
                    if nxt == "n":
                        out.append("\n"); i += 2
                    elif nxt == "t":
                        out.append("\t"); i += 2
                    elif nxt == "r":
                        out.append("\r"); i += 2
                    elif nxt == '"':
                        out.append('"'); i += 2
                    elif nxt == "\\":
                        out.append("\\"); i += 2
                    elif nxt == "u" and i + 6 <= n:
                        out.append(chr(int(s[i + 2 : i + 6], 16))); i += 6
                    elif nxt == "U" and i + 10 <= n:
                        out.append(chr(int(s[i + 2 : i + 10], 16))); i += 10
                    else:
                        out.append(nxt); i += 2
                else:
                    out.append(s[i]); i += 1
            i += 1
        else:
            i += 1
    return "".join(out)


def has_cjk(s: str | None) -> bool:
    if not s:
        return False
    return any("\u4e00" <= c <= "\u9fff" for c in s)


def git_show(rev: str, rel_path: str) -> str:
    r = subprocess.run(["git", "show", f"{rev}:{rel_path}"], cwd=ROOT, capture_output=True)
    if r.returncode != 0:
        return ""
    return r.stdout.decode("utf-8", errors="replace")


def extract_ui_and_aliases() -> tuple[list[tuple[str, str, str]], list[tuple[str, str]]]:
    src = (ROOT / "sys/praat_translate.cpp").read_text(encoding="utf-8")

    # 1. Extract g_translation_map entries: { U"...", U"..." }
    m_start = src.find("g_translation_map")
    m_end = src.find("};", m_start)
    map_block = src[m_start:m_end]
    pair_re = re.compile(r'\{\s*U"((?:[^"\\]|\\.)*)"\s*,\s*U"((?:[^"\\]|\\.)*)"\s*\}')
    ui_dict: dict[tuple[str, str], str] = {}
    for m in pair_re.finditer(map_block):
        en = unescape_cpp_str('"' + m.group(1) + '"')
        zh = unescape_cpp_str('"' + m.group(2) + '"')
        if en and zh:
            ui_dict[("", en)] = zh

    for ctx, en, zh in EXTRA_UI_ENTRIES:
        if (ctx, en) not in ui_dict:
            ui_dict[(ctx, en)] = zh

    # 2. Extract curated reverse disambiguation entries from ensure_reverse_translation_map
    r_start = src.find("ensure_reverse_translation_map")
    r_end = src.find("for (const auto& entry : g_translation_map)", r_start)
    rev_block = src[r_start:r_end]
    rev_re = re.compile(r'emplace\s*\(\s*U"((?:[^"\\]|\\.)*)"\s*,\s*U"((?:[^"\\]|\\.)*)"\s*\)')
    aliases: list[tuple[str, str]] = []
    seen_alias: set[str] = set()
    for m in rev_re.finditer(rev_block):
        zh = unescape_cpp_str('"' + m.group(1) + '"')
        en = unescape_cpp_str('"' + m.group(2) + '"')
        if zh and en and zh not in seen_alias:
            seen_alias.add(zh)
            aliases.append((zh, en))

    ui_rows = [(ctx, en, zh) for (ctx, en), zh in ui_dict.items()]
    return ui_rows, aliases


def extract_macro_pages(src: str) -> dict[str, list[tuple[str, str | None]]]:
    pages: dict[str, list[tuple[str, str | None]]] = {}
    pos = 0
    while True:
        m = re.search(r"\bMAN_BEGIN\s*\(", src[pos:])
        if not m:
            break
        start = pos + m.start()
        end_m = re.search(r"\bMAN_END\b", src[start:])
        if not end_m:
            break
        end = start + end_m.end()
        block = src[start:end]
        pos = end

        tm = re.match(r'MAN_BEGIN\s*\(\s*U"((?:[^"\\]|\\.)*)"', block)
        if not tm:
            continue
        title = unescape_cpp_str('"' + tm.group(1) + '"')

        idx = block.find("(")
        depth = 0
        in_str = False
        esc = False
        body_start = 0
        for i in range(idx, len(block)):
            ch = block[i]
            if in_str:
                if esc:
                    esc = False
                elif ch == "\\":
                    esc = True
                elif ch == '"':
                    in_str = False
            else:
                if ch == '"':
                    in_str = True
                elif ch == "(":
                    depth += 1
                elif ch == ")":
                    depth -= 1
                    if depth == 0:
                        body_start = i + 1
                        break

        body = block[body_start : block.rfind("MAN_END")]
        clean: list[str] = []
        i = 0
        bn = len(body)
        in_str = False
        esc = False
        while i < bn:
            ch = body[i]
            if in_str:
                clean.append(ch)
                if esc:
                    esc = False
                elif ch == "\\":
                    esc = True
                elif ch == '"':
                    in_str = False
                i += 1
            else:
                if ch == '"':
                    in_str = True
                    clean.append(ch); i += 1
                elif ch == "/" and i + 1 < bn and body[i + 1] == "/":
                    while i < bn and body[i] != "\n":
                        i += 1
                elif ch == "/" and i + 1 < bn and body[i + 1] == "*":
                    i += 2
                    while i + 1 < bn and not (body[i] == "*" and body[i + 1] == "/"):
                        i += 1
                    i += 2
                else:
                    clean.append(ch); i += 1
        body = "".join(clean)

        pars: list[tuple[str, str | None]] = []
        i = 0
        bn = len(body)
        while i < bn:
            mm = re.search(r"\b([A-Z][A-Z0-9_]*)\s*\(", body[i:])
            if not mm:
                break
            macro = mm.group(1)
            arg_start = i + mm.end()
            depth = 1
            in_str = False
            esc = False
            j = arg_start
            while j < bn and depth > 0:
                ch = body[j]
                if in_str:
                    if esc:
                        esc = False
                    elif ch == "\\":
                        esc = True
                    elif ch == '"':
                        in_str = False
                else:
                    if ch == '"':
                        in_str = True
                    elif ch == "(":
                        depth += 1
                    elif ch == ")":
                        depth -= 1
                j += 1
            arg_text = body[arg_start : j - 1]
            i = j
            if macro in PAR_MACROS:
                if macro in ("PICTURE", "SCRIPT"):
                    pars.append((macro, None))
                else:
                    pars.append((macro, unescape_cpp_str(arg_text)))
        pages[title] = pars
    return pages


def extract_notebook_pages(src: str) -> dict[str, str]:
    pages: dict[str, str] = {}
    for m in re.finditer(r'R"~~~\(([\s\S]*?)\)~~~"', src):
        raw = m.group(1)
        chunks = re.split(r"(?m)^#{20,}.*$", raw)
        for ch in chunks:
            ch_s = ch.strip()
            if not ch_s:
                continue
            first = ch_s.splitlines()[0].strip()
            if first.startswith('"') and first.rfind('"') > 0:
                title = first[1 : first.rfind('"')]
                pages[title] = ch_s
    return pages


def string_has_ink(line: str) -> bool:
    return line.strip(" \t") != ""


def starts_with_paren_num(s: str) -> bool:
    return bool(re.match(r"^\(\d+\)", s))


def starts_with_num_dot(s: str) -> bool:
    return bool(re.match(r"^(\d+\.)+[ \t]", s))


def parse_notebook_paragraphs(raw_page: str) -> list[tuple[str, str | None]]:
    """Mirror sys/ManPages.cpp::readOnePage_notebook exact paragraph splitting."""
    lines = raw_page.splitlines()
    idx = 0
    n = len(lines)
    if idx >= n:
        return []
    idx += 1  # title
    if idx >= n:
        return []
    idx += 1  # signature
    while idx < n and ":" in lines[idx]:
        idx += 1
    while idx < n and not string_has_ink(lines[idx]):
        idx += 1

    pars: list[list[str | None]] = []
    line = lines[idx] if idx < n else None
    idx += 1

    def next_line() -> str | None:
        nonlocal idx
        if idx < n:
            l = lines[idx]
            idx += 1
            return l
        return None

    while line is not None:
        while line is not None and not string_has_ink(line):
            line = next_line()
        if line is None:
            break
        leading = 0
        p = 0
        while p < len(line) and line[p] in (" ", "\t"):
            if line[p] == "\t":
                leading += 4 - (leading & 3)
            else:
                leading += 1
            p += 1
        rest = line[p:]

        if leading == 0 and rest.startswith("==="):
            if pars:
                pars[-1][0] = "ENTRY"
            line = next_line()
            continue
        if leading == 0 and rest.startswith("---"):
            if pars:
                pars[-1][0] = "SUBHEADER"
            line = next_line()
            continue
        if rest.startswith("//"):
            line = next_line()
            continue

        buf = ""
        ptype: str
        if (
            (rest.startswith(",") and (len(rest) == 1 or rest[1] in (" ", "\t")))
            or (len(rest) >= 2 and rest[0] in ("-", "*", "•") and rest[1] in (" ", "\t"))
            or starts_with_paren_num(rest)
            or starts_with_num_dot(rest)
            or (len(rest) >= 2 and rest[0] == "|" and rest[1] in (" ", "\t"))
        ):
            ptype = (
                "LIST_ITEM" if leading < 3
                else "LIST_ITEM1" if leading < 7
                else "LIST_ITEM2" if leading < 11
                else "LIST_ITEM3"
            )
            if rest[0] == ",":
                buf = " " if len(rest) == 1 else " " + rest[2:]
            elif rest[0] in ("-", "*", "•"):
                buf = "• " + rest[2:]
            elif rest[0] == "|":
                s = rest[2:]
                out = ["\t"]
                for k, ch in enumerate(s):
                    prev_sp = (k > 0 and s[k - 1] in (" ", "\t")) or (k == 0 and rest[1] in (" ", "\t"))
                    next_sp = (k + 1 == len(s)) or (s[k + 1] in (" ", "\t"))
                    out.append("\t" if (ch == "|" and prev_sp and next_sp) else ch)
                buf = "".join(out)
            else:
                buf = rest
        elif len(rest) >= 2 and rest[0] == ":" and rest[1] in (" ", "\t"):
            ptype = (
                "DEFINITION" if leading < 3
                else "DEFINITION1" if leading < 7
                else "DEFINITION2" if leading < 11
                else "DEFINITION3"
            )
            if pars and pars[-1][0] in ("NORMAL", "CAPTION"):
                pars[-1][0] = ptype.replace("DEFINITION", "TERM")
            buf = rest[2:].lstrip(" \t")
        elif len(rest) >= 2 and rest[0] == ">" and rest[1] in (" ", "\t"):
            ptype = (
                "QUOTE" if leading < 3
                else "QUOTE1" if leading < 7
                else "QUOTE2" if leading < 11
                else "QUOTE3"
            )
            buf = rest[2:].lstrip(" \t")
        elif leading == 0 and len(rest) >= 2 and rest[0] == "~" and rest[1] in (" ", "\t"):
            ptype = "EQUATION"
            buf = rest[2:].lstrip(" \t")
        elif leading == 0 and rest.startswith("`") and not string_has_ink(rest[1:]):
            while True:
                line = next_line()
                if line is None:
                    break
                if line.startswith("`") and not string_has_ink(line[1:]):
                    line = next_line()
                    break
                code_out = []
                for k, ch in enumerate(line):
                    if ch == "\t":
                        if k != 0:
                            code_out.append("    ")
                    else:
                        code_out.append(ch)
                pars.append(["CODE", "".join(code_out)])
            continue
        elif leading == 0 and rest.startswith("{"):
            should_show_code = "-" not in rest
            should_show_output = ";" not in rest
            script_buf = []
            while True:
                line = next_line()
                if line is None:
                    break
                if line.startswith("}"):
                    line = next_line()
                    break
                if should_show_code:
                    code_out = []
                    for k, ch in enumerate(line):
                        if ch == "\t":
                            if k != 0:
                                code_out.append("    ")
                        else:
                            code_out.append(ch)
                    pars.append(["CODE", "".join(code_out)])
                if should_show_output:
                    script_buf.append(line)
            if should_show_output and script_buf:
                pars.append(["SCRIPT", None])
            continue
        elif leading >= 3:
            ptype = "CAPTION"
            buf = rest
        else:
            ptype = "NORMAL"
            buf = rest

        par = None
        if buf != "":
            par = [ptype, buf]
            pars.append(par)

        if ptype.startswith("CODE"):
            line = next_line()
        elif par is not None:
            while True:
                cont = next_line()
                if cont is None:
                    line = None
                    break
                cp = 0
                while cp < len(cont) and cont[cp] in (" ", "\t"):
                    cp += 1
                fn = cont[cp:]
                is_new = (
                    fn == ""
                    or fn.startswith(":")
                    or fn.startswith(">")
                    or (fn.startswith(",") and (len(fn) == 1 or fn[1] in (" ", "\t")))
                    or (len(fn) >= 2 and fn[0] in ("-", "*", "•") and fn[1] in (" ", "\t"))
                    or starts_with_paren_num(fn)
                    or starts_with_num_dot(fn)
                    or (len(fn) >= 2 and fn[0] == "|" and fn[1] in (" ", "\t"))
                    or (cp == 0 and fn.startswith("{"))
                    or (cp == 0 and fn.startswith("`") and not string_has_ink(fn[1:]))
                    or (cp == 0 and fn.startswith("~"))
                    or fn.startswith("===")
                    or fn.startswith("---")
                    or fn.startswith("//")
                )
                if is_new:
                    line = cont
                    break
                sep = "" if par[1] == "" else " "
                par[1] = str(par[1]) + sep + fn
        else:
            line = next_line()

    return [(str(t), s) for t, s in pars]


def align_paragraphs(
    title: str,
    u_pars: list[tuple[str, str | None]],
    h_pars: list[tuple[str, str | None]],
) -> list[tuple[int, str, str]]:
    """Return list of (1-based par_idx, en_text, zh_text) for translated paragraphs."""
    # Handle the 3 known split/insert pages explicitly so alignment is 100% lossless
    if title == "Intro 8.1. Manipulation of pitch" and len(u_pars) == 9 and len(h_pars) == 10:
        merged_6 = (h_pars[5][1] or "") + " " + (h_pars[6][1] or "")
        h_pars = h_pars[:5] + [(h_pars[5][0], merged_6)] + h_pars[7:]
    elif title == "Speech recognition" and len(u_pars) == 95 and len(h_pars) == 98:
        # Paragraphs 8..10 in HEAD are the expanded portable-folder note for upstream paragraph 8 (index 7)
        merged_8 = (
            (h_pars[7][1] or "")
            + " "
            + (h_pars[8][1] or "")
            + " "
            + (h_pars[9][1] or "")
            + " "
            + (h_pars[10][1] or "")
        )
        h_pars = h_pars[:7] + [(h_pars[7][0], merged_8)] + h_pars[11:]
    elif title == "Scripting 4.3. Querying objects" and len(u_pars) == 45 and len(h_pars) == 44:
        # Upstream index 23 (1-based 24) is CODE 'selectObject: “Pitch ” + pitchName$' omitted in HEAD
        h_pars = h_pars[:23] + [u_pars[23]] + h_pars[23:]

    if len(u_pars) != len(h_pars):
        # Fallback: align by paragraph type sequence
        sm = difflib.SequenceMatcher(
            None, [t for t, _ in u_pars], [t for t, _ in h_pars], autojunk=False
        )
        aligned: list[tuple[int, str, str]] = []
        for tag, i1, i2, j1, j2 in sm.get_opcodes():
            if tag == "equal":
                for k in range(i2 - i1):
                    ut, us = u_pars[i1 + k]
                    ht, hs = h_pars[j1 + k]
                    if ut not in ("PICTURE", "SCRIPT") and us and hs and hs != us and has_cjk(hs):
                        aligned.append((i1 + k + 1, us, hs))
        return aligned

    out: list[tuple[int, str, str]] = []
    for idx, ((ut, us), (ht, hs)) in enumerate(zip(u_pars, h_pars), start=1):
        if ut in ("PICTURE", "SCRIPT"):
            continue
        if us and hs and hs != us and has_cjk(hs):
            out.append((idx, us, hs))
    return out


def extract_manuals() -> list[tuple[str, int, int, str]]:
    rows: list[tuple[str, int, int, str]] = []
    seen_keys: set[tuple[str, int]] = set()
    pages_covered = 0

    for mf in MANUAL_FILES:
        up = git_show("upstream/master", mf)
        hd = (ROOT / mf).read_text(encoding="utf-8", errors="replace")

        # Macro pages
        up_m = extract_macro_pages(up)
        hd_m = extract_macro_pages(hd)
        for title, h_pars in hd_m.items():
            if not any(has_cjk(t) for _, t in h_pars if t):
                continue
            if title not in up_m:
                continue
            aligned = align_paragraphs(title, up_m[title], h_pars)
            if aligned:
                pages_covered += 1
                for par_idx, en_text, zh_text in aligned:
                    if (title, par_idx) not in seen_keys:
                        seen_keys.add((title, par_idx))
                        rows.append((title, par_idx, fnv1a_u32(en_text), zh_text))

        # Notebook pages
        up_nb = extract_notebook_pages(up)
        hd_nb = extract_notebook_pages(hd)
        for title, h_raw in hd_nb.items():
            if not has_cjk(h_raw):
                continue
            if title not in up_nb:
                continue
            u_pars = parse_notebook_paragraphs(up_nb[title])
            h_pars = parse_notebook_paragraphs(h_raw)
            aligned = align_paragraphs(title, u_pars, h_pars)
            if aligned:
                pages_covered += 1
                for par_idx, en_text, zh_text in aligned:
                    if (title, par_idx) not in seen_keys:
                        seen_keys.add((title, par_idx))
                        rows.append((title, par_idx, fnv1a_u32(en_text), zh_text))

    print(f"Extracted {len(rows)} manual paragraphs across {pages_covered} pages.")
    return rows


def main() -> None:
    LOCALES_DIR.mkdir(parents=True, exist_ok=True)

    ui_rows, aliases = extract_ui_and_aliases()
    ui_path = LOCALES_DIR / "zh_CN.tsv"
    with ui_path.open("w", encoding="utf-8", newline="\n") as f:
        f.write("# context\ten_source\tzh_CN\n")
        for ctx, en, zh in ui_rows:
            f.write(f"{tsv_escape(ctx)}\t{tsv_escape(en)}\t{tsv_escape(zh)}\n")
    print(f"Wrote {len(ui_rows)} UI entries to {ui_path}")

    alias_path = LOCALES_DIR / "zh_CN_aliases.tsv"
    with alias_path.open("w", encoding="utf-8", newline="\n") as f:
        f.write("# zh_alias\ten_canonical\n")
        for zh, en in aliases:
            f.write(f"{tsv_escape(zh)}\t{tsv_escape(en)}\n")
    print(f"Wrote {len(aliases)} disambiguated aliases to {alias_path}")

    manual_rows = extract_manuals()
    man_path = LOCALES_DIR / "zh_CN_manuals.tsv"
    with man_path.open("w", encoding="utf-8", newline="\n") as f:
        f.write("# page_title\tpar_index\ten_hash_hex\tzh_CN\n")
        for title, par_idx, en_hash, zh_text in manual_rows:
            f.write(f"{tsv_escape(title)}\t{par_idx}\t{en_hash:016x}\t{tsv_escape(zh_text)}\n")
    print(f"Wrote {len(manual_rows)} manual entries to {man_path}")


if __name__ == "__main__":
    main()
