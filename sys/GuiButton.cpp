/* GuiButton.cpp
 *
 * Copyright (C) 1993-2008,2010-2020,2024,2026 Paul Boersma,
 *               2007-2008 Stefan de Konink, 2010 Franz Brausse, 2013 Tom Naughton
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

#include "GuiP.h"

Thing_implement (GuiButton, GuiControl, 0);

#if gtk
	#define iam_button  GuiButton me = (GuiButton) userData
#elif motif
	#define iam_button  GuiButton me = (GuiButton) widget -> userData
#elif cocoa
	#define iam_button  GuiButton me = (GuiButton) self -> d_userData
#endif

#if gtk
	static void _GuiGtkButton_destroyCallback (GuiObject /* widget */, gpointer userData) {
		GuiButton me = (GuiButton) userData;
		trace (U"destroying GuiButton ", Melder_pointer (me));
		forget (me);
	}
	static void _GuiGtkButton_activateCallback (GuiObject widget, gpointer userData) {
		GuiButton me = (GuiButton) userData;
		structGuiButtonEvent event { me, false, false, false };
		if (my d_activateCallback) {
			try {
				my d_activateCallback (my d_activateBoss, & event);
			} catch (MelderError) {
				Melder_flushError (U"Your click on button \"", Melder_peek8to32_u (gtk_widget_get_name (GTK_WIDGET (widget))), U"\" was not completely handled.");
			}
		}
	}
