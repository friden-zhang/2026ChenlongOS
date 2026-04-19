# Lecture 0 实验记录：在 TPU-MLIR 中跑通 YOLOv5 ONNX 推理

## 1. 目标

1. 在仓库内置的 Dev Container + `uv` 环境里跑通官方 `yolov5s.onnx` 的最小推理例子。
2. 理清 `输入图片 -> ONNX 推理 -> 输出检测结果图` 这条链路。
3. 解释当前样例在 `dev container + uv` 下会看到的几条 warning，避免把“提示”误判成“失败”。

## 2. 仓库来源与本地改动

这份记录基于我的 TPU-MLIR fork：

```text
git@github.com:friden-zhang/tpu-mlir.git
```

为了让实验过程更适合本地开发和 VS Code Dev Container，我在当前仓库工作区里补了这些改动：

- 新增 `.devcontainer/`，提供 CPU / GPU 两套 Dev Container 配置
- 新增 `.devcontainer/setup.sh`，在容器首次创建时自动执行 `uv sync`
- 新增 `.devcontainer/.bashrc` 和 `.devcontainer/.zshrc`，让新终端自动激活 `/workspace/.venv` 并 `source /workspace/envsetup.sh`
- 新增 `.python-version`、`pyproject.toml` 和 `uv.lock`，把 Python 依赖切到 `uv` 管理
- 更新 `README.md`、`README_cn.md` 和 `docs/quick_start/source_{zh,en}/02_env.rst`，把 Dev Container + `uv` 写成推荐开发流程
- 更新 `.gitignore`，忽略 `.venv/`

因此，下面的命令说明默认针对这个已经做过 Dev Container + `uv` 适配的仓库，而不是未修改的原始环境说明。

## 3. 适用环境

这份记录基于仓库内置的 CPU Dev Container：

- 配置文件：`.devcontainer/default/devcontainer.json`
- 工作目录：`/workspace`
- Python 环境：`/workspace/.venv`
- 容器首次创建时会自动执行：`/setup.sh cpu`

在 VS Code 的 Dev Container 新终端里，仓库已经帮我们做了两件事：

1. 自动激活 `/workspace/.venv`
2. 自动 `source /workspace/envsetup.sh`

因此在这种终端里，可以直接运行文中的 `uv run ...` 命令。

如果你是在下面这些场景里执行命令：

- 新开的非交互 shell
- 自己写的脚本
- CI / task runner
- 不是从 Dev Container 自动初始化出来的终端

那么请先手工补上环境：

```bash
cd /workspace
source .venv/bin/activate
source ./envsetup.sh
```

这一点很重要。`uv run` 负责使用 `.venv`，但 `envsetup.sh` 负责补齐 `PYTHONPATH`、`PATH` 和 TPU-MLIR 相关环境变量。


## 4. 首次准备

如果仓库刚打开，或者想手工确认一次环境，可以执行：

```bash
cd /workspace
uv sync --frozen
./build.sh
```

说明如下：

- `uv sync --frozen`：按照 `uv.lock` 创建并同步仓库依赖
- `./build.sh`：为后续 `model_transform.py`、`model_deploy.py` 等步骤准备 `install/` 下的运行时和工具

对本文这一节的纯 ONNX 推理来说，核心依赖其实是 `.venv`、`PYTHONPATH` 和 `onnxruntime`。

## 5. 直接复用仓库内样例

本实验直接使用仓库自带的模型和图片：

- 模型：`/workspace/regression/model/yolov5s.onnx`
- 图片：`/workspace/regression/image/dog.jpg`

不需要额外下载。

## 6. 运行 ONNX 推理

在 Dev Container 新终端里，从仓库根目录执行：

```bash
cd /workspace

uv run python python/samples/detect_yolov5.py \
  --input regression/image/dog.jpg \
  --model regression/model/yolov5s.onnx \
  --output /tmp/dog_onnx.jpg
```


参数含义：

- `--input`：输入图片
- `--model`：ONNX 模型
- `--output`：输出检测结果图

结果:

![](https://my-obsidian-vault-1419031144.cos.ap-beijing.myqcloud.com/images/20260419144348773.png)

