/* SpeechRecognizer.cpp
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

#include "SpeechRecognizer.h"
#include "Sound.h"
#include "whisper.h"
#include "diarize.h"
#include "melder.h"
#include "ggml-memory-pool.h"
#include "ggml-silero-vad-model-data.h"
#include "Preferences.h"

#include "oo_DESTROY.h"
#include "SpeechRecognizer_def.h"
#include "oo_COPY.h"
#include "SpeechRecognizer_def.h"
#include "oo_EQUAL.h"
#include "SpeechRecognizer_def.h"
#include "oo_CAN_WRITE_AS_ENCODING.h"
#include "SpeechRecognizer_def.h"
#include "oo_WRITE_TEXT.h"
#include "SpeechRecognizer_def.h"
#include "oo_WRITE_BINARY.h"
#include "SpeechRecognizer_def.h"
#include "oo_READ_TEXT.h"
#include "SpeechRecognizer_def.h"
#include "oo_READ_BINARY.h"
#include "SpeechRecognizer_def.h"
#include "oo_DESCRIPTION.h"
#include "SpeechRecognizer_def.h"

autoWhisperContext :: ~autoWhisperContext () {
	//TRACE
	trace (U"Destroying whisper context at ", Melder_pointer (ptr));
	if (ptr)
		whisper_free (ptr);
	trace (U"Number of allocations in the memory pool is  ", theGgmlMemoryPool. n_allocations());
	trace (U"Total memory in bytes is  ", theGgmlMemoryPool. sizeInBytes());
}
autoWhisperContext& autoWhisperContext :: operator= (autoWhisperContext&& other) noexcept {
	if (this != & other) {
		whisper_free (ptr);
		ptr = other.ptr;
		other.ptr = nullptr;
	}
	return *this;
}

autoWhisperVadContext :: ~autoWhisperVadContext () {
	//TRACE
	trace (U"Destroying Silero-VAD context at ", Melder_pointer (ptr));
	whisper_vad_free (ptr);
	trace (U"Number of allocations in the memory pool is  ", theGgmlMemoryPool. n_allocations());
	trace (U"Total memory in bytes is  ", theGgmlMemoryPool. sizeInBytes());
}
autoWhisperVadContext& autoWhisperVadContext :: operator= (autoWhisperVadContext&& other) noexcept {
	if (this != & other) {
		whisper_vad_free (ptr);
		ptr = other.ptr;
		other.ptr = nullptr;
	}
	return *this;
}

autoWhisperVadSegments :: ~autoWhisperVadSegments () {
	//TRACE
	trace (U"Destroying Silero-VAD segments at ", Melder_pointer (ptr));
	whisper_vad_free_segments (ptr);
}
autoWhisperVadSegments& autoWhisperVadSegments :: operator= (autoWhisperVadSegments&& other) noexcept {
	if (this != & other) {
		whisper_vad_free_segments (ptr);
		ptr = other.ptr;
		other.ptr = nullptr;
	}
	return *this;
}

autoDiarizeContext :: ~autoDiarizeContext () {
	//TRACE
	trace (U"Destroying diarize context at ", Melder_pointer (ptr));
	diarize_free (ptr);
	trace (U"Number of allocations in the memory pool is  ", theGgmlMemoryPool. n_allocations());
	trace (U"Total memory in bytes is  ", theGgmlMemoryPool. sizeInBytes());
}
autoDiarizeContext& autoDiarizeContext :: operator= (autoDiarizeContext&& other) noexcept {
	if (this != & other) {
		diarize_free (ptr);
		ptr = other.ptr;
		other.ptr = nullptr;
	}
	return *this;
}

Thing_implement (SpeechRecognizer, Daata, 0);

void structSpeechRecognizer :: v1_info () {
	SpeechRecognizer_Parent :: v1_info ();
	MelderInfo_writeLine (U"Model: ", our d_modelName.get());
	MelderInfo_writeLine (U"Language: ", our d_languageName.get());
}

static void whisper_log_silent (ggml_log_level level, const char *text, void *user_data) {
	(void) level;
	(void) text;
	(void) user_data;
}

static void supressGgmlLogging () {
	if (Melder_debug == 2001)
		whisper_log_set (nullptr, nullptr);
	else
		whisper_log_set (whisper_log_silent, nullptr);
}

static conststring32 theWhisperModelsFolder ();

autoSpeechRecognizer SpeechRecognizer_create (const conststring32 modelName, const conststring32 languageName) {
	try {
		autoSpeechRecognizer me = Thing_new (SpeechRecognizer);
		my d_modelName = Melder_dup (modelName);
		my d_languageName = Melder_dup (languageName);

		/*
			Check if selected model and language are compatible.
		*/
		if (str32str (modelName, U".en.bin")) {
			Melder_require (str32str (languageName, U"Autodetect") || str32str (languageName, U"English"),
					U"Model ", modelName, U" cannot be used for ", languageName, U" transcription. "
					U"Either select a multilingual model (the model name does not include .en) "
					U"or select \"Autodetect language\"/\"English\" from the language list."
			);
		}

		/*
			Create Whisper context.
		*/
		whisper_context_params contextParams = whisper_context_default_params  ();
		contextParams. use_gpu = false;
		contextParams. flash_attn = false;   // needs to be false to use DTW!!!

		/*
			Enable DTW (Dynamic Time Warping algorithm used for more precise token boundaries).
		*/
		Melder_assert (! contextParams. flash_attn);
		contextParams. dtw_token_timestamps = true;
		if (str32str (modelName, U"tiny.en"))
			contextParams. dtw_aheads_preset = WHISPER_AHEADS_TINY_EN;
		else if (str32str (modelName, U"tiny"))
			contextParams. dtw_aheads_preset = WHISPER_AHEADS_TINY;
		else if (str32str (modelName, U"base.en"))
			contextParams. dtw_aheads_preset = WHISPER_AHEADS_BASE_EN;
		else if (str32str (modelName, U"base"))
			contextParams. dtw_aheads_preset = WHISPER_AHEADS_BASE;
		else if (str32str (modelName, U"small.en"))
			contextParams. dtw_aheads_preset = WHISPER_AHEADS_SMALL_EN;
		else if (str32str (modelName, U"small"))
			contextParams. dtw_aheads_preset = WHISPER_AHEADS_SMALL;
		else if (str32str (modelName, U"medium.en"))
			contextParams. dtw_aheads_preset = WHISPER_AHEADS_MEDIUM_EN;
		else if (str32str (modelName, U"medium"))
			contextParams. dtw_aheads_preset = WHISPER_AHEADS_MEDIUM;
		else if (str32str (modelName, U"large-v3-turbo") || str32str (modelName, U"turbo"))
			contextParams. dtw_aheads_preset = WHISPER_AHEADS_LARGE_V3_TURBO;
		else if (str32str (modelName, U"large-v3"))
			contextParams. dtw_aheads_preset = WHISPER_AHEADS_LARGE_V3;
		else if (str32str (modelName, U"large-v2"))
			contextParams. dtw_aheads_preset = WHISPER_AHEADS_LARGE_V2;
		else if (str32str (modelName, U"large-v1") || str32str (modelName, U"large"))
			contextParams. dtw_aheads_preset = WHISPER_AHEADS_LARGE_V1;
		else
			contextParams. dtw_aheads_preset = WHISPER_AHEADS_N_TOP_MOST;

		const conststring32 modelPath = Melder_cat (theWhisperModelsFolder(), U"/", modelName);
		const conststring8 utf8ModelPath = Melder_peek32to8 (modelPath);

		/*
			Initialise Whisper model.
		*/
		supressGgmlLogging ();
		whisper_context *ctx = whisper_init_from_file_with_params (utf8ModelPath, contextParams);
		if (! ctx)
			Melder_throw (U"Cannot create Whisper context from: ", modelPath, U". Model file not found?");

		my whisperContext = autoWhisperContext (ctx);
		theLivingSpeechRecognizers. insert (me.get());
		return me;
	} catch (MelderError) {
		theGgmlMemoryPool. clear();
		Melder_throw (U"SpeechRecognizer not created.");
	}
}

