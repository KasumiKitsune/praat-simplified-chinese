# Praat_ZH 未翻译内容查漏补缺任务单

生成日期：2026-06-14  
目标读者：接手翻译工作的下一位 AI  
仓库路径：`C:\Users\Sager\Desktop\Program\Praat_ZH`

## 一、结论概览

本次只做静态审计和交接整理，没有直接翻译源码。

当前项目仍有三类主要缺口：

1. 手册正文：主要集中在 `fon/manual_*.cpp`。`docs/manual` 是生成结果，不应直接修改。
2. 界面、菜单、表单文字：主要维护入口是 `generate_translation_map.py` 的 `EXACT_MAP`，生成结果是 `sys/praat_translate.cpp`。
3. 弹窗、错误、警告文字：大量来自 `Melder_throw`、`Melder_warning`、`Melder_require` 等调用，不一定经过 `praat_translate()`，需要单独处理。

静态扫描结果：

| 类型 | 候选数量 | 说明 |
| --- | ---: | --- |
| `EXACT_MAP` 现有条目 | 3056 | 当前 UI 翻译表条目数 |
| 主要手册英文候选 | 4574 | 仅统计 `fon/manual_*.cpp` 的正文宏，不含 `docs/manual` |
| UI、菜单、表单英文候选 | 6784 | 已排除 `sys/praat_translate.cpp` 和手册文件 |
| 弹窗、错误、警告英文候选 | 4346 | 来自 `Melder_*` 调用的启发式统计 |
| `docs/manual` HTML 文件 | 1935 | 生成产物，本轮检测到 0 个含中文页面 |

这些数字是启发式候选，不是语义证明。翻译时要人工判断是否真为用户可见文本，代码片段、文件格式标识、类名、函数名、命令名和链接锚点不要盲目翻译。

## 二、必须遵守的工作流边界

1. 不要直接改 `docs/manual/*.html`。手册应改 `fon/manual_*.cpp`，之后由项目流程生成 HTML。
2. UI 翻译优先改 `generate_translation_map.py` 的 `EXACT_MAP`，再运行：

```powershell
python generate_translation_map.py
```

3. `sys/praat_translate.cpp` 是生成结果。除非已经确认某段文字不可能从 `EXACT_MAP` 生成，否则不要把它当唯一维护源。
4. 弹窗和错误文本不要硬塞进 `EXACT_MAP` 后就宣称完成。先确认调用链是否经过 `praat_translate()`；如果不经过，应按源码调用点单独处理。
5. `MAN_BEGIN` 的页面标题、手册链接目标、对象类名、命令名、脚本函数名、文件格式关键字通常要保持英文稳定标识。正文解释可以翻译。
6. 修改手册时必须保留 Praat 手册标记语法，例如 `@@...@`、`@Object`、`#Menu`、`%emphasis`、`##code#`、反斜杠转义、宏名和括号结构。
7. 修改 UI 或手册后，至少执行：

```powershell
git diff --check
python generate_translation_map.py
git diff -- generate_translation_map.py sys/praat_translate.cpp
```

如翻译手册较多，建议再运行现有抽取脚本做审校材料：

```powershell
python extract_full_manuals_bilingual.py
python check_glossary_alignment.py
```

## 三、主要手册翻译优先级

用户这次强调“主要手册，次要的不用”，因此建议先翻 `fon` 下高频、核心、教学价值高的手册；暂不展开 `gram`、`dwtools` 等扩展模块手册。

### A 级：最值得优先补的大块手册

| 文件 | 英文候选 | 当前状态 | 重点页面 |
| --- | ---: | --- | --- |
| `fon/manual_scripting.cpp` | 734 | 已有少量中文，仍是最大缺口 | `Scripting 6.9. Calling from the command line`、`Scripting 6.6. Controlling the user`、`ScriptEditor`、`Scripting 4.1. Selecting objects`、`Scripting 3.6. “For” loops` |
| `fon/manual_formulas.cpp` | 652 | 基本仍为英文 | `Formulas 5. Mathematical functions`、`Formulas 6. String functions`、`Formulas 9. Data in objects`、`Formulas 3. Operators` |
| `fon/manual_Fon.cpp` | 575 | 已有少量中文，核心对象仍缺 | `Keyboard shortcuts`、`Log files`、`Matrix`、`Manipulation`、`IntensityTier` |
| `fon/manual_Picture.cpp` | 357 | 基本仍为英文 | `Picture window`、`Axes...`、`Logarithmic marks left/right/top/bottom...`、`Viewport text...`、`PostScript settings...` |
| `fon/manual_glossary.cpp` | 255 | 部分英文术语页仍未补 | `vector value interpolation`、`vector peak interpolation`、`time domain` |

