import os

# Dahil edilecek uzantılar
INCLUDE_EXTENSIONS = {".c", ".S", ".h", ".ld", ".s", ".mk"}

# Uzantısız ama özellikle dahil edilmesini istediğin dosyalar
INCLUDE_FILENAMES = {"Makefile", ".clang-format", "limine.conf", "Sbuild"}

# Hariç tutulacak klasörler
EXCLUDE_DIRS = {".git", "limine", "build-test", "__pycache__", "node_modules", "target", "build"}

def collect_code(base_dir="."):
    result = []

    for root, dirs, files in os.walk(base_dir):
        # Gereksiz klasörleri çıkar
        dirs[:] = [d for d in dirs if d not in EXCLUDE_DIRS]

        for file in files:
            ext = os.path.splitext(file)[1]
            should_include = (
                ext in INCLUDE_EXTENSIONS or
                file in INCLUDE_FILENAMES
            )

            if should_include:
                path = os.path.join(root, file)
                try:
                    with open(path, "r", encoding="utf-8", errors="ignore") as f:
                        code = f.read()

                    relative_path = os.path.relpath(path, base_dir)
                    # Dil etiketini kod bloğuna ekleyelim
                    lang = ext[1:] if ext else ""
                    block = f"Here is {relative_path}:\n```{lang}\n{code}\n```\n"
                    result.append(block)
                except Exception as e:
                    print(f"Could not read {path}: {e}")

    return "\n".join(result)

if __name__ == "__main__":
    prompt = collect_code(".")
    with open("code_prompt.txt", "w", encoding="utf-8") as f:
        f.write(prompt)
    print("✅ Prompt saved to code_prompt.txt")
