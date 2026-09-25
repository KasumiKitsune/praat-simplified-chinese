/* praat_SpeechRecognizer.cpp
*
 * Copyright (C) 2025 Anastasia Shchupak
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

#include "praat_SpeechRecognizer.h"
#include "SpeechRecognizer.h"
#include "praatM.h"
#include "Gui.h"
#if defined (_WIN32)
	#include <windows.h>
	#include <shellapi.h>
#endif

static bool is_foreign_source_accessible () {
#if defined (_WIN32)
	HMODULE hWinInet = LoadLibraryW (L"wininet.dll");
	if (! hWinInet)
		return false;

	typedef void* HINTERNET_T;
	typedef HINTERNET_T (WINAPI *pfnInternetOpenW)(LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD);
	typedef BOOL (WINAPI *pfnInternetSetOptionW)(HINTERNET_T, DWORD, LPVOID, DWORD);
	typedef HINTERNET_T (WINAPI *pfnInternetOpenUrlW)(HINTERNET_T, LPCWSTR, LPCWSTR, DWORD, DWORD, DWORD_PTR);
	typedef BOOL (WINAPI *pfnInternetCloseHandle)(HINTERNET_T);

	auto pInternetOpenW = (pfnInternetOpenW) GetProcAddress (hWinInet, "InternetOpenW");
	auto pInternetSetOptionW = (pfnInternetSetOptionW) GetProcAddress (hWinInet, "InternetSetOptionW");
	auto pInternetOpenUrlW = (pfnInternetOpenUrlW) GetProcAddress (hWinInet, "InternetOpenUrlW");
	auto pInternetCloseHandle = (pfnInternetCloseHandle) GetProcAddress (hWinInet, "InternetCloseHandle");

	bool accessible = false;
	if (pInternetOpenW && pInternetOpenUrlW && pInternetCloseHandle) {
		HINTERNET_T hSession = pInternetOpenW (L"PraatNetworkProbe", 0 /* INTERNET_OPEN_TYPE_PRECONFIG */, nullptr, nullptr, 0);
		if (hSession) {
			if (pInternetSetOptionW) {
				DWORD timeout = 2500; // 2.5s
				pInternetSetOptionW (hSession, 2 /* INTERNET_OPTION_CONNECT_TIMEOUT */, & timeout, sizeof (timeout));
				pInternetSetOptionW (hSession, 5 /* INTERNET_OPTION_RECEIVE_TIMEOUT */, & timeout, sizeof (timeout));
			}
			HINTERNET_T hUrl = pInternetOpenUrlW (hSession, L"https://huggingface.co", nullptr, 0,
				0x80000000 | 0x00800000 | 0x04000000, 0);
			if (hUrl) {
				accessible = true;
				pInternetCloseHandle (hUrl);
			}
			pInternetCloseHandle (hSession);
		}
	}
	FreeLibrary (hWinInet);
	return accessible;
#else
	return true;
#endif
}

static GuiDialog theWhisperGuideDialog = nullptr;

static void gui_dialog_cb_whisperGoAway (Thing /* boss */) {
	if (theWhisperGuideDialog)
		GuiThing_hide (theWhisperGuideDialog);
}

static void gui_button_cb_whisperOpenFolder (Thing /* boss */, GuiButtonEvent /* event */) {
	conststring32 folder = thePrimaryWhisperModelsFolder ();
	#if defined (_WIN32)
	ShellExecuteW (nullptr, L"open", Melder_peek32toW (folder), nullptr, nullptr, SW_SHOWNORMAL);
	#endif
}

