/* praat_python.cpp
 *
 * Python scripting subsystem for Praat
 */

#include "praatP.h"
#include "Editor.h"
#include "praat_script.h"
#include "praat_python.h"
#include "Preferences.h"
#include "melder_sysenv.h"
#include "praat_translate.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

#if defined (_WIN32)
	#include <windows.h>
#else
	#include <unistd.h>
	#include <sys/wait.h>
#endif

static char32 thePythonExecutablePath [Preferences_STRING_BUFFER_SIZE];

void praat_python_initPreferences () {
	Preferences_addString (U"Python.executablePath", thePythonExecutablePath, U"python");
}

conststring32 praat_python_getExecutablePath () {
	if (thePythonExecutablePath [0] != U'\0')
		return thePythonExecutablePath;
	return U"python";
}

void praat_python_setExecutablePath (conststring32 path) {
	if (path && path [0] != U'\0')
		str32cpy (thePythonExecutablePath, path);
	else
		str32cpy (thePythonExecutablePath, U"python");
}

autostring32 praat_python_getScriptTemplate () {
	return Melder_dup (
		U"# Praat Python Script\n"
		U"# Directly execute Python & call Praat built-in algorithms (Ctrl+R to run)\n"
		U"import sys\n"
		U"import os\n"
		U"import praat\n\n"
		U"print(\"=== Praat Python Integration ===\")\n"
		U"print(f\"Python Version: {sys.version.split()[0]}\")\n\n"
		U"# 1. 获取 Praat 当前选中的对象\n"
		U"selected = praat.get_selected()\n"
		U"print(f\"当前选中对象数量: {len(selected)}\")\n"
		U"for obj in selected:\n"
		U"    print(f\"  • [ID {obj['id']}] {obj['class']}: {obj['name']}\")\n\n"
		U"# 2. 直接调用 Praat 原生算法与指令\n"
		U"# 例如遍历选中的 Sound，调用 Praat 的 To Pitch (ac) 提取音高，并重命名：\n"
		U"for obj in selected:\n"
		U"    if obj['class'] == 'Sound':\n"
		U"        praat.select(obj['id'])\n"
		U"        praat.call('To Pitch (ac)...', 0.0, 75, 15, 'yes', 0.03, 0.45, 0.01, 0.35, 0.14, 600)\n"
		U"        praat.rename(f\"Pitch_{obj['name']}\")\n"
		U"        print(f\"  ⚡ 已为声音 [{obj['name']}] 调度生成 Pitch 对象\")\n\n"
		U"# 3. 如果需要生成音频或数据导回 Praat，可保存到 praat.get_output_dir()\n"
	);
}

autostring32 praat_python_getTutorialDoc () {
	return Melder_dup (
		U"=================================================================\n"
		U"  Praat Python 脚本使用教程 (Python Scripting Tutorial)\n"
		U"=================================================================\n\n"
		U"1. 运行与调试\n"
		U"   • 运行整个脚本：按下快捷键 【Ctrl + R】 或点击菜单 【运行 -> 运行】\n"
		U"   • 运行选区代码：按下快捷键 【Ctrl + T】 或点击菜单 【运行 -> 运行选区】\n"
		U"   • 控制台输出：所有的 print(...) 输出会自动呈现在 Praat 的 Info 窗口中。\n\n"
		U"2. 桥接模块 praat\n"
		U"   在脚本开头输入 import praat 即可使用内置桥接功能：\n"
		U"   • praat.get_selected()：获取当前在 Praat 对象列表选中的所有对象信息\n"
		U"   • praat.call(command, *args)：直接调用任意 Praat 菜单命令或声学算法\n"
		U"   • praat.select(id) / praat.plus_select(id)：选中或增选指定对象\n"
		U"   • praat.rename(name)：为当前选中对象重命名\n"
		U"   • praat.remove()：移除当前选中对象\n"
		U"   • praat.get_output_dir()：获取输出目录，保存在该目录的文件会在脚本结束后自动载入 Praat\n\n"
		U"3. Python 运行环境设置\n"
		U"   • 点击 【Praat -> Python 设置...】 可以配置 Python 解释器路径。\n"
		U"   • 支持直接指定系统 Python、Anaconda 环境或特定的虚拟环境 (venv)。\n\n"
		U"4. 第三方库支持\n"
		U"   只要在您的 Python 环境中安装了相关库，即可在脚本中直接 import 使用，如：\n"
		U"   • parselmouth（Python 版 Praat 核心算法库）\n"
		U"   • praatio（高效处理 TextGrid 标注层）\n"
		U"   • numpy, scipy, pandas（科学计算与数据统计）\n"
		U"   • matplotlib, seaborn（绘制高精度科研图表）\n"
		U"   • whisper, funasr（AI 自动语音识别转写与对齐）\n"
		U"=================================================================\n"
	);
}