static autovector <float> resampleForWhisper (constSound sound) {
	/*
		Resample the sound to 16kHz if needed.
	*/
	autoSound resampled = Sound_resample (sound, 16000.0, 50);
	sound = resampled.get();

	/*
		Convert the sound to float32 for Whisper.
	*/
	autovector <float> samples32 = newvectorzero <float> (sound -> nx);;
	for (integer i = 1; i <= sound -> nx; i ++)
		samples32 [i] = static_cast <float> (sound -> z [1] [i]);

	return samples32;
}

static void SpeechRecognizer_runWhisper (constSpeechRecognizer me, constSound sound, const bool useVad,
		const double speechProbabilityThreshold, const double minNonSpeechDuration, const double minSpeechDuration, const double speechPad) {
	//TRACE
	/*
		Prepare sound for Whispercpp.
	*/
	autovector <float> samples32 = resampleForWhisper (sound);

	/*
		Default maximum number of threads for transcription is half the number of logical processors. This is because:
		1. hyperthreaded CPUs: half = physical cores, optimal for SIMD kernels.
		2. hybrid P+E core CPUs: half is "mostly P-cores"; using all the cores can cause a 100x slowdown.
		3. homogeneous non-hyperthreaded CPUs: half is below optimal, but not disastrous as in case 2.
		The user can override this in Praat preferences.
	*/
	integer n_threads = SpeechRecognizer_getMaxNumberOfThreadsForTranscription ();
	if (n_threads <= 0)
		n_threads = TranscriptionDefaults::n_threads;

	/*
		Set Whisper parameters.
	*/
	whisper_sampling_strategy samplingStrategy =
			Melder_debug == 2002 ? WHISPER_SAMPLING_GREEDY : WHISPER_SAMPLING_BEAM_SEARCH;
	trace (U"Sampling strategy = ", samplingStrategy == WHISPER_SAMPLING_GREEDY ? U"greedy" : U"beam search");
	whisper_full_params params = whisper_full_default_params (samplingStrategy);
	params. token_timestamps = true;   // must be true to use t0 and t1 (non-DTW) token timestamps
	params. n_threads = static_cast<int32_t> (n_threads);

	if (useVad) {
		params. vad = true;   // enable Silero VAD (Voice Activity Detection used to chop away the silences)
		params. vad_model_data = ggml_silero_bin;   // set up either params.vad_model_data or params.vad_model_path
		params. vad_model_data_size = ggml_silero_bin_len;
		params. vad_params = whisper_vad_default_params();
		params. vad_params. threshold = static_cast <float> (speechProbabilityThreshold);
		params. vad_params. min_speech_duration_ms = static_cast <int> (minSpeechDuration * 1000.0);
		params. vad_params. min_silence_duration_ms = static_cast <int> (minNonSpeechDuration * 1000.0);
		params. vad_params. speech_pad_ms = static_cast <int> (speechPad * 1000.0);
		params. vad_params. samples_overlap = 0.1f;   // this is the default, but brought here to remember that it is configurable
	}
	if (whisper_is_multilingual (my whisperContext.get ())) {
		if (my d_languageName && ! str32str (my d_languageName.get(), U"Autodetect")) {
			autostring32 name = Melder_dup (my d_languageName.get());
			name [0] = Melder_toLowerCase (name [0]);   // e.g. "Dutch" -> "dutch"
			params.language = whisper_lang_str (whisper_lang_id (Melder_peek32to8 (name.get())));
		} else {
			params.language = "auto";
		}
	}

	/*
		Run Whisper.
	*/
	supressGgmlLogging ();
	try {
		if (whisper_full (my whisperContext.get(), params, samples32. asArgumentToFunctionThatExpectsZeroBasedArray(),
				static_cast <int> (samples32.size)) != 0)
			Melder_throw (U"Whisper failed to process audio");
		whisper_print_timings(my whisperContext.get());
	} catch (MelderError) {
		theGgmlMemoryPool. clear();
		Melder_throw (U"Whisper run out of memory. SpeechRecognizer objects are no longer usable and must be recreated.");
	}
}

