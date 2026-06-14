/* manual_Fon.cpp
 *
 * Copyright (C) 1992-2008,2010,2011,2014-2017,2019-2025 Paul Boersma
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

void manual_Sampling_init (ManPages me);
void manual_sound_init (ManPages me);
void manual_pitch_init (ManPages me);
void manual_spectrum_init (ManPages me);
void manual_formant_init (ManPages me);
void manual_pointProcess_init (ManPages me);
void manual_annotation_init (ManPages me);
void manual_SpeechRecognizer_init (ManPages me);

void manual_Fon_init (ManPages me);
void manual_Fon_init (ManPages me) {

manual_Sampling_init (me);
manual_sound_init (me);
manual_pitch_init (me);
manual_spectrum_init (me);
manual_formant_init (me);
manual_pointProcess_init (me);
manual_annotation_init (me);
manual_SpeechRecognizer_init (me);


MAN_PAGES_BEGIN
R"~~~(
################################################################################
"Get high index from time..."
© Paul Boersma 2010-12-30

一个 @@Query submenu|查询@，向选定的层对象（@DurationTier、@IntensityTier、@PitchTier）询问
哪个点距离指定时间最近，且不早于该时间。

设置
====
##Time (s)
:	您希望获取其点索引的时间。

返回值
======
此查询返回时间大于或等于 #Time 的最早（时间最小）的点的索引。
如果没有点，则为 @undefined。
如果指定的时间大于最后一个点的时间，则为总点数加 1（向右越界）。

################################################################################
"Get low index from time..."
© Paul Boersma 2010-12-30

一个 @@Query submenu|查询@，向选定的层对象（@DurationTier、@IntensityTier、@PitchTier）询问
哪个点距离指定时间最近，且不晚于该时间。

设置
====
##Time (s)
:	您希望获取其点索引的时间。

返回值
======
此查询返回时间小于或等于 #Time 的最迟（时间最大）的点的索引。
如果没有点，则为 @undefined。
如果指定的时间小于第一个点的时间，则为 0（向左越界）。

################################################################################
"Get nearest index from time..."
© Paul Boersma 2010, 2023-04-14

一个 @@Query submenu|查询@，向选定的层对象（@DurationTier、@IntensityTier、@PitchTier）询问
哪个点最靠近指定的时间。

设置
====
##Time (s)
:	您希望获取其点索引的邻近时间。

返回值
======
此查询返回其时间最接近 #Time 的点的索引。
如果没有点，则为 @undefined。

################################################################################
"Remove point..."
© Paul Boersma 2010-12-30

从每个选中的基于时间的层对象（@DurationTier、@IntensityTier、@PitchTier）中移除一个点的命令。

设置
====
##Point number
:	您要移除的点的索引。

行为
====
如果 ##Point number#（点号）为 3，则将从该层中移除从起点开始计算的第三个点（如果存在）。

################################################################################
"Remove point near..."
© Paul Boersma 2010-12-30

从每个选中的基于时间的层对象（@DurationTier、@IntensityTier、@PitchTier）中移除一个邻近点的命令。

设置
====

##Time (s)
:	您希望在其邻近移除点的时间。

行为
====
最靠近 #Time（时间）的点（如果存在任何点）将从该层中移除。

################################################################################
"Remove points between..."
© Paul Boersma 2010-12-30

从每个选中的基于时间的层对象（@DurationTier、@IntensityTier、@PitchTier）中移除若干个点的命令。

设置
====
##From time (s), To time (s)
:	您想要在其中移除所有点的时间范围起止。

行为
====
介于 ##From time#（起始时间）和 ##To Time#（结束时间）之间（包含首尾）的所有点都将从该层中移除。

################################################################################
"AmplitudeTier"
© Paul Boersma 2007

Praat 中的@@types of objects|对象类型@之一。
一个 AmplitudeTier（振幅层）对象表示一条带有时间戳的振幅曲线，
也就是说，它包含一系列的（%time, %amplitude，即时间，振幅）点。
振幅值的单位为帕斯卡（Pascal）。要查看一些应用，请参考 @IntensityTier（音强层）的信息；
AmplitudeTier 与 IntensityTier 的区别在于，前者的值是以帕斯卡为单位的，
它们与 Sound（例如）进行线性相乘，而后者的值是以分贝（dB）为单位的，
它们与 Sound 进行对数相乘。

################################################################################
"AmplitudeTier: Add point..."
© Paul Boersma 2023

向每个选中的 @AmplitudeTier（振幅层）添加一个点的命令。

设置
====
##Time (s)
: 要添加的点的时间。

##Sound pressure (Pa)
: 请求的新点振幅值（声压）。

行为
====
该层将被修改以包含新添加的点。
如果该层在指定时间已存在一个点，则不执行任何操作。

################################################################################
"Cochleagram"
© Paul Boersma 2003-03-16

Praat 中的@@types of objects|对象类型@之一。它表示内耳中耳蜗基底膜的激发模式
（参见 @Excitation）随时间变化的关系。

################################################################################
"Cochleagram: Formula..."
© Paul Boersma 2002-12-06

用于更改所有选定 @Cochleagram 对象中数据的命令。
示例和说明请参见 @Formulas 教程。

################################################################################
"Create AmplitudeTier..."
© Paul Boersma 2023

@@New menu|新建菜单@ 中的一个命令，用于创建一个空的 @AmplitudeTier（振幅层）对象。
生成的对象将具有指定的名称和时间域，但不包含任何振幅点。
要向其中添加一些点，请使用 @@AmplitudeTier: Add point...@。

################################################################################
"Create DurationTier..."
© Paul Boersma 2014-04-21, 2023-04-30

@@New menu|新建菜单@ 中的一个命令，用于创建一个空的 @DurationTier（时长层）对象。
生成的对象将具有指定的名称和时间域，但不包含任何时长点。
要向其中添加一些点，请使用 @@DurationTier: Add point...@。

示例
====
要创建一个长度为 0.9 秒、且在 0.6 秒附近有减速的时长层，
您可以做以下操作（无论是在脚本中还是手动选择这 4 个命令）：
{
	\#{Create DurationTier:} “dur”, 0, 0.9
	\@{DurationTier: ||Add point:} 0.3, 1
	Add point: 0.6, 2.3
	Add point: 0.7, 1
}
其结果看起来像：
{- 5x2.5
	x# = { 0.0, 0.3, 0.6, 0.7, 0.9 }
	y# = { 1.0, 1.0, 2.3, 1.0, 1.0 }
	Red
	Create Polygon from values: "durationTier", x#, y#
	Draw: 0.0, 0.9, 0.0, 2.5
	Black
	Draw inner box
	Marks left every: 1.0, 1.0, "yes", "yes", "no"
	Marks bottom every: 1.0, 0.1, "yes", "yes", "no"
	Text bottom: "yes", "Time (s)"
	Text left: "yes", "Relative duration"
}
目标时长将是该曲线下方的面积，即 0.9 + 1/2 · 1.3 · 0.4 = 1.16 秒。

################################################################################
"Create IntensityTier..."
© Paul Boersma 2002-12-04

@@New menu|新建菜单@ 中的一个命令，用于创建一个空的 @IntensityTier（音强层）对象。
生成的对象将具有指定的名称和时间域，但不包含任何音强点。
要向其中添加一些点，请使用 @@IntensityTier: Add point...@。

################################################################################
"Create Matrix..."
© Paul Boersma 2002-12-12

@@New menu|新建菜单@ 的 #Generics（通用）子菜单下的 #Matrix（矩阵）子菜单中的一个命令，
用于创建一个具有指定采样属性的 @Matrix（矩阵），
并用公式中的值进行填充（参见 @@Matrix: Formula...@）。

################################################################################
"Create simple Matrix..."
© Paul Boersma 2002-12-04

@@New menu|新建菜单@ 的 #Generics（通用）子菜单下的 #Matrix（矩阵）子菜单中的一个命令，
用于创建一个具有指定行数和列数的 @Matrix（矩阵），
并用公式中的值进行填充（参见 @@Matrix: Formula...@）。

################################################################################
"Create simple Matrix from values..."
© Paul Boersma 2023

@@New menu|新建菜单@ 的 #Generics（通用）子菜单下的 #Matrix（矩阵）子菜单中的一个命令，
用于根据指定单元格的值创建一个 @Matrix（矩阵）。

################################################################################
"Create Photo..."
© Paul Boersma 2023

@@New menu|新建菜单@ 的 #Generics（通用）子菜单下的 #Photo（图像）子菜单中的一个命令，
用于创建一个具有指定采样属性的 @Photo（图像），
且其三个颜色通道分别使用公式中的值进行填充。

################################################################################
"Create simple Photo..."
© Paul Boersma 2023

@@New menu|新建菜单@ 的 #Generics（通用）子菜单下的 #Photo（图像）子菜单中的一个命令，
用于创建一个具有指定行数和列数的 @Photo（图像），
且其三个颜色通道分别使用公式中的值进行填充。

################################################################################
"Create Strings as folder list..."
© Paul Boersma 2006（使用“directory list”），2020，2024

@@New menu|新建菜单@ 中的一个命令，用于创建一个包含给定父文件夹中所有子文件夹列表的 @Strings（字符串列表）对象。
它的工作方式与 @@Create Strings as file list...@ 完全类似。

脚本用法
========
参见 @@Create Strings as file list...@。
如果您不需要最终生成的 @Strings 对象，
使用 @`folderNames$#` 可能更容易。

################################################################################
"Create Strings as directory list..."

@@Create Strings as folder list...@ 的同义词。

您可能在旧的脚本中看到过这个命令，但它已不在 Praat 的菜单中出现。
保留此命令是为了向后兼容。

################################################################################
"Create Strings as file list..."
© Paul Boersma 1998,2006,2013-2015,2020,2024,2025

@@New menu|新建菜单@ 中的一个命令，用于创建一个包含给定文件夹中所有文件列表的 @Strings（字符串列表）对象。

设置
====
{- 5.4x2.44
)~~~"
	Manual_DRAW_SETTINGS_WINDOW ("Create Strings as file list", 2.6)
	Manual_DRAW_SETTINGS_WINDOW_FIELD ("Name", "fileList")
	Manual_DRAW_SETTINGS_WINDOW_TEXT ("File path", "/Users/miep/Sounds/*.wav")
R"~~~(
}

##Name
: 生成的 Strings（字符串列表）对象的名称。

##File path
: 文件夹路径，可使用可选的%通配符（见下文）来筛选文件。

行为
====
生成的 Strings 对象将包含文件名的按字母排序列表（通过简单的 Unicode 排序），
不包含文件夹结构中的前导路径。如果没有与 file path 匹配的文件，
生成的 Strings 对象将不包含任何字符串。

用法
====
指定文件路径有两种方式。

第一种方式是仅指定一个文件夹名称。例如，在 Unix 上，文件路径可以是
`/usr/people/miep/sounds` 或 `/usr/people/miep/sounds/`。在 Windows 上，
为 `C:\Users\Miep\Sounds` 或 `C:\Users\Miep\Sounds\`。
在 Macintosh 上，为 `/Users/miep/Sounds` 或 `/Users/miep/Sounds/`。这些路径中的任何一个
都将生成指定文件夹中所有文件的列表。

另一种方式是在文件名中指定一个%通配符（星号）。
要获取所有以“`hal`”开头且以“`.wav`”结尾的文件列表，
请输入 `/usr/people/miep/sounds/hal*.wav`、`C:\Users\Miep\Sounds\hal*.wav`
或 `/Users/miep/Sounds/hal*.wav`。

您甚至可以使用%两个通配符：`/usr/people/miep/sounds/*al*.wav` 会为您生成一个
文件名中包含 `al` 且以 `wav` 结尾的所有文件列表。

脚本用法
========
在脚本中，您可以使用此命令循环遍历文件夹中的文件。
例如，要读取指定文件夹中的所有声音文件，
您可以使用以下脚本：
{;
	folder$ = “/usr/people/miep/sounds”
	strings = \#{Create Strings as file list:} “list”, folder$ + “/*.wav”
	numberOfFiles = Get number of strings
	for ifile to numberOfFiles
		\`{selectObject}: strings
		fileName$ = Get string: ifile
		\@{Read from file:} folder$ + “/” + fileName$
	endfor
	removeObject: strings
}
如果该脚本已保存到脚本文件中，您可以使用相对于保存脚本的文件夹的相对文件路径。因此，通过使用：
{;
	\#{Create Strings as file list:} “list”, “*.wav”
}
您会得到与包含此行的脚本所在的同一文件夹中的所有 `.wav` 文件的列表。
而要得到与您的脚本所在的同一文件夹下的 `Sounds` 文件夹中所有 `.wav` 文件的列表，
您可以这样做：
{;
	\#{Create Strings as file list:} “list”, “Sounds/*.wav”
}
与 Praat 脚本中的通常做法一样，此示例中的正斜杠（“/”）可用于包括 Windows 在内的所有平台。
这使您的脚本具有跨平台可移植性。

注意，上述功能也可以使用内置函数编写，从而省去四行代码：
{;
	folder$ = “/usr/people/miep/sounds”
	list$# = \#`{fileNames$#}: folder$ + “/*.wav”
	for ifile to \`{size} (list$#)
		\@{Read from file:} folder$ + “/” + list$# [ifile]
	endfor
}
这不会生成 Strings 对象。

另请参阅
========
要获取文件夹列表而非文件列表，请使用 @@Create Strings as folder list...@。

################################################################################
"Photo"
© Paul Boersma 2023

Praat 中的@@types of objects|对象类型@之一。它与 @Matrix（矩阵）对象类似，
但它具有针对红、绿、蓝三个颜色通道的单元格矩阵。

################################################################################
)~~~"
MAN_PAGES_END

MAN_BEGIN (U"Distributions", U"ppgb", 20030316)
INTRO (U"One of the @@types of objects@ in Praat. Inherits most actions from @TableOfReal.")
ENTRY (U"Actions")
LIST_ITEM (U"@@Distributions: To Strings...@")
MAN_END

MAN_BEGIN (U"Distributions: To Strings...", U"ppgb", 19971022)
INTRO (U"A command to create a @Strings object from every selected @Distributions object.")
ENTRY (U"Settings")
TERM (U"##Column number")
DEFINITION (U"the column (in the #Distributions object) that contains the distribution that you are "
	"interested in. Often the #Distributions object will only contain a single distribution, "
	"so this argument will often be 1. If the #Distributions object contains nine distributions, "
	"specify any number between 1 and 9.")
TERM (U"##Number of strings")
DEFINITION (U"the number of times a string will be drawn from the chosen distribution. "
	"This is the number of strings that the resulting @Strings object is going to contain.")
ENTRY (U"Behaviour")
NORMAL (U"Every string in the resulting #Strings object will be a row label of the #Distributions object. "
	"The number in each row at the specified column will be considered the relative frequency of "
	"occurrence of that row.")
NORMAL (U"%#Example. Suppose we have the following #Distributions:")
CODE (U"File type = \"ooTextFile\"")
CODE (U"Object class = \"Distributions\"")
CODE (U"2 (number of columns)")
CODE (U"         \"English\"  \"French\" (column labels)")
CODE (U"3 (number of rows)")
CODE (U"\"the\"     108        1.5")
CODE (U"\"a\"       58.1       33")
CODE (U"\"pour\"    0.7        15.5")
NORMAL (U"If you set %Column to 1 and %%Number of strings% to 1000, "
	"you will get a @Strings object with approximately 647 occurrences of “the”, "
	"348 occurrences of “a”, and 4 occurrences of “pour”. "
	"If you had set %Column to 2 (“French”), you would have gotten about "
	"30 times “the”, 660 times “a”, and 310 times “pour”. "
	"The actual numbers will vary because the choice of a string will not depend on previous choices.")
MAN_END

MAN_BEGIN (U"DurationTier", U"ppgb", 20030316)
INTRO (U"One of the @@types of objects@ in Praat. "
	"A DurationTier object contains a number of (%time, %duration) points, "
	"where %duration is to be interpreted as a relative duration (e.g. the duration of a manipulated sound "
	"as compared to the duration of the original). For instance, "
	"if your DurationTier contains two points, one with a duration value of 1.5 at a time of 0.5 seconds "
	"and one with a duration value of 0.6 at a time of 1.1 seconds, this is to be interpreted as "
	"a relative duration of 1.5 (i.e. a slowing down) for all original times before 0.5 seconds, "
	"a relative duration of 0.6 (i.e. a speeding up) for all original times after 1.1 seconds, "
	"and a linear interpolation between 0.5 and 1.1 seconds (e.g. a relative duration of "
	"1.2 at 0.7 seconds, and of 0.9 at 0.9 seconds).")
NORMAL (U"See @@Intro 8.2. Manipulation of duration@ and @@Create DurationTier...@.")
MAN_END

MAN_BEGIN (U"DurationTier: Add point...", U"ppgb", 20030216)
INTRO (U"A command to add a point to each selected @DurationTier. "
	"For an example, see @@Create DurationTier...@.")
ENTRY (U"Settings")
TERM (U"##Time (s)")
DEFINITION (U"the time at which a point is to be added.")
TERM (U"##Relative duration")
DEFINITION (U"the relative duration value of the requested new point.")
ENTRY (U"Behaviour")
NORMAL (U"The tier is modified so that it contains the new point. "
	"If a point at the specified time was already present in the tier, nothing happens.")
MAN_END

MAN_BEGIN (U"DurationTier: Get target duration...", U"ppgb", 20101228)
INTRO (U"A @@Query submenu|query@ to the selected @DurationTier for the target duration of a specified time range.")
ENTRY (U"Settings")
TERM (U"##From time (s)")
TERM (U"##To time (s)")
DEFINITION (U"the start and end of the (original) time range.")
ENTRY (U"Return value")
NORMAL (U"the target duration in seconds.")
MAN_END

MAN_BEGIN (U"DurationTierEditor", U"ppgb", 20110128)
INTRO (U"Praat 中的@@Editors|编辑器@之一，用于查看和编辑 @@DurationTier|DurationTier@（时长层）对象。要创建 DurationTierEditor 窗口，请选择一个 DurationTier 对象并点击 ##View & Edit#。")
MAN_END

MAN_BEGIN (U"Editors", U"ppgb", 20110128)
INTRO (U"Praat 中的许多@@types of objects|对象类型@都可以在各自专属的窗口中进行查看和编辑。")
ENTRY (U"编辑器窗口")
LIST_ITEM (U"• @@SoundEditor|SoundEditor (声音编辑器)@")
LIST_ITEM (U"• @@LongSoundEditor|LongSoundEditor (长声音编辑器)@")
LIST_ITEM (U"• @@TextGridEditor|TextGridEditor (TextGrid 编辑器)@")
LIST_ITEM (U"• @@ManipulationEditor|ManipulationEditor (操纵编辑器)@")
LIST_ITEM (U"• @@SpectrumEditor|SpectrumEditor (频谱编辑器)@")
LIST_ITEM (U"• @@PitchEditor|PitchEditor (音高编辑器)@")
LIST_ITEM (U"• @@PointEditor|PointEditor (点标记编辑器)@")
LIST_ITEM (U"• @@PitchTierEditor|PitchTierEditor (音高层编辑器)@")
LIST_ITEM (U"• @@IntensityTierEditor|IntensityTierEditor (音强层编辑器)@")
LIST_ITEM (U"• @@DurationTierEditor|DurationTierEditor (时长层编辑器)@")
LIST_ITEM (U"• ##SpectrogramEditor (声谱图编辑器)#")
LIST_ITEM (U"• ##ArtwordEditor (Artword 编辑器)#")
LIST_ITEM (U"• @@OTGrammarEditor|OTGrammarEditor (优选论语法编辑器)@")
LIST_ITEM (U"• (任意类型：@@Inspect|Inspect (检查)@)")
ENTRY (U"如何打开一个对象的编辑器")
NORMAL (U"要为列表中的对象打开编辑器窗口，请在列表中选中该对象并选择 ##View & Edit（查看与编辑）# "
	"（如果存在 ##View & Edit# 按钮，它通常位于 @@Dynamic menu|动态菜单@的顶部）。"
	"该对象的名称将显示为编辑器窗口的标题。")
NORMAL (U"无法被修改的对象（例如 LongSound）通常只有 #View（查看） 命令，而没有 ##View & Edit#。")
ENTRY (U"通用行为")
NORMAL (U"您在编辑器窗口中对对象所做的更改将立即生效。"
	"例如，您无需关闭编辑器窗口便可将更改后的对象保存到磁盘。")
NORMAL (U"如果您在@@List of Objects|对象列表@中将正在查看或编辑的对象@@Remove|删除@，"
	"该编辑器窗口将自动从屏幕上消失。")
NORMAL (U"所有编辑器都是独立的窗口：您可以将它们最小化或最大化；"
	"如果某个编辑器窗口隐藏在其他窗口后面，"
	"您可以通过再次选择 ##View & Edit# 命令将其置顶显示。")
NORMAL (U"如果您重命名正在查看或编辑的对象（使用 @@Rename...|重命名...@），"
	"编辑器窗口的标题将立即更改为新名称。")
ENTRY (U"控制编辑器窗口的方式")
LIST_ITEM (U"• @@Click|单击@")
LIST_ITEM (U"• @@Shift-click|Shift-单击@")
LIST_ITEM (U"• @@Drag|拖动@")
LIST_ITEM (U"• @@Shift-drag|Shift-拖动@")
LIST_ITEM (U"• @@Time selection|时间选择@")
LIST_ITEM (U"• @@Keyboard shortcuts|键盘快捷键@")
MAN_END

MAN_BEGIN (U"Excitation", U"ppgb", 20030316)
INTRO (U"One of the @@types of objects@ in Praat. It represents the excitation pattern "
	"of the basilar membrane in the inner ear.")
ENTRY (U"Inside an Excitation object")
NORMAL (U"With @Inspect, you will see the following attributes.")
TERM (U"%xmin = 0")
DEFINITION (U"minimum place or frequency (Bark).")
TERM (U"%xmax = 25.6 Bark")
DEFINITION (U"maximum place or frequency (Bark).")
TERM (U"%nx")
DEFINITION (U"number of places or frequencies.")
TERM (U"%dx = 25.6 / %nx")
DEFINITION (U"Place or frequency step (Bark).")
TERM (U"%x1 = %dx / 2")
DEFINITION (U"centre of first place or frequency band (Bark).")
TERM (U"%ymin = %ymax = %dy = %y_1 = 1; %ny = 1")
DEFINITION (U"dummies.")
TERM (U"%z [1]")
DEFINITION (U"intensity (sensation level) in phon.")
MAN_END

MAN_BEGIN (U"Excitation: Formula...", U"ppgb", 20021206)
INTRO (U"A command for changing the data in all selected @Excitation objects.")
NORMAL (U"See the @Formulas tutorial for examples and explanations.")
MAN_END

MAN_BEGIN (U"Excitation: Get loudness", U"ppgb", 19991016)
INTRO (U"A @@Query submenu|query@ to ask the selected @Excitation object for its loudness.")
ENTRY (U"Return value")
NORMAL (U"the loudness in sone units.")
ENTRY (U"Algorithm")
NORMAL (U"The loudness is defined as")
EQUATION (U"∫%df 2^^(%e(%f) - 40 phon) / 10^")
NORMAL (U"where %f is the frequency in Bark, and %e(%f) the excitation in phon. "
	"For our discrete Excitation object, the loudness is computed as")
EQUATION (U"Δ%f ∑ 2^^(%e_%i - 40) / 10")
NORMAL (U"where Δ%f is the distance between the excitation channels (in Bark).")
MAN_END

/*"Any object of one of the types Polygon, PointProcess, Sound, Pitch, Spectrum,\n"
"Spectrogram, Excitation, Cochleagram, VocalTract\n"
"is convertible to and from a Matrix.\n"
"This allows extra hacking and drawing possibilities.\n"
"For example, to square the y values of a Polygon, do the following:\n"
"   select a Polygon, choose ##To Matrix#, choose ##Formula...#,\n"
"   type \"self * if row=1 then 1 else self fi\", choose ##To Polygon#."*/

