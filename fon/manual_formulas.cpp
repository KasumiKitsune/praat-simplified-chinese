/* manual_formulas.cpp
 *
 * Copyright (C) 1992-2025 Paul Boersma
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

void manual_formulas_init (ManPages me);
void manual_formulas_init (ManPages me) {

MAN_BEGIN (U"Calculator", U"ppgb", 20210228)
INTRO (U"一个允许您计算各种简单或复杂的数学与字符串表达式的窗口。 "
	"要显示计算器，请键入 @@Keyboard shortcuts|Command-U@ 快捷键或选择 @@Calculator...|Calculator...（计算器...）@ 命令。 "
	"计算结果将显示在 @@Info window|信息窗口@ 中。")
NORMAL (U"有关您可以使用此命令计算的所有内容，请参阅 @Formulas 教程。")
MAN_END

MAN_BEGIN (U"Calculator...", U"ppgb", 20050822)
INTRO (U"@@Objects window|对象窗口@ 的 @@Praat menu|Praat 菜单@ 中 @Goodies 子菜单下的一个命令。快捷键：@@Keyboard shortcuts|Command-U@。 "
	"选择此命令将启动 Praat 的 @calculator（计算器）。")
MAN_END

MAN_BEGIN (U"undefined", U"ppgb", 20170910)
INTRO (U"当您发出查询数值的命令时，Praat 有时会在 @@Info window|信息窗口@ 中写入数值 ##--undefined--# "
	"（单词两边有两个连字符，即“未定义”）。如果您所请求的数值未定义，就会发生这种情况， "
	"例如以下示例：")
LIST_ITEM (U"\\bu 您选择了一个结束时间为 1.0 秒的 Sound（声音），并请求 1.5 秒到 2.0 秒之间的波形最小值点 "
	"（使用查询命令 ##Get minimum...#）。")
LIST_ITEM (U"\\bu 您请求声音的无声（清音）部分的基频值（选择一个 #Pitch， "
	"然后选择 ##Get value at time...#）。")
LIST_ITEM (U"\\bu 您在 @calculator（计算器）中键入以下公式：10\\^ 400。")
ENTRY (U"Usage in a script")
NORMAL (U"在 Praat 脚本中，此值直接表示为 \"undefined\"。您可以用它来测试 "
	"查询命令是否返回了有效的数值：")
CODE (U"selectObject: \"Pitch hallo\"")
CODE (U"meanPitch = Get mean: 0.1, 0.2, \"Hertz\", \"Parabolic\"")
CODE (U"if meanPitch = undefined")
	CODE1 (U"# Take some exceptional action.")
CODE (U"else")
	CODE1 (U"# Take the normal action.")
CODE (U"endif")
ENTRY (U"Details for hackers")
NORMAL (U"在文本文件中，此值写入为 ##--undefined--#。 "
	"在二进制文件中，它被写入为大端序 IEEE 正无穷大（positive infinity）。 "
	"在内存中，它通常是一个特定的“非数”（Not-a-Number, NaN），即 0 除以 0 的结果， "
	"尽管其他 NaN 以及无穷大也会同样被报告为 --undefined--。")
MAN_END

MAN_BEGIN (U"Formulas", U"ppgb", 20170916)
INTRO (U"您可以在 Praat 的许多地方使用数值表达式和字符串（文本）表达式：")
LIST_ITEM (U"\\bu 在 Praat 的 @Goodies 子菜单下的 @calculator（计算器）中；")
LIST_ITEM (U"\\bu 在大多数设置窗口的数值字段中；")
LIST_ITEM (U"\\bu 在 @@Praat script|Praat 脚本@ 中。")
NORMAL (U"对于某些类型的对象（主要是 Sound 和 Matrix），您也可以同时对其所有内容应用公式：")
LIST_ITEM (U"\\bu 当您从 @@New menu|New 菜单@ 创建 Sound 或 Matrix 时；")
LIST_ITEM (U"\\bu 当您为选定的对象从 @Modify 菜单中选择 @@Formula...|Formula...@ 时。")
NORMAL (U"您可以在 “##< 1#” 和 “##1 >#” 按钮的帮助下按顺序阅读本教程。")
LIST_ITEM (U"1. @@Formulas 1. My first formulas|我的第一个公式@")
LIST_ITEM1 (U"1.1. @@Formulas 1.1. Formulas in the calculator|计算器中的公式@")
LIST_ITEM1 (U"1.2. @@Formulas 1.2. Numeric expressions|数值表达式@")
LIST_ITEM1 (U"1.3. @@Formulas 1.3. String expressions|字符串表达式@")
LIST_ITEM1 (U"1.4. @@Formulas 1.4. Array expressions|数组表达式@")
LIST_ITEM1 (U"1.5. @@Formulas 1.5. Formulas in settings windows|设置窗口中的公式@")
LIST_ITEM1 (U"1.6. @@Formulas 1.6. Formulas for creation|用于创建的公式@")
LIST_ITEM1 (U"1.7. @@Formulas 1.7. Formulas for modification|用于修改的公式@")
LIST_ITEM1 (U"1.8. @@Formulas 1.8. Formulas in scripts|脚本中的公式@")
LIST_ITEM (U"2. @@Formulas 2. Representations|表示方式@")
LIST_ITEM1 (U"2.1. @@Formulas 2.1. Representation of numbers|数字的表示方式@")
LIST_ITEM1 (U"2.2. @@Formulas 2.2. Representation of strings|字符串的表示方式@")
LIST_ITEM1 (U"2.3. @@Formulas 2.3. Representation of arrays|数组的表示方式@")
LIST_ITEM (U"3. @@Formulas 3. Operators|操作符@ (`+`, `-`, `*`, `/`, `^`)")
LIST_ITEM (U"4. @@Formulas 4. Constants|常量@ (`pi`, `e`, `undefined`)")
LIST_ITEM (U"5. @@Formulas 5. Mathematical functions|数学函数@")
LIST_ITEM (U"6. @@Formulas 6. String functions|字符串函数@")
LIST_ITEM (U"7. @@Formulas 7. Control structures|控制结构@ (`if then else fi`, 分号)")
LIST_ITEM (U"8. @@Formulas 8. Attributes of objects|对象的属性@")
LIST_ITEM (U"9. @@Formulas 9. Data in objects|对象中的数据@")
MAN_END

MAN_BEGIN (U"Formulas 1. My first formulas", U"ppgb", 20170916)
LIST_ITEM (U"1.1. @@Formulas 1.1. Formulas in the calculator|计算器中的公式@")
LIST_ITEM (U"1.2. @@Formulas 1.2. Numeric expressions|数值表达式@")
LIST_ITEM (U"1.3. @@Formulas 1.3. String expressions|字符串表达式@")
LIST_ITEM (U"1.4. @@Formulas 1.4. Array expressions|数组表达式@")
LIST_ITEM (U"1.5. @@Formulas 1.5. Formulas in settings windows|设置窗口中的公式@")
LIST_ITEM (U"1.6. @@Formulas 1.6. Formulas for creation|用于创建的公式@")
LIST_ITEM (U"1.7. @@Formulas 1.7. Formulas for modification|用于修改的公式@")
LIST_ITEM (U"1.8. @@Formulas 1.8. Formulas in scripts|脚本中的公式@")
MAN_END

MAN_BEGIN (U"Formulas 1.1. Formulas in the calculator", U"ppgb", 20170916)
INTRO (U"要使用 Praat @calculator（计算器），请转到 @@Praat menu|Praat 菜单@， "
	"并从 @Goodies 子菜单中选择 @@Calculator...|Calculator...@。或者只需在 Praat 中的任何地方键入 @@Keyboard shortcuts|Command-U@。")
ENTRY (U"Calculating numbers")
NORMAL (U"您可以进行算术计算。键入公式")
CODE (U"8*17")
NORMAL (U"并点击 OK。信息窗口将弹出并显示结果：")
CODE (U"136")
ENTRY (U"Calculating strings")
NORMAL (U"您还可以进行文本计算。键入公式")
CODE (U"\"see\" + \"king\"")
NORMAL (U"并点击 OK。信息窗口将显示结果：")
CODE (U"seeking")
ENTRY (U"Calculating arrays")
NORMAL (U"您可以对向量（数值数组）进行计算。键入公式")
CODE (U"{ 11, 13, 17 } + 0.5")
NORMAL (U"并点击 OK。信息窗口将显示结果：")
CODE (U"11.5")
CODE (U"13.5")
CODE (U"17.5")
MAN_END

MAN_BEGIN (U"Formulas 1.2. Numeric expressions", U"ppgb", 20170916)
INTRO (U"所有计算结果为数字的公式都称为数值表达式。 "
	"对于以下示例，所有的结果都可以通过 @calculator（计算器）进行验证。")
ENTRY (U"Examples with numbers")
NORMAL (U"有些数值表达式仅涉及数字：")
TERM (U"##8*17")
DEFINITION (U"计算乘法。结果：136。")
TERM (U"##2\\^ 10")
DEFINITION (U"计算 2 的 10 次方。结果：1024。")
TERM (U"##sqrt (2) / 2")
DEFINITION (U"计算 2 的平方根，并将结果除以 2。结果：0.7071067811865476。")
TERM (U"##sin (1/4 * pi)")
DEFINITION (U"计算 %\\pi/4 的正弦值。结果：（依然是）0.7071067811865476。")
ENTRY (U"Examples with strings")
NORMAL (U"有些数值表达式计算字符串的数值属性：")
TERM (U"##length (\"internationalization\")")
DEFINITION (U"计算字符串 \"internationalization\" 的长度。结果：20。")
TERM (U"##index (\"internationalization\", \"ation\")")
DEFINITION (U"计算字符串 \"ation\" 在字符串 \"internationalization\" 中首次出现的位置。结果：7， "
	"因为 \"ation\" 的第一个字母与 \"internationalization\" 的第七个字母对齐。 "
	"如果该子字符串未出现，结果为 0。")
ENTRY (U"Examples with arrays")
NORMAL (U"有些数值表达式计算数值向量的数值属性：")
TERM (U"##size ({ 40, 70, 60, 50 })")
DEFINITION (U"计算向量 { 40, 70, 60, 50 } 的大小，即其元素的数量。结果：4。")
TERM (U"##mean ({ 40, 70, 60, 50 })")
DEFINITION (U"计算 40, 70, 60 和 50 这四个数字的平均值。结果：55。")
MAN_END

MAN_BEGIN (U"Formulas 1.3. String expressions", U"ppgb", 20170916)
INTRO (U"所有计算结果为文本的公式都称为字符串表达式。 "
	"“字符串（string）”是文本的编程术语；把文本想象成一“串”字符（字母）。")
NORMAL (U"同样，以下示例的结果可以通过 @calculator（计算器）进行验证。")
TERM (U"##\"see\" + \"king\"")
DEFINITION (U"拼接两个字符串。结果：seeking。")
TERM (U"##left\\$  (\"internationalization\", 6)")
DEFINITION (U"计算字符串最左边的六个字母；美元符号用于所有结果为字符串的函数。结果：intern。")
TERM (U"##mid\\$  (\"internationalization\", 6, 8)")
DEFINITION (U"计算从 \"internationalization\" 的第六个字母开始的 8 字符子字符串。结果：national。")
TERM (U"##date\\$  ( )")
DEFINITION (U"计算当前的日期和时间。在我写下此内容时的结果：Mon Dec  2 02:23:45 2002。")
MAN_END

MAN_BEGIN (U"Formulas 1.4. Array expressions", U"ppgb", 20170916)
INTRO (U"数值向量表达式是其值为数值向量的表达式。")
NORMAL (U"您可以通过 @calculator（计算器）来验证以下示例的结果。")
TERM (U"##{ 11, 13, 17 } + 0.5")
DEFINITION (U"给包含三个元素的向量的每个元素加上 0.5，从而得到一个包含三个元素的新向量。 "
	"Praat 将结果作为三行写入信息窗口，包含数字 11.5, 13.5 和 17.5：")
CODE (U"11.5")
CODE (U"13.5")
CODE (U"17.5")
TERM (U"@`zero#` (5)")
DEFINITION (U"创建一个带有 5 个零的向量。Praat 将它们写入五行中：")
CODE (U"0")
CODE (U"0")
CODE (U"0")
CODE (U"0")
CODE (U"0")
TERM (U"@`repeat#` ({ 1, 5 }, 6)")
DEFINITION (U"创建一个带有 12 个元素的向量，其中序列 { 1, 5 } 重复了 6 次。结果：")
CODE (U"1")
CODE (U"5")
CODE (U"1")
CODE (U"5")
CODE (U"1")
CODE (U"5")
CODE (U"1")
CODE (U"5")
CODE (U"1")
CODE (U"5")
CODE (U"1")
CODE (U"5")
MAN_END

MAN_BEGIN (U"Formulas 1.5. Formulas in settings windows", U"ppgb", 20170916)
INTRO (U"在设置窗口的数值字段中，您通常只需直接键入一个数字。 "
	"然而，您也可以改用任何数值表达式。")
NORMAL (U"例如，假设您想创建一个包含恰好 10000 个采样点的 Sound（声音）。 "
	"如果采样频率是 44100 Hz，则时长将为 10000/44100秒。 "
	"您可以通过从 @@New menu|New 菜单@ 中选择 @@Create Sound from formula...|Create Sound from formula...@ 来创建这样一个 Sound， "
	"然后输入")
CODE (U"10000/44100")
NORMAL (U"到 ##End time#（结束时间）字段中。")
NORMAL (U"在设置窗口的文本字段中，您只能直接键入文本；无法 "
	"使用字符串表达式（除非您使用脚本；参见 @@Formulas 1.8. Formulas in scripts|脚本中的公式@）。")
NORMAL (U"在设置窗口的数值向量字段中，您可以键入任何数值向量（数组）表达式。")
MAN_END

MAN_BEGIN (U"Formulas 1.6. Formulas for creation", U"ppgb", 20110128)
INTRO (U"通过 @@New menu|New 菜单@ 中的某些命令，您可以提供一个公式，Praat 将把该公式应用于新对象的所有元素。")
ENTRY (U"Creating a Sound from a formula")
NORMAL (U"选择 @@Create Sound from formula...|Create Sound from formula...@ 并在 #%Formula 字段中键入以下内容：")
CODE (U"1/2 * sin (2 * pi * 377 * x)")
NORMAL (U"当您点击 #OK 时，列表中将出现一个新的 @Sound（声音）对象。 "
	"在您点击 ##View & Edit# 并放大几次后，您会看到该声音是一个正弦波， "
	"其频率为 377 赫兹（每秒周期数）。这能起作用是因为公式中的 $x 代表时间， "
	"即该公式被分别应用于每个采样点，每个采样点都具有不同的 $x 值。")
ENTRY (U"Creating a Matrix from a formula")
NORMAL (U"选择 @@Create simple Matrix...|Create simple Matrix...@ 并在 #%Formula 字段中键入以下内容：")
CODE (U"8")
NORMAL (U"当您点击 OK 时，列表中将出现一个新的 @Matrix（矩阵）对象。当您点击 #Info 时， "
	"您将看到它是一个 10 行 10 列的矩阵，且所有 100 个单元格都包含值 8 "
	"（您能看到这点是因为最小值和最大值都被报告为 8）。")
NORMAL (U"一个更有趣的例子是公式")
CODE (U"row * col")
NORMAL (U"对于生成的 Matrix， "
	"选择 @@Matrix: Paint cells...|Paint cells...@ 并点击 #OK。画图窗口将显示一个 10\\xx10 "
	"的矩阵，其元素是行号和列号的乘积，即它们的值介于 "
	"1 到 100 之间。除了 $row 和 $col 之外，您还可以使用 $x 表示沿水平轴的距离，使用 $y 表示 "
	"沿垂直轴的距离；参见下一页的示例。")
MAN_END

MAN_BEGIN (U"Formulas 1.7. Formulas for modification", U"ppgb", 20170916)
INTRO (U"与可用于创建新对象的公式类似（参见前一页）， "
	"您可以使用公式来修改现有对象。您可以通过选择一个对象， "
	"然后在 @Modify 菜单中找到 ##Formula...# 命令来执行此操作。")
ENTRY (U"Modifying a Sound with a formula")
NORMAL (U"用麦克风录制一段声音，并用非常低的声音说话。如果您不知道如何在 Praat 中录制声音， "
	"请咨询 @Intro 教程。一旦 Sound 对象出现在列表中，点击 #Play。声音听起来会非常小。 "
	"然后从 Modify 菜单中选择 ##Formula...# 并键入")
CODE (U"self * 3")
NORMAL (U"点击 OK，然后再次点击 #Play。现在的声音大得多了。您已将声音中 "
	"每个采样点的振幅乘了 3。")
ENTRY (U"Replacing the contents of a Sound with a formula")
NORMAL (U"如果您在公式中不使用 `self`，则该公式不会引用 Sound 的现有内容。因此，公式 ")
CODE (U"1/2 * sin (2 * pi * 377 * x)")
NORMAL (U"将直接用 377-Hz 的正弦波替换您录制的语音。")
ENTRY (U"Modifying a Matrix with a formula")
NORMAL (U"许多对象都可以被看作是矩阵： "
	"它们由许多行和列组成，每个单元格中都有数据：")
LIST_ITEM (U"@Sound：单行；列代表采样点。")
LIST_ITEM (U"@Spectrum：两行（第一行是实部，第二行是虚部）； "
	"列代表频率。")
LIST_ITEM (U"@Spectrogram、@Cochleagram：行代表频率；列代表时间。")
LIST_ITEM (U"@Excitation：单行；列代表频带。")
LIST_ITEM (U"@Harmonicity：单行；列代表时间帧。")
NORMAL (U"该公式应用于每一行的每一列。公式")
CODE (U"self^2")
NORMAL (U"将把所有矩阵元素平方。")
NORMAL (U"该公式首先作用于第一行，并在该行中从第一列开始； "
	"这可以递归作用。公式")
CODE (U"self + self [row, col - 1]")
NORMAL (U"对每一行进行积分。")
ENTRY (U"Referring to the current position in the object")
NORMAL (U"您可以通过索引或 %x 和 %y 值来引用 Matrix（或 Sound 等）中的当前位置：")
TERM (U"#`row`")
DEFINITION (U"当前行")
TERM (U"#`col`")
DEFINITION (U"当前列")
TERM (U"#`x`")
DEFINITION (U"与当前列相关联的 %x 值：")
	LIST_ITEM2 (U"对于 Sound、Spectrogram、Cochleagram 或 Harmonicity：时间（time），如上述 377-Hz 正弦波示例所示")
	LIST_ITEM2 (U"对于 Spectrum：频率（Hz）")
	LIST_ITEM2 (U"对于 Excitation：频率（Bark）")
TERM (U"#`y`")
DEFINITION (U"与当前行相关联 the %y 值：")
	LIST_ITEM2 (U"对于 Spectrogram：频率（Hz）")
	LIST_ITEM2 (U"对于 Cochleagram：频率（Bark）")
ENTRY (U"Referring to the contents of the object itself")
NORMAL (U"您可以通过索引引用当前 Matrix（或 Sound 等）中的值。")
TERM (U"#`self`")
DEFINITION (U"引用当前 Matrix 中当前行和列的值，或当前 Sound 中当前采样点的值。")
TERM (U"#`self` [%`column-expression`]")
DEFINITION (U"引用当前 Sound（或 Intensity 等）中当前采样点（或帧）的值。 "
	"%`column-expression` 将被四舍五入为最接近的整数。 "
	"如果索引超出范围（小于 1 或大于 %n__%x_），则表达式的值计算为 0。")
NORMAL (U"#Example。积分器（integrator）为")
CODE (U"   self [col - 1] + self * dx")
TERM (U"#`self` [%`row-expression`, %`column-expression`]")
DEFINITION (U"引用当前 Matrix（或 Spectrogram 等）中指定行和列的值。 "
	"这些表达式将被四舍五入为最接近的整数。")
NORMAL (U"您可以通过 %x 和 %y 位置引用当前 Matrix（或 Spectrogram 等）中的值：")
TERM (U"#`self` (%`x-expression`, %`y-expression`)")
DEFINITION (U"在四个最接近的矩阵点之间对表达式进行线性插值。")
NORMAL (U"您可以通过 %x 位置引用当前 Sound（或 Intensity 等）中的值：")
TERM (U"#`self` (%`x-expression`)")
DEFINITION (U"在两个最接近的采样点（或帧）之间对表达式进行线性插值。")
MAN_END

MAN_BEGIN (U"Formulas 1.8. Formulas in scripts", U"ppgb", 20170916)
INTRO (U"在脚本中，您可以将数值表达式分配给数值变量， "
	"将字符串表达式分配给字符串变量，以及将数组表达式分配给数组变量。 "
	"您也可以在表达式中使用数值、字符串和数组变量。")
ENTRY (U"Example: report a square")
NORMAL (U"从 @@Praat menu|Praat 菜单@ 中选择 @@New Praat script|新建 Praat 脚本@。脚本编辑器窗口将变得可见。 "
	"在该窗口中键入以下几行：")
CODE (U"x = 99")
CODE (U"x2 = x * x")
CODE (U"writeInfoLine: “The square of ”, x, “ is ”, x2, “.”")
NORMAL (U"这是一个简单的 @@Praat script|Praat 脚本@ 示例； "
	"它将数值公式 `99` 和 `x * x` 的结果 "
	"分配给数值变量 %`x` 和 %`x2`。注意，公式 `x * x` 本身引用了变量 %`x`。 "
	"要运行（执行）此脚本，请键入 @@Keyboard shortcuts|Command-R@ 或从 Run 菜单中选择 #Run。 "
	"Praat 将随后在信息窗口中写入以下文本：")
CODE (U"The square of 99 is 9801.")
NORMAL (U"有关脚本的更多信息，请参阅 @Scripting 教程。")
ENTRY (U"Example: rename the city of Washington")
NORMAL (U"在脚本编辑器窗口中键入以下文本：")
CODE (U"current$ = “Bush”")
CODE (U"previous$ = “Clinton”")
CODE (U"famous$ = “Lincoln”")
CODE (U"newCapital$ = current$ + mid$ (famous$, 2, 3) + right$ (previous$, 3)")
CODE (U"writeInfoLine: “The new capital will be ”, newCapital$, “.”")
NORMAL (U"该脚本将四个字符串表达式的结果分配给四个字符串变量 `current$`、 "
	"`previous$`、`famous$` 和 `newCapital$`。美元符号是字符串变量 "
	"或结果为字符串的函数（如 `left$`）的表示法。注意，第四行中的公式引用了三个现有的变量。")
NORMAL (U"要查看首都的新名称是什么，请选择 #Run。")
ENTRY (U"Example: report five squares")
NORMAL (U"键入以下脚本：")
CODE (U"x# = { 1, 2, 3, 4, 5 }")
CODE (U"x2# = x# * x#")
CODE (U"writeInfoLine: “The squares of ”, x#, “ are ”, x2#, “.”")
NORMAL (U"Praat 将随后在信息窗口中写入以下文本：")
CODE (U"The squares of 1 2 3 4 5 are 1 4 9 16 25.")
ENTRY (U"Example: numeric expressions in settings in scripts")
NORMAL (U"正如在真实的设置窗口中一样，您可以在所有数值字段中使用数值表达式。 "
	"两页前的示例如下：")
CODE (U"Create Sound from formula: “sine”, 1, 0, 10000 / 44100, 44100, ~ 0.9 * sin (2*pi*377*x)")
ENTRY (U"Example: string expressions in settings in scripts")
NORMAL (U"正如在真实的设置窗口中一样，您可以在所有文本字段中使用字符串表达式：")
CODE (U"soundName$ = “hello”")
CODE (U"Read from file: soundName$ + “.wav”")
ENTRY (U"Example: numeric expressions in creation in scripts")
NORMAL (U"假设您想生成一个其频率保存在变量中的正弦波。以下是实现方法：")
CODE (U"frequency = 377")
CODE (U"Create Sound from formula: “sine”, 1, 0.0, 1.0, 44100, ~ 0.9 * sin (2*pi*frequency*x)")
NORMAL (U"在此示例中，如果 `x` 也是一个变量（%`x`），Praat 将会报错，因为这会 "
	"与 Sound 中代表时间的 `x` 产生歧义（参见 @@Formulas 1.7. Formulas for modification|用于修改的公式@）。")
MAN_END

MAN_BEGIN (U"Formulas 2. Representations", U"ppgb", 20170916)
LIST_ITEM (U"2.1. @@Formulas 2.1. Representation of numbers|数字的表示方式@")
LIST_ITEM (U"2.2. @@Formulas 2.2. Representation of strings|字符串的表示方式@")
LIST_ITEM (U"2.3. @@Formulas 2.3. Representation of arrays|数组的表示方式@")
MAN_END

MAN_BEGIN (U"Formulas 2.1. Representation of numbers", U"ppgb", 20170916)
INTRO (U"公式可以处理整数，也可以处理实数。")
ENTRY (U"Real numbers")
NORMAL (U"您可以通过使用十进制表示法来键入许多实数，例如 3.14159, 299792.5 或 -0.000123456789。 "
	"对于非常大或非常小的数字，您可以使用 %e-表示法：6.022\\.c10^^23^ 可以键入为 `6.022e23` 或 `6.022e+23`， "
	"而 -1.6021917\\.c10^^-19^ 键入为 `-1.6021917e-19`。您还可以使用百分比表示法：0.157 "
	"可以键入为 `15.7%`。")
NORMAL (U"在 Praat 中，实数可以具有的值存在一些局限性。 "
	"这些数字必须介于 -10^^308^ 和 +10^^308^ 之间。如果您键入")
CODE (U"1e200 * 1e100")
NORMAL (U"结果将是")
CODE (U"1e+300")
NORMAL (U"但如果您键入")
CODE (U"1e300 * 1e100")
NORMAL (U"结果将是")
CODE (U"--undefined--")
NORMAL (U"另一个局限性是，最接近 0 且非零的数字在 -10^^-308^ 和 +10^^-308^ 附近。如果您键入")
CODE (U"1e-200 / 1e100")
NORMAL (U"结果将是")
CODE (U"1e-300")
NORMAL (U"but if you type")
CODE (U"1e-300 / 1e100")
NORMAL (U"结果将是")
CODE (U"0")
NORMAL (U"Finally, the precision of real numbers is limited by the number of bits that every real number is stored with "
	"in the computer, namely 64. For instance, if you type")
CODE (U"pi")
NORMAL (U"the outcome will be")
CODE (U"3.141592653589793")
NORMAL (U"because only 16 digits of precision are stored. This can lead to unexpected results caused by rounding. "
	"For instance, the formula")
CODE (U"0.34999999999999999 - 0.35")
NORMAL (U"will result in")
CODE (U"0")
NORMAL (U"rather than the correct value of 1e-17. This is because the numbers 0.34999999999999999 and 0.35 cannot "
	"be distinguished in the computer's memory. If you simply type")
CODE (U"0.34999999999999999")
NORMAL (U"the outcome will be")
CODE (U"0.35")
NORMAL (U"(as in this example, the calculator will always come up with the minimum number of digits needed to represent the number unambiguously).")
NORMAL (U"Another example of inaccuracy is the formula")
CODE (U"1 / 7 / 59 * 413")
NORMAL (U"Because of rounding errors, the result will be")
CODE (U"0.9999999999999999")
ENTRY (U"Integer numbers")
NORMAL (U"Formulas can work with integer (whole) numbers between -1,000,000,000,000,000 and +1,000,000,000,000,000. "
	"You type them without commas and without the plus sign: 337, -848947328345289.")
NORMAL (U"You %can work with larger numbers than that (up to 10^^308^), but there will again be rounding errors. "
	"For instance, the formula")
CODE (U"1000000000000000 + 1")
NORMAL (U"correctly yields")
CODE (U"1000000000000001")
NORMAL (U"but the formula")
CODE (U"10000000000000000 + 1")
NORMAL (U"yields an incorrect outcome:")
CODE (U"1e16")
MAN_END

MAN_BEGIN (U"Formulas 2.2. Representation of strings", U"ppgb", 20021203)
INTRO (U"公式可以处理置于两个双引号之间的字符串， "
	"如 “goodbye” 或 “how are you doing?”。")
NORMAL (U"如果字符串本身需要包含双引号， "
	"您必须将其键入两次。例如，如果您键入")
CODE (U"\"I asked: \"\"how are you doing?\"\"\"")
NORMAL (U"到计算器中，结果将是")
CODE (U"I asked: \"how are you doing?\"")
MAN_END

MAN_BEGIN (U"Formulas 2.3. Representation of arrays", U"ppgb", 20170916)
INTRO (U"公式可以处理置于大括号之间的向量， "
	"如 { 11, 17, 13.5 }。如果您在计算器中键入此公式，结果将是")
CODE (U"11")
CODE (U"17")
CODE (U"13.5")
MAN_END

MAN_BEGIN (U"Formulas 3. Operators", U"ppgb", 20230124)
NORMAL (U"在公式中，您可以使用本页描述的数值和逻辑运算符。 "
	"运算符的评估顺序是编程语言中最常用的顺序。 "
	"要强制执行不同的顺序，请使用括号。")
NORMAL (U"优先级最高的运算符是#负号 (-) 和#求幂 (\\^ )：")
CODE (U"--6 \\-> 6")
CODE (U"2^6 \\-> 64")
/*@praat
	# Tests for the manual page "Operators".

	assert --6 = 6
	assert 2^6 = 64
@*/
NORMAL (U"负号和求幂的序列是自右向左进行评估的：")
CODE (U"2^-6 \\-> 0.015625")                       //@praat assert 2^-6 = 0.015625
CODE (U"-(1+1)^6 \\-> -64")                        //@praat assert -(1+1)^6 = -64
CODE (U"4^3^2 \\-> 4^9 \\-> 262144")               //@praat assert 4^3^2 = 262144
NORMAL (U"请注意，改变空格并不会改变其含义：")
CODE (U"4^3 ^ 2 \\-> 262144")                      //@praat assert 4^3 ^ 2 = 262144
NORMAL (U"要更改评估顺序，您必须使用括号：")
CODE (U"(4 ^ 3) ^ 2 \\-> 4096")                    //@praat assert (4 ^ 3) ^ 2 = 4096
NORMAL (U"由于负数与正数的负号之间存在歧义，因此不允许使用以下结构：")
CODE (U"-2^6 \\-> ?")
NORMAL (U"相反，您可以使用以下任意一种结构：")
CODE (U"(-2)^6 \\-> 64")                           //@praat assert (-2)^6 = 64
CODE (U"-(2^6) \\-> -64")                          //@praat assert -(2^6) = -64
CODE (U"-(2)^6 \\-> -64")                          //@praat assert -(2)^6 = -64
NORMAL (U"优先级次高的运算符是#乘法 (*) 和#除法 (/)。它们是自左向右评估的：")
CODE (U"1/4*5 \\-> 1.25        (from left to right)")
CODE (U"1 / 4*5 \\-> 1.25      (spacing does not help)")
CODE (U"1 / (4*5) \\-> 0.05    (use parentheses to change the order)")
/*@praat
	assert 1/4*5 = 1.25
	assert 1 / 4*5 = 1.25
	assert 1 / (4*5) = 0.05
@*/
CODE (U"3 * 2 ^ 4 \\-> 48      (exponentiation before multiplication)")
CODE (U"3*2 ^ 4 \\-> 48        (this spacing does not matter and is misleading)")
CODE (U"(3 * 2) ^ 4 \\-> 1296  (use parentheses to change the order)")
/*@praat
	assert 3 * 2 ^ 4 = 48
	assert 3*2 ^ 4 = 48
	assert (3 * 2) ^ 4 = 1296
@*/
NORMAL (U"##整除# 运算符（#div 和 #mod）具有与 * 和 / 相同的优先级， "
	"并且同样自左向右评估：")
