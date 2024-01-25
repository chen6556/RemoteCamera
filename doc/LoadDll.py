import sys
import os
import shutil


def load_dll() -> list:
    dll_list = list()
    with open("./dll_list.txt", 'r') as f:
        for line in f.readlines():
            if ".dll" in line:
                dll_list.append(line[4:-1])
    return dll_list


def check_bit() -> int:
    with open("./dll_list.txt", 'r') as f:
        for line in f.readlines():
            if "32 bit word machine" in line:
                return 32
            if "64 bit word machine" in line:
                return 64


def move_32_bit_dll(dll_list: list, output_path: str) -> bool:
    count = 0
    dll_path = "C:/Windows/SysWOW64/"
    opencv_path = "D:/CPPLibs/opencv/build/x86/vc16/bin/"
    qt_path = "D:/APPs/Qt/5.15.2/msvc2019/bin/"
    move_platforms = False

    for dll in dll_list:
        if "api-ms-win-crt" == dll[:15]:
            continue
        if "opencv_world" == dll[:13] and os.path.isfile(opencv_path + dll):
            shutil.copy(opencv_path + dll, os.path.join(output_path, dll))
            count += 1
        elif os.path.isfile(dll_path + dll):
            shutil.copy(dll_path + dll, os.path.join(output_path, dll))
            count += 1
        elif "Qt5" == dll[:3] and os.path.isfile(qt_path + dll):
            shutil.copy(qt_path + dll, os.path.join(output_path, dll))
            count += 1
            if not move_platforms: move_platforms = True
            
    # shutil.copy(dll_path + "msiexec.exe", os.path.join(output_path, "msiexec.exe"))

    if move_platforms:
        platforms_path = "D:/APPs/Qt/5.15.2/msvc2019/plugins/platforms/"
        os.makedirs(os.path.join(output_path, "platforms"))
        for file in os.listdir(platforms_path):
            if ".dll" in file:
                shutil.copy(platforms_path + file, os.path.join(os.path.join(output_path, "platforms"), file))

    return count == len(dll_list)


def move_64_bit_dll(dll_list: list, output_path: str) -> bool:
    count = 0
    dll_path = "C:/Windows/System32/"
    dll_path_tools = "D:/APPs/VS/IDE/Common7/Tools/"
    opencv_path = "D:/CPPLibs/opencv460/opencv/build/x64/vc17/bin/"
    qt_path = "D:/APPs/Qt/6.4.2/msvc2019_64/bin/"
    move_platforms = False

    for dll in dll_list:
        if "api-ms-win-crt" == dll[:15]:
            continue
        if "opencv_world" == dll[:13] and os.path.isfile(opencv_path + dll):
            shutil.copy(opencv_path + dll, os.path.join(output_path, dll))
            count += 1
        elif os.path.isfile(dll_path + dll):
            shutil.copy(dll_path + dll, os.path.join(output_path, dll))
            count += 1
        elif os.path.isfile(dll_path_tools + dll):
            shutil.copy(dll_path_tools + dll, os.path.join(output_path, dll))
            count += 1
        elif "Qt" == dll[:2] and os.path.isfile(qt_path + dll):
            shutil.copy(qt_path + dll, os.path.join(output_path, dll))
            count += 1
            if not move_platforms: move_platforms = True

    # shutil.copy(dll_path + "msiexec.exe", os.path.join(output_path, "msiexec.exe"))

    if move_platforms:
        platforms_path = "D:/APPs/Qt/6.4.2/msvc2019_64/plugins/platforms/" 
        os.makedirs(os.path.join(output_path, "platforms"))
        for file in os.listdir(platforms_path):
            if ".dll" in file:
                shutil.copy(platforms_path + file, os.path.join(os.path.join(output_path, "platforms"), file))

    return count == len(dll_list)


def log(dll_list: list, output_path: str) -> None:
    ls0 = os.listdir(output_path)
    ls1 = [dll+"\n" for dll in dll_list if dll not in ls0]
    with open(os.path.join(output_path, "load_dll_log.txt"), 'x') as f:
        f.write("以下dll未被找到:\n")
        f.writelines(ls1)


if __name__ == "__main__":
    _, input_file, output_path = sys.argv
    os.system("D:/APPs/VS/IDE/VC/Tools/MSVC/14.29.30133/bin/Hostx64/x64/dumpbin /headers /dependents {} >./dll_list.txt".format(input_file))
    dll_list = load_dll()

    if check_bit() == 32:
        if not move_32_bit_dll(dll_list, output_path):
            log(dll_list, output_path)
    else:
        if not move_64_bit_dll(dll_list, output_path):
            log(dll_list, output_path)

    shutil.move("./dll_list.txt", os.path.join(output_path, "dll_list.txt"))