MAN_BEGIN (U"Formula...", U"ppgb", 19980319)
NORMAL (U"See @@Matrix: Formula...@")
MAN_END

MAN_BEGIN (U"Frequency selection", U"ppgb", 20010402)
INTRO (U"The way to select a frequency domain in the @SpectrumEditor. "
	"This works completely analogously to the @@time selection@ in other editors.")
MAN_END

MAN_BEGIN (U"Get area...", U"ppgb", 20030216)
INTRO (U"A @@Query submenu|query@ to the selected tier object (@PitchTier, @IntensityTier, @DurationTier).")
ENTRY (U"Return value")
NORMAL (U"the area under the curve.")
ENTRY (U"Settings")
TERM (U"##From time (s)")
TERM (U"##To time (s)")
DEFINITION (U"the selected time domain. Values outside this domain are ignored. "
	"If ##To time# is not greater than ##From time#, the entire time domain of the tier is considered.")
ENTRY (U"Algorithm")
NORMAL (U"The curve consists of a sequence of line segments. The contribution of the line segment from "
	"(%t__1_, %f__1_) to (%t__2_, %f__2_) to the area is")
EQUATION (U"1/2 (%f__1_ + %f__2_) (%t__2_ – %t__1_)")
MAN_END

MAN_BEGIN (U"Intensity", U"ppgb", 20030316)
INTRO (U"One of the @@types of objects@ in Praat.")
NORMAL (U"An Intensity object represents an intensity contour at linearly spaced time points "
	"%t__%i_ = %t__1_ + (%i – 1) %dt, with values in dB SPL, i.e. dB relative to 2·10^^-5^ Pascal, "
	"which is the normative auditory threshold for a 1000-Hz sine wave.")
