import subprocess
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent if Path(__file__).resolve().parent.name == "tools" else Path(__file__).resolve().parent
TOOLS_DIR = Path(__file__).resolve().parent if Path(__file__).resolve().parent.name == "tools" else (Path(__file__).resolve().parent / "tools")

def get_git_diff():
    # Run git diff to get the changes in the manual files
    result = subprocess.run(
        ["git", "diff", "fon/"],
        cwd=ROOT,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="ignore"
    )
    return result.stdout

def parse_diff(diff_text):
    # Split the diff into files
    file_diffs = diff_text.split("diff --git ")
    bilingual_data = []

    for file_diff in file_diffs:
        if not file_diff.strip():
            continue
        
        # Get filename
        lines = file_diff.splitlines()
        filename_match = re.search(r"b/(fon/manual_\w+\.cpp)", lines[0])
        if not filename_match:
            continue
        filename = filename_match.group(1)
        
        # Parse hunks
        hunks = file_diff.split("@@")
        for hunk in hunks[1:]:
            hunk_lines = hunk.splitlines()
            # Ignore the header line of the hunk
            if not hunk_lines:
                continue
            
            english_lines = []
            chinese_lines = []
            
            for line in hunk_lines[1:]:
                if line.startswith("-") and not line.startswith("---"):
                    # Strip the leading '-'
                    english_lines.append(line[1:])
                elif line.startswith("+") and not line.startswith("+++"):
                    # Strip the leading '+'
                    chinese_lines.append(line[1:])
            
            if english_lines or chinese_lines:
                bilingual_data.append({
                    "file": filename,
                    "english": "\n".join(english_lines),
                    "chinese": "\n".join(chinese_lines)
                })
                
    return bilingual_data

def main():
    diff_text = get_git_diff()
    bilingual_data = parse_diff(diff_text)
    
    output_path = TOOLS_DIR / "manuals_bilingual_review.txt"
    with open(output_path, "w", encoding="utf-8") as f:
        f.write("================================================================================\n")
        f.write("Praat ZH Help Manual Bilingual Review Data (4 Core Manuals + 8 Extension Pages)\n")
        f.write("Generated from active git diff. Use this to review translation quality and link syntax.\n")
        f.write("================================================================================\n\n")
        
        for idx, entry in enumerate(bilingual_data, 1):
            f.write(f"Entry {idx} | File: {entry['file']}\n")
            f.write("-" * 80 + "\n")
            f.write("[Original English (Raw C++ Strings)]:\n")
            f.write(entry['english'] + "\n\n")
            f.write("[Chinese Translation (Raw C++ Strings)]:\n")
            f.write(entry['chinese'] + "\n")
            f.write("=" * 80 + "\n\n")
            
    print(f"Successfully exported bilingual manual data to {output_path}")

if __name__ == "__main__":
    main()
