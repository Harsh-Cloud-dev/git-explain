import os
import re
import sys

EXTENSIONS = {".cpp", ".hpp", ".cc", ".cxx", ".h", ".c"}

comment_pattern = re.compile(
    r'//.*?$|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
    re.DOTALL | re.MULTILINE
)

def replacer(match):
    s = match.group(0)
    if s.startswith('/'):
        return ''
    return s

def process_file(path):
    with open(path, "r", encoding="utf-8") as f:
        code = f.read()

    new_code = re.sub(comment_pattern, replacer, code)

    with open(path, "w", encoding="utf-8") as f:
        f.write(new_code)

    print(f"Processed: {path}")

def process_directory(root):
    for dirpath, _, filenames in os.walk(root):
        for filename in filenames:
            if os.path.splitext(filename)[1] in EXTENSIONS:
                process_file(os.path.join(dirpath, filename))

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 remove_comments.py <directory>")
        sys.exit(1)

    process_directory(sys.argv[1])