CODE (U"54 div 5 \\-> 10       (division rounded down)")                             //@praat assert 54 div 5 = 10
CODE (U"54 mod 5 \\-> 4        (the remainder)")                                     //@praat assert 54 mod 5 = 4
CODE (U"54.3 div 5.1 \\-> 10   (works for real numbers as well)")                    //@praat assert 54.3 div 5.1 = 10
CODE (U"54.3 mod 5.1 \\-> 3.3  (the remainder)")                                     //@praat assert abs ((54.3 mod 5.1) - 3.3) < 1e-14
CODE (U"-54 div 5 \\-> -11     (division rounded down; negation before division)")   //@praat assert -54 div 5 = -11
CODE (U"-54 mod 5 \\-> 1       (the remainder)")                                     //@praat assert -54 mod 5 = 1
CODE (U"-(54 div 5) \\-> -10   (use parentheses to change the order)")               //@praat assert -(54 div 5) = -10
CODE (U"-(54 mod 5) \\-> -4")                                                        //@praat assert -(54 mod 5) = -4
CODE (U"3 * 18 div 5 \\-> 10   (from left to right)")                                //@praat assert 3 * 18 div 5 = 10
CODE (U"3 * (18 div 5) \\-> 9")                                                      //@praat assert 3 * (18 div 5) = 9
CODE (U"3 * 18 mod 5 \\-> 4")                                                        //@praat assert 3 * 18 mod 5 = 4
CODE (U"3 * (18 mod 5) \\-> 9")                                                      //@praat assert 3 * (18 mod 5) = 9
CODE (U"54 div 5 * 3 \\-> 30   (from left to right)")                                //@praat assert 54 div 5 * 3 = 30
CODE (U"54 div (5 * 3) \\-> 3")                                                      //@praat assert 54 div (5 * 3) = 3
CODE (U"54 mod 5 * 3 \\-> 12")                                                       //@praat assert 54 mod 5 * 3 = 12
CODE (U"54 mod (5 * 3) \\-> 9")                                                      //@praat assert 54 mod (5 * 3) = 9
NORMAL (U"优先级次高的运算符是#加法 (+) 和#减法 (-)， "
	"自left向右评估：")
