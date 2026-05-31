import os, json

root = os.getcwd()  # 项目根路径
flags = "-Iinclude -Ithirdparty -DDEBUG"  # 你的编译参数
entries = []

for dirpath, _, filenames in os.walk("."):
    for f in filenames:
        if f.endswith(".h") or f.endswith(".cpp") or f.endswith(".hpp"):
            rel = os.path.join(dirpath, f)
            entries.append({
                "directory": root,
                "command": f"clang -c {rel} {flags}",
                "file": rel
            })

with open("compile_commands.json", "w") as out:
    json.dump(entries, out, indent=2)