MAN_END

MAN_BEGIN (U"Intensity: Get maximum...", U"ppgb", 20200912)
INTRO (U"A @@Query submenu|query@ to the selected @Intensity object.")
ENTRY (U"Return value")
NORMAL (U"the maximum value within the specified time domain, expressed in dB.")
ENTRY (U"Settings")
TERM (U"##Time range (s)")
DEFINITION (U"the time range (%t__1_, %t__2_). Values outside this range are ignored. "
	"If %t__1_ is not less than %t__2_, the entire time domain of the Intensity is considered.")
TERM (U"%%Interpolation")
DEFINITION (U"the interpolation method (#none, #parabolic, #cubic, #sinc70, #sinc700) of the @@vector peak interpolation@. "
	"The standard is Parabolic because of the usual nonlinearity (logarithm) in the computation of intensity; "
	"sinc interpolation would be too stiff and may give unexpected results.")
MAN_END

MAN_BEGIN (U"Intensity: Get mean...", U"ppgb", 20041107)
INTRO (U"A @@Query submenu|query@ to the selected @Intensity object.")
ENTRY (U"Return value")
NORMAL (U"the mean (in dB) of the intensity values of the frames within a specified time domain.")
ENTRY (U"Settings")
TERM (U"##Time range (s)")
DEFINITION (U"the time range (%t__1_, %t__2_). Values outside this range are ignored. "
	"If %t__1_ is not less than %t__2_, the entire time domain of the Intensity is considered.")
TERM (U"##Averaging method")
DEFINITION (U"the units in which the averaging is performed. If the method is #energy, "
	"the returned dB value is based on the mean power (in Pa^2/s) between %t__1_ and %t__2_. "
	"If the method is #dB, the returned value is the mean of the intensity curve in dB. "
	"If the method is #sones, the returned value is in between these two, "
	"and based on averaging properties of the human ear.")
ENTRY (U"Algorithm")
NORMAL (U"If the averaging method is #dB, the mean intensity between the times %t__1_ and %t__2_ is defined as")
EQUATION (U"1/(%t__2_ - %t__1_)  ∫__%%t%1_^^%%t%2^ %x(%t) %dt")
NORMAL (U"where %x(%t) is the intensity as a function of time, in dB. If the method is #energy, the result is")
EQUATION (U"10 log__10_ { 1/(%t__2_ - %t__1_)  ∫__%%t%1_^^%%t%2^ 10^^%x(%t)/10^ %dt }")
NORMAL (U"If the method is #sones, the result is")
EQUATION (U"10 log__2_ { 1/(%t__2_ - %t__1_)  ∫__%%t%1_^^%%t%2^ 2^^%x(%t)/10^ %dt }")
ENTRY (U"Behaviour")
NORMAL (U"After you do @@Sound: To Intensity...@, the mean intensity of the resulting #Intensity, "
	"if the averaging method is #energy, should be close to the mean SPL of the original #Sound, "
	"which can be found with #Info.")
MAN_END

MAN_BEGIN (U"Intensity: Get minimum...", U"ppgb", 20200912)
INTRO (U"A @@Query submenu|query@ to the selected @Intensity object.")
ENTRY (U"Return value")
NORMAL (U"the minimum value within a specified time domain, expressed in dB.")
ENTRY (U"Settings")
TERM (U"##Time range (s)")
DEFINITION (U"the time range (%t__1_, %t__2_). Values outside this range are ignored. "
	"If %t__1_ is not less than %t__2_, the entire time domain of the Intensity is considered.")
TERM (U"##Interpolation")
DEFINITION (U"the interpolation method (#none, #parabolic, #cubic, #sinc70, #sinc700) of the @@vector peak interpolation@. "
	"The standard is Parabolic because of the usual nonlinearity (logarithm) in the computation of intensity; "
	"sinc interpolation would be too stiff and may give unexpected results.")
MAN_END

MAN_BEGIN (U"Intensity: Get standard deviation...", U"ppgb", 20041107)
INTRO (U"A @@Query submenu|query@ to the selected @Intensity object.")
ENTRY (U"Return value")
NORMAL (U"the standard deviation (in dB) of the intensity values of the frames within a specified time domain.")
ENTRY (U"Settings")
TERM (U"%%Time range (s)")
DEFINITION (U"the time range (%t__1_, %t__2_). Values outside this range are ignored. "
	"If %t__1_ is not less than %t__2_, the entire time domain of the Intensity is considered.")
ENTRY (U"Algorithm")
NORMAL (U"The standard deviation between the times %t__1_ and %t__2_ is defined as")
EQUATION (U"√ {1/(%t__2_ - %t__1_)  ∫__%%t%1_^^%%t%2^ %dt (%x(%t) - %μ)^2}")
NORMAL (U"where %x(%t) is the intensity (in dB) as a function of time, and %μ its mean. "
	"For our discrete Intensity object, the standard deviation is approximated by")
EQUATION (U"√ {1/(%n-1) ∑__%i=%m..%m+%n-1_ (%x__%i_ - %μ)^2}")
NORMAL (U"where %n is the number of frames between %t__1_ and %t__2_. Note the \"minus 1\".")
MAN_END

MAN_BEGIN (U"Intensity: Get time of maximum...", U"ppgb", 20200912)
INTRO (U"A @@Query submenu|query@ to the selected @Intensity object.")
ENTRY (U"Return value")
NORMAL (U"the time (in seconds) associated with the maximum intensity within a specified time domain.")
ENTRY (U"Settings")
TERM (U"%%Time range (s)")
DEFINITION (U"the time range (%t__1_, %t__2_). Values outside this range are ignored, except for purposes of interpolation. "
	"If %t__1_ is not less than %t__2_, the entire time domain of the Intensity is considered.")
TERM (U"%%Interpolation")
DEFINITION (U"the interpolation method (#none, #parabolic, #cubic, #sinc70, #sinc700) of the @@vector peak interpolation@. "
	"The standard is Parabolic because of the usual nonlinearity (logarithm) in the computation of intensity; "
	"sinc interpolation would be too stiff and may give unexpected results.")
MAN_END

MAN_BEGIN (U"Intensity: Get time of minimum...", U"ppgb", 20200912)
INTRO (U"A @@Query submenu|query@ to the selected @Intensity object.")
ENTRY (U"Return value")
NORMAL (U"the time (in seconds) associated with the minimum intensity within a specified time domain.")
ENTRY (U"Settings")
TERM (U"##Time range (s)")
DEFINITION (U"the time range (%t__1_, %t__2_). Values outside this range are ignored, except for purposes of interpolation. "
	"If %t__1_ is not less than %t__2_, the entire time domain of the Intensity is considered.")
TERM (U"##Interpolation")
DEFINITION (U"the interpolation method (#none, #parabolic, #cubic, #sinc70, #sinc700) of the @@vector peak interpolation@. "
	"The standard is Parabolic because of the usual nonlinearity (logarithm) in the computation of intensity; "
	"sinc interpolation would be too stiff and may give unexpected results.")
MAN_END

MAN_BEGIN (U"Intensity: Get value at time...", U"ppgb", 20030916)
INTRO (U"A @@Query submenu|query@ to the selected @Intensity object.")
ENTRY (U"Return value")
NORMAL (U"the intensity (in dB) at a specified time. If %time is outside the frames of the Intensity, the result is 0.")
ENTRY (U"Settings")
TERM (U"##Time (s)")
DEFINITION (U"the time at which the value is to be evaluated.")
TERM (U"##Interpolation")
DEFINITION (U"the interpolation method, see @@vector value interpolation@. "
	"The standard is “cubic” because of the usual nonlinearity (logarithm) in the computation of intensity; "
	"sinc interpolation would be too stiff and may give unexpected results.")
MAN_END

MAN_BEGIN (U"Intensity: Get value in frame...", U"ppgb", 19991016)
INTRO (U"A @@Query submenu|query@ to the selected @Intensity object.")
ENTRY (U"Setting")
TERM (U"##Frame number")
DEFINITION (U"the frame whose value is to be looked up.")
ENTRY (U"Return value")
NORMAL (U"the intensity value (in dB) in the specified frame. "
	"If the index is less than 1 or greater than the number of frames, the result is 0; "
	"otherwise, it is %z [1] [%%frame number%].")
MAN_END

MAN_BEGIN (U"Intensity: To IntensityTier", U"ppgb", 19970321)
INTRO (U"A command to convert each selected @Intensity object to an @IntensityTier.")
ENTRY (U"Behaviour")
NORMAL (U"Every sample in the @Intensity object is copied to a point on the @IntensityTier.")
ENTRY (U"Postconditions")
DEFINITION (U"Equal time domains:")
LIST_ITEM (U"• %result. %xmin == %intensity. %xmin")
LIST_ITEM (U"• %result. %xmax == %intensity. %xmax")
DEFINITION (U"Equal number of points:")
LIST_ITEM (U"• %result. %points. %size == %intensity. %nx")
NORMAL (U"For all points %i = 1 ... %intensity. %nx:")
DEFINITION (U"   Explicit times:")
LIST_ITEM (U"   • %result. %points. %item [%i]. %time == %intensity. %x1 + (%i – 1) * %intensity. %dx")
DEFINITION (U"   Equal number of points:")
LIST_ITEM (U"   • %result. %points. %item [%i]. %value == %intensity. %z [1] [%i]")
MAN_END