#elif motif
	void _GuiWinButton_destroy (GuiObject widget) {
		iam_button;
		if (widget == widget -> shell -> defaultButton)
			widget -> shell -> defaultButton = nullptr;   // remove dangling reference
		if (widget == widget -> shell -> cancelButton)
			widget -> shell -> cancelButton = nullptr;   // remove dangling reference
		_GuiNativeControl_destroy (widget);
		forget (me);   // NOTE: my widget is not destroyed here
	}
	void _GuiWinButton_handleClick (GuiObject widget) {
		iam_button;
		if (my d_activateCallback) {
			structGuiButtonEvent event { me, false, false, false };
			try {
				my d_activateCallback (my d_activateBoss, & event);
			} catch (MelderError) {
				Melder_flushError (U"Your click on button \"", widget -> name.get(), U"\" was not completely handled.");
			}
		}
	}
	bool _GuiWinButton_tryToHandleShortcutKey (GuiObject widget) {
		iam_button;
		if (my d_activateCallback) {
			structGuiButtonEvent event { me, false, false, false };
			try {
				my d_activateCallback (my d_activateBoss, & event);
			} catch (MelderError) {
				Melder_flushError (U"Your key click on button \"", widget -> name.get(), U"\" was not completely handled.");
			}
			return true;
		}
		return false;
	}

	static const wchar_t *getButtonIconGlyph (const wchar_t *text) {
		if (! text || text [0] == L'\0') return nullptr;
		while (*text == L'+' || *text == 0x207A || *text == L' ' || *text == L'\t') text ++;

		#define MATCHES(t) (wcsstr (text, t) != nullptr)
		if (MATCHES (L"View & Edit") || MATCHES (L"查看与编辑") || MATCHES (L"查看并编辑") ||
		    MATCHES (L"Edit alone") || MATCHES (L"Edit...") || wcscmp (text, L"Edit") == 0 || wcscmp (text, L"编辑") == 0)
			return L"\uE70F";   // Edit pencil

		if (MATCHES (L"Pause") || MATCHES (L"暂停"))
			return L"\uE769";   // Pause double bars

		if (MATCHES (L"Play") || MATCHES (L"播放") || MATCHES (L"Resume") || MATCHES (L"继续"))
			return L"\uE768";   // Play triangle

		if (MATCHES (L"Stop") || MATCHES (L"停止"))
			return L"\uE71A";   // Stop square

		if (MATCHES (L"Rename") || MATCHES (L"重命名"))
			return L"\uE8EC";   // Clean tag icon

		if (MATCHES (L"Copy") || MATCHES (L"复制"))
			return L"\uE8C8";   // Copy pages

		if (MATCHES (L"Remove") || MATCHES (L"删除") || MATCHES (L"移除"))
			return L"\uE74D";   // Trash can

		if (MATCHES (L"Inspect") || MATCHES (L"检查"))
			return L"\uE721";   // Magnifying glass

		if (MATCHES (L"Info") || MATCHES (L"信息"))
			return L"\uE946";   // Info circle

		if (MATCHES (L"Bilingual") || MATCHES (L"双语") || MATCHES (L"对照表") || MATCHES (L"术语对照表"))
			return L"\uF2B7";   // Globe with A and 字 (Bilingual Translation)

		if (MATCHES (L"Tabulate") || MATCHES (L"制表"))
			return L"\uF0E3";   // Tabulate / Report checklist

		if (MATCHES (L"Table") || MATCHES (L"表格"))
			return L"\uE80A";   // Grid table

		if (MATCHES (L"Record") || MATCHES (L"录音") || MATCHES (L"录制"))
			return L"\uE720";   // Microphone

		if (MATCHES (L"Draw") || MATCHES (L"绘制"))
			return L"\uE90A";   // Paint brush

		if (MATCHES (L"Query") || MATCHES (L"查询"))
			return L"\uE721";   // Search

		if (MATCHES (L"Modify") || MATCHES (L"修改"))
			return L"\uE790";   // Settings wrench

		if (MATCHES (L"Apply") || MATCHES (L"应用"))
			return L"\uE895";   // Refresh / Sync / Update (distinct from OK)

		if (wcscmp (text, L"OK") == 0 || wcscmp (text, L"确定") == 0)
			return L"\uE73E";   // OK check

		if (wcscmp (text, L"Cancel") == 0 || wcscmp (text, L"取消") == 0 ||
		    wcscmp (text, L"Close") == 0 || wcscmp (text, L"关闭") == 0)
			return L"\uE711";   // Close X

		if (MATCHES (L"Help") || MATCHES (L"帮助"))
			return L"\uE897";   // Help question mark

		if (MATCHES (L"Save") || MATCHES (L"保存") || MATCHES (L"Write") || MATCHES (L"写入"))
			return L"\uE74E";   // Save disk

		if (MATCHES (L"Open") || MATCHES (L"打开"))
			return L"\uE8E5";   // Folder open

		// Zoom buttons (FunctionEditor bottom-left controls)
		if (wcscmp (text, L"all") == 0 || wcscmp (text, L"全部") == 0 || wcscmp (text, L"All") == 0 ||
		    MATCHES (L"Show all") || MATCHES (L"显示全部"))
			return L"\uE9A6";   // Fit page / Fit to window (enclosing bounds)

		if (wcscmp (text, L"in") == 0 || wcscmp (text, L"放大") == 0 || MATCHES (L"Zoom in"))
			return L"\uE8A3";   // Zoom in (+)

		if (wcscmp (text, L"out") == 0 || wcscmp (text, L"缩小") == 0 || MATCHES (L"Zoom out"))
			return L"\uE71F";   // Zoom out (-)

		if (wcscmp (text, L"sel") == 0 || wcscmp (text, L"选择") == 0 || MATCHES (L"Zoom to selection") || MATCHES (L"缩放到选区"))
			return L"\uE740";   // FullScreen / Expand selection to full window

		if (wcscmp (text, L"bak") == 0 || wcscmp (text, L"返回") == 0 || wcscmp (text, L"back") == 0 || MATCHES (L"Zoom back") || MATCHES (L"缩放返回"))
			return L"\uE72B";   // Back arrow (<-)

		#undef MATCHES
		return nullptr;
	}

	struct GuiButtonWaveform {
		int count;
		float peaks [160];
	};

	static bool getPillRect (HWND hwnd, RECT *rcPill) {
		if (! rcPill) return false;
		HANDLE hL = GetPropW (hwnd, L"PraatPillL");
		HANDLE hR = GetPropW (hwnd, L"PraatPillR");
		HANDLE hT = GetPropW (hwnd, L"PraatPillT");
		HANDLE hB = GetPropW (hwnd, L"PraatPillB");
		if (hL && hR && hT && hB) {
			rcPill -> left   = (int)(intptr_t) hL - 1;
			rcPill -> right  = (int)(intptr_t) hR - 1;
			rcPill -> top    = (int)(intptr_t) hT - 1;
			rcPill -> bottom = (int)(intptr_t) hB - 1;
			if (rcPill -> right > rcPill -> left)
				return true;
		}
		RECT rc;
		GetClientRect (hwnd, & rc);
		bool hasSplit = (GetPropW (hwnd, L"PraatSplitButton") != nullptr);
		int cx = (rc.right - (hasSplit ? 26 : 0)) / 2;
		rcPill -> left   = cx - 35;
		rcPill -> right  = cx + 35;
		rcPill -> top    = rc.top + 2;
		rcPill -> bottom = rc.bottom - 2;
		return true;
	}

	static LRESULT CALLBACK _ModernButtonSubclassProc (
		HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData
	) {
		switch (uMsg) {
			case WM_SETCURSOR: {
				if ((HWND) wParam == hwnd) {
					bool isSeeking = (GetPropW (hwnd, L"PraatAllowSeeking") != nullptr);
					bool hasSplit = (GetPropW (hwnd, L"PraatSplitButton") != nullptr);
					if (isSeeking || hasSplit) {
						POINT pt;
						GetCursorPos (& pt);
						ScreenToClient (hwnd, & pt);
						RECT rc;
						GetClientRect (hwnd, & rc);
						if (PtInRect (& rc, pt)) {
							RECT rcSplit = { rc.right - 26, rc.top, rc.right, rc.bottom };
							if (hasSplit && PtInRect (& rcSplit, pt)) {
								SetCursor (LoadCursor (nullptr, IDC_ARROW));
								return TRUE;
							}
							if (isSeeking) {
								SetCursor (LoadCursor (nullptr, IDC_HAND));
								return TRUE;
							}
						}
					}
				}
				break;
			}

			case WM_MOUSEMOVE: {
				TRACKMOUSEEVENT tme;
				tme.cbSize = sizeof (TRACKMOUSEEVENT);
				tme.dwFlags = TME_LEAVE;
				tme.hwndTrack = hwnd;
				tme.dwHoverTime = 0;
				TrackMouseEvent (& tme);

				if (! GetPropW (hwnd, L"PraatHover")) {
					SetPropW (hwnd, L"PraatHover", (HANDLE) 1);
					InvalidateRect (hwnd, nullptr, FALSE);
				}

				POINT pt = { (short) LOWORD (lParam), (short) HIWORD (lParam) };
				RECT rc;
				GetClientRect (hwnd, & rc);
				bool hasSplit = (GetPropW (hwnd, L"PraatSplitButton") != nullptr);
				bool isSeeking = (GetPropW (hwnd, L"PraatAllowSeeking") != nullptr);
				RECT rcSplit = { rc.right - 26, rc.top, rc.right, rc.bottom };

				if (hasSplit && PtInRect (& rcSplit, pt)) {
					if (! GetPropW (hwnd, L"PraatSplitHover")) {
						SetPropW (hwnd, L"PraatSplitHover", (HANDLE) 1);
						InvalidateRect (hwnd, nullptr, FALSE);
					}
					if (GetPropW (hwnd, L"PraatPillHover")) {
						RemovePropW (hwnd, L"PraatPillHover");
						InvalidateRect (hwnd, nullptr, FALSE);
					}
					if (GetPropW (hwnd, L"PraatHoverX")) {
						RemovePropW (hwnd, L"PraatHoverX");
						InvalidateRect (hwnd, nullptr, FALSE);
					}
				} else {
					if (GetPropW (hwnd, L"PraatSplitHover")) {
						RemovePropW (hwnd, L"PraatSplitHover");
						InvalidateRect (hwnd, nullptr, FALSE);
					}
					if (isSeeking) {
						RECT rcPill;
						getPillRect (hwnd, & rcPill);
						if (PtInRect (& rcPill, pt)) {
							if (! GetPropW (hwnd, L"PraatPillHover")) {
								SetPropW (hwnd, L"PraatPillHover", (HANDLE) 1);
								InvalidateRect (hwnd, nullptr, FALSE);
							}
							if (GetPropW (hwnd, L"PraatHoverX")) {
								RemovePropW (hwnd, L"PraatHoverX");
								InvalidateRect (hwnd, nullptr, FALSE);
							}
						} else {
							if (GetPropW (hwnd, L"PraatPillHover")) {
								RemovePropW (hwnd, L"PraatPillHover");
								InvalidateRect (hwnd, nullptr, FALSE);
							}
							int prevHx = (int)(intptr_t) GetPropW (hwnd, L"PraatHoverX");
							if (prevHx != pt.x + 1) {
								SetPropW (hwnd, L"PraatHoverX", (HANDLE)(intptr_t)(pt.x + 1));
								InvalidateRect (hwnd, nullptr, FALSE);
							}
						}
					} else {
						if (GetPropW (hwnd, L"PraatPillHover")) {
							RemovePropW (hwnd, L"PraatPillHover");
							InvalidateRect (hwnd, nullptr, FALSE);
						}
						if (GetPropW (hwnd, L"PraatHoverX")) {
							RemovePropW (hwnd, L"PraatHoverX");
							InvalidateRect (hwnd, nullptr, FALSE);
						}
					}
				}

				if (GetPropW (hwnd, L"PraatDraggingSeek")) {
					GuiButton_SeekCallback seekCb = (GuiButton_SeekCallback) GetPropW (hwnd, L"PraatSeekCb");
					if (seekCb) {
						float padX = 8.0f;
						float availW = (float) rc.right - (hasSplit ? 26.0f : 0.0f) - 2.0f * padX;
						double frac = (availW > 1.0f) ? ((double) pt.x - padX) / (double) availW : 0.0;
						if (frac < 0.0) frac = 0.0;
						if (frac > 1.0) frac = 1.0;
						GuiObject widget = (GuiObject) GetWindowLongPtr (hwnd, GWLP_USERDATA);
						GuiButton btn = widget ? (GuiButton) _GuiObject_getUserData (widget) : nullptr;
						seekCb (btn, (void *) hwnd, frac, true);
					}
				}

				LRESULT res = DefSubclassProc (hwnd, uMsg, wParam, lParam);
				if (GetCapture () == hwnd) {
					InvalidateRect (hwnd, nullptr, FALSE);
					UpdateWindow (hwnd);
				}
				return res;
			}
			case WM_MOUSELEAVE: {
				RemovePropW (hwnd, L"PraatHover");
				RemovePropW (hwnd, L"PraatSplitHover");
				RemovePropW (hwnd, L"PraatSplitPressed");
				RemovePropW (hwnd, L"PraatPillHover");
				RemovePropW (hwnd, L"PraatPillPressed");
				RemovePropW (hwnd, L"PraatHoverX");
				InvalidateRect (hwnd, nullptr, FALSE);
				break;
			}
			case WM_LBUTTONDOWN: {
				POINT pt = { (short) LOWORD (lParam), (short) HIWORD (lParam) };
				RECT rc;
				GetClientRect (hwnd, & rc);
				bool hasSplit = (GetPropW (hwnd, L"PraatSplitButton") != nullptr);
				bool isSeeking = (GetPropW (hwnd, L"PraatAllowSeeking") != nullptr);
				RECT rcSplit = { rc.right - 26, rc.top, rc.right, rc.bottom };

				if (hasSplit && PtInRect (& rcSplit, pt)) {
					SetPropW (hwnd, L"PraatSplitPressed", (HANDLE) 1);
					InvalidateRect (hwnd, nullptr, FALSE);
					UpdateWindow (hwnd);
					POINT ptScreen = pt;
					ClientToScreen (hwnd, & ptScreen);
					GuiButton_SplitCallback splitCb = (GuiButton_SplitCallback) GetPropW (hwnd, L"PraatSplitCb");
					if (splitCb) {
						GuiObject widget = (GuiObject) GetWindowLongPtr (hwnd, GWLP_USERDATA);
						GuiButton btn = widget ? (GuiButton) _GuiObject_getUserData (widget) : nullptr;
						splitCb (btn, (void *) hwnd, ptScreen.x, ptScreen.y);
					}
					if (! IsWindow (hwnd))
						return 0;
					RemovePropW (hwnd, L"PraatSplitPressed");
					InvalidateRect (hwnd, nullptr, FALSE);
					UpdateWindow (hwnd);
					return 0;
				}

				if (isSeeking) {
					RECT rcPill;
					getPillRect (hwnd, & rcPill);
					if (PtInRect (& rcPill, pt)) {
						SetPropW (hwnd, L"PraatPillPressed", (HANDLE) 1);
						LRESULT res = DefSubclassProc (hwnd, uMsg, wParam, lParam);
						InvalidateRect (hwnd, nullptr, FALSE);
						UpdateWindow (hwnd);
						return res;
					} else {
						SetCapture (hwnd);
						SetPropW (hwnd, L"PraatDraggingSeek", (HANDLE) 1);
						GuiButton_SeekCallback seekCb = (GuiButton_SeekCallback) GetPropW (hwnd, L"PraatSeekCb");
						if (seekCb) {
							float padX = 8.0f;
							float availW = (float) rc.right - (hasSplit ? 26.0f : 0.0f) - 2.0f * padX;
							double frac = (availW > 1.0f) ? ((double) pt.x - padX) / (double) availW : 0.0;
							if (frac < 0.0) frac = 0.0;
							if (frac > 1.0) frac = 1.0;
							GuiObject widget = (GuiObject) GetWindowLongPtr (hwnd, GWLP_USERDATA);
							GuiButton btn = widget ? (GuiButton) _GuiObject_getUserData (widget) : nullptr;
							seekCb (btn, (void *) hwnd, frac, true);
						}
						InvalidateRect (hwnd, nullptr, FALSE);
						UpdateWindow (hwnd);
						return 0;
					}
				}

				LRESULT res = DefSubclassProc (hwnd, uMsg, wParam, lParam);
				InvalidateRect (hwnd, nullptr, FALSE);
				UpdateWindow (hwnd);
				return res;
			}
			case WM_LBUTTONUP: {
				if (GetPropW (hwnd, L"PraatDraggingSeek")) {
					ReleaseCapture ();
					RemovePropW (hwnd, L"PraatDraggingSeek");
					POINT pt = { (short) LOWORD (lParam), (short) HIWORD (lParam) };
					RECT rc;
					GetClientRect (hwnd, & rc);
					bool hasSplit = (GetPropW (hwnd, L"PraatSplitButton") != nullptr);
					GuiButton_SeekCallback seekCb = (GuiButton_SeekCallback) GetPropW (hwnd, L"PraatSeekCb");
					if (seekCb) {
						float padX = 8.0f;
						float availW = (float) rc.right - (hasSplit ? 26.0f : 0.0f) - 2.0f * padX;
						double frac = (availW > 1.0f) ? ((double) pt.x - padX) / (double) availW : 0.0;
						if (frac < 0.0) frac = 0.0;
						if (frac > 1.0) frac = 1.0;
						GuiObject widget = (GuiObject) GetWindowLongPtr (hwnd, GWLP_USERDATA);
						GuiButton btn = widget ? (GuiButton) _GuiObject_getUserData (widget) : nullptr;
						seekCb (btn, (void *) hwnd, frac, false);
					}
					InvalidateRect (hwnd, nullptr, FALSE);
					UpdateWindow (hwnd);
					return 0;
				}
				if (GetPropW (hwnd, L"PraatPillPressed")) {
					RemovePropW (hwnd, L"PraatPillPressed");
					LRESULT res = DefSubclassProc (hwnd, uMsg, wParam, lParam);
					InvalidateRect (hwnd, nullptr, FALSE);
					UpdateWindow (hwnd);
					return res;
				}
				LRESULT res = DefSubclassProc (hwnd, uMsg, wParam, lParam);
				InvalidateRect (hwnd, nullptr, FALSE);
				UpdateWindow (hwnd);
				return res;
			}
			case WM_CAPTURECHANGED: {
				RemovePropW (hwnd, L"PraatDraggingSeek");
				RemovePropW (hwnd, L"PraatPillPressed");
				RemovePropW (hwnd, L"PraatSplitPressed");
				LRESULT res = DefSubclassProc (hwnd, uMsg, wParam, lParam);
				InvalidateRect (hwnd, nullptr, FALSE);
				UpdateWindow (hwnd);
				return res;
			}
			case BM_SETSTATE: {
				LRESULT res = DefSubclassProc (hwnd, uMsg, wParam, lParam);
				InvalidateRect (hwnd, nullptr, FALSE);
				UpdateWindow (hwnd);
				return res;
			}
			case WM_TIMER:
				return 0;   // Prevent Windows default button pulse / blink animation timers
			case WM_SETFOCUS:
			case WM_KILLFOCUS:
			case WM_ENABLE: {
				LRESULT res = DefSubclassProc (hwnd, uMsg, wParam, lParam);
				InvalidateRect (hwnd, nullptr, FALSE);
				return res;
			}

			case WM_ERASEBKGND:
				return 1;   // Double-buffered in WM_PAINT to prevent flicker

			case WM_PAINT: {
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint (hwnd, & ps);
				if (! hdc)
					return 0;

				RECT rc;
				GetClientRect (hwnd, & rc);
				if (rc.right <= 0 || rc.bottom <= 0) {
					EndPaint (hwnd, & ps);
					return 0;
				}

				// Double buffering: create memory DC & bitmap
				HDC memDC = CreateCompatibleDC (hdc);
				HBITMAP memBmp = CreateCompatibleBitmap (hdc, rc.right, rc.bottom);
				HBITMAP oldBmp = (HBITMAP) SelectObject (memDC, memBmp);

				// Fill background with window background color so rounded corners blend seamlessly
				FillRect (memDC, & rc, theWinGuiBackgroundBrush ());

				bool isEnabled = IsWindowEnabled (hwnd);
				bool isPressed = (SendMessage (hwnd, BM_GETSTATE, 0, 0) & BST_PUSHED) != 0 && isEnabled;

				POINT pt;
				GetCursorPos (& pt);
				ScreenToClient (hwnd, & pt);
				bool isCursorInside = PtInRect (& rc, pt);

				bool isHovered = isCursorInside && isEnabled;
				bool isFocus   = (GetFocus () == hwnd) && isEnabled;
				bool isDefault = (dwRefData & (GuiButton_DEFAULT | GuiButton_ATTRACTIVE)) != 0;

				COLORREF bgCol, borderCol, textCol, bottomLineCol;
				if (! isEnabled) {
					bgCol         = RGB (248, 249, 250);   // #F8F9FA
					borderCol     = RGB (226, 232, 240);   // #E2E8F0
					textCol       = RGB (156, 163, 175);   // #9CA3AF
					bottomLineCol = borderCol;
				} else if (isDefault) {
					// Windows 11 Fluent Primary Blue Accent
					if (isPressed) {
						bgCol     = RGB (0, 95, 184);      // #005FB8
						borderCol = RGB (0, 78, 152);
					} else if (isHovered) {
						bgCol     = RGB (24, 115, 196);    // #1873C4
						borderCol = RGB (0, 95, 184);
					} else {
						bgCol     = RGB (0, 103, 192);     // #0067C0
						borderCol = RGB (0, 95, 184);
					}
					textCol       = RGB (255, 255, 255);
					bottomLineCol = borderCol;
				} else {
					// Modern Flat Card Button
					if (isPressed) {
						bgCol         = RGB (226, 232, 240);   // #E2E8F0
						borderCol     = RGB (148, 163, 184);   // #94A3B8
						bottomLineCol = borderCol;
					} else if (isHovered) {
						bgCol         = RGB (241, 245, 249);   // #F1F5F9
						borderCol     = RGB (148, 163, 184);   // #94A3B8
						bottomLineCol = borderCol;
					} else if (isFocus) {
						bgCol         = RGB (255, 255, 255);   // #FFFFFF
						borderCol     = RGB (0, 103, 192);     // #0067C0 Fluent Blue border
						bottomLineCol = borderCol;
					} else {
						bgCol         = RGB (255, 255, 255);   // #FFFFFF Crisp Card
						borderCol     = RGB (209, 213, 219);   // #D1D5DB Subtle Border
						bottomLineCol = RGB (190, 195, 203);   // Subtle 3D depth
					}
					textCol = RGB (31, 41, 55);                // #1F2937 Clean Dark Slate
				}

				// Draw modern rounded rectangle with GDI+ for perfectly uniform borders on all 4 sides
				_GuiWin_ensureGdiplus ();
				{
					Gdiplus::Graphics g (memDC);
					g.SetSmoothingMode (Gdiplus::SmoothingModeAntiAlias);
					g.SetPixelOffsetMode (Gdiplus::PixelOffsetModeHighQuality);

					float strokeW = isFocus ? 1.5f : 1.0f;
					float x = strokeW / 2.0f;
					float y = strokeW / 2.0f;
					float w = (float) rc.right - strokeW;
					float h = (float) rc.bottom - strokeW;
					float r = 5.0f;
					if (r * 2.0f > h) r = h / 2.0f;
					if (r * 2.0f > w) r = w / 2.0f;

					Gdiplus::GraphicsPath path;
					path.AddArc (x, y, r * 2.0f, r * 2.0f, 180.0f, 90.0f);
					path.AddArc (x + w - r * 2.0f, y, r * 2.0f, r * 2.0f, 270.0f, 90.0f);
					path.AddArc (x + w - r * 2.0f, y + h - r * 2.0f, r * 2.0f, r * 2.0f, 0.0f, 90.0f);
					path.AddArc (x, y + h - r * 2.0f, r * 2.0f, r * 2.0f, 90.0f, 90.0f);
					path.CloseFigure ();

					Gdiplus::SolidBrush bgBrush (Gdiplus::Color (255, GetRValue (bgCol), GetGValue (bgCol), GetBValue (bgCol)));
					Gdiplus::Pen borderPen (Gdiplus::Color (255, GetRValue (borderCol), GetGValue (borderCol), GetBValue (borderCol)), strokeW);

					g.FillPath (& bgBrush, & path);

					// Render waveform or visual progress bar
					GuiButtonWaveform *wf = (GuiButtonWaveform *) GetPropW (hwnd, L"PraatWaveform");
					HANDLE hProg = GetPropW (hwnd, L"PraatProgress");
					float frac = 0.0f;
					int progState = 0;
					if (hProg) {
						int progVal = (int)(intptr_t) hProg;
						if (progVal > 0) {
							frac = (float) progVal / 10000.0f;
							if (frac > 1.0f) frac = 1.0f;
						}
						progState = (int)(intptr_t) GetPropW (hwnd, L"PraatProgressState");
					}
					bool isPausedState = (progState == 2);

					bool hasSplit = (GetPropW (hwnd, L"PraatSplitButton") != nullptr);
					bool isSeeking = (GetPropW (hwnd, L"PraatAllowSeeking") != nullptr);
					float splitW = hasSplit ? 26.0f : 0.0f;
					float mainW = (float) rc.right - splitW;

					if (wf && wf -> count > 0) {
						g.SetClip (& path);

						float padX = 8.0f;
						float availW = mainW - 2.0f * padX;
						float playheadX = padX + (availW > 1.0f ? availW * frac : 0.0f);

						// 1. If actively playing or paused, draw soft background progress tint
						if (frac > 0.0f) {
							Gdiplus::Color tintCol = isPausedState
								? Gdiplus::Color (22, 217, 119, 6)    // Soft amber tint
								: Gdiplus::Color (22, 16, 124, 65);   // Soft emerald tint
							Gdiplus::SolidBrush tintBrush (tintCol);
							g.FillRectangle (& tintBrush, 0.0f, 0.0f, playheadX, (float) rc.bottom);
						}

						// 2. Waveform capsule bars
						int n = wf -> count;
						if (availW > 10.0f && n > 0) {
							float slotW = availW / (float) n;
							float barW = slotW * 0.65f;
							if (barW < 1.5f) barW = 1.5f;
							if (barW > 3.5f) barW = 3.5f;
							float midY = (float) rc.bottom / 2.0f + (isPressed ? 1.0f : 0.0f);
							float maxH = (float) rc.bottom * 0.76f;
							float minH = 2.5f;

							Gdiplus::Color unplayedCol = isHovered
								? Gdiplus::Color (120, 140, 150, 165)
								: Gdiplus::Color (95, 156, 163, 175);
							Gdiplus::Color playedCol = isPausedState
								? Gdiplus::Color (190, 217, 119, 6)    // Amber orange
								: Gdiplus::Color (190, 16, 124, 65);   // Emerald green

							Gdiplus::Pen unplayedPen (unplayedCol, barW);
							unplayedPen.SetStartCap (Gdiplus::LineCapRound);
							unplayedPen.SetEndCap (Gdiplus::LineCapRound);

							Gdiplus::Pen playedPen (playedCol, barW);
							playedPen.SetStartCap (Gdiplus::LineCapRound);
							playedPen.SetEndCap (Gdiplus::LineCapRound);

							for (int i = 0; i < n; ++ i) {
								float barCenterX = padX + ((float) i + 0.5f) * slotW;
								float val = wf -> peaks [i];
								if (val < 0.0f) val = 0.0f;
								if (val > 1.0f) val = 1.0f;
								float curH = minH + (maxH - minH) * val;
								float halfH = curH / 2.0f;
								if (halfH < barW / 2.0f) halfH = barW / 2.0f;

								bool isPlayed = (frac > 0.0f && barCenterX <= playheadX);
								Gdiplus::Pen *curPen = isPlayed ? & playedPen : & unplayedPen;

								float y1 = midY - halfH + barW / 2.0f;
								float y2 = midY + halfH - barW / 2.0f;
								if (y2 < y1) y2 = y1;
								g.DrawLine (curPen, barCenterX, y1, barCenterX, y2);
							}
						}

						// 3. Playhead cursor & bottom progress line
						if (frac > 0.0f) {
							Gdiplus::Color cursorCol = isPausedState
								? Gdiplus::Color (220, 217, 119, 6)
								: Gdiplus::Color (220, 16, 124, 65);
							Gdiplus::Pen cursorPen (cursorCol, 1.5f);
							g.DrawLine (& cursorPen, playheadX, 2.0f, playheadX, (float) rc.bottom - 4.0f);

							Gdiplus::SolidBrush barBrush (cursorCol);
							float barH = 3.0f;
							g.FillRectangle (& barBrush, 0.0f, (float) rc.bottom - barH, playheadX, barH);
						}

						g.ResetClip ();
					} else if (hProg && frac > 0.0f) {
						g.SetClip (& path);

						Gdiplus::Color tintCol = isPausedState
							? Gdiplus::Color (38, 217, 119, 6)    // Soft Amber
							: Gdiplus::Color (38, 16, 124, 65);   // Soft Emerald
						Gdiplus::SolidBrush tintBrush (tintCol);
						g.FillRectangle (& tintBrush, 0.0f, 0.0f, mainW * frac, (float) rc.bottom);

						Gdiplus::Color barCol = isPausedState
							? Gdiplus::Color (235, 217, 119, 6)   // Amber accent
							: Gdiplus::Color (235, 16, 124, 65);  // Emerald accent
						Gdiplus::SolidBrush barBrush (barCol);
						float barH = 3.0f;
						g.FillRectangle (& barBrush, 0.0f, (float) rc.bottom - barH, mainW * frac, barH);

						g.ResetClip ();
					}

					// 4. Hover seek indicator line
					if (isSeeking) {
						HANDLE hHover = GetPropW (hwnd, L"PraatHoverX");
						if (hHover) {
							int hx = (int)(intptr_t) hHover - 1;
							float padX = 8.0f;
							float availW = mainW - 2.0f * padX;
							if (hx >= (int) padX && hx <= (int)(padX + availW)) {
								g.SetClip (& path);

								// Soft Fluent accent glow column (5px wide)
								Gdiplus::SolidBrush hGlow (Gdiplus::Color (45, 0, 103, 192));
								g.FillRectangle (& hGlow, (float) hx - 2.0f, 2.0f, 5.0f, (float) rc.bottom - 4.0f);

								// Sharp vertical guideline (Fluent accent blue, 1.5px width)
								Gdiplus::Pen hPen (Gdiplus::Color (230, 0, 103, 192), 1.5f);
								g.DrawLine (& hPen, (float) hx, 2.0f, (float) hx, (float) rc.bottom - 4.0f);

								// Top and bottom guide notches/caps
								Gdiplus::SolidBrush capBrush (Gdiplus::Color (255, 0, 103, 192));
								g.FillRectangle (& capBrush, (float) hx - 2.0f, 2.0f, 5.0f, 3.0f);
								g.FillRectangle (& capBrush, (float) hx - 2.0f, (float) rc.bottom - 5.0f, 5.0f, 3.0f);

								g.ResetClip ();
							}
						}
					}

					g.DrawPath (& borderPen, & path);

					// Render split sub-button if enabled
					if (hasSplit) {
						float divX = (float) rc.right - 26.0f;
						Gdiplus::Color divCol = Gdiplus::Color (80, 209, 213, 219);
						Gdiplus::Pen divPen (divCol, 1.0f);
						g.DrawLine (& divPen, divX, 4.0f, divX, (float) rc.bottom - 4.0f);

						bool isSplitHover = (GetPropW (hwnd, L"PraatSplitHover") != nullptr);
						bool isSplitPressed = (GetPropW (hwnd, L"PraatSplitPressed") != nullptr);

						if (isSplitHover || isSplitPressed) {
							Gdiplus::Color shCol = isSplitPressed
								? Gdiplus::Color (45, 0, 103, 192)
								: Gdiplus::Color (20, 0, 103, 192);
							Gdiplus::SolidBrush shBrush (shCol);
							g.FillRectangle (& shBrush, divX + 1.0f, 1.0f, 24.0f, (float) rc.bottom - 2.0f);
						}

						float arrCx = (float) rc.right - 13.0f;
						float arrCy = (float) rc.bottom / 2.0f + (isSplitPressed ? 1.0f : 0.0f);
						Gdiplus::PointF pts [3] = {
							{ arrCx - 3.5f, arrCy - 1.5f },
							{ arrCx,        arrCy + 2.0f },
							{ arrCx + 3.5f, arrCy - 1.5f }
						};
						Gdiplus::Pen arrPen (isSplitHover ? Gdiplus::Color (255, 0, 103, 192) : Gdiplus::Color (170, 100, 116, 139), 1.5f);
						arrPen.SetStartCap (Gdiplus::LineCapRound);
						arrPen.SetEndCap (Gdiplus::LineCapRound);
						arrPen.SetLineJoin (Gdiplus::LineJoinRound);
						g.DrawLines (& arrPen, pts, 3);
					}
				}

				// Draw Button Text & Icon
				WCHAR textBuf [512];
				int textLen = GetWindowTextW (hwnd, textBuf, 512);
				if (textLen > 0) {
					// Collapse '&&' (expanded by _GuiWin_expandAmpersands) back to single '&' because we draw with DT_NOPREFIX
					WCHAR *dst = textBuf;
					for (const WCHAR *src = textBuf; *src != L'\0'; ++ src) {
						if (src [0] == L'&' && src [1] == L'&')
							++ src;
						*dst ++ = *src;
					}
					*dst = L'\0';
					textLen = (int) (dst - textBuf);

					HFONT hFont = (isDefault ? theWinGuiBoldLabelFont () : theWinGuiNormalLabelFont ());
					HFONT oldFont = (HFONT) SelectObject (memDC, hFont);
					SetBkMode (memDC, TRANSPARENT);
					SetTextColor (memDC, textCol);

					RECT textRc = rc;
					bool hasSplit = (GetPropW (hwnd, L"PraatSplitButton") != nullptr);
					bool isSeeking = (GetPropW (hwnd, L"PraatAllowSeeking") != nullptr);
					float splitW = hasSplit ? 26.0f : 0.0f;
					float mainW = (float) rc.right - splitW;
					textRc.right = (int) mainW;

					int boxW = textRc.right - textRc.left;
					const bool isCompact = (boxW <= 48);
					// Inset slightly to prevent text clipping
					InflateRect (& textRc, isCompact ? -2 : -3, 0);
					if (isPressed)
						OffsetRect (& textRc, 0, 1);

					GuiButtonWaveform *wf = (GuiButtonWaveform *) GetPropW (hwnd, L"PraatWaveform");
					const wchar_t *iconGlyph = getButtonIconGlyph (textBuf);
					if (iconGlyph && ! (dwRefData & GuiButton_MULTILINE)) {
						const int iconW = isCompact ? 11 : 13;
						const int gap = isCompact ? 2 : (boxW <= 64 ? 4 : 6);
						SIZE szText;
						GetTextExtentPoint32W (memDC, textBuf, textLen, & szText);
						int totalW = iconW + gap + szText.cx;
						int innerBoxW = textRc.right - textRc.left;
						int startX = textRc.left + (innerBoxW - totalW) / 2;
						if (startX < textRc.left + 1)
							startX = textRc.left + 1;

						// If waveform is present, draw frosted-glass capsule pill behind text & icon
						if (wf && wf -> count > 0) {
							float pillPadX = 10.0f;
							float pillPadY = 4.0f;
							float pillW = (float) totalW + pillPadX * 2.0f;
							float pillH = (float) szText.cy + pillPadY * 2.0f;
							float pillX = (float) startX - pillPadX;
							if (pillX < 2.0f) pillX = 2.0f;
							if (pillX + pillW > mainW - 2.0f) pillW = mainW - 2.0f - pillX;
							float pillY = ((float) rc.bottom - pillH) / 2.0f + (isPressed ? 1.0f : 0.0f);
							float pillR = pillH / 2.0f;
							if (pillR * 2.0f > pillW) pillR = pillW / 2.0f;

							SetPropW (hwnd, L"PraatPillL", (HANDLE)(intptr_t)((int) pillX + 1));
							SetPropW (hwnd, L"PraatPillR", (HANDLE)(intptr_t)((int)(pillX + pillW) + 1));
							SetPropW (hwnd, L"PraatPillT", (HANDLE)(intptr_t)((int) pillY + 1));
							SetPropW (hwnd, L"PraatPillB", (HANDLE)(intptr_t)((int)(pillY + pillH) + 1));

							bool isPillHover = (GetPropW (hwnd, L"PraatPillHover") != nullptr);

							Gdiplus::Graphics gPill (memDC);
							gPill.SetSmoothingMode (Gdiplus::SmoothingModeAntiAlias);
							Gdiplus::GraphicsPath pillPath;
							pillPath.AddArc (pillX, pillY, pillR * 2.0f, pillR * 2.0f, 90.0f, 180.0f);
							pillPath.AddArc (pillX + pillW - pillR * 2.0f, pillY, pillR * 2.0f, pillR * 2.0f, 270.0f, 180.0f);
							pillPath.CloseFigure ();

							Gdiplus::SolidBrush pillBg ((isSeeking && isPillHover)
								? Gdiplus::Color (248, 255, 255, 255)
								: Gdiplus::Color (225, 255, 255, 255));
							Gdiplus::Pen pillBorder ((isSeeking && isPillHover)
								? Gdiplus::Color (255, 0, 103, 192)
								: Gdiplus::Color (140, 203, 213, 225), (isSeeking && isPillHover) ? 1.5f : 1.0f);
							gPill.FillPath (& pillBg, & pillPath);
							gPill.DrawPath (& pillBorder, & pillPath);
						}

						// 1. Draw Icon
						HFONT hIconFont = theWinGuiIconFont (isCompact ? -10 : (rc.bottom >= 40 ? -13 : -12));
						SelectObject (memDC, hIconFont);
						RECT rcIcon = { startX, textRc.top, startX + iconW, textRc.bottom };
						HANDLE hProgIcon = GetPropW (hwnd, L"PraatProgress");
						if (hProgIcon) {
							int progState = (int)(intptr_t) GetPropW (hwnd, L"PraatProgressState");
							if (progState == 2)
								SetTextColor (memDC, RGB (217, 119, 6)); // Amber icon for paused
							else if (progState == 1)
								SetTextColor (memDC, RGB (16, 124, 65));  // Emerald icon for playing
						}
						DrawTextW (memDC, iconGlyph, -1, & rcIcon, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
						SetTextColor (memDC, textCol);

						// 2. Draw Text
						SelectObject (memDC, hFont);
						RECT rcLabel = { startX + iconW + gap, textRc.top, textRc.right - 1, textRc.bottom };
						DrawTextW (memDC, textBuf, -1, & rcLabel, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
					} else {
						if (wf && wf -> count > 0) {
							SIZE szText;
							GetTextExtentPoint32W (memDC, textBuf, textLen, & szText);
							float pillPadX = 10.0f;
							float pillPadY = 4.0f;
							float pillW = (float) szText.cx + pillPadX * 2.0f;
							float pillH = (float) szText.cy + pillPadY * 2.0f;
							float pillX = ((float) mainW - pillW) / 2.0f;
							if (pillX < 2.0f) pillX = 2.0f;
							if (pillX + pillW > mainW - 2.0f) pillW = mainW - 2.0f - pillX;
							float pillY = ((float) rc.bottom - pillH) / 2.0f + (isPressed ? 1.0f : 0.0f);
							float pillR = pillH / 2.0f;
							if (pillR * 2.0f > pillW) pillR = pillW / 2.0f;

							SetPropW (hwnd, L"PraatPillL", (HANDLE)(intptr_t) pillX);
							SetPropW (hwnd, L"PraatPillR", (HANDLE)(intptr_t)(pillX + pillW));
							SetPropW (hwnd, L"PraatPillT", (HANDLE)(intptr_t) pillY);
							SetPropW (hwnd, L"PraatPillB", (HANDLE)(intptr_t)(pillY + pillH));

							bool isPillHover = (GetPropW (hwnd, L"PraatPillHover") != nullptr);

							Gdiplus::Graphics gPill (memDC);
							gPill.SetSmoothingMode (Gdiplus::SmoothingModeAntiAlias);
							Gdiplus::GraphicsPath pillPath;
							pillPath.AddArc (pillX, pillY, pillR * 2.0f, pillR * 2.0f, 90.0f, 180.0f);
							pillPath.AddArc (pillX + pillW - pillR * 2.0f, pillY, pillR * 2.0f, pillR * 2.0f, 270.0f, 180.0f);
							pillPath.CloseFigure ();

							Gdiplus::SolidBrush pillBg ((isSeeking && isPillHover)
								? Gdiplus::Color (248, 255, 255, 255)
								: Gdiplus::Color (225, 255, 255, 255));
							Gdiplus::Pen pillBorder ((isSeeking && isPillHover)
								? Gdiplus::Color (255, 0, 103, 192)
								: Gdiplus::Color (140, 203, 213, 225), (isSeeking && isPillHover) ? 1.5f : 1.0f);
							gPill.FillPath (& pillBg, & pillPath);
							gPill.DrawPath (& pillBorder, & pillPath);
						}

						UINT drawFlags = DT_CENTER | DT_VCENTER;
						if (dwRefData & GuiButton_MULTILINE)
							drawFlags |= DT_WORDBREAK;
						else
							drawFlags |= DT_SINGLELINE | DT_NOPREFIX;

						DrawTextW (memDC, textBuf, -1, & textRc, drawFlags);
					}

					SelectObject (memDC, oldFont);
				}

				BitBlt (hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);

				SelectObject (memDC, oldBmp);
				DeleteObject (memBmp);
				DeleteDC (memDC);
				EndPaint (hwnd, & ps);
				return 0;
			}

			case WM_NCDESTROY: {
				RemovePropW (hwnd, L"PraatHover");
				RemovePropW (hwnd, L"PraatProgress");
				RemovePropW (hwnd, L"PraatProgressState");
				RemovePropW (hwnd, L"PraatSplitButton");
				RemovePropW (hwnd, L"PraatSplitHover");
				RemovePropW (hwnd, L"PraatSplitPressed");
				RemovePropW (hwnd, L"PraatAllowSeeking");
				RemovePropW (hwnd, L"PraatPillHover");
				RemovePropW (hwnd, L"PraatPillPressed");
				RemovePropW (hwnd, L"PraatHoverX");
				RemovePropW (hwnd, L"PraatDraggingSeek");
				RemovePropW (hwnd, L"PraatPillL");
				RemovePropW (hwnd, L"PraatPillR");
				RemovePropW (hwnd, L"PraatPillT");
				RemovePropW (hwnd, L"PraatPillB");
				RemovePropW (hwnd, L"PraatSplitCb");
				RemovePropW (hwnd, L"PraatSeekCb");
				GuiButtonWaveform *wf = (GuiButtonWaveform *) RemovePropW (hwnd, L"PraatWaveform");
				if (wf)
					delete wf;
				RemoveWindowSubclass (hwnd, _ModernButtonSubclassProc, uIdSubclass);
				break;
			}
			default: break;
		}
		return DefSubclassProc (hwnd, uMsg, wParam, lParam);
	}

	void _GuiWin_subclassModernButton (HWND hwnd, uint32 flags) {
		if (hwnd)
			SetWindowSubclass (hwnd, _ModernButtonSubclassProc, 1, (DWORD_PTR) flags);
	}
#elif cocoa
	@implementation GuiCocoaButton {
		GuiButton d_userData;
	}
	- (void) dealloc {   // override
		GuiButton me = d_userData;
		forget (me);
		trace (U"deleting a button");
		[super dealloc];
	}
	- (GuiThing) getUserData {
		return d_userData;
	}
	- (void) setUserData: (GuiThing) userData {
		Melder_assert (userData == nullptr || Thing_isa (userData, classGuiButton));
		d_userData = static_cast <GuiButton> (userData);
	}
	- (void) _guiCocoaButton_activateCallback: (id) widget {
		Melder_assert (self == widget);   // sender (widget) and receiver (self) happen to be the same object
		GuiButton me = d_userData;
		if (my d_activateCallback) {
			structGuiButtonEvent event { me, false, false, false };
			try {
				my d_activateCallback (my d_activateBoss, & event);
			} catch (MelderError) {
				Melder_flushError (U"Your click on button \"", my name.get(), U"\" was not completely handled.");
			}
		}
	}
	@end
#endif

static void gui_blocking_dialog_cb_ok (GuiDialog me, GuiButtonEvent event) {
	my clickedButtonId = event -> button -> d_sequentalIdInDialog;
	#if gtk
		gtk_dialog_response (GTK_DIALOG (my d_gtkWindow), 0);   // or `my clickedButtonId`, but return value of gtk_dialog_run is ignored
	#elif cocoa
		[NSApp stopModal];
	#endif
}
GuiButton GuiButton_create (GuiForm parent, int left, int right, int top, int bottom,
	conststring32 buttonText, GuiButton_ActivateCallback activateCallback, Thing activateBoss, uint32 flags)
{
	conststring32 translatedText = praat_translate (buttonText);
	autoGuiButton me = Thing_new (GuiButton);
	my d_shell = parent -> d_shell;
	my d_parent = parent;
	my d_activateCallback = activateCallback;
	my d_activateBoss = activateBoss;
	if (! my d_activateCallback) {
		/*
			This must be a button in a blocking dialog.
		*/
		Melder_assert (Thing_isa (my d_shell, classGuiDialog));
		GuiDialog dialog = (GuiDialog) my d_shell;
		my d_activateCallback = gui_blocking_dialog_cb_ok;
		my d_activateBoss = dialog;
		my d_sequentalIdInDialog = ++ dialog -> latestCreatedButtonId;
		if (flags & GuiButton_DEFAULT) {
			dialog -> defaultButtonId = my d_sequentalIdInDialog;
			dialog -> defaultButton = me.get();
		}
	}
	#if gtk
		my d_widget = gtk_button_new_with_label (Melder_peek32to8 (translatedText));
		gtk_button_set_relief (GTK_BUTTON (my d_widget), GTK_RELIEF_NORMAL);
		_GuiObject_setUserData (my d_widget, me.get());
		my v_positionInForm (my d_widget, left, right, top, bottom, parent);
		if (flags & GuiButton_DEFAULT || flags & GuiButton_ATTRACTIVE) {
			gtk_widget_set_can_default (GTK_WIDGET (my d_widget), true);
			GtkWidget *shell = gtk_widget_get_toplevel (GTK_WIDGET (my d_widget));
			Melder_assert (shell);
			gtk_window_set_default (GTK_WINDOW (shell), GTK_WIDGET (my d_widget));
		} else if (1) {
			gtk_button_set_focus_on_click (GTK_BUTTON (my d_widget), false);
			gtk_widget_set_can_default (GTK_WIDGET (my d_widget), false);
		}
		g_signal_connect (G_OBJECT (my d_widget), "destroy", G_CALLBACK (_GuiGtkButton_destroyCallback), me.get());
		g_signal_connect (GTK_BUTTON (my d_widget), "clicked", G_CALLBACK (_GuiGtkButton_activateCallback), me.get());
		GtkWidget *internalLabel = gtk_bin_get_child (GTK_BIN (my d_widget));
		if (GTK_IS_LABEL (internalLabel))
			gtk_label_set_justify (GTK_LABEL (internalLabel), GTK_JUSTIFY_CENTER);
//		if (flags & GuiButton_CANCEL) {
//			parent -> shell -> cancelButton = parent -> cancelButton = my widget;
//		}
	#elif motif
		my d_widget = _Gui_initializeWidget (xmPushButtonWidgetClass, parent -> d_widget, translatedText);
		_GuiObject_setUserData (my d_widget, me.get());
		my d_widget -> window = CreateWindow (L"button", Melder_peek32toW (_GuiWin_expandAmpersands (my d_widget -> name.get())),
			WS_CHILD
			| ( flags & (GuiButton_DEFAULT | GuiButton_ATTRACTIVE) ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON )
			| ( flags & GuiButton_MULTILINE ? BS_MULTILINE : 0 )
			| WS_CLIPSIBLINGS,
			my d_widget -> x, my d_widget -> y, my d_widget -> width, my d_widget -> height,
			my d_widget -> parent -> window, (HMENU) 1, theGui.instance, nullptr
		);
		SetWindowLongPtr (my d_widget -> window, GWLP_USERDATA, (LONG_PTR) my d_widget);
		SetWindowFont (my d_widget -> window, flags & GuiButton_DEFAULT ? theWinGuiBoldLabelFont () : theWinGuiNormalLabelFont (), false);
		_GuiWin_subclassModernButton (my d_widget -> window, flags);
		my v_positionInForm (my d_widget, left, right, top, bottom, parent);
		if (flags & GuiButton_DEFAULT || flags & GuiButton_ATTRACTIVE)
			parent -> d_widget -> shell -> defaultButton = parent -> d_widget -> defaultButton = my d_widget;
		if (flags & GuiButton_CANCEL)
			parent -> d_widget -> shell -> cancelButton = parent -> d_widget -> cancelButton = my d_widget;
	#elif cocoa
		GuiCocoaButton *button = [[GuiCocoaButton alloc] init];
		my name = Melder_dup_f (translatedText);
		my d_widget = (GuiObject) button;
		my v_positionInForm (my d_widget, left, right, top, bottom, parent);
		[button setUserData: me.get()];
		[button setButtonType: NSMomentaryPushInButton];
		if (bottom - top < 30)
			[button setBezelStyle: NSBezelStylePush];   // fixed height, ...
		else
			[button setBezelStyle: NSBezelStyleFlexiblePush];   // ... custom height
		[button setImagePosition: NSNoImage];
		[button setBordered: YES];
		static NSFont *theButtonFont;
		if (! theButtonFont)
			theButtonFont = [NSFont systemFontOfSize: 13.0];
		[button setFont: theButtonFont];
		[button setTitle: (NSString *) Melder_peek32toCfstring (translatedText)];
		[button setTarget: (id) my d_widget];
		[button setAction: @selector (_guiCocoaButton_activateCallback:)];
		//[button setAutoresizingMask: NSViewNotSizable];
		if (flags & GuiButton_DEFAULT) {
			[button setKeyEquivalent: @"\r"];
			[[button window] setDefaultButtonCell: [button cell]];   // does nothing (and doesn't help flexible push buttons)
		}
		if (flags & GuiButton_CANCEL)
			[button setKeyEquivalent: [NSString stringWithFormat: @"%c", 27]];   // Escape key
		if (flags & GuiButton_ATTRACTIVE) {
			//[button setKeyEquivalent: @"\r"];   // slow!
			[button highlight: YES];   // lasts only till it's clicked!
			//[button setBezelStyle: NSThickerSquareBezelStyle];
			//[button setFont: [NSFont boldSystemFontOfSize: 14.0]];
		}
	#endif
	if (flags & GuiButton_INSENSITIVE)
		GuiThing_setSensitive (me.get(), false);
	GuiI18n_bind (me.get(), GuiI18nKind::Button, buttonText);
	return me.releaseToAmbiguousOwner();
}

GuiButton GuiButton_createShown (GuiForm parent, int left, int right, int top, int bottom,
	conststring32 buttonText, GuiButton_ActivateCallback activateCallback, Thing activateBoss, uint32 flags)
{
	GuiButton me = GuiButton_create (parent, left, right, top, bottom, buttonText, activateCallback, activateBoss, flags);
	GuiThing_show (me);
	return me;
}

void GuiButton_setText (GuiButton me, conststring32 text /* cattable */) {
	GuiI18n_bind (me, GuiI18nKind::Button, text);
	conststring32 translatedText = praat_translate (text);
	#if gtk
		gtk_button_set_label (GTK_BUTTON (my d_widget), Melder_peek32to8 (translatedText));
	#elif motif
		my d_widget -> name = Melder_dup_f (translatedText);
		if (my d_widget -> window) {
			SetWindowTextW (my d_widget -> window, Melder_peek32toW (_GuiWin_expandAmpersands (my d_widget -> name.get())));
			InvalidateRect (my d_widget -> window, nullptr, FALSE);
		}
	#elif cocoa
		[(NSButton *) my d_widget setTitle: (NSString *) Melder_peek32toCfstring (translatedText)];
	#endif
}

void GuiButton_setProgress (GuiButton me, double fraction, int state) {
	if (! me) return;
	#if motif
		if (my d_widget && my d_widget -> window) {
			HWND hwnd = my d_widget -> window;
			if (fraction < 0.0 || state == 0) {
				RemovePropW (hwnd, L"PraatProgress");
				RemovePropW (hwnd, L"PraatProgressState");
			} else {
				int progVal = (int)(fraction * 10000.0);
				if (progVal < 1) progVal = 1;
				if (progVal > 10000) progVal = 10000;
				SetPropW (hwnd, L"PraatProgress", (HANDLE)(intptr_t) progVal);
				SetPropW (hwnd, L"PraatProgressState", (HANDLE)(intptr_t) state);
			}
			InvalidateRect (hwnd, nullptr, FALSE);
			UpdateWindow (hwnd);
		}
	#else
		(void) me; (void) fraction; (void) state;
	#endif
}

void GuiButton_setWaveform (GuiButton me, const float *peaks, int numPeaks) {
	if (! me) return;
	#if motif
		if (my d_widget && my d_widget -> window) {
			HWND hwnd = my d_widget -> window;
			GuiButtonWaveform *wf = (GuiButtonWaveform *) GetPropW (hwnd, L"PraatWaveform");
			if (! peaks || numPeaks <= 0) {
				if (wf) {
					RemovePropW (hwnd, L"PraatWaveform");
					delete wf;
				}
			} else {
				if (! wf) {
					wf = new GuiButtonWaveform ();
					SetPropW (hwnd, L"PraatWaveform", (HANDLE) wf);
				}
				if (numPeaks > 160) numPeaks = 160;
				wf -> count = numPeaks;
				for (int i = 0; i < numPeaks; ++ i)
					wf -> peaks [i] = peaks [i];
			}
			InvalidateRect (hwnd, nullptr, FALSE);
			UpdateWindow (hwnd);
		}
	#else
		(void) me; (void) peaks; (void) numPeaks;
	#endif
}

void GuiButton_enableSplit (GuiButton me, bool enable) {
	if (! me) return;
	#if motif
		if (my d_widget && my d_widget -> window) {
			HWND hwnd = my d_widget -> window;
			if (enable)
				SetPropW (hwnd, L"PraatSplitButton", (HANDLE) 1);
			else
				RemovePropW (hwnd, L"PraatSplitButton");
			InvalidateRect (hwnd, nullptr, FALSE);
			UpdateWindow (hwnd);
		}
	#else
		(void) me; (void) enable;
	#endif
}

void GuiButton_setSplitCallback (GuiButton me, GuiButton_SplitCallback cb) {
	if (! me) return;
	#if motif
		if (my d_widget && my d_widget -> window) {
			HWND hwnd = my d_widget -> window;
			if (cb)
				SetPropW (hwnd, L"PraatSplitCb", (HANDLE) cb);
			else
				RemovePropW (hwnd, L"PraatSplitCb");
		}
	#else
		(void) me; (void) cb;
	#endif
}

void GuiButton_setSeekCallback (GuiButton me, GuiButton_SeekCallback cb) {
	if (! me) return;
	#if motif
		if (my d_widget && my d_widget -> window) {
			HWND hwnd = my d_widget -> window;
			if (cb)
				SetPropW (hwnd, L"PraatSeekCb", (HANDLE) cb);
			else
				RemovePropW (hwnd, L"PraatSeekCb");
		}
	#else
		(void) me; (void) cb;
	#endif
}

void GuiButton_setSeekingEnabled (GuiButton me, bool enabled) {
	if (! me) return;
	#if motif
		if (my d_widget && my d_widget -> window) {
			HWND hwnd = my d_widget -> window;
			if (enabled)
				SetPropW (hwnd, L"PraatAllowSeeking", (HANDLE) 1);
			else
				RemovePropW (hwnd, L"PraatAllowSeeking");
			InvalidateRect (hwnd, nullptr, FALSE);
			UpdateWindow (hwnd);
		}
	#else
		(void) me; (void) enabled;
	#endif
}

bool GuiButton_isSeekingEnabled (GuiButton me) {
	if (! me) return false;
	#if motif
		if (my d_widget && my d_widget -> window) {
			return GetPropW (my d_widget -> window, L"PraatAllowSeeking") != nullptr;
		}
	#endif
	return false;
}

/* End of file GuiButton.cpp */

