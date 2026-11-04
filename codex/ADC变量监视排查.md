# rawPara / adcPara 在 CubeIDE 中不可评估

## 已确认的原因

原工程的 `.cproject` 在 Debug 和 Release 的 sourceEntries 中只列出 Core、Drivers，没有 Utils/Src。添加头文件搜索路径只负责查找头文件，不会让该目录的 C 文件参与编译。

`Debug/sources.mk` 和 `Debug/objects.list` 中也没有 Utils。因此定义变量的 `Utils/Src/Global_Var.c`、更新变量的 `Utils/Src/Callback.c` 都没有编译链接。

对原 `Debug/FSBB_Self.elf` 执行离线 GDB 查询，两个变量均返回：

```
No symbol "rawPara" in current context.
No symbol "adcPara" in current context.
```

原 map 显示 HAL_ADCEx_InjectedConvCpltCallback 来自 HAL 驱动 `stm32g4xx_hal_adc_ex.o`，使用的是默认回调，并非 Utils 中的实现。

现有 Debug 编译参数是 `-O0 -g3`，本次已确认的问题是源文件没有参与构建。

## 已实施修改

1. `.cproject`：Debug、Release 均添加 Utils/Src 源码目录；补充 Release 的 Utils/Inc 头文件路径。
2. `Utils/Inc/Global_Var.h`：包含 Global_Types.h，使 Paratyedef 在 extern 声明之前可见。
3. `Utils/Inc/Global_Inc.h`：包含 Global_Var.h，使 Callback.c 可见两个变量的 extern 声明。

上述头文件依赖缺失会在启用 Utils 编译后暴露为编译错误，已一并修复。原文件备份在 codex/backup。

## 验证结果

使用本机 CubeIDE 自带 GNU Tools for STM32 14.3.rel1，按原 Debug 的源文件清单加 Utils/Src 所有 C 文件，以 `-O0 -g3` 完整重新编译，并使用原 FLASH 链接脚本链接成功。所有验证产物保存在 codex/verify-build，日志在 codex/verification.log，复现脚本为 codex/verify-debug-symbols.ps1。

离线 GDB 成功查到：

| 符号 | 本次验证 ELF 地址 |
| --- | --- |
| rawPara | 0x2000022c |
| adcPara | 0x2000023c |
| adcCount | 0x20000228 |
| HAL_ADCEx_InjectedConvCpltCallback | 0x08006dd4 |

两个变量均能解析出 InputVol、OutVol、InputCurr、OutCurr 四个 float 成员。回调的调试行号指向 Utils/Src/Callback.c，确认用户实现已参与链接。地址可能随下一次构建变化，不应硬编码。

本次没有覆盖 Debug 中的旧 ELF，没有连接或下载硬件，也没有验证板上实时采样。

## CubeIDE 中接下来执行

1. 结束当前调试会话。重新打开工程以加载外部修改的 `.cproject`（必要时关闭并重新启动 IDE）。
2. 选择 Debug 构建配置，执行 Project → Clean，再 Build Project。
3. 在构建日志中确认出现 Utils/Src/Global_Var.c 和 Utils/Src/Callback.c 的编译命令。
4. 使用原 `FSBB_Self Debug` 配置重新启动调试，它加载的是 Debug/FSBB_Self.elf。必须重新构建，当前 Debug 中仍是旧程序。
5. 暂停 CPU 后，在 Expressions 添加 rawPara、adcPara，或 rawPara.InputVol、adcPara.InputVol。若需要运行时查看，可在 Live Expressions 中添加各成员。

不要仅对旧 Debug/makefile 执行 make 来期望包含新源文件；先让 CubeIDE 根据更新的工程配置重新生成构建文件。

## 与“不可评估”分开的采样问题

main() 当前完成 ADC 初始化并启动 HRTIM 主计数器，但没有调用 HAL_ADCEx_InjectedStart_IT(&hadc2)。只初始化 ADC、启动触发定时器，不等于已经启动 ADC 注入组中断采样。因此即使变量现在可见，仍可能保持零。

后续若要启用采样，应结合硬件状态在 HRTIM 触发前完成 ADC 校准及 HAL_ADCEx_InjectedStart_IT，并检查返回值，再通过回调断点或 adcCount 确认中断发生。本次未更改采样启动流程。

adcPara 当前计算为 rawPara / 4095.0f，是归一化结果，尚未换算为实际电压或电流。
