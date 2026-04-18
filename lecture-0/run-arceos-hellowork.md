# Lecture 0 实验报告：运行 ArceOS Helloworld

## 1. 目的

1. 配置把 ArceOS 的基本开发环境。
2. 成功跑通官方的 `examples/helloworld`。
3. 理清编译运行流程

```bash
make A=examples/helloworld ARCH=aarch64 LOG=info SMP=4 run NET=y
```

## 2. 实验环境

目录结构比较简单，代码放在 `~/Code` 下面，ArceOS 仓库在：

```text
~/Code/arceos
```

我平时用的 shell 是 `fish`，所以后面配环境变量时我也是按 `fish` 的方式写的。

## 3. 实验准备

### 3.1 拉取源码

执行：

```bash
cd ~/Code
git clone https://github.com/arceos-org/arceos.git
cd arceos
```

后面所有 `make` 命令，默认都是在仓库根目录里执行的：

```bash
cd ~/Code/arceos
```

### 3.2 安装 Rust 相关工具

```bash
sudo apt update
sudo apt install curl build-essential -y
curl https://sh.rustup.rs -sSf | sh

cargo install cargo-binutils axconfig-gen cargo-axplat
```

仓库里的 `rust-toolchain.toml`，已经固定好了工具链版本，所以进入仓库后一般会自动切到对应的 nightly。

- `nightly-2025-05-20`
- `rust-src`
- `llvm-tools`
- `rustfmt`
- `clippy`

另外它还会用到下面这些 target：

- `x86_64-unknown-none`
- `riscv64gc-unknown-none-elf`
- `aarch64-unknown-none-softfloat`
- `loongarch64-unknown-none-softfloat`


### 3.3 安装 QEMU 和 Clang


```bash
sudo apt install qemu-system libclang-dev -y
```

### 3.4 安装 musl 交叉编译工具链

```bash
cd /path/to/toolchains

wget https://musl.cc/aarch64-linux-musl-cross.tgz
wget https://musl.cc/riscv64-linux-musl-cross.tgz
wget https://musl.cc/x86_64-linux-musl-cross.tgz
wget https://github.com/LoongsonLab/oscomp-toolchains-for-oskernel/releases/download/loongarch64-linux-musl-cross-gcc-13.2.0/loongarch64-linux-musl-cross.tgz

tar zxf aarch64-linux-musl-cross.tgz
tar zxf riscv64-linux-musl-cross.tgz
tar zxf x86_64-linux-musl-cross.tgz
tar zxf loongarch64-linux-musl-cross.tgz
```

因为我用的是 `fish`，把路径加到了 `~/.config/fish/config.fish`：

```fish
set TOOLCHAIN_ROOT /path/to/toolchains

fish_add_path $TOOLCHAIN_ROOT/x86_64-linux-musl-cross/bin
fish_add_path $TOOLCHAIN_ROOT/aarch64-linux-musl-cross/bin
fish_add_path $TOOLCHAIN_ROOT/riscv64-linux-musl-cross/bin
fish_add_path $TOOLCHAIN_ROOT/loongarch64-linux-musl-cross/bin
```

这里要注意，`/path/to/toolchains` 只是示例，实际要换成自己本机上的解压目录。

## 4. 运行前自检

```bash
rustc --version
cargo --version
cargo axplat --version
axconfig-gen --version
rust-objcopy --version
qemu-system-aarch64 --version
aarch64-linux-musl-gcc --version
```

## 5. 运行官方 helloworld

在 ArceOS 根目录执行了下面这条命令：

```bash
cd ~/Code/arceos
make A=examples/helloworld ARCH=aarch64 LOG=info SMP=4 run NET=y
```

### 5.1 这条命令里各个参数是什么意思

- `A=examples/helloworld`
  这个参数表示跑哪个应用。这里选的是官方自带的 `examples/helloworld`。
- `ARCH=aarch64`
  这个参数表示目标架构是 `aarch64`，也就是 ARM64。对这条命令来说，它最后会对应到 `aarch64-qemu-virt` 平台。
- `LOG=info`
  这个参数表示日志级别设成 `info`。
- `SMP=4`
  这个参数表示启动 4 个 CPU 核。
- `run`
  这不是参数，而是 `make` 的目标。它的意思不是只构建，而是“构建完以后直接运行”。
- `NET=y`
  这个参数表示给 QEMU 加一块虚拟网卡。

除了显式写出来的参数外，还有几个默认值：

- `MODE=release`
- `BUS=pci`
- `MEM=128M`
- `GRAPHIC=n`
- `NET_DEV=user`

### 5.2 `make` 之后到底发生了什么

