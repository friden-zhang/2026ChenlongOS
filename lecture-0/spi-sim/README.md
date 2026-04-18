# Lecture 0 实验记录：SPI 屏幕仿真

这次我在 `lecture-0/spi-sim` 里做了一个很小的 SPI 仿真实验：用 `Arduino Uno` 通过 `SPI` 驱动一块 `ILI9341` TFT 屏幕，在 `PlatformIO + Wokwi` 里直接跑通。

## 1. 这次实验做了什么

这个目录里只有一套很直接的内容：

- `src/main.cpp`
  初始化屏幕，然后持续往屏幕刷内容
- `diagram.json`
  Wokwi 里的电路连线
- `wokwi.toml`
  告诉 Wokwi 去哪里加载 PlatformIO 生成的固件
- `platformio.ini`
  板子配置和库依赖

我这次没有再做“双 MCU 主从”的仿真，而是改成了更贴近常见开发场景的版本：一个 `Uno` 当主控，通过 SPI 连一块屏幕。

## 2. 接线

我这里用的是 `ILI9341`，连线很简单：

- `5V -> VCC`
- `GND -> GND`
- `D10 -> CS`
- `D9 -> D/C`
- `D11 -> MOSI`
- `D13 -> SCK`

这次没有接 `MISO`，因为这里只需要把数据从 `Uno` 发到屏幕，不需要从屏幕读回数据。

## 3. 代码里实际干了什么

`main.cpp` 里主要做了几件事：

1. 通过 `Adafruit_ILI9341` 初始化屏幕。
2. 把屏幕旋转成横屏显示。
3. 先画一套静态界面，包括标题、接线说明和显示区域。
4. 在 `loop()` 里周期性刷新进度条、颜色条、帧计数和运行时间。

这样做的目的很简单，就是想让屏幕上一直有可见变化，能比较直观地说明：`Uno` 正在持续通过 `SPI` 往 `ILI9341` 里写像素数据。

## 4. 我是怎么跑这个实验的

先进入目录编译：

```bash
cd lecture-0/spi-sim
pio run
```

然后在 VS Code 里打开 `diagram.json`，执行 `Wokwi: Start Simulator`。

如果一切正常，终端里会先打印：

```text
=== SPI TFT Demo ===
board : Arduino Uno
screen: ILI9341 TFT
pins  : CS=D10 DC=D9 MOSI=D11 SCK=D13
display initialized
```

同时屏幕上会看到一套彩色界面，并且下面这些内容会持续变化：

- 进度条
- 颜色扫带
- 帧计数
- 运行时间

