
### Not-Knob Controller Firmware

本仓库基于 **X-Knob** 项目改造，用作我的个人节奏游戏 **Not-Knob**（暂定名，见 [Lunar-525/not-knob](https://github.com/Lunar-525/not-knob)）的实体控制器固件。

原始 X-Knob 项目地址：<https://github.com/SmallPond/X-Knob>  
本仓库主要做与音乐游戏相关的适配和精简，保留 LVGL 旋钮 UI 与电机触觉反馈能力。

---

### 快速上手（开发者）

1. 使用 VSCode + PlatformIO 打开工程。  
2. 将 `src/secrets.h.example` 复制为 `src/secrets.h`，并根据实际环境修改 Wi-Fi / MQTT 等配置（如不需要 MQTT，可在 `platformio.ini` 中关闭相关宏）。  
~~3. 连接 ESP32-S3，编译并烧录固件，然后在 PC 上启动 Not-Knob 游戏进行联调。~~

更多底层硬件与 UI 实现细节请参考原项目 README：<https://github.com/SmallPond/X-Knob>。

---

### 协议与致谢

- 本仓库沿用原 X-Knob 的 MIT 相关约束及上游依赖协议，请在使用和分发时遵守相应开源许可。  
- 特别感谢 X-Knob 提供的基础代码。 