建议第一轮先处理 `manual_scripting.cpp` 和 `manual_formulas.cpp`。这两块直接影响用户学习脚本、公式、自动化和计算器，是主手册里最明显的缺口。

### B 级：声学和语音分析核心手册

| 文件 | 英文候选 | 重点页面 |
| --- | ---: | --- |
| `fon/manual_spectrum.cpp` | 381 | `Sound: To Spectrogram...`、`Sound: To Spectrum...`、`Spectrogram`、`Spectrum` |
| `fon/manual_formant.cpp` | 349 | `Formant: Track...`、`Formant`、`FormantGrid`、`Formant: Get mean...` |
| `fon/manual_pitch.cpp` | 324 | `Pitch`、`PitchTier`、`Sound: To Harmonicity (ac)...` |
| `fon/manual_pointProcess.cpp` | 312 | `PointProcess`、`PointProcess: Get jitter (local)...`、`PointProcess: Get jitter (ddp)...` |

这些都是 Praat 用户常用的声学分析手册，建议在 A 级完成后继续处理。

### C 级：实验与专题手册

| 文件 | 英文候选 | 重点页面 |
| --- | ---: | --- |
| `fon/manual_ExperimentMFC.cpp` | 431 | `ExperimentMFC 8. Running multiple experiments`、`ExperimentMFC 2.1. The experiment file`、`ExperimentMFC 5.2. Stimulus-dependent response buttons`、`ExperimentMFC 3.3. A 4I-oddity experiment` |
| `fon/manual_Sampling.cpp` | 94 | `Get sample number from time...`、`Get frame number from time...`、`Get time from sample number...` |

`ExperimentMFC` 体量大，但使用面比脚本、公式、音频分析窄。建议作为独立批次处理。

### D 级：常用手册的小尾巴

这些文件整体已经中文化较多，只需要查漏补缺：

| 文件 | 英文候选 | 说明 |
| --- | ---: | --- |
| `fon/manual_annotation.cpp` | 53 | 主要集中在 `WordList`，`TextGrid` 残留很少 |
| `fon/manual_tutorials.cpp` | 45 | `Objects window`、`Save as text file...`、`Periodicity menu` 等小段残留 |
| `fon/manual_soundFiles.cpp` | 9 | 只剩少量文件读写说明 |
| `fon/manual_Manual.cpp` | 1 | 仅一个残留候选 |

这些适合作为收尾任务，不建议为了它们打断 A、B 级大块翻译。

### 暂缓或无需处理

本轮扫描中以下 `fon/manual_*.cpp` 没发现明显英文正文候选，或属于低优先级维护材料：

`fon/manual_sound.cpp`、`fon/manual_commands.cpp`、`fon/manual_functions.cpp`、`fon/manual_demoWindow.cpp`、`fon/manual_exampleSound.cpp`、`fon/manual_programming.cpp`、`fon/manual_references.cpp`、`fon/manual_licenses.cpp`、`fon/manual_SpeechRecognizer.cpp`、`fon/manual_voice.cpp`、`fon/manual_whatsnew.cpp`。

其中 `manual_whatsnew.cpp` 属于更新日志性质，即使有少量英文也不属于“主要手册”。

## 四、UI、菜单、表单文字补漏

### 主要 UI 候选

| 文件 | 候选数 | 示例 |
| --- | ---: | --- |
| `sys/praat_picture.cpp` | 226 | `left Horizontal range (inches)`、`right Vertical range (inches)` |
| `fon/praat_Sound.cpp` | 243 | `LongSound: Extract part...`、`left Time range (s)` |
| `fon/praat_Tiers.cpp` | 208 | `AmplitudeTier: Get shimmer (local)...`、`Shortest period (s)` |
| `fon/praat_TextGrid_init.cpp` | 100 | `Text`、`Xwaves label file` |
| `fon/praat_Matrix.cpp` | 94 | `Name`、`Number of columns`、`ymin` |
| `foned/SoundAnalysisArea.cpp` | 85 | `No spectrogram is visible. First choose "Show spectrogram" from the Spectrogram menu.` |
| `sys/praat_objectMenus.cpp` | 72 | `Control`、`Window`、`Create` |
| `sys/ScriptEditor.cpp` | 31 | `Add to menu`、`Window`、`Menu` |

建议优先处理以上文件，因为它们更靠近普通用户的主窗口、脚本窗口、Picture 窗口、Sound、TextGrid 和分析面板。

### 大量但偏扩展模块的 UI 候选

