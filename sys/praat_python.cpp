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
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <iomanip>

#if defined (_WIN32)
	#include <windows.h>
	#include <process.h>
#else
	#include <unistd.h>
	#include <sys/wait.h>
	#include <sys/types.h>
#endif

static std::filesystem::path get_process_workspace () {
#if defined (_WIN32)
	unsigned long pid = GetCurrentProcessId ();
#else
	pid_t pid = getpid ();
#endif
	return std::filesystem::temp_directory_path () / ("praat_py_workspace_" + std::to_string (pid));
}

static std::string escape_json_string (const std::string &str) {
	std::ostringstream oss;
	for (unsigned char c : str) {
		switch (c) {
			case '"':  oss << "\\\""; break;
			case '\\': oss << "\\\\"; break;
			case '\b': oss << "\\b"; break;
			case '\f': oss << "\\f"; break;
			case '\n': oss << "\\n"; break;
			case '\r': oss << "\\r"; break;
			case '\t': oss << "\\t"; break;
			default:
				if (c < 0x20) {
					oss << "\\u" << std::hex << std::setw (4) << std::setfill ('0') << (int) c;
				} else {
					oss << (char) c;
				}
				break;
		}
	}
	return oss.str ();
}

static std::string path_to_utf8 (const std::filesystem::path &p) {
#if defined (_WIN32)
	auto u8 = p.u8string ();
	return std::string (reinterpret_cast<const char *> (u8.data ()), u8.size ());
#else
	return p.string ();
#endif
}

static std::filesystem::path utf8_to_path (const std::string &u8) {
#if defined (_WIN32)
	return std::filesystem::path (reinterpret_cast<const char8_t *> (u8.data ()), reinterpret_cast<const char8_t *> (u8.data () + u8.size ()));
#else
	return std::filesystem::path (u8);
#endif
}

#if defined (_WIN32)
static std::wstring utf8_to_wstring (const std::string &u8) {
	if (u8.empty ()) return std::wstring ();
	int len = MultiByteToWideChar (CP_UTF8, 0, u8.c_str (), (int) u8.size (), nullptr, 0);
	if (len <= 0) return std::wstring ();
	std::wstring w (len, 0);
	MultiByteToWideChar (CP_UTF8, 0, u8.c_str (), (int) u8.size (), & w [0], len);
	return w;
}
#endif

static char32 thePythonExecutablePath [Preferences_STRING_BUFFER_SIZE];

void praat_python_initPreferences () {
	#if defined (_WIN32)
		Preferences_addString (U"Python.executablePath", thePythonExecutablePath, U"python");
	#else
		Preferences_addString (U"Python.executablePath", thePythonExecutablePath, U"python3");
	#endif
}

conststring32 praat_python_getExecutablePath () {
	if (thePythonExecutablePath [0] != U'\0')
		return thePythonExecutablePath;
	#if defined (_WIN32)
		return U"python";
	#else
		return U"python3";
	#endif
}

