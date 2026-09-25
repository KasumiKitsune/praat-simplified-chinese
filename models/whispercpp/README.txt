【Whisper 语音识别模型放置目录】
将从 Hugging Face 或国内镜像下载的 ggml-*.bin 模型权重文件（例如 ggml-base.bin）放入本文件夹。
Praat 启动或运行语音识别时会自动扫描本目录，无需额外配置。

推荐模型：
- ggml-base.bin（约 142 MB，综合推荐，支持中文等多语言）
- ggml-small.bin（约 466 MB，更高识别准确率）
- ggml-tiny.bin（约 75 MB，轻量快捷）

模型下载地址：
- 官方源（需要国际网络支持）：https://huggingface.co/ggerganov/whisper.cpp/tree/main
- 国内镜像源：https://hf-mirror.com/ggerganov/whisper.cpp/tree/main
