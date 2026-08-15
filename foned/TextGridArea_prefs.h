/* TextGridArea_prefs.h
 *
 * Copyright (C) 2013,2015,2016,2019,2022 Paul Boersma
 *
 * This code is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
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

Prefs_begin (TextGridArea)

	ClassPrefs_overrideBool       (TextGridArea, picture_garnish,          1, true)

	InstancePrefs_addBool         (TextGridArea, useTextStyles,            1, false)
	InstancePrefs_addDouble       (TextGridArea, fontSize,                 1, U"18")
	InstancePrefs_addEnum         (TextGridArea, alignment,                1, kGraphics_horizontalAlignment, DEFAULT)
	InstancePrefs_addBool         (TextGridArea, shiftDragMultiple,        1, true)
	InstancePrefs_addEnum         (TextGridArea, showNumberOf,             1, kTextGridArea_showNumberOf, DEFAULT)
	InstancePrefs_addEnum         (TextGridArea, greenMethod,              1, kMelder_string, DEFAULT)
	InstancePrefs_addString       (TextGridArea, greenString,              1, U"some text here for green paint")
	ClassPrefs_addBool            (TextGridArea, picture_showBoundaries,   1, true)
	ClassPrefs_addBool            (TextGridArea, picture_pitch_speckle,    1, false)
	InstancePrefs_addString       (TextGridArea, align_language,           1, U"English")
	InstancePrefs_addBool         (TextGridArea, align_includeWords,       1, true)
	InstancePrefs_addBool         (TextGridArea, align_includePhonemes,    1, false)
	InstancePrefs_addBool         (TextGridArea, align_allowSilences,      1, false)
	InstancePrefs_addString       (TextGridArea, transcribe_model,         1, TranscriptionDefaults::modelName)
	InstancePrefs_addString       (TextGridArea, transcribe_language,      1, TranscriptionDefaults::languageName)
	InstancePrefs_addBool		  (TextGridArea, transcribe_includeWords,  1, TranscriptionDefaults::includeWords)
	InstancePrefs_addBool		  (TextGridArea, transcribe_diarize,       1, TranscriptionDefaults::includeDiarization)
	InstancePrefs_addBool		  (TextGridArea, transcribe_useVad,		   1, TranscriptionDefaults::useVad)
	InstancePrefs_addDouble       (TextGridArea, vad_speechThreshold,      1, VadDefaults::speechThreshold)
	InstancePrefs_addDouble       (TextGridArea, vad_minNonSpeech,         1, VadDefaults::minNonSpeechDuration)
	InstancePrefs_addDouble       (TextGridArea, vad_minSpeech,            1, VadDefaults::minSpeechDuration)
	InstancePrefs_addDouble       (TextGridArea, vad_speechPadding,        1, VadDefaults::speechPad)
	InstancePrefs_addInteger      (TextGridArea, diarize_maxNumSpeakers,   1, DiarizationDefaults::maxNumSpeakers)
	InstancePrefs_addBool         (TextGridArea, diarize_allowOverlap,     1, DiarizationDefaults::allowOverlap)
	InstancePrefs_addString		  (TextGridArea, diarize_nonSpeechLabel,   1, DiarizationDefaults::nonSpeechLabel)
	InstancePrefs_addString		  (TextGridArea, diarize_speechLabel,      1, DiarizationDefaults::speechLabel)
	InstancePrefs_addDouble       (TextGridArea, diarize_clusterThreshold, 1, DiarizationDefaults::clusterThreshold)
	InstancePrefs_addDouble       (TextGridArea, diarize_segmentationStep, 1, DiarizationDefaults::segmentationStep)

Prefs_end (TextGridArea)

/* End of file TextGridArea_prefs.h */
