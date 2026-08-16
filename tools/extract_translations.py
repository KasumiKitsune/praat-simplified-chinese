import json
import os
import sys
from pathlib import Path

TOOLS_DIR = Path(__file__).resolve().parent
if str(TOOLS_DIR) not in sys.path:
    sys.path.insert(0, str(TOOLS_DIR))

import generate_translation_map

def main():
    # Load translation map
    translations = generate_translation_map.EXACT_MAP
    total = len(translations)

    # 1. Output as JSON (very easy for AIs to parse directly)
    json_path = TOOLS_DIR / "praat_translations.json"
    with open(json_path, "w", encoding="utf-8", newline='\n') as f:
        json.dump(translations, f, ensure_ascii=False, indent=2)
    print(f"Successfully saved JSON format to {json_path} ({total} entries)")

    # 2. Output as TXT (structured and clear for human reading or prompt pasting)
    txt_path = TOOLS_DIR / "praat_translations.txt"
    with open(txt_path, "w", encoding="utf-8", newline='\n') as f:
        f.write(f"=== Praat Chinese Translation Database (Total: {total} entries) ===\n")
        f.write("Format: [English] block followed by [Chinese] block, separated by '===================='.\n")
        f.write("This structure prevents ambiguity with newlines or special characters in the text.\n\n")
        
        for eng, zhs in sorted(translations.items()):
            f.write("[English]\n")
            f.write(f"{eng}\n")
            f.write("[Chinese]\n")
            f.write(f"{zhs}\n")
            f.write("=" * 20 + "\n")
            
    print(f"Successfully saved TXT format to {txt_path} ({total} entries)")

if __name__ == "__main__":
    main()
