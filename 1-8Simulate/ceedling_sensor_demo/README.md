# Ceedling 传感器状态判断 Demo

这是一个独立的原生 Windows C 单元测试项目，不依赖现有固件。业务代码读取压力传感器，并将结果判断为正常、达到阈值或传感器读取失败。

## 工具角色

- **Ruby**：Ceedling 的运行时环境。
- **Ceedling**：组织源码、生成测试构建并执行测试的构建框架。
- **Unity**：提供断言和 C 测试运行器。
- **CMock**：根据 `sensor_hal.h` 自动生成硬件接口 mock，用于设置预期调用、返回值和输出参数。

当前环境使用 Ruby `3.4.10`、RubyGems `3.6.9` 和 Ceedling `1.1.6`。Ceedling gem 自带 Unity `2.7.2` 与 CMock `2.7.2`。

## 目录结构

```text
ceedling_sensor_demo/
├── src/
│   ├── pressure_monitor.c    # 被测业务逻辑
│   ├── pressure_monitor.h    # 状态类型和业务接口
│   └── sensor_hal.h          # 硬件读取接口
├── test/
│   └── test_pressure_monitor.c
├── .gitignore
├── project.yml               # Ceedling 与 CMock 配置
├── run_tests.cmd             # 固定工具路径的测试入口
└── README.md
```

测试文件包含 `mock_sensor_hal.h`。Ceedling 在构建时识别该头文件，并让 CMock 从 `sensor_hal.h` 生成 mock；项目中没有手写硬件 stub。

## 在新 PowerShell 中运行

进入项目目录后执行：

```powershell
cd 'E:\Codes\CAndC++\1-8Simulate\ceedling_sensor_demo'
.\run_tests.cmd
```

脚本使用 Ruby 和 Ceedling 的绝对安装位置，并显式加入已安装的 GCC（GNU Compiler Collection）目录，因此不依赖当前 PowerShell 的临时 `PATH`。

也可以直接执行完整清理和测试：

```powershell
$env:PATH = 'E:\ProgramFiles\Ruby34\bin;E:\Program Files\msys64\ucrt64\bin;' + $env:PATH
& 'E:\ProgramFiles\Ruby34\bin\ceedling.bat' clobber
& 'E:\ProgramFiles\Ruby34\bin\ceedling.bat' test:all
```

RubyInstaller 可能已将 Ruby 写入用户 `PATH`；已有终端不会自动刷新该值，重新打开终端后才会生效。无论是否刷新，`run_tests.cmd` 都可直接运行。