autostring32 praat_python_getApiDoc () {
	return Melder_dup (
		U"=================================================================\n"
		U"  Praat Python 桥接模块 API 参考手册 (API Reference)\n"
		U"=================================================================\n\n"
		U"• praat.get_selected()\n"
		U"  返回当前在 Praat 对象列表中选中的所有对象列表。\n"
		U"  返回值格式：[ {'id': 1, 'name': 'Sound_1', 'class': 'Sound', 'file': 'path/to/file'}, ... ]\n\n"
		U"• praat.call(command_name, *args)\n"
		U"  在 Praat 中执行任意原生命令（算法）。\n"
		U"  示例：\n"
		U"    praat.call(\"To Pitch (ac)...\", 0.0, 75, 15, \"yes\", 0.03, 0.45, 0.01, 0.35, 0.14, 600)\n"
		U"    praat.call(\"To Formant (burg)...\", 0.0, 5, 5500, 0.025, 50)\n"
		U"    praat.call(\"Filter (pass Hann band)...\", 500, 1500, 100)\n\n"
		U"• praat.select(object_id)\n"
		U"  单选指定 ID 的对象。\n"
		U"  示例：praat.select(1)\n\n"
		U"• praat.plus_select(object_id)\n"
		U"  增选指定 ID 的对象（保持之前已选对象）。\n"
		U"  示例：praat.plus_select(2)\n\n"
		U"• praat.rename(new_name)\n"
		U"  为当前选中的对象重命名。\n"
		U"  示例：praat.rename(\"Vowel_A\")\n\n"
		U"• praat.remove()\n"
		U"  从 Praat 对象列表中删除当前选中的对象。\n\n"
		U"• praat.run_praat_script(script_text)\n"
		U"  执行一段原生 Praat 脚本文本。\n\n"
		U"• praat.get_output_dir()\n"
		U"  获取 Praat 自动监听的临时输出目录。\n"
		U"  任何保存在该目录下的音频 (.wav) 或标注 (.TextGrid) 文件都会在脚本运行结束后自动载入 Praat。\n"
		U"=================================================================\n"
	);
}

autostring32 praat_python_getExampleBatch () {
	return Melder_dup (
		U"# 示例：批量为选中的 Sound 对象提取音高与共振峰\n"
		U"import praat\n\n"
		U"print(\"🚀 开始批量分析选中的音频...\")\n"
		U"selected = praat.get_selected()\n"
		U"sound_count = 0\n\n"
		U"for obj in selected:\n"
		U"    if obj['class'] == 'Sound':\n"
		U"        sound_count += 1\n"
		U"        # 1. 选中该音频\n"
		U"        praat.select(obj['id'])\n"
		U"        # 2. 提取音高 Pitch (自相关法 ac)\n"
		U"        praat.call('To Pitch (ac)...', 0.0, 75, 15, 'yes', 0.03, 0.45, 0.01, 0.35, 0.14, 600)\n"
		U"        praat.rename(f\"Pitch_{obj['name']}\")\n\n"
		U"        # 3. 提取共振峰 Formant (Burg 法)\n"
		U"        praat.select(obj['id'])\n"
		U"        praat.call('To Formant (burg)...', 0.0, 5, 5500, 0.025, 50)\n"
		U"        praat.rename(f\"Formant_{obj['name']}\")\n\n"
		U"        print(f\"  • 已为 [{obj['name']}] 生成 Pitch 与 Formant\")\n\n"
		U"print(f\"🎉 处理完成，共处理了 {sound_count} 个音频对象！\")\n"
	);
}

