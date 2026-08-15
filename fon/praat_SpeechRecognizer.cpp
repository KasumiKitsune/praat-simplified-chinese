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


DIRECT (HELP__SpeechRecognizer_help) {
	HELP (U"SpeechRecognizer")
}

FORM (CREATE_ONE__SpeechRecognizer_create, U"Create SpeechRecognizer", U"Create SpeechRecognizer...") {
	LISTNUMSTR (modelIndex, modelName, U"Whisper model", constSTRVEC(), 1)
	LISTNUMSTR (languageIndex, languageName, U"Language", constSTRVEC(), 1)
OK
	static autoSTRVEC modelNames;
	modelNames = copy_STRVEC (theCurrentSpeechRecognizerModelNames());   // cannot be called twice in the same scope

	Melder_require (modelNames.size > 0,
		U"Found no Whisper-cpp models to do speech recognition with.\n"
		U"You can install them into the subfolder “whispercpp” of the folder “models” in the Praat preferences folder."
	);

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
	praat_addMenuCommand (U"Objects", U"Settings", U"-- AI settings --", nullptr, 0, nullptr);
	praat_addMenuCommand (U"Objects", U"Settings", U"AI settings...", nullptr, 0, SETTINGS__SpeechRecognizerSettings);

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