CODE (U"3 - 8 + 7 \\-> 2       (from left to right)")                                //@praat assert 3 - 8 + 7 = 2
CODE (U"3 - (8 + 7) \\-> -12   (use parentheses to change the order)")               //@praat assert 3 - (8 + 7) = -12
CODE (U"3 + 8 * 7 \\-> 59      (multiplication before addition)")                    //@praat assert 3 + 8 * 7 = 59
CODE (U"(3 + 8) * 7 \\-> 77    (use parentheses to change the order)")               //@praat assert (3 + 8) * 7 = 77
CODE (U"3 + - (2 \\^  4) \\-> -13   (exponentiation, negation, addition)")           //@praat assert 3 + - (2 ^ 4) = -13
CODE (U"3 + 5 / 2 + 3 \\-> 8.5")                                                     //@praat assert 3 + 5 / 2 + 3 = 8.5
CODE (U"(3 + 5) / (2 + 3) \\-> 1.6")                                                 //@praat assert (3 + 5) / (2 + 3) = 1.6
NORMAL (U"优先级次高的运算符是#比较运算符 "
	"（`=, <>, <, >, <=, >=`）。这些运算符总是返回 0（%false）或 1（%true）：")
CODE (U"5 + 6 = 10 \\-> 0      (equal)")                                             //@praat assert ( 5 + 6 = 10 ) = 0
CODE (U"5 + 6 = 11 \\-> 1")                                                          //@praat assert ( 5 + 6 = 11 ) = 1
CODE (U"5 + 6 <> 10 \\-> 1     (unequal)")                                           //@praat assert ( 5 + 6 <> 10 ) = 1
CODE (U"5 + 6 <> 11 \\-> 0")                                                         //@praat assert ( 5 + 6 <> 11 ) = 0
CODE (U"5 + 6 < 10 \\-> 0      (less than)")                                         //@praat assert ( 5 + 6 < 10 ) = 0
CODE (U"5 + 6 < 11 \\-> 0")                                                          //@praat assert ( 5 + 6 < 11 ) = 0
CODE (U"5 + 6 > 10 \\-> 1      (greater than)")                                      //@praat assert ( 5 + 6 > 10 ) = 1
CODE (U"5 + 6 > 11 \\-> 0")                                                          //@praat assert ( 5 + 6 > 11 ) = 0
CODE (U"5 + 6 <= 10 \\-> 0     (less than or equal)")                                //@praat assert ( 5 + 6 <= 10 ) = 0
CODE (U"5 + 6 <= 11 \\-> 1")                                                         //@praat assert ( 5 + 6 <= 11 ) = 1
CODE (U"5 + 6 >= 10 \\-> 1     (greater or equal)")                                  //@praat assert ( 5 + 6 >= 10 ) = 1
CODE (U"5 + 6 >= 11 \\-> 1")                                                         //@praat assert ( 5 + 6 >= 11 ) = 1
NORMAL (U"比较运算符主要用于 #if、#while 和 #until 条件中。")
NORMAL (U"最低优先级的运算符是#逻辑运算符（#not、#and 和 #or），其中 #not 的优先级 "
	"最高，#or 的优先级最低：")
