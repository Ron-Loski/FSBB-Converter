# FSBB-Converter

四开关升降压变换器（FSBB）的 STM32 工程与 Simulink 模型。

## 文件结构

- `Buck_Boost/`：完整 STM32CubeIDE 工程，包含源码、驱动、工程配置、链接脚本以及现有 `Debug/` 构建文件，共 223 个文件。
- `Simulink/FSBB_Converter.slx`：当前 Simulink 模型。
- `Simulink/backup/FSBB_Converter_original.slx`：原始模型备份。
- `Simulink/backup/双端10A旧版/FSBB_Converter.slx`：双端 10 A 旧版模型备份。

## 使用

在 STM32CubeIDE 中通过“Import → Existing Projects into Workspace”导入 `Buck_Boost/`，或使用 STM32CubeMX 打开 `Buck_Boost/Buck_Boost.ioc`。

在 MATLAB / Simulink 中打开 `Simulink/FSBB_Converter.slx`。本次归档仅收录 Simulink 模型文件，未包含原目录中的仿真结果、缓存和辅助 MATLAB 脚本。

本仓库为 2026-10-20 的文件快照；本次上传未重新编译固件或运行仿真。