static void gui_button_cb_whisperDownloadBase (Thing /* boss */, GuiButtonEvent /* event */) {
	conststring32 folder = thePrimaryWhisperModelsFolder ();
	#if defined (_WIN32)
	ShellExecuteW (nullptr, L"open", Melder_peek32toW (folder), nullptr, nullptr, SW_SHOWNORMAL);
	#endif

	bool foreignAccessible = is_foreign_source_accessible ();
	const wchar_t *url = foreignAccessible
		? L"https://huggingface.co/ggerganov/whisper.cpp/resolve/main/ggml-base.bin"
		: L"https://hf-mirror.com/ggerganov/whisper.cpp/resolve/main/ggml-base.bin";

	#if defined (_WIN32)
	ShellExecuteW (nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);
	#endif

	Melder_information (foreignAccessible ?
		praat_translate (U"已检测到国际网络连接正常（官方源），正在使用默认浏览器下载推荐模型 ggml-base.bin (约 142 MB)。\n\n已为您自动打开目标模型文件夹，待浏览器下载完成后，请将 ggml-base.bin 放入该文件夹即可。") :
		praat_translate (U"未检测到国际网络连接，已自动切换为国内镜像源 (HF-Mirror)，正在使用默认浏览器下载推荐模型 ggml-base.bin (约 142 MB)。\n\n已为您自动打开目标模型文件夹，待浏览器下载完成后，请将 ggml-base.bin 放入该文件夹即可。")
	);
}

static void gui_button_cb_whisperBrowseAll (Thing /* boss */, GuiButtonEvent /* event */) {
	bool foreignAccessible = is_foreign_source_accessible ();
	const wchar_t *url = foreignAccessible
		? L"https://huggingface.co/ggerganov/whisper.cpp/tree/main"
		: L"https://hf-mirror.com/ggerganov/whisper.cpp/tree/main";

	#if defined (_WIN32)
	ShellExecuteW (nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);
	#endif
}

static void gui_button_cb_whisperClose (Thing /* boss */, GuiButtonEvent /* event */) {
	if (theWhisperGuideDialog)
		GuiThing_hide (theWhisperGuideDialog);
}

void SpeechRecognizer_showModelGuideDialog () {
	if (! theWhisperGuideDialog) {
		const int dialogWidth = 630;
		const int dialogHeight = 225;
		theWhisperGuideDialog = GuiDialog_create (theCurrentPraatApplication -> topShell,
			100, 70, dialogWidth, dialogHeight,
			praat_translate (U"Whisper 模型下载与配置向导"),
			gui_dialog_cb_whisperGoAway, nullptr,
			GuiDialog_Modality::MODAL);

		int y = Gui_TOP_DIALOG_SPACING;
		GuiLabel_createShown (theWhisperGuideDialog,
			Gui_LEFT_DIALOG_SPACING, dialogWidth - Gui_RIGHT_DIALOG_SPACING,
			y, y + Gui_LABEL_HEIGHT,
			praat_translate (U"未检测到 Whisper 语音识别模型 (.bin)"), GuiLabel_BOLD);
		y += Gui_LABEL_HEIGHT + 10;

		GuiLabel_createShown (theWhisperGuideDialog,
			Gui_LEFT_DIALOG_SPACING, dialogWidth - Gui_RIGHT_DIALOG_SPACING,
			y, y + Gui_LABEL_HEIGHT,
			praat_translate (U"Praat 已内置 Whisper.cpp 语音识别引擎，但需要至少一个模型权重文件以执行转写。"), 0);
		y += Gui_LABEL_HEIGHT + 6;

		GuiLabel_createShown (theWhisperGuideDialog,
			Gui_LEFT_DIALOG_SPACING, dialogWidth - Gui_RIGHT_DIALOG_SPACING,
			y, y + Gui_LABEL_HEIGHT,
			praat_translate (U"推荐模型：ggml-base.bin（约 142 MB，速度与准确率平衡，支持中文等多语言识别）。"), 0);
		y += Gui_LABEL_HEIGHT + 6;

		GuiLabel_createShown (theWhisperGuideDialog,
			Gui_LEFT_DIALOG_SPACING, dialogWidth - Gui_RIGHT_DIALOG_SPACING,
			y, y + Gui_LABEL_HEIGHT,
			praat_translate (U"支持便携目录：可直接放置于 Praat 程序同级的 models\\whispercpp\\ 或用户偏好目录。"), 0);

		const int buttonY = dialogHeight - Gui_BOTTOM_DIALOG_SPACING - Gui_PUSHBUTTON_HEIGHT;

		// Button 1: Open Folder
		const int w1 = 135;
		GuiButton_createShown (theWhisperGuideDialog,
			Gui_LEFT_DIALOG_SPACING, Gui_LEFT_DIALOG_SPACING + w1,
			buttonY, buttonY + Gui_PUSHBUTTON_HEIGHT,
			praat_translate (U"打开模型文件夹"), gui_button_cb_whisperOpenFolder, nullptr, 0);

		// Button 2: Download base model
		int x2 = Gui_LEFT_DIALOG_SPACING + w1 + 10;
		const int w2 = 175;
		GuiButton_createShown (theWhisperGuideDialog,
			x2, x2 + w2,
			buttonY, buttonY + Gui_PUSHBUTTON_HEIGHT,
			praat_translate (U"下载 base 模型"), gui_button_cb_whisperDownloadBase, nullptr, GuiButton_DEFAULT);

		// Button 3: Browse all models
		int x3 = x2 + w2 + 10;
		const int w3 = 145;
		GuiButton_createShown (theWhisperGuideDialog,
			x3, x3 + w3,
			buttonY, buttonY + Gui_PUSHBUTTON_HEIGHT,
			praat_translate (U"浏览所有模型..."), gui_button_cb_whisperBrowseAll, nullptr, 0);

		// Button 4: Close
		const int w4 = 85;
		int x4 = dialogWidth - Gui_RIGHT_DIALOG_SPACING - w4;
		GuiButton_createShown (theWhisperGuideDialog,
			x4, x4 + w4,
			buttonY, buttonY + Gui_PUSHBUTTON_HEIGHT,
			praat_translate (U"关闭"), gui_button_cb_whisperClose, nullptr, GuiButton_CANCEL);
	}

	GuiThing_show (theWhisperGuideDialog);
}

