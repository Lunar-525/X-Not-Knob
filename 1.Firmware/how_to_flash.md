# 如何刷入固件

本文档用于说明如何将固件编译并通过USB上传到设备。主要步骤如下：

1. **安装 VS Code 和 PlatformIO 插件**

   - 如果还未安装，请从 [Visual Studio Code 官网](https://code.visualstudio.com/) 下载并安装 VS Code。
   - 打开1.FIRMWARE文件夹作为工作区，但是打开上级目录（X-Not-Knob）作为repository。
   - 进入扩展市场（大部分情况下右下角会提示是否自动安装 PlatformIO IDE），搜索并安装 `PlatformIO IDE` 插件。
   - 安装完成后，重启 VS Code。

2. **修改 PlatformIO 上传脚本以支持 USB 上传**

   默认情况下，PlatformIO 的 espressif32 平台使用串口在复位后上传固件。为了使用 USB 传输，需要更改：

   1. 打开文件 `C:\Users\<User name>\.platformio\platforms\espressif32\builder\main.py`。
   2. 找到类似下面的代码行：
      ```python
      "--before", board.get("upload.before_reset", "default_reset"),
      ```
   3. 将其替换为：
      ```python
      "--before", board.get("upload.before_reset", "usb_reset"),
      ```
   4. 保存文件。

   这个修改会让 esptool 在上传前执行 USB 重置，从而支持通过 USB 直接刷写。

3. **编译并上传固件**

   - 在 VS Code 中打开本工程的工作区。确保 `platformio.ini` 配置正确（你通常不需要做什么）。
   - 在编辑窗口的右上角，点击那个向右的箭头图标（或使用快捷键 `Ctrl+Alt+U`），PlatformIO 会先编译固件，然后自动上传。
   - 等待任务完成，终端会显示上传成功的信息。

> **注意**: 任何时候如果需要恢复原始行为，只需将 `upload.before_reset` 的 `usb_reset` 改回 `default_reset`。若你需要刷回原固件，请参见 https://github.com/SmallPond/X-Knob ，具体流程大同小异。