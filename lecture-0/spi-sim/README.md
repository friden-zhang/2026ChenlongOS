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
3. 用几条很短的绘图指令发出一小段有限的 SPI 传输。
4. 传输结束后，程序进入基本空转状态，不再持续刷屏。

这样做的目的很简单，就是让逻辑分析仪只抓一小段清楚的 SPI 波形，而不是整屏刷新的大流量数据。

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
capture sequence finished
logic analyzer trigger returned low
loop() stays idle so stopping the simulator is easier
```

同时屏幕上只会出现一小段很轻量的绘制结果：

- 几个彩色像素点
- 一条短横线
- 一个小矩形

## 5. 如果我想看 SPI 波形

我已经把 `Wokwi Logic Analyzer` 直接接进这个项目里了，对应关系是：

- `D0 -> SCK`
- `D1 -> MOSI`
- `D2 -> CS`
- `D3 -> D/C`

这里我还顺手配了一个比较实用的触发条件：

- `triggerMode = level`
- `triggerPin = D7`
- `triggerLevel = high`

这里的 `D7` 不是屏幕连线的一部分，而是我额外拿来做逻辑分析仪触发的辅助脚。程序会在真正想抓的那一小段 SPI 传输开始前，把 `D7` 拉高；传输结束后，再把它拉低。

另外，逻辑分析仪缓冲区我也主动调小到了一个更适合教学观察的范围，所以现在更偏向“抓一小段清晰波形”，而不是“长时间录整段显示流量”。

停止仿真以后，项目根目录下会生成一个波形文件：

```text
lecture-0/spi-sim/spi-capture.vcd
```

这个文件可以用 `PulseView`、`GTKWave` 或 `Surfer` 打开。

如果我想在 `PulseView` 里直接解码成 SPI，可以把通道这样对应：

- `CLK = D0`
- `MOSI = D1`
- `CS# = D2`

`D/C` 不是 SPI 四线里的标准信号，但对屏幕很重要，因为它可以帮助区分当前传的是命令还是显示数据。
