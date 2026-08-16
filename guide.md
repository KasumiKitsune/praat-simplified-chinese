# Praat ZH AI Agent 维护指南

本文件面向后续接手本仓库的 AI agent。它不是项目宣传稿，而是任务执行手册：先判断任务属于哪条工作流，再按现有代码结构修改、生成和验证，避免被旧说明或表面文件误导。

## 0. 接任务后的默认动作

1. 先看工作区状态，不要覆盖用户已有改动：

   ```powershell
   git status --short
   git diff --stat
   ```

2. 再按任务类型定位源码：

   - UI / 菜单 / 对话框文字：优先看 `tools/generate_translation_map.py` 和 `sys/praat_translate.cpp`。
   - 内置帮助手册：看 `fon/manual_*.cpp`，必要时用抽取脚本生成双语审校材料。
   - Windows 文件拖拽：看 `sys/motifEmulator.cpp`、`sys/GuiMenu.cpp`、`sys/praat_objectMenus.cpp`。
   - 启动时图像窗口隐藏：看 `sys/praat_picture.cpp` 和 `sys/praat_objectMenus.cpp`。
   - 发布打包：看 `.github/workflows/release-windows.yml`、`README.txt`。

3. 修改前先读相关上下文。不要只凭英文字符串全局替换；Praat 有大量手写宏、菜单别名和旧拼写，错误替换很容易导致启动时报错或 UI 不匹配。

4. 如果用户给的是截图里的“漏网之鱼”，先把截图转成受控清单和定位表，不要立刻做全仓库审计。清单越窄，修改越快也越安全。

## 1. 项目现状速览

本仓库是 Praat 源码的非官方简体中文本地化版本，不是基于 gettext/Qt/i18n 框架的普通翻译项目。当前中文化主要由三部分组成：

- 运行时 UI 翻译：`sys/praat_translate.cpp` 中的 `g_translation_map`，通过 `praat_translate()` 被 GUI 和表单代码调用。
- 内置手册翻译：直接改 `fon/manual_*.cpp` 中的 Praat 手册源字符串。
- 本地化功能差异：Windows 原生拖拽打开文件、启动时默认隐藏 Picture window，并在 Objects 窗口的 Praat 菜单提供打开入口。

当前仓库还包含若干用于抽取、恢复、审校翻译的 Python 脚本和中间产物。永久性翻译维护应优先改规范源头，不要把临时审校输出当作唯一真相。

## 2. UI 翻译工作流

### 2.1 真正的入口

UI 翻译运行时入口是：

- `sys/praat_translate.h`
- `sys/praat_translate.cpp`

翻译函数已接入这些常见控件路径：

- `sys/GuiButton.cpp`
- `sys/GuiCheckButton.cpp`
- `sys/GuiDialog.cpp`
- `sys/GuiLabel.cpp`
- `sys/GuiMenu.cpp`
- `sys/GuiMenuItem.cpp`
- `sys/Ui.cpp`

如果某个新 UI 文本完全没有被翻译，先确认它是否经过上述 GUI/Ui 创建路径。不要只往翻译表里加词条后就假定完成。

### 2.2 推荐修改方式

`sys/praat_translate.cpp` 是可编译产物，但当前更稳的维护入口是 `tools/generate_translation_map.py` 里的 `EXACT_MAP`。

常规流程：

1. 在源码中找到未翻译英文原文，保持原文逐字符准确。
2. 在 `tools/generate_translation_map.py` 的 `EXACT_MAP` 里添加或修正映射。
3. 运行生成脚本：

   ```powershell
   python tools/generate_translation_map.py
   ```

4. 检查生成结果：

   ```powershell
   git diff -- tools/generate_translation_map.py sys/praat_translate.cpp
   Get-Content -TotalCount 40 tools/debug_translations.txt
   Get-Content -TotalCount 40 tools/translation_candidates.txt
   ```

5. 需要给其他 AI 或人工审校导出翻译库时，再运行：

   ```powershell
   python tools/extract_translations.py
   ```

注意：

- `tools/debug_translations.txt` 和 `tools/translation_candidates.txt` 是生成/审校辅助文件，默认被 `.gitignore` 忽略，不要把它们当作需要发布的源码。
- `tools/praat_translations.json` 和 `tools/praat_translations.txt` 是从 `EXACT_MAP` 导出的参考资料，不是运行时读取的文件。
- `recovered_exact_map.py` 和 `scratch/` 更像恢复或实验材料。除非任务明确要求恢复旧翻译，否则不要把它们并回主流程。

### 2.3 匹配规则和坑

翻译键必须精确匹配 Praat 传入 `praat_translate()` 的原文。

