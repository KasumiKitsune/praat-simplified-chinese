/* PreferencesDialog.cpp
 *
 * Copyright (C) 1992-2026 Paul Boersma, KasumiKitsune
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

#include "PreferencesDialog.h"
#include "GuiP.h"
#include "praatP.h"
#include "praatM.h"
#include "ButtonEditor.h"
#include "GraphicsP.h"
#include "praat_translate.h"
#include "melder_audio.h"
#include "MelderThread.h"
extern "C" void PRAAT__editButtons (UiForm, integer, Stackel, conststring32, Interpreter, conststring32, bool, void *, Editor);

/* Forward declarations for subsystem preferences */
integer SoundRecorder_getBufferSizePref_MB ();
void SoundRecorder_setBufferSizePref_MB (integer bufferSize);

integer LongSound_getBufferSizePref_seconds ();
void LongSound_setBufferSizePref_seconds (integer maxViewable);

integer SpeechRecognizer_getMaxNumberOfThreadsForTranscription ();
void SpeechRecognizer_setMaxNumberOfThreadsForTranscription (integer numberOfThreads);
integer SpeechRecognizer_getMaxNumberOfThreadsForDiarization ();
void SpeechRecognizer_setMaxNumberOfThreadsForDiarization (integer numberOfThreads);

Thing_define (PreferencesDialog, Thing) {
	GuiDialog dialog;
	GuiList categoryList;
	integer currentPanel;
	std::vector <GuiControl> panelControls [6]; // 1-indexed: panels 1 to 5

	// Panel 1: General & Language
	GuiOptionMenu languageMenu;
	GuiOptionMenu cjkFontStyleMenu;
	GuiButton openButtonEditorButton;

	// Panel 2: Sound & Audio
	GuiText recordingBufferSizeText;
	GuiOptionMenu inputSoundSystemMenu;
	GuiOptionMenu inputSoundDeviceMenu;
	GuiOptionMenu outputSoundSystemMenu;
	GuiOptionMenu maximumAsynchronicityMenu;
	GuiText silenceBeforeText;
	GuiText silenceAfterText;
	GuiText longSoundMaxViewablePartText;

	// Panel 3: Text & Encoding
	GuiOptionMenu textInputEncodingMenu;
	GuiOptionMenu textOutputEncodingMenu;

	// Panel 4: AI & Speech
	GuiText transcriptionThreadsText;
	GuiText diarizationThreadsText;

	// Panel 5: Advanced & Multithreading
	GuiCheckButton useMultithreadingCheck;
	GuiText maxThreadsText;
	GuiText minElementsText;
	GuiCheckButton traceThreadsCheck;
	GuiCheckButton tracingCheck;
	GuiText debugOptionText;

	// Bottom action buttons
	GuiButton restoreDefaultsButton;
	GuiButton applyButton;
	GuiButton cancelButton;
	GuiButton okButton;
};

Thing_implement (PreferencesDialog, Thing, 0);

static PreferencesDialog thePreferencesDialogInstance = nullptr;

static void PreferencesDialog_close (PreferencesDialog me);
static bool PreferencesDialog_apply (PreferencesDialog me);
static void PreferencesDialog_restoreDefaults (PreferencesDialog me);
static void PreferencesDialog_loadValues (PreferencesDialog me);

static void PreferencesDialog_showPanel (PreferencesDialog me, integer panelIndex) {
	if (panelIndex < 1 || panelIndex > 5)
		return;
	for (integer ipanel = 1; ipanel <= 5; ipanel ++) {
		if (ipanel != panelIndex) {
			for (GuiControl control : my panelControls [ipanel])
				GuiThing_hide (control);
		}
	}
	for (GuiControl control : my panelControls [panelIndex])
		GuiThing_show (control);
	my currentPanel = panelIndex;

	#if motif
		if (my dialog && my dialog -> d_widget && my dialog -> d_widget -> window) {
			RECT r { 215, 10, 715, 450 };
			RedrawWindow ((HWND) my dialog -> d_widget -> window, & r, NULL,
				RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN | RDW_UPDATENOW);
		}
	#elif gtk
		if (my dialog && my dialog -> d_widget) {
			gtk_widget_queue_draw (GTK_WIDGET (my dialog -> d_widget));
		}
	#elif cocoa
		if (my dialog && my dialog -> d_widget) {
			[(NSView *) my dialog -> d_widget setNeedsDisplay: YES];
		}
	#endif
}

static void gui_categoryList_cb_selectionChanged (Thing boss, GuiList_SelectionChangedEvent event) {
	PreferencesDialog me = (PreferencesDialog) boss;
	autoINTVEC selected = GuiList_getSelectedPositions (event -> list);
	if (selected.size > 0 && selected [1] >= 1 && selected [1] <= 5) {
		PreferencesDialog_showPanel (me, selected [1]);
	}
}

