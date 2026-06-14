/* manual_glossary.cpp
 *
 * Copyright (C) 1992-2008,2010,2011,2014-2017,2020-2025 Paul Boersma
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

#include "manual_exampleSound.h"

static void draw_TimeDomain_Sound (Graphics g) {
	Sound_draw (manual_exampleSound (), g, 0, 0, 0, 0, true, U"curve");
}
static void draw_TimeDomain_Pitch (Graphics g) {
	Pitch_draw (manual_examplePitch (), g, 0, 0, 200.0, 500.0, true, Pitch_speckle_NO, kPitch_unit::HERTZ);
}

void manual_glossary_init (ManPages me);
void manual_glossary_init (ManPages me) {

MAN_BEGIN (U"aliasing", U"ppgb", 20040331)
INTRO (U"混叠（荷兰语：%vouwvervorming%）是采样信号产生歧义的现象。")
ENTRY (U"示例")
NORMAL (U"在 10 kHz 的采样频率下，频率为 3 kHz 的正弦波与频率为 7 kHz、"
	"13 kHz 或 17 kHz 等的正弦波具有相同的表示形式。如果采样信号旨在表示从 0 Hz 开始的 "
	"连续频谱范围（这是语音录音中最常见的情况），"
	"采样后所有这些音调都很可能被解释为 3 kHz 的音调。")
NORMAL (U"为了纠正这种不希望出现的情况，通常在采样之前对信号进行低通滤波，"
	"其截止频率稍低于 5 kHz。")
MAN_END

MAN_BEGIN (U"Click", U"ppgb", 19960913)
INTRO (U"控制 @@Editors|编辑器@ 的方法之一。")
ENTRY (U"如何点击")
LIST_ITEM (U"1. 将鼠标悬停在您想要点击的对象上方。")
LIST_ITEM (U"2. 按下并释放（左）鼠标键。")
NORMAL (U"参见 @@Shift-click@。")
ENTRY (U"在 Praat 程序中的用法")
NORMAL (U"点击一个对象用于选择该对象，同时取消选择 "
	"所有之前选中的对象；点击也用于移动光标线。")
MAN_END

MAN_BEGIN (U"constant extrapolation", U"ppgb", 20080425)
INTRO (U"- 对层中第一个点之前或最后一个点之后的数值的解释。")
ENTRY (U"示例")
NORMAL (U"以下是一个包含三个点的 @PitchTier（音高层）：")
SCRIPT (4, 3, U""
	"Create PitchTier: \"tier\", 0, 0.5\n"
	"Add point: 0.10, 170\n"
	"Add point: 0.20, 180\n"
	"Add point: 0.45, 110\n"
	"Draw: 0, 0, 50, 250, \"yes\"\n"
	"Remove\n"
	"One mark left: 100, \"yes\", \"yes\", \"yes\", \"\"\n"
	"One mark left: 150, \"yes\", \"yes\", \"yes\", \"\"\n"
	"One mark left: 200, \"yes\", \"yes\", \"yes\", \"\"\n"
	"One mark bottom: 0.1, \"yes\", \"yes\", \"yes\", \"\"\n"
	"One mark bottom: 0.2, \"yes\", \"yes\", \"yes\", \"\"\n"
	"One mark bottom: 0.3, \"yes\", \"yes\", \"yes\", \"\"\n"
	"One mark bottom: 0.4, \"yes\", \"yes\", \"yes\", \"\"\n"
)
NORMAL (U"在 0.10 和 0.20 秒之间，音高从 170 Hz 升至 180 Hz，"
	"在 0.20 和 0.45 秒之间，它从 180 Hz 降至 110 Hz。"
	"这是 @@linear interpolation|线性插值@。")
NORMAL (U"但在 0.10 秒之前发生了什么？在那里，音高恰好是 170 Hz，"
	"即第一个音高点的值。这是向左的 %%constant extrapolation%（常数外推）。")
NORMAL (U"同样，在 0.45 秒之后，音高恰好是 110 Hz，"
	"即最后一个音高点的值。这是向右的常数外推。")
ENTRY (U"Praat 中的常数外推")
NORMAL (U"Praat 在所有包含时间点数值的层和网格中都使用常数外推 "
	"（@PitchTier、@IntensityTier、@DurationTier、@AmplitudeTier、@FormantGrid）。")
MAN_END

MAN_BEGIN (U"Drag", U"ppgb", 19960913)
INTRO (U"拖动是控制 @@Editors|编辑器@ 的方法之一。")
ENTRY (U"如何拖动")
LIST_ITEM (U"1. 将鼠标悬停在您想要拖动的对象上方。")
LIST_ITEM (U"2. 按下（左）鼠标键。")
LIST_ITEM (U"3. 保持按下鼠标键的同时，在窗口中移动鼠标。"
	"该对象的阴影将会跟随移动。")
LIST_ITEM (U"4. 当鼠标移动到您希望放置对象的位置（%drop %site，即释放位置）时释放鼠标键。如果该释放位置有效，"
	"对象将移动到那里。")
NORMAL (U"参见 @@Shift-drag@。")
ENTRY (U"在 Praat 程序中的用法")
NORMAL (U"拖动用于操纵一个或多个标记、目标点或边界的时间和值：")
MAN_END

MAN_BEGIN (U"end time", U"ppgb", 20060714)
INTRO (U"- @@time domain|时间域@（见该词条）的终点。")
MAN_END

MAN_BEGIN (U"Fast Fourier Transform", U"ppgb", 20231115)  // 20041123, 2024
INTRO (U"一种用于快速计算采样信号傅里叶变换的算法。"
	"它涉及将样本数 %N 增加到紧邻的下一个 2 的幂，"
	"计算时间按 %N log %N 的比例缩放。")
NORMAL (U"在 Praat 中，快速傅里叶变换（FFT）用于：")
LIST_ITEM (U"1. 整个声音的傅里叶变换：@@Sound: To Spectrum...@ 并在 #Fast 设置中选择 #yes，"
	"以及 @@Spectrum: To Sound@。")
LIST_ITEM (U"2. 声音中连续帧 of a sound 的傅里叶变换。参见 @@Sound: To Spectrogram...@。")
LIST_ITEM (U"3. 相关性的快速计算，例如在 @@Sound: To Pitch (raw autocorrelation)...@ 中。")
MAN_END

MAN_BEGIN (U"FFT", U"ppgb", 20011121)
INTRO (U"@@Fast Fourier Transform|快速傅里叶变换@的缩写。")
MAN_END

MAN_BEGIN (U"frequency", U"ppgb", 20030314)
INTRO (U"频率是指某事在特定时间内发生的频繁程度，例如 "
	"Praat 主页 `praat.org` 每天被访问的次数。")
NORMAL (U"在 Praat 中，频率是指每秒的振动循环次数。"
	"虽然有时可以看到缩写 %cps，但 Praat 始终使用 Hz（%hertz 的缩写），"
	"它们的含义相同。")
NORMAL (U"不幸的是，在语音分析中存在两种截然不同的振动。"
	"对于基频/音高，频率是指每秒的声门周期数；而对于频谱分析，"
	"频率是指每秒的正弦波周期数。声学语音学家的很大一部分训练 "
	"在于理解 F0 和 F1 背后概念之间的差异，"
	"并且可能要花很多年才能理解它们在产生、"
	"声学、感知或测量中彼此产生的影响...")
NORMAL (U"为了防止混淆，Praat 始终要求频率以 Hz 表示。"
	"因此，如果您想要提供 20 kHz（20 千赫兹）的采样频率，您需要填写 20000、2e4 或 20e3。"
	"如果您想要在某些频谱分析中关闭预加重，您需要在 \"from-frequency\" 处提供 1 GHz（吉赫兹），即输入 1e9。")
NORMAL (U"在 Praat 编辑器窗口中，由于时间已经必须 "
	"从左向右流动，因此频率通常自底向上排列。这适用于声谱图、音高曲线和共振峰曲线。"
	"在频谱切片中，频率自左向右排列，因为这些图没有时间轴。")
MAN_END

MAN_BEGIN (U"linear interpolation", U"ppgb", 20080426)
INTRO (U"- 对层中第一个点和最后一个点之间的数值的解释。")
ENTRY (U"示例")
NORMAL (U"以下是一个包含三个点的 @PitchTier（音高层）：")
SCRIPT (4, 3, U""
	"Create PitchTier: \"tier\", 0, 0.5\n"
	"Add point: 0.10, 170\n"
	"Add point: 0.20, 180\n"
	"Add point: 0.45, 110\n"
	"Draw: 0, 0, 50, 250, \"yes\"\n"
	"Remove\n"
	"One mark left: 100, \"yes\", \"yes\", \"yes\", \"\"\n"
	"One mark left: 150, \"yes\", \"yes\", \"yes\", \"\"\n"
	"One mark left: 200, \"yes\", \"yes\", \"yes\", \"\"\n"
	"One mark bottom: 0.1, \"yes\", \"yes\", \"yes\", \"\"\n"
	"One mark bottom: 0.2, \"yes\", \"yes\", \"yes\", \"\"\n"
	"One mark bottom: 0.3, \"yes\", \"yes\", \"yes\", \"\"\n"
	"One mark bottom: 0.4, \"yes\", \"yes\", \"yes\", \"\"\n"
)
NORMAL (U"在 0.10 和 0.20 秒之间，音高线性地从 170 Hz 升至 180 Hz，"
	"在 0.20 和 0.45 秒之间，它线性地从 180 Hz 降至 110 Hz。"
	"这是 %%linear interpolation%（线性插值）：在两个相邻点之间的所有时刻，"
	"音高值都遵循连接这两个点的直线。")
NORMAL (U"（在 0.10 秒之前和 0.45 秒之后，进行 @@constant extrapolation|常数外推@。）")
ENTRY (U"Praat 中的线性插值")
NORMAL (U"Praat 在所有包含时间点数值的层和网格中都使用线性插值 "
	"（@PitchTier、@IntensityTier、@DurationTier、@AmplitudeTier、@FormantGrid）。")
MAN_END

MAN_BEGIN (U"Nyquist frequency", U"ppgb", 20040331)
INTRO (U"%%Nyquist frequency%（奈奎斯特频率）是采样信号的带宽，"
	"等于该信号采样频率的一半。"
	"如果采样信号旨在表示从 0 Hz 开始的连续频谱范围（这是语音录音中最常见的情况），"
	"则奈奎斯特频率是采样信号"
	"能够无歧义地表示的最高频率。")
ENTRY (U"示例")
NORMAL (U"如果语音信号以 22050 Hz 的频率进行采样，那么我们可以 "
	"期望在采样信号中出现的最高频率是 11025 Hz。"
	"这意味着为了满足这一期望，我们应该使连续信号 "
	"通过截止频率低于 11025 Hz 的低通滤波器；"
	"否则，我们将遇到 @@aliasing|混叠@ 现象。")
NORMAL (U"当然，在 22050 Hz 的采样频率下，我们也可以表示一个 "
	"带宽受限在例如 40000 Hz 到 51025 Hz 之间的信号，但这在语音研究中 "
	"似乎用处不大。")
MAN_END

MAN_BEGIN (U"objects", U"ppgb", 20030528)
INTRO (U"Praat 程序中包含数据的实体。")
NORMAL (U"这些对象在 @@List of Objects|对象列表@ 中的 @@Objects window|对象窗口@ 中可见。")
NORMAL (U"大多数对象都保存在内存中：它们%不是文件%！"
	"因此，在您 @@Quit|退出@ 之前，您可能希望使用 @@Save menu|保存菜单@ 中的某个命令来保存它们。"
	"例外是 Praat 程序中的 LongSound 对象以及 ALS 程序中基于文件的词典。")
NORMAL (U"您可以通过从 @@New menu|新建菜单@ 或 "
	"@@Open menu|打开菜单@ 选择命令来创建对象。")
NORMAL (U"当您选择一个或多个对象时，您可以对它们执行在 @@dynamic menu|动态菜单@ 中、"
	"在对象列表下方的 ##fixed buttons#（固定按钮）上或在 @@Save menu|保存菜单@ 中显示的操作。您可以在 @ButtonEditor 帮助下选择隐藏的操作。")
MAN_END

MAN_BEGIN (U"overlap-add", U"ppgb", 20070816)
INTRO (U"一种用于操纵声音语音信号的音高和时长的方法。"
	"该方法由 @@Moulines & Charpentier (1990)@ 实现，"
	"他们称之为时域基音同步叠加技术（TD-PSOLA）。"
	"如今，PSOLA™ 和 TD-PSOLA™ 是法国电信（France Telecom^®）拥有的商标。")
ENTRY (U"重叠相加合成")
NORMAL (U"当从 @Sound 创建 @Manipulation 对象时，会执行以下步骤：")
LIST_ITEM (U"1. 从 @PitchTier 中，沿着整个时间域生成新的点，"
	"使用 @@PitchTier: To PointProcess@ 的方法。")
LIST_ITEM (U"2. 原始脉冲中的周期信息（在 Manipulation 对象中可用）"
	"用于从新脉冲中删除所有位于无声（清音）区间内的点"
	"（即，原始脉冲中相邻点之间的距离大于 20 ms 的位置）。")
LIST_ITEM (U"3. 清音部分从源 Sound 复制到目标 Sound，"
	"如果局部时长大于 1，则重用某些部分。")
LIST_ITEM (U"4. 对于每个目标点，我们查找最近的源点。源 Sound 的一部分，"
	"以源点为中心，被复制到目标 Sound 的由目标点决定的位置，"
	"使用钟形窗口，其左侧半长度是源点和目标点相邻的左侧周期的最小值"
	"（右侧半长度依此类推）。")
MAN_END

MAN_BEGIN (U"pitch floor", U"ppgb", 20060914)
INTRO (U"音高下限是 Sound 窗口中音高范围的底部（参见 @Intro）。"
	"标准设置为 75 赫兹；低于该音高下限的音高值将不会被 Praat 计算或显示。")
NORMAL (U"要更改音高下限，请选择 @@Pitch settings...@。")
MAN_END

MAN_BEGIN (U"power spectral density", U"ppgb", 20101026)
INTRO (U"在一定时间范围和一定频率范围内声音的平均功率，单位为 Pa^2/Hz。")
ENTRY (U"数学定义")
NORMAL (U"声音 %x(%t) 在时间范围 (%t__1_, %t__2_) 内的%%复数频谱%为")
EQUATION (U"%X(%f) ≡ ∫__%%t%1_^^%%t%2^ %x(%t) e^^-2%π%ift^ %dt")
NORMAL (U"对于双侧频域 (-%F, +%F) 中的任何频率 %f。 "
	"如果 %x(%t) 以帕斯卡（Pascal）为单位表示，则 %X(%f) 以 Pa/Hz 为单位表示。"
	"在 Praat 中，该复数频谱是存储在 @Spectrum 中的物理量。")
NORMAL (U"从复数频谱中，我们可以计算以 Pa^2/Hz 为单位的%%单侧功率谱密度%为")
EQUATION (U"PSD(%f) ≡ 2|%X(%f)|^2 / (%t__2_ - %t__1_)")
NORMAL (U"其中因子 2 是由于将正频率和负频率的贡献相加所致。"
	"在 Praat 中，该功率谱密度是存储在 @Spectrogram 中的物理量。")
NORMAL (U"功率谱密度（PSD）分配了声音的总功率。"
	"为了说明这一点，我们在其整个单侧频域 (0, %F) 上对其进行积分：")
EQUATION (U"∫__0_^^%F^ PSD(%f) %df = ∫__0_^^%F^ 2|%X(%f)|^2/(%t__2_-%t__1_) %df =")
EQUATION (U"= 1/(%t__2_-%t__1_) ∫__-%F_^^+%F^ |%X(%f)|^2 %df = 1/(%t__2_-%t__1_) ∫__%%t%1_^^%%t%2^ |%x(%t)|^2 %dt")
NORMAL (U"其中最后一步使用了%%帕塞瓦尔定理%（Parceval's theorem）。"
	"结果恰好是声音在时间范围 (%t__1_, %t__2_) 内的平均功率。")
ENTRY (U"对数功率谱密度")
NORMAL (U"相对于 %%P__ref_% = 2·10^^-5^ Pa，将功率谱密度以 dB 表示通常很有用：")
EQUATION (U"PSD__dB_(%f) = 10 log__10_ { PSD(%f) / %%P__ref_%^2 }")
NORMAL (U"由于对数的自变量单位为 Hz^^-1^，这套频谱度量可以通俗地说是"
	"以 “dB/Hz” 为单位。在 Praat 中，该对数功率谱密度是存储在 @Ltas 中的物理量；"
	"它也是在 @Spectrum 和 @Spectrogram 图像中显示的物理量。")
MAN_END

MAN_BEGIN (U"quantile algorithm", U"ppgb", 19980101)
INTRO (U"一种用于计算已排序实数数组的指定分位数的算法。")
NORMAL (U"连续实值分布的 %n\%  %分位数是指预计有 %n\%  的值低于该值。如果我们得到一个我们想要解释为从某个分布中抽取的实数数组，我们可以%估计%底层分布的分位数。")
ENTRY (U"1. 中位数")
NORMAL (U"中位数（%median%）是分位数的一个特例：它是 50\%  分位数。它通常估算如下："
	"对于奇数个值，取中间的值；对于偶数个值，取中间两个值的平均值。例如，如果我们的值是 15, 20 和 32，中位数就是 20；"
	"如果我们的值是 15, 20, 32 和 60，中位数就是 26。")
NORMAL (U"此估算是与方向无关的：如果我们将所有值乘以 -1（即变成 -60, -32, -20 和 -15），"
	"中位数也会乘以 -1（变成 -26）。")
ENTRY (U"2. 百分位数？")
NORMAL (U"一组值的第 %%n% 个%百分位数通常被定义为已证实的最大值，"
	"该值使得最多有 %n\%  的已证实值小于或等于它。"
	"例如，如果我们的值是 15, 20, 32 和 60，第 30 个百分位数是 15。下面是一个详细列表：")
LIST_ITEM (U"\t百分位数序号\t值")
LIST_ITEM (U"\t0\t-")
LIST_ITEM (U"\t10\t-")
LIST_ITEM (U"\t20\t-")
LIST_ITEM (U"\t30\t15")
LIST_ITEM (U"\t40\t15")
LIST_ITEM (U"\t50\t20")
LIST_ITEM (U"\t60\t20")
LIST_ITEM (U"\t70\t20")
LIST_ITEM (U"\t80\t32")
LIST_ITEM (U"\t90\t32")
LIST_ITEM (U"\t100\t60")
NORMAL (U"然而，此过程并不能产生底层分布分位数的估算值。"
	"例如，估算是与方向相关的：如果我们将所有值乘以 -1，第 50 个百分位数"
	"变为 -32 而不是 -20，第 70 个百分位数变为 -32 而不是预期的 -15（"
	"即原始数据集第 30 个百分位数的相反数）。")
ENTRY (U"3. 无偏分位数")
NORMAL (U"为了更好地估计底层分布的分位数，我们将用于确定中位数的插值方法推广到%任何分位数%。")
NORMAL (U"我们假设已证实的数值 15、20、32 和 60 各自占去 \"分位数空间\" 的四分之一。"
	"这四个值分别位于这四个四分之一区间的中间，因此它们处于 0.125、0.375、0.625 和 0.875 分位数处。")
NORMAL (U"在 0.125 和 0.875 之间的分位数通过线性插值进行评估：0.25、0.50 和 0.75 分位数分别为 17.5、26 和 46。"
	"注意 0.50 分位数是中位数。例如，0.40 分位数估算为 20 + (32 - 20)·(0.40 - 0.375)/(0.625 - 0.375) = 21.2。")
NORMAL (U"介于 0 和 0.125 之间或介于 0.875 和 1 之间的分位数，通过对最低或最高的一对数值进行线性外推进行评估："
	"0\%  分位数估算为 15 - 1/2 (20 - 15) = 12.5，"
	"而 100\%  分位数估算为 60 + 1/2 (60 – 32) = 74。0.10 分位数估算为 "
	"12.5 + (15 – 12.5)·(0.10 – 0.0)/(0.125 – 0.0) = 14.5。")
NORMAL (U"请注意，极低或极高分位数的估算值可能会落在已证实数值的范围之外。"
	"事实上，计算出的 0\%  和 100\%  分位数被认为是该分布的最小值和最大值的估算值。"
	"对于均匀分布，这些估算是合理的；当然，对于正态分布，"
	"0\%  和 100\%  分位数是毫无意义的。")
MAN_END

MAN_BEGIN (U"sampling frequency", U"ppgb", 20220102)
INTRO (U"采样频率（或%%采样率%）是 @Sound 中每秒的样本数。"
	"例如：如果采样频率为 44100 赫兹，持续时间为 60 秒的录音"
	"将包含 2,646,000 个样本。")
NORMAL (U"采样频率的常用值有 44100 Hz（CD 音质）和 48000 Hz（许多现代音频设备）"
	"（两者对于人类听觉系统来说都足够了，因为人类听不到 20000 Hz 以上的频率；参见 @@aliasing|混叠@）。")
NORMAL (U"要获取所选 #Sound 的采样频率，请点击 #Info 或选择 @@Get sampling frequency@。")
MAN_END

MAN_BEGIN (U"sampling period", U"ppgb", 20040415)
INTRO (U"采样周期是 @Sound 中两个连续样本之间的时间差。"
	"它是 @@sampling frequency|采样频率@ 的倒数。例如：如果采样频率"
	"为 44100 Hz，采样周期为 1/44100 = 2.2675736961451248e-05 秒："
	"样本之间大约间隔 23 微秒。")
NORMAL (U"要获取所选 #Sound 的采样周期，请点击 #Info 或选择 @@Get sampling period@。")
MAN_END

MAN_BEGIN (U"Shift-drag", U"ppgb", 19980823)
INTRO (U"Shift-拖动是控制 @@Editors|编辑器@ 的方法之一。")
ENTRY (U"How to Shift-drag")
LIST_ITEM (U"1. 将鼠标悬停在您想要拖动的任何对象上方（这些对象可能已预先被选中）。")
LIST_ITEM (U"2. 按下 Shift 键。")
LIST_ITEM (U"3. 按下（左）鼠标键。")
LIST_ITEM (U"4. 在保持按下鼠标键的同时，在窗口中移动鼠标。这些对象的阴影将会跟随移动。")
LIST_ITEM (U"5. 当鼠标移动到您希望放置对象的位置时释放鼠标键。如果该释放位置（%%drop site%）有效，对象将移动到那里。")
ENTRY (U"在 Praat 程序中的用法")
NORMAL (U"虽然普通 @@drag|拖动@ 用于移动首先通过点击选中的对象，"
	"但 ##Shift-dragging#（Shift-拖动）用于同时操纵更多标记、目标点或边界的时间和值：")
MAN_END

MAN_BEGIN (U"Shift-click", U"ppgb", 19960913)
INTRO (U"控制 @@Editors|编辑器@ 的方法之一。")
ENTRY (U"如何 Shift-点击")
LIST_ITEM (U"1. 将鼠标悬停在您要进行 Shift-点击的对象上方。")
LIST_ITEM (U"2. 按下 Shift 键。")
LIST_ITEM (U"3. 按下并释放（左）鼠标键。")
ENTRY (U"在 Praat 程序中的用法")
NORMAL (U"普通的 @@click|点击@ 用于在取消选择所有先前选中对象的同时仅选择一个对象，"
	"而 ##Shift-click#（Shift-点击）用于选择标记、目标点或边界，%而不会取消选择%先前选中的对象：")
MAN_END

MAN_BEGIN (U"sound pressure calibration", U"ppgb", 20041119)
NORMAL (U"您电脑的录音软件会返回介于 -32768 到 32767 之间的整数值。"
	"Praat 在将它们放入 Sound 对象之前会除以 32768，"
	"以便 Sound 对象中的值始终介于 -1 和 +1 之间。")
NORMAL (U"Praat 程序将这些数值视为以帕斯卡（Pascal）为单位的空气压强。")
NORMAL (U"这些帕斯卡值可能并非进入麦克风的实际真实空气压强。"
	"为了确定实际空气压强是多少，您必须录制一段带有已知空气压强的额外声音，"
	"且录音增益与其余录音相同。最好这段额外声音是像白噪声之类的，"
	"以便可以可靠地确定其 @@sound pressure level|声压级@。例如，"
	"400 Hz 的正弦波就不太合适，因为麦克风处的声压级可能非常强烈地"
	"取决于扬声器的精确位置和方向，以及房间内其他物体（例如作为实验者的您）"
	"的位置（相差 10 dB 并不罕见）。")
NORMAL (U"例如，假设您使用 CD 录音机或 Praat 的 SoundRecorder 录制了一些语音。"
	"在录音之前或之后，您还录制了由噪声发生器产生的白噪声。"
	"使用放置在与录音麦克风相同位置的分贝计（线性或 C 设置），"
	"您测得该噪声的声压级（SPL）为 76.5 dB。"
	"当您稍后在 Praat 的声音窗口中打开录制的噪声时，Praat 会告诉您其平均音强"
	"（开启 ##Show intensity#）为 68.6 dB。然后您就知道必须将 Praat 中测得的音强加上 7.9 dB"
	"才能得到真实的声压级。因此，如果您的语音包含一个长元音 [a:]，其在声音窗口中测得的平均音强"
	"为 71.1 dB，则其真实的声压级必须为 79.0 dB。")
NORMAL (U"在此示例中，如果将声音乘以 10^^(7.9/20)^ 的系数，"
	"则可以让 Praat 的声音窗口显示以帕斯卡为单位的真实声压和以 dB 为单位的真实声压级，"
	"您可以通过选择该 Sound 并从 #Modify 菜单中选择 ##Multiply...#，"
	"然后提供 `10^(7.9/20)` 作为乘数来做到这一点。")
MAN_END

MAN_BEGIN (U"sound pressure level", U"ppgb", 20041124)
NORMAL (U"空气压强以帕斯卡（Pa，即每平方米牛顿 N/m^2）为单位进行测量。"
	"环境大气压约为 100,000 Pa，肺部在发音时仅对其进行微小的改变，"
	"即 200 至 1000 Pa（= 2 至 10 厘米水柱 H__2_O）。"
	"在您的身体之外，由您的说话引起的空气压强又要小得多，"
	"在距离您嘴唇 1 米处通常为 0.01 至 1 Pa。这与您"
	"在 Praat 的声音窗口中看到的典型语音录音的值相当，"
	"尽管只有在您执行 @@sound pressure calibration|声压校准@（包括乘法）时，"
	"Praat 中的这些数值才能被解释为真实的空气压强。")
NORMAL (U"对于频率为 1000 Hz 的正弦波，标准人耳可以检测到 "
	"小至 0.00002 Pa 的均方根空气压强。"
	"%%sound pressure level%（声压级，SPL）可以相对于该标准阈值以 dB 表示：")
EQUATION (U"SPL = 10 log__10_ { 1/(%t__2_-%t__1_) ∫__t1_^^t2^ %x^2(%t) %dt / (2·10^^-5^ Pa)^2 }")
NORMAL (U"其中 %x(%t) 是作为时间函数的以 Pa 为单位的声压，"
	"而 %t__1_ and %t__2_ 是计算能量（平方空气压强）平均值的时间起点和终点。"
	"对于已校准的声音，如果您开启 ##Show intensity# 并将其 ##averaging method# 设置为 #energy（通过 ##Intensity settings...#），"
	"Praat 的声音窗口将向您显示作为时间函数的声压级（SPL）。")
MAN_END

MAN_BEGIN (U"spectro-temporal representation", U"ppgb", 20030314)
INTRO (U"一种将（例如声音信号的）某种强度表示为 @time（时间）和 @frequency（频率）函数的表示形式。"
	"在 Praat 中，我们有 @Spectrogram（声谱图），它是作为以秒为单位的时间和以 Hz 为单位的频率函数的声能量密度；"
	"以及 @Cochleagram（耳蜗图），它是作为以秒为单位的时间和以 Bark 为单位的频率函数的基底膜激发模式。")
NORMAL (U"有关教程信息，请参见 @@Intro 3.1. Viewing a spectrogram@。")
MAN_END

MAN_BEGIN (U"start time", U"ppgb", 20060714)
INTRO (U"- @@time domain|时间域@（见该词条）的起点。")
MAN_END

MAN_BEGIN (U"stereo", U"ppgb", 20110131) // 2023
INTRO (U"立体声声音是包含左声道和右声道的音频。"
	"Praat 可以读取和写入单声道和双声道（立体声）声音。")
ENTRY (U"从磁盘读取立体声声音")
NORMAL (U"如果您使用 @@Read from file...@ 从磁盘中将一个 @Sound 读取到 Praat 中，"
	"并且该声音文件是一个双声道文件，那么生成的 Sound 对象将是一个立体声 Sound。")
NORMAL (U"如果您想单独查看左声道或右声道，"
	"可以使用 @@Extract one channel...@。")
ENTRY (U"将立体声声音保存到磁盘")
NORMAL (U"如果您选择 @@Save as WAV file...@，并且该声音是一个立体声声音，"
	"那么生成的声音文件将是一个双声道文件。"
	"如果您有两个或多个单声道声音，并且想要将它们组合成一个立体声声音，"
	"可以使用 @@Sounds: Combine to stereo@。")
MAN_END

MAN_BEGIN (U"time", U"ppgb", 20110128)
INTRO (U"在日常生活中，时间是指手表上显示的是几点钟。")
NORMAL (U"在 Praat 中，这个定义基本上是无关紧要的。"
	"音频文件很少能告诉我们录音 of recording 的绝对时间。"
	"因此，当您将声音文件读取到 Praat 并点击 ##View & Edit# 时，您会看到"
	"该 Sound 从 0 秒开始，如果其时长为 3.5 秒，"
	"您会看到该 Sound 在 3.5 秒处结束。")
NORMAL (U"除了声音，Praat 中的许多其他类型的对象也具有时间轴："
	"声谱图、音高曲线、共振峰曲线、点过程等等。这些都"
	"不要求其时间域从 0 秒开始。例如，在声音编辑器窗口中，"
	"您可以选择 1.4 到 1.7 秒之间的部分，并将其“提取”到对象窗口，"
	"同时选择“保留时间”（preserving the times）。生成的 Sound 对象将具有 1.4 秒的开始时间"
	"和 1.7 秒的结束时间，当您点击 ##View & Edit# 时即可看到。"
	"您从该声音创建的声谱图和音高曲线也将具有 1.4 到 1.7 秒的"
	"时间域。如果您将这些对象保存为文本文件"
	"或二进制文件并在稍后重新读取到 Praat 中，该时间域将被保留。只有当您将 Sound 对象"
	"保存为音频文件（WAV、AIFF）时，该文件才不会保留时间信息；"
	"如果您再次将该音频文件读取到 Praat 中，新的 Sound 对象的时间域"
	"将从 0 到 0.3 秒运行。")
NORMAL (U"为了防止混淆，Praat 始终要求时间以秒为单位表示。"
	"因此，如果您想提供 5 毫秒（5 ms）的窗口长度，请输入 0.005 或 5e-3。"
	"对于 83.2 微秒（83.2 μs），您可以输入 0.0000832，或者最好输入 83.2e-6 或 8.32e-5。")
NORMAL (U"在时钟上，时间是循环运转的。但在 Praat 的编辑器窗口中，时间是从左向右运行的。"
	"您通常在窗口中只能看到时间轴的一部分。"
	"要查看其他部分，您可以向后或向前%滚动%。")
MAN_END

MAN_BEGIN (U"time domain", U"ppgb", 20221202)
INTRO (U"本手册页面假定您已阅读过 @Intro。")
NORMAL (U"Praat 中的许多对象都是%%时间的函数%。例如： "
	"@Sound、@Pitch、@Spectrogram、@Formant、@Intensity、@TextGrid、"
	"@PitchTier、@DurationTier、@Harmonicity、@PointProcess。")
NORMAL (U"在 Praat 中，这些函数具有一个连续的 ##time domain#（时间域），即"
	"带有 @@start time|开始时间@ 和 @@end time|结束时间@ 的单一时间跨度。"
	"此类函数的 @@total duration|总时长@ 是开始时间"
	"与结束时间之差。最多有五种方法可以查看一个对象"
	"的时间域。")
ENTRY (U"编辑器窗口中的时间域")
NORMAL (U"如果您选择一个作为时间函数的对象并点击 ##View & Edit#，屏幕上将出现一个编辑器窗口。"
	"底部的长条区域将显示开始时间、结束时间以及总时长。")
ENTRY (U"Picture 窗口中的时间域")
NORMAL (U"如果您选择一个作为时间函数的对象并选择其中的一个 #Draw 命令，"
	"弹出的窗口将邀请您提供一个时间范围。如果您将此时间范围保持为"
	"其标准设置（从 0.0 到 0.0 秒），Praat 将绘制整个时间域的对象，"
	"并在水平轴下方打印开始时间和结束时间（如果开启了 #Garnish 装饰）：")
PICTURE (5, 2.5, draw_TimeDomain_Sound)
PICTURE (5, 2.5, draw_TimeDomain_Pitch)
ENTRY (U"Info 窗口中的时间域")
NORMAL (U"如果您选择一个作为时间函数的对象并点击 #Info，"
	"信息窗口将告诉您开始时间、结束时间以及总时长（以及该对象的其他属性）。")
ENTRY (U"时间域查询命令")
NORMAL (U"如果您选择一个作为时间函数的对象，"
	"以下三个命令将在 @@Query submenu|查询子菜单@ 中变得可用：")
LIST_ITEM (U"##Get start time")
LIST_ITEM (U"##Get end time")
LIST_ITEM (U"##Get total duration")
NORMAL (U"如果您选择这些命令之一，信息窗口将告诉您结果，"
	"以秒为单位表示。这些命令在 Praat 脚本中最有用。示例：")
CODE (U"selectObject: \"Pitch hello\"")
CODE (U"startTime = Get start time")
CODE (U"endTime = Get end time")
CODE (U"centreTime = (startTime + endTime) / 2")
CODE (U"writeInfoLine: \"This Pitch runs from \", startTime, \" to \", endTime, \" seconds,\"")
CODE (U"appendInfoLine: \"and the centre of its time domain is at \", centreTime, \" seconds.\"")
ENTRY (U"供黑客了解的细节")
NORMAL (U"如果您选择一个作为时间函数的对象并点击 @Inspect，"
	"您可以看到时间域信息是如何存储在对象中的："
	"开始时间是对象的 #xmin 属性，结束时间是其 #xmax 属性。"
	"总时长没有存储在对象中，"
	"因为它可以很容易地由 #xmax 减去 #xmin 计算得出。")
MAN_END

MAN_BEGIN (U"time selection", U"ppgb", 20220907)
INTRO (U"在 Praat 的部分编辑器（即包含与时间轴相关的数据的编辑器）中选择特定时间范围的操作方法。")
NORMAL (U"时间选区用于决定接下来要播放、复制、剪切、修改或进行属性查询的具体时间段：")
ENTRY (U"1. 如何创建选区")
NORMAL (U"最简单的方法是直接在波形或图表上按住鼠标左键并向左右 @@drag|拖动@。这与在文本编辑器中拖动鼠标选择一段文字的操作完全一致。")
ENTRY (U"2. 如何扩大或收缩选区")
NORMAL (U"最简单的方法是按住 Shift 键并点击鼠标。此时，现有选区距离点击位置最近的那个边缘会自动移动到您点击的时刻。这同样类似于文本编辑器中按住 Shift 键调整选区范围的操作。")
NORMAL (U"例如：如果当前选中的时间范围是 2 到 5 秒，而您按住 Shift 键点击了 4 秒的位置，那么选区的终点就会从 5 秒处缩回至 4 秒处，从而收缩了选区范围。")
NORMAL (U"您也可以按住 Shift 键并 @@Shift-drag|拖动@ 鼠标，即按住 Shift 键的同时按下鼠标左键并拖动鼠标。")
ENTRY (U"3. 其他技巧")
NORMAL (U"若要精确定位到特定的时间范围，或者快速将选区折叠到起点或终点，可以使用 Time（时间）菜单。")
MAN_END

MAN_BEGIN (U"total duration", U"ppgb", 20040505)
INTRO (U"- @@time domain|时间域@（见该词条）的范围（跨度）。")
MAN_END

MAN_BEGIN (U"vector peak interpolation", U"ppgb", 20200912 20250418)
INTRO (U"一种用于在采样信号中寻找极大值或极小值的算法。")
ENTRY (U"概述")
NORMAL (U"该信号由序列 %y__%i_, %i = 1...%n 描述，其中 %n 是样本数。"
	"每个样本 %i 都与一个 %x 值（通常是时间）相关联，由 %x__%i_ = %x__1_ + (%i - 1) %dx 给出，"
	"其中 %dx 是采样周期。")
NORMAL (U"在以下两类位置寻找极值（以极大值为例）：")
LIST_ITEM (U"1. 左边缘和右边缘处，即在 %i = 1 处和在 %i = %n 处。")
LIST_ITEM (U"2. 所有局部极大值处或%其附近%，即在满足 %y__%i-1_ < %y__%i_ ≤ %y__%i+1_ 的那些 %i 处或%其附近%。")
NORMAL (U"因此，下列各值中最大的一项将是整个序列的最大值：")
LIST_ITEM (U"1. %y__1_。")
LIST_ITEM (U"2. 局部极大值，位于满足 %y__%i-1_ < %y__%i_ ≤ %y__%i+1_ 的 %y__%i_ 处或%其附近%。")
LIST_ITEM (U"3. %y__%n_。")
NORMAL (U"我们现在来看看%附近%意味着什么。结果的精度取决于该算法的%%interpolation method%（插值方法）。")
ENTRY (U"1. 最低精度：四舍五入到样本点")
NORMAL (U"如果插值方法为 “none”（无），则局部极大值位于满足 %y__%m-1_ < %y__%m_ ≤ %y__%m+1_ 的样本 %m 处。"
	"因此，它们的 %x 值在 %x__%m_ = %x__1_ + (%m - 1) %dx 处，其 %y 值为 %y__%m_。")
NORMAL (U"这种精度适用于无序的值序列 %y__%i_：结果仅仅是最大的可用值。")
ENTRY (U"2. 中等精度：抛物线插值")
NORMAL (U"如果插值方法是 “parabolic”（抛物线），该算法在每个局部极大值 %y__%m_ 的两侧各使用一个点"
	"来估计局部极大值的位置和大小。因为泰勒展开式表明任何光滑曲线"
	"在任何局部极大值附近都可以近似为抛物线，所以极大值位置 %x__%max_ 和极大值 %y__%max_ 可以通过"
	"以下步骤找到：")
EQUATION (U"%dy ≡ 1/2 (%y__%m+1_ - %y__%m-1_)")
EQUATION (U"%d^2%y ≡ (%y__%m_ - %y__%m-1_) + (%y__%m_- %y__%m+1_)")
EQUATION (U"%m′ ≡ %m + %dy/%d^2%y")
EQUATION (U"%x__%max_ = %x__1_ + (%m′ - 1) %dx")
EQUATION (U"%y__%max_ = %y__%m_ + 1/2 %dy^2 / %d^2%y")
NORMAL (U"如果 %y 被认为是 %x 的光滑函数，则这种精度是合适的，例如在：")
LIST_ITEM (U"@@Formant: Get minimum...")
LIST_ITEM (U"@@Formant: Get time of minimum...")
LIST_ITEM (U"@@Formant: Get maximum...")
LIST_ITEM (U"@@Formant: Get time of maximum...")
LIST_ITEM (U"@@Intensity: Get minimum...")
LIST_ITEM (U"@@Intensity: Get time of minimum...")
LIST_ITEM (U"@@Intensity: Get maximum...")
LIST_ITEM (U"@@Intensity: Get time of maximum...")
ENTRY (U"3. 较高精度：三次插值")
NORMAL (U"如果插值方法为 “cubic”（三次），则插值在四个点上进行（参见 @@vector value interpolation|向量值插值@）。"
	"其结果类似于抛物线插值方法，但如果您希望像 ##Get maximum...# 这样的命令的结果"
	"等同于 ##Get time of maximum...# 和 ##Get value at time...# 这一系列命令的执行结果，"
	"则可以使用它（或 sinc 插值）。")
ENTRY (U"4. 最高精度：sinc 插值")
NORMAL (U"如果插值方法是 “sinc70” 或 “sinc700”，该算法会假定信号"
	"是 sinc 函数的和，因此必须考虑初始猜测值 %m 两侧的若干个点（即 70 或 700 个点）"
	"（参见 @@vector value interpolation|向量值插值@）。该算法通过 Brent 方法寻找该连续函数的最大值（参见 @@Press et al. (1992)@）。")
NORMAL (U"此方法适用于对连续信号进行低通滤波（滤波截止频率为 @@Nyquist frequency|奈奎斯特频率@）"
	"后采样得到的信号。参见：")
LIST_ITEM (U"@@Sound: Get minimum...")
LIST_ITEM (U"@@Sound: Get time of minimum...")
LIST_ITEM (U"@@Sound: Get maximum...")
LIST_ITEM (U"@@Sound: Get time of maximum...")
LIST_ITEM (U"@@Sound: Get absolute extremum...")
MAN_END

MAN_BEGIN (U"vector value interpolation", U"ppgb", 20200912)
INTRO (U"一种用于估算采样信号在指定位置的数值的算法。")
ENTRY (U"概述")
NORMAL (U"该信号由序列 %y__%i_, %i = 1...%n 描述，其中 %n 是样本数。"
	"每个样本 %i 都与一个 %x位置（通常是时间）相关联，由 %x__%i_ = %x__1_ + (%i - 1) %dx 给出，"
	"其中 %dx 是采样周期。因此，与给定时间 %x 关联的实数值样本序号为")
EQUATION (U"%s = (%x - %x__1_) / %dx + 1")
NORMAL (U"如果计算出的 %s 是整数，则 %y 的值必然是 %y__%s_。否则，估计的 %y 值 %y(%s) 必须从"
	"附近的 %y 值进行插值。结果的精度取决于该算法的%%interpolation method%（插值方法）。")
ENTRY (U"1. 最低精度：四舍五入到样本点")
NORMAL (U"如果插值方法是 “nearest”（最邻近），我们取任一方向上最近点的值：")
EQUATION (U"%near ≡ round (%s)")
EQUATION (U"%y(%s) ≈ %y__%near_")
ENTRY (U"2. 中等精度：线性插值")
NORMAL (U"如果您知道或假定点底层的函数是连续的，那么“四舍五入”插值的效果会很差，"
	"因为四舍五入的值会在样本点之间的中心点发生突变。")
NORMAL (U"对于线性插值，我们使用 %s 两侧（%left 和 %right）的已证实值：")
EQUATION (U"%s__%l_ ≡ floor (%s) ;   %s__%r_ ≡ %s__%l_ + 1")
EQUATION (U"%y(%s) ≈ %y__%l_ + (%s - %s__%l_) · (%y__%r_ - %y__%l_)")
NORMAL (U"其中 #floor (%x) 计算不大于 %x 的最大整数。这种插值是连续的。")
ENTRY (U"3. 较高精度：三次插值")
NORMAL (U"如果您知道或假定点底层的函数是%光滑%的，即其导数对每个 %x 都有定义，"
	"线性插值可能会很差，因为插值函数的导数会在每个样本点处发生突变。")
NORMAL (U"因此，紧接着更高一级的插值（“三次插值”，即 “cubic”）在样本点处是可微的。为了实现这一点，我们根据其"
	"紧邻的邻域点（即算法需要四个样本点）来定义左侧和右侧样本点处的导数 %y′__%l_ 和 %y′__%r_，"
	"这可能是通过这三个点进行抛物线插值来完成的。"
	"抛物线插值的优点是，如果底层函数可以在极值附近近似为抛物线，则极值将被正确计算"
	"（参见 @@vector peak interpolation|向量峰值插值@）。")
NORMAL (U"因为抛物线函数的导数是 %x 的线性函数，"
	"所以左侧和右侧样本点处的导数可以简单地估计为")
EQUATION (U"%y′__%l_ ≈ (%y__%r_ - %y__%l-1_) / 2 ;   %y′__%r_ ≈ (%y__%r+1_ - %y__%l_) / 2")
NORMAL (U"现在我们知道了 %y__%l_、%y__%r_、%y′__%l_ 和 %y′__%r_，我们可以用一个"
	"三次多项式来拟合这些值：")
EQUATION (U"%As__%l_^3 + %Bs__%l_^2 + %Cs__%l_ + %D = %y__%l_")
EQUATION (U"%As__%r_^3 + %Bs__%r_^2 + %Cs__%r_ + %D = %y__%r_")
EQUATION (U"3%As__%l_^2 + 2%Bs__%l_ + %C = %y′__%l_")
EQUATION (U"3%As__%r_^2 + 2%Bs__%r_ + %C = %y′__%r_")
NORMAL (U"如果我们将 %x 轴平移到左侧样本点，我们可以将这四个方程简化为")
EQUATION (U"%D = %y__%l_")
EQUATION (U"%A + %B + %C + %D = %y__%r_")
EQUATION (U"%C = %y′__%l_")
EQUATION (U"3%A + 2%B + %C = %y′__%r_")
NORMAL (U"所以插值数值 %y(%s) 在 %s__%l_ 和 %s__%r_ 之间的任何点 %s 估计为")
EQUATION (U"(%y′__%r_ + %y′__%l_ - 2%y__%r_ + 2%y__%l_) %φ__%l_^3 + "
	"(3%y__%r_ - 3%y__%l_ - 2%y′__%l_ - %y′__%r_) %φ__%l_^2 + %y′__%l_ %φ__%l_ + %y__%l_")
NORMAL (U"其中 %φ__%l_ ≡ %s - %s__%l_。经过重新整理得到")
EQUATION (U"%y(%s) ≈ %y__%l_ %φ__%r_ + %y__%r_ %φ__%l_ +")
EQUATION (U"- %φ__%l_ %φ__%r_ [1/2 (%y′__%r_ - %y′__%l_) + (%φ__%l_ - 1/2) "
	"(%y′__%l_ + %y′__%r_ - 2(%y__%r_ - %y__%l_))]")
NORMAL (U"其中 %φ__%r_ ≡ 1 - %φ__%l_。从这个公式我们可以看到：")
LIST_ITEM (U"1. 前两项定义了线性插值。")
LIST_ITEM (U"2. If the underlying function is linear, so that %y′__%l_ equals %y′__%r_ and both equal %y__%r_ - %y__%l_, "
	"the higher-degree terms are zero.")
LIST_ITEM (U"3. If %y′__%l_ + %y′__%r_ equals 2(%y__%r_ - %y__%l_), the third-degree term is zero, "
	"so that the interpolated function is parabolic.")
LIST_ITEM (U"4. At the left and right points, one of the %φ is 0 and the other is 1, so that at these boundary points, "
	"%y is computed with exact precision.")
ENTRY (U"4. 最高精度：sinc 插值")
NORMAL (U"如果插值方法是 “sinc70” 或 “sinc700”，该算法假定信号"
	"是 sinc 函数的和，因此必须考虑 %s 两侧的若干个点（%%interpolation depth%，即插值深度：70 或 700 个点）。")
NORMAL (U"因为插值深度必须是有限的，所以 sinc 函数之和乘以一个汉宁窗（Hanning window）：")
EQUATION (U"%s__%l_ ≡ floor (%s);   %s__%r_ ≡ %s__%l_ + 1")
EQUATION (U"%φ__%l_ ≡ %s - %s__%l_;   %φ__%r_ ≡ 1 - %φ__%l_")
EQUATION (U"%y(%s) ≈ ∑__%i=1...%N_ %y__%r-%i_ sinc (%π(%φ__%l_+%i-1)) (1/2 + 1/2 cos (%π(%φ__%l_+%i-1)/(%φ__%l_+%N))) +")
EQUATION (U"+ ∑__%i=1...%N_ %y__%l+%i_ sinc (%π(%φ__%r_+%i-1)) (1/2 + 1/2 cos (%π(%φ__%r_+%i-1)/(%φ__%r_+%N)))")
NORMAL (U"其中 sinc 函数被定义为")
EQUATION (U"sinc (0) ≡ 1;   sinc (%x) ≡ sin %x / %x   当 %x ≠ 0 时")
NORMAL (U"如果 %s 小于插值深度或大于 %n + 1 减去插值深度，"
	"插值深度将相应减小。")
NORMAL (U"此方法适用于对连续信号进行低通滤波（滤波截止频率为 @@Nyquist frequency|奈奎斯特频率@）"
	"后采样得到的信号。参见：")
LIST_ITEM (U"@@Sound: Get value at time...")
MAN_END

MAN_BEGIN (U"waveform", U"ppgb", 20060914)
INTRO (U"波形是作为时间函数的声压，或者说是作为时间函数的麦克风输出。"
	"在 Praat 中，波形显示在 Sound 窗口的上半部分（参见 @Intro）。")
MAN_END
}

/* End of file manual_glossary.cpp */