static bool endsWithTerminalPunctuation(const conststring32 token) {
	if (! token || token [0] == U'\0')
		return false;

	const integer lengthOfToken = Melder_length (token);
	const char32 lastChar = token [lengthOfToken - 1];
	return lastChar == U'.' || lastChar == U'!' || lastChar == U'?' ||
			lastChar == U'。' || lastChar == U'！' || lastChar == U'？';
}

static bool isPurePunctuation(const conststring32 token) {
	if (! token || token [0] == U'\0')
		return false;

	const integer lengthOfToken = Melder_length (token);
	for (integer i = 0; i < lengthOfToken; i ++)
		if (Melder_isAlphanumeric (token [i]))
			return false;
	return true;
}

WhisperTranscription SpeechRecognizer_recognize (constSpeechRecognizer me, constSound sound, const bool useVad,
		const double speechProbabilityThreshold, const double minNonSpeechDuration, const double minSpeechDuration, const double speechPad) {
	try {
		TRACE
		Melder_require (my whisperContext.get(),
			U"This SpeechRecognizer object is not usable anymore as it ran out of memory. ",
			U"Please remove it and create a new one."
		);
		trace (U"Sound xmin = ", sound -> xmin, U", sound xmax = ", sound -> xmax);

		/*
			Run Whisper and control for blank audio.
		*/
		SpeechRecognizer_runWhisper (me, sound, useVad, speechProbabilityThreshold, minNonSpeechDuration, minSpeechDuration, speechPad);
		const int numberOfSegments = whisper_full_n_segments (my whisperContext.get());
		if (! numberOfSegments) {
			WhisperTranscription transcription;
			transcription. fullTranscription. text = Melder_dup (U"[BLANK_AUDIO]");
			transcription. fullTranscription. tmin = sound -> xmin;
			transcription. fullTranscription. tmax = sound -> xmax;
			transcription. words = newvectorzero <SpeechSegment> (1);
			transcription. words [1]. text = Melder_dup (U"BLANKAUDIO");
			transcription. words [1]. tmin = sound -> xmin;
			transcription. words [1]. tmax = sound -> xmax;
			transcription. sentences = newvectorzero <SpeechSegment> (1);
			transcription. sentences [1]. text = Melder_dup (U"[BLANK_AUDIO]");
			transcription. sentences [1]. tmin = sound -> xmin;
			transcription. sentences [1]. tmax = sound -> xmax;
			return transcription;
		}

		const int numberOfVadSegments = whisper_full_n_vad_segments (my whisperContext.get());
		struct VadSegment {
			double origStart;
			double origEnd;
			double vadStart;
			double vadEnd;
		};
		autovector <VadSegment> vadSegments = newvectorzero <VadSegment> (numberOfVadSegments);
		for (integer i = 1; i <= numberOfVadSegments; i ++) {
			vadSegments [i]. origStart = std::min (
					static_cast <double> (whisper_full_get_vad_segment_orig_start (my whisperContext.get(), i - 1)) / 100.0,
					sound -> xmax
				);
			vadSegments [i]. origEnd = std::min (
					static_cast <double> (whisper_full_get_vad_segment_orig_end (my whisperContext.get(), i - 1)) / 100.0,
					sound -> xmax
				);
			vadSegments [i]. vadStart  =
					static_cast <double> (whisper_full_get_vad_segment_vad_start (my whisperContext.get(), i - 1)) / 100.0;
			vadSegments [i]. vadEnd  =
					static_cast <double> (whisper_full_get_vad_segment_vad_end (my whisperContext.get(), i - 1)) / 100.0;
			trace (U"VAD segment ", i, U", orig_start = ", vadSegments [i]. origStart, U", orig_end = ", vadSegments [i]. origEnd,
					U", vad_start = ", vadSegments [i]. vadStart, U", vad_end = ", vadSegments [i]. vadEnd);
		}

		/*
			Lambda for VAD time to real time conversion.
		*/
		auto vadToRealTime = [& vadSegments, useVad] (double vadTime) {
			if (! useVad || ! vadSegments.size)
				return vadTime;   // no VAD: VAD time is the real time

			/* mutable scan */ integer k = 1;
			while (k < vadSegments.size && vadSegments [k + 1]. vadStart <= vadTime)
				k ++;
			return vadTime + vadSegments [k]. origStart - vadSegments [k]. vadStart;
		};

		/*
			Lambda for finding the end of the VAD segment (in VAD time) that contains vadTime.
			Note that vadSegments [k]. vadEnd is not used. This is because whisper.cpp pads each VAD segment
			with 0.1 seconds of context on the right side, and vadEnd points to the end of that padding.
			Whisper.cpp additionally adds 0.1 seconds of zeroes after each padded VAD segment,
			resulting in 0.2 seconds of "non-speech" between VAD segments.
			A vadTime in these 0.2 seconds after a VAD segment is also considered belonging to this segment.
		*/
		auto vadSegmentEnd = [& vadSegments] (double vadTime) {
			Melder_assert (vadTime >= vadSegments [1]. vadStart);
			/* mutable scan */ integer k = 1;
			while (k < vadSegments.size && vadSegments [k + 1]. vadStart <= vadTime)
				k ++;
			return vadSegments [k]. vadStart + (vadSegments [k]. origEnd - vadSegments [k]. origStart);
		};

		/*
			Collect all the tokens into a flat list, repairing incomplete UTF8 tokens.
		*/
		struct RawToken {
			autostring32 text;   // leading space intact
			double tmax;
			integer whisperSegment;
		};
		autovector <RawToken> rawTokens = newvectorzero <RawToken> (0);

		/* mutable accumulate */ std::string partialTokenText;   // here we will accumulate whisper token texts until it is a proper UTF8 string
		const whisper_token firstSpecialTokenId = whisper_token_eot (my whisperContext.get());   // see struct whisper_vocab in whisper.cpp

		for (integer i = 0; i < numberOfSegments; i ++) {
			/* mutable flag for tracing */ bool isFirstPartialTokenInSegment = true;
			for (integer j = 0; j < whisper_full_n_tokens (my whisperContext.get(), static_cast<int> (i)); j ++) {
				const whisper_token_data tokenData = whisper_full_get_token_data (
						my whisperContext.get(),  static_cast<int> (i),  static_cast<int> (j));
				if (tokenData. id >= firstSpecialTokenId) {
					trace (U"Skipping special token: ", tokenData. id);
					continue;   // skip special tokens
				}

				/*
					Clamp the tail of the previous segment.
				*/
				if (isFirstPartialTokenInSegment) {
					if (rawTokens.size >= 1) {
						/* mutable search */ integer lastPreserved = rawTokens.size;
						/*
							Find last segment's tokens to keep before clamping by the beginning of the current Whisper segment.
						*/
						const integer previousSegment = rawTokens [rawTokens.size]. whisperSegment;
						/* mutable, may be lowered */ double border = std::min (tokenData. t0 / 100.0, tokenData. t_dtw / 100.0);
						while (lastPreserved >= 1 && rawTokens [lastPreserved]. whisperSegment == previousSegment
								&& rawTokens [lastPreserved]. tmax > border)
							lastPreserved --;
						/*
							Also apply Inter-VAD segment clamping: the tail of the previous Whisper segment must stay
							in the VAD segment where the preserved part of the tail ends.
						*/
						if (useVad && vadSegments.size >= 1 && lastPreserved >= 1 && lastPreserved < rawTokens.size) {
							const double ceiling = vadSegmentEnd (rawTokens [lastPreserved]. tmax);
							if (ceiling < border) {
								border = ceiling;
								while (lastPreserved >= 1 && rawTokens [lastPreserved]. whisperSegment == previousSegment
										&& rawTokens [lastPreserved]. tmax > border)
									lastPreserved --;
							}
						}
						if (lastPreserved < rawTokens.size)   // there is something to clamp
							trace (U"Clamping ", rawTokens.size - lastPreserved, U" tail token(s) of segment ", previousSegment, U" to ", vadToRealTime (border));
						for (integer m = lastPreserved + 1; m <= rawTokens.size; m ++)
							rawTokens [m]. tmax = border;   // actual clamping
					}
					trace (U"Segment ", i, U" first-token t0 = ", vadToRealTime (tokenData. t0 / 100.0),
						U", segment_t0 = ", whisper_full_get_segment_t0 (my whisperContext.get(), i) / 100.0,
						U", segment_t1 = ", whisper_full_get_segment_t1 (my whisperContext.get(), i) / 100.0
					);
					isFirstPartialTokenInSegment = false;
				}

				partialTokenText += whisper_full_get_token_text (my whisperContext.get(), i, j);
				const double currentTokenTmax = tokenData. t_dtw / 100.0;

				if (! Melder_str8IsValidUtf8 (partialTokenText.c_str()))
					continue;   // continue accumulating token texts until partialTokenText is a proper UTF8 string

				/*
					Now partialTokenText is valid UTF8.
				*/
				RawToken *rawToken = rawTokens. append();
				rawToken -> text = Melder_8to32_e (partialTokenText.c_str());
				rawToken -> tmax = currentTokenTmax;
				rawToken -> whisperSegment = i;
				partialTokenText. clear();

				trace (U"Segment ", i, U"; original token in it ", j, U": text = \"", rawToken -> text.get(),
					U"\", t_dtw = ", vadToRealTime (currentTokenTmax),
					U"\", t0 = ", vadToRealTime (tokenData. t0 / 100.0),
					U"\", t1 = ", vadToRealTime (tokenData. t1 / 100.0)
				);
			}
		}

		/*
			Group Whisper tokens into words.
		*/
		struct WhisperToken {
			autostring32 text;
			double tmax;
		};

		struct WhisperWord {
			autovector <WhisperToken> whisperTokens = newvectorzero <WhisperToken> (0);
			autostring32 textWithPunctuation;
			double tmax;
		};

		autovector <WhisperWord> whisperWords = newvectorzero <WhisperWord> (0);
		for (integer i = 1; i <= rawTokens.size; i ++) {
			RawToken& rawToken = rawTokens [i];

			/*
				Create a new WhisperWord if
				- the current token starts with a space or if autovector <WhisperWord> whisperWords is still empty
				- and the previous word is not a standalone "-".
				Otherwise, append the current token to the existing word.
			*/
			const bool isNewWord = rawToken. text.get() [0] == U' ';
			const bool isLastWordDash = whisperWords.size >= 1 &&
					Melder_length (whisperWords [whisperWords.size]. textWithPunctuation.get()) == 1 &&
					whisperWords [whisperWords.size]. textWithPunctuation.get() [0] == U'-';

			WhisperWord *word;
			if ((isNewWord || whisperWords.size == 0) && ! isLastWordDash)
				word = whisperWords. append();
			else
				word = &whisperWords [whisperWords.size];
			WhisperToken *token = word -> whisperTokens.append();

			token -> text = Melder_dup (rawToken. text.get() + (isNewWord ? 1 : 0));
			word -> textWithPunctuation = Melder_dup (Melder_cat (word -> textWithPunctuation.get(), token -> text.get()));
			token -> tmax = rawToken. tmax;
			if (word -> whisperTokens.size >= 2 && isPurePunctuation (token -> text.get())
					&& token -> text.get() [0] != U')' && token -> text.get() [0] != U']')
				word -> tmax = (word -> tmax + token -> tmax) / 2;
			else
				word -> tmax = token -> tmax;
		}

		/*
			Resolve zero-length words. There might be runs of consecutive words sharing the same tmax (they can be results
			of clamping or just poor DTW). Distribute them equally over the time from the preceding word's tmax till their tmax,
			so that there are no zero-length words anymore.
		*/
		/* mutable scan */ integer iword = 1;
		while (iword <= whisperWords.size) {
			/* mutable scan */ integer k = iword + 1;
			while (k <= whisperWords.size && whisperWords [k]. tmax == whisperWords [iword]. tmax)
				k ++;
			const integer n = k - iword;
			if (n >= 2) {
				const double prevTmax = ( iword > 1 ? whisperWords [iword - 1]. tmax : 0.0 );
				const double interval = (whisperWords [iword]. tmax - prevTmax) / n;
				for (integer l = 1; l < n; l ++)
					whisperWords [iword + l - 1]. tmax = prevTmax + interval * l;
			}
			iword = k;
		}

		/*
			Build the list of word intervals (including silences) with VAD-adjustments if VAD is in use.
		*/
		struct WordInterval {
			autostring32 textWithPunctuation;
			double tmax;
		};
		autovector <WordInterval> wordIntervals;

		if (! useVad || ! numberOfVadSegments) {
			/*
				VAD is not used or no VAD segments detected: no VAD adjustments needed.
			*/
			wordIntervals = newvectorzero <WordInterval> (whisperWords.size);
			for (integer i = 1; i <= whisperWords.size; i ++) {
				wordIntervals [i]. textWithPunctuation = whisperWords [i]. textWithPunctuation.move();
				wordIntervals [i]. tmax = whisperWords [i]. tmax;
			}
		} else {
			/*
				VAD is used and at least one VAD segment detected: translate timestamps back to original time
				and insert silence tokens between consecutive VAD segments.
			*/
			wordIntervals = newvectorzero <WordInterval> (0);

			/*
				Lambda to append a silent interval with the given tmax to wordIntervals.
			*/
			auto appendSilence = [& wordIntervals] (double tmax) {
				WordInterval *silence;
				if (wordIntervals.size >= 1 && wordIntervals [wordIntervals.size]. textWithPunctuation [0] == U'\0') {
					silence = &wordIntervals [wordIntervals.size];
					silence -> tmax = tmax;   // extend the last silence to the end of this one
				} else {
					silence = wordIntervals. append();
					silence -> textWithPunctuation = Melder_dup (U"");
					silence -> tmax = tmax;
				}
				trace (U"Silence ", wordIntervals.size, U": text = ", silence -> textWithPunctuation.get(), U", tmax = ", silence -> tmax);
			};

			/*
				Lambda to append a word to wordIntervals, with the given tmax, which is translated from VAD to original timestamps.
				Note: this might move the text member out of word, leaving it empty.
			*/
			auto appendWord = [& wordIntervals, & vadSegments, & appendSilence] (WhisperWord& word, integer vadSegment) {
				Melder_assert (word. tmax >= vadSegments [vadSegment]. vadStart);
				const double tmax = std::min (
					word. tmax - vadSegments [vadSegment]. vadStart + vadSegments [vadSegment]. origStart,   // original tmax
					vadSegments [vadSegment]. origEnd
				);   // clamp to the size of VAD interval

				const conststring32 text = word. textWithPunctuation.get();
				if (text [0] == U'\0' || text [0] == U'(' || text [0] == U'[')
					appendSilence (tmax);   // insert (append) silence if the word is empty or is non-speech (e.g., "(laughs)")
				else {
					WordInterval *wordInterval = wordIntervals. append();
					wordInterval -> textWithPunctuation = word. textWithPunctuation.move();
					wordInterval -> tmax = tmax;
					trace (U"Word interval ", wordIntervals.size, U": text = ", wordInterval -> textWithPunctuation.get(), U", tmax = ", wordInterval -> tmax);
				}
			};

			/*
				First, append a silence token in case speech does not start directly from the beginning of the sound.
			*/
			if (vadSegments [1]. origStart > sound -> xmin)
				appendSilence (vadSegments [1]. origStart);

			/*
				Then iterate over the list of words, inserting silences between VAD segments.
			*/
			/* mutable increment */ integer currentVadSegment = 1;
			for (integer i = 1; i <= whisperWords.size; i ++) {
				WhisperWord& word = whisperWords [i];
				const integer numberOfTokensInWord = word. whisperTokens.size;
				const conststring32 lastToken = word. whisperTokens [numberOfTokensInWord]. text.get();
				/* mutable scan */ integer firstContentToken = 1;
				while (firstContentToken < numberOfTokensInWord && isPurePunctuation (word. whisperTokens [firstContentToken]. text.get()))
					firstContentToken ++;
				const double firstContentTokenTmax = word. whisperTokens [firstContentToken]. tmax;   // first token that is not a pure punctuation

				const bool landsInCurrentVadSegment = ( currentVadSegment == numberOfVadSegments ||
						firstContentTokenTmax < vadSegments [currentVadSegment + 1]. vadStart && Melder_isPunctuationOrSymbol (lastToken [0]) );

				if (! landsInCurrentVadSegment)
					while (currentVadSegment < numberOfVadSegments && word. tmax > vadSegments [currentVadSegment + 1]. vadStart) {
						appendSilence (vadSegments [currentVadSegment + 1]. origStart);   // insert silence between current and next VAD segments;
						currentVadSegment ++;
					}
				appendWord (word, currentVadSegment);
			}

			/*
				Insert trailing silence.
			*/
			if (wordIntervals.size > 0 && wordIntervals [wordIntervals.size]. tmax < sound -> xmax)
				appendSilence (sound -> xmax);
		}

		/*
			Build word and sentence intervals from the flat token list.
		*/
		autoMelderString sentenceText;
		autoMelderString fullText;
		autovector <SpeechSegment> words = newvectorzero <SpeechSegment> (0);
		autovector <SpeechSegment> sentences = newvectorzero <SpeechSegment> (0);

		/* mutable per-sentence */ double sentenceTmin = sound -> xmin;   // (re)set at each sentence start, default assignment is just a guard
		/* mutable per-token */ double previousWordTmax = sound -> xmin;   // default for the first token

		for (integer i = 1; i <= wordIntervals.size; i ++) {
			const double wordTmin = previousWordTmax;
			const double wordTmax = wordIntervals [i]. tmax;
			previousWordTmax = wordTmax;

			const conststring32 wordTextWithPunctuation = wordIntervals [i]. textWithPunctuation.get();
			const bool isEmptyWord = wordTextWithPunctuation [0] == U'\0';

			/*
				Remove all leading and trailing punctuation except ' ('cause, speakers').
			*/
			/* mutable adjust */ conststring32 clean = wordTextWithPunctuation;
			while (*clean != U'\0' && ! Melder_isAlphanumeric (*clean) && *clean != U'\'')
				++ clean;   // move beyond leading punctuation
			/* mutable adjust */ integer cleanLength = Melder_length (clean);
			while (cleanLength > 0 && ! Melder_isAlphanumeric (clean [cleanLength - 1]) && clean [cleanLength - 1] != U'\'')
				-- cleanLength;   // exclude trailing punctuation
			autostring32 wordTextWithoutPunctuation = Melder_ndup (clean, cleanLength);

			/*
				Add word text to the sentence and to the full transcription text (only for non-silent words to not multiply spaces).
			*/
			if (! isEmptyWord) {
				MelderString_append (& sentenceText, sentenceText.length > 0 ? U" " : U"", wordTextWithPunctuation);
				MelderString_append (& fullText, fullText.length > 0 ? U" " : U"", wordTextWithPunctuation);
			}

			/*
				Add a word segment or append a zero-length word to the last word segment.
			*/
			Melder_assert (wordTmax >= wordTmin);
			if (wordTmax > wordTmin) {   // not a zero-length word
				SpeechSegment *word = words. append();
				word -> text = wordTextWithoutPunctuation.move();
				word -> tmin = wordTmin;
				word -> tmax = wordTmax;
				trace (U"Word ", words.size, U": \"", word -> text.get(), U"\" [ ", word -> tmin, U" - ", word -> tmax, U" ]");
			} else if (wordTextWithoutPunctuation [0] != U'\0' && words.size > 0) {   // zero-length word: add its text to previous one
				SpeechSegment& lastWord = words [words.size];
				lastWord.text = Melder_dup (Melder_cat (lastWord.text.get(), U" ", wordTextWithoutPunctuation.get()));
				trace (U"Word ", words.size, U": \"", lastWord.text.get(), U"\" [ ", lastWord. tmin, U" - ", lastWord. tmax, U" ] (appended zero-length)");
			}

			/*
				Finalize sentence segment
				1. if the sentence is complete (ends with terminal punctuation);
				2. if the current word is the last word of the whole sound;
				3. if the current word does not contain text (non-speech interval).
			*/
			if (endsWithTerminalPunctuation (wordTextWithPunctuation) || i == wordIntervals.size || (isEmptyWord && sentenceText.length == 0)) {
				if (wordTmax > sentenceTmin) {
					SpeechSegment *sentence = sentences. append();
					sentence -> text = Melder_dup (sentenceText.string ? sentenceText.string : U"");
					sentence -> tmin = sentenceTmin;
					sentence -> tmax = wordTmax;
					trace (U"Sentence: ", sentences.size, U": \"", sentence -> text.get(), U"\" [ ", sentence -> tmin, U" - ", sentence -> tmax, U" ]");
					MelderString_empty (& sentenceText);
					sentenceTmin = wordTmax;
				} else if (sentences.size > 0) {
					if (sentenceText.length > 0) {
						SpeechSegment& lastSentence = sentences [sentences.size];
						lastSentence.text = Melder_dup (Melder_cat (lastSentence.text.get(), U" ", sentenceText.string));
					}
					MelderString_empty (& sentenceText);
				}
			}
		}

		WhisperTranscription transcription;
		transcription. fullTranscription. text = Melder_dup (fullText.string ? fullText.string : U"");
		transcription. fullTranscription. tmin = sound -> xmin;
		transcription. fullTranscription. tmax = sound -> xmax;
		transcription. words = words.move();
		transcription. sentences = sentences.move();

		trace (U"Full transcription:",
			U" [ ", transcription. fullTranscription. tmin, U" - ",
			transcription. fullTranscription. tmax, U" ]", transcription. fullTranscription. text.get()
		);

		return transcription;

	} catch (MelderError) {
		Melder_throw (U"Sound not transcribed.");
	}
}