MAN_BEGIN (U"Intensity & PointProcess: To IntensityTier...", U"ppgb", 20101230)
INTRO (U"A command to copy information from an @Intensity, at times specified by a @PointProcess, "
	"to points on an @IntensityTier.")
ENTRY (U"Behaviour")
NORMAL (U"For all the times of the points in the PointProcess, an intensity is computed from the "
	"information in the Intensity object, by linear interpolation.")
MAN_END

MAN_BEGIN (U"IntensityTier", U"ppgb", 20101230)
INTRO (U"Praat 中的@@types of objects|对象类型@之一。 "
	"一个 IntensityTier（音强层）对象表示一条带有时间戳的音强曲线，也就是说，它包含一系列的（%time, %intensity，即时间，音强）点。 "
	"音强值的单位为分贝（dB）。")
NORMAL (U"示例请参阅 @@Source-filter synthesis|源-滤波器合成@。")
ENTRY (U"IntensityTier 命令")
NORMAL (U"创建：")
LIST_ITEM (U"从头创建：")
LIST_ITEM (U"• @@Create IntensityTier...")
LIST_ITEM (U"• @@IntensityTier: Add point...")
LIST_ITEM (U"从另一个对象复制：")
LIST_ITEM (U"• @@Intensity: To IntensityTier@：复制等间距排列的音强点。")
LIST_ITEM (U"• @@Intensity & PointProcess: To IntensityTier...@：在指定的点上复制插值后的音强值。")
LIST_ITEM (U"• @@PointProcess: Up to IntensityTier...@：在指定的点上设为相等的值。")
NORMAL (U"查看与编辑：")
LIST_ITEM (U"• @IntensityTierEditor")
NORMAL (U"转换：")
LIST_ITEM (U"• @@IntensityTier: Down to PointProcess@：复制时间点。")
NORMAL (U"合成（参见 @@Source-filter synthesis|源-滤波器合成@）：")
LIST_ITEM (U"• @@Sound & IntensityTier: Multiply@")
NORMAL (U"查询：")
LIST_ITEM (U"• @@Get low index from time...")
LIST_ITEM (U"• @@Get high index from time...")
LIST_ITEM (U"• @@Get nearest index from time...")
NORMAL (U"修改：")
LIST_ITEM (U"• @@Remove point...")
LIST_ITEM (U"• @@Remove point near...")
LIST_ITEM (U"• @@Remove points between...")
LIST_ITEM (U"• @@IntensityTier: Add point...@")
MAN_END

MAN_BEGIN (U"IntensityTier: Add point...", U"ppgb", 20010410)
INTRO (U"向每个选中的 @IntensityTier（音强层）对象添加一个点的命令。")
ENTRY (U"设置")
TERM (U"##Time (s)")
DEFINITION (U"要添加的点的时间。")
TERM (U"##Intensity (dB)")
DEFINITION (U"所请求新点的音强值。")
ENTRY (U"行为")
NORMAL (U"该音强层将被修改，以包含新添加的点。 "
	"如果该音强层在指定时间已存在一个点，则不执行任何操作。")
MAN_END

MAN_BEGIN (U"IntensityTier: Down to PointProcess", U"ppgb", 20010410)
INTRO (U"将每个选中的 @IntensityTier（音强层）降级为 @PointProcess（点过程）对象的命令。")
ENTRY (U"行为")
NORMAL (U"所有点的时间点将被简单地复制，时间域也同样被复制。音强信息将会丢失。")
MAN_END

MAN_BEGIN (U"IntensityTierEditor", U"ppgb", 20110128)
INTRO (U"Praat 中的@@Editors|编辑器@之一，用于查看和编辑 @@IntensityTier|IntensityTier@（音强层）对象。要创建 IntensityTierEditor 窗口，请选择一个 IntensityTier 对象并点击 ##View & Edit#。")
MAN_END

MAN_BEGIN (U"Command-click", U"ppgb", 20210228)
INTRO (U"%%Command-click%（按住 Command/Ctrl 键单击）是指在按下 %%Command key% 的同时单击鼠标。"
	"Command 键在苹果（Apple）键盘上是标有苹果标志或“command”字样的键；"
	"如果您使用的是 Windows 或 Linux 电脑，则对应的是键盘上的“Ctrl”键。")
MAN_END

MAN_BEGIN (U"Keyboard shortcuts", U"ppgb", 20220701)  // 2026
INTRO (U"为了在 Praat 中加快菜单命令的执行速度，您有时可以使用键盘来选择它们，"
	"有时在按住 Command 键、Option 键或 Shift 键的同时按下相应按键。"
	"所有这些命令也可以从菜单中进行选择。")
NORMAL (U"当提到 %%Command 键% 时，本手册在苹果（Apple）键盘上是指标有苹果标志"
	"或“command”字样的键；"
	"如果您使用的是 Windows 或 Linux 电脑，则指的是键盘上的“Ctrl”键。")
NORMAL (U"当提到 %%Option 键% 时，本手册在苹果键盘上是指标有"
	"“option”字样的键，在 Windows 或 Linux 上是指标有“Alt”字样的键。")
NORMAL (U"当提到 %%Extra-Command 键% 时，本手册指的是"
	"同时按下“Command”和“Shift”键（在 Windows 上）"
	"或同时按下“Command”和“Option”键（在 Mac 上）。"
	"在 Praat 中，Extra-Command 有时用于执行"
	"与仅使用 Command 键调用的操作相反的破坏性操作。"
	"例如，如果 Command-T表示“在光标位置添加目标点”，"
	"则 Extra-Command-T 可能表示“移除选中的目标点”。")
ENTRY (U"快捷键")
LIST_ITEM (U"Command-A: Zoom all（全部缩放）")
LIST_ITEM (U"Command-C: Copy（复制选中的文本、选中的声音或 Picture 窗口的选中部分）")
LIST_ITEM (U"Command-D（在 TextGrid 窗口中）：Align interval（对齐区间）")
LIST_ITEM (U"Command-D（在操纵对象窗口中）：Insert duration point at cursor（在光标处插入时长点）")
LIST_ITEM (U"Extra-Command-D（在操纵对象窗口中）：Remove selected duration points（移除选中的时长点）")
LIST_ITEM (U"Command-E（在 Picture 窗口中）：Erase all（擦除全部）")
LIST_ITEM (U"Command-E（在优选论窗口中）：Edit ranking（编辑排序）")
LIST_ITEM (U"Command-F: Find（查找）")
LIST_ITEM (U"Extra-Command-F: Replace（替换，是的，这是破坏性的……）")
LIST_ITEM (U"Command-G: Find again（再次查找）")
LIST_ITEM (U"Extra-Command-G: Replace again（再次替换，是的，这是破坏性的……）")
LIST_ITEM (U"Command-H（在脚本窗口中）：Paste history（粘贴历史记录）")
LIST_ITEM (U"Extra-Command-H: Move cursor to maximum pitch（将光标移至最大音高处）")
LIST_ITEM (U"Command-I: Zoom in（放大）")
LIST_ITEM (U"Command-L（在对象窗口中）：@@Open long sound file...@")
LIST_ITEM (U"Command-L（在声音窗口中）：@@Intro 3.6. Viewing a spectral slice|View spectral slice（查看频谱切片）@")
LIST_ITEM (U"Extra-Command-L: Move cursor to minimum pitch（将光标移至最小音高处）")
LIST_ITEM (U"Command-M: Search Praat manual...（搜索 Praat 手册...）")
LIST_ITEM (U"Command-N: Zoom to selection（缩放到选区）")
LIST_ITEM (U"Command-O（在对象窗口中）：@@Read from file...@")
LIST_ITEM (U"Command-O（在声音窗口中）：Zoom out（缩小）")
LIST_ITEM (U"Command-P（在 Picture 窗口中）：Print（打印）")
LIST_ITEM (U"Command-P（在 PointProcess 窗口中）：Add point at cursor（在光标处添加点）")
LIST_ITEM (U"Command-P（在操纵对象窗口中）：Add pulse at cursor（在光标处添加脉冲）")
LIST_ITEM (U"Extra-Command-P（在操纵对象窗口中）：Remove selected pulses（移除选中的脉冲）")
LIST_ITEM (U"Command-Q: Quit Praat（退出 Praat）")
LIST_ITEM (U"Command-R（在脚本窗口中）：Run（运行）")
LIST_ITEM (U"Command-R: Reverse selection（反转选区）")
LIST_ITEM (U"Command-S: Save（保存）")
LIST_ITEM (U"Command-T（在脚本窗口中）：Run selection（运行选中部分）")
LIST_ITEM (U"Command-T（在 TextGrid 窗口中）：Transcribe interval（转录区间）")
LIST_ITEM (U"Command-T（在音高层/时长层/实数层/共振峰网格窗口中）：Add point at cursor（在光标处添加点）")
LIST_ITEM (U"Command-T（在操纵对象窗口中）：Add pitch point at cursor（在光标处添加音高点）")
LIST_ITEM (U"Extra-Command-T（在操纵对象窗口中）：Remove selected pitch points（移除选中的音高点）")
LIST_ITEM (U"Command-U: @@Calculator...@（计算器）")
LIST_ITEM (U"Command-V（在文本窗口或 TextGrid 窗口中）：Paste（在选中文本上覆盖插入粘贴）")
LIST_ITEM (U"Command-V（在声音窗口中）：Paste after selection（在选区后粘贴，在选中声音后插入声音剪贴板内容）")
LIST_ITEM (U"Extra-Command-V（在声音窗口中）：Paste over selection（在选区上覆盖粘贴，在选中声音上覆盖插入声音剪贴板内容）")
LIST_ITEM (U"Command-W: Close window（关闭窗口）")
LIST_ITEM (U"Command-X: Cut（剪切选中的文本或选中的声音）")
LIST_ITEM (U"Command-Y: Redo（重做）")
LIST_ITEM (U"Command-Z: Undo（撤销）")
LIST_ITEM (U"Command-0（在声音窗口中）：Move cursor to nearest zero crossing（将光标移至最近的过零点）")
LIST_ITEM (U"Command-0（在优选论窗口中）：Evaluate with zero noise（在无噪声下评估）")
LIST_ITEM (U"Command-2（在操纵对象窗口中）：Stylize pitch (2 semitones)（音高简化为2半音）")
LIST_ITEM (U"Command-2（在优选论窗口中）：Evaluate with noise 2.0（以噪声2.0评估）")
LIST_ITEM (U"Command-4（在操纵对象窗口中）：Interpolate quadratically (4 points)（以4个点进行二次插值）")
LIST_ITEM (U"Shift-Command-?: Local help（本地帮助）")
LIST_ITEM (U"Command-,: Move start of selection to nearest zero crossing（将选区起点移至最近的过零点）")
LIST_ITEM (U"Command-.: Move end of selection to nearest zero crossing（将选区终点移至最近的过零点）")
LIST_ITEM (U"Command-F1: Formant listing（列出共振峰）")
LIST_ITEM (U"F1: Get first formant（获取第一共振峰）")
LIST_ITEM (U"F2: Get second formant（获取第二共振峰）")
LIST_ITEM (U"F3: Get third formant（获取第三共振峰）")
LIST_ITEM (U"F4: Get fourth formant（获取第四共振峰）")
LIST_ITEM (U"在 Windows 和 Linux 上：F5: Get pitch（获取音高）")
LIST_ITEM (U"在 Windows 和 Linux 上：Command-F5: Pitch listing（列出音高）")
LIST_ITEM (U"在 Windows 和 Linux 上：Option-F5: Get minimum pitch（获取最小音高）")
LIST_ITEM (U"在 Windows 和 Linux 上：Shift-F5: Get maximum pitch（获取最大音高）")
LIST_ITEM (U"F6: Get cursor（获取光标位置）")
LIST_ITEM (U"F7: Get spectral power at cursor cross（获取光标交叉处的频谱功率）")
LIST_ITEM (U"F8: Get intensity（获取音强）")
LIST_ITEM (U"Command-F8: Intensity listing（列出音强）")
LIST_ITEM (U"Option-F8: Get minimum intensity（获取最小音强）")
LIST_ITEM (U"Shift-F8: Get maximum intensity（获取最大音强）")
LIST_ITEM (U"Command-F9: Pulse listing（列出脉冲）")
LIST_ITEM (U"在 Mac 上：F10: Get pitch（获取音高）")
LIST_ITEM (U"在 Mac 上：Command-F10: Pitch listing（列出音高）")
LIST_ITEM (U"在 Mac 上：Option-F10: Get minimum pitch（获取最小音高）")
LIST_ITEM (U"在 Mac 上：Shift-F10: Get maximum pitch（获取最大音高）")
LIST_ITEM (U"F11: Voice report（嗓音报告）")
LIST_ITEM (U"F12: Log 1（记录 1）")
LIST_ITEM (U"Shift-F12: Log 2（记录 2）")
LIST_ITEM (U"Option-F12: Log script 3（记录脚本 3）")
LIST_ITEM (U"Command-F12: Log script 4（记录脚本 4）【注：Command-Fxx 组合键并非在所有计算机上都对应用程序可用】")
LIST_ITEM (U"Tab（在声音窗口中）：Play selection（播放选区）")
LIST_ITEM (U"Shift-Tab（在声音窗口中）：Play window（播放整个窗口）")
LIST_ITEM (U"Arrow-up（在声音窗口中）：Select earlier（选择更早部分）")
LIST_ITEM (U"Arrow-down（在声音窗口中）：Select later（选择更晚部分）")
LIST_ITEM (U"Shift-Arrow-up（在声音窗口中）：Move start of selection left（将选区起点向左移动）")
LIST_ITEM (U"Shift-Arrow-down（在声音窗口中）：Move start of selection right（将选区起点向右移动）")
LIST_ITEM (U"Command-Arrow-up（在声音窗口中）：Move end of selection left（将选区终点向左移动）")
LIST_ITEM (U"Command-Arrow-down（在声音窗口中）：Move end of selection right（将选区终点向右移动）")
LIST_ITEM (U"Page-up（在声音窗口中）：Scroll page back（向前翻页）")
LIST_ITEM (U"Page-down（在声音窗口中）：Scroll page forward（向后翻页）")
LIST_ITEM (U"Escape: Interrupt playing（中断播放）")
MAN_END