autostring32 praat_python_getExampleTextGrid () {
	return Melder_dup (
		U"# 示例：读取并分析选中的 TextGrid 标注层内容\n"
		U"import os\n"
		U"import praat\n\n"
		U"print(\"📝 正在遍历分析选中的 TextGrid...\")\n"
		U"for obj in praat.get_selected():\n"
		U"    if obj['class'] == 'TextGrid':\n"
		U"        print(f\"\\n--- TextGrid: {obj['name']} ---\")\n"
		U"        if os.path.exists(obj['file']):\n"
		U"            with open(obj['file'], 'r', encoding='utf-8', errors='ignore') as f:\n"
		U"                lines = f.readlines()\n"
		U"            labels = [line.strip().split('=')[1].strip(' \"') for line in lines if 'text =' in line and 'text = \"\"' not in line]\n"
		U"            print(f\"  有效标注总数: {len(labels)} 个\")\n"
		U"            print(f\"  前10个标注内容: {labels[:10]}\")\n"
	);
}

autostring32 praat_python_getExampleSound () {
	return Melder_dup (
		U"# 示例：使用 Python 生成双音（440Hz + 880Hz）并自动载入 Praat\n"
		U"import os\n"
		U"import math\n"
		U"import wave\n"
		U"import struct\n"
		U"import praat\n\n"
		U"out_dir = praat.get_output_dir()\n"
		U"wav_path = os.path.join(out_dir, \"Python_DualTone_440_880.wav\")\n"
		U"sr = 44100\n"
		U"duration = 2.0\n"
		U"num_samples = int(sr * duration)\n\n"
		U"with wave.open(wav_path, 'w') as f:\n"
		U"    f.setnchannels(1)\n"
		U"    f.setsampwidth(2)\n"
		U"    f.setframerate(sr)\n"
		U"    frames = bytearray()\n"
		U"    for i in range(num_samples):\n"
		U"        t = float(i) / sr\n"
		U"        s1 = math.sin(2.0 * math.pi * 440.0 * t)\n"
		U"        s2 = math.sin(2.0 * math.pi * 880.0 * t)\n"
		U"        val = int(32767.0 * 0.4 * (s1 + s2))\n"
		U"        frames.extend(struct.pack('<h', val))\n"
		U"    f.writeframes(frames)\n\n"
		U"print(f\"✅ 已生成音频文件: {wav_path}\")\n"
		U"print(\"🚀 Praat 正在自动将该音频载入到对象列表中！\")\n"
	);
}

static void ensure_helper_module (const std::filesystem::path& tempDir) {
	std::filesystem::path helperPath = tempDir / "praat.py";
	std::ofstream ofs (helperPath, std::ios::out | std::ios::trunc);
	if (ofs.is_open()) {
		ofs << "# Praat Python Bridge Helper Module\n"
		       "import json, os, sys\n\n"
		       "_context = None\n"
		       "_commands_file = None\n\n"
		       "def _get_commands_file():\n"
		       "    global _commands_file\n"
		       "    if _commands_file is None:\n"
		       "        temp = os.environ.get('PRAAT_TEMP_DIR')\n"
		       "        if temp:\n"
		       "            _commands_file = os.path.join(temp, 'praat_commands.praat')\n"
		       "        else:\n"
		       "            _commands_file = 'praat_commands.praat'\n"
		       "    return _commands_file\n\n"
		       "def get_context():\n"
		       "    global _context\n"
		       "    if _context is None:\n"
		       "        ctx_file = os.environ.get('PRAAT_CONTEXT')\n"
		       "        if ctx_file and os.path.exists(ctx_file):\n"
		       "            try:\n"
		       "                with open(ctx_file, 'r', encoding='utf-8') as f:\n"
		       "                    _context = json.load(f)\n"
		       "            except Exception:\n"
		       "                _context = {'selected_objects': []}\n"
		       "        else:\n"
		       "            _context = {'selected_objects': []}\n"
		       "    return _context\n\n"
		       "def get_selected():\n"
		       "    \"\"\"Returns a list of all objects currently selected in Praat.\"\"\"\n"
		       "    return get_context().get('selected_objects', [])\n\n"
		       "def get_output_dir():\n"
		       "    \"\"\"Returns output directory for new files to be loaded into Praat.\"\"\"\n"
		       "    temp = os.environ.get('PRAAT_TEMP_DIR')\n"
		       "    if temp:\n"
		       "        out = os.path.join(temp, 'output')\n"
		       "        os.makedirs(out, exist_ok=True)\n"
		       "        return out\n"
		       "    return '.'\n\n"
		       "def run_praat_script(script_text):\n"
		       "    \"\"\"Appends raw Praat script lines to execute in Praat.\"\"\"\n"
		       "    with open(_get_commands_file(), 'a', encoding='utf-8') as f:\n"
		       "        f.write(script_text.strip() + '\\n')\n\n"
		       "def call(command_name, *args):\n"
		       "    \"\"\"Calls any native Praat command (e.g. To Pitch, To Formant, Filter, Rename, Draw).\"\"\"\n"
		       "    cmd_clean = command_name.rstrip('.')\n"
		       "    if args:\n"
		       "        arg_strs = []\n"
		       "        for arg in args:\n"
		       "            if isinstance(arg, str):\n"
		       "                escaped = arg.replace('\"', '\"\"')\n"
		       "                arg_strs.append(f'\"{escaped}\"')\n"
		       "            elif isinstance(arg, bool):\n"
		       "                arg_strs.append('yes' if arg else 'no')\n"
		       "            else:\n"
		       "                arg_strs.append(str(arg))\n"
		       "        cmd_line = f'{cmd_clean}: {\", \".join(arg_strs)}'\n"
		       "    else:\n"
		       "        cmd_line = f'{cmd_clean}'\n"
		       "    with open(_get_commands_file(), 'a', encoding='utf-8') as f:\n"
		       "        f.write(cmd_line + '\\n')\n\n"
		       "def select(object_id):\n"
		       "    \"\"\"Selects an object by ID in Praat.\"\"\"\n"
		       "    with open(_get_commands_file(), 'a', encoding='utf-8') as f:\n"
		       "        f.write(f'selectObject: {object_id}\\n')\n\n"
		       "def plus_select(object_id):\n"
		       "    \"\"\"Adds an object to current selection in Praat.\"\"\"\n"
		       "    with open(_get_commands_file(), 'a', encoding='utf-8') as f:\n"
		       "        f.write(f'plusObject: {object_id}\\n')\n\n"
		       "def rename(new_name):\n"
		       "    \"\"\"Renames the currently selected object(s).\"\"\"\n"
		       "    call('Rename...', new_name)\n\n"
		       "def remove():\n"
		       "    \"\"\"Removes currently selected object(s).\"\"\"\n"
		       "    with open(_get_commands_file(), 'a', encoding='utf-8') as f:\n"
		       "        f.write('Remove\\n')\n";
		ofs.close();
	}
}

