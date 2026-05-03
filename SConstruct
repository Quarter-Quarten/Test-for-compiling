#!/usr/bin/env python3
import os
import sys

# 获取 godot-cpp 的环境配置
env = SConscript("godot-cpp/SConstruct")

# 添加源文件路径
env.Append(CPPPATH=["src/"])

# 收集所有源文件 丑陋但能用
sources = Glob("src/*.cpp")
for i in range(1, 100):
    addition = Glob("src/" + "**/" * i + "*.cpp")
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
        
elif target_platform == "linux":
    # Linux 特定设置
    if env["target"] == "template_release":
        env.Append(CCFLAGS=["-O3", "-flto=auto"])
    else:
        env.Append(CCFLAGS=["-O0", "-g3"])
        
elif target_platform == "macos":
    # macOS 特定设置
    if env["target"] == "template_release":
        env.Append(CCFLAGS=["-O3", "-flto=auto"])
    else:
        env.Append(CCFLAGS=["-O0", "-g3"])
        
elif target_platform == "android":
    # Android 特定设置
    env.Append(CCFLAGS=["-fPIC"])
    
elif target_platform == "ios":
    # iOS 特定设置
    env.Append(CCFLAGS=["-fPIC"])
    
elif target_platform == "web":
    # Web/Emscripten 特定设置
    env.Append(CCFLAGS=["-s", "SIDE_MODULE=1"])

# 构建库文件
# 根据平台和配置生成对应的文件名
library_name = "bin/libopentower"
library_name += env["suffix"]           # 添加目标后缀 (template_debug/release)
library_name += env["SHLIBSUFFIX"]       # 添加平台后缀 (.dll/.so/.dylib)

# 对于 macOS framework，特殊处理
if target_platform == "macos" and env.get("macos_create_framework", False):
    # 可选：创建 framework bundle
    framework_path = "bin/libopentower{}.framework".format(env["suffix"])
    library = env.SharedLibrary(
        framework_path + "/libopentower{}".format(env["suffix"]),
        source=sources
    )
else:
    library = env.SharedLibrary(library_name, source=sources)

# 设置默认目标
Default(library)

# 辅助函数：清理不同平台的构建文件
def clean_all(target, source, env):
    """清理所有平台的构建产物"""
    platforms = ["windows", "linux", "macos", "android", "ios", "web"]
    configs = ["template_debug", "template_release"]
    
    for platform in platforms:
        for config in configs:
            build_cmd = "scons platform={} target={} -c".format(platform, config)
            print("Cleaning: {}".format(build_cmd))
            os.system(build_cmd)

# 添加清理别名
env.Alias("clean_all", "", clean_all)

# 打印构建信息
print("=" * 50)
print("Building GDExtension for platform: {}".format(target_platform))
print("Configuration: {}".format(env.get("target", "default")))
print("Output: {}".format(library_name))
print("=" * 50)


# 在 SConstruct 文件末尾添加

from SCons.Script import AddOption, GetOption

# 添加批量构建选项
AddOption('--all', dest='all', action='store_true', 
          help='Build for all supported platforms')

if GetOption('all'):
    platforms = [
        ('windows', 'x86_64'),
        ('linux', 'x86_64'),
        ('macos', 'x86_64'),
        ('macos', 'arm64'),
        ('android', 'x86_64'),
        ('android', 'arm64'),
        ('ios', 'arm64'),
        ('web', 'wasm32')
    ]
    
    for platform, arch in platforms:
        cmd = f"scons platform={platform} arch={arch} target={env['target']}"
        # 眼睛比较瞎看不到这玩意所以多打印几次
        for i in range(3):
            print(f"Building: {cmd}")
        os.system(cmd)
    
    sys.exit(0)