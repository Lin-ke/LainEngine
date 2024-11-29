# 检查是否所有.h 文件都以 #ifdef  开头
import os
def val_a_file(lines):
    for line in lines:
        if(len(line) == 0 or line == "\n") : # 读开头空行
            continue
        if not line.strip().startswith('#ifndef') :
            if line.startswith('#pragma once'):
                continue
            # 不是 pragma 也不是 ifdef
            return False, line
        return True, "123"
    return True, "123"
def validate():
    for root, dirs, files in os.walk('..'):
        for file in files:
            if "meta_parser" or "include" in root:
                continue
            if file.endswith('.h'):
                try:
                    with open(os.path.join(root, file), 'r', encoding="gbk") as f:
                        ok, line = val_a_file(f.readlines())
                        if not ok:
                            print(f'Error: {file} does not start with #ifndef', line)
                except:
                    # 使用utf8编码
                    with open(os.path.join(root, file), 'r', encoding='utf-8') as f:
                        ok, line = val_a_file(f.readlines())
                        if not ok:
                            print(f'Error: {file} does not start with #ifndef', line)
def change_encode(path):
    for root, dirs, files in os.walk(path):
        for file in files:
            # if file.endswith('.h') or file.endswith('.cpp'):
                try:
                    with open(os.path.join(root, file), 'r', encoding="utf-8") as f:
                        lines = f.readlines()
                except:
                    with open(os.path.join(root, file), 'r', encoding="gbk") as f:
                        # 保存f 用 utf8
                        lines = f.readlines()
                    with open(os.path.join(root, file), 'w', encoding="utf-8") as f:
                        f.writelines(lines)
change_encode("../engine/source/meta_parser")