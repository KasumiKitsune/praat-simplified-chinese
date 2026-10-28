/* EditorAppearance.cpp
 *
 * Copyright (C) 2026 yjzxkxdn, 2026 Paul Boersma
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

#include "EditorAppearance.h"

namespace {
	/*
		The shared functional colours. Both spectrogram schemes use them unchanged, so switching the
		main spectrogram colour never changes what a pulse, a pitch point or a formant dot means.
		Every value is a design constant of this appearance scheme, not of any analysis setting.
	*/
	MelderColour rgb8 (integer red, integer green, integer blue) {
		return MelderColour (red / 255.0, green / 255.0, blue / 255.0);
	}

	const EditorAppearanceMarking theMistBlueGreyMarking = {
		kEditorAppearancePalette::MIST_BLUE_GREY,
		Spectrogram_monoColourMapDarkest (kSpectrogram_colourMap::MIST_BLUE_GREY),   // #71889B against white
		rgb8 (0x4F, 0x60, 0x54), rgb8 (0xFF, 0xFF, 0xFF),   // pulses: graphite green on white halo
		rgb8 (0x75, 0x61, 0x3F), rgb8 (0xFF, 0xFF, 0xFF), rgb8 (0x75, 0x61, 0x3F),   // pitch: gold brown
		rgb8 (0x31, 0x5F, 0x5B), rgb8 (0xFF, 0xFF, 0xFF),   // formants: teal on white edge
		rgb8 (0x59, 0x61, 0x6B), rgb8 (0x59, 0x61, 0x6B),   // intensity: graphite grey
		rgb8 (0x3F, 0x46, 0x4D),   // waveform
		rgb8 (0x4A, 0x50, 0x56),   // cursor: neutral graphite
		rgb8 (0xFF, 0xFF, 0xFF),   // canvas
		rgb8 (0x34, 0x3A, 0x40), rgb8 (0x73, 0x7B, 0x82),   // coordinate text, auxiliary ticks
		rgb8 (0xE5, 0xE8, 0xEA), rgb8 (0xD5, 0xD9, 0xDC),   // zero line and grid, selection fill
		1.8, 3.2,   // pulse core and halo widths
		2.8, 5.2,   // pitch core and halo widths
		1.2,   // intensity line width
		6.4 / 4.4,   // formant halo diameter over the user's own dot diameter
	};
	const EditorAppearanceMarking theClayGreyMarking = {
		kEditorAppearancePalette::CLAY_GREY,
		Spectrogram_monoColourMapDarkest (kSpectrogram_colourMap::CLAY_GREY),   // #9F7E71 against white
		rgb8 (0x4F, 0x60, 0x54), rgb8 (0xFF, 0xFF, 0xFF),
		rgb8 (0x75, 0x61, 0x3F), rgb8 (0xFF, 0xFF, 0xFF), rgb8 (0x75, 0x61, 0x3F),
		rgb8 (0x31, 0x5F, 0x5B), rgb8 (0xFF, 0xFF, 0xFF),
		rgb8 (0x59, 0x61, 0x6B), rgb8 (0x59, 0x61, 0x6B),
		rgb8 (0x3F, 0x46, 0x4D),
		rgb8 (0x4A, 0x50, 0x56),
		rgb8 (0xFF, 0xFF, 0xFF),
		rgb8 (0x34, 0x3A, 0x40), rgb8 (0x73, 0x7B, 0x82),
		rgb8 (0xE5, 0xE8, 0xEA), rgb8 (0xD5, 0xD9, 0xDC),
		1.8, 3.2,
		2.8, 5.2,
		1.2,
		6.4 / 4.4,
	};
}

const EditorAppearanceMarking * EditorAppearance_marking (kEditorAppearancePalette palette) {
	if (palette == kEditorAppearancePalette::MIST_BLUE_GREY)
		return & theMistBlueGreyMarking;
	if (palette == kEditorAppearancePalette::CLAY_GREY)
		return & theClayGreyMarking;
	return nullptr;   // ORIGINAL: every drawing area keeps its existing colours
}

double EditorAppearance_lineWidth (int resolution, double logicalPixels) {
	/*
		A logical pixel is one pixel at 100 % display scaling. The screen graphics of a window has
		`resolution` pixels per inch, so the same logical pixel is `resolution / 96` device pixels wide.
		Graphics enlarges a line width by itself above 192 dpi (see LINE_WIDTH_IN_PIXELS),
		so we undo that enlargement and never let a line grow twice.
	*/
	Melder_clipLeft (96, & resolution);
	const double devicePixels = logicalPixels * resolution / 96.0;
	return resolution > 192 ? devicePixels * 192.0 / resolution : devicePixels;
}

double EditorAppearance_diameter (double logicalPixels) {
	return logicalPixels * 25.4 / 96.0;   // millimetres: one logical pixel is 25.4/96 mm
}

kSpectrogram_colourMap EditorAppearance_colourMapForDrawing (kSpectrogram_colourMap colourMap, kEditorAppearancePalette palette) {
	if (EditorAppearancePalette_isOverride (palette))
		return EditorAppearancePalette_colourMap (palette);
	return colourMap;
}

/* End of file EditorAppearance.cpp */
