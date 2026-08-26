/* Gui.cpp
 *
 * Copyright (C) 1992-2008,2010-2017,2019-2021,2024-2026 Paul Boersma
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
#include "machine.h"
#include <locale.h>

Thing_implement (GuiThing, Thing, 0);
Thing_implement (GuiControl, GuiThing, 0);

int Gui_getResolution (GuiObject widget) {
	static int resolution = 0;
	if (0) {
		#if gtk
			resolution = gdk_screen_get_resolution (gdk_display_get_default_screen (gtk_widget_get_display (GTK_WIDGET (widget))));
		#elif motif
			(void) widget;
			resolution = 100;
		#elif cocoa
			(void) widget;
			CGDirectDisplayID display = CGMainDisplayID ();
			CGSize size = CGDisplayScreenSize (display);
			resolution = Melder_iround (25.4 * (double) CGDisplayPixelsWide (display) / size.width);
			//resolution = 72;
		#else
			Melder_crash (U"Gui_getResolution: unknown platform.");
		#endif
	}
	return 100;   // in conformance with most other applications; and so that fonts always look the same size in the Demo window
	return resolution;
}

#if gtk
	void GuiGtk_initialize () {
		static bool gtkHasBeenInitialized = false;
		if (! gtkHasBeenInitialized) {
			trace (U"before initing GTK: locale is ", Melder_peek8to32_u (setlocale (LC_ALL, nullptr)));
			gtk_disable_setlocale ();   // otherwise 1.5 will be written "1,5" on computers with a French or German locale
			trace (U"during initing GTK: locale is ", Melder_peek8to32_u (setlocale (LC_ALL, nullptr)));
			gtk_init_check (nullptr, nullptr);
			trace (U"after initing GTK: locale is ", Melder_peek8to32_u (setlocale (LC_ALL, nullptr)));
			gtkHasBeenInitialized = true;
		}
	}
#endif

void Gui_getWindowPositioningBounds (double *x, double *y, double *width, double *height) {
	#if gtk
		GuiGtk_initialize ();
		GdkScreen *screen = gdk_screen_get_default ();
		/*
		if (parent) {
			GuiObject parent_win = gtk_widget_get_ancestor (GTK_WIDGET (parent), GTK_TYPE_WINDOW);
			if (parent_win) {
				screen = gtk_window_get_screen (GTK_WINDOW (parent_win));
			}
		}
		*/
		if (x) *x = 0;
		if (y) *y = 0;
		if (width) *width = gdk_screen_get_width (screen);
		if (height) *height = gdk_screen_get_height (screen);
	#elif motif
		#if 1
			RECT rect;
			SystemParametersInfo (SPI_GETWORKAREA, 0, & rect, 0);   // BUG: use GetMonitorInfo instead
			if (x) *x = rect. left;
			if (y) *y = rect. top;
			if (width) *width = rect. right - rect. left - 2 * GetSystemMetrics (SM_CXSIZEFRAME);
			if (height) *height = rect.bottom - rect.top - GetSystemMetrics (SM_CYCAPTION) - 2 * GetSystemMetrics (SM_CYSIZEFRAME);
		#else
			HMONITOR monitor = MonitorFromWindow (HWND window, MONITOR_DEFAULTTONEAREST);
			MONITORINFO monitorInfo;
			monitorInfo. cbSize = sizeof (MONITORINFO);
			GetMonitorInfo (monitor, & monitorInfo);
			if (x) *x = monitorInfo. rcWork. left;
			if (y) *y = monitorInfo. rcWork. top;
			if (width) *width = monitorInfo. rcWork. right - monitorInfo. rcWork. left;
			if (height) *height = monitorInfo. rcWork.bottom - monitorInfo. rcWork.top /*- GetSystemMetrics (SM_CYMINTRACK)*/;   // SM_CXSIZEFRAME  SM_CYCAPTION
		#endif
	#elif cocoa
		NSRect rect;
		NSArray *screenArray = [NSScreen screens];
		NSUInteger screenCount = [screenArray count];
		Melder_assert (screenCount >= 1);   // otherwise, output would be garbage
		NSUInteger index = 0;
		for (index = 0; index < screenCount; index ++) {
			NSScreen *screen = [screenArray objectAtIndex: index];
			rect = [screen visibleFrame];
		}
		if (x) *x = rect. origin. x;
		if (y) *y = rect. origin. y;
		if (width) *width = rect. size. width;
		if (height) *height = rect. size. height - 22;   // subtract title bar height (or is it the menu height?)
	#endif
}