void praat_python_setExecutablePath (conststring32 path) {
	if (path && path [0] != U'\0')
		str32cpy (thePythonExecutablePath, path);
	else
		#if defined (_WIN32)
			str32cpy (thePythonExecutablePath, U"python");
		#else
			str32cpy (thePythonExecutablePath, U"python3");
		#endif
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
		U"   • 控制台输出：所有的 print(...) 与异常报错会自动呈现在 Praat 的 Info 窗口中。\n\n"
		U"2. 桥接模块 praat\n"
		U"   在脚本开头输入 import praat 即可使用内置桥接功能：\n"
		U"   • praat.get_selected()：获取 Praat 当前选中的对象信息。选中的 Sound 对象已自动导出为标准 .wav 音频（可直接用 scipy/librosa/parselmouth 读取）。\n"
		U"   • praat.call(command, *args)：调度调用 Praat 菜单命令或声学算法（在 Python 结束后由 Praat 顺序执行）。\n"
		U"   • praat.select(id) / praat.plus_select(id)：调度选中指定对象\n"
		U"   • praat.rename(name)：调度重命名当前选中对象\n"
		U"   • praat.remove()：调度删除当前选中对象\n"
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
		U"  返回值格式：[ {'id': 1, 'name': 'Sound_1', 'class': 'Sound', 'file': 'path/to/file.wav'}, ... ]\n"
		U"  提示：Sound 对象文件为标准 16-bit PCM .wav，TextGrid 为标准 .TextGrid 格式。\n\n"
		U"• praat.call(command_name, *args)\n"
		U"  调度执行 Praat 原生菜单命令或算法（在 Python 执行完毕后统一批处理执行）。\n"
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

void praat_runPythonScriptFile (conststring32 filePath, conststring32 optionalWorkingDir) {
	if (! filePath || filePath [0] == U'\0')
		Melder_throw (U"No Python script file specified.");

	std::filesystem::path tempDir = get_process_workspace ();
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

	// Record currently selected objects
	std::vector<integer> selectedIds;
	if (theCurrentPraatObjects && theCurrentPraatObjects -> totalSelection > 0) {
		for (integer iobj = 1; iobj <= theCurrentPraatObjects -> n; iobj ++) {
			if (theCurrentPraatObjects -> list [iobj]. isSelected)
				selectedIds.push_back (iobj);
		}
	}

	// Export selected objects to workspace
	std::filesystem::path contextJsonPath = tempDir / "praat_context.json";
	std::ofstream jsonOfs (contextJsonPath, std::ios::out | std::ios::trunc);
	if (jsonOfs.is_open()) {
		jsonOfs << "{\n  \"selected_objects\": [\n";
		bool first = true;
		for (integer iobj : selectedIds) {
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

			conststring32 className = Thing_className (object);
			bool isSound = str32equ (className, U"Sound");
			bool isTextGrid = str32equ (className, U"TextGrid");

			const char32 *ext = isSound ? U".wav" : (isTextGrid ? U".TextGrid" : U".ooTextFile");

			autoMelderString objFileName;
			MelderString_append (& objFileName, iobj, U"_", sanitizedBaseName, ext);
			autostring8 objFileName8 = Melder_32to8 (objFileName.string);
			std::filesystem::path objPath = tempDir / (objFileName8 ? objFileName8.get() : "obj");
			std::string objPathStr = path_to_utf8 (objPath);

			if (isSound) {
				// Export Sound as standard PCM WAV file
				autoMelderString saveCmd;
				autostring32 objPath32 = Melder_8to32_e (objPathStr.c_str());
				autoMelderString escapedObjPath;
				if (objPath32) {
					for (const char32 *p = objPath32.get(); *p != U'\0'; p++) {
						if (*p == U'"')
							MelderString_append (& escapedObjPath, U"\"\"");
						else
							MelderString_appendCharacter (& escapedObjPath, *p);
					}
				}
				MelderString_append (& saveCmd, U"selectObject: ", iobj, U"\nSave as WAV file: \"", escapedObjPath.string, U"\"\n");
				try {
					praat_executeScriptFromText (saveCmd.string);
				} catch (...) {
					structMelderFile objMelderFile { };
					Melder_pathToFile (objPath32.get(), & objMelderFile);
					try { Data_writeToTextFile (object, & objMelderFile); } catch (...) {}
				}
			} else {
				structMelderFile objMelderFile { };
				autostring32 objPath32 = Melder_8to32_e (objPathStr.c_str());
				Melder_pathToFile (objPath32.get(), & objMelderFile);
				try {
					Data_writeToTextFile (object, & objMelderFile);
				} catch (...) {}
			}

			autostring8 utf8Name = Melder_32to8 (theCurrentPraatObjects -> list [iobj]. name.get());
			autostring8 utf8Class = Melder_32to8 (className);

			jsonOfs << "    {\n"
			        << "      \"id\": " << iobj << ",\n"
			        << "      \"name\": \"" << escape_json_string (utf8Name ? utf8Name.get() : "") << "\",\n"
			        << "      \"class\": \"" << escape_json_string (utf8Class ? utf8Class.get() : "") << "\",\n"
			        << "      \"file\": \"" << escape_json_string (objPathStr) << "\"\n"
			        << "    }";
		}
		jsonOfs << "\n  ]\n}\n";
		jsonOfs.close();
	}

	// Restore original selection
	if (! selectedIds.empty ()) {
		autoMelderString restoreSelectionCmd;
		MelderString_append (& restoreSelectionCmd, U"selectObject: ", selectedIds [0], U"\n");
		for (size_t i = 1; i < selectedIds.size (); i ++) {
			MelderString_append (& restoreSelectionCmd, U"plusObject: ", selectedIds [i], U"\n");
		}
		try {
			praat_executeScriptFromText (restoreSelectionCmd.string);
		} catch (...) {}
	}

	conststring32 pyExec = praat_python_getExecutablePath ();

	#if defined (_WIN32)
		HANDLE hPipeRead = NULL, hPipeWrite = NULL;
		SECURITY_ATTRIBUTES saAttr;
		saAttr.nLength = sizeof (SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = TRUE;
		saAttr.lpSecurityDescriptor = NULL;

		CreatePipe (& hPipeRead, & hPipeWrite, & saAttr, 0);
		SetHandleInformation (hPipeRead, HANDLE_FLAG_INHERIT, 0);

		STARTUPINFOW si;
		ZeroMemory (& si, sizeof (STARTUPINFOW));
		si.cb = sizeof (STARTUPINFOW);
		si.hStdOutput = hPipeWrite;
		si.hStdError = hPipeWrite; // Merge stderr to single pipe to prevent deadlocks and maintain ordering
		si.hStdInput = GetStdHandle (STD_INPUT_HANDLE);
		si.dwFlags |= STARTF_USESTDHANDLES;

		PROCESS_INFORMATION pi;
		ZeroMemory (& pi, sizeof (PROCESS_INFORMATION));

		// Backup parent process environment variables
		wchar_t oldPythonPath [4096] = {0};
		DWORD oldPythonPathLen = GetEnvironmentVariableW (L"PYTHONPATH", oldPythonPath, 4096);
		wchar_t oldContext [4096] = {0};
		DWORD oldContextLen = GetEnvironmentVariableW (L"PRAAT_CONTEXT", oldContext, 4096);
		wchar_t oldTempDir [4096] = {0};
		DWORD oldTempDirLen = GetEnvironmentVariableW (L"PRAAT_TEMP_DIR", oldTempDir, 4096);
		wchar_t oldIOEnc [4096] = {0};
		DWORD oldIOEncLen = GetEnvironmentVariableW (L"PYTHONIOENCODING", oldIOEnc, 4096);
		wchar_t oldUnbuf [4096] = {0};
		DWORD oldUnbufLen = GetEnvironmentVariableW (L"PYTHONUNBUFFERED", oldUnbuf, 4096);

		// Set Environment variables for python
		SetEnvironmentVariableW (L"PRAAT_CONTEXT", contextJsonPath.wstring().c_str());
		SetEnvironmentVariableW (L"PRAAT_TEMP_DIR", tempDir.wstring().c_str());
		SetEnvironmentVariableW (L"PYTHONIOENCODING", L"utf-8");
		SetEnvironmentVariableW (L"PYTHONUNBUFFERED", L"1");

		std::wstring existingPythonPath;
		if (oldPythonPathLen > 0)
			existingPythonPath = tempDir.wstring() + L";" + oldPythonPath;
		else
			existingPythonPath = tempDir.wstring();
		SetEnvironmentVariableW (L"PYTHONPATH", existingPythonPath.c_str());

		autostring8 pyExec8 = Melder_32to8 (pyExec);
		autostring8 filePath8 = Melder_32to8 (filePath);
		std::string cmdLineA = "\"" + std::string (pyExec8 ? pyExec8.get() : "python") + "\" \"" + std::string (filePath8 ? filePath8.get() : "") + "\"";
		std::wstring cmdLineW = utf8_to_wstring (cmdLineA);

		// Determine script working directory
		std::wstring cwdW;
		if (optionalWorkingDir && optionalWorkingDir [0] != U'\0') {
			autostring8 optCwd8 = Melder_32to8 (optionalWorkingDir);
			if (optCwd8 && optCwd8.get() [0] != '\0') {
				std::filesystem::path p = utf8_to_path (optCwd8.get());
				if (std::filesystem::is_directory (p))
					cwdW = p.wstring();
				else if (p.has_parent_path())
					cwdW = p.parent_path().wstring();
			}
		}
		if (cwdW.empty()) {
			std::filesystem::path scriptDirPath = utf8_to_path (filePath8 ? filePath8.get() : "").parent_path();
			cwdW = scriptDirPath.wstring();
		}
		const wchar_t *cwd = cwdW.empty() ? nullptr : cwdW.c_str();

		std::vector<wchar_t> cmdLineMutable (cmdLineW.begin(), cmdLineW.end());
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

		// Close parent's write handle so ReadFile gets EOF when child exits
		CloseHandle (hPipeWrite);

		// Restore parent process environment variables
		if (oldPythonPathLen > 0) SetEnvironmentVariableW (L"PYTHONPATH", oldPythonPath);
		else SetEnvironmentVariableW (L"PYTHONPATH", NULL);

		if (oldContextLen > 0) SetEnvironmentVariableW (L"PRAAT_CONTEXT", oldContext);
		else SetEnvironmentVariableW (L"PRAAT_CONTEXT", NULL);

		if (oldTempDirLen > 0) SetEnvironmentVariableW (L"PRAAT_TEMP_DIR", oldTempDir);
		else SetEnvironmentVariableW (L"PRAAT_TEMP_DIR", NULL);

		if (oldIOEncLen > 0) SetEnvironmentVariableW (L"PYTHONIOENCODING", oldIOEnc);
		else SetEnvironmentVariableW (L"PYTHONIOENCODING", NULL);

		if (oldUnbufLen > 0) SetEnvironmentVariableW (L"PYTHONUNBUFFERED", oldUnbuf);
		else SetEnvironmentVariableW (L"PYTHONUNBUFFERED", NULL);

		if (! success) {
			CloseHandle (hPipeRead);
			Melder_throw (U"Cannot start Python subprocess: <<", pyExec, U">>.\nPlease check your Python executable path in Praat -> Python settings.");
		}

		// Read output continuously
		std::string outputAccum;
		char buffer [4096];
		DWORD bytesRead = 0;

		while (ReadFile (hPipeRead, buffer, sizeof (buffer) - 1, & bytesRead, NULL) && bytesRead > 0) {
			buffer [bytesRead] = '\0';
			outputAccum.append (buffer, bytesRead);
		}

		WaitForSingleObject (pi.hProcess, INFINITE);
		DWORD exitCode = 0;
		GetExitCodeProcess (pi.hProcess, & exitCode);

		CloseHandle (pi.hProcess);
		CloseHandle (pi.hThread);
		CloseHandle (hPipeRead);

		autostring32 out32 = Melder_8to32_e (outputAccum.c_str());

		if (out32 && out32 [0] != U'\0') {
			MelderInfo_open ();
			MelderInfo_write (out32.get());
			MelderInfo_close ();
		}

		if (exitCode != 0) {
			Melder_throw (U"Python script exited with error (exit code ", (int) exitCode, U"). See Info window for details.");
		}
	#else
		// POSIX implementation (macOS and Linux)
		const char *oldPythonPath = getenv ("PYTHONPATH");
		std::string oldPythonPathStr = oldPythonPath ? oldPythonPath : "";

		setenv ("PRAAT_CONTEXT", contextJsonPath.string().c_str(), 1);
		setenv ("PRAAT_TEMP_DIR", tempDir.string().c_str(), 1);
		setenv ("PYTHONIOENCODING", "utf-8", 1);
		setenv ("PYTHONUNBUFFERED", "1", 1);

		std::string newPythonPath = tempDir.string();
		if (! oldPythonPathStr.empty ())
			newPythonPath += ":" + oldPythonPathStr;
		setenv ("PYTHONPATH", newPythonPath.c_str(), 1);

		autostring8 pyExec8 = Melder_32to8 (pyExec);
		autostring8 filePath8 = Melder_32to8 (filePath);

		std::string safePyExec = pyExec8 ? pyExec8.get() : "python3";
		std::string safeFilePath = filePath8 ? filePath8.get() : "";

		std::string escapedPyExec;
		for (char c : safePyExec) {
			if (c == '\'') escapedPyExec += "'\\''";
			else escapedPyExec += c;
		}
		std::string escapedFilePath;
		for (char c : safeFilePath) {
			if (c == '\'') escapedFilePath += "'\\''";
			else escapedFilePath += c;
		}

		std::string cmd = "'" + escapedPyExec + "' '" + escapedFilePath + "' 2>&1";

		std::string outputAccum;
		FILE *pipe = popen (cmd.c_str(), "r");

		// Restore environment variables
		if (! oldPythonPathStr.empty ())
			setenv ("PYTHONPATH", oldPythonPathStr.c_str(), 1);
		else
			unsetenv ("PYTHONPATH");
		unsetenv ("PRAAT_CONTEXT");
		unsetenv ("PRAAT_TEMP_DIR");
		unsetenv ("PYTHONIOENCODING");
		unsetenv ("PYTHONUNBUFFERED");

		if (! pipe) {
			Melder_throw (U"Cannot execute Python command: <<", pyExec, U">>.");
		}
		char buffer [4096];
		while (fgets (buffer, sizeof (buffer), pipe) != nullptr) {
			outputAccum.append (buffer);
		}
		int status = pclose (pipe);
		int exitCode = WIFEXITED (status) ? WEXITSTATUS (status) : -1;

		autostring32 out32 = Melder_8to32_e (outputAccum.c_str());
		if (out32 && out32 [0] != U'\0') {
			MelderInfo_open ();
			MelderInfo_write (out32.get());
			MelderInfo_close ();
		}

		if (exitCode != 0) {
			Melder_throw (U"Python script exited with error (exit code ", exitCode, U"). See Info window for details.");
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

	// Auto-import any files generated in output/ during this run (sorted & protected)
	try {
		if (std::filesystem::exists (outputDir)) {
			std::vector<std::filesystem::path> filesToImport;
			for (const auto& entry : std::filesystem::directory_iterator (outputDir)) {
				if (entry.is_regular_file()) {
					std::string fname = entry.path().filename().string();
					if (! fname.empty() && fname[0] != '.') {
						filesToImport.push_back (entry.path());
					}
				}
			}
			std::sort (filesToImport.begin(), filesToImport.end());

			for (const auto& fpath : filesToImport) {
				std::string fullPath = path_to_utf8 (fpath);
				autostring32 path32 = Melder_8to32_e (fullPath.c_str());
				if (! path32) continue;

				autoMelderString escapedPath;
				for (const char32 *p = path32.get(); *p != U'\0'; p++) {
					if (*p == U'"')
						MelderString_append (& escapedPath, U"\"\"");
					else
						MelderString_appendCharacter (& escapedPath, *p);
				}

				autoMelderString cmd;
				MelderString_append (& cmd, U"Read from file: \"", escapedPath.string, U"\"\n");
				try {
					praat_executeScriptFromText (cmd.string);
				} catch (MelderError) {
					Melder_clearError ();
				}
			}
			// Clean up output dir after loading into memory
			std::filesystem::remove_all (outputDir);
			std::filesystem::create_directories (outputDir);
		}
	} catch (...) {}
}

void praat_runPythonScriptText (conststring32 scriptText, conststring32 optionalScriptDirectory) {
	if (! scriptText || scriptText [0] == U'\0')
		Melder_throw (U"Python script text is empty.");

	std::filesystem::path tempDir = get_process_workspace ();
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

	std::string scriptPathStr = path_to_utf8 (scriptPath);
	autostring32 scriptPath32 = Melder_8to32_e (scriptPathStr.c_str());
	praat_runPythonScriptFile (scriptPath32.get(), optionalScriptDirectory);
}
