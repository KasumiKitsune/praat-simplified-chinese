/* GuiCheckButton.cpp
 *
 * Copyright (C) 1993-2012-2018,2024 Paul Boersma,
 *               2007-2008 Stefan de Konink, 2010 Franz Brausse, 2013 Tom Naughton
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

#include "GuiP.h"

Thing_implement (GuiCheckButton, GuiControl, 0);

#if motif
	#define iam_checkbutton \
		Melder_assert (widget -> widgetClass == xmToggleButtonWidgetClass); \
		GuiCheckButton me = (GuiCheckButton) widget -> userData
#else
	#define iam_checkbutton \
		GuiCheckButton me = (GuiCheckButton) _GuiObject_getUserData (widget)
#endif

#if gtk
	static void _GuiGtkCheckButton_destroyCallback (GuiObject widget, gpointer void_me) {
		(void) widget;
		iam (GuiCheckButton);
		forget (me);
	}
	static void _GuiGtkCheckButton_valueChangedCallback (GuiObject widget, gpointer void_me) {
		iam (GuiCheckButton);
		if (my d_valueChangedCallback && ! my d_blockValueChangedCallbacks) {
			struct structGuiCheckButtonEvent event { me };
			my d_valueChangedCallback (my d_valueChangedBoss, & event);
		}
	}
#elif motif
	void _GuiWinCheckButton_destroy (GuiObject widget) {
		iam_checkbutton;
		_GuiNativeControl_destroy (widget);
		forget (me);   // NOTE: my widget is not destroyed here
	}
	void _GuiWinCheckButton_handleClick (GuiObject widget) {
		iam_checkbutton;
		if (my d_valueChangedCallback) {
			struct structGuiCheckButtonEvent event { me };
			my d_valueChangedCallback (my d_valueChangedBoss, & event);
		}
	}

	static LRESULT CALLBACK _ModernCheckButtonSubclassProc (
		HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
		UINT_PTR uIdSubclass, DWORD_PTR dwRefData
	) {
		(void) dwRefData;
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
				if (GetCapture () == hwnd) {
					InvalidateRect (hwnd, nullptr, FALSE);
					UpdateWindow (hwnd);
				}
				break;
			}
			case WM_MOUSELEAVE: {
				if (GetPropW (hwnd, L"PraatHover")) {
					RemovePropW (hwnd, L"PraatHover");
					InvalidateRect (hwnd, nullptr, FALSE);
				}
				break;
			}
			case WM_LBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_CAPTURECHANGED:
			case BM_SETSTATE:
			case BM_SETCHECK: {
				LRESULT res = DefSubclassProc (hwnd, uMsg, wParam, lParam);
				InvalidateRect (hwnd, nullptr, FALSE);
				UpdateWindow (hwnd);
				return res;
			}
			case WM_SETFOCUS:
			case WM_KILLFOCUS:
			case WM_ENABLE:
			case WM_KEYDOWN:
			case WM_KEYUP: {
				LRESULT res = DefSubclassProc (hwnd, uMsg, wParam, lParam);
				InvalidateRect (hwnd, nullptr, FALSE);
				return res;
			}
			case WM_ERASEBKGND:
				return 1;
			case WM_PAINT: {
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint (hwnd, & ps);
				RECT rc;
				GetClientRect (hwnd, & rc);

				HDC memDC = CreateCompatibleDC (hdc);
				HBITMAP memBitmap = CreateCompatibleBitmap (hdc, rc.right, rc.bottom);
				HBITMAP oldBitmap = (HBITMAP) SelectObject (memDC, memBitmap);

				// Fill background with parent dialog background
				FillRect (memDC, & rc, theWinGuiBackgroundBrush ());

				int state = Button_GetState (hwnd);
				bool isChecked  = (state & 0x0003) == BST_CHECKED;
				bool isEnabled  = IsWindowEnabled (hwnd);
				bool isPressed  = ((state & BST_PUSHED) != 0 || (GetCapture () == hwnd)) && isEnabled;
				bool isHover    = (GetPropW (hwnd, L"PraatHover") != nullptr) && isEnabled;

				// Checkbox geometry: 18x18px rounded square, vertically centered
				int boxSize = 18;
				int boxLeft = rc.left + 2;
				int boxTop  = rc.top + (rc.bottom - rc.top - boxSize) / 2;
				if (boxTop < 0)
					boxTop = 0;

				_GuiWin_ensureGdiplus ();
				{
					Gdiplus::Graphics g (memDC);
					g.SetSmoothingMode (Gdiplus::SmoothingModeAntiAlias);
					g.SetPixelOffsetMode (Gdiplus::PixelOffsetModeHighQuality);

					float x = (float) boxLeft;
					float y = (float) boxTop;
					float size = (float) boxSize;
					float r = 4.0f;

					Gdiplus::GraphicsPath path;
					path.AddArc (x, y, r * 2.0f, r * 2.0f, 180.0f, 90.0f);
					path.AddArc (x + size - r * 2.0f, y, r * 2.0f, r * 2.0f, 270.0f, 90.0f);
					path.AddArc (x + size - r * 2.0f, y + size - r * 2.0f, r * 2.0f, r * 2.0f, 0.0f, 90.0f);
					path.AddArc (x, y + size - r * 2.0f, r * 2.0f, r * 2.0f, 90.0f, 90.0f);
					path.CloseFigure ();

					if (! isEnabled) {
						Gdiplus::SolidBrush disabledBg (Gdiplus::Color (255, 243, 244, 246));
						Gdiplus::Pen disabledBorder (Gdiplus::Color (255, 209, 213, 219), 1.0f);
						g.FillPath (& disabledBg, & path);
						g.DrawPath (& disabledBorder, & path);
					} else if (isChecked) {
						Gdiplus::Color bgCol = isPressed ? Gdiplus::Color (255, 0, 90, 168)
							: (isHover ? Gdiplus::Color (255, 0, 120, 215) : Gdiplus::Color (255, 0, 103, 192));
						Gdiplus::SolidBrush blueBrush (bgCol);
						g.FillPath (& blueBrush, & path);

						// Anti-aliased white checkmark with rounded joins and ends
						Gdiplus::Pen checkPen (Gdiplus::Color (255, 255, 255, 255), 2.0f);
						checkPen.SetStartCap (Gdiplus::LineCapRound);
						checkPen.SetEndCap (Gdiplus::LineCapRound);
						checkPen.SetLineJoin (Gdiplus::LineJoinRound);

						Gdiplus::PointF pts [3] = {
							{ x + 4.5f, y + 9.5f },
							{ x + 7.5f, y + 13.0f },
							{ x + 13.8f, y + 5.5f }
						};
						g.DrawLines (& checkPen, pts, 3);
					} else {
						Gdiplus::Color bgCol = isPressed ? Gdiplus::Color (255, 229, 231, 235)
							: (isHover ? Gdiplus::Color (255, 248, 250, 252) : Gdiplus::Color (255, 255, 255, 255));
						Gdiplus::Color borderCol = isHover ? Gdiplus::Color (255, 100, 116, 139)
							: Gdiplus::Color (255, 140, 140, 140);
						Gdiplus::SolidBrush bgBrush (bgCol);
						Gdiplus::Pen borderPen (borderCol, 1.2f);
						g.FillPath (& bgBrush, & path);
						g.DrawPath (& borderPen, & path);
					}
				}

				// Draw label text
				WCHAR textBuf [512];
				int textLen = GetWindowTextW (hwnd, textBuf, 512);
				if (textLen > 0) {
					HFONT hFont = theWinGuiNormalLabelFont ();
					HFONT oldFont = (HFONT) SelectObject (memDC, hFont);
					SetBkMode (memDC, TRANSPARENT);
					SetTextColor (memDC, isEnabled ? RGB (17, 24, 39) : RGB (156, 163, 175));

					RECT textRc = rc;
					textRc.left = boxLeft + boxSize + 8;
					DrawTextW (memDC, textBuf, -1, & textRc, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

					SelectObject (memDC, oldFont);
				}

				BitBlt (hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
				SelectObject (memDC, oldBitmap);
				DeleteObject (memBitmap);
				DeleteDC (memDC);
				EndPaint (hwnd, & ps);
				return 0;
			}
			case WM_NCDESTROY: {
				RemovePropW (hwnd, L"PraatHover");
				RemoveWindowSubclass (hwnd, _ModernCheckButtonSubclassProc, uIdSubclass);
				break;
			}
			default: break;
		}
		return DefSubclassProc (hwnd, uMsg, wParam, lParam);
	}
#elif cocoa
	@implementation GuiCocoaCheckButton {
		GuiCheckButton d_userData;
	}
	- (void) dealloc {   // override
		GuiCheckButton me = d_userData;
		forget (me);
		trace (U"deleting a check button");
		[super dealloc];
	}
	- (GuiThing) getUserData {
		return d_userData;
	}
	- (void) setUserData: (GuiThing) userData {
		Melder_assert (userData == nullptr || Thing_isa (userData, classGuiCheckButton));
		d_userData = static_cast <GuiCheckButton> (userData);
	}
	- (void) _guiCocoaButton_activateCallback: (id) widget {
		Melder_assert (self == widget);   // sender (widget) and receiver (self) happen to be the same object
		GuiCheckButton me = d_userData;
		if (my d_valueChangedCallback) {
			Melder_assert (! my d_blockValueChangedCallbacks);
			struct structGuiCheckButtonEvent event { me };
			my d_valueChangedCallback (my d_valueChangedBoss, & event);
		}
	}
	@end
#endif

GuiCheckButton GuiCheckButton_create (GuiForm parent, int left, int right, int top, int bottom,
	conststring32 buttonText, GuiCheckButton_ValueChangedCallback valueChangedCallback, Thing valueChangedBoss, uint32 flags)
{
	autoGuiCheckButton me = Thing_new (GuiCheckButton);
	my d_shell = parent -> d_shell;
	my d_parent = parent;
	my d_valueChangedCallback = valueChangedCallback;
	my d_valueChangedBoss = valueChangedBoss;
	conststring32 translatedText = praat_translate (buttonText);
	#if gtk
		my d_widget = gtk_check_button_new_with_label (Melder_peek32to8 (translatedText));
		_GuiObject_setUserData (my d_widget, me.get());
		my v_positionInForm (my d_widget, left, right, top, bottom, parent);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (my d_widget), (flags & GuiCheckButton_SET) != 0);
		if (flags & GuiCheckButton_INSENSITIVE) {
			GuiThing_setSensitive (me.get(), false);
		}
		g_signal_connect (G_OBJECT (my d_widget), "destroy", G_CALLBACK (_GuiGtkCheckButton_destroyCallback), me.get());
		g_signal_connect (GTK_TOGGLE_BUTTON (my d_widget), "toggled", G_CALLBACK (_GuiGtkCheckButton_valueChangedCallback), me.get());
	#elif motif
		my d_widget = _Gui_initializeWidget (xmToggleButtonWidgetClass, parent -> d_widget, translatedText);
		_GuiObject_setUserData (my d_widget, me.get());
		my d_widget -> isRadioButton = false;
		my d_widget -> window = CreateWindow (L"button", Melder_peek32toW (_GuiWin_expandAmpersands (translatedText)),
			WS_CHILD | BS_AUTOCHECKBOX | WS_CLIPSIBLINGS,
			my d_widget -> x, my d_widget -> y, my d_widget -> width, my d_widget -> height,
			my d_widget -> parent -> window, (HMENU) 1, theGui.instance, nullptr);
		SetWindowLongPtr (my d_widget -> window, GWLP_USERDATA, (LONG_PTR) my d_widget);
		SetWindowFont (my d_widget -> window, theWinGuiNormalLabelFont (), false);
		SetWindowSubclass (my d_widget -> window, _ModernCheckButtonSubclassProc, 1, 0);
		my v_positionInForm (my d_widget, left, right, top, bottom, parent);
		if (flags & GuiCheckButton_SET) {
			Button_SetCheck (my d_widget -> window, BST_CHECKED);
		}
		if (flags & GuiCheckButton_INSENSITIVE) {
			GuiThing_setSensitive (me.get(), false);
		}
	#elif cocoa
		GuiCocoaCheckButton *checkButton = [[GuiCocoaCheckButton alloc] init];
		my d_widget = (GuiObject) checkButton;
		my v_positionInForm (my d_widget, left, right, top, bottom, parent);
		[checkButton setUserData: me.get()];
		[checkButton setButtonType: NSSwitchButton];
		[checkButton setTitle: (NSString *) Melder_peek32toCfstring (translatedText)];
		[checkButton setTarget: checkButton];
		[checkButton setAction: @selector (_guiCocoaButton_activateCallback:)];
		if (flags & GuiCheckButton_SET) {
			[checkButton setState: NSOnState];
		}
	#endif
	return me.releaseToAmbiguousOwner();
}

GuiCheckButton GuiCheckButton_createShown (GuiForm parent, int left, int right, int top, int bottom,
	conststring32 buttonText, GuiCheckButton_ValueChangedCallback valueChangedCallback, Thing valueChangedBoss, uint32 flags)
{
	GuiCheckButton me = GuiCheckButton_create (parent, left, right, top, bottom, buttonText, valueChangedCallback, valueChangedBoss, flags);
	GuiThing_show (me);
	return me;
}

bool GuiCheckButton_getValue (GuiCheckButton me) {
	bool value = false;
	#if gtk
		value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (my d_widget));   // gtk_check_button inherits from gtk_toggle_button
	#elif motif
		value = (Button_GetState (my d_widget -> window) & 0x0003) == BST_CHECKED;
	#elif cocoa
        GuiCocoaCheckButton *checkButton = (GuiCocoaCheckButton *) my d_widget;
        value = [checkButton state] == NSOnState;
	#endif
	return value;
}

void GuiCheckButton_setValue (GuiCheckButton me, bool value) {
	GuiControlBlockValueChangedCallbacks block (me);
	#if gtk
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (my d_widget), value);
	#elif motif
		Button_SetCheck (my d_widget -> window, value ? BST_CHECKED : BST_UNCHECKED);
	#elif cocoa
		GuiCocoaCheckButton *checkButton = (GuiCocoaCheckButton *) my d_widget;
		[checkButton setState: value ? NSOnState: NSOffState];
	#endif
}

/* End of file GuiCheckButton.cpp */