static void gui_button_cb_openButtonEditor (Thing /* boss */, GuiButtonEvent /* event */) {
	PRAAT__editButtons (nullptr, 0, nullptr, nullptr, nullptr, nullptr, false, nullptr, nullptr);
}

static void gui_button_cb_restoreDefaults (Thing boss, GuiButtonEvent /* event */) {
	PreferencesDialog me = (PreferencesDialog) boss;
	PreferencesDialog_restoreDefaults (me);
}

static void gui_button_cb_apply (Thing boss, GuiButtonEvent /* event */) {
	PreferencesDialog me = (PreferencesDialog) boss;
	PreferencesDialog_apply (me);
}

static void gui_button_cb_cancel (Thing boss, GuiButtonEvent /* event */) {
	PreferencesDialog me = (PreferencesDialog) boss;
	PreferencesDialog_close (me);
}

static void gui_button_cb_ok (Thing boss, GuiButtonEvent /* event */) {
	PreferencesDialog me = (PreferencesDialog) boss;
	if (PreferencesDialog_apply (me)) {
		PreferencesDialog_close (me);
	}
}

static void gui_preferences_dialog_cb_close (Thing boss) {
	PreferencesDialog me = (PreferencesDialog) boss;
	PreferencesDialog_close (me);
}

static void PreferencesDialog_close (PreferencesDialog me) {
	if (thePreferencesDialogInstance == me)
		thePreferencesDialogInstance = nullptr;
	if (my dialog) {
		GuiThing_hide (my dialog);
	}
	forget (me);
}

static void PreferencesDialog_loadValues (PreferencesDialog me) {
	// Panel 1: General & Language
	GuiOptionMenu_setValue (my languageMenu, g_language_choice + 1);
	GuiOptionMenu_setValue (my cjkFontStyleMenu, (int) theGraphicsCjkFontStyle - (int) kGraphics_cjkFontStyle::MIN + 1);

	// Panel 2: Sound & Audio
	GuiText_setString (my recordingBufferSizeText, Melder_integer (SoundRecorder_getBufferSizePref_MB ()));
	GuiOptionMenu_setValue (my inputSoundSystemMenu, (int) MelderAudio_getInputSoundSystem () - (int) kMelder_inputSoundSystem::MIN + 1);
	{
		conststring32 curDevName = MelderAudio_getInputDeviceName ();
		int selectedOption = 1; // 1 = "System default"
		if (curDevName && curDevName [0] != U'\0') {
			MelderAudio_DeviceList devList = { 0 };
			MelderAudio_getInputDeviceList (& devList);
			for (integer i = 0; i < devList. count; i ++) {
				if (Melder_equ (curDevName, devList. names [i]. get())) {
					selectedOption = (int) i + 2;
					break;
				}
			}
		}
		GuiOptionMenu_setValue (my inputSoundDeviceMenu, selectedOption);
	}
	GuiOptionMenu_setValue (my outputSoundSystemMenu, (int) MelderAudio_getOutputSoundSystem () - (int) kMelder_outputSoundSystem::MIN + 1);
	GuiOptionMenu_setValue (my maximumAsynchronicityMenu, (int) MelderAudio_getOutputMaximumAsynchronicity () - (int) kMelder_asynchronicityLevel::MIN + 1);
	GuiText_setString (my silenceBeforeText, Melder_double (MelderAudio_getOutputSilenceBefore ()));
	GuiText_setString (my silenceAfterText, Melder_double (MelderAudio_getOutputSilenceAfter ()));
	GuiText_setString (my longSoundMaxViewablePartText, Melder_integer (LongSound_getBufferSizePref_seconds ()));

	// Panel 3: Text & Encoding
	GuiOptionMenu_setValue (my textInputEncodingMenu, (int) Melder_getInputEncoding () - (int) kMelder_textInputEncoding::MIN + 1);
	GuiOptionMenu_setValue (my textOutputEncodingMenu, (int) Melder_getOutputEncoding () - (int) kMelder_textOutputEncoding::MIN + 1);

	// Panel 4: AI & Speech
	GuiText_setString (my transcriptionThreadsText, Melder_integer (SpeechRecognizer_getMaxNumberOfThreadsForTranscription ()));
	GuiText_setString (my diarizationThreadsText, Melder_integer (SpeechRecognizer_getMaxNumberOfThreadsForDiarization ()));

	// Panel 5: Advanced & Multithreading
	GuiCheckButton_setValue (my useMultithreadingCheck, MelderThread_getUseMultithreading ());
	GuiText_setString (my maxThreadsText, Melder_integer (MelderThread_getMaximumNumberOfConcurrentThreads ()));
	GuiText_setString (my minElementsText, Melder_integer (MelderThread_getMinimumNumberOfElementsPerThread ()));
	GuiCheckButton_setValue (my traceThreadsCheck, MelderThread_getTraceThreads ());
	GuiCheckButton_setValue (my tracingCheck, Melder_isTracingGlobally);
	GuiText_setString (my debugOptionText, Melder_integer (Melder_debug));
}

