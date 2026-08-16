import subprocess
import re
import os
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent if Path(__file__).resolve().parent.name == "tools" else Path(__file__).resolve().parent
TOOLS_DIR = Path(__file__).resolve().parent if Path(__file__).resolve().parent.name == "tools" else (Path(__file__).resolve().parent / "tools")

def get_git_diff():
    result = subprocess.run(
        ["git", "diff", "fon/"],
        cwd=ROOT,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="ignore"
    )
    return result.stdout

def get_original_file_content(filepath):
    # Run git show HEAD:path/to/file to get the original content
    result = subprocess.run(
        ["git", "show", f"HEAD:{filepath}"],
        cwd=ROOT,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="ignore"
    )
    return result.stdout

def get_current_file_content(filepath):
    with open(ROOT / filepath, "r", encoding="utf-8", errors="ignore") as f:
        return f.read()

def extract_page_block(content, page_title, page_type):
    if page_type == "macro":
        # Search for the block starting with MAN_BEGIN(U"page_title", ... up to MAN_END
        pattern = r'(MAN_BEGIN\s*\(\s*U"' + re.escape(page_title) + r'"[\s\S]*?MAN_END)'
        match = re.search(pattern, content)
        if match:
            return match.group(1).strip()
    elif page_type == "sep":
        # For SoundEditor and Intro, find the block and read until the next #################### banner
        # Or closing of raw string block
        pattern = r'(\"' + re.escape(page_title) + r'\"[\s\S]*?)(?=\#\#\#\#\#\#\#\#|\Z)'
        match = re.search(pattern, content)
        if match:
            return match.group(1).strip()
    return None

def main():
    # 1. Start with the 12 core translated pages
    pages_to_extract = [
        {"title": "SoundEditor", "file": "fon/manual_sound.cpp", "type": "sep"},
        {"title": "LongSound", "file": "fon/manual_soundFiles.cpp", "type": "macro"},
        {"title": "LongSoundEditor", "file": "fon/manual_soundFiles.cpp", "type": "macro"},
        {"title": "TextGridEditor", "file": "fon/manual_annotation.cpp", "type": "macro"},
        {"title": "Objects window", "file": "fon/manual_tutorials.cpp", "type": "macro"},
        {"title": "List of Objects", "file": "fon/manual_tutorials.cpp", "type": "macro"},
        {"title": "Dynamic menu", "file": "fon/manual_tutorials.cpp", "type": "macro"},
        {"title": "New menu", "file": "fon/manual_tutorials.cpp", "type": "macro"},
        {"title": "Open menu", "file": "fon/manual_tutorials.cpp", "type": "macro"},
        {"title": "Save menu", "file": "fon/manual_tutorials.cpp", "type": "macro"},
        {"title": "time selection", "file": "fon/manual_glossary.cpp", "type": "macro"},
        {"title": "Command-click", "file": "fon/manual_Fon.cpp", "type": "macro"}
    ]
    
    # 2. Add the main "Intro" index page
    pages_to_extract.append({"title": "Intro", "file": "fon/manual_tutorials.cpp", "type": "sep"})
    
    # 3. Dynamically discover all "Intro [1-8]" sub-pages in manual_tutorials.cpp
    tutorials_content = get_current_file_content("fon/manual_tutorials.cpp")
    intro_titles = re.findall(r'MAN_BEGIN\s*\(\s*U"(Intro\s+[1-8][^"]*)"', tutorials_content)
    
    for title in sorted(list(set(intro_titles))):
        pages_to_extract.append({"title": title, "file": "fon/manual_tutorials.cpp", "type": "macro"})
        
    output_path = TOOLS_DIR / "manuals_bilingual_review.txt"
    
    with open(output_path, "w", encoding="utf-8") as out:
        out.write("================================================================================\n")
        out.write("Praat ZH Help Manual Bilingual Review Data (Intro Pages + Core Manuals + Extension Pages)\n")
        out.write("Generated from actual codebase (Current Chinese vs Original English in Git HEAD).\n")
        out.write("This file is formatted cleanly for other AIs to review translation quality.\n")
        out.write("================================================================================\n\n")
        
        total_pages = len(pages_to_extract)
        for idx, page in enumerate(pages_to_extract, 1):
            title = page["title"]
            filepath = page["file"]
            ptype = page["type"]
            
            # Read current and original contents
            current_content = get_current_file_content(filepath)
            original_content = get_original_file_content(filepath)
            
            # Extract page blocks
            chinese_block = extract_page_block(current_content, title, ptype)
            english_block = extract_page_block(original_content, title, ptype)
            
            out.write(f"[{idx}/{total_pages}] Manual Page: {title}\n")
            out.write(f"Source File: {filepath}\n")
            out.write("-" * 80 + "\n")
            
            if english_block:
                out.write("[Original English (Raw C++ Strings)]:\n")
                out.write(english_block + "\n\n")
            else:
                out.write("[Original English]: (Not found)\n\n")
                
            if chinese_block:
                out.write("[Chinese Translation (Raw C++ Strings)]:\n")
                out.write(chinese_block + "\n")
            else:
                out.write("[Chinese Translation]: (Not found)\n")
                
            out.write("=" * 80 + "\n\n")
            
    print(f"Successfully generated clean bilingual manuals file with {total_pages} pages: {output_path}")

if __name__ == "__main__":
    main()