这些文件候选很多，但更像专门模块或扩展功能。若只做“主要内容”，可以放到后续批次：

| 文件 | 候选数 | 示例 |
| --- | ---: | --- |
| `dwtools/praat_David_init.cpp` | 2197 | `Match begin positions`、`Match end positions` |
| `gram/praat_gram.cpp` | 424 | `Spreading rate`、`Activity range` |
| `fon/praat_uvafon_init.cpp` | 416 | `myCorpus`、`wav` |
| `LPC/praat_LPC_init.cpp` | 407 | `Quefrency range (s)`、`FormantPath: Draw as grid` |
| `dwtools/praat_DataModeler_init.cpp` | 315 | `Create simple DataModeler` |
| `dwtools/praat_MDS_init.cpp` | 297 | `Create letter R example` |
| `dwtools/praat_HMM_init.cpp` | 182 | `GaussianMixture: Draw concentration ellipses` |
| `stat/praat_Stat.cpp` | 172 | `Get mean difference`、`Column number` |
| `FFNet/praat_FFNet_init.cpp` | 157 | `Create FFNet` |
| `EEG/praat_EEG.cpp` | 156 | `Get channel name` |

## 五、弹窗、错误、警告补漏

弹窗和错误文本不一定走 UI 翻译表。接手者应优先处理用户最容易遇到的主路径，而不是先翻专业模块深处的异常。

### 主要弹窗候选

| 文件 | 候选数 | 示例 |
| --- | ---: | --- |
| `sys/Formula.cpp` | 471 | `No object with number`、`Formula: stack overflow. Please simplify your formulas.` |
| `sys/Interpreter.cpp` | 228 | `You cannot use “pi” as the name of a variable.` |
| `sys/Ui.cpp` | 101 | `should be greater than 0.0`、`Element` |
| `sys/praat_script.cpp` | 59 | `Missing space in name.`、`No object with that name.` |
| `sys/praat_picture.cpp` | 33 | `Mouse commands are not available inside pictures.` |
| `fon/TextGrid.cpp` | 82 | `Text point not created.`、`Text interval not created.` |
| `fon/TextGrid_Sound.cpp` | 65 | `Nothing to be done, because you asked neither for word alignment nor for phoneme alignment.` |
| `fon/Sound.cpp` | 39 | `Sound not created.`、`Cannot create sounds with more than...` |
| `foned/TextGridArea.cpp` | 31 | `Cannot add a boundary at...` |
| `foned/SoundRecorder.cpp` | 43 | `No sound driver present.`、`Out of memory.` |

### 通用底层错误，可后续集中处理

| 文件 | 候选数 | 说明 |
| --- | ---: | --- |
| `melder/abcio.cpp` | 187 | 文本、数值、枚举读取错误 |
| `melder/melder_audiofiles.cpp` | 83 | 音频文件读写错误 |
| `melder/melder_alloc.cpp` | 39 | 内存分配错误 |
| `melder/melder_audio.cpp` | 38 | 音频播放底层错误 |

底层错误覆盖面广，但翻译时要更谨慎，避免破坏文件格式诊断信息和开发调试语义。

## 六、建议交给下一位 AI 的执行批次

### 批次 1：主手册大缺口

目标文件：

1. `fon/manual_scripting.cpp`
2. `fon/manual_formulas.cpp`

要求：

1. 只翻译正文解释、章节说明、教程叙述和用户可读标题。
2. 保留脚本命令、函数名、对象类型、菜单命令锚点和链接目标。
3. 翻译后运行抽取脚本生成审校材料。

### 批次 2：主窗口、Picture、核心对象手册

目标文件：

1. `fon/manual_Fon.cpp`
2. `fon/manual_Picture.cpp`
3. `fon/manual_glossary.cpp`

要求：

1. 优先翻 `Keyboard shortcuts`、`Log files`、`Matrix`、`Picture window`、`vector value interpolation`、`vector peak interpolation`。
2. 术语对齐 `praat_glossary.md`，不要临时创造多套译名。

### 批次 3：声学分析手册

目标文件：

1. `fon/manual_spectrum.cpp`
2. `fon/manual_formant.cpp`
3. `fon/manual_pitch.cpp`
4. `fon/manual_pointProcess.cpp`

要求：

1. `Spectrogram`、`Spectrum`、`Formant`、`Pitch`、`PointProcess` 作为对象名保留英文。
2. 解释性短语翻译为中文，例如 “One of the types of objects in Praat.” 可以统一处理为“Praat 中的一种对象类型。”
3. 参数名如果同时是 UI 表单标签，应参考现有 UI 译名。