static void PreferencesDialog_restoreDefaults (PreferencesDialog me) {
	// Panel 1: General & Language
	GuiOptionMenu_setValue (my languageMenu, 2); // Simplified Chinese
	GuiOptionMenu_setValue (my cjkFontStyleMenu, (int) kGraphics_cjkFontStyle::DEFAULT - (int) kGraphics_cjkFontStyle::MIN + 1);

	// Panel 2: Sound & Audio
	GuiText_setString (my recordingBufferSizeText, U"60");
	GuiOptionMenu_setValue (my inputSoundSystemMenu, (int) kMelder_inputSoundSystem::DEFAULT - (int) kMelder_inputSoundSystem::MIN + 1);
	GuiOptionMenu_setValue (my inputSoundDeviceMenu, 1);
	GuiOptionMenu_setValue (my outputSoundSystemMenu, (int) kMelder_outputSoundSystem::DEFAULT - (int) kMelder_outputSoundSystem::MIN + 1);
	GuiOptionMenu_setValue (my maximumAsynchronicityMenu, (int) kMelder_asynchronicityLevel::DEFAULT - (int) kMelder_asynchronicityLevel::MIN + 1);
	GuiText_setString (my silenceBeforeText, Melder_double (kMelderAudio_outputSilenceBefore_DEFAULT));
	GuiText_setString (my silenceAfterText, Melder_double (kMelderAudio_outputSilenceAfter_DEFAULT));
	GuiText_setString (my longSoundMaxViewablePartText, U"60");

	// Panel 3: Text & Encoding
	GuiOptionMenu_setValue (my textInputEncodingMenu, (int) kMelder_textInputEncoding::DEFAULT - (int) kMelder_textInputEncoding::MIN + 1);
	GuiOptionMenu_setValue (my textOutputEncodingMenu, (int) kMelder_textOutputEncoding::DEFAULT - (int) kMelder_textOutputEncoding::MIN + 1);

	// Panel 4: AI & Speech
	GuiText_setString (my transcriptionThreadsText, U"0");
	GuiText_setString (my diarizationThreadsText, U"0");

	// Panel 5: Advanced & Multithreading
	GuiCheckButton_setValue (my useMultithreadingCheck, true);
	GuiText_setString (my maxThreadsText, U"0");
	GuiText_setString (my minElementsText, U"0");
	GuiCheckButton_setValue (my traceThreadsCheck, false);
	GuiCheckButton_setValue (my tracingCheck, false);
	GuiText_setString (my debugOptionText, U"0");
}