autovector <SpeechSegment> doSileroVad (constSound sound, const double speechProbabilityThreshold, const double minNonSpeechDuration,
	const double minSpeechDuration, const double speechPad, const conststring32 nonSpeechLabel, const conststring32 speechLabel
) {
	//TRACE
	trace (U"Sound xmin = ", sound -> xmin, U", sound xmax = ", sound -> xmax);
	supressGgmlLogging ();

	/*
		Remember original sound -> xmin and sound -> xmax before resampling; and resample.
	*/
	const double soundStart = sound -> xmin;
	const double soundEnd = sound -> xmax;
	autovector <float> samples32 = resampleForWhisper (sound);

	try {
		/*
			Initialize VAD context.
		*/
		whisper_vad_context_params vad_ctx_params = whisper_vad_default_context_params ();
		autoWhisperVadContext vad_ctx = whisper_vad_init_from_memory_with_params (ggml_silero_bin, ggml_silero_bin_len, vad_ctx_params);
		if (! vad_ctx.get())
			Melder_throw (U"Failed to initialize VAD context.");

		/*
			Set VAD parameters and run Silero VAD.
		*/
		whisper_vad_params vad_params = whisper_vad_default_params ();
		vad_params. threshold = static_cast <float> (speechProbabilityThreshold);
		vad_params. min_speech_duration_ms = static_cast <int> (minSpeechDuration * 1000.0);
		vad_params. min_silence_duration_ms = static_cast <int> (minNonSpeechDuration * 1000.0);
		vad_params. speech_pad_ms = static_cast <int> (speechPad * 1000.0);
		autoWhisperVadSegments vad_segments = whisper_vad_segments_from_samples (vad_ctx.get(), vad_params,
			samples32.asArgumentToFunctionThatExpectsZeroBasedArray(), static_cast <int> (samples32.size));
		if (! vad_segments.get())
			Melder_throw (U"Failed to obtain VAD segments.");

		/*
			Collect all VAD segments and wrap them with "non-voice" intervals.
		*/
		const int numberOfVadSegments = whisper_vad_segments_n_segments (vad_segments.get());
		autovector <SpeechSegment> allIntervals = newvectorzero <SpeechSegment> (0);

		for (int i = 0; i < numberOfVadSegments; i ++) {
			const double t0 = whisper_vad_segments_get_segment_t0 (vad_segments.get(), i) / 100.0;
			const double t1 = whisper_vad_segments_get_segment_t1 (vad_segments.get(), i) / 100.0;

			/*
				Insert a non-voice interval before the first voice interval or between two voice intervals.
			*/
			const double previousEnd = (i == 0) ? soundStart : whisper_vad_segments_get_segment_t1 (vad_segments.get(), i - 1) / 100.0;
			if (t0 > previousEnd) {
				SpeechSegment *nonVoiceInterval = allIntervals. append ();
				nonVoiceInterval -> text = Melder_dup (nonSpeechLabel);
				nonVoiceInterval -> tmin = previousEnd;
				nonVoiceInterval -> tmax = t0;
				trace (U"Non-voice: t0 = ", allIntervals [allIntervals.size]. tmin, U", t1 = ", allIntervals [allIntervals.size]. tmax);
			}
			/*
				Insert the voice interval.
			*/
			SpeechSegment *voiceInterval = allIntervals. append();
			voiceInterval -> text = Melder_dup (speechLabel);
			voiceInterval -> tmin = t0;
			voiceInterval -> tmax = t1;
			trace (U"VAD segment ", i + 1, U": t0 = ", allIntervals [allIntervals.size]. tmin, U", t1 = ", allIntervals [allIntervals.size]. tmax);

			/*
				After the last voice interval, add non-voice if there is remaining audio.
			*/
			if (i == numberOfVadSegments - 1 && t1 < soundEnd) {
				SpeechSegment *nonVoiceInterval = allIntervals. append ();
				nonVoiceInterval -> text = Melder_dup (nonSpeechLabel);
				nonVoiceInterval -> tmin = t1;
				nonVoiceInterval -> tmax = soundEnd;
				trace (U"Non-voice: t0 = ", allIntervals [allIntervals.size]. tmin, U", t1 = ", allIntervals [allIntervals.size]. tmax);
			}
		}

		/*
			If no voice activity detected at all, then entire sound is non-voice.
		*/
		if (! numberOfVadSegments) {
			SpeechSegment *nonVoice = allIntervals. append ();
			nonVoice -> text = Melder_dup (nonSpeechLabel);
			nonVoice -> tmin = soundStart;
			nonVoice -> tmax = soundEnd;
		}

		return allIntervals;
	} catch (MelderError) {
		theGgmlMemoryPool. clear();   // this runs after destructors of local objects, therefore it must be safe!
		Melder_throw (U"Voice Activity not detected for sound.");
	}
}

