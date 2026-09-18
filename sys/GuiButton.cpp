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

		if (MATCHES (L"Play") || MATCHES (L"播放"))
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

		if (MATCHES (L"Record") || MATCHES (L"录音"))
			return L"\uE720";   // Microphone

		if (MATCHES (L"Draw") || MATCHES (L"绘制"))
			return L"\uE90A";   // Paint brush

		if (MATCHES (L"Query") || MATCHES (L"查询"))
			return L"\uE721";   // Search

		if (MATCHES (L"Modify") || MATCHES (L"修改"))
			return L"\uE790";   // Settings wrench

		if (MATCHES (L"Apply") || MATCHES (L"应用"))
			return L"\uE895";   // Refresh / Sync / Update (distinct from OK)

		if (MATCHES (L"Pause") || MATCHES (L"暂停"))
			return L"\uE769";   // Pause double bars

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

		#undef MATCHES
		return nullptr;
	}

	static LRESULT CALLBACK _ModernButtonSubclassProc (
		HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData
	) {
		switch (uMsg) {
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

				LRESULT res = DefSubclassProc (hwnd, uMsg, wParam, lParam);
				if (GetCapture () == hwnd) {
					InvalidateRect (hwnd, nullptr, FALSE);
					UpdateWindow (hwnd);
				}
				return res;
			}
			case WM_MOUSELEAVE: {
				if (GetPropW (hwnd, L"PraatHover")) {
					RemovePropW (hwnd, L"PraatHover");
					InvalidateRect (hwnd, nullptr, FALSE);
				}
				break;
			}
			case WM_LBUTTONDOWN: {
				LRESULT res = DefSubclassProc (hwnd, uMsg, wParam, lParam);
				InvalidateRect (hwnd, nullptr, FALSE);
				UpdateWindow (hwnd);
				return res;
			}
			case WM_LBUTTONUP: {
				LRESULT res = DefSubclassProc (hwnd, uMsg, wParam, lParam);
				InvalidateRect (hwnd, nullptr, FALSE);
				UpdateWindow (hwnd);
				return res;
			}
			case WM_CAPTURECHANGED: {
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
					g.DrawPath (& borderPen, & path);
				}

				// Draw Button Text & Icon
				WCHAR textBuf [512];
				int textLen = GetWindowTextW (hwnd, textBuf, 512);
				if (textLen > 0) {
					HFONT hFont = (isDefault ? theWinGuiBoldLabelFont () : theWinGuiNormalLabelFont ());
					HFONT oldFont = (HFONT) SelectObject (memDC, hFont);
					SetBkMode (memDC, TRANSPARENT);
					SetTextColor (memDC, textCol);

					RECT textRc = rc;
					// Inset slightly to prevent text clipping
					InflateRect (& textRc, -3, 0);
					if (isPressed)
						OffsetRect (& textRc, 0, 1);

					const wchar_t *iconGlyph = getButtonIconGlyph (textBuf);
					if (iconGlyph && ! (dwRefData & GuiButton_MULTILINE)) {
						SIZE szText;
						GetTextExtentPoint32W (memDC, textBuf, textLen, & szText);
						const int iconW = 14;
						const int gap = 6;
						int totalW = iconW + gap + szText.cx;
						int boxW = textRc.right - textRc.left;
						int startX = textRc.left + (boxW - totalW) / 2;
						if (startX < textRc.left + 2)
							startX = textRc.left + 2;

						// 1. Draw Icon
						HFONT hIconFont = theWinGuiIconFont (-12);
						SelectObject (memDC, hIconFont);
						RECT rcIcon = { startX, textRc.top, startX + iconW, textRc.bottom };
						DrawTextW (memDC, iconGlyph, -1, & rcIcon, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

						// 2. Draw Text
						SelectObject (memDC, hFont);
						RECT rcLabel = { startX + iconW + gap, textRc.top, textRc.right - 2, textRc.bottom };
						DrawTextW (memDC, textBuf, -1, & rcLabel, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
					} else {
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
	#if gtk
		gtk_button_set_label (GTK_BUTTON (my d_widget), Melder_peek32to8 (text));
	#elif motif
		my d_widget -> name = Melder_dup_f (text);
		_GuiNativeControl_setTitle (my d_widget);
	#elif cocoa
		[(NSButton *) my d_widget setTitle: (NSString *) Melder_peek32toCfstring (text)];
	#endif
}

/* End of file GuiButton.cpp */