- 保留上游拼写错误。例如当前有 `"- Draw spectogram to picture window:"`，这里的 `spectogram` 少了 `r`，但必须照抄。
- 保留前后缀、空格、冒号、省略号、换行和别名分隔符。`"Open Picture window"`、`"Open Picture window..."`、`"- Query pitch:"` 是不同键。
- `tools/generate_translation_map.py` 会按 Python `sorted()` 输出 `g_translation_map`，不要手动维护排序。
- 中文在生成的 C++ 里会转为 `\uXXXX` 形式，这是脚本行为。不要手写一半 UTF-8、一半转义导致 diff 难审。
- `praat_translate()` 只额外处理前导空格复用；其他变化不会自动模糊匹配。

### 2.4 什么时候可以直接改 `sys/praat_translate.cpp`

只有在做非常小的临时验证时才考虑直接改生成后的 C++。如果最终要保留改动，必须同步回 `tools/generate_translation_map.py`，否则下一次运行脚本会把改动覆盖。

### 2.5 截图漏网 UI 的高效处理

当用户只给截图并说“把这些漏网之鱼翻译一下”时，不要从截图直接扩展成全项目翻译审计。先要求或自行整理一个最小清单：

```text
类型：菜单 / 按钮 / 对话框 / 弹窗 / 手册
英文原文：Extract intervals
建议中文：提取区间
出现场景：TextGrid 编辑器菜单
截图：用户已附
备注：只处理本条，不扩大到相邻菜单
```

推荐执行顺序：

1. 先抄出截图中可见的英文原文，保持大小写、冒号、省略号和前后空格。
2. 给出定位表，再动手修改：

   ```text
   英文原文 | 类型 | 命中的文件 | 修改入口 | 建议中文 | 状态
   ```

3. 每个英文原文优先做精确搜索，搜索预算控制在 2 次左右。找不到就列入“未定位”，不要因为一个短词把全仓库都扫一遍。
4. 普通 UI / 菜单 / 按钮优先回到 `tools/generate_translation_map.py` 的 `EXACT_MAP`；确认不经过 `praat_translate()` 时，才继续追具体 GUI 创建路径。
5. 弹窗、报错、警告要单独标记。许多 `Melder_throw` / `Melder_warning` 文案可能不经过普通 UI 翻译表，不要硬塞进 `EXACT_MAP` 后就宣称完成。
6. 用户只要求修截图里的几条时，不要生成大而全的 `implementation_plan.md`；定位表足够，确认后直接按清单改。

可直接复用的任务提示：

```text
请根据下面的漏网英文清单补翻译。不要做全项目审计，不要写 implementation_plan。
只处理我列出的字符串；每个字符串最多精确搜索 2 次，找不到就报告未定位。

规则：
1. 普通 UI / 菜单 / 按钮优先修改 tools/generate_translation_map.py，并按项目流程更新 sys/praat_translate.cpp。
2. 手册正文只修改 fon/manual_*.cpp，不修改 docs/manual 生成结果。
3. 弹窗 / 报错单独标记；如果不经过 praat_translate，不要硬塞进 EXACT_MAP。
4. 修改前先给出“英文原文 | 位置 | 修改入口 | 建议中文”的定位表。
5. 我确认后再执行修改。

漏网清单：
- ...
```

## 3. 内置帮助手册工作流

### 3.1 真正的入口

内置手册位于各模块的 `manual_*.cpp`，其中高频文件包括：

- `fon/manual_tutorials.cpp`
- `fon/manual_annotation.cpp`
- `fon/manual_sound.cpp`
- `fon/manual_soundFiles.cpp`
- `fon/manual_glossary.cpp`
- `fon/manual_Fon.cpp`

这些文件不是 Markdown。它们使用 Praat 自研手册宏，例如 `MAN_BEGIN`、`INTRO`、`NORMAL`、`LIST_ITEM`、`ENTRY`、`TERM`。

### 3.2 手册漏网内容的快速定位

手册相关任务也不要从 `docs/manual/*.html` 或全仓库英文短词开始搜。更高效的定位顺序是：

1. 如果截图或页面顶部能看到手册页标题，先精确搜索标题：

   ```powershell
   rg -n 'MAN_BEGIN\s*\(\s*U"TextGridEditor"' fon dwtools stat gram
   ```

   如果标题来自生成的 HTML 文件名，例如 `TextGridEditor.html`，优先回到 `fon/manual_*.cpp` 查对应 `MAN_BEGIN`。

2. 如果只有正文里的英文短句，先限定手册源文件：

   ```powershell
   rg -n -F "every interval whose label" fon/manual_*.cpp
   rg -n -F "every interval whose label" dwtools/manual_*.cpp stat/manual_*.cpp gram/manual_*.cpp
   ```

