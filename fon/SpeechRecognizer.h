#ifndef _SpeechRecognizer_h_
#define _SpeechRecognizer_h_
/* SpeechRecognizer.h
 *
 * Copyright (C) 2025,2026 Anastasia Shchupak
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

#include "Sound.h"
#include <set>

struct structSpeechRecognizer;
struct whisper_context;
struct whisper_vad_context;
struct whisper_vad_segments;
struct diarize_context;

struct autoWhisperContext {
	whisper_context *ptr;

	autoWhisperContext (whisper_context *p = nullptr) : ptr(p) {}
	~autoWhisperContext ();

	autoWhisperContext (const autoWhisperContext&) = delete;
	autoWhisperContext& operator= (const autoWhisperContext&) = delete;

	autoWhisperContext (autoWhisperContext&& other) noexcept : ptr (other.ptr) {
		other.ptr = nullptr;
	}
	autoWhisperContext& operator= (autoWhisperContext&& other) noexcept;

	[[nodiscard]]
	whisper_context *get () const { return ptr; }
};

struct autoWhisperVadContext {
	whisper_vad_context *ptr;

	autoWhisperVadContext (whisper_vad_context *p = nullptr) : ptr (p) {}
	~autoWhisperVadContext ();

	autoWhisperVadContext (const autoWhisperVadContext&) = delete;
	autoWhisperVadContext& operator= (const autoWhisperVadContext&) = delete;

	autoWhisperVadContext (autoWhisperVadContext&& other) noexcept : ptr (other.ptr) {
		other.ptr = nullptr;
	}
	autoWhisperVadContext& operator= (autoWhisperVadContext&& other) noexcept;

	[[nodiscard]]
	whisper_vad_context *get () const { return ptr; }
};

struct autoWhisperVadSegments {
	whisper_vad_segments *ptr;

	autoWhisperVadSegments (whisper_vad_segments *p = nullptr) : ptr (p) {}
	~autoWhisperVadSegments ();

	autoWhisperVadSegments (const autoWhisperVadSegments&) = delete;
	autoWhisperVadSegments& operator= (const autoWhisperVadSegments&) = delete;

	autoWhisperVadSegments (autoWhisperVadSegments && other) noexcept : ptr (other.ptr) {
		other.ptr = nullptr;
	}
	autoWhisperVadSegments& operator= (autoWhisperVadSegments && other) noexcept;

	[[nodiscard]]
	whisper_vad_segments *get () const { return ptr; }
};

struct autoDiarizeContext {
	diarize_context *ptr;

	autoDiarizeContext (diarize_context *p = nullptr) : ptr(p) {}
	~autoDiarizeContext ();

	autoDiarizeContext (const autoDiarizeContext&) = delete;
	autoDiarizeContext& operator= (const autoDiarizeContext&) = delete;

	autoDiarizeContext (autoDiarizeContext&& other) noexcept : ptr(other.ptr) {
		other.ptr = nullptr;
	}
	autoDiarizeContext& operator= (autoDiarizeContext&& other) noexcept;

	[[nodiscard]]
	diarize_context *get () const { return ptr; }
};

/*
	Default parameter values for the UI.
*/
namespace TranscriptionDefaults {
	inline constexpr conststring32 modelName = U"ggml-base.bin";
	inline constexpr conststring32 languageName = U"Autodetect language";
	inline constexpr bool includeWords = true;
	inline constexpr bool includeDiarization = false;
	inline constexpr bool useVad = true;
	inline const integer n_threads = MelderThread_getNumberOfProcessors () / 2;
}

namespace VadDefaults {
	inline constexpr conststring32 speechThreshold = U"0.5";
	inline constexpr conststring32 minNonSpeechDuration = U"0.1";
	inline constexpr conststring32 minSpeechDuration = U"0.25";
	inline constexpr conststring32 speechPad = U"0.0";
	inline constexpr conststring32 nonSpeechLabel = U"";
	inline constexpr conststring32 speechLabel = U"speech";
}

namespace DiarizationDefaults {
	inline constexpr conststring32 maxNumSpeakers = U"2";
	inline constexpr bool allowOverlap = true;
	inline constexpr conststring32 nonSpeechLabel = U"";
	inline constexpr conststring32 speechLabel = U"speech";
	inline constexpr conststring32 clusterThreshold = U"0.7";
	inline constexpr conststring32 segmentationStep = U"0.1";
	inline const integer n_threads = MelderThread_getNumberOfProcessors () / 2;
}

struct SpeechSegment {
	autostring32 text;
	double tmin;
	double tmax;
};

struct WhisperTranscription {
	SpeechSegment fullTranscription;
	autovector <SpeechSegment> words;
	autovector <SpeechSegment> sentences;
};

inline std::set <structSpeechRecognizer *> theLivingSpeechRecognizers;
#include "SpeechRecognizer_def.h"

/*
	Functions to access lists of models and languages from everywhere.
*/
constSTRVEC theCurrentSpeechRecognizerModelNames ();
constSTRVEC theSpeechRecognizerLanguageNames ();

/*
	Class SpeechRecognizer functions.
*/
autoSpeechRecognizer SpeechRecognizer_create (conststring32 modelName, conststring32 languageName);
WhisperTranscription SpeechRecognizer_recognize (constSpeechRecognizer me, constSound sound, bool useVad,
		double speechProbabilityThreshold, double minNonSpeechDuration, double minSpeechDuration, double speechPad);

/*
	Silero-VAD functions.
*/
autovector <SpeechSegment> doSileroVad (constSound sound, double speechProbabilityThreshold, double minNonSpeechDuration,
		double minSpeechDuration, double speechPad, conststring32 nonSpeechLabel, conststring32 speechLabel);

/*
	Diarization functions.
*/
/*
	Run speaker diarization on Sound. Return a vector of timelines, one timeline per speaker.
	Each timeline covers [sound->xmin, sound->xmax] as alternating silent/active intervals,
	labeled `nonSpeechLabel` and `speechLabel` respectively.
*/
autovector <autovector <SpeechSegment>> doDiarization (constSound sound,
	integer maxNumSpeakers, bool allowSpeakersOverlap,
	double clusterThreshold, double segmentationStep,
	conststring32 nonSpeechLabel, conststring32 speechLabel);

/*
	Preferences for AI settings (maximum number of threads for transcription & diarization).
*/
void SpeechRecognizer_preferences ();

void SpeechRecognizer_setMaxNumberOfThreadsForTranscription (integer numberOfThreads);
void SpeechRecognizer_setMaxNumberOfThreadsForDiarization (integer numberOfThreads);

integer SpeechRecognizer_getMaxNumberOfThreadsForTranscription ();
integer SpeechRecognizer_getMaxNumberOfThreadsForDiarization ();

/* End of file SpeechRecognizer.h */
#endif