extern unsigned char model_ggml_segmentation_data[];
extern unsigned int model_ggml_segmentation_length;
extern unsigned char model_ggml_embedding_data[];
extern unsigned int model_ggml_embedding_length;

autovector <autovector <SpeechSegment>> doDiarization (constSound sound,
	const integer maxNumSpeakers, const bool allowSpeakersOverlap,
	const double clusterThreshold, const double segmentationStep,
	const conststring32 nonSpeechLabel, const conststring32 speechLabel
) {
	//TRACE
	autovector <float> samples32 = resampleForWhisper (sound);
	try {
		/*
			Default maximum number of threads for diarization is half the number of logical processors. This is because:
			1. hyperthreaded CPUs: half = physical cores, optimal for SIMD kernels.
			2. hybrid P+E core CPUs: not tested yet; chosen by analogy with transcription.
			3. homogeneous non-hyperthreaded CPUs: not tested yet; half is likely below optimal but the slowdown is probably
			   smaller than the slowdown in case 1.
			The user can override this in Praat preferences.
		*/
		integer n_threads = SpeechRecognizer_getMaxNumberOfThreadsForDiarization ();
		if (n_threads <= 0)
			n_threads = DiarizationDefaults::n_threads;

		supressGgmlLogging ();
		autoDiarizeContext diarizeContext = diarize_init_from_memory (
			model_ggml_segmentation_data, model_ggml_segmentation_length,
			model_ggml_embedding_data, model_ggml_embedding_length
			);
		diarize_full_params diarizeParams = diarize_default_params ();
		diarizeParams. n_threads = static_cast<int> (n_threads);
		diarizeParams. max_speakers = static_cast <int> (maxNumSpeakers);
		diarizeParams. max_simultaneous_speakers = allowSpeakersOverlap ? 2 : 1;
		diarizeParams. cluster_threshold = static_cast <float> (clusterThreshold);
		diarizeParams. seg_step_ratio = static_cast <float> (segmentationStep);
		diarize_full (diarizeContext.get(), diarizeParams,samples32.asArgumentToFunctionThatExpectsZeroBasedArray(),
				static_cast <int> (samples32.size));
		const unsigned int numberOfSegments = diarize_full_n_segments (diarizeContext.get());
		const int numberOfSpeakers = diarize_full_n_speakers(diarizeContext.get());

		trace(U"Speakers:", numberOfSpeakers, U", Segments: ", numberOfSegments);

		autovector <autovector <SpeechSegment>> speakers = newvectorzero <autovector <SpeechSegment>> (numberOfSpeakers);
		/*
			Collect segments for each speaker.
		*/
		for (int i = 1; i <= numberOfSpeakers; i ++) {
			/* mutable increment */ double currentIntervalStart = sound -> xmin;

			for (int segment = 0; segment < numberOfSegments; segment ++) {
				if (diarize_full_get_segment_speaker (diarizeContext.get(), segment) + 1 != i)   // this segment is from a different speaker
					continue;

				const double tmin = diarize_full_get_segment_t0 (diarizeContext.get(), segment);
				const double tmax = diarize_full_get_segment_t1 (diarizeContext.get(), segment);

				if (tmin > currentIntervalStart) {
					SpeechSegment *gap = speakers [i]. append();
					gap -> text = Melder_dup (nonSpeechLabel);
					gap -> tmin = currentIntervalStart;
					gap -> tmax = tmin;
				}

				SpeechSegment *speakerSegment = speakers [i]. append();
				speakerSegment -> text = Melder_dup (speechLabel);
				speakerSegment -> tmin = tmin;
				speakerSegment -> tmax = tmax;

				trace (U"Diarization: [ ", speakerSegment -> tmin, U" - ", speakerSegment -> tmax, U" ] \"",
						speakerSegment -> text.get(), U"\"");

				currentIntervalStart = tmax;
			}

			if (currentIntervalStart < sound -> xmax) {
				SpeechSegment *gap = speakers [i].append();
				gap -> text = Melder_dup (nonSpeechLabel);
				gap -> tmin = currentIntervalStart;
				gap -> tmax = sound -> xmax;
			}
		}

		return speakers;

	} catch (MelderError) {
		theGgmlMemoryPool. clear();
		Melder_throw (U"Diarization failed.");
	}
}