static bool PreferencesDialog_apply (PreferencesDialog me) {
	try {
		// 1. Language & Fonts
		int langVal = GuiOptionMenu_getValue (my languageMenu);
		int newLang = (langVal >= 1 && langVal <= 3) ? (langVal - 1) : 1;
		bool langChanged = (g_language_choice != newLang);
		g_language_choice = newLang;

		int cjkVal = GuiOptionMenu_getValue (my cjkFontStyleMenu);
		theGraphicsCjkFontStyle = (kGraphics_cjkFontStyle) (cjkVal - 1 + (int) kGraphics_cjkFontStyle::MIN);

		// 2. Sound & Audio
		autostring32 recBufStr = GuiText_getString (my recordingBufferSizeText);
		integer recBuf = Melder_atoi (recBufStr.get());
		if (recBuf <= 0)
			Melder_throw (U"Recording buffer size must be a positive integer.");
		if (recBuf > 1000)
			Melder_throw (U"Recording buffer size cannot exceed 1000 megabytes.");
		SoundRecorder_setBufferSizePref_MB (recBuf);

		int inSysVal = GuiOptionMenu_getValue (my inputSoundSystemMenu);
		MelderAudio_setInputSoundSystem ((kMelder_inputSoundSystem) (inSysVal - 1 + (int) kMelder_inputSoundSystem::MIN));

		int inDevChoice = GuiOptionMenu_getValue (my inputSoundDeviceMenu);
		if (inDevChoice <= 1) {
			MelderAudio_setInputDeviceName (U"");
		} else {
			MelderAudio_DeviceList devList = { 0 };
			MelderAudio_getInputDeviceList (& devList);
			int devIndex = inDevChoice - 2;
			if (devIndex >= 0 && devIndex < devList. count) {
				MelderAudio_setInputDeviceName (devList. names [devIndex]. get());
			} else {
				MelderAudio_setInputDeviceName (U"");
			}
		}

		int outSysVal = GuiOptionMenu_getValue (my outputSoundSystemMenu);
		MelderAudio_setOutputSoundSystem ((kMelder_outputSoundSystem) (outSysVal - 1 + (int) kMelder_outputSoundSystem::MIN));

		int asyncVal = GuiOptionMenu_getValue (my maximumAsynchronicityMenu);
		MelderAudio_setOutputMaximumAsynchronicity ((kMelder_asynchronicityLevel) (asyncVal - 1 + (int) kMelder_asynchronicityLevel::MIN));

		autostring32 sbStr = GuiText_getString (my silenceBeforeText);
		double sb = Melder_atof (sbStr.get());
		MelderAudio_setOutputSilenceBefore (sb);

		autostring32 saStr = GuiText_getString (my silenceAfterText);
		double sa = Melder_atof (saStr.get());
		MelderAudio_setOutputSilenceAfter (sa);

		autostring32 lsStr = GuiText_getString (my longSoundMaxViewablePartText);
		integer ls = Melder_atoi (lsStr.get());
		if (ls <= 0)
			Melder_throw (U"LongSound maximum viewable part must be positive.");
		LongSound_setBufferSizePref_seconds (ls);

		// 3. Text & Encoding
		int inEncVal = GuiOptionMenu_getValue (my textInputEncodingMenu);
		Melder_setInputEncoding ((kMelder_textInputEncoding) (inEncVal - 1 + (int) kMelder_textInputEncoding::MIN));

		int outEncVal = GuiOptionMenu_getValue (my textOutputEncodingMenu);
		Melder_setOutputEncoding ((kMelder_textOutputEncoding) (outEncVal - 1 + (int) kMelder_textOutputEncoding::MIN));

		// 4. AI & Speech
		autostring32 tThStr = GuiText_getString (my transcriptionThreadsText);
		integer tTh = Melder_atoi (tThStr.get());
		if (tTh < 0)
			Melder_throw (U"Transcription threads cannot be negative.");
		SpeechRecognizer_setMaxNumberOfThreadsForTranscription (tTh);

		autostring32 dThStr = GuiText_getString (my diarizationThreadsText);
		integer dTh = Melder_atoi (dThStr.get());
		if (dTh < 0)
			Melder_throw (U"Diarization threads cannot be negative.");
		SpeechRecognizer_setMaxNumberOfThreadsForDiarization (dTh);

		// 5. Advanced & Multithreading
		bool useMt = GuiCheckButton_getValue (my useMultithreadingCheck);
		autostring32 maxThStr = GuiText_getString (my maxThreadsText);
		integer maxTh = Melder_atoi (maxThStr.get());
		autostring32 minElStr = GuiText_getString (my minElementsText);
		integer minEl = Melder_atoi (minElStr.get());
		bool traceTh = GuiCheckButton_getValue (my traceThreadsCheck);
		MelderThread_debugMultithreading (useMt, maxTh, minEl, traceTh);

		bool traceGlob = GuiCheckButton_getValue (my tracingCheck);
		Melder_setTracing (traceGlob);
		autostring32 dbgStr = GuiText_getString (my debugOptionText);
		integer dbg = Melder_atoi (dbgStr.get());
		Melder_debug = dbg;

		if (langChanged) {
			praat_languageChanged ();
		}

		return true;
	} catch (MelderError) {
		Melder_flushError (U"Preferences could not be completely saved.");
		return false;
	}
}

#define ADD_PANEL_CONTROL(panelIndex, control) \
	my panelControls [panelIndex].push_back (control)

