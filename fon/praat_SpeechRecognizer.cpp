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
static GuiLabel theWhisperHeaderLabel = nullptr;
static GuiLabel theWhisperDesc1 = nullptr;
static GuiLabel theWhisperDesc2 = nullptr;
static GuiLabel theWhisperDesc3 = nullptr;
static GuiButton theWhisperDownloadButton = nullptr;

static void updateWhisperGuideUi () {
	if (! theWhisperGuideDialog)
		return;

	constSTRVEC modelNames = theCurrentSpeechRecognizerModelNames ();
	bool isZh = (g_language_choice != 0);

	if (modelNames.size > 0) {
		conststring32 headerText = isZh
			? Melder_cat (U"已就绪：检测到 ", Melder_integer (modelNames.size), U" 个可用 Whisper 模型")
			: Melder_cat (U"Ready: ", Melder_integer (modelNames.size), U" Whisper model(s) detected");
		GuiLabel_setText (theWhisperHeaderLabel, headerText);

		MelderString listStr;
		for (integer i = 1; i <= modelNames.size; i ++) {
			if (i > 1)
				MelderString_append (& listStr, U",  ");
			MelderString_append (& listStr, modelNames [i]);
		}
		conststring32 desc1Text = isZh
			? Melder_cat (U"当前就绪模型：", listStr.string)
			: Melder_cat (U"Available models: ", listStr.string);
		GuiLabel_setText (theWhisperDesc1, desc1Text);

		GuiLabel_setText (theWhisperDesc2,
			U"Speech recognition is ready! Create a SpeechRecognizer or transcribe from TextGrid window.");

		GuiLabel_setText (theWhisperDesc3,
			U"Models location: models\\whispercpp\\ or preferences folder. Use buttons below to manage models.");

		if (theWhisperDownloadButton) {
			bool hasBase = false;
			for (integer i = 1; i <= modelNames.size; i ++) {
				if (str32str (modelNames [i], U"base")) {
					hasBase = true;
					break;
				}
			}
			GuiButton_setText (theWhisperDownloadButton, hasBase
				? U"Re-download base model"
				: U"Download base model");
		}
	} else {
		GuiLabel_setText (theWhisperHeaderLabel,
			U"No Whisper speech recognition models found (.bin)");
		GuiLabel_setText (theWhisperDesc1,
			U"Praat includes Whisper.cpp engine, but requires at least one model weight file.");
		GuiLabel_setText (theWhisperDesc2,
			U"Recommended model: ggml-base.bin (~142 MB, good balance of speed and accuracy).");
		GuiLabel_setText (theWhisperDesc3,
			U"Place models in models\\whispercpp\\ or user preferences directory.");
		if (theWhisperDownloadButton)
			GuiButton_setText (theWhisperDownloadButton, U"Download base model");
	}
}

static void gui_button_cb_whisperRefresh (Thing /* boss */, GuiButtonEvent /* event */) {
	updateWhisperGuideUi ();
}

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

	Melder_information (Melder_tr (foreignAccessible ?
		U"International network connection detected (official source). Downloading recommended model ggml-base.bin (~142 MB) using the default browser.\n\nThe target models folder has been opened for you. After the download finishes, place ggml-base.bin into that folder." :
		U"No international network connection detected; switched to mirror source (HF-Mirror). Downloading recommended model ggml-base.bin (~142 MB) using the default browser.\n\nThe target models folder has been opened for you. After the download finishes, place ggml-base.bin into that folder."
	));
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
		const int dialogWidth = 640;
		const int dialogHeight = 225;
		theWhisperGuideDialog = GuiDialog_create (theCurrentPraatApplication -> topShell,
			100, 70, dialogWidth, dialogHeight,
			U"Whisper Model Download & Setup Guide",
			gui_dialog_cb_whisperGoAway, nullptr,
			GuiDialog_Modality::MODAL);

		int y = Gui_TOP_DIALOG_SPACING;
		theWhisperHeaderLabel = GuiLabel_createShown (theWhisperGuideDialog,
			Gui_LEFT_DIALOG_SPACING, dialogWidth - Gui_RIGHT_DIALOG_SPACING,
			y, y + Gui_LABEL_HEIGHT,
			U"", GuiLabel_BOLD);
		y += Gui_LABEL_HEIGHT + 10;

		theWhisperDesc1 = GuiLabel_createShown (theWhisperGuideDialog,
			Gui_LEFT_DIALOG_SPACING, dialogWidth - Gui_RIGHT_DIALOG_SPACING,
			y, y + Gui_LABEL_HEIGHT,
			U"", 0);
		y += Gui_LABEL_HEIGHT + 6;

		theWhisperDesc2 = GuiLabel_createShown (theWhisperGuideDialog,
			Gui_LEFT_DIALOG_SPACING, dialogWidth - Gui_RIGHT_DIALOG_SPACING,
			y, y + Gui_LABEL_HEIGHT,
			U"", 0);
		y += Gui_LABEL_HEIGHT + 6;

		theWhisperDesc3 = GuiLabel_createShown (theWhisperGuideDialog,
			Gui_LEFT_DIALOG_SPACING, dialogWidth - Gui_RIGHT_DIALOG_SPACING,
			y, y + Gui_LABEL_HEIGHT,
			U"", 0);

		const int buttonY = dialogHeight - Gui_BOTTOM_DIALOG_SPACING - Gui_PUSHBUTTON_HEIGHT;

		// Button 1: Open Folder
		int x = Gui_LEFT_DIALOG_SPACING;
		const int w1 = 120;
		GuiButton_createShown (theWhisperGuideDialog,
			x, x + w1,
			buttonY, buttonY + Gui_PUSHBUTTON_HEIGHT,
			U"Open Folder", gui_button_cb_whisperOpenFolder, nullptr, 0);
		x += w1 + 8;

		// Button 2: Refresh Status
		const int w2 = 90;
		GuiButton_createShown (theWhisperGuideDialog,
			x, x + w2,
			buttonY, buttonY + Gui_PUSHBUTTON_HEIGHT,
			U"Refresh", gui_button_cb_whisperRefresh, nullptr, 0);
		x += w2 + 8;

		// Button 3: Download base model
		const int w3 = 155;
		theWhisperDownloadButton = GuiButton_createShown (theWhisperGuideDialog,
			x, x + w3,
			buttonY, buttonY + Gui_PUSHBUTTON_HEIGHT,
			U"Download base model", gui_button_cb_whisperDownloadBase, nullptr, GuiButton_DEFAULT);
		x += w3 + 8;

		// Button 4: Browse all models
		const int w4 = 130;
		GuiButton_createShown (theWhisperGuideDialog,
			x, x + w4,
			buttonY, buttonY + Gui_PUSHBUTTON_HEIGHT,
			U"All Models...", gui_button_cb_whisperBrowseAll, nullptr, 0);

		// Button 5: Close
		const int w5 = 75;
		int x5 = dialogWidth - Gui_RIGHT_DIALOG_SPACING - w5;
		GuiButton_createShown (theWhisperGuideDialog,
			x5, x5 + w5,
			buttonY, buttonY + Gui_PUSHBUTTON_HEIGHT,
			U"Close", gui_button_cb_whisperClose, nullptr, GuiButton_CANCEL);
	}

	updateWhisperGuideUi ();
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
		Melder_throw (
			U"No Whisper speech recognition model detected.\nThe model download & setup guide has been opened. You can also place ggml-*.bin models directly into the models/whispercpp folder next to the executable.");
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