MAN_BEGIN (U"Log files", U"ppgb", 20230122)
INTRO (U"利用 @SoundEditor（声音编辑器）和 @TextGridEditor 中的 @@Analyses menu|分析菜单@ 中的一些命令，"
	"您可以将关于时间、音高值、共振峰和音强的组合信息"
	"输出到 @@Info window|信息窗口@ 和/或日志文件中。")
NORMAL (U"日志文件是磁盘上的一个文本文件。它由若干行格式相似的内容组成，"
	"其格式由分析菜单中的日志设置（log settings）决定。")
NORMAL (U"每当您按下 @@Keyboard shortcuts|F12@（或在分析菜单中选择 ##Log 1# 选项）时，"
	"Praat 将会向信息窗口和/或日志文件 1 写入一行内容。"
	"如果您按下 @@Keyboard shortcuts|Shift-F12@（或在分析菜单中选择 ##Log 2#），"
	"Praat 将会向信息窗口 and/or to log file 2 写入一行内容。")
NORMAL (U"在 ##log settings#（日志设置）窗口中，您可以指定以下设置：")
TERM (U"##Write log 1 to")
DEFINITION (U"这决定了您的日志行是仅写入日志文件 1、仅写入信息窗口，还是同时写入两者。")
TERM (U"##Log file 1")
DEFINITION (U"日志文件的名称。单击 Browse（浏览）选择要写入的文件。"
	"在大多数平台上，您也可以使用相对于主目录的名称，例如 `~/Desktop/pitchLog.txt`。")
TERM (U"##Log 1 format")
DEFINITION (U"Praat 将写入的行的格式。见下文。")
NORMAL (U"对于日志文件 2 也是如此。")
ENTRY (U"用法")
NORMAL (U"Praat 中实现的日志记录功能主要是为了方便以前 Kay CSL 的用户，"
	"他们多年来一直习惯于这样做，并希望在 Praat 中继续使用此功能。"
	"否则，您可能更倾向于使用 @TextGridEditor 来标记时间点，然后运行"
	"自动分析。")
NORMAL (U"如果您确实想使用日志记录功能，如果您想重新使用该文件名，通常首先要删除旧的"
	"日志文件（通过选择 ##Delete log file 1# 或 ##Delete log file 2#）。"
	"否则，您可以更改日志文件名称（使用 ##Log settings...#）。"
	"在此之后，您可以将光标移动到各个时间位置，并每次按下 @@Keyboard shortcuts|F12@（或 @@Keyboard shortcuts|Shift-F12@），"
	"这样关于当前时间的信息就会被写入日志文件中。")
ENTRY (U"示例 1：音高记录")
NORMAL (U"假设您想记录光标的时间以及光标处的音高值。"
	"您可以使用以下日志格式：")
CODE (U"Time 'time:6' seconds, pitch 'f0:2' hertz")
NORMAL (U"如果现在您单击 3.456789876 秒，且该时间点的音高恰好是 355.266 赫兹，"
	"则以下行将会追加到日志文件和/或信息窗口中：")
CODE (U"Time 3.456790 seconds, pitch 355.27 hertz.")
NORMAL (U"“:6”和“:2”部分表示小数点后的位数。"
	"如果省去它们，写入的值将具有 17 位的精度。")
NORMAL (U"“time”和“f0”两个词的含义与命令"
	"##Get cursor# 和 ##Get pitch# 的结果完全相同。因此，如果在设置光标线时，您选择了一个较大的"
	"声音片段，则“time”将给出选区的中心，而“f0”将给出选区内的平均音高。")
NORMAL (U"注意以下陷阱：如果您的音高单位不是赫兹而是半音，"
	"那么“f0”将以半音给出结果。此示例中的格式将会产生误导。")
ENTRY (U"示例 2：共振峰记录")
NORMAL (U"假设您想记录选区的开始和结束、其时长，以及前三个共振峰的平均值，"
	"所有这些都用制表符隔开，以便导入到 Microsoft® Excel™ 中。"
	"您可以使用以下日志格式：")
CODE (U"'t1:4''tab$''t2:4''tab$''f1:0''tab$''f2:0''tab$''f3:0'")
NORMAL (U"您可以看到“t1”和“t2”分别是选区的开始和结束，"
	"并且它们在小数点后保留了 4 位数字。通过使用“:0”，三个共振峰值"
	"被四舍五入为赫兹的整数。单词 `tab$` 是制表符。")
ENTRY (U"可记录的值")
NORMAL (U"以下值可以被记录：")
LIST_ITEM (U"`time`：光标的时间，或选区的中心。")
LIST_ITEM (U"`t1`：选区的起点（“B”）。")
LIST_ITEM (U"`t2`：选区的终点（“E”）。")
LIST_ITEM (U"`dur`：选区的时长。")
LIST_ITEM (U"`freq`：频率光标处的频率。")
LIST_ITEM (U"`f0`：光标时间的音高，或选区内的平均音高。")
LIST_ITEM (U"`f1`、`f2`、`f3`、`f4`、`f5`：光标时间的第一/第二/第三/第四/第五共振峰，"
	"或选区内第一/第二/第三/第四/第五共振峰的平均值。")
LIST_ITEM (U"`b1`、`b2`、`b3`、`b4`、`b5`：第一/第二/第三/第四/第五共振峰的带宽"
	"在光标时间或选区中心的值。")
LIST_ITEM (U"`intensity`：光标时间的音强，或选区内的平均音强，单位为分贝（dB）。")
LIST_ITEM (U"`power`：光标交叉处的频谱功率，单位为 Pa^2/Hz。")
LIST_ITEM (U"`tab$`：制表符。")
LIST_ITEM (U"`editor$`：编辑器窗口的标题（即可见的 Sound 或 TextGrid 的名称）。")
ENTRY (U"更具弹性的记录方式")
NORMAL (U"您有时可能需要在日志文件中记录一些无法直接"
	"由上述可记录值生成的信息。例如，假设您想要记录单击点处的 F1 和 F2-F1 的值。"
	"您可以编写以下脚本：")
CODE (U"f1 = Get first formant")
CODE (U"f2 = Get second formant")
CODE (U"f21 = f2 - f1")
CODE (U"appendInfoLine: fixed$ (f1, 0), \" \", fixed$ (f21, 0)")
CODE (U"appendFileLine: \"D:\\Praat logs\\Formant log.txt\", fixed$ (f1, 0), tab$, fixed$ (f21, 0)")
NORMAL (U"通过此脚本，信息将同时追加到信息窗口和您桌面上的"
	"“Formant log.txt”文件中。")
NORMAL (U"您可以通过保存脚本并在 ##Log settings...# 窗口的 ##Log script 3#（或 #4）字段中指定该脚本文件的名称，"
	"从而使用 @@Keyboard shortcuts|Option-F12@（或 @@Keyboard shortcuts|Command-F12@）来访问此脚本。")
NORMAL (U"这些脚本可以接收参数。例如，假设您希望在按下 @@Keyboard shortcuts|Option-F12@ 时指定一个元音符号。"
	"以下脚本将处理这一点：")
CODE (U"form: \"Save vowel and formants\"")
	CODE1 (U"word: \"Vowel\", \"a\"")
CODE (U"endform")
CODE (U"f1 = Get first formant")
CODE (U"f2 = Get second formant")
CODE (U"f21 = f2 - f1")
CODE (U"appendInfoLine: vowel$, \" \", fixed$ (f1, 0), \" \", fixed$ (f21, 0)")
CODE (U"appendFileLine: \"~/Praat logs/Vowels and formants log\", vowel$, tab$, fixed$ (f1, 0), tab$, fixed$ (f21, 0)")
NORMAL (U"注意以下陷阱：由于脚本的特性，当您有两个同名的编辑器窗口时，不要尝试这样做。"
	"我们无法预测这两个窗口中哪一个会响应 #Get 查询……")
MAN_END

MAN_BEGIN (U"Manipulation", U"ppgb", 20030316)   // 2023
INTRO (U"Praat 中的@@types of objects|对象类型@之一，用于更改声音的音高和时长曲线。")
ENTRY (U"Manipulation（操纵）对象内部")
NORMAL (U"使用 @Inspect（检查）时，您将看到以下属性：")
TERM (U"##timeStep")
DEFINITION (U"音高分析中使用的时间步长（或%%frame length%，帧长）。常用值为 0.010 秒。")
TERM (U"##pitchFloor")
DEFINITION (U"音高分析中考虑的最小音高频率。常用值为 75 赫兹。")
TERM (U"##pitchCeiling")
DEFINITION (U"音高分析中考虑的最大音高频率。常用值为 600 赫兹。")
NORMAL (U"一个 Manipulation 对象还包含以下较小的对象：")
LIST_ITEM (U"1. 原始 @Sound（声音）对象。")
LIST_ITEM (U"2. 表示声门脉冲的 @PointProcess（点过程）对象。")
LIST_ITEM (U"3. 一个 @PitchTier（音高层）对象。")
LIST_ITEM (U"4. 一个 @DurationTier（时长层）对象。")
ENTRY (U"分析")
NORMAL (U"当从声音创建 Manipulation 对象时，会执行以下步骤：")
LIST_ITEM (U"1. 使用 @@Sound: To Pitch...@ 的方法对原始声音进行音高分析。"
	"这会使用时间步长、音高下限和音高上限参数。")
