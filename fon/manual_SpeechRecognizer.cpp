/* manual_SpeechRecognizer.cpp
 *
 * Copyright (C) 2026 Anastasia Shchupak
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this work. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ManPagesM.h"

void manual_SpeechRecognizer_init (ManPages me);
void manual_SpeechRecognizer_init (ManPages me) {

MAN_PAGES_BEGIN R"~~~(
################################################################################
"Speech recognition"
© Anastasia Shchupak 2026-06-01

Praat 中提供了两套语音识别工具：自动语音转写（将语音转换为文字）与自动说话人分离/日志（识别谁在何时说话）。本教程将介绍如何在 Praat 中使用这些工具。阅读前假定您已熟悉 @Intro，尤其是 @@Intro 7. Annotation|Intro 7. 标注@。

转写功能通过 @@whisper.cpp@ 实现，需要计算机中至少安装一个外部模型；说话人分离功能则基于改编的 @@pyannote.audio@ 分离流水线，内置集成，无需任何外部模型。说话人分离还可以与转写协同运行，将转写出的文字自动归属划分至检测到的各个说话人。

在阅读完 ##1. 自动转写# 和 ##2. 自动说话人分离# 两章后，如果您希望提升这些工具在 Praat 中的运行性能，可以在 ##3. 性能优化# 中找到详尽建议。

1. 自动转写
==========
Praat 中的转写可以将 @Sound 对象中的语音转换为文本。通常您可能希望将转写文字填入 @TextGrid 中，使其与声音在时间轴上精确对齐（参见 ##1.2. 转写到 TextGrid#）。但您也可以直接获取纯文本结果（参见 ##1.3. 转写为纯文本#）。无论哪种方式，首先都需要安装 Whisper 模型，具体方法见 ##1.1. 安装 Whisper 模型#。

1.1. 安装 Whisper 模型
======================
要进行语音转写，计算机中必须至少安装一个 Whisper 模型。这里的“安装”，是指下载模型文件并将其放入特定的文件夹中，以便 Praat 能够找到它。

在 Praat 中有两种放置模型的方式：
1. @@preferences folder|首选项文件夹@：在此文件夹中创建名为 `models` 的子文件夹，并在其内创建名为 `whispercpp` 的文件夹。在 Windows 上，该路径通常形如：
`C:\Users\您的用户名\Praat\models\whispercpp`
2. 便携模式推荐：直接在 Praat 程序同级目录下创建 `models\whispercpp` 文件夹（即 `<Praat程序目录>\models\whispercpp\`），解压缩即用。
在 Praat 中，您还可以随时通过菜单 ##对象 -> 新建 -> 下载 Whisper 模型...# 打开向导，一键直达模型文件夹或下载推荐模型。

模型可以从官方源 `https://huggingface.co/ggerganov/whisper.cpp` 下载（国内用户亦可使用镜像源 `https://hf-mirror.com/ggerganov/whisper.cpp`）。该页面提供了 whisper.cpp 所需的 ggml 格式模型文件（扩展名为 `.bin`）。请注意，OpenAI 官方发布的原始 Whisper 模型为 PyTorch 格式（`.pt` 文件），无法在 Praat 中直接使用，必须转换为 ggml 格式；上述页面中的文件即为转换好的模型。

可用模型列表很长，您可能会困惑该如何选择。建议针对具体的任务在速度与准确率之间进行权衡尝试，以下是快速选型概览：

模型名称中带有 `.en` 的为纯英文模型（例如 `ggml-small.en.bin`）；其他所有模型均为多语言模型（包括中文）。
在尺寸上，模型涵盖了从 `tiny`（约 75 MB）、`base`（约 142 MB）、`small`（约 466 MB）、`medium`（约 1.5 GB）到 `large`（约 2.9 GB）。其中 large 模型有三个不同版本：`large-v1`、`large-v2` 和 `large-v3`，后续版本在前序版本基础上有所改进。总体而言，模型越大准确率越高，但占用磁盘和内存更多，转写耗时也更长。对于中文日常使用，`base` 或 `small` 是非常理想的起点。

名称以 `-q5_0`、`-q5_1` 或 `-q8_0` 结尾的模型属于%量化模型%：其权重使用更少的位数存储（5 位或 8 位，而非标准的 16 位），占用磁盘与内存更小，运行速度比同名未量化模型更快。此外还有 `large-v3-turbo`，这是 `large-v3` 的精简版，减少了解码器层数，尺寸约为原版的一半，速度明显更快。这些优化变体模型旨在以轻微牺牲准确率为代价换取更高的运行速度。

确定所需的模型后，下载其 `.bin` 文件并放入 `whispercpp` 文件夹中。放置在该文件夹内的任何 `.bin` 文件都会被 Praat 自动扫描并加载。如果您想对比不同模型的效果，可以下载多个模型，Praat 在每次执行转写前都会提供下拉菜单供您选用。

1.2. 转写到 TextGrid
====================
1.2.1. 如何开始
===============
您需要一个 @Sound 和与该声音相配的 @TextGrid（转写会修改已有的 TextGrid，而不是创建新的）。该 TextGrid 应当至少包含一个区间层（interval tier）：转写针对选定的一个区间运行，因此需要一个区间层来承载该区间。如果要对整段声音进行转写，可以使用一个没有任何内部边界的层（即仅包含一个跨越整个声音时长的区间 1）。

启动转写到 TextGrid 有两种途径：
1. 在 @TextGridEditor 编辑窗口中：在 #Interval（区间）菜单中选择 ##Transcribe interval#（转写区间）；您可以在 @@Objects window|对象窗口@ 中同时选定 TextGrid 和 Sound，然后点击 @@Dynamic menu|动态菜单@ 中的 ##View & Edit# 进入 TextGridEditor；
2. 直接在对象窗口中：同时选定 TextGrid 和 Sound，然后点击 @@TextGrid & Sound: Transcribe interval...|TextGrid & Sound: 转写区间...@。

这两种途径的效果完全一致，选用哪一种取决于个人操作习惯。区别仅在于选择转写区间的方式（参见 ##1.2.4. 开始转写！#），以及所配置的转写设置是否跨 Praat 会话保持（详见下文两段）。

从 TextGridEditor 执行转写时，首先需要通过独立的 ##Transcription settings...#（转写设置...，在 #Interval 菜单中）调整参数。这些设置在多次转写及不同的 Praat 会话之间均会持久保存，因此后续再次转写时若沿用上次设置可直接跳过此步。注意，其中的 ##Diarization...#（说话人分离）设置块也可以通过 ##Diarization settings...#（同样位于 #Interval 菜单中）进行配置，供独立说话人分离使用（参见 ##2. 自动说话人分离#）。这意味着在任一处修改说话人分离设置，都会同时影响带分离的转写和独立分离。

如果直接在对象窗口运行转写，所有的转写设置都会显示在 ##Transcribe interval...# 弹出的命令窗口中。在此窗口中，设置会在单次 Praat 运行期间跨转写保留，但退出 Praat 后不会持久保存。

1.2.2. 如何配置设置
===================
设置包含三个模块：##Transcription...#（转写）、##Non-speech detection...#（非语音检测）和 ##Diarization...#（说话人分离）。后两项是转写流程中的可选步骤，均可通过各自模块中的第一个复选框开关。

##Transcription...# 模块有三个设置项：##Whisper model#（Whisper 模型）、#Language（语言）和 ##Include words#（包含词级对齐）。

##Whisper model# 列出了您在 ##1.1. 安装 Whisper 模型# 中安装的所有模型；此时请选择要使用的模型。如果您在此步骤发现模型列表为空，说明模型安装未成功，请返回 ##1.1. 安装 Whisper 模型# 检查下载的模型文件是否放置在正确的文件夹中。确认无误后重新打开设置窗口，模型便会出现在列表中。

#Language 是待识别的语言；您可以从全部支持的语言列表中选择，或保留默认的 ##Autodetect language#（自动检测语言）。注意：纯英文模型只能选择 #English 或默认的自动检测。

第三项转写设置是 ##Include words#。勾选此项后，TextGrid 中将在句子层下方额外添加一个词层（word tier），每个词单独对应一个区间。如果同时启用了说话人分离（如下所述），每个说话人还将拥有各自独立的词层。下方的示例 2 和示例 4 分别展示了未启用与启用说话人分离时勾选 ##Include words# 生成的 TextGrid。

##Non-speech detection...# 通过首项 ##Detect non-speech#（检测非语音）控制开关：开启后，声音在送入 Whisper 模型前会先剔除非语音静音部分。这不仅能大幅加快转写速度，还能防止模型在无声或纯噪声段“幻觉”捏造文字。使用非语音检测的另一个好处是使句子和词的边界更加精准；若不开启，文字可能会不自然地拉伸覆盖整个静音段。影响非语音检测的各项参数详见 @@speech activity detection with Silero VAD@。

##Diarization...# 通过首项 ##Include diarization#（包含说话人分离）控制开关：开启后，系统在转写的同时对声音执行说话人分离（独立于转写运行），随后将转写与分离结果合并：转写文本将按检测到的说话人分别归入各自专属的层。说话人分离的具体参数说明见 @@speaker diarization with adapted pyannote.audio@ 及 ##2.2. 如何配置设置#。

1.2.3. 示例
===========
生成的 TextGrid 结构取决于 ##Include words# 和 ##Include diarization# 设置的组合。以下四个示例展示了当转写所选区间跨越整个 “Mary” 层时，这两种设置的不同组合产生的 TextGrid 结果。

##示例 1#：同时关闭 ##Include words# 和 ##Include diarization#。选定的转写区间被切分为若干区间：每个句子对应一个包含句子文本的区间，非语音段为空白区间。句子边界由 whisper.cpp 返回的标点符号决定。不插入任何新层。
{- 6.0x3.0
	tierName$ = "Mary"
	textgrid = Create TextGrid: 0, 11, tierName$, ""
	Insert boundary: 1, 0.8
	Insert boundary: 1, 8.4
	Insert boundary: 1, 8.5
	Insert boundary: 1, 9.5
	Set interval text: 1, 2, "I start the sentence and I finish it."
	Set interval text: 1, 4, "Right."
	Draw: 0.0, 0.0, 1, 1, 1
	Axes: 0, 100, 0, 7
	One mark right: 0.7, 0, 0, 0, tierName$
	selectObject: textgrid
	Remove
}

##示例 2#：开启 ##Include words#，关闭 ##Include diarization#。原始区间如同##示例 1#一样被划分为句子，此外在 “Mary” 下方紧邻添加名为 “Mary/word” 的词层，每个词对应一个区间。
{- 6.0x3.0
	tier1Name$ = "Mary"
	tier2Name$ = "Mary/word"
	textgrid = Create TextGrid: 0, 11, tier1Name$ + " " + tier2Name$, ""
	Insert boundary: 1, 0.8
	Insert boundary: 1, 8.4
	Insert boundary: 1, 8.5
	Insert boundary: 1, 9.5
	Insert boundary: 2, 0.8
	Insert boundary: 2, 1.6
	Insert boundary: 2, 2.3
	Insert boundary: 2, 2.7
	Insert boundary: 2, 3.2
	Insert boundary: 2, 4.8
	Insert boundary: 2, 5.1
	Insert boundary: 2, 5.7
	Insert boundary: 2, 6.5
	Insert boundary: 2, 6.7
	Insert boundary: 2, 7.8
	Insert boundary: 2, 8.1
	Insert boundary: 2, 8.4
	Insert boundary: 2, 8.5
	Insert boundary: 2, 9.5
	Set interval text: 1, 2, "I start the sentence and I finish it."
	Set interval text: 1, 4, "Right."
	Set interval text: 2, 2, "I"
	Set interval text: 2, 3, "start"
	Set interval text: 2, 5, "the"
	Set interval text: 2, 6, "sentence"
	Set interval text: 2, 8, "and"
	Set interval text: 2, 10, "I"
	Set interval text: 2, 11, "finish"
	Set interval text: 2, 13, "it"
	Set interval text: 2, 15, "Right"
	Draw: 0.0, 0.0, 1, 1, 1
	Axes: 0, 100, 0, 7
	One mark right: 0.6, 0, 0, 0, tier2Name$
	One mark right: 1.7, 0, 0, 0, tier1Name$
	selectObject: textgrid
	Remove
}

##示例 3#：关闭 ##Include words#，开启 ##Include diarization#（且检测到至少两位说话人）。原层 “Mary” 被重命名为 “Mary/sp1”，并追加 “Mary/sp2”、“Mary/sp3” 等层，每个检测到的说话人各占一层。每个说话人的层包含该说话人所讲句子的区间。如果一个句子由一位说话人开始、另一位接续说完，该句子将被拆分分布在不同的说话人层中。
{- 6.0x3.0
	tier1Name$ = "Mary/sp1"
	tier2Name$ = "Mary/sp2"
	textgrid = Create TextGrid: 0, 11, tier1Name$ + " " + tier2Name$, ""
	Insert boundary: 1, 0.8
	Insert boundary: 1, 4.8
	Insert boundary: 2, 5.1
	Insert boundary: 2, 8.4
	Insert boundary: 1, 8.5
	Insert boundary: 1, 9.5
	Set interval text: 1, 2, "I start the sentence..."
	Set interval text: 2, 2, "... and I finish it."
	Set interval text: 1, 4, "Right."
	Draw: 0.0, 0.0, 1, 1, 1
	Axes: 0, 100, 0, 7
	One mark right: 0.6, 0, 0, 0, tier2Name$
	One mark right: 1.7, 0, 0, 0, tier1Name$
	selectObject: textgrid
	Remove
}

##示例 4#：同时开启 ##Include words# 和 ##Include diarization#。除了句子层外，每个说话人还会获得一个紧随在其句子层下方的词层（说话人 1 对应 “Mary/sp1/w”，说话人 2 对应 “Mary/sp2/w”，以此类推）。
{- 6.0x3.0
	tier1Name$ = "Mary/sp1"
	tier2Name$ = "Mary/sp1/w"
	tier3Name$ = "Mary/sp2"
	tier4Name$ = "Mary/sp2/w"
	textgrid = Create TextGrid: 0, 11, tier1Name$ + " " + tier2Name$ + " " + tier3Name$ + " " + tier4Name$, ""
	Insert boundary: 1, 0.8
	Insert boundary: 1, 4.8
	Insert boundary: 3, 5.1
	Insert boundary: 3, 8.4
	Insert boundary: 1, 8.5
	Insert boundary: 1, 9.5
	Set interval text: 1, 2, "I start the sentence..."
	Set interval text: 3, 2, "... and I finish it."
	Set interval text: 1, 4, "Right."
	Insert boundary: 2, 0.8
	Insert boundary: 2, 1.6
	Insert boundary: 2, 2.3
	Insert boundary: 2, 2.7
	Insert boundary: 2, 3.2
	Insert boundary: 2, 4.8
	Insert boundary: 2, 8.5
	Insert boundary: 2, 9.5
	Insert boundary: 4, 5.1
	Insert boundary: 4, 5.7
	Insert boundary: 4, 6.5
	Insert boundary: 4, 6.7
	Insert boundary: 4, 7.8
	Insert boundary: 4, 8.1
	Insert boundary: 4, 8.4
	Set interval text: 2, 2, "I"
	Set interval text: 2, 3, "start"
	Set interval text: 2, 5, "the"
	Set interval text: 2, 6, "sentence"
	Set interval text: 4, 2, "and"
	Set interval text: 4, 4, "I"
	Set interval text: 4, 5, "finish"
	Set interval text: 4, 7, "it"
	Set interval text: 2, 8, "Right"
	Draw: 0.0, 0.0, 1, 1, 1
	Axes: 0, 100, 0, 7
	One mark right: 0.45, 0, 0, 0, tier4Name$
	One mark right: 1.3, 0, 0, 0, tier3Name$
	One mark right: 2.15, 0, 0, 0, tier2Name$
	One mark right: 3, 0, 0, 0, tier1Name$
	selectObject: textgrid
	Remove
}

1.2.4. 开始转写！
=================
完成转写设置后，即可开始转写。

如果在 TextGridEditor 中转写，点击要转写的区间以选中它，然后在 #Interval 菜单中选择 ##Transcribe interval#。

如果在对象窗口中转写，在 ##Transcribe interval...# 命令窗口顶部的 ##Tier number#（层编号）和 ##Interval number#（区间编号）中指定要转写的区间。

无论哪种情况，请注意：无法对名称中已包含斜杠（/）的层上的区间执行转写，以防止派生层名称无限嵌套增长。

执行转写需要一定时间，具体耗时取决于所选区间的时长、使用的 Whisper 模型以及是否启用了说话人分离。如果您希望提高转写运行速度，请阅读 ##3. 性能优化#。

1.3. 转写为纯文本
=================
您还可以将整段 @Sound 直接转写为纯文本。为此您需要一个 @SpeechRecognizer 对象。在 @@Objects window|对象窗口@ 的 @@New menu|新建菜单@ 中选择 @@Create SpeechRecognizer...@ 即可创建一个。此时弹出的命令窗口包含两项设置：##Whisper model# 和 #Language，两者的说明详见 ##1.2.2. 如何配置设置#。点击 #确定 后，对象列表中将出现新的 #SpeechRecognizer 对象。

要执行转写，请同时选中该 SpeechRecognizer 对象和 Sound 对象，然后在 @@Dynamic menu|动态菜单@ 中选择 ##SpeechRecognizer & Sound: Transcribe#。转写结果将输出至 @@Info window|信息窗口@。注意：此处无法关闭非语音检测设置，@@speech activity detection with Silero VAD|Silero VAD@ 始终以默认参数保持开启。

2. 自动说话人分离
=================
Praat 中的说话人分离（Diarization）用于检测 @Sound 中的不同说话人。对于每一位检测到的说话人，它会识别出%%语音片段（speech segments）%，即该说话人发声的时间区间。说话人分离可以作为转写流程的一部分协同运行，也可以单独运行。无论何种方式，它都会修改现有的 @TextGrid，并为每个检测到的说话人生成一个单独的区间层。

说话人分离设置（参见 ##2.2. 如何配置设置#）在独立分离与转写协同分离之间通用。本章其余内容专门针对独立分离；协同转写的情况已在 ##1.2. 转写到 TextGrid# 中介绍。

2.1. 如何开始
=============
要执行说话人分离，需要一个 @Sound 和与该声音相配的 @TextGrid（分离操作修改现有 TextGrid，不新建 TextGrid）。TextGrid 应至少包含一个区间层：分离针对选定的单一区间运行，因此需要区间层承载该区间。若要对整段声音进行分离，可以使用一个没有内部边界、仅由跨越整个声音的区间 1 构成的层。

启动说话人分离有两种途径：
1. 在 @TextGridEditor 编辑窗口中：在 #Interval（区间）菜单中选择 ##Diarize interval#（分离区间说话人）；在对象窗口中同时选中 TextGrid 和 Sound 并点击动态菜单的 ##View & Edit# 即可打开编辑器；
2. 直接在对象窗口中：同时选定 TextGrid 和 Sound，然后选择 @@TextGrid & Sound: Diarize interval...|TextGrid & Sound: 分离区间说话人...@。

这两种途径的效果完全相同，选择哪种全凭个人喜好。区别仅在于选择目标区间的方式（参见 ##2.4. 开始分离说话人！#），以及设置是否跨 Praat 会话持久保留。

在 TextGridEditor 中运行时，首先需通过独立的 ##Diarization settings...# 命令（在 #Interval 菜单中）调整参数。这些参数在多次运行与跨会话间均会保留。由于这些设置与协同转写共享，因此如果您上次是在 ##Transcription settings...# 中修改的，运行前值得核对一下。

如果在对象窗口中运行，所有设置均在 ##Diarize interval...# 命令窗口中呈现，在当前 Praat 运行期间跨任务保留，但退出后不保存。

2.2. 如何配置设置
=================
##Non-speech interval label#（非语音区间标签）和 ##Speech interval label#（语音区间标签）分别指定输出 TextGrid 中分类为非语音和语音的区间文字标签。这两项仅影响视觉显示，不影响分离算法本身。

##Max. number of speakers (≥ 2)#（最大说话人数）与 ##Clustering threshold (0-2)#（聚类阈值）共同影响算法最终检测到的说话人数量。##Max. number of speakers (≥ 2)# 设定了说话人数量的上限；虽然没有对应的下限设置，但可以通过调低 ##Clustering threshold (0-2)# 来推高识别出的说话人数。如果您确切知道声音中有多少位说话人，可以按照以下建议提升分离质量：
1. 将 ##Max. number of speakers (≥ 2)# 设定为实际说话人数；
2. 在声音的局部片段上进行测试分离；
3. 如果检测出的说话人数少于实际人数，尝试降低 ##Clustering threshold (0-2)#（例如每次下调 0.1）。

重复步骤 2 和 3，直到算法检测出正确的说话人数。

当说话人的音色较为接近，或者录音环境噪声较大时，微调 ##Clustering threshold (0-2)# 尤为有效。如果您想了解聚类阈值的原理和作用机制，请阅读 @@speaker diarization with adapted pyannote.audio@ 的#算法 章节，特别是#聚类 部分。

##Allow speakers to overlap#（允许说话人重叠）：开启后，算法能够检测两位说话人同时发声的情况。注意：算法目前最多支持两位说话人重叠，不支持三人或以上同时重叠。

最后一项 ##Segmentation step (0-1)#（分割步长）用于在速度与准确率之间取得平衡，详见 ##3. 性能优化#。初次使用建议保持默认值 0.1。

2.3. 示例
=========
本示例展示了在与 ##1.2.3. 示例# 相同的声音上独立运行说话人分离的结果；选定的区间跨越整个 “Mary” 层。分离算法检测到两位说话人，因此 “Mary” 层被重命名为 “Mary/sp1”，并新增了第二层 “Mary/sp2”。每个说话人的层包含交替出现的非语音与语音区间，并分别标记为配置的标签（本例中非语音为空白，语音为 “speech”）。
{- 6.0x3.0
	tier1Name$ = "Mary/sp1"
	tier2Name$ = "Mary/sp2"
	textgrid = Create TextGrid: 0, 11, tier1Name$ + " " + tier2Name$, ""
	Insert boundary: 1, 0.8
	Insert boundary: 1, 4.8
	Insert boundary: 2, 5.1
	Insert boundary: 2, 8.4
	Insert boundary: 1, 8.5
	Insert boundary: 1, 9.5
	Set interval text: 1, 2, "speech"
	Set interval text: 2, 2, "speech"
	Set interval text: 1, 4, "speech"
	Draw: 0.0, 0.0, 1, 1, 1
	Axes: 0, 100, 0, 7
	One mark right: 0.6, 0, 0, 0, tier2Name$
	One mark right: 1.7, 0, 0, 0, tier1Name$
	selectObject: textgrid
	Remove
}

2.4. 开始分离说话人！
=====================
配置好设置后即可开始分离。

在 TextGridEditor 中：点击选中目标区间，在 #Interval 菜单中选择 ##Diarize interval#。

在对象窗口中：在 ##Diarize interval...# 命令窗口顶部的 ##Tier number# 和 ##Interval number# 中指定目标区间。

同样地，请注意：不能对名称中已带有斜杠的层执行分离，以防止派生层名称无节制增长。

说话人分离需要一定运行时间，取决于选定区间的长度及参数设置。如需加速，请参阅 ##3. 性能优化#。

3. 性能优化
===========
Praat 中的语音识别工具依赖神经网络模型，需要消耗大量计算资源。因此在处理大型语料库或长录音时，转写与分离的耗时可能成为瓶颈。本章提供了一些提升语音识别工具运行速度的实用建议。

3.1. AI 设置与线程数优化
========================
绝大多数现代计算机都配备了多个%%物理处理器（核心）%。每个物理核心可以处理一个或两个并发计算线程。计算机能够并发运行的最大线程总数即为其%%逻辑处理器%数量。

按常理推断，当 Praat 使用与计算机逻辑处理器相同数量的线程时速度最快。但在实际测试中，使用全部逻辑处理器线程在某些架构的电脑上反而会导致严重的性能下降（拥堵减速）。那么，究竟设置多少线程最佳？答案因各家处理器的硬件缓存与架构特性而异，难以预先定论。不过多方测试表明，使用可用逻辑处理器数量的**一半**是一个最稳妥的起点，既能充分发挥多核性能，又能避免极端拥堵，因此 Praat 将其作为转写与说话人分离的默认并发线程数。如果您希望进一步挖掘电脑性能，可以在 @@Praat menu|Praat 菜单@ 的 #Settings（设置）子菜单下的 ##AI settings...#（AI 设置...）中进行调整。但请记住：设置的线程数最好不要超过计算机物理拥有的逻辑处理器总数。

由于转写与说话人分离采用不同的模型架构，二者的并行化方式亦不同，因此最优线程数可能各不相同。建议分别进行调优测试：
- 调优转写：调整其 ##Max. number of threads#，并在**不包含**说话人分离的情况下测量转写耗时；
- 调优说话人分离：调整其 ##Max. number of threads#，并在独立运行分离时测量耗时。

3.2. 加快转写速度的其他方法
============================
Whisper 模型的选择对转写耗时有决定性影响。如果转写过慢，可以尝试切换到更小的模型或量化模型，各类模型的对比详见 ##1.1. 安装 Whisper 模型#。

开启 ##Detect non-speech#（非语音检测，见 ##1.2.2. 如何配置设置#）也能显著加速转写，尤其是对于包含较多静音停顿的声音文件。因为非语音段在送入 Whisper 前已被剔除，大幅缩短了模型实际推理的音频长度。强烈建议始终开启此项，因为它同时能提高词界与句界的对齐精度。

如果启用了转写协同说话人分离（勾选了 ##Include diarization#），总耗时还将受到说话人分离速度的影响。

3.3. 加快说话人分离速度的其他方法
==================================
##Segmentation step (0-1)#（分割步长，在 @@speaker diarization with adapted pyannote.audio@ 中有详细说明）控制连续 10 秒分析窗口之间的重叠程度，进而决定了分割模型需要处理的窗口总数。分割步长是两个相邻窗口起始点间距占窗口长度的比例，间距越小，重叠度越高。

例如，步长 0.1 对应的起始间距为 1 秒，两个相邻窗口有 90\% 的重叠。若将步长翻倍至 0.2，重叠率降至 80\%，模型所需处理的分析窗口总数减半，分离速度约提升一倍。若将步长增至 0.5，重叠率进一步降低，分离速度可加快约五倍。

然而速度提升的代价是准确率降低。在处理完全部分析窗口后，分离算法通过对所有覆盖窗口的模型预测结果取平均来重构全局结果。窗口重叠越少，参与平均的窗口数量越少，结果的准确率也随之降低。

如果您希望加快分离速度，可以尝试适当调大该值。但建议不要超过 0.5，以确保声音中的每个时刻至少落在两个分析窗口的覆盖范围内。

################################################################################
"speech activity detection with Silero VAD"
© Anastasia Shchupak 2026-06-01

Praat 使用了 @@whisper.cpp@ 对 @@Silero VAD@ 语音活动检测器的移植实现。
预训练的 Silero VAD 模型权重已转换为 ggml 格式并内置编译进 Praat 中，因此无需下载任何外部模型文件。声音在送入模型前会自动重采样至 16 kHz（Silero VAD 模型所要求的采样频率）。

用途
====
检测声音中哪些部分包含语音。输出为一个语音片段列表，每个片段由起始时间和结束时间定义。

算法
====
Silero VAD 模型以 512 个采样点（在 16 kHz 采样率下对应 32 毫秒）的固定帧长连续处理声音。对于每一帧，模型输出该帧包含语音的概率。基于该输出，按以下规则构建语音片段列表（此描述对应 whisper.cpp 中的实现，与 Silero 原生实现略有差异）：

当某一帧的语音概率超过 ##Speech probability threshold#（语音概率阈值）时，语音片段开始。只要语音概率保持在负阈值（Silero 源码中称为“negative threshold”，等于语音概率阈值减去 0.15）之上，片段就持续延伸。只有当概率持续低于负阈值达到 ##Min. gap between speech segments#（语音片段最小间隔）时，当前片段才告结束。片段结束时，唯有时长超过 ##Min. speech segment#（最小语音片段时长）的片段才会被保留，过短片段将被丢弃。

所有片段初建完成后，间隔小于 0.2 秒的相邻片段将被自动合并（此项固化在 whisper.cpp 内部，不可配置）。随后进行边界扩展填充：每个片段在首尾两侧各延伸 ##Padding around speech segments#（填充时长）。若填充导致两个片段重叠，则它们在中间点相接而不交叉。

最终输出整齐的语音片段列表。

设置
====
##Speech probability threshold (0-1)#（标准值：0.5）
:   决定语音检测器的灵敏度。数值越高，检测器越严格（灵敏度越低），即某一帧需要具备更高的语音概率才会被认定为语音片段，这能减少将非语音误报为语音的假阳性，但可能漏检弱语音；数值越低则越敏感。默认值 0.5 适合绝大多数应用场景。

##Min. gap between speech segments (s)#（标准值：0.1）
:   两个语音片段之间的最小间隔时长。如果语音内部的短暂停顿（如塞音闭塞段）导致语音被切碎成过多碎段，可适度调大此值。请注意：无论此项设为多少，间隔短于 0.2 秒的片段都会被自动合并。

##Min. speech segment (s)#（标准值：0.25）
:   语音片段的最小持续时长，短于该值的零碎片段将被丢弃。

##Padding around speech segments (s)#（标准值：0.0）
:   在每个检测到的语音片段前后两侧各延伸扩展的时长。如果发现语音的开头起音或末尾收音存在轻微被截断的情况，可适当增大此值。

在 Praat 中的可用性
===================
Silero VAD 语音活动检测在 Praat 中有两种使用方式：
- 作为转写的一部分协同运行：在转写前先行检测并剔除静音非语音区域（参见 @@transcription with whisper.cpp@）；
- 独立运行：直接为选定声音生成带有非语音和语音区间的全新 @TextGrid（参见 @@Sound: To TextGrid (speech activity, Silero)...@）。

################################################################################
"Sound: To TextGrid (speech activity, Silero)..."
© Anastasia Shchupak 2026-06-01

此命令为每个选定的 @Sound 对象创建一个包含单个区间层的 @TextGrid。
该区间层包含非语音和语音区间，其边界由 Silero VAD 模型判定（算法与设置详见 @@speech activity detection with Silero VAD@）。区间的文字标签由 ##Non-speech interval label# 和 ##Speech interval label# 设定。

设置
====
##Speech probability threshold (0-1)#（标准值：0.5）
:	参见 @@speech activity detection with Silero VAD@。

##Min. gap between speech segments (s)#（标准值：0.1）
:	参见 @@speech activity detection with Silero VAD@。

##Min. speech segment (s)#（标准值：0.25）
:	参见 @@speech activity detection with Silero VAD@。

##Padding around speech segments (s)#（标准值：0.0）
:	参见 @@speech activity detection with Silero VAD@。

##Non-speech interval label#（标准值：“”）
:	生成 TextGrid 中归类为非语音的区间的文字标签。

##Speech interval label#（标准值：“speech”）
:	生成 TextGrid 中归类为语音的区间的文字标签。

################################################################################
"transcription with whisper.cpp"
© Anastasia Shchupak 2026-06-01

Praat 可以使用 @@whisper.cpp@ 对声音进行自动语音转写。使用此功能前，计算机中必须安装至少一个 Whisper 模型。有关如何安装模型以及如何进行转写的详细信息，请参阅 @@Speech recognition@ 教程。声音在送入 whisper.cpp 转写前会自动重采样至 16 kHz（该模型期望的采样频率）。本页专门记录各项转写参数设置。

行为
====
当对 @TextGrid 的某个区间运行转写时，它会就地修改该 TextGrid：原有区间被切分，可能会新增或重命名层。具体的 TextGrid 结果结构取决于 ##Include words# 和 ##Include diarization# 的组合。关于不同设置组合下的详细输出格式，请参阅 @@Speech recognition@ 教程。

设置
====
##Whisper model#
:	选择要使用的 Whisper 模型。
	列表中包含在 Praat 便携目录（程序同级 `models\whispercpp`）或 @@preferences folder|首选项文件夹@ `models\whispercpp` 子文件夹中找到的所有 `.bin` 格式模型文件。模型安装详情请参阅 @@Speech recognition@ 教程。

##Language#（标准值：##Autodetect language#）
:	指定用于转写的语言。
	选择 ##Autodetect language# 让模型自动侦测语言；若已知说话语言，明确指定语言可显著提升识别准确率与抗噪能力。注意纯英文模型（文件名带 ##.en#）只能搭配 ##Autodetect language# 或 ##English# 使用。

##Include words#（标准：开启）
:	开启后，每个识别出的词都会获得起始和结束时间戳，使用 whisper.cpp 内置的动态时间规整（DTW）算法计算得到。

##Detect non-speech#（标准：开启）
:	开启后，在转写前先运行 @@speech activity detection with Silero VAD@ 识别语音区域，仅将有效语音区域送入 Whisper 模型。这不仅大幅加快转写速度（缩短实际送入模型的音频长度），还能有效防止模型在无声或背景噪声段“幻觉”虚构文字。

##Speech probability threshold (0-1)#（标准值：0.5）
:	参见 @@speech activity detection with Silero VAD@。

##Min. gap between speech segments (s)#（标准值：0.1）
:	参见 @@speech activity detection with Silero VAD@。

##Min. speech segment (s)#（标准值：0.25）
:	参见 @@speech activity detection with Silero VAD@。

##Padding around speech segments (s)#（标准值：0.0）
:	参见 @@speech activity detection with Silero VAD@。

##Include diarization#（标准：关闭）
:	开启后，在转写的同时执行说话人分离（参见 @@speaker diarization with adapted pyannote.audio@）。随后将两者的结果融合，将转写的语音文本精确归属到各个说话人。

##Max. number of speakers (≥ 2)#（标准值：2）
:	参见 @@speaker diarization with adapted pyannote.audio@。

##Allow speakers to overlap#（标准：开启）
:	参见 @@speaker diarization with adapted pyannote.audio@。

##Clustering threshold (0-2)#（标准值：0.7）
:	参见 @@speaker diarization with adapted pyannote.audio@。

##Segmentation step (0-1)#（标准值：0.1）
:	参见 @@speaker diarization with adapted pyannote.audio@。

在 Praat 中的可用性
===================
在 Praat 中使用 whisper.cpp 转写有两种方式：
- 同时选定 @Sound 与相对应的 @TextGrid，并选择 @@TextGrid & Sound: Transcribe interval...|转写区间...@；
- 在 @TextGridEditor 编辑窗口的 #Interval（区间）菜单中选择 ##Transcribe interval#。该命令的参数通过同菜单下的 ##Transcription settings...# 设置，并会在不同 Praat 会话间持久记忆。

################################################################################
"TextGrid & Sound: Transcribe interval..."
© Anastasia Shchupak 2026-06-01

此命令获取 @TextGrid 中指定的区间，对 @Sound 中相对应的音频片段进行语音转写，并将转写结果写回 TextGrid 中。

设置
====
##Tier number
:	包含待转写区间的层编号。

##Interval number
:	待转写的区间编号。

此对话框中的其余设置用于控制转写过程本身，其具体含义请参见 @@transcription with whisper.cpp@。

################################################################################
"speaker diarization with adapted pyannote.audio"
© Anastasia Shchupak 2026-06-01

说话人分离（Speaker diarization）用于检测声音中哪些部分包含语音，并将每个语音部分归属到一个或多个说话人。其输出为一个片段列表，每个片段由起始时间、结束时间和说话人标识符定义。说话人标识符为自然数（1, 2, 3, ...），大致按照说话人在声音中首次出现的先后顺序编号。

Praat 中的说话人分离始终通过为每个检测到的说话人生成一个单独的区间层来修改现有 TextGrid。它既可以作为转写的一部分协同运行（将转写文本分配到各说话人层），也可以独立运行（使每个说话人层包含标记为非语音或语音的区间）。详见 @@Speech recognition@ 教程。

Praat 使用 @@pyannote.audio@ 的 `pyannote/speaker-diarization-3.1` 流水线的 C++/ggml 改编版本执行说话人分离。该流水线采用的两个神经网络模型：`pyannote/segmentation-3.0`（用于语音分割）和 `wespeaker-voxceleb-resnet34-LM`（用于提取说话人嵌入向量，参见 @@WeSpeaker@）均已转换为 ggml 格式并内置编译于 Praat 中，无需下载外部模型文件。声音在处理前会自动重采样至 16 kHz（两模型所要求的采样频率）。

设置
====
##Max. number of speakers (≥ 2)#（标准值：2）
:	算法可输出的说话人数量上限，必须至少为 2。没有相对应的“最小说话人数”设置；如果希望促使算法区分出更多说话人，可以尝试调低 ##Clustering threshold#。

##Allow speakers to overlap#（标准：开启）
:   若开启，允许在同一时刻至多有两位说话人同时处于发声状态；若关闭，则每个时刻均严格归属于单一说话人（即根据分割模型在该时刻发声最活跃的那一位）。

##Clustering threshold (0-2)#（标准值：0.7）
:	控制算法检测出的说话人数量。阈值越低产生的说话人越多（但无论如何不超过 ##Max. number of speakers (≥ 2)#）；阈值越高产生的说话人越少。此设置是调高检测说话人数的关键旋钮，若检测出的说话人数少于实际人数，可考虑降低此值。

##Segmentation step (0-1)#（标准值：0.1）
:   相邻重叠分析窗口起始点之间的间距，以分析窗口长度的比例表示。声音被切分为时长 10 秒的重叠片段（称为%%分析窗口%）；由于存在重叠，时间轴上的每个时刻都会被多个分析窗口覆盖。较小的分割步长意味着更多重叠分析窗口，通常更精确但耗时更长；较大的分割步长使得分析窗口更少、重叠度更低，速度更快但精度会有所下降。若希望加快分离速度可适度增大该值，但建议保持在 0.5 以下，确保每个时刻至少被两个分析窗口覆盖。

算法
====
该算法是 @@pyannote.audio@ 的 `pyannote/speaker-diarization-3.1` 流水线（参见 @@Bredin (2023)@）经过改编的 C++/ggml 移植版，分为四个阶段：

##1. 分割（Segmentation）#。声音被切分为相互重叠的 10 秒%%分析窗口%。两个连续窗口起始点之间的间距由 ##Segmentation step (0-1)#（以窗口长度的比例）决定。例如，步长 0.1 对应间距 1 秒，相邻窗口有 90\% 的重叠。

每个分析窗口中的音频被送入%%分割模型%，该模型将其划分为 589 帧（每帧约 17 毫秒），并按如下方式为每帧标注标签：
- 模型假定在一个分析窗口中至多有三位说话人，且每帧中至多两位同时发声；
- 对每一帧，模型计算活跃说话人下列组合的 7 维概率向量：{}、{1}、{2}、{3}、{1, 2}、{1, 3}、{2, 3}；
- 取概率最高的组合作为该帧的标签。

说话人编号 1、2、3 仅在当前分析窗口局部有效：一个窗口中的说话人 1 与另一个窗口中的说话人 1 不一定是同一人。将窗口局部的说话人映射到全局说话人正是阶段 2 和阶段 3 的任务。

##2. 说话人嵌入向量（Speaker embeddings）#。%嵌入（Embedding）%是机器学习中借用自数学的术语。在机器学习中，%嵌入%是将某个对象表示为高维空间中的一个向量（或空间中的一个点），其映射规则使得相似对象在空间中彼此靠近。对象因此被“嵌入”到了高维空间中，所得向量本身亦称为%嵌入向量%。什么是“对象”以及何为“相似”取决于具体任务；在说话人分离中，对象是声音片段，若两段声音是由同一位说话人讲的，则视它们为相似。

上一阶段确定了每个分析窗口中哪位说话人在哪些帧中发声。现在，针对每个窗口中的每位局部说话人（1、2 或 3），找出该说话人发声的所有帧。例如，阶段 1 中组合 {1}、{1, 2} 或 {1, 3} 中至少有一项概率非零的帧即为说话人 1 活跃的帧。随后，对窗口中发声的每位说话人，将其活跃的所有帧拼接成一段音频并送入%%嵌入模型%。该模型为每段音频生成一个 256 维的%嵌入向量%，代表该特定分析窗口中的该位特定说话人。来自不同分析窗口但属于同一说话人的嵌入向量在高维空间中往往彼此更近，从而为下一步的聚类奠定了基础。

##3. 聚类（Clustering）#。上一阶段为每个分析窗口中的每位活跃说话人生成了一个嵌入向量（每个窗口最多 3 个）。拥有足够多无重叠语音支撑的嵌入向量被视为“可靠向量”并参与聚类，其他向量暂时搁置。

来自所有分析窗口的可靠嵌入向量首先经过 L2 归一化，使其全部位于 256 维单位超球面上。随后使用%%质心链接的层次凝聚聚类（agglomerative hierarchical clustering with centroid linkage）%进行分组。初始时每个嵌入向量各自成一组（质心即为该向量本身）。在每一步中，质心距离最近的两组（以欧氏距离度量）被合并，新组的质心为该组内所有嵌入向量的均值。由于新质心未经 L2 归一化，合并组的质心会落在 256 维超球面的内部，每次合并后模长都会略微缩小。当接下来两个最近组之间的距离超过 ##Clustering threshold (0-2)# 时，聚类过程终止。

因为原始向量已作 L2 归一化，任意两个向量间的距离都在 0（完全相同）到 2（完全相反）之间，这正是聚类阈值取值范围的由来。

最终形成的每一个组对应一位全局说话人；如果达到阈值后组数仍然多于 ##Max. number of speakers (≥ 2)#，合并将继续进行，直至组数不超过该最大值。

最后，先前搁置的不可靠嵌入向量被分配至距离其最近的组中。至此，每个窗口局部的说话人都成功映射到了全局说话人。

下方的两幅示意图展示了二维空间中四个嵌入向量的聚类过程示例。每个组的质心绘制为实线箭头，外围环绕灰色圆圈，圆圈半径即为聚类阈值（此处为默认值 0.7）。两组能够合并的充要条件是它们的质心彼此落入对方的灰色圆圈内（即间距小于聚类阈值）。

##左图#展示了初始的四个组质心（即四个原始嵌入向量）。间距最近的两个质心以红色绘制，它们最先合并。接下来合并次近的两个质心（以蓝色绘制）。##右图#展示了两次合并后的状态：实线箭头是新生成的两个组的质心；虚线箭头是构成各组的原始嵌入向量。此时，两个新组的质心均未落入对方的灰色圆圈内（间距已超过聚类阈值），因此聚类停止，最终确定为两个说话人组。

{- 5.5x3
	b1 = 35
	b2 = 75
	r1 = 235
	r2 = 245
	b1x = cos(b1*pi/180)
	b1y = sin(b1*pi/180)
	b2x = cos(b2*pi/180)
	b2y = sin(b2*pi/180)
	r1x = cos(r1*pi/180)
	r1y = sin(r1*pi/180)
	r2x = cos(r2*pi/180)
	r2y = sin(r2*pi/180)
	threshold = 0.7

	# ==== LEFT ========
	Select inner viewport: 0, 2.5, 0.25, 2.75
	Axes: -1.5, 1.5, -1.5, 1.5
	Solid line

	# thresholds
	Line width: 0.6
	Colour: "Grey"
	Paint circle: 0.9, b1x, b1y, threshold
	Paint circle: 0.9, b2x, b2y, threshold
	Paint circle: 0.9, r1x, r1y, threshold
	Paint circle: 0.9, r2x, r2y, threshold
	Draw circle: b1x, b1y, threshold
	Draw circle: b2x, b2y, threshold
	Draw circle: r1x, r1y, threshold
	Draw circle: r2x, r2y, threshold

	# axes
	Line width: 1
	Colour: "Grey"
	Draw line: -1.3, 0, 1.3, 0
	Draw line: 0, -1.3, 0, 1.3
	Text special: 1.03, "left", 0, "bottom", "Times", 10, "0", "1"
	Text special: -1.03, "right", 0, "bottom", "Times", 10, "0", "\-m1"
	Text special: 0.03, "left", 1.01, "bottom", "Times", 10, "0", "1"
	Text special: -0.03, "right", -1.02, "top", "Times", 10, "0", "\-m1"

	# unit hypersphere
	Line width: 2
	Colour: "Black"
	Draw circle: 0, 0, 1

	# blue vectors
	Colour: "Blue"
	Line width: 2
	Draw arrow: 0.0, 0.0, b1x, b1y
	Draw arrow: 0.0, 0.0, b2x, b2y

	# red vectors
	Colour: "Red"
	Line width: 2
	Draw arrow: 0.0, 0.0, r1x, r1y
	Draw arrow: 0.0, 0.0, r2x, r2y

	# ==== RIGHT ========
	b3x = (b1x + b2x) / 2
	b3y = (b1y + b2y) / 2
	r3x = (r1x + r2x) / 2
	r3y = (r1y + r2y) / 2

	Select inner viewport: 3, 5.5, 0.25, 2.75
	Axes: -1.5, 1.5, -1.5, 1.5
	Solid line

	# thresholds
	Line width: 0.6
	Colour: "Grey"
	Paint circle: 0.9, b3x, b3y, threshold
	Paint circle: 0.9, r3x, r3y, threshold
	Draw circle: b3x, b3y, threshold
	Draw circle: r3x, r3y, threshold

	# axes
	Line width: 1
	Colour: "Grey"
	Draw line: -1.3, 0, 1.3, 0
	Draw line: 0, -1.3, 0, 1.3
	Text special: 1.03, "left", 0, "bottom", "Times", 10, "0", "1"
	Text special: -1.03, "right", 0, "bottom", "Times", 10, "0", "\-m1"
	Text special: 0.03, "left", 1.01, "bottom", "Times", 10, "0", "1"
	Text special: -0.03, "right", -1.02, "top", "Times", 10, "0", "\-m1"

	# unit hypersphere
	Line width: 2
	Colour: "Black"
	Draw circle: 0, 0, 1

	# blue vectors
	Dotted line
	Colour: "Blue"
	Line width: 1
	Draw arrow: 0.0, 0.0, b1x, b1y
	Draw arrow: 0.0, 0.0, b2x, b2y
	Solid line
	Line width: 2
	Draw arrow: 0.0, 0.0, b3x, b3y

	# red vectors
	Dotted line
	Colour: "Red"
	Line width: 1
	Draw arrow: 0.0, 0.0, r1x, r1y
	Draw arrow: 0.0, 0.0, r2x, r2y
	Solid line
	Line width: 2
	Draw arrow: 0.0, 0.0, r3x, r3y
}

##4. 重建（Reconstruction）#。在阶段 1 中，每个分析窗口被划分为 589 帧（每帧约 17 毫秒），每帧获得一个关于局部说话人活跃状态的 7 维概率向量。由于分析窗口相互重叠，全局时间轴上的每一帧都会被多个窗口同时覆盖。本阶段的目标是利用阶段 3 建立的局部到全局映射，将所有覆盖窗口的帧信息融合成全局结果。

利用阶段 1 的 7 维概率向量，针对每个窗口帧中的每位说话人，将其发声的所有可能组合的概率累加，计算出%%软激活度（soft activation）%（0 到 1 之间的数值，0 表示绝对静音，1 表示绝对活跃）。接着利用阶段 3 的映射将局部软激活度归属到全局说话人，并在所有覆盖该帧的窗口间取平均值，得到每个全局说话人在每个全局帧上的%%平均激活度%。

同时，以类似方式确定全局每一帧中同时活跃的说话人数。每个窗口帧都有一个发声胜出组合（阶段 1 的帧标签），包含 0、1 或 2 位说话人。在全局各帧上对所有覆盖窗口的说话人数取平均并四舍五入；若关闭了 ##Allow speakers to overlap#，则上限截断为 1。该数字决定了在该帧中将激活度最高的前几位说话人标记为活跃状态。

最后，将每位说话人所有连续活跃的帧序列聚合为一个%片段（segment）%。最终输出片段列表，每个片段归属于一位具体的说话人。

在 Praat 中的可用性
===================
说话人分离作为转写流程的一部分协同使用，参见 @@transcription with whisper.cpp@。

独立说话人分离在 Praat 中有两种使用方式：
- 同时选定 @Sound 与相对应的 @TextGrid，并选择 @@TextGrid & Sound: Diarize interval...|分离区间说话人...@；
- 在 @TextGridEditor 编辑窗口的 #Interval（区间）菜单中选择 ##Diarize interval#。该命令的参数通过同菜单下的 ##Diarization settings...# 设置，并会在不同 Praat 会话间持久保存。

################################################################################
"TextGrid & Sound: Diarize interval..."
© Anastasia Shchupak 2026-06-01

此命令获取 @TextGrid 中指定的区间，对 @Sound 中相对应的音频片段执行说话人分离，并将分离结果写回 TextGrid 中。区间的文字标签由 ##Non-speech interval label# 和 ##Speech interval label# 设定。

设置
====
##Tier number
:	包含待分离区间的层编号。

##Interval number
:	待分离的区间编号。

##Max. number of speakers (≥ 2)#（标准值：2）
:	参见 @@speaker diarization with adapted pyannote.audio@。

##Allow speakers to overlap#（标准：开启）
:	参见 @@speaker diarization with adapted pyannote.audio@。

##Clustering threshold (0-2)#（标准值：0.7）
:	参见 @@speaker diarization with adapted pyannote.audio@。

##Segmentation step (0-1)#（标准值：0.1）
:	参见 @@speaker diarization with adapted pyannote.audio@。

##Non-speech interval label#（标准值：“”）
:	生成 TextGrid 中归类为非语音的区间的文字标签。

##Speech interval label#（标准值：“speech”）
:	生成 TextGrid 中归类为语音的区间的文字标签。

################################################################################
"SpeechRecognizer"
© Anastasia Shchupak 2026-03-15

Praat 中的 @@types of objects|对象类型@ 之一。它能对 @Sound 对象执行 @@transcription with whisper.cpp|基于 whisper.cpp 的语音转写@。如果您初次接触 Praat 的语音识别功能，请先参阅 @@Speech recognition|语音识别@ 教程。

命令
====

创建：
,	@@Create SpeechRecognizer...@

转写：
,	@@SpeechRecognizer & Sound: Transcribe@

################################################################################
"Create SpeechRecognizer..."
© Anastasia Shchupak 2026-03-15

创建 @SpeechRecognizer 语音识别器对象。
如果您初次接触 Praat 的语音识别功能，请先参阅 @@Speech recognition|语音识别@ 教程。

设置
====
##Whisper model
##Language
:	两项设置的详细说明请参见 @@transcription with whisper.cpp@。

################################################################################
"SpeechRecognizer & Sound: Transcribe"
© Anastasia Shchupak 2026-03-15

使用选定的 @SpeechRecognizer 对象对选定的 @Sound 对象执行 @@transcription with whisper.cpp|转写@，并将转写文本结果输出到 @@Info window|信息窗口@。

音频在处理前会自动重采样至 16 kHz（@@whisper.cpp@ 所要求的采样频率）。

转写过程中会使用 @@whisper.cpp@ 内置的 @@speech activity detection with Silero VAD@ 跳过声音中的非语音部分，从而兼具更快的转写速度与更高的识别准确率。

转写结果为一个包含完整文本的字符串。

################################################################################
"Silero VAD"
© Anastasia Shchupak 2026-06-01

Silero VAD 是由 Silero 团队研发的预训练语音活动检测器；模型与文档可参见 @@Silero Team (2024)@。VAD 代表“语音活动检测（Voice Activity Detection）”，但我们通常称其为“speech activity detection”，因为 Silero VAD 既能检测浊音语音，也能可靠检测清音语音。Silero VAD 模型已内置编译进 Praat 中（参见 @Acknowledgments）。

有关 Silero VAD 在 Praat 中的具体用法、算法及设置，请参阅 @@speech activity detection with Silero VAD@。

################################################################################
"whisper.cpp"
© Anastasia Shchupak 2026-06-01

Whisper 是 OpenAI 开发的用于将语音转写为文字的自动语音识别（ASR）系统；有关模型架构、训练与评测可参见 @@Radford et al. (2022)@。OpenAI 官方的 Whisper 实现基于 Python (PyTorch)。

Praat 采用了 whisper.cpp，这是由 Georgi Gerganov 及众多贡献者基于机器学习张量库 ggml 开发的轻量级 C/C++ 移植版本（参见 @Acknowledgments）。OpenAI 原生的 Whisper 模型必须转换为 ggml 格式后方可供 whisper.cpp 使用。

有关转写在 Praat 中的完整用法，请参阅 @@Speech recognition@ 教程。
有关转写参数设置的详细说明，请参阅 @@transcription with whisper.cpp@。

################################################################################
"pyannote.audio"
© Anastasia Shchupak 2026-06-01

pyannote.audio 是由 Hervé Bredin 及其合作者开发的自动说话人分离开源工具包（参见 @@Plaquet & Bredin (2023)@ 及 @@Bredin (2023)@）。

Praat 内置了其 `pyannote/speaker-diarization-3.1` 流水线的 C++/ggml 改编版本。该流水线包含两个神经网络模型：用于语音分割的 pyannote.audio `pyannote/segmentation-3.0`，以及用于提取说话人嵌入向量的 @@WeSpeaker@ `wespeaker-voxceleb-resnet34-LM`。`pyannote/segmentation-3.0` 的权重已转换为 ggml 格式并内置于 Praat 中（参见 @Acknowledgments）。

有关说话人分离在 Praat 中的具体用法，请参阅 @@Speech recognition@ 教程。
有关说话人分离设置的详细说明，请参阅 @@speaker diarization with adapted pyannote.audio@。

################################################################################
"WeSpeaker"
© Anastasia Shchupak 2026-06-01

WeSpeaker 是由 WeNet 社区开发的面向学术研究与工业落地的说话人嵌入学习工具包（参见 @@Wang et al. (2023)@ 及 @@Wang et al. (2024)@）。Praat 将其预训练的 `wespeaker-voxceleb-resnet34-LM` 嵌入模型用作 @@pyannote.audio@ 的 `pyannote/speaker-diarization-3.1` 流水线的重要组成部分。

`wespeaker-voxceleb-resnet34-LM` 在 VoxCeleb2 数据集上训练完成（参见 @@Chung, Nagrani & Zisserman (2018)@ 及 @@Nagrani, Chung & Zisserman (2017)@）。模型权重已转换为 ggml 格式并内置于 Praat 中（参见 @Acknowledgments）。Praat 包含了针对该模型进行推理的带有 TSTP 池化的 WeSpeaker ResNet34 架构的 C++/ggml 移植版本。

################################################################################
"Silero Team (2024)"
© Anastasia Shchupak 2026-06-01

Silero Team (2024). Silero VAD: pre-trained enterprise-grade voice activity detector (VAD),
number detector and language classifier [Computer software]. Version 6.2.0.

Available on `https://github.com/snakers4/silero-vad`.

################################################################################
"Radford et al. (2022)"
© Anastasia Shchupak 2026-06-01

Alec Radford, Jong Wook Kim, Tao Xu, Greg Brockman, Christine McLeavey & Ilya Sutskever (2022):
“Robust speech recognition via large-scale weak supervision.”

Available on `https://cdn.openai.com/papers/whisper.pdf`.

################################################################################
"Plaquet & Bredin (2023)"
© Anastasia Shchupak 2026-06-01

Alexis Plaquet & Hervé Bredin (2023): “Powerset multi-class cross entropy loss for neural
speaker diarization.” %%Proc. Interspeech 2023%.

Available on `https://www.isca-archive.org/interspeech_2023/plaquet23_interspeech.html`.

################################################################################
"Bredin (2023)"
© Anastasia Shchupak 2026-06-01

Hervé Bredin (2023): “pyannote.audio 2.1 speaker diarization pipeline: principle, benchmark,
and recipe.” %%Proc. Interspeech 2023%.

Available on `https://www.isca-archive.org/interspeech_2023/bredin23_interspeech.html`.

################################################################################
"Wang et al. (2023)"
© Anastasia Shchupak 2026-06-01

Hongji Wang, Chengdong Liang, Shuai Wang, Zhengyang Chen, Binbin Zhang, Xu Xiang, Yanlei Deng
& Yanmin Qian (2023): “Wespeaker: a research and production oriented speaker embedding learning
toolkit.” %%ICASSP 2023 - 2023 IEEE International Conference on Acoustics, Speech and Signal
Processing (ICASSP)%, 1–5.

Available on `https://doi.org/10.48550/arXiv.2210.17016`.

################################################################################
"Wang et al. (2024)"
© Anastasia Shchupak 2026-06-01

Shuai Wang, Zhengyang Chen, Bing Han, Hongji Wang, Chengdong Liang, Binbin Zhang, Xu Xiang,
Wen Ding, Johan Rohdin, Anna Silnova, Yanmin Qian & Haizhou Li (2024): “Advancing speaker
embedding learning: Wespeaker toolkit for research and production.” %%Speech Communication%
##162#: 103104.

Available on `https://doi.org/10.1016/j.specom.2024.103104`.

################################################################################
"Chung, Nagrani & Zisserman (2018)"
© Anastasia Shchupak 2026-06-01

Joon Son Chung, Arsha Nagrani & Andrew Zisserman (2018): “VoxCeleb2: deep speaker recognition.”
%%Proc. Interspeech 2018%.

Available on `https://www.isca-archive.org/interspeech_2018/chung18b_interspeech.html`.

################################################################################
"Nagrani, Chung & Zisserman (2017)"
© Anastasia Shchupak 2026-06-01

Arsha Nagrani, Joon Son Chung & Andrew Zisserman (2017): “VoxCeleb: a large-scale speaker
identification dataset.” %%Proc. Interspeech 2017%.

Available on `https://www.isca-archive.org/interspeech_2017/nagrani17_interspeech.html`.

################################################################################
)~~~"
MAN_PAGES_END

}

/* End of file manual_SpeechRecognizer.cpp */