3. 如果短句太普通，例如 `the text`、`is equal to`、`click`，不要直接全仓库扫。先结合截图里的页面标题、对象名、菜单名、章节名或相邻句子扩大成更长的搜索片段。
4. 定位到页面后，只改包含该页面的 `manual_*.cpp` 段落；不要修改 `docs/manual/*.html`，这些是生成产物。
5. 找不到源头时，输出“未定位：英文原文 + 已搜索范围 + 下一步建议”，不要为了凑结果扩大成手册全量审计。

手册漏网清单建议使用这种格式：

```text
页面标题：TextGridEditor
英文原文：every interval whose label...
建议中文：每个标签为……的区间
源文件范围：fon/manual_annotation.cpp 优先
是否允许调整链接文本：否，链接目标保持英文
```

现有 `tools/extract_full_manuals_bilingual.py` 和 `tools/extract_manuals_bilingual.py` 更适合“已有手册改动后的审校”，不是截图漏网任务的第一步。截图漏网任务应先定位源文件和页面，再按需要运行脚本导出审校材料。

### 3.3 链接和页面标题规则

手册最容易踩坑的是 dangling link。

- `MAN_BEGIN (U"PageTitle", ...)` 的页面标题必须保持官方英文标题，除非你同时能证明所有引用都同步更新且不会破坏上游页面查找。
- `@PageTitle` 和 `@@PageTitle@` 中的链接目标必须是存在的英文页面标题。
- 如需显示中文链接文本，用这种形式：

  ```cpp
  NORMAL (U"请参见 @@waveform|波形图@。")
  ```

- 不要写成 `@波形图` 或 `@@波形图@`，这会让 Praat 在启动/打开手册时查找不存在的页面。
- `##Command#`、`%emphasis%`、`\xx`、`\uXXXX`、`@@Page|label@` 这些标记要按 Praat 语法保留。

### 3.4 审校辅助脚本

当前有两条脚本路径，但要注意它们的比较基准：

- `tools/extract_full_manuals_bilingual.py`：从当前源码和 `git show HEAD:<file>` 抽取核心页面，输出 `tools/manuals_bilingual_review.txt`。因为当前 `HEAD` 已经包含中文手册翻译，所以它不能稳定代表“官方英文原文”；更适合在你刚做了未提交手册改动时查看当前工作区相对 HEAD 的变化。
- `tools/extract_manuals_bilingual.py`：基于 `git diff fon/` 提取未提交 diff。只适合当前有手册改动时使用。
- 如果需要真正的上游英文原文，应改用 `upstream/master` 或匹配发布版本的上游 tag，例如 `git show upstream/master:fon/manual_tutorials.cpp`，然后再和当前中文源码对照。

使用建议：

```powershell
python tools/extract_full_manuals_bilingual.py
```

或在有手册 diff 后：

```powershell
python tools/extract_manuals_bilingual.py
```

`tools/manuals_bilingual_review.txt` 是审校材料，不是手册源码。发现问题后要回到 `fon/manual_*.cpp` 修改。

### 3.5 术语表维护

术语表文件是 `tools/praat_glossary.md`，生成入口是 `tools/build_glossary.py`。如果发现术语不准，优先改 `tools/build_glossary.py` 里的 `MANUAL_OVERRIDES`，再重新生成：

```powershell
python tools/build_glossary.py
```

术语原则：

- Praat 对象类型和编辑器类名优先保留英文专名，再用括号给中文释义，例如 `TextGrid（文本标注）`、`SoundEditor（声音编辑器）`。
- UI 命令尽量和 `tools/praat_translations.json` / `tools/generate_translation_map.py` 保持一致，例如 `Remove` 对应“删除”，`Inspect` 对应“检查”。
- 声学概念避免过度合并。`tier` 译为“层”，`boundary` 译为“边界”，`time domain` 译为“时间域”；只有在上下文需要时再补充说明。
- 不要把 `tools/praat_glossary.md` 当作手工孤立文件长期维护；否则下一次运行生成脚本会覆盖手改结果。

## 4. 本地化功能改动入口

### 4.1 Windows 原生文件拖拽

现有实现已经有完整链路：

- `sys/motifEmulator.cpp`：Windows/Motif 层启用 `DragAcceptFiles`，处理 `WM_DROPFILES`，逐个调用打开回调。
- `sys/GuiMenu.cpp`：Cocoa 侧有 `openFiles` 回调，错误文案也走翻译表。
- `sys/praat_objectMenus.cpp`：`cb_openDocument` / `cb_finishedOpeningDocuments` 负责把文件读入 Praat 对象系统，并通过 `Gui_setOpenDocumentCallback` 绑定回调。

