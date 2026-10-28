#ifndef _EditorAppearance_h_
#define _EditorAppearance_h_
/* EditorAppearance.h
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

#include "melder.h"
#include "Spectrogram.h"   // for kSpectrogram_colourMap

/*
	The appearance of one editor window: which spectrogram colour scheme it uses and how
	transparent the whole window is. Both are fields of the editor itself (see FunctionEditor.h);
	they are never written into any preference, never inherited by a new window and never
	synchronised between windows.
*/

enum class kEditorAppearancePalette : integer { ORIGINAL = 0, MIST_BLUE_GREY = 1, CLAY_GREY = 2 };

inline bool EditorAppearancePalette_isOverride (kEditorAppearancePalette palette) {
	return palette != kEditorAppearancePalette::ORIGINAL;
}

inline kSpectrogram_colourMap EditorAppearancePalette_colourMap (kEditorAppearancePalette palette) {
	if (palette == kEditorAppearancePalette::MIST_BLUE_GREY)
		return kSpectrogram_colourMap::MIST_BLUE_GREY;
	if (palette == kEditorAppearancePalette::CLAY_GREY)
		return kSpectrogram_colourMap::CLAY_GREY;
	return kSpectrogram_colourMap::GREY;   // the caller has to use the window's own colour map instead
}

inline kEditorAppearancePalette kEditorAppearancePalette_fromColourMap (kSpectrogram_colourMap colourMap) {
	if (colourMap == kSpectrogram_colourMap::MIST_BLUE_GREY)
		return kEditorAppearancePalette::MIST_BLUE_GREY;
	if (colourMap == kSpectrogram_colourMap::CLAY_GREY)
		return kEditorAppearancePalette::CLAY_GREY;
	return kEditorAppearancePalette::ORIGINAL;
}

/*
	The colours and the line and dot sizes of one palette. The sizes are logical pixels at
	100 % display scaling, with the halo widths included: a halo is the total width of the
	white under-drawing plus the core that goes on top of it.
	Drawing with these values does not change any analysis parameter: it only selects colours
	and widths, and everything that is set has to be restored afterwards.
*/
struct EditorAppearanceMarking {
	kEditorAppearancePalette palette;
	MelderColour spectrogramDarkest;   // the colour of the strongest energy; the ramp starts at white
	MelderColour pulseCore, pulseHalo, pitchCore, pitchHalo, pitchText;
	MelderColour formantCore, formantHalo;
	MelderColour intensityCore, intensityText, waveform, cursor;
	MelderColour canvas, coordinateText, auxiliaryTick, gridLine, selectionFill;
	double pulseCoreWidth, pulseHaloWidth;   // logical pixels
	double pitchCoreWidth, pitchHaloWidth;   // logical pixels
	double intensityWidth;                   // logical pixels
	double formantHaloDiameterRatio;         // times the user's own dot size
};

/*
	The marking of the palette that is in effect, or `nullptr` if the window draws in the
	original style and every caller should keep its existing colours.
*/
const EditorAppearanceMarking * EditorAppearance_marking (kEditorAppearancePalette palette);

/*
	Line and dot sizes are logical pixels, and the window has to draw them at the resolution of
	its screen. `Graphics::resolution` is the number of pixels per inch (96 at 100 % scaling), and
	GDI draws `Graphics_setLineWidth` as that many pixels, except above 192 dpi, where it
	enlarges the width by itself (see LINE_WIDTH_IN_PIXELS).
*/
double EditorAppearance_lineWidth (int resolution, double logicalPixels);
	/*
		The value to pass to `Graphics_setLineWidth` so that a line is `logicalPixels` wide at the
		current display scaling; `resolution` is `Graphics :: resolution` of the graphics that draws.
	*/

/*
	The colour map that a drawing area should use now: the window's own choice if it runs one of the
	two new schemes, the user's `spectrogram_colourMap` preference otherwise.
*/
kSpectrogram_colourMap EditorAppearance_colourMapForDrawing (kSpectrogram_colourMap colourMap,
		kEditorAppearancePalette palette);

/*
	Millimetres for a circle of `logicalPixels` pixels at any display scaling: the drawing
	commands turn millimetres back into pixels with the resolution, so one logical pixel is
	always 25.4/96 millimetre and the circles scale with the display by themselves.
*/
double EditorAppearance_diameter (double logicalPixels);

/* End of file EditorAppearance.h */
#endif