### 批次 4：ExperimentMFC 专题手册

目标文件：

1. `fon/manual_ExperimentMFC.cpp`

要求：

1. 保留实验文件格式示例中的关键字段和值。
2. 翻译解释段落、说明文字、操作指引。
3. 不要翻译会被复制到实验文本文件中的格式标识。

### 批次 5：常用手册小尾巴

目标文件：

1. `fon/manual_annotation.cpp`
2. `fon/manual_tutorials.cpp`
3. `fon/manual_soundFiles.cpp`
4. `fon/manual_Manual.cpp`

要求：

1. 只做补漏，不扩大到非主要手册。
2. 重点检查 `WordList`、`Objects window`、`Save as text file...`、`Sound files 3. Files that Praat can read`。

### 批次 6：主 UI 翻译表补漏

目标文件：

1. `generate_translation_map.py`
2. 生成后的 `sys/praat_translate.cpp`

优先补这些来源中的常用文本：

1. `sys/praat_picture.cpp`
2. `fon/praat_Sound.cpp`
3. `fon/praat_Tiers.cpp`
4. `fon/praat_TextGrid_init.cpp`
5. `fon/praat_Matrix.cpp`
6. `sys/praat_objectMenus.cpp`
7. `sys/ScriptEditor.cpp`
8. `foned/SoundAnalysisArea.cpp`

要求：

1. 在 `EXACT_MAP` 增补精确英文键。
2. 运行 `python generate_translation_map.py`。
3. 检查 `sys/praat_translate.cpp` 中对应条目是否生成。

### 批次 7：主路径弹窗和错误

目标文件：

1. `sys/Formula.cpp`
2. `sys/Interpreter.cpp`
3. `sys/Ui.cpp`
4. `sys/praat_script.cpp`
5. `fon/TextGrid.cpp`
6. `fon/TextGrid_Sound.cpp`
7. `fon/Sound.cpp`
8. `foned/TextGridArea.cpp`
9. `foned/SoundRecorder.cpp`

要求：

1. 先确认这些文本是否经过统一错误显示函数。
2. 如果直接改源码字符串，保留变量拼接、引号、标点和单位。
3. 不要翻译对象内部名称、文件格式标识、脚本关键字。

## 七、翻译规范

1. 中文要自然、简洁，避免机器直译。
2. Praat 对象名、类名、函数名、命令锚点通常保留英文，例如 `Sound`、`TextGrid`、`Pitch`、`Formant`、`PointProcess`。
3. 菜单和按钮可采用中文译名，但必须与 UI 翻译表一致。
4. 参数名优先使用已有译法。例如 `Time range (s)`、`Frequency range (Hz)`、`From time (s)`、`To time (s)` 不要每个文件翻成不同说法。
5. `undefined`、`NaN`、`ooTextFile`、脚本函数名、实验文件字段名、文件扩展名应保留英文。
6. 引用、链接和手册标记只翻译可读文本，不改结构：

```cpp
NORMAL (U"To view your history, you first open a @ScriptEditor with @@New Praat script@ or @@Open Praat script...@.")
```

可以翻译为中文解释，但 `@ScriptEditor`、`@@New Praat script@`、`@@Open Praat script...@` 这类链接目标应谨慎保留。

## 八、建议的验收清单

每完成一个批次，至少检查：

1. `git diff --check` 通过。
2. 手册宏、括号、字符串引号没有被破坏。
3. 对 `generate_translation_map.py` 的修改已经重新生成 `sys/praat_translate.cpp`。
4. 没有直接修改 `docs/manual/*.html`。
5. `rg -n "TODO|未翻译|待翻译" fon/manual_*.cpp generate_translation_map.py sys` 没有留下临时标记，除非明确是任务单注释。
6. 术语与 `praat_glossary.md` 基本一致。
7. 能构建时再跑一次构建；不能构建时，在交接说明里写明未构建原因。

## 九、本次扫描命令口径

本次审计使用了静态脚本抽取这些候选：

1. `fon/manual_*.cpp` 中 `INTRO`、`NORMAL`、`ENTRY`、`TERM`、`DEFINITION`、`LIST_ITEM` 等正文宏里的英文字符串。
2. 核心源码中的 `FORM`、`LABEL`、`BOOLEAN`、`CHOICE`、`OPTION`、`Gui*`、`praat_addAction`、`praat_addMenuCommand` 等 UI 创建点。
3. `Melder_throw`、`Melder_warning`、`Melder_require`、`Melder_information` 等弹窗和错误输出点。

扫描结果只用于排期。真正翻译时，要逐条判断用户可见性和是否应保留英文标识。