LIST_ITEM (U"2. 得到的音高曲线信息（频率和有声/无声决策）"
	"被用于在原始声音包含较多能量的位置放置声门脉冲。"
	"其方法与 @@Sound & Pitch: To PointProcess (cc)@ 相同。")
LIST_ITEM (U"3. 使用 @@Pitch: To PitchTier@ 的方法将音高曲线转换为包含许多点（目标点）的音高层。")
LIST_ITEM (U"4. 创建一个空的 @DurationTier（时长层）对象。")
ENTRY (U"重合成（Resynthesis）")
TERM (U"Manipulation 对象可以生成 Sound 输入。该 Sound 可以通过以下几种方式进行计算：")
LIST_ITEM (U"• @@overlap-add|重叠相加法@：基于 原始声音 + 脉冲 + 音高层 + 时长层；")
LIST_ITEM (U"• #LPC：基于 LPC（来自原始声音）+ 脉冲 + 音高层；")
LIST_ITEM (U"• 仅基于脉冲，以脉冲列或嗡嗡声（蜂鸣）形式；")
LIST_ITEM (U"• 仅基于音高层，以脉冲列或嗡嗡声（蜂鸣）形式。")
MAN_END

MAN_BEGIN (U"Manipulation: Extract duration tier", U"ppgb", 20010330)
INTRO (U"从每个选中的 @Manipulation 对象中提取时长信息，复制到一个新的 @DurationTier 对象的命令。")
MAN_END

MAN_BEGIN (U"Manipulation: Extract original sound", U"ppgb", 20010330)
INTRO (U"将每个选中的 @Manipulation 对象中的原始声音复制到一个新的 @Sound 对象中的命令。")
MAN_END

MAN_BEGIN (U"Manipulation: Extract pitch tier", U"ppgb", 20010330)
INTRO (U"从每个选中的 @Manipulation 对象中提取音高信息，复制到一个新的 @PitchTier 对象的命令。")
MAN_END

MAN_BEGIN (U"Manipulation: Extract pulses", U"ppgb", 20010330)
INTRO (U"从每个选中的 @Manipulation 对象中提取声带脉冲信息，复制到一个新的 @PointProcess 对象的命令。")
MAN_END

MAN_BEGIN (U"Manipulation: Play (overlap-add)", U"ppgb", 20070722)
INTRO (U"播放每个选中的 @Manipulation 对象，通过 @@overlap-add|重叠相加法@ 重合成声音的命令。")
MAN_END

MAN_BEGIN (U"Manipulation: Get resynthesis (overlap-add)", U"ppgb", 20070722)
INTRO (U"从每个选中的 @Manipulation 对象中提取声音，通过 @@overlap-add|重叠相加法@ 重合成的命令。")
MAN_END

MAN_BEGIN (U"Manipulation: Replace duration tier", U"ppgb", 20030216)
INTRO (U"您可以使用一个单独的 @DurationTier 对象来替换您在 @Manipulation 对象中看到的时长层，"
	"例如，您从另一个 Manipulation 提取的时长层，"
	"或者您使用 @@Create DurationTier...@ 创建的时长层。")
NORMAL (U"要做到这一点，请同时选择您的 Manipulation 对象和 @DurationTier 对象，然后点击 ##Replace duration tier#。")
MAN_END

MAN_BEGIN (U"Manipulation: Replace pitch tier", U"ppgb", 20030216)
INTRO (U"您可以使用一个单独的 @PitchTier 对象来替换您在 @Manipulation 对象中看到的音高层，"
	"例如，您从另一个 Manipulation 提取的音高层，"
	"或者您使用 @@Create PitchTier...@ 创建的音高层。")
NORMAL (U"要做到这一点，请同时选择您的 Manipulation 对象和 @PitchTier 对象，然后点击 ##Replace pitch tier#。")
MAN_END

MAN_BEGIN (U"Manipulation: Replace pulses", U"ppgb", 20010330)
INTRO (U"用选中的 @PointProcess 对象替换选中 @Manipulation 对象中的声带脉冲信息的命令。")
MAN_END

MAN_BEGIN (U"Manipulation: Replace original sound", U"ppgb", 20010330)
INTRO (U"用选中的 @Sound 对象替换选中 @Manipulation 对象中的原始声音的命令。")
MAN_END

MAN_BEGIN (U"ManipulationEditor", U"ppgb", 20030316)  // 2023-06-08
	INTRO (U"Praat 中的@@Editors|编辑器@之一，用于查看和操纵 @@Manipulation|Manipulation@（声音操纵）对象。")
ENTRY (U"显示的内容")
	NORMAL (U"该编辑器显示：")
	LIST_ITEM (U"• 原始 @@Sound|Sound@（声音）对象。")
	LIST_ITEM (U"• 表示声门%%pulses%（脉冲）的 @@PointProcess|PointProcess@（点标记过程）对象。您可以对其进行编辑以改善音高分析。")
	LIST_ITEM (U"• 基于脉冲位置的音高曲线，用于对比（绘制为灰色圆点）。如果编辑脉冲，其形状会发生改变。")
	LIST_ITEM (U"• 决定重合成 @@Sound|Sound@ 的音高曲线的 @@PitchTier|PitchTier@（音高层）对象（绘制为蓝色圆圈）。在创建 @@Manipulation|Manipulation@ 对象时，它会从原始音高曲线计算得出。您可以通过简化它（例如删除目标点），或者上下、前后移动它的各个部分来对其进行操纵。")
	LIST_ITEM (U"• 用于操纵声音有声部分相对时长的 @@DurationTier|DurationTier@（时长层）对象。")
ENTRY (U"播放")
	NORMAL (U"若要播放（一部分）%%resynthesized%（重合成）声音（通过 #Synth 菜单中显示的任何方法，例如 @@overlap-add|重叠相加法@ 和 #LPC），请@@Click|单击@绘制区域下方和上方的 1 到 8 个播放按钮，或者使用 Play（播放）菜单中的命令。")
	NORMAL (U"若要播放%%original%（原始）声音，请使用 ##Shift-click#（Shift-单击）。")
ENTRY (U"脉冲")
	TERM (U"添加：")
	DEFINITION (U"在所需的时间位置@@Click|单击@，然后选择 ##Add pulse at cursor#（在光标处添加脉冲）或键入 @@Keyboard shortcuts|Command-P@。")
	TERM (U"移除：")
	DEFINITION (U"进行 @@time selection|时间选择@，联同选择 ##Remove pulse(s)#（移除脉冲）或键入 @@Keyboard shortcuts|Option-Command-P@。如果没有进行选择，则移除最靠近光标的脉冲。")
ENTRY (U"音高点")
	TERM (U"在指定的%%time and frequency%（时间和频率）处添加点：")
	DEFINITION (U"在所需的时间-频率位置@@Click|单击@，然后选择 ##Add pitch point at cursor#（在光标处添加音高点）或键入 @@Keyboard shortcuts|Command-t@。")
	TERM (U"仅在指定%time（时间）处添加点：")
	DEFINITION (U"在所需的时间处@@Click|单击@，然后选择 ##Add pitch point at time slice#（在时间片处添加音高点）。ManipulationEditor 会尝试根据脉冲之间的间隔来计算频率，基本上采用的是三值中值法。")
	TERM (U"移除：")
	DEFINITION (U"进行 @@time selection|时间选择@，然后选择 ##Remove pitch point(s)#（移除音高点）或键入 @@Keyboard shortcuts|Option-Command-t@。如果没有进行选择，则移除最靠近光标的音高点。")
	TERM (U"移动多个点：")
	DEFINITION (U"进行 @@time selection|时间选择@（被选中的点会变为红色），然后按住 ##Shift-drag#（Shift-拖动）在窗口中移动这些点。您不能将它们拖过相邻的点，或者拖到 50 Hz 以下或最大频率以上。如果%%dragging strategy%（拖动策略）是 ##All#（所有）或 ##Only horizontal#（仅水平），则您只能水平拖动它们；如果拖动策略不是 ##Only horizontal#，则您可以垂直拖动它们。您可以通过 #Pitch 菜单中的 ##Set pitch dragging strategy...#（设置音高拖动策略...）来更改拖动策略。")
	TERM (U"移动单个点：")
	DEFINITION (U"使用鼠标@@Drag|拖动@该点。如果拖动策略不是 ##Only vertical#（仅垂直），则您只能水平拖动它；如果拖动策略不是 ##Only horizontal#（仅水平），则您可以垂直拖动它。")
ENTRY (U"时长点")
	NORMAL (U"其工作方式与音高点基本相同。")
ENTRY (U"简化（Stylization）")
	NORMAL (U"在编辑音高点之前，您可能希望通过选择 #Pitch 菜单中的任何 #Stylize（简化）命令来减少点数。")
MAN_END

MAN_BEGIN (U"Matrix", U"ppgb", 20240712)
INTRO (U"Praat 中的@@types of objects|对象类型@之一。 "
	"一个 Matrix（矩阵）对象表示定义域 [%x__%min_, %x__%max_] × [%y__%min_, %y__%max_] 上的一个函数 %z (%x, %y)。 "
	"该定义域已在 %x 和 %y 方向上进行采样，"
	"在每个方向上具有恒定的采样间隔（%dx and %dy）。 "
	"因此，样本为 %z [%i_%y] [%i_%x], %i_%x = 1 ... %n_%x, %i_%y = 1 ... %n_%y。 "
	"这些样本表示函数值 %z (%x__1_ + (%i_%x - 1) %dx, %y__1_ + (%i_%y - 1) %dy)。")
ENTRY (U"Matrix 命令")
NORMAL (U"创建：")
LIST_ITEM (U"• @@Create Matrix...")
LIST_ITEM (U"• @@Create simple Matrix...")
LIST_ITEM (U"• @@Read from file...")
LIST_ITEM (U"• @@Read Matrix from raw text file...")
LIST_ITEM (U"• ##Read Matrix from LVS AP file...")
NORMAL (U"绘制：")
LIST_ITEM (U"• ##Matrix: Draw rows...")
LIST_ITEM (U"• ##Matrix: Draw contours...")
LIST_ITEM (U"• ##Matrix: Paint contours...")
LIST_ITEM (U"• ##Matrix: Paint cells...")
LIST_ITEM (U"• ##Matrix: Scatter plot...")
LIST_ITEM (U"• @@Matrix: Draw as squares...")
LIST_ITEM (U"• ##Matrix: Draw value distribution...")
LIST_ITEM (U"• ##Matrix: Paint surface...")
NORMAL (U"修改：")
LIST_ITEM (U"• @@Matrix: Formula...")
LIST_ITEM (U"• ##Matrix: Scale...")
ENTRY (U"Matrix 对象内部")
NORMAL (U"使用 @Inspect（检查）时，您将看到以下属性。")
TERM (U"%xmin, %xmax ≥ %xmin")
DEFINITION (U"%x 定义域。")
TERM (U"%nx ≥ 1")
DEFINITION (U"列数。")
TERM (U"%dx > 0.0")
DEFINITION (U"列之间的距离。")
TERM (U"%x1")
DEFINITION (U"与第一列关联的 %x 值。")
TERM (U"%ymin, %ymax ≥ %ymin")
DEFINITION (U"%y 定义域。")
TERM (U"%ny ≥ 1")
DEFINITION (U"行数。")
TERM (U"%dy > 0.0")
DEFINITION (U"行之间的距离。")
TERM (U"%y1")
DEFINITION (U"与第一行关联的 %y 值。")
TERM (U"%z [1..%ny] [1..%nx]")
DEFINITION (U"样本值。")
NORMAL (U"在创建 #Matrix（矩阵）之后，%xmin, %xmax, %ymin, %ymax, "
	"%nx, %ny, %dx, %dy, %x1 和 %y1 "
	"通常不会改变，而 %z 的内容会改变。")