CODE (U"not 5 + 6 = 10 \\-> 1")                                                      //@praat assert ( not 5 + 6 = 10 ) = 1
CODE (U"x > 5 and x < 10               (is x between 5 and 10?)")
CODE (U"not x <= 5 and not x >= 10     (means the same as the previous line, except if x is undefined)")
CODE (U"not (x <= 5 or x >= 10)        (again means the same, except if x is undefined)")
/*@praat
	x = 7
	assert ( x > 5 and x < 10 ) = 1
	assert ( not x <= 5 and not x >= 10 ) = 1
	assert ( not (x <= 5 or x >= 10) ) = 1
	x = 3
	assert ( x > 5 and x < 10 ) = 0
	assert ( not x <= 5 and not x >= 10 ) = 0
	assert ( not (x <= 5 or x >= 10) ) = 0
	x = 11
	assert ( x > 5 and x < 10 ) = 0
	assert ( not x <= 5 and not x >= 10 ) = 0
	assert ( not (x <= 5 or x >= 10) ) = 0
	x = undefined
	assert ( x > 5 and x < 10 ) = 0
	assert ( not x <= 5 and not x >= 10 ) = 1
	assert ( not (x <= 5 or x >= 10) ) = 1
@*/
ENTRY (U"String comparison")
TERM (U"#`a$ = b$`")
DEFINITION (U"如果字符串相等，则返回 %true (= 1)，否则返回 %false (= 0)。")
TERM (U"#`a$ <> b$`")
DEFINITION (U"如果字符串不相等，则返回 %true，否则返回 %false。")
TERM (U"#`a$ < b$`")
DEFINITION (U"如果字符串 %`a$` 在 Unicode 排序顺序中排在 %`b$` 之前，则返回 %true。 "
	"因此，\"ha\" < \"hal\" 和 \"ha\" < \"ja\" 为真，但 \"ha\" < \"JA\" 为假， "
	"因为在 Unicode 排序顺序中，所有大写字母都在所有小写字母之前。")
