####
# Credits: https://github.com/anurag3301
####

import json
import subprocess
import shlex
from pathlib import Path

# Constants
PROJECT_ROOT = Path.cwd()
SRC_FILE = PROJECT_ROOT / "src" / "main.cpp"
CCDB_ORIG = PROJECT_ROOT / "compile_commands.json"
CCDB_BAK = PROJECT_ROOT / "compile_commands.json.bak"
CCLS_FILE = PROJECT_ROOT / ".ccls"
CCDB_NEW = PROJECT_ROOT / "compile_commands.json"

KEEP_FLAGS = ("-I", "-D", "-std", "-Wall")


def ensure_compile_commands():
    if not CCDB_ORIG.exists():
        subprocess.run(["pio", "run", "-t", "compiledb"], check=True)
    if not CCDB_BAK.exists():
        CCDB_ORIG.rename(CCDB_BAK)


def extract_compiler():
    return "g++"


def parse_ccls():
    common_flags = []
    cpp_flags = []

    with open(CCLS_FILE) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue

            tokens = line.split()  # simple split

            if tokens[0].startswith("%cpp"):
                flags = tokens[1:]
                target = cpp_flags
            elif tokens[0].startswith("%c"):
                continue
            elif not tokens[0].startswith("%"):
                flags = tokens
                target = common_flags
            else:
                continue

            i = 0
            while i < len(flags):
                flag = flags[i]

                # Handle separated -I /path
                if flag == "-I" and i + 1 < len(flags):
                    combined = f"-I{flags[i + 1]}"
                    target.append(combined)
                    i += 2
                    continue

                # Handle -I/path with spaces
                if flag.startswith("-I"):
                    # combine all following tokens that don't start with '-' as part of path
                    j = i + 1
                    path_parts = []
                    while j < len(flags) and not flags[j].startswith("-"):
                        path_parts.append(flags[j])
                        j += 1
                    if path_parts:
                        flag = flag + " " + " ".join(path_parts)
                        i = j
                        target.append(flag)
                        continue

                    target.append(flag)
                    i += 1
                    continue

                # keep other flags
                if flag == "-Wall" or flag.startswith(("-D", "-std")):
                    target.append(flag)

                i += 1

    return common_flags + cpp_flags

def generate_single_entry_compile_command(compiler, flags):
    entry = {
            "directory": str(PROJECT_ROOT),
            "file": str(SRC_FILE),
            "arguments": [compiler] + flags + [str(SRC_FILE)],
            }
    CCDB_NEW.write_text(json.dumps([entry], indent=2))
    return 1


# Execution
ensure_compile_commands()
compiler_path = extract_compiler()
filtered_flags = parse_ccls()
entry_count = generate_single_entry_compile_command(compiler_path, filtered_flags)