NORMAL (U"通常情况下，您会希望 %xmin ≤ %x1 且 %xmax ≥ %x1 + (%nx - 1) %dx。")
ENTRY (U"示例：简单矩阵")
NORMAL (U"如果简单矩阵的 %x 等于列号，"
	"且 %y 等于行号，它具有以下属性：")
LIST_ITEM (U"%xmin = 1;   %xmax = %nx;   %dx = 1;  %x1 = 1;")
LIST_ITEM (U"%ymin = 1;   %ymax = %ny;   %dy = 1;  %y1 = 1;")
ENTRY (U"示例：采样信号")
NORMAL (U"如果矩阵表示持续时间为 1 秒且采样频率为 10 kHz 的采样信号，"
	"它具有以下属性：")
LIST_ITEM (U"%xmin = 0.0;   %xmax = 1.0;   %nx = 10000 ;   %dx = 1.0·10^^-4^;   %x1 = 0.5·10^^-4^;")
LIST_ITEM (U"%ymin = 1;   %ymax = 1;   %ny = 1;   %dy = 1;   %y1 = 1;")
ENTRY (U"示例：复杂信号")
NORMAL (U"如果矩阵代表一个复杂频谱，"
	"该频谱是用 @FFT（快速傅里叶变换）从示例 2 中的声音导出的，它具有以下属性：")
LIST_ITEM (U"%xmin = 0.0;   %xmax = 5000.0;   %nx = 8193 ;   %dx = 5000.0 / 8192;   %x1 = 0.0;")
LIST_ITEM (U"%ny = 2（实部和虚部）；")
LIST_ITEM (U"%ymin = 1（第一行，实部）；")
LIST_ITEM (U"%ymax = 2（第二行，虚部）；")
LIST_ITEM (U"%dy = 1;   %y1 = 1;（使得 %y 等于行号）")
MAN_END

MAN_BEGIN (U"Matrix: Draw as squares...", U"ppgb", 19980319)
INTRO (U"将 @Matrix 对象绘制到 @@Picture window|Picture 窗口@ 中的命令。")
ENTRY (U"设置")
TERM (U"##Xmin")
TERM (U"##Xmax")
DEFINITION (U"%x 方向的窗口域。在此范围之外的元素将不会被绘制。 "
	"%Autowindowing（自动窗口化）：如果（%Xmin ≥ %Xmax），将使用该矩阵的整个 %x 域 [%x__%min_, %x__%max_]。")
TERM (U"##Ymin")
TERM (U"##Ymax")
DEFINITION (U"%y 方向的窗口域。在此范围之外的元素将不会被绘制。 "
	"%Autowindowing（自动窗口化）：如果（%Ymin ≥ %Ymax），将使用该矩阵的整个 %y 域 [%y__%min_, %y__%max_]。")
TERM (U"##Garnish")
DEFINITION (U"决定是否在图画周围绘制轴。 "
	"如果您更喜欢自己使用 @Margins（边距）菜单来点缀图画，请关闭此按钮。")
ENTRY (U"行为")
NORMAL (U"对于在指定窗口域内的 Matrix 的每个元素， "
	"会绘制一个不透明的白色或黑色矩形（如果元素的值为正，则为白色，"
	"如果为负，则为黑色），周围环绕着一个黑色细线框。 "
	"矩形的%面积与元素的值成正比。")
ENTRY (U"技巧")
NORMAL (U"如果您希望矩形的%边长（而不是面积）与元素的值成正比， "
	"您可以在绘制前使用公式“`self^2`”（参见 @@Matrix: Formula...@）。")
MAN_END

MAN_BEGIN (U"Matrix: Formula...", U"ppgb", 20021206)
INTRO (U"更改所有选定 @Matrix 对象中数据的命令。")
NORMAL (U"有关示例和解释，请参阅 @Formulas 教程。")
MAN_END

MAN_BEGIN (U"Matrix: Paint cells...", U"ppgb", 20021204)
INTRO (U"将 @Matrix 的内容绘制到 @@Picture window|Picture 窗口@ 中的命令。")
NORMAL (U"矩阵的每个单元格都绘制为一个矩形，矩形填充有介于白色（如果单元格的内容较小）和黑色（如果内容较大）之间的灰色值。")
MAN_END

MAN_BEGIN (U"Matrix: Set value...", U"ppgb", 19980319)
INTRO (U"更改每个选定 @Matrix 对象中一个单元格的值的命令。")
ENTRY (U"设置")
TERM (U"##Row number")
DEFINITION (U"您要更改其值的单元格所在的行号。")
TERM (U"##Column number")
DEFINITION (U"您要更改其值的单元格所在的列号。")
TERM (U"##New value")
DEFINITION (U"您希望指定的单元格具有的新值。")
MAN_END

/*
1. The Hilbert transform.

I wondered whether my use of taking the square was worse than taking
the Hilbert envelope. My formula is

   my_envelope1(t) = x(t) ^ 2

For a sampled signal, this introduces higher frequencies, so I should have
filtered the signal at half the Nyquist frequency to prevent aliasing. I wonder
how the Hilbert envelope behaves in this respect:

   your_envelope1(t) = x(t) ^ 2 + Px(t) ^ 2

I think it has the same problem, since it involves squaring the real and imaginary parts.

Of course, the Hilbert transform is much smoother, but that cannot matter for
frequencies above 100 Hz or so, since we are going to filter at 30 Hz anyway.
Thus, I think these methods are comparable. Do you agree?


2. The non-linearity.

In the second step, I take the logarithm:

   my_envelope2(t) = 10 log10 (my_envelope1(t) + 1e-6)

Of course, the 1e-6 is there to guard against zero values in my_envelope1.
I took the logarithm because it can handle negative values, which will arise
in the filtering process. You take the square root instead:

   your_envelope2(t) = sqrt (your_envelope1(t))


3. Filtering between 3 and 30 Hz.

You use a second-order Butterworth filter in the time domain:

   your_envelope3(t) = your_envelope2(t) * Butterworth(t)

What do you mean by "forward and backward filtering"?
Did you use the second-order filter twice, thus making it fourth-order effectively?

My alternative was to use the two Gaussians:

   my_spectrum2(f) = fft (my_envelope2(t))
   my_spectrum3(f) = my_spectrum2(f) (exp(-(f/35)^2)-exp(-(f/3.5)^2)
   my_envelope3(t) = ifft (my_spectrum3 (f))

This is a filter with several desirable properties: no ringing, phase-preserving,
an area of zero. On the other hand, it is not causal, but I think that that is not
an issue here. Is it?


4. The new band.

In my algorithm, I have to undo the non-linearity:

   my_envelope4(t) = 10 ^ (my_envelope3(t) / 2)

You see that I divide by 2, not 10, thus effectively multiplying the dynamics,
expressed in dB, by a factor of 5. Since I am going to use the new envelope as
a multiplication factor, I will have to limit it to a factor of 10, because the
maximum enhancement is 20 dB. But this limiting factor is band-dependent:

   ceiling(Fmid) = 1 + 9 (1/2 - 1/2 cos (pi Fmid / 13))

where Fmid is expressed in Bark. The limiting is done smoothly:

   my_envelope5(t) = 1 / (1 / my_envelope4(t) + 1 / ceiling)

So this is the final multiplication factor for the band-filtered signal:

   new_band(t) = x(t) my_envelope5(t)

Your multiplication factor is very different. Please tell me whether I am correct.

   your_envelope4(t) = rectify (your_envelope3(t))

This step is needed because you cannot handle negative values. It introduces
higher-frequency components, which will have to be filtered out later.
The new signal will have to carry the new envelope, so the multiplication
factor is taken relative to the envelope of the original signal:

   your_envelope5(t) = your_envelope4(t) / your_envelope2(t)

The components that were introduced by the rectification must be thrown out:

   your_envelope6(t) = your_envelope5(t) * Butterworth?(t)

What were the characteristics of that filter? You did not specify them in your paper.
Well, the next step is normalization to the power of the original band:

   new_band(t) = your_envelope6(t) ||x(t)|| / ||your_envelope5(t)||

At least, that's your formula (12). I suppose that it contains a mistake, and that
the norm in the denominator should refer to the filtered your_envelope6(t) instead?

By the normalization, the fast movements within each band have been strengtened,
and the slow parts have been weakened. Thus, there is dynamic expansion within
each band, whereas my algorithm has this only in the mid frequency range, am I correct?
Could you also explain figure 5 to me? Some of the thick curves show enhanced peaks,
but how about the valleys, are some of them deepened as well?


5. The new signal.

My algorithm simply adds all the bands:

   new_signal(t) = SUM new_band(Fmid,t)

The mid-frequency bands are favoured because they have stronger modulation deepening
than the other bands. In your algorithm, the favouring of the mid frequencies is
implemented in the last step:

   new_signal(t) = SUM new_band(Fmid,t) gain(Fmid)

This would mean, for instance, that if the modulation is not deepened (for instance
if there are no fast movements), the mid-frequency range is still multiplied by a factor
of 10, whereas my algorithm would not change the signal at all in such a case. Is that so?

If so, could any of the learning results have been produced by the general emphasis
in the 1-4 kHz range? Have you any proof that the modulation deepening itself
is the cause of the learning results?

So the differences between the two algorithms are clear now: mine does modulation
deepening selectively in the F2 range, yours does modulation deepening everywhere
plus an independent emphasis in the F2 range. Since both algorithms will lead to
emphasis in the F2 range, that leaves the question of whether a modulation change
is necessary for the other frequency bands. Have you any idea, or was your choice
for this just one of the minor decisions while constructing your algorithm?

To sum up, I think that both algorithms perform what they were intended to do,
namely selectively enhancing fast F2 transitions. If you agree with this, I may
advise XXX to use my algorithm without change. I would, however, welcome any
suggestions as to oversights in my implementation, or other possible problems
or incorrect choices.
*/

MAN_BEGIN (U"Matrix: To TableOfReal", U"ppgb", 19991030)
INTRO (U"将每个选中的 @Matrix 转换为 @TableOfReal 的命令。")
NORMAL (U"该命令可从 #Cast 菜单中获得。生成的 TableOfReal "
	"具有与原始 Matrix 相同的行数和列数， "
	"并且单元格中具有相同的数据。但是，它尚没有任何行或列 "
	"标签；您可以使用 TableOfReal #Modify 菜单中的一些命令来添加这些标签。")
MAN_END

MAN_BEGIN (U"Modify", U"ppgb", 20021204)
INTRO (U"许多对象类型的 @@dynamic menu|动态菜单@ 子菜单的标题。 "
	"此子菜单通常收集所有可以更改所选对象的命令。")
MAN_END

MAN_BEGIN (U"PairDistribution", U"ppgb", 20030316)
INTRO (U"One of the @@types of objects@ in Praat. A "
	"PairDistribution object represents the relative probabilities with which "
	"the specified pairs of strings occur.")
ENTRY (U"Class description")
TERM (U"##struct-list# pairs")
DEFINITION (U"a list of relative string-pair probabilities. Each element consists of:")
TERM1 (U"#string %string1")
DEFINITION1 (U"the first string.")
TERM1 (U"#string %string2")
DEFINITION1 (U"the second string.")
TERM1 (U"#real %weight")
DEFINITION1 (U"the relative probability associated with the string pair. This value cannot be negative.")
MAN_END