TERM (U"#`a$ > b$`")
DEFINITION (U"如果 %`a$` 在 Unicode 排序顺序中排在 %`b$` 之后，则为 %true。")
TERM (U"#`a$ <= b$`")
DEFINITION (U"如果字符串 %`a$` 在 Unicode 排序顺序中排在 %`b$` 之前， "
	"或者如果两个字符串相等，则返回 %true。")
TERM (U"#`a$ >= b$`")
DEFINITION (U"%true if %`a$` comes after %`b$` or the two are equal.")
ENTRY (U"String concatenation and truncation")
TERM (U"#`a$ + b$`")
DEFINITION (U"拼接两个字符串。执行")
	CODE1 (U"text$ = \"hallo\" + \"dag\"")
DEFINITION (U"之后，变量 %`text$` 包含字符串 “hallodag”。")
TERM (U"#`a$ - b$`")
DEFINITION (U"从第一个字符串的末尾减去第二个字符串。执行")
CODE2 (U"soundFileName$ = \"hallo.aifc\"")
CODE2 (U"textgridFileName$ = soundFileName$ - \".aifc\" + \".TextGrid\"")
DEFINITION (U"之后，变量 %`textgridFileName$` 包含字符串 \"hallo.TextGrid\"。 "
	"如果第一个字符串 %`a$` 不以字符串 %`b$` 结尾，则减法的结果是字符串 %`a$`。")
MAN_END

MAN_BEGIN (U"Formulas 4. Constants", U"ppgb", 20080318)
TERM (U"##pi")
DEFINITION (U"%\\pi, 3.14159265358979323846264338328")
TERM (U"##e")
DEFINITION (U"%e, 2.71828182845904523536028747135")
TERM (U"##undefined")
DEFINITION (U"一个特殊的值，参见 @undefined")
MAN_END

MAN_BEGIN (U"Formulas 5. Mathematical functions", U"ppgb", 20200801)
TERM (U"##abs (%x)")
DEFINITION (U"绝对值")
TERM (U"##round (%x)")
DEFINITION (U"最接近的整数；round (1.5) = 2")
TERM (U"##floor (%x)")
DEFINITION (U"向下取整：不大于 %x 的最大整数值")
TERM (U"##ceiling (%x)")
DEFINITION (U"向上取整：不小于 %x 的最小整数值")
TERM (U"##sqrt (%x)")
DEFINITION (U"平方根：\\Vr%x，%x \\>_ 0")
TERM (U"##min (%x, ...)")
DEFINITION (U"一系列数中的最小值，例如 min (7.2, -5, 3) = -5")
TERM (U"##max (%x, ...)")
DEFINITION (U"一系列数中的最大值，例如 max (7.2, -5, 3) = 7.2")
TERM (U"##imin (%x, ...)")
DEFINITION (U"最小值的位置，例如 imin (7.2, -5, 3) = 2")
TERM (U"##imax (%x, ...)")
DEFINITION (U"最大值的位置，例如 imax (7.2, -5, 3) = 1")
TERM (U"##sin (%x)")
DEFINITION (U"正弦")
TERM (U"##cos (%x)")
DEFINITION (U"余弦")
TERM (U"##tan (%x)")
DEFINITION (U"正切")
TERM (U"##arcsin (%x)")
DEFINITION (U"反正弦，-1 \\<_ %x \\<_ 1")
TERM (U"##arccos (%x)")
DEFINITION (U"反余弦，-1 \\<_ %x \\<_ 1")
TERM (U"##arctan (%x)")
DEFINITION (U"反正切")
TERM (U"##arctan2 (%y, %x)")
DEFINITION (U"幅角")
TERM (U"##sinc (%x)")
DEFINITION (U"辛格函数（归一化正弦）：sin (%x) / %x")
TERM (U"##sincpi (%x)")
DEFINITION (U"辛格函数__%\\pi_：sin (%\\pi%x) / (%\\pi%x)")
TERM (U"##exp (%x)")
DEFINITION (U"指数：%e^%x；与 ##e\\^ %x 相同")
TERM (U"##ln (%x)")
DEFINITION (U"自然对数，以 %e 为底")
TERM (U"##log10 (%x)")
DEFINITION (U"对数，以 10 为底")
TERM (U"##log2 (%x)")
DEFINITION (U"对数，以 2 为底")
TERM (U"##sinh (%x)")
DEFINITION (U"双曲正弦：(%e^%x - %e^^-%x^) / 2")
TERM (U"##cosh (%x)")
DEFINITION (U"双曲余弦：(%e^%x + %e^^-%x^) / 2")
TERM (U"##tanh (%x)")
DEFINITION (U"双曲正切：sinh (%x) / cosh (%x)")
TERM (U"##arcsinh (%x)")
DEFINITION (U"反双曲正弦：ln (%x + \\Vr(1+%x^2))")
TERM (U"##arccosh (%x)")
DEFINITION (U"反双曲余弦：ln (%x + \\Vr(%x^2−1))")
TERM (U"##arctanh (%x)")
DEFINITION (U"反双曲正切")
TERM (U"##sigmoid (%x)")
DEFINITION (U"#R \\-> (0,1)：1 / (1 + %e^^−%x^) 或 1 − 1 / (1 + %e^%x)")
TERM (U"##invSigmoid (%x)")
DEFINITION (U"(0,1) \\-> #R：ln (%x / (1 − %x))")
TERM (U"##erf (%x)")
DEFINITION (U"误差函数：2/\\Vr%\\pi __0_\\in^%x exp(-%t^2) %dt")
TERM (U"##erfc (%x)")
DEFINITION (U"互补误差函数：1 - erf (%x)")
TERM (U"##randomUniform (%min, %max)")
DEFINITION (U"在 %min（闭区间）和 %max（开区间）之间的均匀分布随机实数")
TERM (U"##randomInteger (%min, %max)")
DEFINITION (U"在 %min 和 %max 之间的均匀分布随机整数（闭区间）")
TERM (U"##randomGauss (%\\mu, %\\si)")
DEFINITION (U"以 %\\mu 为均值、%\\si 为标准差的高斯分布随机实数")
TERM (U"##randomPoisson (%mean)")
DEFINITION (U"泊松分布随机实数")
TERM (U"##randomGamma (%%shape%, %%rate%)")
DEFINITION (U"从 Gamma 分布中抽取的随机数，其形状参数为 %\\al "
	"且率参数为 %\\be，其定义为 "
	"%f(%x; %\\al, %\\be) = (1 / \\Ga (%\\al)) %\\be%^^%\\al^ %x^^%\\al−1^ %e^^−%\\be %x^，"
	"对于 %x > 0、%\\al > 0 且 %\\be > 0，遵循 @@Marsaglia & Tsang (2000)@ 的方法")
TERM (U"##random\\_ initializeWithSeedUnsafelyButPredictably (%seed)")
DEFINITION (U"可用于脚本中以创建可重复的随机数序列 "
	"（警告：这种异常状态将在整个 Praat 中持续存在，直到您调用以下函数为止）")
TERM (U"##random\\_ initializeSafelyAndUnpredictably ()")
DEFINITION (U"撤销由前一个函数引起的异常状态")
TERM (U"##lnGamma (%x)")
DEFINITION (U"\\Ga 函数的对数")
TERM (U"##gaussP (%z)")
DEFINITION (U"从 −\\oo 到 %z 的高斯分布下的面积")
TERM (U"##gaussQ (%z)")
DEFINITION (U"在 %z 到 +\\oo 之间的高斯分布下面积："
	"偏离高斯分布均值 %z 个标准差的数值的单尾\"统计显著性 %p\"")
TERM (U"##invGaussQ (%q)")
DEFINITION (U"使得 $gaussQ (%z) = %q 的 %z 值")
TERM (U"##chiSquareP (%`chiSquare`, %`df`)")
DEFINITION (U"在 0 到 %chiSquare 之间的 %\\ci^2 分布下的面积，具有 %`df` 自由度")
TERM (U"##chiSquareQ (%`chiSquare`, %`df`)")
DEFINITION (U"在 %`chiSquare` 到 +\\oo 之间的 %\\ci^2 分布下面积，"
	"具有 %df 自由度：在 %df+1 维中两个分布之间的 %\\ci^2 差异的 \"统计显著性 %p\"")
TERM (U"##invChiSquareQ (%q, %df)")
DEFINITION (U"使得 $chiSquareQ (%\\ci^2, %df) = %q 的 %\\ci^2 值")
TERM (U"##studentP (%t, %df)")
DEFINITION (U"从 -\\oo 到 %t 的学生 t-分布面积")
TERM (U"##studentQ (%t, %df)")
DEFINITION (U"从 %t 到 +\\oo 的学生 t-分布面积")
TERM (U"##invStudentQ (%q, %df)")
DEFINITION (U"使得 $studentQ (%t, %df) = %q 的 %t 值")
TERM (U"##fisherP (%f, %df1, %df2)")
DEFINITION (U"从 0 到 %f 的费希尔 F-分布面积")
TERM (U"##fisherQ (%f, %df1, %df2)")
DEFINITION (U"从 %f 到 +\\oo 的费希尔 F-分布面积")
TERM (U"##invFisherQ (%q, %df1, %df2)")
DEFINITION (U"使得 $fisherQ (%f, %df1, %df2) = %q 的 %f 值")
TERM (U"##binomialP (%p, %k, %n)")
DEFINITION (U"在 %n 次试验中，发生概率为 %p 的事件最多发生 %k 次的概率")
TERM (U"@`binomialQ` (%p, %k, %n)")
DEFINITION (U"在 %n 次试验中，发生概率为 %p 的事件最少发生 %k 次的概率；等于 1 - $binomialP (%p, %k - 1, %n)")
TERM (U"##invBinomialP (%P, %k, %n)")
DEFINITION (U"使得 $binomialP (%p, %k, %n) = %P 的 %p 值")
TERM (U"@`invBinomialQ` (%Q, %k, %n)")
DEFINITION (U"使得 $binomialQ (%p, %k, %n) = %Q 的 %p 值")
TERM (U"##hertzToBark (%x)")
DEFINITION (U"从声学频率到 Bark 频率（知觉频谱频率；基底膜上的位置）："
	"7 ln (%x/650 + \\Vr(1 + (%x/650)^2))")