static conststring32 theWhisperModelsFolder () {
	static autostring32 whisperModelFolderPath;
	if (! whisperModelFolderPath) {
		try {
			structMelderFolder modelsFolder { };
			MelderFolder_getSubfolder (Melder_preferencesFolder (),	U"models", & modelsFolder);
			if (! MelderFolder_exists (& modelsFolder))
				MelderFolder_create (& modelsFolder);
			structMelderFolder whispercppFolder { };
			MelderFolder_getSubfolder (& modelsFolder,	U"whispercpp", & whispercppFolder);
			if (! MelderFolder_exists (& whispercppFolder))
				MelderFolder_create (& whispercppFolder);
			whisperModelFolderPath = Melder_dup (MelderFolder_peekPath (& whispercppFolder));
			if (Melder_debug == 2001)
				Melder_casual (U"Whispercpp model folder path: ", whisperModelFolderPath.get());
		} catch (MelderError) {
			Melder_clearError ();
		}
	}
	return whisperModelFolderPath.get();
}

constSTRVEC theCurrentSpeechRecognizerModelNames () {
	static autoSTRVEC whisperModelNames;
	try {
		whisperModelNames = fileNames_STRVEC (Melder_cat (theWhisperModelsFolder (), U"/*.bin"));
	} catch (MelderError) {
		Melder_clearError ();
	}
	return whisperModelNames.get();
}

