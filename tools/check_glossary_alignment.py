import re
import json
import os
import sys

def parse_markdown_table(content):
    terms = {}
    lines = content.splitlines()
    for line in lines:
        if '|' in line:
            parts = [p.strip() for p in line.split('|')]
            if len(parts) >= 4:
                # Remove markdown formatting like backticks and bold
                eng = re.sub(r'[`*]', '', parts[1]).strip()
                chi = re.sub(r'[`*]', '', parts[2]).strip()
                if eng and chi and eng != "英文原词 (English Term)" and eng != "English" and not eng.startswith("---"):
                    terms[eng] = chi
    return terms

from pathlib import Path

TOOLS_DIR = Path(__file__).resolve().parent

def main():
    glossary_path = TOOLS_DIR / "praat_glossary.md"
    translations_path = TOOLS_DIR / "praat_translations.json"
    manuals_path = TOOLS_DIR / "manuals_bilingual_review.txt"
    report_path = TOOLS_DIR / "translation_check_report.md"
    
    if not os.path.exists(glossary_path):
        print(f"Glossary file {glossary_path} not found.")
        return
        
    with open(glossary_path, "r", encoding="utf-8") as f:
        glossary_content = f.read()
        
    glossary_terms = parse_markdown_table(glossary_content)
    
    report_lines = []
    report_lines.append("# Praat 中文汉化与术语表一致性检查报告\n")
    report_lines.append("本报告由自动检查脚本生成，对比了 `praat_glossary.md` 中的推荐译法与当前界面翻译（`praat_translations.json`）及帮助手册（`manuals_bilingual_review.txt`）的实际译文。\n")
    
    report_lines.append("## 一、 术语表加载详情\n")
    report_lines.append(f"共加载了 **{len(glossary_terms)}** 个术语规范：\n")
    report_lines.append("| 英文原词 | 推荐中文翻译 |")
    report_lines.append("| --- | --- |")
    for eng, chi in glossary_terms.items():
        report_lines.append(f"| `{eng}` | {chi} |")
    report_lines.append("\n")
    
    # Check UI translations
    ui_mismatches = []
    if os.path.exists(translations_path):
        with open(translations_path, "r", encoding="utf-8") as f:
            ui_translations = json.load(f)
            
        for eng, expected_chi in glossary_terms.items():
            if eng in ui_translations:
                actual_chi = ui_translations[eng]
                
                clean_expected = expected_chi
                if '（' in expected_chi:
                    clean_expected = expected_chi.split('（')[0]
                
                match = False
                # Exact match
                if actual_chi == clean_expected or actual_chi == expected_chi:
                    match = True
                # If expected contains options separated by '/'
                elif '/' in expected_chi:
                    options = [opt.strip() for opt in expected_chi.split('/')]
                    clean_opts = []
                    for opt in options:
                        if '（' in opt:
                            clean_opts.append(opt.split('（')[0])
                        clean_opts.append(opt)
                    
                    if actual_chi in clean_opts or any(opt == actual_chi for opt in clean_opts):
                        match = True
                
                if not match:
                    ui_mismatches.append((eng, expected_chi, actual_chi))
                    
        report_lines.append("## 二、 界面翻译（UI Strings）不一致项\n")
        report_lines.append(f"共发现 **{len(ui_mismatches)}** 处不一致：\n")
        report_lines.append("| 英文原词 | 推荐中文翻译 (Glossary) | 当前界面翻译 (Actual UI) |")
        report_lines.append("| --- | --- | --- |")
        for eng, exp, act in ui_mismatches:
            report_lines.append(f"| `{eng}` | {exp} | {act} |")
        report_lines.append("\n")
    else:
        report_lines.append("## 二、 界面翻译（UI Strings）不一致项\n未找到 `praat_translations.json` 文件。\n\n")
        
    # Check manual translation text
    manual_matches = []
    if os.path.exists(manuals_path):
        with open(manuals_path, "r", encoding="utf-8") as f:
            manuals_content = f.read()
            
        pages = manuals_content.split("================================================================================")
        
        # Terms to look out for in manuals (old translation vs new translation)
        bad_patterns = [
            ("长声音", "长音频"),
            ("文本标注", "标注网格"),
            ("操纵对象", "操控对象"),
            ("删除", "移除"),
            ("简短文本文件", "短文本文件"),
            ("反向选定部分", "反转选区"),
            ("设置选区为零", "将选区置零"),
            ("粘贴到选择之后", "粘贴到选区之后"),
            ("复制选择到声音剪贴板", "将选区复制到 Sound 剪贴板"),
        ]
        
        for idx, page in enumerate(pages):
            if "[Chinese Translation (Raw C++ Strings)]:" in page:
                lines = page.split("\n")
                title = "Unknown"
                for line in lines:
                    if "Manual Page:" in line:
                        title = line.split("Manual Page:")[-1].strip()
                        break
                
                chi_start = page.find("[Chinese Translation (Raw C++ Strings)]:")
                if chi_start != -1:
                    chi_section = page[chi_start:]
                    for bad, good in bad_patterns:
                        # Skip if it is code like LongSoundEditor
                        occurrences = []
                        for m in re.finditer(re.escape(bad), chi_section):
                            start = m.start()
                            # Check surrounding context to avoid matching class names
                            # e.g., LongSound is fine, but "长声音" is bad.
                            occurrences.append(start)
                            
                        if occurrences:
                            matching_lines = []
                            sec_lines = chi_section.split("\n")
                            for line in sec_lines:
                                if bad in line:
                                    matching_lines.append(line.strip())
                            manual_matches.append({
                                "page": title,
                                "bad": bad,
                                "good": good,
                                "count": len(occurrences),
                                "lines": matching_lines
                            })
                            
        report_lines.append("## 三、 帮助手册（Manuals）中的陈旧/不一致词条\n")
        report_lines.append(f"共在帮助手册中发现 **{len(manual_matches)}** 处需要修改的词汇使用：\n")
        
        for match in manual_matches:
            report_lines.append(f"### 手册页面：`{match['page']}`\n")
            report_lines.append(f"- **陈旧词汇**：`{match['bad']}` $\\rightarrow$ **推荐译法**：`{match['good']}` ({match['count']} 次)\n")
            report_lines.append("- **相关上下文行**：\n")
            for line in match['lines']:
                report_lines.append(f"  > {line}\n")
            report_lines.append("\n")
    else:
        report_lines.append("## 三、 帮助手册（Manuals）中的陈旧/不一致词条\n未找到 `manuals_bilingual_review.txt` 文件。\n\n")
        
    with open(report_path, "w", encoding="utf-8") as f:
        f.write("\n".join(report_lines))
        
    print(f"Check complete. Report written to {report_path}")

if __name__ == "__main__":
    main()