TERM (U"##barkToHertz (%x)")
DEFINITION (U"650 sinh (%x / 7)")
TERM (U"##hertzToMel (%x)")
DEFINITION (U"从声学频率到知觉音高：550 ln (1 + %x / 550)")
TERM (U"##melToHertz (%x)")
DEFINITION (U"550 (exp (%x / 550) - 1)")
TERM (U"##hertzToSemitones (%x)")
DEFINITION (U"从声学频率到对数音乐音阶（相对于 100 Hz）：12 ln (%x / 100) / ln 2")
TERM (U"##semitonesToHertz (%x)")
DEFINITION (U"100 exp (%x ln 2 / 12)")
TERM (U"##erb (%f)")
DEFINITION (U"对于指定的声学频率（也以赫兹为单位），以赫兹为单位的知觉 %%等效矩形带宽% (ERB)："
	"6.23\\.c10^^-6^ %f^2 + 0.09339 %f + 28.52")
TERM (U"##hertzToErb (%x)")
DEFINITION (U"从声学频率到 ERB 频率：11.17 ln ((%x + 312) / (%x + 14680)) + 43")
TERM (U"##erbToHertz (%x)")
DEFINITION (U"(14680 %d - 312) / (1 - %d)，其中 %d = exp ((%x - 43) / 11.17)")
TERM (U"@`phonToDifferenceLimens` (%x)")
DEFINITION (U"从以方（phon）为单位的知觉响度（音强感觉）水平，到阈值之上的音强差辨限数量：30 · ((61/60)^^ %x^ − 1)。")
TERM (U"@`differenceLimensToPhon` (%x)")
DEFINITION (U"前者的逆函数：ln (1 + %x / 30) / ln (61 / 60)。")
TERM (U"##beta (%x, %y)")
TERM (U"##besselI (%n, %x)")
TERM (U"##besselK (%n, %x)")
NORMAL (U"关于带有数组的函数，参见 @@Scripting 5.7. Vectors and matrices@。")
MAN_END

MAN_BEGIN (U"Formulas 6. String functions", U"ppgb", 20180825)
INTRO (U"字符串函数是返回文本字符串或至少以一个文本字符串作为参数的函数。"
	"由于字符串计算在 @calculator、设置窗口或创建和修改公式中不太有用，因此本页仅给出脚本中字符串的示例，以便示例可以包含字符串变量。")
/*@praat
	string$ = "hallo"
	length = length (string$ + "dag")
	assert length = 8
@*/
TERM (U"#`length (a$)")
DEFINITION (U"给出字符串的长度。在执行")
		CODE2 (U"string$ = \"hallo\"")
		CODE2 (U"length = length (string$ + \"dag\")")
DEFINITION (U"之后，变量 %`length` 包含数字 8（顺便提一下，从这个例子中您可以看到变量可以与函数同名，而不会有混淆解释器的危险）。")
/*@praat
	head$ = left$ ("hallo", 3)
	assert head$ = "hal"
@*/
TERM (U"#`left$ (a$, n)")
DEFINITION (U"给出一个由 %`a$` 的前 %n 个字符组成的字符串。在执行")
		CODE2 (U"head$ = left$ (\"hallo\", 3)")
DEFINITION (U"之后，变量 %`head$` 包含字符串 \"hal\"。")
/*@praat
	english$ = "he" + right$ ("hallo", 3)
	assert english$ = "hello"
@*/
TERM (U"#`right$ (a$, n)")
DEFINITION (U"给出一个由 %`a$` 的后 %n 个字符组成的字符串。在执行")
		CODE2 (U"english$ = \"he\" + right$ (\"hallo\", 3)")
DEFINITION (U"之后，变量 %`english$` 包含字符串 \"hello\"。")
/*@praat
	assert mid$ ("hello", 3, 2) = "ll"
@*/
TERM (U"#`mid$ (\"hello\" , 3, 2)")
DEFINITION (U"给出一个由 “hello” 中从第三个字符开始的 2 个字符组成的字符串。结果为：ll。")
/*@praat
	where = index ("hallo allemaal", "al")
	assert where = 2
	assert index ("hallo allemaal", "fhjgfhj") = 0
@*/
TERM (U"#`index (a$, b$)")
DEFINITION (U"给出字符串 %`b$` 在字符串 %`a$` 中首次出现的位置索引。在执行")
		CODE2 (U"where = index (\"hallo allemaal\", \"al\")")
DEFINITION (U"之后，变量 %`where` 包含数字 2，因为第一个 “al” 从长字符串的第二个字符开始。"
	"如果第一个字符串不包含第二个字符串，则 %`index` 返回 0。")
/*@praat
	where = rindex ("hallo allemaal", "al")
	assert where = 13
	assert rindex ("hallo allemaal", "fhjgfhj") = 0
@*/
TERM (U"#`rindex (a$, b$)")
DEFINITION (U"给出字符串 %`b$` 在字符串 %`a$` 中最后一次出现的位置索引。在执行")
		CODE2 (U"where = rindex (\"hallo allemaal\", \"al\")")
DEFINITION (U"之后，变量 %`where` 包含数字 13，因为最后一个 “al” 从第 13 个字符开始。"
	"如果第一个字符串不包含第二个字符串，则 %`rindex` 返回 0。")
/*@praat
	where = startsWith ("internationalization", "int")
	assert where = 1
@*/
TERM (U"#`startsWith (a$, b$)")
DEFINITION (U"判断字符串 %`a$` 是否以字符串 %`b$` 开头。在执行")
		CODE2 (U"where = startsWith (\"internationalization\", \"int\")")
DEFINITION (U"之后，变量 %`where` 包含数字 1（真）。")
/*@praat
	where = endsWith ("internationalization", "nation")
	assert where = 0
@*/
TERM (U"#`endsWith (a$, b$)")
DEFINITION (U"判断字符串 %`a$` 是否以字符串 %`b$` 结尾。在执行")
		CODE2 (U"where = endsWith (\"internationalization\", \"nation\")")
DEFINITION (U"之后，变量 %where 包含数字 0（假）。")
TERM (U"#`replace$ (a$, b$, c$, n)")
DEFINITION (U"给出一个类似于 %`a$` 的字符串，但其中（最多 %n 处）出现的 %`b$` 被替换为字符串 %`c$`。在执行")
		CODE2 (U"s$ = replace$ (\"hello\", \"l\", \"m\", 0)")
DEFINITION (U"之后，变量 %`s$` 包含字符串 \"hemmo\"。在执行")
		CODE2 (U"s$ = replace$ (\"hello\", \"l\", \"m\", 1)")
DEFINITION (U"之后，变量 %`s$` 包含字符串 \"hemlo\"。数字 %n 决定了可以替换的 %`b$` 的最大出现次数。如果 %n 为 0，则替换所有出现的地方。")
TERM (U"#`index_regex (a$, b$)")
DEFINITION (U"判断字符串 %`a$` 首次匹配 @@regular expressions|正则表达式@ %`b$` 的位置。在执行")
		CODE2 (U"where = index_regex (\"internationalization\", \"a.*n\")")
DEFINITION (U"之后，变量 %`where` 包含数字 7。如果没有匹配项，则结果为 0。")
TERM (U"#`rindex_regex (a$, b$)")
DEFINITION (U"判断字符串 %`a$` 最后一次匹配 @@regular expressions|正则表达式@ %`b$` 的位置。在执行")
		CODE2 (U"where = rindex_regex (\"internationalization\", \"a.*n\")")
DEFINITION (U"之后，变量 %`where` 包含数字 16。如果没有匹配项，则结果为 0。")
TERM (U"#`replace_regex$ (a$, b$, c$, n)")
DEFINITION (U"给出一个类似于 %`a$` 的字符串，但其中（最多 %n 个）匹配 @@regular expressions|正则表达式@ %`b$` 的子字符串被替换为表达式 %`c$`。在执行")
		CODE2 (U"s$ = replace_regex$ (\"hello\", \".\", \"&&\", 0)")
DEFINITION (U"之后，变量 %`s$` 包含字符串 \"hheelllloo\"。如果没有匹配项，则结果为原始字符串 %`a$`。在执行")
		CODE2 (U"s$ = replace_regex$ (\"hello\", \".\", \"&&\", 1)")
DEFINITION (U"之后，变量 %`s$` 包含字符串 \"hhello\"。数字 %n 决定了可以替换的匹配文本片段的最大数量。如果 %n 为 0，则替换所有匹配的文本片段。")
TERM (U"#`string$ (number)")
DEFINITION (U"将数字格式化为字符串。因此，`string$ (5e6)`"
	"变成字符串 `5000000`，而 `string$ (56%)` 变成字符串 `0.56`。")
TERM (U"#`fixed$ (number, precision)")
DEFINITION (U"将数字格式化为在小数点后具有 %precision 位数字的字符串。因此，`fixed$ (72.65687, 3)`"
	"变成字符串 `72.657`，而 `fixed$ (72.65001, 3)` 变成字符串 `72.650`。"
	"在这些示例中，我们看到结果可以四舍五入，并且保留了末尾的零。"
	"总是会给出至少一位精度的数字，例如 `fixed$ (0.0000157, 3)` 变成字符串 `0.00002`。"
	"数字 0 总是变成字符串 `0`。")