constSTRVEC theSpeechRecognizerLanguageNames () {
	static autoSTRVEC sortedWhisperLanguageNames;
	if (! sortedWhisperLanguageNames) {
		autoSTRVEC unsorted;
		try {
			const uint8 numberOfLanguages = whisper_lang_max_id ();
			for (uint8 i = 0; i < numberOfLanguages; i ++) {
				autostring32 languageName = Melder_8to32_e (whisper_lang_str_full (i));

				/*
					Capitalize the first letter, e.g. "dutch" -> "Dutch".
				*/
				if (languageName [0] != U'\0')
					languageName [0] = Melder_toUpperCase (languageName [0]);

				unsorted. append (languageName.get());
			}
			autoSTRVEC sorted = sort_STRVEC (unsorted.get());

			/*
				Create a list to return (with autodetect option).
			*/
			sortedWhisperLanguageNames. append (U"Autodetect language");
			for (integer i = 1; i <= sorted.size; i ++)
				sortedWhisperLanguageNames. append (sorted [i].get());

		} catch (MelderError) {
			unsorted. reset();
			sortedWhisperLanguageNames. reset();
			Melder_clearError ();
		}
	}
	return sortedWhisperLanguageNames.get();
}

/*
	Preferences.
*/
static struct {
	integer maxNumberOfThreadsForTranscription = 0;   // "0" signals automatic (MelderThread_getNumberOfProcessors () / 2)
	integer maxNumberOfThreadsForDiarization = 0;   // "0" signals automatic (MelderThread_getNumberOfProcessors () / 2)
} preferences;

void SpeechRecognizer_preferences () {
	Preferences_addInteger (U"SpeechRecognizer.maxNumberOfThreadsForTranscription",
		& preferences. maxNumberOfThreadsForTranscription, 0);
	Preferences_addInteger (U"SpeechRecognizer.maxNumberOfThreadsForDiarization",
			& preferences. maxNumberOfThreadsForDiarization, 0);
}

void SpeechRecognizer_setMaxNumberOfThreadsForTranscription (integer numberOfThreads) {
	preferences. maxNumberOfThreadsForTranscription = numberOfThreads;
}
void SpeechRecognizer_setMaxNumberOfThreadsForDiarization (integer numberOfThreads) {
	preferences. maxNumberOfThreadsForDiarization = numberOfThreads;
}

integer SpeechRecognizer_getMaxNumberOfThreadsForTranscription () {
	return preferences. maxNumberOfThreadsForTranscription;
}
integer SpeechRecognizer_getMaxNumberOfThreadsForDiarization () {
	return preferences. maxNumberOfThreadsForDiarization;
}

/* End of file SpeechRecognizer.cpp */