如果拖拽有问题，先追这条链路。不要另写一套文件读取逻辑绕过 Praat 的对象加载流程。

### 4.2 启动时默认隐藏 Picture window

现有实现不是删除 Picture window，而是在初始化后隐藏窗口：

- `sys/praat_picture.cpp` 中 `praat_picture_init(bool showPictureWindowAtStartUp)` 会先创建 Picture 相关对象和菜单，再在 `!showPictureWindowAtStartUp` 时隐藏实际窗口。
- `sys/praat_objectMenus.cpp` 在 Objects 窗口的 Praat 菜单注册 `"Open Picture window"`，中文显示由翻译表映射为“打开图像窗口 (Praat Picture)”。

不要为了“隐藏图像窗口”跳过 Picture 初始化；绘图、脚本和手册中的 Picture 功能仍依赖这些对象存在。

## 5. 构建、验证和打包

### 5.1 本地 Windows 构建

本地 Windows 开发通常使用 MSYS2 shell 调用仓库根目录的 `Makefile`。当前本机常用命令为：

```powershell
$env:MSYSTEM="UCRT64"
C:\msys64\usr\bin\bash.exe -lc "cd /c/path/to/Praat_ZH && make PRAAT_COMPILER=gcc -j16"
```

构建成功后根目录生成 `Praat.exe`。如果只改文档或 guide，不需要构建；如果改 C++、翻译表或手册源字符串，至少应尝试本地构建，或明确说明未构建原因。

### 5.2 最低验证清单

按改动类型选择验证：

- UI 翻译：运行 `python tools/generate_translation_map.py`，检查 `sys/praat_translate.cpp` 中目标键存在；能构建时再构建。
- 手册翻译：检查 `@@...@` / `@...` 链接目标仍是英文页面；能构建/启动时确认没有 dangling link 警告。
- Windows 拖拽：构建后把 `.wav`、`.TextGrid`、`.praat` 等文件拖入窗口，确认对象列表或脚本编辑器有响应。
- Picture window 行为：启动后确认 Picture window 默认不弹出；通过 Objects 窗口 `Praat -> Open Picture window` 可打开。
- 发布改动：确认 `README.txt`、`README.md` 和 workflow 中的说明一致。

### 5.3 本地打包

本地 Windows zip 包只包含运行必需项和简短说明：

```powershell
Compress-Archive -Path Praat.exe, README.txt -DestinationPath Praat-ZH-Windows-x64.zip -Force
```

`*.zip` 和 `Praat.exe` 默认被 `.gitignore` 忽略，不要误以为未跟踪就是源码遗漏。

## 6. GitHub Actions 发布工作流

发布工作流位于 `.github/workflows/release-windows.yml`，实际包含：

- `build-windows-x64`：在 `windows-latest` 上用 MSYS2 `CLANG64` 构建 `make PRAAT_ARCH=x64v3 -j2`，产出 Windows x64 zip。
- `build-macos`：在 `macos-26` 上准备 Xcode project，必要时从上游 release 下载 `praatXXXX_xcodeproj.zip`，并把 `sys/praat_translate.cpp` 注入 `praat_mac` target，产出 macOS universal dmg。
- `create-release`：下载两个构建产物，用 `gh release create` 创建 GitHub release。

该 workflow 是手动触发的 `workflow_dispatch`，输入包括：

- `tag_name`
- `release_name`
- `praat_upstream_tag`
- `praat_version_digits`
- `xcodeproj_url`
- `draft`
- `prerelease`

不要把它描述成自动随 push 发布，也不要说它构建 Linux 或 Windows ARM64。当前 release 产物只有 Windows x64 zip 和 macOS universal dmg。

如果新增了需要 macOS 编译的 `.cpp` 文件，必须检查 Xcode project 注入逻辑是否也要更新；否则 Windows 本地构建可能过了，macOS CI 会漏文件。

## 7. Agent 输出和提交前检查

完成任务前给出真实验证结果，不要只说“应该可以”。

推荐最后检查：

```powershell
git status --short
git diff -- guide.md
```

如果改了源码，再补充对应构建或脚本命令。若因为环境限制没有运行构建，要明确说明。

提交前注意：

- 不要提交 `Praat.exe`、zip 包、`.o`、`debug_translations.txt`、`translation_candidates.txt` 等忽略产物。
- 如果修改了生成后的 `sys/praat_translate.cpp`，确认 `tools/generate_translation_map.py` 中的源映射也同步了。
- 如果修改了手册源码，优先审查链接语法和页面标题，不要让中文显示文本变成链接目标。