/*
	Fonts, brushes.
*/

#if defined (macintosh)
	NSFont *theMacGuiNormalLabelFont () {
		static NSFont *font;
		if (! font)
			font = [NSFont systemFontOfSize: 13.0];
		return font;
	}
	NSFont *theMacGuiBoldLabelFont () {
		static NSFont *font;
		if (! font)
			font = [NSFont boldSystemFontOfSize: 13.0];
		return font;
	}
#elif defined (_WIN32)
	HFONT theWinGuiNormalLabelFont () {
		static HFONT font;
		if (! font)
			font = CreateFont (15, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, nullptr);
		return font;
	}
	HFONT theWinGuiBoldLabelFont () {
		static HFONT font;
		if (! font)
			font = CreateFont (15, 0, 0, 0, FW_HEAVY/*FW_BOLD*/, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, nullptr);
		return font;
	}
	HBRUSH theWinGuiBackgroundBrush () {
		static HBRUSH brush;
		if (! brush)
			brush = CreateSolidBrush (RGB (224, 224, 224));
		return brush;
	}
#endif

void Gui_copyTextToClipboard (conststring32 text) {
	if (! text || text [0] == U'\0') return;
	autostring8 text8 = Melder_32to8 (text);
	if (! text8) return;

	#if defined (_WIN32)
		int lenW = MultiByteToWideChar (CP_UTF8, 0, text8.get(), -1, nullptr, 0);
		if (lenW <= 0) return;
		if (! OpenClipboard (nullptr)) return;
		EmptyClipboard ();
		HGLOBAL hMem = GlobalAlloc (GMEM_MOVEABLE, lenW * sizeof (wchar_t));
		if (hMem) {
			wchar_t *pMem = (wchar_t *) GlobalLock (hMem);
			if (pMem) {
				MultiByteToWideChar (CP_UTF8, 0, text8.get(), -1, pMem, lenW);
				GlobalUnlock (hMem);
				SetClipboardData (CF_UNICODETEXT, hMem);
			}
		}
		CloseClipboard ();
	#elif defined (macintosh)
		NSString *nsStr = [NSString stringWithUTF8String: text8.get()];
		if (nsStr) {
			NSPasteboard *pb = [NSPasteboard generalPasteboard];
			[pb clearContents];
			[pb setString: nsStr forType: NSPasteboardTypeString];
		}
	#elif gtk
		GtkClipboard *cb = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
		if (cb) {
			gtk_clipboard_set_text (cb, text8.get(), -1);
		}
	#endif
}

void Gui_openUrl (conststring32 url) {
	if (! url || url [0] == U'\0') return;
	autostring8 url8 = Melder_32to8 (url);
	if (! url8) return;

	#if defined (_WIN32)
		int lenW = MultiByteToWideChar (CP_UTF8, 0, url8.get(), -1, nullptr, 0);
		if (lenW > 0) {
			std::wstring w (lenW, 0);
			MultiByteToWideChar (CP_UTF8, 0, url8.get(), -1, & w [0], lenW);
			ShellExecuteW (nullptr, L"open", w.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
		}
	#elif defined (macintosh)
		NSString *nsUrl = [NSString stringWithUTF8String: url8.get()];
		if (nsUrl) {
			NSURL *u = [NSURL URLWithString: nsUrl];
			if (u) {
				[[NSWorkspace sharedWorkspace] openURL: u];
			}
		}
	#else
		std::string safeUrl = url8.get();
		std::string escaped;
		for (char c : safeUrl) {
			if (c == '\'') escaped += "'\\''";
			else escaped += c;
		}
		std::string cmd = "xdg-open '" + escaped + "' 2>/dev/null &";
		system (cmd.c_str());
	#endif
}

/* End of file Gui.cpp */