void PRAAT_preferencesDialog (GuiWindow parentWindow) {
	if (thePreferencesDialogInstance) {
		GuiThing_show (thePreferencesDialogInstance -> dialog);
		return;
	}

	autoPreferencesDialog me = Thing_new (PreferencesDialog);

	const int dialogWidth = 720;
	const int dialogHeight = 500;

	double screenX, screenY, screenWidth, screenHeight;
	Gui_getWindowPositioningBounds (& screenX, & screenY, & screenWidth, & screenHeight);
	const int dialogX = Melder_iround (screenX + (screenWidth - dialogWidth) / 2.0);
	const int dialogY = Melder_iround (screenY + (screenHeight - dialogHeight) / 2.0);

	my dialog = GuiDialog_create (parentWindow, dialogX, dialogY, dialogWidth, dialogHeight,
			praat_translate (U"Preferences"), gui_preferences_dialog_cb_close, me.get(), GuiDialog_Modality::MODELESS);

	GuiForm form = my dialog;

	/*
	 * 1. Left Sidebar: Category List
	 */
	my categoryList = GuiList_create (form, 16, 210, 16, 440, false, nullptr);
	GuiList_insertItem (my categoryList, praat_translate (U"General & Language"), 1);
	GuiList_insertItem (my categoryList, praat_translate (U"Sound Recording & Playback"), 2);
	GuiList_insertItem (my categoryList, praat_translate (U"Text & Encoding"), 3);
	GuiList_insertItem (my categoryList, praat_translate (U"AI & Speech"), 4);
	GuiList_insertItem (my categoryList, praat_translate (U"Advanced & Multi-threading"), 5);
	GuiList_setSelectionChangedCallback (my categoryList, gui_categoryList_cb_selectionChanged, me.get());
	GuiThing_show (my categoryList);

	const int labelX1 = 230, labelX2 = 420;
	const int fieldX1 = 430, fieldX2 = 700;
	const int fullX1 = 230, fullX2 = 700;

	/*
	 * 2. Panel 1: General & Language
	 */
	{
		int y = 20;
		GuiLabel h1 = GuiLabel_createShown (form, fullX1, fullX2, y, y + 20, praat_translate (U"Interface & Fonts"), GuiLabel_BOLD);
		ADD_PANEL_CONTROL (1, h1);
		y += 30;

		GuiLabel l1 = GuiLabel_createShown (form, labelX1, labelX2, y, y + 20, praat_translate (U"Interface language:"), GuiLabel_RIGHT);
		ADD_PANEL_CONTROL (1, l1);
		my languageMenu = GuiOptionMenu_createShown (form, fieldX1, fieldX2, y, y + 20, 0);
		GuiOptionMenu_addOption (my languageMenu, U"English");
		GuiOptionMenu_addOption (my languageMenu, U"简体中文 (Simplified Chinese)");
		GuiOptionMenu_addOption (my languageMenu, U"繁体中文 (Traditional Chinese)");
		ADD_PANEL_CONTROL (1, my languageMenu);
		y += 32;

		GuiLabel l2 = GuiLabel_createShown (form, labelX1, labelX2, y, y + 20, praat_translate (U"CJK font style:"), GuiLabel_RIGHT);
		ADD_PANEL_CONTROL (1, l2);
		my cjkFontStyleMenu = GuiOptionMenu_createShown (form, fieldX1, fieldX2, y, y + 20, 0);
		for (int i = (int) kGraphics_cjkFontStyle::MIN; i <= (int) kGraphics_cjkFontStyle::MAX; i ++)
			GuiOptionMenu_addOption (my cjkFontStyleMenu, praat_translate (kGraphics_cjkFontStyle_getText ((kGraphics_cjkFontStyle) i)));
		ADD_PANEL_CONTROL (1, my cjkFontStyleMenu);
		y += 44;

		GuiLabel h2 = GuiLabel_createShown (form, fullX1, fullX2, y, y + 20, praat_translate (U"Shortcuts & Customization"), GuiLabel_BOLD);
		ADD_PANEL_CONTROL (1, h2);
		y += 30;

		my openButtonEditorButton = GuiButton_createShown (form, fullX1, fullX1 + 220, y, y + Gui_PUSHBUTTON_HEIGHT,
				praat_translate (U"Open Buttons Editor..."), gui_button_cb_openButtonEditor, me.get(), 0);
		ADD_PANEL_CONTROL (1, my openButtonEditorButton);
		y += 34;
		GuiLabel cap1 = GuiLabel_createShown (form, fullX1, fullX2, y, y + 20, praat_translate (U"(Customize menu commands and action buttons)"), 0);
		ADD_PANEL_CONTROL (1, cap1);
	}

	/*
	 * 3. Panel 2: Sound & Audio
	 */
	{
		int y = 20;
		GuiLabel h1 = GuiLabel_createShown (form, fullX1, fullX2, y, y + 20, praat_translate (U"Sound Recording"), GuiLabel_BOLD);
		ADD_PANEL_CONTROL (2, h1);
		y += 28;

		GuiLabel l1 = GuiLabel_createShown (form, labelX1, labelX2, y, y + 20, praat_translate (U"Buffer size (MB):"), GuiLabel_RIGHT);
		ADD_PANEL_CONTROL (2, l1);
		my recordingBufferSizeText = GuiText_createShown (form, fieldX1, fieldX2, y, y + Gui_TEXTFIELD_HEIGHT, 0);
		ADD_PANEL_CONTROL (2, my recordingBufferSizeText);
		y += 30;

		GuiLabel l2 = GuiLabel_createShown (form, labelX1, labelX2, y, y + 20, praat_translate (U"Input sound system:"), GuiLabel_RIGHT);
		ADD_PANEL_CONTROL (2, l2);
		my inputSoundSystemMenu = GuiOptionMenu_createShown (form, fieldX1, fieldX2, y, y + 20, 0);
		for (int i = (int) kMelder_inputSoundSystem::MIN; i <= (int) kMelder_inputSoundSystem::MAX; i ++)
			GuiOptionMenu_addOption (my inputSoundSystemMenu, praat_translate (kMelder_inputSoundSystem_getText ((kMelder_inputSoundSystem) i)));
		ADD_PANEL_CONTROL (2, my inputSoundSystemMenu);
		y += 30;

		GuiLabel l2b = GuiLabel_createShown (form, labelX1, labelX2, y, y + 20, praat_translate (U"Input sound device:"), GuiLabel_RIGHT);
		ADD_PANEL_CONTROL (2, l2b);
		my inputSoundDeviceMenu = GuiOptionMenu_createShown (form, fieldX1, fieldX2, y, y + 20, 0);
		GuiOptionMenu_addOption (my inputSoundDeviceMenu, praat_translate (U"System default"));
		MelderAudio_DeviceList devList = { 0 };
		MelderAudio_getInputDeviceList (& devList);
		for (integer i = 0; i < devList. count; i ++) {
			GuiOptionMenu_addOption (my inputSoundDeviceMenu, devList. names [i]. get());
		}
		ADD_PANEL_CONTROL (2, my inputSoundDeviceMenu);
		y += 38;

		GuiLabel h2 = GuiLabel_createShown (form, fullX1, fullX2, y, y + 20, praat_translate (U"Sound Playback"), GuiLabel_BOLD);
		ADD_PANEL_CONTROL (2, h2);
		y += 28;

		GuiLabel l3 = GuiLabel_createShown (form, labelX1, labelX2, y, y + 20, praat_translate (U"Output sound system:"), GuiLabel_RIGHT);
		ADD_PANEL_CONTROL (2, l3);
		my outputSoundSystemMenu = GuiOptionMenu_createShown (form, fieldX1, fieldX2, y, y + 20, 0);
		for (int i = (int) kMelder_outputSoundSystem::MIN; i <= (int) kMelder_outputSoundSystem::MAX; i ++)
			GuiOptionMenu_addOption (my outputSoundSystemMenu, praat_translate (kMelder_outputSoundSystem_getText ((kMelder_outputSoundSystem) i)));
		ADD_PANEL_CONTROL (2, my outputSoundSystemMenu);
		y += 30;

		GuiLabel l4 = GuiLabel_createShown (form, labelX1, labelX2, y, y + 20, praat_translate (U"Maximum asynchronicity:"), GuiLabel_RIGHT);
		ADD_PANEL_CONTROL (2, l4);
		my maximumAsynchronicityMenu = GuiOptionMenu_createShown (form, fieldX1, fieldX2, y, y + 20, 0);
		for (int i = (int) kMelder_asynchronicityLevel::MIN; i <= (int) kMelder_asynchronicityLevel::MAX; i ++)
			GuiOptionMenu_addOption (my maximumAsynchronicityMenu, praat_translate (kMelder_asynchronicityLevel_getText ((kMelder_asynchronicityLevel) i)));
		ADD_PANEL_CONTROL (2, my maximumAsynchronicityMenu);
		y += 30;

		GuiLabel l5 = GuiLabel_createShown (form, labelX1, labelX2, y, y + 20, praat_translate (U"Silence before (seconds):"), GuiLabel_RIGHT);
		ADD_PANEL_CONTROL (2, l5);
		my silenceBeforeText = GuiText_createShown (form, fieldX1, fieldX2, y, y + Gui_TEXTFIELD_HEIGHT, 0);
		ADD_PANEL_CONTROL (2, my silenceBeforeText);
		y += 30;

		GuiLabel l6 = GuiLabel_createShown (form, labelX1, labelX2, y, y + 20, praat_translate (U"Silence after (seconds):"), GuiLabel_RIGHT);
		ADD_PANEL_CONTROL (2, l6);
		my silenceAfterText = GuiText_createShown (form, fieldX1, fieldX2, y, y + Gui_TEXTFIELD_HEIGHT, 0);
		ADD_PANEL_CONTROL (2, my silenceAfterText);
		y += 38;

		GuiLabel h3 = GuiLabel_createShown (form, fullX1, fullX2, y, y + 20, praat_translate (U"LongSound"), GuiLabel_BOLD);
		ADD_PANEL_CONTROL (2, h3);
		y += 28;

		GuiLabel l7 = GuiLabel_createShown (form, labelX1, labelX2, y, y + 20, praat_translate (U"Max viewable part (seconds):"), GuiLabel_RIGHT);
		ADD_PANEL_CONTROL (2, l7);
		my longSoundMaxViewablePartText = GuiText_createShown (form, fieldX1, fieldX2, y, y + Gui_TEXTFIELD_HEIGHT, 0);
		ADD_PANEL_CONTROL (2, my longSoundMaxViewablePartText);
	}

	/*
	 * 4. Panel 3: Text & Encoding
	 */
	{
		int y = 20;
		GuiLabel h1 = GuiLabel_createShown (form, fullX1, fullX2, y, y + 20, praat_translate (U"Text Files Encoding"), GuiLabel_BOLD);
		ADD_PANEL_CONTROL (3, h1);
		y += 30;

		GuiLabel l1 = GuiLabel_createShown (form, labelX1, labelX2, y, y + 20, praat_translate (U"8-bit text reading encoding:"), GuiLabel_RIGHT);
		ADD_PANEL_CONTROL (3, l1);
		my textInputEncodingMenu = GuiOptionMenu_createShown (form, fieldX1, fieldX2, y, y + 20, 0);
		for (int i = (int) kMelder_textInputEncoding::MIN; i <= (int) kMelder_textInputEncoding::MAX; i ++)
			GuiOptionMenu_addOption (my textInputEncodingMenu, praat_translate (kMelder_textInputEncoding_getText ((kMelder_textInputEncoding) i)));
		ADD_PANEL_CONTROL (3, my textInputEncodingMenu);
		y += 32;

		GuiLabel l2 = GuiLabel_createShown (form, labelX1, labelX2, y, y + 20, praat_translate (U"Text writing output encoding:"), GuiLabel_RIGHT);
		ADD_PANEL_CONTROL (3, l2);
		my textOutputEncodingMenu = GuiOptionMenu_createShown (form, fieldX1, fieldX2, y, y + 20, 0);
		for (int i = (int) kMelder_textOutputEncoding::MIN; i <= (int) kMelder_textOutputEncoding::MAX; i ++)
			GuiOptionMenu_addOption (my textOutputEncodingMenu, praat_translate (kMelder_textOutputEncoding_getText ((kMelder_textOutputEncoding) i)));
		ADD_PANEL_CONTROL (3, my textOutputEncodingMenu);
		y += 40;

		GuiLabel note = GuiLabel_createShown (form, fullX1, fullX2, y, y + 20,
				praat_translate (U"Note: UTF-8 is recommended for modern cross-platform multilingual compatibility."), 0);
		ADD_PANEL_CONTROL (3, note);
	}

	/*
	 * 5. Panel 4: AI & Speech
	 */
	{
		int y = 20;
		GuiLabel h1 = GuiLabel_createShown (form, fullX1, fullX2, y, y + 20, praat_translate (U"Speech-to-Text & Diarization"), GuiLabel_BOLD);
		ADD_PANEL_CONTROL (4, h1);
		y += 28;

		GuiLabel cap0 = GuiLabel_createShown (form, fullX1, fullX2, y, y + 20,
				praat_translate (U"Configure parallel thread allocation for local AI transcription & diarization."), 0);
		ADD_PANEL_CONTROL (4, cap0);
		y += 32;

		GuiLabel l1 = GuiLabel_createShown (form, labelX1, labelX2, y, y + 20, praat_translate (U"Transcription max threads:"), GuiLabel_RIGHT);
		ADD_PANEL_CONTROL (4, l1);
		my transcriptionThreadsText = GuiText_createShown (form, fieldX1, fieldX2, y, y + Gui_TEXTFIELD_HEIGHT, 0);
		ADD_PANEL_CONTROL (4, my transcriptionThreadsText);
		y += 26;

		GuiLabel cap1 = GuiLabel_createShown (form, fullX1, fullX2, y, y + 18,
				praat_translate (U"(0 = automatic; uses default CPU concurrency for Whisper transcription)"), 0);
		ADD_PANEL_CONTROL (4, cap1);
		y += 34;

		GuiLabel l2 = GuiLabel_createShown (form, labelX1, labelX2, y, y + 20, praat_translate (U"Diarization max threads:"), GuiLabel_RIGHT);
		ADD_PANEL_CONTROL (4, l2);
		my diarizationThreadsText = GuiText_createShown (form, fieldX1, fieldX2, y, y + Gui_TEXTFIELD_HEIGHT, 0);
		ADD_PANEL_CONTROL (4, my diarizationThreadsText);
		y += 26;

		GuiLabel cap2 = GuiLabel_createShown (form, fullX1, fullX2, y, y + 18,
				praat_translate (U"(0 = automatic; uses default CPU concurrency for speaker diarization)"), 0);
		ADD_PANEL_CONTROL (4, cap2);
	}

	/*
	 * 6. Panel 5: Advanced & Multithreading
	 */
	{
		int y = 20;
		GuiLabel h1 = GuiLabel_createShown (form, fullX1, fullX2, y, y + 20, praat_translate (U"Multi-threading Settings"), GuiLabel_BOLD);
		ADD_PANEL_CONTROL (5, h1);
		y += 28;

		my useMultithreadingCheck = GuiCheckButton_createShown (form, fullX1, fullX2, y, y + Gui_CHECKBUTTON_HEIGHT,
				praat_translate (U"Enable multi-threading"), nullptr, nullptr, 0);
		ADD_PANEL_CONTROL (5, my useMultithreadingCheck);
		y += 30;

		GuiLabel l1 = GuiLabel_createShown (form, labelX1, labelX2, y, y + 20, praat_translate (U"Max concurrent threads:"), GuiLabel_RIGHT);
		ADD_PANEL_CONTROL (5, l1);
		my maxThreadsText = GuiText_createShown (form, fieldX1, fieldX2, y, y + Gui_TEXTFIELD_HEIGHT, 0);
		ADD_PANEL_CONTROL (5, my maxThreadsText);
		y += 30;

		GuiLabel l2 = GuiLabel_createShown (form, labelX1, labelX2, y, y + 20, praat_translate (U"Min elements per thread:"), GuiLabel_RIGHT);
		ADD_PANEL_CONTROL (5, l2);
		my minElementsText = GuiText_createShown (form, fieldX1, fieldX2, y, y + Gui_TEXTFIELD_HEIGHT, 0);
		ADD_PANEL_CONTROL (5, my minElementsText);
		y += 30;

		my traceThreadsCheck = GuiCheckButton_createShown (form, fullX1, fullX2, y, y + Gui_CHECKBUTTON_HEIGHT,
				praat_translate (U"Trace thread execution info"), nullptr, nullptr, 0);
		ADD_PANEL_CONTROL (5, my traceThreadsCheck);
		y += 38;

		GuiLabel h2 = GuiLabel_createShown (form, fullX1, fullX2, y, y + 20, praat_translate (U"Debugging & Diagnostics"), GuiLabel_BOLD);
		ADD_PANEL_CONTROL (5, h2);
		y += 28;

		my tracingCheck = GuiCheckButton_createShown (form, fullX1, fullX2, y, y + Gui_CHECKBUTTON_HEIGHT,
				praat_translate (U"Enable global tracing"), nullptr, nullptr, 0);
		ADD_PANEL_CONTROL (5, my tracingCheck);
		y += 30;

		GuiLabel l3 = GuiLabel_createShown (form, labelX1, labelX2, y, y + 20, praat_translate (U"Debug option code:"), GuiLabel_RIGHT);
		ADD_PANEL_CONTROL (5, l3);
		my debugOptionText = GuiText_createShown (form, fieldX1, fieldX2, y, y + Gui_TEXTFIELD_HEIGHT, 0);
		ADD_PANEL_CONTROL (5, my debugOptionText);
	}

	/*
	 * 7. Bottom Action Buttons
	 */
	const int buttonY = 455;
	my restoreDefaultsButton = GuiButton_createShown (form, 16, 160, buttonY, buttonY + Gui_PUSHBUTTON_HEIGHT,
			praat_translate (U"Restore Defaults"), gui_button_cb_restoreDefaults, me.get(), 0);

	my applyButton = GuiButton_createShown (form, 440, 520, buttonY, buttonY + Gui_PUSHBUTTON_HEIGHT,
			praat_translate (U"Apply"), gui_button_cb_apply, me.get(), 0);

	my cancelButton = GuiButton_createShown (form, 530, 610, buttonY, buttonY + Gui_PUSHBUTTON_HEIGHT,
			praat_translate (U"Cancel"), gui_button_cb_cancel, me.get(), 0);

	my okButton = GuiButton_createShown (form, 620, 704, buttonY, buttonY + Gui_PUSHBUTTON_HEIGHT,
			praat_translate (U"OK"), gui_button_cb_ok, me.get(), GuiButton_DEFAULT);

	/*
	 * 8. Initial State: Load values, select Panel 1, and show
	 */
	PreferencesDialog_loadValues (me.get());
	GuiList_selectItem (my categoryList, 1);
	PreferencesDialog_showPanel (me.get(), 1);

	thePreferencesDialogInstance = me.get();
	GuiThing_show (my dialog);

	me.releaseToUser ();
}

DIRECT (PRAAT_preferences) {
	PRAAT
		PRAAT_preferencesDialog (theCurrentPraatApplication -> topShell);
	PRAAT_END
}

/* End of file PreferencesDialog.cpp */