void praat_runPythonScriptFile (conststring32 filePath) {
	if (! filePath || filePath [0] == U'\0')
		Melder_throw (U"No Python script file specified.");

	std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "praat_py_workspace";
	std::filesystem::create_directories (tempDir);
	std::filesystem::path outputDir = tempDir / "output";
	try {
		if (std::filesystem::exists (outputDir))
			std::filesystem::remove_all (outputDir);
		std::filesystem::create_directories (outputDir);
	} catch (...) {}
	ensure_helper_module (tempDir);

	// Clean any previous commands file
	std::filesystem::path commandsPath = tempDir / "praat_commands.praat";
	try {
		if (std::filesystem::exists (commandsPath))
			std::filesystem::remove (commandsPath);
	} catch (...) {}

	// Export selected objects to workspace
	std::filesystem::path contextJsonPath = tempDir / "praat_context.json";
	std::ofstream jsonOfs (contextJsonPath, std::ios::out | std::ios::trunc);
	if (jsonOfs.is_open()) {
		jsonOfs << "{\n  \"selected_objects\": [\n";
		bool first = true;
		if (theCurrentPraatObjects && theCurrentPraatObjects -> totalSelection > 0) {
			for (integer iobj = 1; iobj <= theCurrentPraatObjects -> n; iobj ++) {
				if (theCurrentPraatObjects -> list [iobj]. isSelected) {
					Daata object = theCurrentPraatObjects -> list [iobj]. object;
					if (! object) continue;

					if (! first) jsonOfs << ",\n";
					first = false;

					char32 sanitizedBaseName [128];
					str32cpy (sanitizedBaseName, theCurrentPraatObjects -> list [iobj]. name.get());
					for (char32 *p = sanitizedBaseName; *p != U'\0'; p ++) {
						if (*p == U' ' || *p == U'\\' || *p == U'/' || *p == U':' || *p == U'*' || *p == U'?' || *p == U'"' || *p == U'<' || *p == U'>' || *p == U'|')
							*p = U'_';
					}

					autoMelderString objFileName;
					MelderString_append (& objFileName, iobj, U"_", sanitizedBaseName, U".ooTextFile");
					std::filesystem::path objPath = tempDir / Melder_peek32toW (objFileName.string);

					structMelderFile objMelderFile { };
					Melder_pathToFile (Melder_peekWto32 (objPath.c_str()), & objMelderFile);
					try {
						Data_writeToTextFile (object, & objMelderFile);
					} catch (...) {}

					autostring8 utf8Name = Melder_32to8 (theCurrentPraatObjects -> list [iobj]. name.get());
					autostring8 utf8Class = Melder_32to8 (Thing_className (object));
					std::string objPathStr = objPath.u8string();

					jsonOfs << "    {\n"
					        << "      \"id\": " << iobj << ",\n"
					        << "      \"name\": \"" << (utf8Name ? utf8Name.get() : "") << "\",\n"
					        << "      \"class\": \"" << (utf8Class ? utf8Class.get() : "") << "\",\n"
					        << "      \"file\": \"";
					for (char c : objPathStr) {
						if (c == '\\') jsonOfs << "\\\\";
						else jsonOfs << c;
					}
					jsonOfs << "\"\n    }";
				}
			}
		}
		jsonOfs << "\n  ]\n}\n";
		jsonOfs.close();
	}

	conststring32 pyExec = praat_python_getExecutablePath ();

	#if defined (_WIN32)
		HANDLE hStdoutRead = NULL, hStdoutWrite = NULL;
		HANDLE hStderrRead = NULL, hStderrWrite = NULL;
		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof (SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		CreatePipe (& hStdoutRead, & hStdoutWrite, & saAttr, 0);
		SetHandleInformation (hStdoutRead, HANDLE_FLAG_INHERIT, 0);

		CreatePipe (& hStderrRead, & hStderrWrite, & saAttr, 0);
		SetHandleInformation (hStderrRead, HANDLE_FLAG_INHERIT, 0);

		STARTUPINFOW si;
		ZeroMemory (& si, sizeof (STARTUPINFOW));
		si.cb = sizeof (STARTUPINFOW);
		si.hStdOutput = hStdoutWrite;
		si.hStdError = hStderrWrite;
		si.hStdInput = GetStdHandle (STD_INPUT_HANDLE);
		si.dwFlags |= STARTF_USESTDHANDLES;

		PROCESS_INFORMATION pi;
		ZeroMemory (& pi, sizeof (PROCESS_INFORMATION));

		// Set Environment variables for python
		SetEnvironmentVariableW (L"PRAAT_CONTEXT", contextJsonPath.c_str());
		SetEnvironmentVariableW (L"PRAAT_TEMP_DIR", tempDir.c_str());
		SetEnvironmentVariableW (L"PYTHONIOENCODING", L"utf-8");
		SetEnvironmentVariableW (L"PYTHONUNBUFFERED", L"1");

		std::wstring existingPythonPath;
		wchar_t envBuf [4096];
		if (GetEnvironmentVariableW (L"PYTHONPATH", envBuf, 4096) > 0)
			existingPythonPath = std::wstring (tempDir.c_str()) + L";" + envBuf;
		else
			existingPythonPath = tempDir.c_str();
		SetEnvironmentVariableW (L"PYTHONPATH", existingPythonPath.c_str());

		std::wstring cmdLine = L"\"" + std::wstring (Melder_peek32toW (pyExec)) + L"\" \"" + std::wstring (Melder_peek32toW (filePath)) + L"\"";

		// Determine script working directory
		std::filesystem::path scriptDirPath = std::filesystem::path (Melder_peek32toW (filePath)).parent_path();
		const wchar_t *cwd = scriptDirPath.empty() ? nullptr : scriptDirPath.c_str();

		std::vector<wchar_t> cmdLineMutable (cmdLine.begin(), cmdLine.end());
		cmdLineMutable.push_back (L'\0');

		BOOL success = CreateProcessW (
			nullptr,
			cmdLineMutable.data(),
			nullptr,
			nullptr,
			TRUE,
			CREATE_NO_WINDOW,
			nullptr,
			cwd,
			& si,
			& pi
		);

		CloseHandle (hStdoutWrite);
		CloseHandle (hStderrWrite);

		if (! success) {
			CloseHandle (hStdoutRead);
			CloseHandle (hStderrRead);
			Melder_throw (U"Cannot start Python subprocess: <<", pyExec, U">>.\nPlease check your Python executable path in Praat -> Python settings.");
		}

		// Read stdout and stderr
		std::string stdoutAccum, stderrAccum;
		char buffer [4096];
		DWORD bytesRead;

		while (ReadFile (hStdoutRead, buffer, sizeof (buffer) - 1, & bytesRead, NULL) && bytesRead > 0) {
			buffer [bytesRead] = '\0';
			stdoutAccum.append (buffer, bytesRead);
		}
		while (ReadFile (hStderrRead, buffer, sizeof (buffer) - 1, & bytesRead, NULL) && bytesRead > 0) {
			buffer [bytesRead] = '\0';
			stderrAccum.append (buffer, bytesRead);
		}

		WaitForSingleObject (pi.hProcess, INFINITE);
		DWORD exitCode = 0;
		GetExitCodeProcess (pi.hProcess, & exitCode);

		CloseHandle (pi.hProcess);
		CloseHandle (pi.hThread);
		CloseHandle (hStdoutRead);
		CloseHandle (hStderrRead);

		autostring32 out32 = Melder_8to32_e (stdoutAccum.c_str());
		autostring32 err32 = Melder_8to32_e (stderrAccum.c_str());

		if (out32 && out32 [0] != U'\0') {
			MelderInfo_open ();
			MelderInfo_write (out32.get());
			MelderInfo_close ();
		}

		if (exitCode != 0) {
			if (err32 && err32 [0] != U'\0') {
				MelderInfo_open ();
				if (out32 && out32 [0] != U'\0') {
					MelderInfo_write (out32.get());
					MelderInfo_write (U"\n--- Traceback / Errors ---\n");
				}
				MelderInfo_write (err32.get());
				MelderInfo_close ();
			}
			Melder_throw (U"Python script exited with error (exit code ", (int) exitCode, U"). See Info window for details.");
		}
	#endif

	// Execute commands generated by praat.call / praat.run_praat_script
	if (std::filesystem::exists (commandsPath)) {
		std::ifstream ifs (commandsPath, std::ios::in | std::ios::binary);
		if (ifs.is_open()) {
			std::string content ((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
			ifs.close();
			if (! content.empty()) {
				autostring32 script32 = Melder_8to32_e (content.c_str());
				if (script32 && script32 [0] != U'\0') {
					try {
						praat_executeScriptFromText (script32.get());
					} catch (MelderError) {
						Melder_flushError (U"Error executing Praat commands from Python script.");
					}
				}
			}
		}
		try { std::filesystem::remove (commandsPath); } catch (...) {}
	}

	// Auto-import any files generated in output/ during this run
	try {
		if (std::filesystem::exists (outputDir)) {
			std::vector<std::filesystem::path> filesToImport;
			for (const auto& entry : std::filesystem::directory_iterator (outputDir)) {
				if (entry.is_regular_file()) {
					filesToImport.push_back (entry.path());
				}
			}
			for (const auto& fpath : filesToImport) {
				std::wstring fullPath = fpath.wstring();
				conststring32 path32 = Melder_peekWto32 (fullPath.c_str());
				autoMelderString cmd;
				MelderString_append (& cmd, U"Read from file: ~", path32, U"\n");
				praat_executeScriptFromText (cmd.string);
			}
			// Clean up output dir after loading into memory
			std::filesystem::remove_all (outputDir);
			std::filesystem::create_directories (outputDir);
		}
	} catch (...) {}
}

void praat_runPythonScriptText (conststring32 scriptText, conststring32 /* optionalScriptDirectory */) {
	if (! scriptText || scriptText [0] == U'\0')
		Melder_throw (U"Python script text is empty.");

	std::filesystem::path tempDir = std::filesystem::temp_directory_path() / "praat_py_workspace";
	std::filesystem::create_directories (tempDir);
	std::filesystem::path scriptPath = tempDir / "temp_exec_script.py";

	std::ofstream ofs (scriptPath, std::ios::out | std::ios::trunc | std::ios::binary);
	if (! ofs.is_open())
		Melder_throw (U"Cannot write temporary Python script.");

	autostring8 utf8Text = Melder_32to8 (scriptText);
	if (utf8Text) {
		ofs.write (utf8Text.get(), strlen (utf8Text.get()));
	}
	ofs.close();

	conststring32 scriptPath32 = Melder_peekWto32 (scriptPath.c_str());
	praat_runPythonScriptFile (scriptPath32);
}
