/* EditorAppearancePanel.cpp
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

#include "EditorAppearancePanel.h"
#include "Gui.h"   // defines gtk, motif and cocoa

#if motif
	#include "GuiP.h"
	#include <algorithm>
	#include <cmath>
	#include <vector>

	#define CHIP_MIST      101
	#define CHIP_CLAY      102
	#define CHIP_ORIGINAL  103
	#define SLIDER_ID      110
	#define PERCENT_ID     111

	struct structEditorAppearancePanel {
		HWND window;
		HWND chrome;
		EditorAppearancePanel_callbacks callbacks;
		EditorAppearancePanel_state state;
		HFONT font;
		HFONT labelFont;
		HWND hoveredControl;
		HWND chip [3];
		HWND slider;
		HWND percent;
		int dpi;
		int width, height;
		int paletteLabelTop, opacityLabelTop;
		DWORD closedByClickTick;   // when the panel last stepped aside for a click elsewhere, else 0
		float travelLeft, travelRight;   // device pixels: the handle centre at 20 % and at 100 %
		bool dragging;         // this panel holds a slider drag with the left button
		float grabOffset;      // pointer x minus the drawn handle centre, taken when the handle was grabbed
		bool dragChanged;      // the current drag has asked for another value than the one it started with
		integer lastCommittedPercent;   // the last value that went out as a finished change
	};

	namespace {
		/* Every size below is a logical pixel: one pixel at 100 % display scaling. */
		constexpr int PANEL_WIDTH = 328;
		constexpr int PADDING = 24;
		constexpr int ROW_GAP = 28;
		constexpr int CHIP_GAP = 8;
		constexpr int LABEL_HEIGHT = 18;
		constexpr int LABEL_GAP = 12;
		constexpr int CHIP_HEIGHT = 82;
		constexpr int SLIDER_HEIGHT = 32;   // the usable height, not just the groove
		constexpr int SLIDER_GAP = 10;
		constexpr int CORNER_RADIUS = 14;
		constexpr int SHADOW_MARGIN = 18;
		constexpr int PERCENT_WIDTH = 48;
		constexpr int MINIMUM_OPACITY = 20;
		constexpr int MAXIMUM_OPACITY = 100;
		constexpr int THUMB_DIAMETER = 18;   // the visible handle
		constexpr int GRAB_WIDTH = 24;   // the smallest rectangle a press can grab the handle with
		constexpr int GRAB_HEIGHT = 32;   // as high as the row, so the edge of the circle is never missed

		int scaled (int logical, int dpi) {
			return MulDiv (logical, dpi, 96);
		}
		int thumbStrokeWidth (int dpi) {
			return std::max (1, dpi / 96);
		}
		float thumbRadius (int dpi) {
			return (float) scaled (THUMB_DIAMETER / 2, dpi);
		}
		COLORREF colourref (MelderColour colour) {
			return RGB (Melder_iround (colour. red * 255.0), Melder_iround (colour. green * 255.0),
					Melder_iround (colour. blue * 255.0));
		}
		const COLORREF theTextColour = RGB (0x27, 0x29, 0x2C);
		const COLORREF theMutedColour = RGB (0x66, 0x6A, 0x70);
		const COLORREF theInkColour = RGB (0x37, 0x3B, 0x40);
		const COLORREF theSelectionColour = RGB (0x7C, 0x82, 0x89);

		const wchar_t *chipTitle (int slot) {
			if (slot == 1) return L"雾蓝灰";
			if (slot == 2) return L"陶土灰";
			return L"原版";
		}
		kEditorAppearancePalette chipPalette (int slot) {
			if (slot == 1) return kEditorAppearancePalette::MIST_BLUE_GREY;
			if (slot == 2) return kEditorAppearancePalette::CLAY_GREY;
			return kEditorAppearancePalette::ORIGINAL;
		}
		int chipSlotOf (int identifier) {
			if (identifier == CHIP_MIST) return 1;
			if (identifier == CHIP_CLAY) return 2;
			return 0;
		}
		Gdiplus::Color paintColour (COLORREF colour) {
			return Gdiplus::Color (255, GetRValue (colour), GetGValue (colour), GetBValue (colour));
		}
		void roundedPath (Gdiplus::GraphicsPath &path, float left, float top, float width, float height, float radius) {
			const float diameter = 2.0f * radius;
			path.AddArc (left, top, diameter, diameter, 180.0f, 90.0f);
			path.AddArc (left + width - diameter, top, diameter, diameter, 270.0f, 90.0f);
			path.AddArc (left + width - diameter, top + height - diameter, diameter, diameter, 0.0f, 90.0f);
			path.AddArc (left, top + height - diameter, diameter, diameter, 90.0f, 90.0f);
			path.CloseFigure ();
		}

		/*
			One set of numbers describes the handle: where it is drawn, how large it is and which
			presses still count as grabbing it. Painting, hit-testing and the pointer-to-percentage
			mapping all read these same values, so a press on the visible edge cannot disagree with
			what the user sees. Everything is device pixels inside the client area of the control.
		*/
		struct SliderGeometry {
			RECT client;
			float centreX, centreY;   // the drawn handle centre, half-pixel alignment included
			float radius;
			int strokeWidth;
			RECT grab;   // exclusive right and bottom, clipped to the control
		};

		SliderGeometry sliderGeometry (EditorAppearancePanel me) {
			SliderGeometry geometry {};
			GetClientRect (me -> slider, & geometry.client);
			RECT thumb;
			SendMessageW (me -> slider, TBM_GETTHUMBRECT, 0, (LPARAM) & thumb);
			geometry.strokeWidth = thumbStrokeWidth (me -> dpi);
			geometry.radius = thumbRadius (me -> dpi);
			/* The ring keeps both of its edges on the pixel grid, which shifts its centre by half. */
			const float phase = geometry.strokeWidth % 2 ? 0.5f : 0.0f;
			geometry.centreX = std::round ((thumb.left + thumb.right) / 2.0f - phase) + phase;
			geometry.centreY = std::round ((thumb.top + thumb.bottom) / 2.0f - phase) + phase;
			/* The whole visible circle and its stroke, but never smaller than the press target. */
			const float ink = geometry.radius + geometry.strokeWidth / 2.0f + 0.5f;
			const float halfWidth = std::max (ink, (float) scaled (GRAB_WIDTH / 2, me -> dpi));
			const float halfHeight = std::max (ink, (float) scaled (GRAB_HEIGHT / 2, me -> dpi));
			/*
				The native control keeps its handle above the middle of the row and the travel ends at the
				border, so a band cut to the control would lose part of its size exactly where the user is
				pressing. The band grows on the side that still has room instead: the circle keeps a grab
				area of the full size, and no press on the visible handle reads as a press on the channel.
			*/
			const LONG smallestWidth = std::min ((LONG) std::lround (2 * halfWidth),
					geometry.client.right - geometry.client.left);
			const LONG smallestHeight = std::min ((LONG) std::lround (2 * halfHeight),
					geometry.client.bottom - geometry.client.top);
			geometry.grab.left = std::max (geometry.client.left, (LONG) std::floor (geometry.centreX - halfWidth));
			geometry.grab.top = std::max (geometry.client.top, (LONG) std::floor (geometry.centreY - halfHeight));
			geometry.grab.right = std::min (geometry.client.right, (LONG) std::ceil (geometry.centreX + halfWidth));
			geometry.grab.bottom = std::min (geometry.client.bottom, (LONG) std::ceil (geometry.centreY + halfHeight));
			if (geometry.grab.right - geometry.grab.left < smallestWidth) {
				const LONG missing = smallestWidth - (geometry.grab.right - geometry.grab.left);
				if (geometry.grab.left - missing >= geometry.client.left)
					geometry.grab.left -= missing;
				else
					geometry.grab.right += missing;
			}
			if (geometry.grab.bottom - geometry.grab.top < smallestHeight) {
				const LONG missing = smallestHeight - (geometry.grab.bottom - geometry.grab.top);
				if (geometry.grab.top - missing >= geometry.client.top)
					geometry.grab.top -= missing;
				else
					geometry.grab.bottom += missing;
			}
			return geometry;
		}

		/* The percentage a pointer position asks for: the grab offset out, the travel clamped, 1 % steps. */
		integer sliderPercentForPointerX (EditorAppearancePanel me, int pointerX) {
			const float travel = me -> travelRight - me -> travelLeft;
			if (travel <= 0.0f)
				return me -> state. opacityPercent;
			const float x = std::min (std::max ((float) pointerX - me -> grabOffset, me -> travelLeft), me -> travelRight);
			integer percent = (integer) std::lround (MINIMUM_OPACITY +
					(MAXIMUM_OPACITY - MINIMUM_OPACITY) * (double) ((x - me -> travelLeft) / travel));
			if (percent < MINIMUM_OPACITY)
				percent = MINIMUM_OPACITY;
			if (percent > MAXIMUM_OPACITY)
				percent = MAXIMUM_OPACITY;
			return percent;
		}

		bool paintChrome (EditorAppearancePanel me, int left, int top) {
			const int margin = scaled (SHADOW_MARGIN, me -> dpi);
			const int width = me -> width + 2 * margin, height = me -> height + 2 * margin;
			BITMAPINFO info {};
			info.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
			info.bmiHeader.biWidth = width;
			info.bmiHeader.biHeight = - height;
			info.bmiHeader.biPlanes = 1;
			info.bmiHeader.biBitCount = 32;
			void *bits;
			HDC dc = CreateCompatibleDC (nullptr);
			HBITMAP bitmap = CreateDIBSection (dc, & info, DIB_RGB_COLORS, & bits, nullptr, 0);
			if (! bitmap) {
				DeleteDC (dc);
				return false;
			}
			HGDIOBJ previous = SelectObject (dc, bitmap);
			{
				Gdiplus::Bitmap surface (width, height, width * 4, PixelFormat32bppPARGB, (BYTE *) bits);
				Gdiplus::Graphics canvas (& surface);
				canvas.Clear (Gdiplus::Color (0, 0, 0, 0));
				canvas.SetSmoothingMode (Gdiplus::SmoothingModeAntiAlias);
				canvas.SetPixelOffsetMode (Gdiplus::PixelOffsetModeHalf);
				const float scale = me -> dpi / 96.0f;
				Gdiplus::SolidBrush shadow (Gdiplus::Color (3, 39, 49, 66));
				for (int step = 12; step > 0; step --) {
					const float spread = step * scale;
					Gdiplus::GraphicsPath path;
					roundedPath (path, margin - spread, margin + 4.0f * scale - spread,
						me -> width + 2.0f * spread, me -> height + 2.0f * spread,
						CORNER_RADIUS * scale + spread);
					canvas.FillPath (& shadow, & path);
				}
				Gdiplus::GraphicsPath plate;
				roundedPath (plate, (float) margin, (float) margin, (float) me -> width, (float) me -> height,
					CORNER_RADIUS * scale);
				Gdiplus::SolidBrush white (Gdiplus::Color (255, 255, 255, 255));
				canvas.FillPath (& white, & plate);
			}
			POINT destination { left - margin, top - margin }, source { 0, 0 };
			SIZE size { width, height };
			BLENDFUNCTION blend { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
			const bool painted = UpdateLayeredWindow (me -> chrome, nullptr, & destination, & size, dc,
				& source, 0, & blend, ULW_ALPHA);
			SelectObject (dc, previous);
			DeleteObject (bitmap);
			DeleteDC (dc);
			return painted;
		}

		void drawChip (const EditorAppearancePanel me, const DRAWITEMSTRUCT *item) {
			const int slot = chipSlotOf (item -> CtlID);
			const bool selected = me -> state. palette == chipPalette (slot);
			const RECT face = item -> rcItem;
			const int dpi = me -> dpi;
			const int width = face. right - face. left, height = face. bottom - face. top;
			HDC graphics = CreateCompatibleDC (item -> hDC);
			HBITMAP bitmap = CreateCompatibleBitmap (item -> hDC, width, height);
			HGDIOBJ oldBitmap = SelectObject (graphics, bitmap);
			FillRect (graphics, & face, (HBRUSH) GetStockObject (WHITE_BRUSH));
			const float scale = dpi / 96.0f;
			{
				Gdiplus::Graphics canvas (graphics);
				canvas.SetSmoothingMode (Gdiplus::SmoothingModeAntiAlias);
				canvas.SetPixelOffsetMode (Gdiplus::PixelOffsetModeHalf);
				const bool hot = me -> hoveredControl == item -> hwndItem || (item -> itemState & ODS_SELECTED);
				const COLORREF surface = selected ? RGB (0xEE, 0xF0, 0xF2) :
					hot ? RGB (0xEC, 0xEE, 0xF0) : RGB (0xF4, 0xF5, 0xF6);
				Gdiplus::SolidBrush fill (paintColour (surface));
				Gdiplus::GraphicsPath tile;
				const float tileInset = (float) scaled (1, dpi);
				roundedPath (tile, tileInset, tileInset, width - 2.0f * tileInset, height - 2.0f * tileInset, 9.0f * scale);
				canvas.FillPath (& fill, & tile);
				if (selected) {
					/* Keep both stroke edges on device-pixel boundaries; only the curves need antialiasing. */
					const float strokeWidth = (float) std::max (1, dpi / 96);
					const float strokeInset = tileInset + strokeWidth / 2.0f;
					Gdiplus::GraphicsPath border;
					roundedPath (border, strokeInset, strokeInset, width - 2.0f * strokeInset,
						height - 2.0f * strokeInset, 9.0f * scale - strokeWidth / 2.0f);
					Gdiplus::Pen outline (paintColour (theSelectionColour), strokeWidth);
					canvas.DrawPath (& outline, & border);
				}
				const COLORREF tone = slot == 0 ? RGB (0x59, 0x61, 0x69) :
					colourref (EditorAppearance_marking (chipPalette (slot)) -> spectrogramDarkest);
				const float swatchLeft = (width - 36.0f * scale) / 2.0f;
				const float strengths [] = { 0.30f, 0.62f, 1.0f };
				for (int bar = 0; bar < 3; bar ++) {
					const float strength = strengths [bar];
					Gdiplus::SolidBrush colour (Gdiplus::Color (255,
						(BYTE) (GetRValue (surface) + strength * (GetRValue (tone) - GetRValue (surface))),
						(BYTE) (GetGValue (surface) + strength * (GetGValue (tone) - GetGValue (surface))),
						(BYTE) (GetBValue (surface) + strength * (GetBValue (tone) - GetBValue (surface)))));
					Gdiplus::GraphicsPath sample;
					roundedPath (sample, swatchLeft + bar * 13.0f * scale, 14.0f * scale,
						10.0f * scale, 29.0f * scale, 1.5f * scale);
					canvas.FillPath (& colour, & sample);
				}
				if (selected) {
					Gdiplus::Pen tick (paintColour (theInkColour), 1.25f * scale);
					tick.SetStartCap (Gdiplus::LineCapRound);
					tick.SetEndCap (Gdiplus::LineCapRound);
					tick.SetLineJoin (Gdiplus::LineJoinRound);
					Gdiplus::PointF points [] = {
						{ width - 17.0f * scale, 12.0f * scale },
						{ width - 14.0f * scale, 15.0f * scale },
						{ width - 8.0f * scale, 9.0f * scale }
					};
					canvas.DrawLines (& tick, points, 3);
				}
				if ((item -> itemState & ODS_FOCUS) && ! (item -> itemState & ODS_NOFOCUSRECT)) {
					Gdiplus::Pen focus (paintColour (theMutedColour), scale);
					focus.SetDashStyle (Gdiplus::DashStyleDot);
					Gdiplus::GraphicsPath ring;
					roundedPath (ring, 4.0f * scale, 4.0f * scale, width - 8.0f * scale, height - 8.0f * scale, 6.0f * scale);
					canvas.DrawPath (& focus, & ring);
				}
			}
			SetBkMode (graphics, TRANSPARENT);
			SetTextColor (graphics, theTextColour);
			HFONT oldFont = (HFONT) SelectObject (graphics, me -> font);
			RECT label { 0, scaled (53, dpi), width, height - scaled (10, dpi) };
			DrawTextW (graphics, chipTitle (slot), -1, & label, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
			SelectObject (graphics, oldFont);
			BitBlt (item -> hDC, face. left, face. top, width, height, graphics, 0, 0, SRCCOPY);
			SelectObject (graphics, oldBitmap);
			DeleteObject (bitmap);
			DeleteDC (graphics);
		}

		void drawSliderThumb (Gdiplus::Graphics &canvas, const SliderGeometry & geometry) {
			const int strokeWidth = geometry.strokeWidth;
			const float cx = geometry.centreX, cy = geometry.centreY;
			const int radius = (int) geometry.radius;
			const int size = 2 * radius + strokeWidth + 2;
			const int left = (int) (cx - radius - strokeWidth / 2.0f) - 1;
			const int top = (int) (cy - radius - strokeWidth / 2.0f) - 1;
			std::vector <BYTE> pixels (size * size * 4);
			/* Distance coverage keeps the four axial extrema solid while smoothing the remaining arc. */
			for (int y = 0; y < size; y ++) {
				for (int x = 0; x < size; x ++) {
					const float dx = left + x + 0.5f - cx, dy = top + y + 0.5f - cy;
					const float distance = std::sqrt (dx * dx + dy * dy);
					const float outer = std::clamp (radius + strokeWidth / 2.0f + 0.5f - distance, 0.0f, 1.0f);
					const float inner = std::clamp (radius - strokeWidth / 2.0f + 0.5f - distance, 0.0f, 1.0f);
					const float ring = outer - inner;
					BYTE *pixel = & pixels [(y * size + x) * 4];
					pixel [0] = (BYTE) std::lround (GetBValue (theInkColour) * ring + 255.0f * inner);
					pixel [1] = (BYTE) std::lround (GetGValue (theInkColour) * ring + 255.0f * inner);
					pixel [2] = (BYTE) std::lround (GetRValue (theInkColour) * ring + 255.0f * inner);
					pixel [3] = (BYTE) std::lround (255.0f * outer);
				}
			}
			Gdiplus::Bitmap image (size, size, size * 4, PixelFormat32bppPARGB, pixels.data ());
			const Gdiplus::GraphicsState saved = canvas.Save ();
			canvas.SetInterpolationMode (Gdiplus::InterpolationModeNearestNeighbor);
			canvas.DrawImage (& image, Gdiplus::Rect (left, top, size, size), 0, 0, size, size, Gdiplus::UnitPixel);
			canvas.Restore (saved);
		}

		void drawSlider (EditorAppearancePanel me, HDC dc) {
			const SliderGeometry geometry = sliderGeometry (me);
			RECT channel;
			SendMessageW (me -> slider, TBM_GETCHANNELRECT, 0, (LPARAM) & channel);
			const RECT & client = geometry.client;
			HDC buffer = CreateCompatibleDC (dc);
			HBITMAP bitmap = CreateCompatibleBitmap (dc, client. right, client. bottom);
			HGDIOBJ oldBitmap = SelectObject (buffer, bitmap);
			FillRect (buffer, & client, (HBRUSH) GetStockObject (WHITE_BRUSH));
			const float scale = me -> dpi / 96.0f;
			{
				Gdiplus::Graphics canvas (buffer);
				canvas.SetSmoothingMode (Gdiplus::SmoothingModeAntiAlias);
				canvas.SetPixelOffsetMode (Gdiplus::PixelOffsetModeHalf);
				const float y = geometry.centreY, x = geometry.centreX;
				Gdiplus::Pen rail (Gdiplus::Color (255, 225, 227, 230), 4.0f * scale);
				rail.SetStartCap (Gdiplus::LineCapRound);
				rail.SetEndCap (Gdiplus::LineCapRound);
				canvas.DrawLine (& rail, (float) channel. left, y, (float) channel. right, y);
				Gdiplus::Pen fill (paintColour (theInkColour), 4.0f * scale);
				fill.SetStartCap (Gdiplus::LineCapRound);
				fill.SetEndCap (Gdiplus::LineCapRound);
				canvas.DrawLine (& fill, (float) channel. left, y, x, y);
				drawSliderThumb (canvas, geometry);
			}
			if (GetFocus () == me -> slider && ! (SendMessageW (me -> slider, WM_QUERYUISTATE, 0, 0) & UISF_HIDEFOCUS)) {
				RECT focus = client;
				InflateRect (& focus, - scaled (2, me -> dpi), - scaled (2, me -> dpi));
				DrawFocusRect (buffer, & focus);
			}
			BitBlt (dc, 0, 0, client. right, client. bottom, buffer, 0, 0, SRCCOPY);
			SelectObject (buffer, oldBitmap);
			DeleteObject (bitmap);
			DeleteDC (buffer);
		}

		void updateReadout (EditorAppearancePanel me, int percent) {
			wchar_t text [16];
			swprintf (text, L"%d%%", percent);
			SetWindowTextW (me -> percent, text);
		}

		void updateSliderName (EditorAppearancePanel me) {
			/*
				Assistive technology reads the value from the control itself, so the percentage also
				has to be part of the slider name, not only of the label next to it.
			*/
			wchar_t name [40];
			swprintf (name, L"不透明度 %d %%", (int) me -> state. opacityPercent);
			SetWindowTextW (me -> slider, name);
		}

		void applyStateToControls (EditorAppearancePanel me) {
			for (HWND chip : me -> chip)
				InvalidateRect (chip, nullptr, TRUE);
			SendMessageW (me -> slider, TBM_SETPOS, TRUE, me -> state. opacityPercent);
			updateReadout (me, (int) me -> state. opacityPercent);
			updateSliderName (me);
			/*
				Whatever the window really shows is the value that has been reached: the panel copies it
				in from outside, so there is nothing finished left to report.
			*/
			me -> lastCommittedPercent = me -> state. opacityPercent;
		}

		/*
			The one place where a number becomes something the user can see and reach: the native control
			carries the percentage for the keyboard and for assistive technology, the read-out shows it,
			and the handle is painted again. Compositing is not touched here; the caller says whether this
			is a preview of a drag or a finished change.
		*/
		void setSliderValue (EditorAppearancePanel me, integer percent) {
			if (percent == me -> state. opacityPercent)
				return;
			SendMessageW (me -> slider, TBM_SETPOS, TRUE, percent);
			me -> state. opacityPercent = percent;
			updateReadout (me, (int) percent);
			InvalidateRect (me -> slider, nullptr, FALSE);
		}

		/*
			Every request for another opacity comes through this entry: a drag movement, the release that
			ends a drag, a key, and the notification that follows a key. `finished` marks the value the
			user settled on; anything else only shows what the window would look like. A percentage that
			is already in effect is never reported a second time.
		*/
		void reportOpacity (EditorAppearancePanel me, integer percent, bool finished) {
			if (percent < MINIMUM_OPACITY)
				percent = MINIMUM_OPACITY;
			if (percent > MAXIMUM_OPACITY)
				percent = MAXIMUM_OPACITY;
			if (! finished) {
				if (percent == me -> state. opacityPercent)
					return;   // a pointer that has not crossed a whole percent asks for nothing
				setSliderValue (me, percent);
				if (me -> callbacks. previewOpacity)
					me -> callbacks. previewOpacity (me -> callbacks. boss, percent);
				updateSliderName (me);   // the readable name follows the value that took effect
				return;
			}
			if (percent == me -> lastCommittedPercent)
				return;   // the native end of a key repeats a change that is already finished
			setSliderValue (me, percent);
			me -> lastCommittedPercent = percent;
			if (me -> callbacks. commitOpacity)
				me -> callbacks. commitOpacity (me -> callbacks. boss, percent);
			if (me -> lastCommittedPercent != me -> state. opacityPercent)
				/* The system refused: the editor has written back the percentage that is in effect. */
				me -> lastCommittedPercent = me -> state. opacityPercent;
			updateSliderName (me);
		}

		/*
			The one way out of a drag. The release, Escape, the panel hiding, and a capture that another
			window has taken all end here. The exit flag goes before anything is reported, so the capture
			notification that follows our own release cannot finish a second change. `percent` is the value
			the pointer ended on, or -1 when the drag stopped without one: the last value that was applied
			successfully is kept and nothing is worked out from a pointer that has moved on its own.
		*/
		void endSliderDrag (EditorAppearancePanel me, integer percent) {
			if (! me -> dragging)
				return;
			me -> dragging = false;
			bool changed = me -> dragChanged;
			me -> dragChanged = false;
			if (percent < 0)
				percent = me -> state. opacityPercent;   // ended without a position: keep what is in effect
			else if (percent != me -> state. opacityPercent)
				changed = true;
			if (changed)
				reportOpacity (me, percent, true);
			if (GetCapture () == me -> slider)
				ReleaseCapture ();
		}

		/* One step of a drag: only a value the window does not already have reaches the compositor. */
		void previewSliderDrag (EditorAppearancePanel me, integer percent) {
			if (! me -> dragging || percent == me -> state. opacityPercent)
				return;
			me -> dragChanged = true;
			reportOpacity (me, percent, false);
			if (me -> state. opacityPercent != percent) {
				/* The editor wrote the percentage that really works back, so this drag is over. */
				endSliderDrag (me, percent);
			}
		}

		/*
			A press on the handle, on its grab area, or on the channel. Grabbing the handle keeps the
			distance between the pointer and the circle centre, so the handle does not jump under the
			pointer; a press on the channel goes straight to that position and continues as a drag,
			without the native paging timer that would otherwise chase it ten percent at a time.
		*/
		void beginSliderDrag (EditorAppearancePanel me, int x, int y) {
			const SliderGeometry geometry = sliderGeometry (me);
			SetFocus (me -> slider);   // the keyboard follows the pointer, as in any native control
			SetCapture (me -> slider);
			if (GetCapture () != me -> slider)
				return;   // without the capture the handle would be lost at the edge of the row
			me -> dragging = true;
			me -> dragChanged = false;
			const bool grabbed = PtInRect (& geometry.grab, POINT { x, y }) != FALSE;
			me -> grabOffset = grabbed ? (float) x - geometry.centreX : 0.0f;
			if (! grabbed)
				previewSliderDrag (me, sliderPercentForPointerX (me, x));
			InvalidateRect (me -> slider, nullptr, FALSE);   // the focus ring appears
		}

		void moveSliderDrag (EditorAppearancePanel me, int x) {
			previewSliderDrag (me, sliderPercentForPointerX (me, x));
		}

		/* A drag that cannot finish any more: the windows go away, so nothing is reported. */
		void clearSliderDrag (EditorAppearancePanel me) {
			me -> dragging = false;
			me -> dragChanged = false;
			me -> grabOffset = 0.0f;
			if (me -> slider && IsWindow (me -> slider) && GetCapture () == me -> slider)
				ReleaseCapture ();
		}

		/*
			The travel of the handle belongs to the native control: it is the distance between the two
			circle centres the control itself uses, never the width of the control or of the groove. The
			ends are read once per layout, while the panel is still hidden, and the percentage the user
			has is put back straight afterwards; a drag never probes the ends again.
		*/
		void measureSliderTravel (EditorAppearancePanel me) {
			const integer keep = me -> state. opacityPercent;
			RECT thumb, channel;
			SendMessageW (me -> slider, TBM_GETCHANNELRECT, 0, (LPARAM) & channel);
			/* TRUE synchronizes the native thumb rectangle as well as the logical value, while hidden. */
			SendMessageW (me -> slider, TBM_SETPOS, TRUE, MINIMUM_OPACITY);
			SendMessageW (me -> slider, TBM_GETTHUMBRECT, 0, (LPARAM) & thumb);
			const float width = (float) (thumb.right - thumb.left);
			me -> travelLeft = (thumb.left + thumb.right) / 2.0f;
			SendMessageW (me -> slider, TBM_SETPOS, TRUE, MAXIMUM_OPACITY);
			SendMessageW (me -> slider, TBM_GETTHUMBRECT, 0, (LPARAM) & thumb);
			me -> travelRight = (thumb.left + thumb.right) / 2.0f;
			SendMessageW (me -> slider, TBM_SETPOS, TRUE, keep);
			if (me -> travelRight <= me -> travelLeft) {
				/* The control gave no travel of its own: the groove with half a handle at each end. */
				me -> travelLeft = channel.left + width / 2.0f;
				me -> travelRight = channel.right - width / 2.0f;
			}
		}

		void hidePanel (EditorAppearancePanel me, bool closedByClick) {
			if (me -> window && IsWindow (me -> window)) {
				endSliderDrag (me, -1);   // a held button does not survive the panel going away
				HWND owner = (HWND) GetWindowLongPtrW (me -> window, GWLP_HWNDPARENT);
				const bool wasActive = GetActiveWindow () == me -> window;
				ShowWindow (me -> chrome, SW_HIDE);
				ShowWindow (me -> window, SW_HIDE);
				if (closedByClick)
					me -> closedByClickTick = GetTickCount ();
				/*
					While the panel is open it holds the keyboard, so the editor has to take it back;
					otherwise the window would look normal but stop answering keys after a dismissal.
				*/
				if (wasActive && owner && IsWindow (owner))
					SetForegroundWindow (owner);
			}
		}

		LRESULT CALLBACK childProcedure (HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
			WNDPROC original = (WNDPROC) GetPropW (window, L"PraatAppearancePanelProcedure");
			EditorAppearancePanel me = (EditorAppearancePanel) GetWindowLongPtr (GetParent (window), GWLP_USERDATA);
			if (me && window == me -> slider) {
				if (message == WM_ERASEBKGND)
					return 1;
				if (message == WM_LBUTTONDOWN || message == WM_LBUTTONDBLCLK) {
					/* This one press is handled here: the native paging must not start beside it. */
					SendMessageW (GetParent (window), WM_CHANGEUISTATE, MAKEWPARAM (UIS_SET, UISF_HIDEFOCUS), 0);
					beginSliderDrag (me, GET_X_LPARAM (lParam), GET_Y_LPARAM (lParam));
					return 0;
				}
				if (message == WM_MOUSEMOVE && me -> dragging) {
					/* Signed client coordinates: outside the row, and even outside the panel, still count. */
					moveSliderDrag (me, GET_X_LPARAM (lParam));
					return 0;
				}
				if (message == WM_LBUTTONUP) {
					if (me -> dragging)
						endSliderDrag (me, (integer) sliderPercentForPointerX (me, GET_X_LPARAM (lParam)));
					return 0;
				}
				if (message == WM_CAPTURECHANGED || message == WM_CANCELMODE) {
					/* Somebody else has the pointer now: finish, keep the last value, never re-capture. */
					endSliderDrag (me, -1);
				}
				if (message == WM_PAINT) {
					/* The circular thumb is wider than the trackbar's native partial invalidation. */
					InvalidateRect (window, nullptr, FALSE);
					PAINTSTRUCT paint;
					HDC dc = BeginPaint (window, & paint);
					drawSlider (me, dc);
					EndPaint (window, & paint);
					return 0;
				}
				if (message == WM_PRINTCLIENT) {
					drawSlider (me, (HDC) wParam);
					return 0;
				}
			}
			if (me && message == WM_MOUSEMOVE && me -> hoveredControl != window) {
				me -> hoveredControl = window;
				TRACKMOUSEEVENT tracking { sizeof (TRACKMOUSEEVENT), TME_LEAVE, window, 0 };
				TrackMouseEvent (& tracking);
				InvalidateRect (window, nullptr, FALSE);
			}
			if (me && message == WM_MOUSELEAVE) {
				if (me -> hoveredControl == window)
					me -> hoveredControl = nullptr;
				InvalidateRect (window, nullptr, FALSE);
			}
			if (message == WM_KEYDOWN)
				SendMessageW (GetParent (window), WM_CHANGEUISTATE, MAKEWPARAM (UIS_CLEAR, UISF_HIDEFOCUS), 0);
			if (message == WM_LBUTTONDOWN)
				SendMessageW (GetParent (window), WM_CHANGEUISTATE, MAKEWPARAM (UIS_SET, UISF_HIDEFOCUS), 0);
			if (message == WM_SETFOCUS || message == WM_KILLFOCUS || message == WM_UPDATEUISTATE) {
				const LRESULT result = CallWindowProcW (original, window, message, wParam, lParam);
				InvalidateRect (window, nullptr, FALSE);
				return result;
			}
			if (message == WM_KEYDOWN && wParam == VK_ESCAPE) {
				SendMessageW (GetParent (window), WM_CLOSE, 0, 0);
				return 0;
			}
			if (message == WM_KEYDOWN && wParam == VK_TAB) {
				HWND next = GetNextDlgTabItem (GetParent (window), window, GetKeyState (VK_SHIFT) < 0);
				if (next)
					SetFocus (next);
				return 0;
			}
			if (message == WM_KEYDOWN && wParam == VK_RETURN && GetDlgCtrlID (window) >= CHIP_MIST &&
					GetDlgCtrlID (window) <= CHIP_ORIGINAL) {
				SendMessageW (window, BM_CLICK, 0, 0);
				return 0;
			}
			if (message == WM_KEYDOWN && (wParam == VK_LEFT || wParam == VK_RIGHT)) {
				HWND parent = GetParent (window);
				me = (EditorAppearancePanel) GetWindowLongPtr (parent, GWLP_USERDATA);
				for (int slot = 0; me && slot < 3; slot ++) {
					if (me -> chip [slot] != window)
						continue;
					int next = slot + (wParam == VK_RIGHT ? 1 : -1);
					if (next < 0) next = 2;
					if (next > 2) next = 0;
					SetFocus (me -> chip [next]);
					return 0;
				}
			}
			return CallWindowProcW (original, window, message, wParam, lParam);
		}

		LRESULT CALLBACK panelProcedure (HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
			EditorAppearancePanel me = (EditorAppearancePanel) GetWindowLongPtr (window, GWLP_USERDATA);
			switch (message) {
				case WM_NCCREATE: {
					CREATESTRUCTW *creation = (CREATESTRUCTW *) lParam;
					SetWindowLongPtr (window, GWLP_USERDATA, (LONG_PTR) creation -> lpCreateParams);
				} break;
				case WM_PAINT: {
					PAINTSTRUCT ps;
					HDC graphics = BeginPaint (window, & ps);
					if (me) {
						SetBkMode (graphics, TRANSPARENT);
						SetTextColor (graphics, theTextColour);
						HFONT oldFont = (HFONT) SelectObject (graphics, me -> labelFont);
						TextOutW (graphics, scaled (PADDING, me -> dpi), me -> paletteLabelTop, L"配色", 2);
						TextOutW (graphics, scaled (PADDING, me -> dpi), me -> opacityLabelTop, L"不透明度", 4);
						SelectObject (graphics, oldFont);
					}
					EndPaint (window, & ps);
					return 0;
				}
				case WM_DRAWITEM: {
					if (me) {
						const DRAWITEMSTRUCT *item = (const DRAWITEMSTRUCT *) lParam;
						if (item -> CtlID >= CHIP_MIST && item -> CtlID <= CHIP_ORIGINAL) {
							drawChip (me, item);
							return TRUE;
						}
					}
				} break;
				case WM_CTLCOLORSTATIC: {
					static HBRUSH white { nullptr };
					if (! white)
						white = CreateSolidBrush (RGB (0xFF, 0xFF, 0xFF));
					SetBkMode ((HDC) wParam, TRANSPARENT);
					SetTextColor ((HDC) wParam, theTextColour);
					SetBkColor ((HDC) wParam, RGB (0xFF, 0xFF, 0xFF));
					return (LRESULT) white;
				}
				case WM_SHOWWINDOW: {
					if (me && ! wParam && me -> chrome)
						ShowWindow (me -> chrome, SW_HIDE);
				} break;
				case WM_COMMAND: {
					if (me) {
						const int id = LOWORD (wParam);
						if (id >= CHIP_MIST && id <= CHIP_ORIGINAL && HIWORD (wParam) == BN_CLICKED) {
							me -> state. palette = chipPalette (chipSlotOf (id));
							InvalidateRect (me -> chip [0], nullptr, TRUE);
							InvalidateRect (me -> chip [1], nullptr, TRUE);
							InvalidateRect (me -> chip [2], nullptr, TRUE);
							if (me -> callbacks. changePalette)
								me -> callbacks. changePalette (me -> callbacks. boss, me -> state. palette);
							return 0;
						}
					}
				} break;
				case WM_HSCROLL: {
					if (me && (HWND) lParam == me -> slider) {
						/*
							A drag never comes this way: the panel takes the presses itself, so the native
							control cannot page or drag beside it. What is left are the keys — one percent
							for an arrow, ten for PageUp and PageDown, the two ends for Home and End — and
							each of them is a finished change as it arrives. The end notification that
							follows a key names the percentage that is already in effect, and is not
							reported a second time.
						*/
						const bool tracking = LOWORD (wParam) == TB_THUMBTRACK;
						const integer position = (integer) SendMessageW (me -> slider, TBM_GETPOS, 0, 0);
						reportOpacity (me, position, ! tracking);
						return 0;
					}
				} break;
				case WM_CLOSE: {
					if (me) {
						hidePanel (me, false);
						return 0;
					}
				} break;
				case WM_ACTIVATE: {
					if (me && LOWORD (wParam) == WA_INACTIVE) {
						/*
							A click outside the panel took the keyboard away. The panel does not know
							where that click landed, so the time is recorded: if the same click turns
							out to be the menu-bar entry, the editor must not open the panel again.
						*/
						hidePanel (me, true);
						return 0;
					}
				} break;
				case WM_KEYDOWN: {
					if (me && wParam == VK_ESCAPE) {
						hidePanel (me, false);
						return 0;
					}
				} break;
			}
			return DefWindowProcW (window, message, wParam, lParam);
		}

		void createControls (EditorAppearancePanel me) {
			const DWORD chipStyle = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW;
			const int identifiers [] = { CHIP_ORIGINAL, CHIP_MIST, CHIP_CLAY };
			for (int slot = 0; slot < 3; slot ++)
				me -> chip [slot] = CreateWindowExW (0, L"BUTTON", chipTitle (slot), chipStyle, 0, 0, 0, 0, me -> window,
					(HMENU) (INT_PTR) identifiers [slot], theGui. instance, nullptr);
			me -> slider = CreateWindowExW (0, TRACKBAR_CLASSW, L"不透明度",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | TBS_NOTICKS | TBS_FIXEDLENGTH, 0, 0, 0, 0, me -> window,
					(HMENU) (INT_PTR) SLIDER_ID, theGui. instance, nullptr);
			me -> percent = CreateWindowExW (0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_RIGHT,
					0, 0, 0, 0, me -> window, (HMENU) (INT_PTR) PERCENT_ID, theGui. instance, nullptr);
			for (HWND control : { me -> chip [0], me -> chip [1], me -> chip [2], me -> slider, me -> percent }) {
				if (! control)
					continue;
				SendMessageW (control, WM_SETFONT, (WPARAM) me -> font, TRUE);
				SetPropW (control, L"PraatAppearancePanelProcedure",
						(HANDLE) SetWindowLongPtr (control, GWLP_WNDPROC, (LONG_PTR) childProcedure));
			}
			SendMessageW (me -> slider, TBM_SETRANGE, TRUE, MAKELONG (MINIMUM_OPACITY, MAXIMUM_OPACITY));
			SendMessageW (me -> slider, TBM_SETLINESIZE, 0, 1);   // one percent per arrow key
			SendMessageW (me -> slider, TBM_SETPAGESIZE, 0, 10);
			SendMessageW (me -> slider, TBM_SETTHUMBLENGTH, scaled (THUMB_DIAMETER, me -> dpi), 0);
			const int pad = scaled (PADDING, me -> dpi);
			const int contentWidth = me -> width - 2 * pad;
			me -> paletteLabelTop = pad;
			int y = pad + scaled (LABEL_HEIGHT, me -> dpi) + scaled (LABEL_GAP, me -> dpi);
			const int chipGap = scaled (CHIP_GAP, me -> dpi);
			const int chipWidth = (contentWidth - 2 * chipGap) / 3;
			const int chipHeight = scaled (CHIP_HEIGHT, me -> dpi);
			for (int slot = 0; slot < 3; slot ++)
				SetWindowPos (me -> chip [slot], nullptr, pad + slot * (chipWidth + chipGap), y,
					chipWidth, chipHeight, SWP_NOZORDER | SWP_NOACTIVATE);
			y += chipHeight + scaled (ROW_GAP, me -> dpi);
			me -> opacityLabelTop = y;
			const int percentWidth = scaled (PERCENT_WIDTH, me -> dpi);
			SetWindowPos (me -> percent, nullptr, me -> width - pad - percentWidth, y,
					percentWidth, scaled (LABEL_HEIGHT, me -> dpi), SWP_NOZORDER | SWP_NOACTIVATE);
			y += scaled (LABEL_HEIGHT + SLIDER_GAP, me -> dpi);
			SetWindowPos (me -> slider, nullptr, pad, y, contentWidth, scaled (SLIDER_HEIGHT, me -> dpi),
					SWP_NOZORDER | SWP_NOACTIVATE);
			/* The handle travel is measured now, while the panel has not been shown yet. */
			measureSliderTravel (me);
			/* The native window stays inside the opaque part of the alpha-smoothed outer plate. */
			SetWindowRgn (me -> window, CreateRoundRectRgn (1, 1, me -> width - 1, me -> height - 1,
					2 * scaled (CORNER_RADIUS, me -> dpi), 2 * scaled (CORNER_RADIUS, me -> dpi)), TRUE);
		}

		void destroyWindow (EditorAppearancePanel me) {
			clearSliderDrag (me);   // a drag that is interrupted by destruction reports nothing
			if (me -> chrome && IsWindow (me -> chrome))
				DestroyWindow (me -> chrome);
			me -> chrome = nullptr;
			if (me -> window && IsWindow (me -> window)) {
				for (HWND control : { me -> chip [0], me -> chip [1], me -> chip [2], me -> slider, me -> percent }) {
					if (control && IsWindow (control)) {
						WNDPROC original = (WNDPROC) GetPropW (control, L"PraatAppearancePanelProcedure");
						if (original) {
							SetWindowLongPtr (control, GWLP_WNDPROC, (LONG_PTR) original);
							RemovePropW (control, L"PraatAppearancePanelProcedure");
						}
					}
				}
				DestroyWindow (me -> window);
			}
			if (me -> font)
				DeleteObject (me -> font);
			if (me -> labelFont)
				DeleteObject (me -> labelFont);
			me -> font = nullptr;
			me -> labelFont = nullptr;
			me -> hoveredControl = nullptr;
			me -> window = nullptr;
			me -> dpi = 0;
			me -> travelLeft = 0.0f;
			me -> travelRight = 0.0f;
			me -> chip [0] = me -> chip [1] = me -> chip [2] = me -> slider = me -> percent = nullptr;
		}
	}

	EditorAppearancePanel EditorAppearancePanel_create (const EditorAppearancePanel_callbacks *callbacks) {
		_GuiWin_ensureGdiplus ();
		static ATOM windowClass { [] () {
			WNDCLASSW definition { };
			definition. lpfnWndProc = panelProcedure;
			definition. hInstance = theGui. instance;
			definition. hCursor = LoadCursorW (nullptr, (LPCWSTR) IDC_ARROW);
			definition. hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
			definition. lpszClassName = L"PraatAppearancePanel";
			return RegisterClassW (& definition);
		} () };
		static ATOM chromeClass { [] () {
			WNDCLASSW definition {};
			definition.lpfnWndProc = DefWindowProcW;
			definition.hInstance = theGui.instance;
			definition.lpszClassName = L"PraatAppearanceChrome";
			return RegisterClassW (& definition);
		} () };
		if (! windowClass || ! chromeClass)
			return nullptr;
		EditorAppearancePanel me = new structEditorAppearancePanel ();
		me -> window = nullptr;
		me -> chrome = nullptr;
		me -> callbacks = * callbacks;
		me -> state. palette = kEditorAppearancePalette::ORIGINAL;
		me -> state. opacityPercent = 100;
		me -> font = nullptr;
		me -> labelFont = nullptr;
		me -> hoveredControl = nullptr;
		me -> dpi = 0;
		me -> closedByClickTick = 0;
		me -> travelLeft = 0.0f;
		me -> travelRight = 0.0f;
		me -> dragging = false;
		me -> grabOffset = 0.0f;
		me -> dragChanged = false;
		me -> lastCommittedPercent = me -> state. opacityPercent;
		return me;
	}

	void EditorAppearancePanel_dispose (EditorAppearancePanel me) {
		if (! me)
			return;
		destroyWindow (me);
		delete me;
	}

	bool EditorAppearancePanel_show (EditorAppearancePanel me, void *ownerWindow,
			int anchorLeft, int anchorTop, int anchorWidth, int anchorHeight,
			const EditorAppearancePanel_state *state)
	{
		if (! me || ! ownerWindow)
			return false;
		const HWND owner = (HWND) ownerWindow;
		const int dpi = [&owner] () {
			const UINT windowDpi = GetDpiForWindow (owner);
			return windowDpi ? (int) windowDpi : 96;
		} ();
		if (me -> window && IsWindow (me -> window) && dpi != me -> dpi)
			destroyWindow (me);   // the layout is fixed to the DPI it was created for
		if (! me -> window) {
			me -> dpi = dpi;
			me -> font = CreateFontW (- scaled (13, me -> dpi), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
					DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
					DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
			me -> labelFont = CreateFontW (- scaled (13, me -> dpi), 0, 0, 0, FW_MEDIUM, FALSE, FALSE, FALSE,
					DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
					DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
			me -> width = scaled (PANEL_WIDTH, me -> dpi);
			me -> height = scaled (PADDING + LABEL_HEIGHT + LABEL_GAP + CHIP_HEIGHT + ROW_GAP + LABEL_HEIGHT
					+ SLIDER_GAP + SLIDER_HEIGHT + PADDING, me -> dpi);
			me -> window = CreateWindowExW (WS_EX_TOOLWINDOW, L"PraatAppearancePanel", L"外观",
					WS_POPUP, 0, 0, me -> width, me -> height, owner, nullptr, theGui. instance, me);
			if (! me -> window) {
				destroyWindow (me);
				return false;
			}
			me -> chrome = CreateWindowExW (WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT,
				L"PraatAppearanceChrome", L"", WS_POPUP, 0, 0, 0, 0, owner, nullptr, theGui.instance, nullptr);
			if (! me -> chrome) {
				destroyWindow (me);
				return false;
			}
			createControls (me);
		}
		/*
			Anchor below the rectangle that the menu entry really occupies, then keep the whole panel
			within the work area of the monitor that the menu bar is on.
		*/
		int left = anchorLeft;
		int top = anchorTop + anchorHeight;
		HMONITOR monitor = MonitorFromPoint (POINT { anchorLeft + anchorWidth / 2, anchorTop }, MONITOR_DEFAULTTONEAREST);
		MONITORINFO info;
		info. cbSize = sizeof (MONITORINFO);
		if (GetMonitorInfoW (monitor, & info)) {
			const RECT &work = info. rcWork;
			if (left + me -> width > work. right)
				left = work. right - me -> width;
			if (left < work. left)
				left = work. left;
			if (top + me -> height > work. bottom)
				top = anchorTop - me -> height - 2 > work. top ? anchorTop - me -> height - 2 : work. bottom - me -> height;
			if (top < work. top)
				top = work. top;
		}
		SetWindowLongPtr (me -> window, GWLP_HWNDPARENT, (LONG_PTR) owner);
		SetWindowLongPtr (me -> chrome, GWLP_HWNDPARENT, (LONG_PTR) owner);
		if (! paintChrome (me, left, top))
			return false;
		me -> state = * state;
		me -> hoveredControl = nullptr;
		clearSliderDrag (me);   // an open panel always starts from a handle that is not held
		applyStateToControls (me);
		ShowWindow (me -> chrome, SW_SHOWNOACTIVATE);
		SetWindowPos (me -> window, HWND_TOP, left, top, me -> width, me -> height, SWP_SHOWWINDOW);
		SetWindowPos (me -> chrome, me -> window, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		/*
			The panel is activated and not merely shown: a click anywhere else then deactivates it,
			which is how the panel knows that the user has moved on to something else.
		*/
		SetForegroundWindow (me -> window);
		SendMessageW (me -> window, WM_CHANGEUISTATE, MAKEWPARAM (UIS_SET, UISF_HIDEFOCUS), 0);
		SetFocus (me -> chip [me -> state. palette == kEditorAppearancePalette::CLAY_GREY ? 2 :
				me -> state. palette == kEditorAppearancePalette::MIST_BLUE_GREY ? 1 : 0]);
		me -> closedByClickTick = 0;
		return true;
	}

	void EditorAppearancePanel_dismiss (EditorAppearancePanel me) {
		if (me && me -> window && IsWindow (me -> window))
			hidePanel (me, false);
	}

	void EditorAppearancePanel_setState (EditorAppearancePanel me, const EditorAppearancePanel_state *state) {
		if (! me)
			return;
		me -> state = * state;
		if (me -> window && IsWindow (me -> window))
			applyStateToControls (me);
	}

	bool EditorAppearancePanel_isShowing (EditorAppearancePanel me) {
		return me && me -> window && IsWindowVisible (me -> window);
	}

	bool EditorAppearancePanel_steppedAsideForAClick (EditorAppearancePanel me) {
		if (! me || me -> closedByClickTick == 0)
			return false;
		const DWORD since = GetTickCount () - me -> closedByClickTick;
		return since < 300;   // one press and release on the menu bar never takes longer
	}
#else
	struct structEditorAppearancePanel {
		int unused;
	};

	EditorAppearancePanel EditorAppearancePanel_create (const EditorAppearancePanel_callbacks * /* callbacks */) {
		return nullptr;   // whole-window translucency and its panel are Windows editor features
	}
	void EditorAppearancePanel_dispose (EditorAppearancePanel /* me */) { }
	bool EditorAppearancePanel_show (EditorAppearancePanel /* me */, void * /* ownerWindow */,
			int /* anchorLeft */, int /* anchorTop */, int /* anchorWidth */, int /* anchorHeight */,
			const EditorAppearancePanel_state * /* state */)
	{
		return false;
	}
	void EditorAppearancePanel_dismiss (EditorAppearancePanel /* me */) { }
	void EditorAppearancePanel_setState (EditorAppearancePanel /* me */,
			const EditorAppearancePanel_state * /* state */) { }
	bool EditorAppearancePanel_isShowing (EditorAppearancePanel /* me */) { return false; }
	bool EditorAppearancePanel_steppedAsideForAClick (EditorAppearancePanel /* me */) { return false; }
#endif

/* End of file EditorAppearancePanel.cpp */