DIRECT (DOWNLOAD__SpeechRecognizer_downloadGuide) {
	SpeechRecognizer_showModelGuideDialog ();
END_NO_NEW_DATA
}

DIRECT (HELP__SpeechRecognizer_help) {
	HELP (U"SpeechRecognizer")
}

FORM (CREATE_ONE__SpeechRecognizer_create, U"Create SpeechRecognizer", U"Create SpeechRecognizer...") {
	LISTNUMSTR (modelIndex, modelName, U"Whisper model", constSTRVEC(), 1)
	LISTNUMSTR (languageIndex, languageName, U"Language", constSTRVEC(), 1)
OK
	static autoSTRVEC modelNames;
	modelNames = copy_STRVEC (theCurrentSpeechRecognizerModelNames());   // cannot be called twice in the same scope

	if (modelNames.size == 0) {
		SpeechRecognizer_showModelGuideDialog ();
		Melder_throw (praat_translate (
			U"未检测到 Whisper 语音识别模型。\n"
			U"已为您打开模型下载与配置向导，您也可以直接将 ggml-*.bin 模型放置于软件同级目录的 models/whispercpp 文件夹中。"
		));
	}

	SET_LIST (modelIndex, modelName, modelNames.get(), NUMfindFirst (modelNames.get(), TranscriptionDefaults::modelName))
	SET_LIST (languageIndex, languageName, theSpeechRecognizerLanguageNames(),
			NUMfindFirst (theSpeechRecognizerLanguageNames(), TranscriptionDefaults::languageName))
DO
	CREATE_ONE
		autoSpeechRecognizer result = SpeechRecognizer_create (modelName, languageName);
		Thing_setName (result.get(), Melder_cat (modelName, U"_", languageName));
	CREATE_ONE_END (U"")
}

DIRECT (QUERY_ONE_FOR_STRING__SpeechRecognizer_getModelName) {
	QUERY_ONE_FOR_STRING (SpeechRecognizer)
		conststring32 result = my d_modelName.get();
	QUERY_ONE_FOR_STRING_END
}

DIRECT (QUERY_ONE_FOR_STRING__SpeechRecognizer_getLanguageName) {
	QUERY_ONE_FOR_STRING (SpeechRecognizer)
		conststring32 result = my d_languageName.get();
	QUERY_ONE_FOR_STRING_END
}

