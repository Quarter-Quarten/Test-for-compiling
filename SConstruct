#!/usr/bin/env python3
import os
import sys

# 获取 godot-cpp 的环境配置
env = SConscript("godot-cpp/SConstruct")



# 添加源文件路径
env.Append(CPPPATH=["src/"])

# 收集所有源文件 丑陋但能用
sources = Glob("src/*.cpp") + Glob("src/*.c")
for i in range(1, 100):
    addition = Glob("src/" + "**/" * i + "*.cpp") + Glob("src/" + "**/" * i + "*.c")
    sources += addition

# 获取目标平台（通过命令行参数或自动检测）
target_platform = ARGUMENTS.get("platform", sys.platform)

# 平台特定的构建优化设置
if target_platform == "windows":
    # Windows 特定设置
    if env["target"] == "template_release":
        env.Append(CCFLAGS=["/O2", "/MD"])
    else:
        env.Append(CCFLAGS=["/Zi", "/MDd"])
elif target_platform == "android":
    # 检查系统环境变量
    required_vars = ["ANDROID_NDK_ROOT", "ANDROID_SDK_ROOT", "JAVA_HOME"]
    for var in required_vars:
        val = os.getenv(var)
        if val is None:
            print(f"ERROR: {var} is not set!")
            Exit(1)
        env["ENV"][var] = val

    env.Append(CCFLAGS=["-fPIC"])   

# 构建库文件
# 根据平台和配置生成对应的文件名
library_name = "bin/libopentower"
library_name += env["suffix"]           # 添加目标后缀 (template_debug/release)
library_name += env["SHLIBSUFFIX"]       # 添加平台后缀 (.dll/.so/.dylib)

library = env.SharedLibrary(library_name, source=sources)

# 设置默认目标
Default(library)


# 打印构建信息
print("=" * 50)
print("Building GDExtension for platform: {}".format(target_platform))
print("Configuration: {}".format(env.get("target", "default")))
print("Output: {}".format(library_name))
print("=" * 50)