这条命令背后大概做了下面几件事：

1. `make` 先读取顶层 `Makefile`，再去加载 `scripts/make/` 下面的一些辅助脚本。
2. 它会先检查依赖工具是不是齐的，比如 `cargo-axplat`、`axconfig-gen`、`cargo-binutils`。
3. 然后根据 `ARCH=aarch64` 去解析平台，最后得到的是 `aarch64-qemu-virt`。
4. 接着生成或者更新 `.axconfig.toml`。这里 `SMP=4` 会被写进配置里，对应 `plat.max-cpu-num=4`。
5. 然后开始解析 feature。对这次构建来说，主要会带上：
   - `axstd/defplat`
   - `axstd/log-level-info`
   - `axstd/smp`
6. 再往下就是实际的 Rust 编译，也就是调用 `cargo build` 去编 `examples/helloworld`。
7. 编译结束后，Makefile 会把生成出来的 ELF 文件复制到应用目录下，再用 `rust-objcopy` 把它转成 `.bin`。
8. 最后调用 `qemu-system-aarch64`，把这个 `.bin` 当作内核镜像启动起来。

如果把它近似展开成几个关键命令，大概长这样：

```bash
# 1. 生成/更新配置
axconfig-gen configs/defconfig.toml <platform-config> \
  -w 'arch="aarch64"' \
  -w 'platform="aarch64-qemu-virt"' \
  -w 'plat.max-cpu-num=4' \
  -o .axconfig.toml

# 2. 编译应用
cargo -C examples/helloworld build \
  -Z unstable-options \
  --target aarch64-unknown-none-softfloat \
  --target-dir target \
  --release \
  --features "axstd/defplat axstd/log-level-info axstd/smp"

# 3. 生成镜像
cp target/aarch64-unknown-none-softfloat/release/arceos-helloworld \
  examples/helloworld/helloworld_aarch64-qemu-virt.elf

rust-objcopy --binary-architecture=aarch64 \
  examples/helloworld/helloworld_aarch64-qemu-virt.elf \
  --strip-all -O binary \
  examples/helloworld/helloworld_aarch64-qemu-virt.bin

# 4. 用 QEMU 启动
qemu-system-aarch64 \
  -m 128M \
  -smp 4 \
  -cpu cortex-a72 \
  -machine virt \
  -kernel examples/helloworld/helloworld_aarch64-qemu-virt.bin \
  -device virtio-net-pci,netdev=net0 \
  -netdev user,id=net0,hostfwd=tcp::5555-:5555,hostfwd=udp::5555-:5555 \
  -nographic
```

另外，Makefile 在构建的时候还会额外设置一些环境变量，比如：

- `AX_ARCH=aarch64`
- `AX_PLATFORM=aarch64-qemu-virt`
- `AX_LOG=info`
- `AX_TARGET=aarch64-unknown-none-softfloat`
- `AX_CONFIG_PATH=.axconfig.toml`

这些变量会在编译 ArceOS 相关 crate 的时候被读取，所以它们其实也是整条构建链里的一部分。

### 5.3 运行结果

如果环境没有问题，最终终端里会看到类似下面的输出：

```text
Hello, world!
```

截图：

![](https://my-obsidian-vault-1419031144.cos.ap-beijing.myqcloud.com/images/20260418211433324.png)

## 6. 我做的一个小扩展

官方版 `helloworld` 很适合入门，基于此做了一个小拓展

```text
lecture-0/helloworld/src/main.rs
```

改了一个稍微有趣一点的版本，为了控制复杂度，依然只有一个 `main.rs`。

我加进去的内容主要有：

- 启动标题
- 当前架构、平台、日志级别显示
- 一个简单倒计时
- Fibonacci 数列输出
- 20 以内素数输出
- 一个文本进度条

替换官方 helloworld example
```bash
cp lecture-0/helloworld/src/main.rs \
   ~/Code/arceos/examples/helloworld/src/main.rs

cd ~/Code/arceos
make A=examples/helloworld ARCH=aarch64 LOG=info SMP=4 run NET=y
```

这样做的好处是最省事，因为原来的工作区、Makefile、平台配置和依赖关系都已经是通的，只需要替换掉 `main.rs` 就行。

运行以后，输出会比官方版丰富很多，大概像这样：

```text
=== ChenlongOS Mini Demo ===
Hello from ArceOS!
arch     : aarch64
platform : aarch64-qemu-virt
log      : info

countdown:
  T-3
  T-2
  T-1
  lift off!
```

![](https://my-obsidian-vault-1419031144.cos.ap-beijing.myqcloud.com/images/20260418212757249.png)