DIRECT (QUERY_ONE_AND_ONE_FOR_STRING__SpeechRecognizer_Sound_recognize) {
	QUERY_ONE_AND_ONE_FOR_STRING (SpeechRecognizer, Sound)
		bool useVad = true;
		/*
			Use default Silero VAD parameters.
			FIXME make them configurable, like in other places
		*/
		double speechProbabilityThreshold = 0.5;   // probability threshold to decide that sound is speech
		double minNonSpeechDuration = 0.1;   // min duration of a non-speech segment
		double minSpeechDuration = 0.25;   // min duration of a speech segment
		double speechPad = 0.03;   // padding added before and after each speech segment
		WhisperTranscription whisperTranscription = SpeechRecognizer_recognize (me, you, useVad,
				speechProbabilityThreshold, minNonSpeechDuration, minSpeechDuration, speechPad);
		conststring32 result = whisperTranscription.fullTranscription.text.get();
	QUERY_ONE_AND_ONE_FOR_STRING_END
}

FORM (SETTINGS__SpeechRecognizerSettings, U"AI settings", nullptr) {
	COMMENT (U"These settings determine how fast transcription and diarization")
	COMMENT (U"procedures are performed on your computer.")
	HEADING (U"Transcription...")
	INTEGER (maxNumberOfThreadsForTranscription, U"Max. number of threads", U"0 (= auto)")
	CAPTION (Melder_cat (U"(“auto” is ", TranscriptionDefaults::n_threads, U" on this computer)"))
	HEADING (U"Diarization...")
	INTEGER (maxNumberOfThreadsForDiarization, U"Max. number of threads", U"0 (= auto)")
	CAPTION (Melder_cat (U"(“auto” is ", DiarizationDefaults::n_threads, U" on this computer)"))
OK
	SET_INTEGER (maxNumberOfThreadsForTranscription, SpeechRecognizer_getMaxNumberOfThreadsForTranscription ())
	SET_INTEGER (maxNumberOfThreadsForDiarization,   SpeechRecognizer_getMaxNumberOfThreadsForDiarization ())
DO
	PREFS
		SpeechRecognizer_setMaxNumberOfThreadsForTranscription (maxNumberOfThreadsForTranscription);
		SpeechRecognizer_setMaxNumberOfThreadsForDiarization   (maxNumberOfThreadsForDiarization);
	PREFS_END
}

void praat_SpeechRecognizer_init () {
	Thing_recognizeClassesByName (classSpeechRecognizer);

	SpeechRecognizer_preferences ();

	praat_addMenuCommand (U"Objects", U"New", U"Speech-to-text recognition", nullptr, 0, nullptr);
		praat_addMenuCommand (U"Objects", U"New", U"SpeechRecognizer help", nullptr, 1, HELP__SpeechRecognizer_help);
		praat_addMenuCommand (U"Objects", U"New", U"-- new SpeechRecognizer --", nullptr, 1, nullptr);
		praat_addMenuCommand (U"Objects", U"New", U"Create SpeechRecognizer...", nullptr, 1, CREATE_ONE__SpeechRecognizer_create);
		praat_addMenuCommand (U"Objects", U"New", U"Download Whisper models...", nullptr, 1, DOWNLOAD__SpeechRecognizer_downloadGuide);
	praat_addMenuCommand (U"Objects", U"Settings", U"AI settings...", nullptr, GuiMenu_HIDDEN, SETTINGS__SpeechRecognizerSettings);

	praat_addAction1 (classSpeechRecognizer, 0, U"SpeechRecognizer help", nullptr, 0,
			HELP__SpeechRecognizer_help);
	praat_addAction1 (classSpeechRecognizer, 0, U"Query -", nullptr, 0, nullptr);
		praat_addAction1 (classSpeechRecognizer, 1, U"Get Whisper model name", nullptr, 1,
				QUERY_ONE_FOR_STRING__SpeechRecognizer_getModelName);
		praat_addAction1 (classSpeechRecognizer, 1, U"Get language name", nullptr, 1,
				QUERY_ONE_FOR_STRING__SpeechRecognizer_getLanguageName);

	praat_addAction2 (classSpeechRecognizer, 1, classSound, 1, U"Transcribe", nullptr, 0,
			QUERY_ONE_AND_ONE_FOR_STRING__SpeechRecognizer_Sound_recognize);
}