TERM (U"#`percent$ (number, precision)")
DEFINITION (U"与 `fixed$` () 相同，但带有百分号。例如，`percent$ (0.157, 3)` 变成 `15.700%`，"
	"`percent$ (0.000157, 3)` 变成 `0.016%`，而 `percent$ (0.000000157, 3)` 变成 `0.00002%`。"
	"数字 0 总是变成字符串 `0`。")
/*@praat
	string$ = "5e6"
	assert 3 + number (string$) = 5000003
@*/
TERM (U"#`number (a$)")
DEFINITION (U"将字符串解析为数字。在执行")
		CODE2 (U"string$ = \"5e6\"")
		CODE2 (U"writeInfoLine: 3 + number (string$)")
DEFINITION (U"之后，信息窗口（Info window）中包含数字 5000003。")
TERM (U"#`date$ ( )")
DEFINITION (U"以以下格式给出日期和时间：")
		CODE2 (U"Mon Jun 24 17:11:21 2002")
DEFINITION (U"要在信息窗口中写入该月的第几天，您可以输入：")
		CODE2 (U"date$ = date$ ()")
		CODE2 (U"day$ = mid$ (date$, 9, 2)")
		CODE2 (U"writeInfoLine: \"The month day is \", day$, \".\"")
TERM (U"#`unicode$ (228)")
DEFINITION (U"给出第 228 个 Unicode 码点，即 \"ä\"。")
TERM (U"#`unicode (\"ä\")")
DEFINITION (U"给出 \"ä\" 的 Unicode 码点值，即 228。")
TERM (U"#`extractNumber (\"Type: Sound\" + newline$ + \"Name: hello there\" + newline$ + \"Size: 44007\", \"Size:\")")
DEFINITION (U"在长字符串中首次出现 “Size:” 之后寻找数字。结果：44007。"
	"这在尝试从长报告中获取信息的脚本中非常有用，例如以下在声音编辑器窗口中运行的脚本：")
		CODE2 (U"report$ = Editor info")
		CODE2 (U"maximumFrequency = extractNumber (report$, \"Spectrogram window length:\")")
TERM (U"#`extractWord$ (\"Type: Sound\" + newline$ + \"Name: hello there\" + newline$ + \"Size: 44007\", \"Type:\")")
DEFINITION (U"在长字符串中首次出现 \"Type:\" 之后寻找不带空格的单词。结果：Sound。")
TERM (U"#`extractLine$ (\"Type: Sound\" + newline$ + \"Name: hello there\" + newline$ + \"Size: 44007\", \"Name: \")")
DEFINITION (U"在长字符串中首次出现 “Name: ” 之后寻找该行的其余部分（包括空格）。"
	"结果：hello there。请注意 “Name: ” 是如何包含一个空格的，因此 “该行的其余部分” 以 %h 开头。")
TERM (U"#`backslashTrigraphsToUnicode$ (x$), unicodeToBackslashTrigraphs$ (x$)")
DEFINITION (U"例如将 \\bsct 转换成 \\ct 或反向转换。参见 @@Special symbols@。")
MAN_END

MAN_BEGIN (U"Formulas 7. Control structures", U"ppgb", 20031228)
ENTRY (U"if ... then ... else ... fi")
NORMAL (U"您可以在所有公式中使用条件表达式。例如，")
CODE (U"3 * if 52% * 3908 > 2000 then 5 else 6 fi")
NORMAL (U"的值为 15。除了 #`fi`，您也可以使用 #`endif`。")
NORMAL (U"另一个例子：您可以通过提供以下公式将 @Sound（声音） 的绝对振幅限制在 0.5 内：")
CODE (U"if abs(self)>0.5 then if self>0 then 0.5 else -0.5 fi else self fi")
ENTRY (U"分号")
NORMAL (U"分号结束公式的计算。如果您不想覆盖文本框中的长公式，这会很方便：公式")
CODE (U"800;sqrt(2)*sin(2*pi*103*0.5)+10^(-40/20)*randomGauss(0,1)")
NORMAL (U"的值为 800。")
MAN_END

MAN_BEGIN (U"Formulas 8. Attributes of objects", U"ppgb", 20221202)
NORMAL (U"您可以引用在 @@List of Objects|对象列表@ 中可见的对象的几个属性。"
	"为此，可以使用该对象的唯一 ID，或者使用该对象的类型和名称。"
	"因此，`object[113]` 指的是列表中编号为 113 的对象，"
	"而 `object[\"Sound hallo\"]` 指的是名为 “hallo” 的现有 Sound（声音） 对象"
	"（如果存在多个此类对象，则指的是最后创建的那个）。")
NORMAL (U"要引用属性，可以使用点号 (\".\")。"
	"因此，`object[\"Sound hallo\"].nx` 是名为 `hallo` 的 Sound（声音） 的采样数，而"
	"`1/object[\"Sound hallo\"].dx` 是其采样频率。")
ENTRY (U"在计算器中引用属性")
NORMAL (U"录制一段 Sound（声音）（如果您不知道该怎么做，请阅读 @Intro ），"
	"并将其命名为 `mysound`（或任何其他名称）。一个类似于 `3. Sound mysound` 的对象将出现在列表中。"
	"然后在 @calculator 中输入公式")
CODE (U"object[3].nx")
NORMAL (U"或")
CODE (U"object[\"Sound mysound\"].nx")
NORMAL (U"单击“确定”（OK）后，信息窗口（Info window）将显示采样数。由于您只需选择对象并从 @@Query submenu|查询子菜单@ 中选择 ##Get number of samples# 即可获得此结果，"
	"因此这些属性技巧在计算器中并不是很有用。"
	"我们将看到它们在创建和修改公式以及脚本中要有用得多。")
ENTRY (U"可能属性的列表")
NORMAL (U"以下属性可用：")
TERM (U"#`xmin`")
DEFINITION (U"对于 @Sound（声音）、@Pitch（基频/音高）、@Formant（共振峰）、"
	"@Spectrogram（声谱图）、@Intensity（音强）、@Cochleagram（耳蜗图）、@PointProcess（点过程） 或 @Harmonicity（谐噪比） 对象，为时间域的起点（通常为 0），以秒为单位；"
	"对于 @Spectrum（频谱） 对象，为最低频率（总是 0），以赫兹为单位；"
	"对于 @Excitation（激发图） 对象，为最低频率（通常为 0），以 Bark 为单位；"
	"对于 @Matrix（矩阵） 对象，为 %x 域的左边界。")
TERM (U"#`xmax`")
DEFINITION (U"对于 Sound（声音）、Pitch（基频/音高）、Formant（共振峰）、"
	"Spectrogram（声谱图）、Intensity（音强）、Cochleagram（耳蜗图）、PointProcess（点过程） 或 Harmonicity（谐噪比） 对象，为时间域的终点（通常为时长，如果 %xmin 为零），以秒为单位；"
	"对于 Spectrum（频谱） 对象，为最高频率（@@Nyquist frequency|奈奎斯特频率@），例如 11025 赫兹；"
	"对于 Excitation（激发图） 对象，为最高频率，通常为 25.6 Bark；"
	"对于 Matrix（矩阵） 对象，为 %x 域的右边界。")
TERM (U"#`ncol`")
DEFINITION (U"在 @Matrix（矩阵）、@TableOfReal 或 @Table（表） 对象中的列数。")
TERM (U"#`nrow`")
DEFINITION (U"在 @Matrix（矩阵）、@TableOfReal 或 @Table（表） 对象中的行数。")
TERM (U"#`col$` [%`i`]")
DEFINITION (U"在 @TableOfReal 或 @Table（表） 对象中第 %`i` 列的名称。")
TERM (U"#`row$` [%`i`]")
DEFINITION (U"在 @TableOfReal 对象中第 %`i` 行的名称。")
TERM (U"#`nx`")
DEFINITION (U"在 Sound（声音） 对象中的采样数；"
	"在 Pitch（基频/音高）、Formant（共振峰）、Spectrogram（声谱图）、Intensity（音强）、Cochleagram（耳蜗图） 或 Harmonicity（谐噪比） 对象中的分析帧数；"
	"在 Spectrum（频谱） 或 Excitation（激发图） 对象中的频率分量数（bins）；"
	"在 Matrix（矩阵） 对象中对 %x 域的划分份数（= %`ncol`）。")
TERM (U"#`dx`")
DEFINITION (U"在 Sound（声音） 对象中的采样周期（连续采样之间的时间距离）（采样频率的倒数），以秒为单位；"
	"在 Pitch（基频/音高）、Formant（共振峰）、Spectrogram（声谱图）、Intensity（音强）、Cochleagram（耳蜗图） 或 Harmonicity（谐噪比） 对象中的相邻帧时间步长，以秒为单位；"
	"在 Spectrum（频谱） 对象中频率分量（bin）的宽度，以赫兹为单位；"
	"在 Excitation（激发图） 对象中频率分量（bin）的宽度，以 Bark 为单位；"
	"在 Matrix（矩阵） 对象中单元格之间的水平距离。")
TERM (U"#`ymin`")
DEFINITION (U"对于 Spectrogram（声谱图） 对象，为最低频率（通常为 0），以赫兹为单位；"
	"对于 Cochleagram（耳蜗图） 对象，为最低频率（通常为 0），以 Bark 为单位；"
	"对于 Matrix（矩阵） 对象，为 %y 域的下边界。")
TERM (U"#`ymax`")
DEFINITION (U"对于 Spectrogram（声谱图） 对象，为最高频率，例如 5000 赫兹；"
	"对于 Cochleagram（耳蜗图） 对象，为最高频率，通常为 25.6 Bark；"
	"对于 Matrix（矩阵） 对象，为 %y 域的上边界。")
TERM (U"#`ny`")
DEFINITION (U"在 Spectrogram（声谱图） 或 Cochleagram（耳蜗图） 对象中的频带数；"
	"对于 Spectrum（频谱） 对象：总是 2（第一行为实部，第二行为虚部）；"
	"在 Matrix（矩阵） 对象中对 %y 域的划分份数（= %`nrow`）。")