MAN_BEGIN (U"PairDistribution: To Stringses...", U"ppgb", 20030916)
INTRO (U"A command to generate a number of string pairs from the selected @PairDistribution object. "
	"This command will create two aligned @Strings objects of equal size.")
ENTRY (U"Settings")
TERM (U"##Number# (standard: 1000)")
DEFINITION (U"the number of the strings in either resulting Strings object.")
TERM (U"##Name of first Strings# (standard: \"input\")")
DEFINITION (U"the name of the resulting Strings object associated with the first string of each pair.")
TERM (U"##Name of second Strings# (standard: \"output\")")
DEFINITION (U"the name of the resulting Strings object associated with the second string of each pair.")
ENTRY (U"Example")
NORMAL (U"Suppose the PairDistribution contains the following:")
CODE (U"4 pairs")
CODE (U"\"at+ma\"  \"atma\"  100")
CODE (U"\"at+ma\"  \"apma\"    0")
CODE (U"\"an+pa\"  \"anpa\"   20")
CODE (U"\"an+pa\"  \"ampa\"   80")
NORMAL (U"The resulting Strings object \"input\" may then contain:")
EQUATION (U"at+ma, an+pa, an+pa, at+ma, at+ma, an+pa, an+pa, an+pa, an+pa, at+ma, ...")
NORMAL (U"The Strings object \"output\" may then contain:")
EQUATION (U"atma,  ampa,  ampa,  atma,  atma,  ampa,  anpa,  ampa,  ampa,  atma, ...")
MAN_END

MAN_BEGIN (U"ParamCurve", U"ppgb", 20030316)
INTRO (U"One of the @@types of objects@ in Praat.")
NORMAL (U"An object of class #ParamCurve represents a sequence of time-stamped points (%x (%%t__i_), %y (%%t__i_)) "
	"in a two-dimensional space.")
MAN_END

#if 0

MAN_BEGIN (U"Polygon", U"ppgb", 20030316)
INTRO (U"One of the @@types of objects@ in Praat.")
NORMAL (U"A Polygon object represents a sequence of points (%%x__i_, %%y__i_) in a two-dimensional space.")
MAN_END

MAN_BEGIN (U"Create Polygon from values...", U"David Weenink & Paul Boersma", 20230116)
INTRO (U"Creates a @@Polygon@ from user-supplied x\\--y pairs (“points” or “vertices”).")
ENTRY (U"Settings")
TERM (U"##Name")
DEFINITION (U"defines the name of the resulting Polygon.")
TERM (U"##X values")
DEFINITION (U"the X-coordinates of the vertices of the Polygon.")
TERM (U"##Y values")
DEFINITION (U"the Y-coordinates of the vertices of the Polygon.")
ENTRY (U"Example")
NORMAL (U"The command ##Create Polygon from values: \"triangle\", \"0 50 20\", \"0 10 80\"# "
	"defines a Polygon with the three points (0, 0), (50, 10), and (20, 80). "
	"In the figure the three points are indicated with open circles while the Polygon is drawn as a closed figure.")
{ kManPage_type::SCRIPT, UR"~~~(
	Create Polygon from values: "triangle", "0 50 20", "0 10 80"
	Draw circles: 0, 0, 0, 0, 3
	Draw closed: 0, 0, 0, 0
	Remove
)~~~", 4, 4, nullptr },
MAN_END

#else


MAN_PAGES_BEGIN R"~~~(
"Polygon"
© ppgb 2003-03-16

One of the @@types of objects@ in Praat.

A Polygon object represents a sequence of points (%%x__i_, %%y__i_) in a two-dimensional space.
)~~~" MAN_PAGES_END


MAN_PAGES_BEGIN R"~~~(
"Create Polygon from values..."
© David Weenink & Paul Boersma 2023-01-16

Creates a @@Polygon@ from user-supplied x\--y pairs (“points” or “vertices”).

Settings
===

##Name
: defines the name of the resulting Polygon.

##X values
: the X-coordinates of the vertices of the Polygon.

##Y values
: the Y-coordinates of the vertices of the Polygon.

Example
===
The command ##Create Polygon from values: "triangle", "0 50 20", "0 10 80"#
defines a Polygon with the three points (0, 0), (50, 10), and (20, 80).
In the figure the three points are indicated with open circles while the Polygon is drawn as a closed figure.
{+ 4x4
	Create Polygon from values: "triangle", "0 50 20", "0 10 80"
	Draw circles: 0, 0, 0, 0, 3
	Draw closed: 0, 0, 0, 0
}
)~~~" MAN_PAGES_END

#endif


MAN_BEGIN (U"Read Matrix from raw text file...", U"ppgb", 19980322)
INTRO (U"A command to read a @Matrix object from a file on disk.")
ENTRY (U"File format")
NORMAL (U"The file should contain each row of the matrix on a separate line. Within each row, "
	"the elements must be separated by spaces or tabs.")
NORMAL (U"For instance, the following text file will be read as a Matrix with three rows and four columns:")
CODE (U"0.19 3 245 123")
CODE (U"18e-6 -3e18 0 0.0")
CODE (U"1.5 2.5 3.5 4.5")
NORMAL (U"The resulting Matrix will have the same domain and sampling as Matrices created with "
	"##Create simple Matrix...#. In the above example, this means that the Matrix will have "
	"%x__%min_ = 0.5, %x__%max_ = 4.5, %n__%x_ = 4, %dx = 1.0, %x__1_ = 1.0, "
	"%y__%min_ = 0.5, %y__%max_ = 3.5, %n__%y_ = 3, %dy = 1.0, %y__1_ = 1.0.")
MAN_END

MAN_BEGIN (U"Read Strings from raw text file...", U"ppgb", 19990502)
INTRO (U"A command to read a @Strings object from a simple text file. "
	"Each line is read as a separate string. See @Strings for an example.")
MAN_END

MAN_BEGIN (U"RealTier", U"ppgb", 20210612)
INTRO (U"One of the @@types of objects@ in Praat. "
	"An RealTier object represents a time-stamped curve, i.e., it contains a series of (%time, %value) points. "
	"The values have no physical units, so a RealTier is a generic, \"mathematical\", type, "
	"just as a @Matrix is. Special, physically interpretable, kinds of RealTiers "
	"are @PitchTier, @IntensityTier, @DurationTier and @AmplitudeTier. "
	"You can convert a RealTier to and from these other tier types.")
NORMAL (U"A RealTier can come in handy if you have time-stamped real-valued data "
	"that cannot be interpreted as any of the physical tier types. "
	"You can \"View & Edit\" a RealTier in its own window.")
ENTRY (U"How to import a RealTier from raw data")
NORMAL (U"While you create a RealTier from the New menu, you will often have data in a text file produced by "
	"a different app than Praat. For instance, your text file may look like this, with a header:")
CODE (U"Time Value")
CODE (U"0.134 67.9")
CODE (U"0.178 -138.6")
CODE (U"0.211 78.9")
CODE (U"0.213 100.0")
CODE (U"0.456 -97.0")
NORMAL (U"You can import this text file as a Table object with ##Read Table from whitespace-separated file...# "
	"or perhaps with ##Read Table from tab-separated file...#). You can then choose ##To RealTier...# from the Convert menu. "
	"Alternatively, your text file may simply look like this, without header:")
CODE (U"0.134 67.9")
CODE (U"0.178 -138.6")
CODE (U"0.211 78.9")
CODE (U"0.213 100.0")
CODE (U"0.456 -97.0")
NORMAL (U"You can import this text file as a Matrix object with @@Read Matrix from raw text file...@. "
	"You can then choose ##To RealTier...# from the Cast menu.")
MAN_END

MAN_BEGIN (U"Sound: To Intensity...", U"ppgb", 20100605)   // 2023
INTRO (U"A command to create an @Intensity object from every selected @Sound.")
ENTRY (U"Settings")
TERM (U"##Pitch floor (Hz)")
DEFINITION (U"the minimum periodicity frequency in your signal. If you set it too high, "
	"you will end up with a pitch-synchronous intensity modulation. If you set it too low, "
	"your intensity contour may appear smeared, so you should set it as high as allowed by the signal "
	"if you want a sharp contour.")
TERM (U"##Time step (s)")
DEFINITION (U"the time step of the resulting intensity contour. If you set it to zero, the time step is computed as "
	"one quarter of the effective window length, i.e. as 0.8 / %pitchFloor.")
TERM (U"##Subtract mean")
DEFINITION (U"See @@Intro 6.2. Configuring the intensity contour@.")
ENTRY (U"Algorithm")
NORMAL (U"The values in the sound are first squared, then convolved with a Gaussian analysis window (Kaiser-20; sidelobes below -190 dB). "
	"The effective duration of this analysis window is 3.2 / %pitchFloor, which will guarantee that a periodic signal is analysed as having a "
	"pitch-synchronous intensity ripple not greater than 0.00001 dB.")
MAN_END

MAN_BEGIN (U"Sound & IntensityTier: Multiply", U"ppgb", 20000724)
INTRO (U"A command to create a new Sound from the selected @Sound and @Intensity objects.")
NORMAL (U"The resulting Sound equals the original sound, multiplied by a linear interpolation of the intensity. "
	"Afterwards, the resulting Sound is scaled so that its maximum absolute amplitude is 0.9.")
MAN_END

MAN_BEGIN (U"Strings", U"ppgb", 20201226)
INTRO (U"One of the @@types of objects@ in Praat. Represents an ordered list of strings.")
ENTRY (U"Creation")
NORMAL (U"The difficult way is to create a #Strings object from a generic Praat text file "
	"(if there are non-ASCII symbols, use UTF-8 or UTF-16 format):")
CODE (U"\"ooTextFile\"")
CODE (U"\"Strings\"")
CODE (U"5 ! number of strings")
CODE (U"\"Hello\"")
CODE (U"\"Goodbye\"")
CODE (U"\"Auf wiedersehen\"")
CODE (U"\"Tschüss\"")
CODE (U"\"Arrivederci\"")
NORMAL (U"In this example, we see that a double quote within a string should be written twice; "
	"the fourth string will therefore be read as ##Tschüss#. "
	"This file can be read simply with the generic @@Read from file...@ command from the #Open menu.")
NORMAL (U"An easier way is to use the special command @@Read Strings from raw text file...@. "
	"The file can then simply look like this:")
CODE (U"Hello")
CODE (U"Goodbye")
CODE (U"Auf wiedersehen")
CODE (U"Tschüss")
CODE (U"Arrivederci")
NORMAL (U"You can also create a #Strings object from a folder listing or from some other objects:")
LIST_ITEM (U"• @@Create Strings as file list...")
LIST_ITEM (U"• @@Distributions: To Strings...@")
LIST_ITEM (U"• @@OTGrammar: Generate inputs...@")
LIST_ITEM (U"• @@OTGrammar & Strings: Inputs to outputs...@")
MAN_END

MAN_BEGIN (U"Strings: To Distributions", U"ppgb", 19971025)
INTRO (U"A command to analyse each selected @Strings object into a @Distributions object.")
NORMAL (U"The resulting #Distributions will collect the occurrences of every string in the #Strings object, "
	"and put the number of occurrences in its first and only column.")
ENTRY (U"Example")
NORMAL (U"We start from the following #Strings:")
CODE (U"6 (number of strings)")
CODE (U"\"hallo\"")
CODE (U"\"dag allemaal\"")
CODE (U"\"hallo\"")
CODE (U"\"tot morgen\"")
CODE (U"\"hallo\"")
CODE (U"\"tot morgen\"")
NORMAL (U"This will give us the following #Distributions:")
CODE (U"1 (number of columns) \"\" (no column name)")
CODE (U"\"hallo\"         3")
CODE (U"\"dag allemaal\"  1")
CODE (U"\"tot morgen\"    2")
MAN_END

}

/* End of file manual_Fon.cpp */