TERM (U"#`dy`")
DEFINITION (U"在 Spectrogram（声谱图） 对象中相邻频带之间的距离，以赫兹为单位；"
	"在 Cochleagram（耳蜗图） 对象中相邻频带之间的距离，以 Bark 为单位；"
	"在 Matrix（矩阵） 对象中单元格之间的垂直距离。")
ENTRY (U"在创建公式中引用属性")
NORMAL (U"在创建新对象的公式中，您可以引用任何对象的属性，但通常您会希望引用正在创建的对象的属性。您可以通过两种方式来做到这一点。")
NORMAL (U"第一种方法是像上面一样使用对象的名称。选择 @@Create Sound from formula...|从公式创建声音...@，为其名称提供 %hello，"
	"为起始时间和结束时间提供任意值，然后输入以下公式：")
CODE (U"(x - object[\"Sound hello\"].xmin) / (object[\"Sound hello\"].xmax - object[\"Sound hello\"].xmin)")
NORMAL (U"当您编辑此声音时，您可以看到它在时间域内创建了一条从 0 上升到 1 的直线。")
NORMAL (U"如果正在创建的 Sound（声音） 称为 %goodbye，并且已经存在名为 %hello 的 Sound（声音），上面的公式也将起作用；"
	"当然，在这种情况下，`object[\"Sound hello\"].xmax` 指的是已经存在的声音的属性。")
NORMAL (U"如果公式引用正在创建的对象，则有更简短的方法：您根本不需要提供对象的名称，只需写为")
CODE (U"(x - xmin) / (xmax - xmin)")
NORMAL (U"您可以以这种隐式方式使用的属性是 %xmin、%xmax, %ncol, %nrow, %nx, %dx, %ny 和 %dy。"
	"为了消除由于同时存在脚本变量 %xmin 而引起的歧义（如果发生这种情况，Praat 会抱怨），您可以写为 `Self.xmin`。")
ENTRY (U"在修改公式中引用属性")
NORMAL (U"在修改现有对象的公式中，您以与创建公式相同的方式引用属性，"
	"即您不必指定正在修改的对象的名称。公式")
CODE (U"self * 20 ^ (- (x - xmin) / (xmax - xmin))")
NORMAL (U"导致声音呈指数衰减，使其在结束时仅有初始振幅的 5%。"
	"如果您同时将此公式应用于多个 Sound（声音） 对象，则在修改每个 Sound（声音） 时，xmax 将分别引用该 Sound（声音） 的结束时间。")
NORMAL (U"关于使用属性的更多示例，请参见下一页。")
MAN_END

MAN_BEGIN (U"Formulas 9. Data in objects", U"ppgb", 20170614)
NORMAL (U"通过使用方括号，您可以获取一些对象内部的值。")
ENTRY (U"在计算器中引用对象内容")
NORMAL (U"以下示例的结果可以使用 @calculator 进行检查。")
TERM (U"##object [%%objectName\\$  or id%, %rowNumber, %columnNumber]")
TERM (U"$$object [\"Matrix hello\", 10, 3]")
DEFINITION (U"给出名为 %hello 的 Matrix（矩阵） 第 10 行第 3 列的单元格中的值。")
TERM (U"$$object [5, 10, 3]")
DEFINITION (U"给出唯一 ID 为 5 的 Matrix（矩阵）（即在对象列表中标有数字 5）的第 10 行第 3 列单元格中的值。")
TERM (U"$$object [\"Sound hello\", 0, 10000]")
DEFINITION (U"给出 Sound（声音） %hello 的第 10000 个采样的值（以帕斯卡为单位），通道间取平均值。")
TERM (U"$$object [23, 1, 10000]")
DEFINITION (U"给出唯一 ID 为 23 的 Sound（声音） 左声道第 10000 个采样的值（以帕斯卡为单位）。")
TERM (U"$$object [23, 2, 10000]")
DEFINITION (U"给出唯一 ID 为 23 的 Sound（声音） 右声道第 10000 个采样的值（以帕斯卡为单位）。")
TERM (U"$$object [\"TableOfReal tokens\", 5, 12]")
DEFINITION (U"给出名为 %tokens 的 TableOfReal 第 5 行第 12 列单元格中的值。")
TERM (U"$$object [\"TableOfReal tokens\", 5, \"F1\"]")
DEFINITION (U"给出名为 %tokens 的 TableOfReal 中标为 %F1 列的第 5 行单元格中的值。")
TERM (U"$$object [\"TableOfReal tokens\", \"\\bsct\", \"F1\"]")
DEFINITION (U"给出名为 %tokens 的 TableOfReal 中第 %F1 列中标为 %%\\bsct% 行单元格中的值。")
TERM (U"$$object [\"Table listeners\", 3, \"m3ae\"]")
DEFINITION (U"给出名为 %listeners 的 Table（表） 第 %m3ae 列第 3 行单元格中的数值。")
TERM (U"$$object [\"Table listeners\", 3, 12]")
DEFINITION (U"给出名为 %listeners 的 Table（表） 第 12 列第 3 行单元格中的数值。")
TERM (U"$$object\\$  [\"Table results\", 3, \"response\"]")
DEFINITION (U"给出名为 %results 的 Table（表） 第 %response 列第 3 行单元格中的字符串值。")
TERM (U"$$object\\$  [\"Table results\", 3, 12]")
DEFINITION (U"给出名为 %results 的 Table（表） 第 12 列第 3 行单元格中的字符串值。")
TERM (U"$$object [\"PitchTier hello\", 8]")
DEFINITION (U"给出 PitchTier（音高层） %hello 中第 8 个点处的音高（以赫兹为单位）。")
NORMAL (U"对象之外的单元格（或采样、或点）被认为包含零值。")
ENTRY (U"插值")
NORMAL (U"一些对象内部的值可以被插值。")
TERM (U"$$object (\"Sound hello\", 0.7, 0)")
DEFINITION (U"给出 Sound（声音） %hello 在时间为 0.7 秒处的值（以帕斯卡为单位），通过距离 0.7 秒最近的两个采样之间的线性插值计算得出。声道间取平均值。")
TERM (U"$$object (\"Sound hello\", 0.7, 1)")
DEFINITION (U"给出 Sound（声音） %hello 的左声道在时间为 0.7 秒处的插值（以帕斯卡为单位）。")
TERM (U"$$object (\"Sound hello\", 0.7, 2)")
DEFINITION (U"给出 Sound（声音） %hello 的右声道在时间为 0.7 秒处的插值（以帕斯卡为单位）。")
TERM (U"$$object (\"Spectrogram hallo\", 0.7, 2500)")
DEFINITION (U"给出 Spectrogram（声谱图） %hallo 在时间为 0.7 秒且频率为 2500 赫兹处的值，通过距离该点最近的四个采样之间的线性插值计算得出。")
TERM (U"$$object (\"PitchTier hullo\", 0.7)")
DEFINITION (U"给出 PitchTier（音高层） %hullo 在时间为 0.7 秒处的音高（以赫兹为单位）。")
NORMAL (U"在插值中，对象时间域之外的时间被认为包含零值（这不适用于 PitchTier（音高层） 等对象，它们将进行 @@constant extrapolation|常数外推@）。")
ENTRY (U"在修改公式中引用对象内容")
NORMAL (U"假设您想使用复杂的方法来反转名为 `hello` 的 Sound（声音） 的内容（简单的方法是自 @Modify（修改） 菜单中选择 #Reverse（反转） ）。"
	"您选择此声音，然后选择 @@Copy...|复制...@ 以将其复制为一个新的 Sound（声音），并将其命名为 `hello_reverse`。"
	"您选择这个新的 Sound（声音） 并自 @Modify（修改） 菜单中选择 ##Formula...# 。公式将为")
CODE (U"object [\"Sound hello\", row, ncol + 1 - col]")
NORMAL (U"从此示例中，您可以看到 [ ] 之间的索引本身可以是公式，并且您可以使用隐式属性（例如 %ncol）和位置引用（例如 %col）（以及 %row，此处意味着对每个声道执行反转）。"
	"另一个替代公式是")
CODE (U"object (\"Sound hello\", xmax - x, y)")
NORMAL (U"至少在 %xmin 为零时有效。第二种方法的优点是如果两个声音具有不同的采样频率，它也能正确工作；"
	"缺点是它可能会在采样之间进行一些插值，这会降低声音质量（这里使用 %y 意味着对所有 %y values 执行反转，即所有声道）。")
ENTRY (U"在脚本中引用对象内容")
NORMAL (U"在脚本中，[ ] 之间的索引和 ( ) 之间的值本身可以是公式且包含变量。"
	"以下脚本计算了 Matrix（矩阵） 沿对角线的所有单元格之和。")
CODE (U"matrix = Create simple matrix: 10, 10, \"x*y\"")
CODE (U"sumDiagonal = 0")
CODE (U"for i to object[matrix].ncol")
	CODE1 (U"sumDiagonal += object [matrix, i, i]")
CODE (U"endfor")
CODE (U"writeInfoLine: \"The sum of the cells along the diagonal is \", sumDiagonal, \".\"")
NORMAL (U"这个例子完全可以使用动态菜单中的命令来编写：")
CODE (U"matrix = Create simple matrix: 10, 10, \"x*y\"")
CODE (U"sumDiagonal = 0")
CODE (U"ncol = Get number of columns")
CODE (U"for i to ncol")
	CODE1 (U"value = Get value in cell: i, i")
	CODE1 (U"sumDiagonal += value")
CODE (U"endfor")
CODE (U"writeInfoLine: \"The sum of the cells along the diagonal is \", sumDiagonal, \".\"")
NORMAL (U"第一个直接访问内容的版本不仅短了两行，而且速度快了三倍。")
MAN_END

}

/* End of file manual_formulas.cpp */
