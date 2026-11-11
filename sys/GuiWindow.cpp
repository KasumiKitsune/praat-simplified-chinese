/* GuiWindow.cpp
 *
 * Copyright (C) 1993-2018,2020,2021,2024,2025 Paul Boersma, 2013 Tom Naughton
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
#include "praat_translate.h"
#include "../kar/UnicodeData.h"
#include "machine.h"
#include <locale.h>

Thing_implement (GuiWindow, GuiShell, 0);

#undef iam
#define iam(x)  x me = (x) void_me

#if gtk
	static gboolean _GuiWindow_destroyCallback (GuiObject widget, GdkEvent *event, gpointer void_me) {
		(void) widget;
		iam (GuiWindow);
		forget (me);
		return true;
	}
	static gboolean _GuiWindow_goAwayCallback (GuiObject widget, GdkEvent *event, gpointer void_me) {
		(void) widget;
		iam (GuiWindow);
		if (my d_goAwayCallback) {
			my d_goAwayCallback (my d_goAwayBoss);
		}
		return true;
	}
	static void _GuiWindow_child_resizeCallback (GtkWidget *childWidget, gpointer data) {
		GtkAllocation *parentAllocation = (GtkAllocation *) data;
		GtkWidget *parentWidget = gtk_widget_get_parent (childWidget);
		Thing_cast (GuiThing, child, _GuiObject_getUserData (childWidget));
		if (child) {
			GuiControl control = nullptr;
			if (Thing_isa (child, classGuiWindow)) {
				control = static_cast <GuiWindow> (child);
				//Melder_casual (U"menu bar");
			} else if (Thing_isa (child, classGuiControl)) {
				control = static_cast <GuiControl> (child);
				//Melder_casual (U"control");
			} else if (Thing_isa (child, classGuiMenu)) {
				Thing_cast (GuiMenu, menu, child);
				control = menu -> d_cascadeButton.get();
				//Melder_casual (U"menu");
			}
			if (control) {
				/*
					Move and resize.
				*/
				trace (U"moving child of class ", Thing_className (control));
				int left = control -> d_left, right = control -> d_right, top = control -> d_top, bottom = control -> d_bottom;
				if (left   <  0) left   += parentAllocation -> width;   // this replicates structGuiControl :: v_positionInForm ()
				if (right  <= 0) right  += parentAllocation -> width;
				if (top    <  0) top    += parentAllocation -> height;
				if (bottom <= 0) bottom += parentAllocation -> height;
				trace (U"moving child to (", left, U",", top, U") with size ", right - left, U" x ", bottom - top, U".");
				GtkAllocation childAllocation { parentAllocation -> x + left, parentAllocation -> y + top, right - left, bottom - top };
				gtk_widget_size_allocate (GTK_WIDGET (childWidget), & childAllocation);
				trace (U"moved child of class ", Thing_className (control));
			}
		}
	}
	static void _GuiWindow_resizeCallback (GuiObject widget, GtkAllocation *allocation, gpointer void_me) {
		(void) widget;
		iam (GuiWindow);
		trace (U"fixed received size allocation: (", allocation -> x, U", ", allocation -> y,
			U"), ", allocation -> width, U" x ", allocation -> height, U".");
		/*
			Apparently, GTK sends the size allocation message both to the shell and to its fixed-container child.
			we could capture the message either from the shell or from the fixed; we choose to do it from the fixed.
		*/
		Melder_assert (GTK_IS_FIXED (widget));
		Melder_assert (GTK_IS_FIXED (my d_widget));
		/*
			We move and resize all the children of the fixed.
		*/
		#if defined (chrome)
		{
			GtkAllocation surrogateShellTitleLabelWidgetAllocation { allocation -> x, allocation -> y, allocation -> width, 31 };
			gtk_widget_size_allocate (GTK_WIDGET (my chrome_surrogateShellTitleLabelWidget), & surrogateShellTitleLabelWidgetAllocation);
		}
		#endif
		gtk_container_foreach (GTK_CONTAINER (widget), _GuiWindow_child_resizeCallback, allocation);
		//gtk_container_foreach (GTK_CONTAINER (my d_widget), _GuiWindow_child_resizeCallback, allocation);
		my d_width = allocation -> width;
		my d_height = allocation -> height;
		trace (U"end");
	}
#elif motif
	static void _GuiMotifWindow_destroyCallback (GuiObject widget, XtPointer void_me, XtPointer call) {
		(void) widget; (void) call;
		iam (GuiWindow);
		if (my d_xmMenuBar) {
		}
		trace (U"destroying window widget");
		forget (me);
	}
	static void _GuiMotifWindow_goAwayCallback (GuiObject widget, XtPointer void_me, XtPointer call) {
		(void) widget; (void) call;
		iam (GuiWindow);
		if (my d_goAwayCallback) {
			//Melder_casual (U"_GuiMotifWindow_goAwayCallback: before");
			my d_goAwayCallback (my d_goAwayBoss);
			//Melder_casual (U"_GuiMotifWindow_goAwayCallback: after");
		}
	}
#endif

GuiWindow GuiWindow_create (int x, int y, int width, int height, int minimumWidth, int minimumHeight,
	conststring32 title /* cattable */, GuiShell_GoAwayCallback goAwayCallback, Thing goAwayBoss, uint32 flags)
{
	autoGuiWindow me = Thing_new (GuiWindow);
	if (Melder_debug == 55)
		Melder_casual (U"\t", Thing_messageNameAndAddress (me.get()), U" init");
	my d_parent = nullptr;
	my d_goAwayCallback = goAwayCallback;
	my d_goAwayBoss = goAwayBoss;
	#if gtk
		(void) flags;
		GuiGtk_initialize ();
		my d_gtkWindow = (GtkWindow *) gtk_window_new (GTK_WINDOW_TOPLEVEL);
		#if 0
			static const GdkRGBA backgroundColour { 0.92, 0.92, 0.92, 1.0 };
			gtk_widget_override_background_color (GTK_WIDGET (my d_gtkWindow), GTK_STATE_FLAG_NORMAL, & backgroundColour);
		#endif
		g_signal_connect (G_OBJECT (my d_gtkWindow), "delete-event", goAwayCallback ? G_CALLBACK (_GuiWindow_goAwayCallback) : G_CALLBACK (gtk_widget_hide_on_delete), me.get());
		g_signal_connect (G_OBJECT (my d_gtkWindow), "destroy-event", G_CALLBACK (_GuiWindow_destroyCallback), me.get());

		gtk_window_set_default_size (GTK_WINDOW (my d_gtkWindow), width, height);
		gtk_window_set_resizable (GTK_WINDOW (my d_gtkWindow), true);

		my d_widget = gtk_fixed_new ();
		_GuiObject_setUserData (my d_widget, me.get());
		gtk_widget_set_size_request (GTK_WIDGET (my d_widget), minimumWidth, minimumHeight);
		gtk_container_add (GTK_CONTAINER (my d_gtkWindow), GTK_WIDGET (my d_widget));
		GdkGeometry geometry = { minimumWidth, minimumHeight, 0, 0, 0, 0, 0, 0, 0, 0, GDK_GRAVITY_NORTH_WEST };
		gtk_window_set_geometry_hints (my d_gtkWindow, GTK_WIDGET (my d_widget), & geometry, GDK_HINT_MIN_SIZE);
		g_signal_connect (G_OBJECT (my d_widget), "size-allocate", G_CALLBACK (_GuiWindow_resizeCallback), me.get());
		//g_signal_connect (G_OBJECT (my d_gtkWindow), "size-allocate", G_CALLBACK (_GuiWindow_resizeCallback), me.get());
		#if defined (chrome)
			my chrome_surrogateShellTitleLabelWidget = gtk_label_new (Melder_peek32to8 (title));
			gtk_label_set_use_markup (GTK_LABEL (my chrome_surrogateShellTitleLabelWidget), true);
			gtk_widget_set_size_request (GTK_WIDGET (my chrome_surrogateShellTitleLabelWidget), minimumWidth, 31 /*Machine_getTextHeight()*/);
			gtk_misc_set_alignment (GTK_MISC (my chrome_surrogateShellTitleLabelWidget), 0.5, 0.0);
			gtk_fixed_put (GTK_FIXED (my d_widget), GTK_WIDGET (my chrome_surrogateShellTitleLabelWidget), 8, 0);
			gtk_widget_show (GTK_WIDGET (my chrome_surrogateShellTitleLabelWidget));
		#endif
		GuiShell_setTitle (me.get(), title);
		gint rootX, rootY;
		gtk_window_get_position (my d_gtkWindow, & rootX, & rootY);
		trace (rootX, U" root ", rootY);
	#elif motif
		my d_xmShell = (flags & GuiWindow_TRANSLUCENT_SHELL ? XmCreateTranslucentShell : XmCreateShell) (
			nullptr, flags & GuiWindow_FULLSCREEN ? "Praatwulgfullscreen" : "Praatwulg", nullptr, 0);
		XtVaSetValues (my d_xmShell, XmNdeleteResponse, goAwayCallback ? XmDO_NOTHING : XmUNMAP, nullptr);
		XtVaSetValues (my d_xmShell, XmNx, x, XmNy, y, XmNwidth, (Dimension) width, XmNheight, (Dimension) height, nullptr);
		if (goAwayCallback)
			XmAddWMProtocolCallback (my d_xmShell, 'delw', _GuiMotifWindow_goAwayCallback, (char *) me.get());
		GuiShell_setTitle (me.get(), title);
		my d_widget = XmCreateForm (my d_xmShell, "dialog", nullptr, 0);
		_GuiObject_setUserData (my d_widget, me.get());
		XtAddCallback (my d_widget, XmNdestroyCallback, _GuiMotifWindow_destroyCallback, me.get());
		XtVaSetValues (my d_widget, XmNdialogStyle, XmDIALOG_MODELESS, XmNautoUnmanage, False, nullptr);
	#elif cocoa
		(void) flags;
		NSRect rect = { { static_cast<CGFloat>(x), static_cast<CGFloat>(y) }, { static_cast<CGFloat>(width), static_cast<CGFloat>(height) } };
		my d_cocoaShell = [[GuiCocoaShell alloc]
			initWithContentRect: rect
			styleMask: NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask
			backing: NSBackingStoreBuffered
			defer: false];
		//GuiCocoaShellDelegate *delegate = [[GuiCocoaShellDelegate alloc] init];
		//my d_cocoaShell. delegate = delegate;
		if (Melder_debug == 55)
			Melder_casual (U"\t\tGuiCocoaShell-", Melder_pointer (my d_cocoaShell), U" init in ", Thing_messageNameAndAddress (me.get()));
		[my d_cocoaShell   setCollectionBehavior: NSWindowCollectionBehaviorFullScreenPrimary];
		[my d_cocoaShell   setMinSize: NSMakeSize (minimumWidth, minimumHeight)];
		GuiShell_setTitle (me.get(), title);
		[my d_cocoaShell   makeKeyAndOrderFront: nil];
		[my d_cocoaShell   layoutIfNeeded];
		[my d_cocoaShell   setDistinctiveBackGround];
		my d_widget = (GuiObject) [my d_cocoaShell   contentView];   // BUG: this d_widget doesn't have the GuiCocoaAny protocol
		_GuiObject_setUserData (my d_cocoaShell, me.get());
		//if (! theGuiCocoaWindowDelegate) {
		//	theGuiCocoaWindowDelegate = [[GuiCocoaWindowDelegate alloc] init];
		//}
		//[my d_cocoaWindow   setDelegate: theGuiCocoaWindowDelegate];
	#endif
	my d_width = width;
	my d_height = height;
	my d_shell = me.get();
	return me.releaseToAmbiguousOwner();
}

uinteger theGuiTopLowAccelerators [8];

void GuiWindow_addMenuBar (GuiWindow me) {
	#if gtk
		my d_gtkMenuBar = (GtkMenuBar *) gtk_menu_bar_new ();
		_GuiObject_setUserData (my d_gtkMenuBar, me);
		my v_positionInForm (my d_gtkMenuBar, 0, 0, Machine_getMenuBarTop (), Machine_getMenuBarBottom (), me);   // BUG?

		// we need an accelerator group for each window we're creating accelerated menus on
		GuiObject topwin = gtk_widget_get_toplevel (GTK_WIDGET (my d_widget));
		Melder_assert (topwin == my d_gtkWindow);
		GtkAccelGroup *ag = gtk_accel_group_new ();
		gtk_window_add_accel_group (GTK_WINDOW (topwin), ag);
		// unfortunately, menu-bars don't fiddle with accel-groups, so we need a way
		// to pass it to the sub-menus created upon this bar for their items to have
		// access to the accel-group
		g_object_set_data (G_OBJECT (my d_gtkMenuBar), "accel-group", ag);
		gtk_widget_show (GTK_WIDGET (my d_gtkMenuBar));
	#elif motif
		my d_xmMenuBar = XmCreateMenuBar (my d_widget, "menuBar", nullptr, 0);
		XtVaSetValues (my d_xmMenuBar, XmNleftAttachment, XmATTACH_FORM, XmNrightAttachment, XmATTACH_FORM, nullptr);
		XtManageChild (my d_xmMenuBar);
	#elif cocoa
		(void) me;   // this is handled elsewhere
	#endif
}

bool GuiWindow_setOpacity (GuiWindow me, int opacityPercent) {
	if (! me)
		return false;
	#if motif
		return _GuiWinShell_setOpacity (my d_xmShell, opacityPercent);
	#else
		(void) opacityPercent;
		return false;
	#endif
}

int GuiWindow_getOpacity (GuiWindow me) {
	if (! me)
		return 100;
	#if motif
		return _GuiWinShell_getOpacity (my d_xmShell);
	#else
		return 100;
	#endif
}

#if motif

namespace {

constexpr int kMinOpacityPercent = 20;
constexpr int kMaxOpacityPercent = 100;
constexpr const wchar_t *kOpacityPanelClassName = L"PraatOpacityFlyout1";

int queryWindowDpi (HWND hwnd) noexcept {
	using GetDpiForWindowFn = UINT (WINAPI *) (HWND);
	static bool resolved = false;
	static GetDpiForWindowFn fn = nullptr;
	if (! resolved) {
		HMODULE user32 = GetModuleHandleW (L"user32.dll");
		if (user32)
			fn = reinterpret_cast <GetDpiForWindowFn> (GetProcAddress (user32, "GetDpiForWindow"));
		resolved = true;
	}
	if (fn && hwnd) {
		const UINT dpi = fn (hwnd);
		if (dpi >= 72 && dpi <= 480)
			return (int) dpi;
	}
	return 96;
}

inline int scaleDpi (int px, int dpi) noexcept {
	return MulDiv (px, dpi, 96);
}

} // namespace

struct structGuiOpacityPanel {
	GuiWindow owner = nullptr;
	HWND ownerHwnd = nullptr;
	HWND hwnd = nullptr;
	GuiOpacityPanel_ChangedCallback changedCallback = nullptr;
	Thing boss = nullptr;
	int value = 100;
	int dpi = 96;
	HFONT customNormalFont = nullptr;
	HFONT customBoldFont = nullptr;
	RECT anchorRect { };
	RECT badgeRect { };
	bool hasDwmRoundCorners = false;
	bool isShowing = false;
	bool isDragging = false;
	bool hoverThumb = false;
	bool hoverBadge = false;
	bool trackingLeave = false;
	bool closedByAnchorClick = false;
	ULONGLONG closedByAnchorTick = 0;

	void releaseCustomFonts () noexcept {
		if (customNormalFont) {
			DeleteObject (customNormalFont);
			customNormalFont = nullptr;
		}
		if (customBoldFont) {
			DeleteObject (customBoldFont);
			customBoldFont = nullptr;
		}
	}

	void ensureFontsForDpi (int targetDpi) noexcept {
		if (targetDpi <= 0)
			targetDpi = 96;
		if (dpi == targetDpi && (targetDpi == 96 || (customNormalFont && customBoldFont)))
			return;
		releaseCustomFonts ();
		dpi = targetDpi;
		if (dpi != 96) {
			customNormalFont = CreateFontW (-scaleDpi (13, dpi), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
				DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
				DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
			customBoldFont = CreateFontW (-scaleDpi (13, dpi), 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
				DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
				DEFAULT_PITCH | FF_DONTCARE, L"Microsoft YaHei UI");
		}
	}

	HFONT normalFont () const noexcept {
		return customNormalFont ? customNormalFont : theWinGuiNormalLabelFont ();
	}

	HFONT boldFont () const noexcept {
		return customBoldFont ? customBoldFont : theWinGuiBoldLabelFont ();
	}

	int cardWidth () const noexcept  { return scaleDpi (236, dpi); }
	int cardHeight () const noexcept { return scaleDpi (78, dpi); }
	int trackLeft () const noexcept  { return scaleDpi (18, dpi); }
	int trackRight () const noexcept { return cardWidth () - scaleDpi (18, dpi); }
	int trackY () const noexcept     { return scaleDpi (54, dpi); }
	int thumbRadius () const noexcept { return scaleDpi (8, dpi); }

	int thumbXFromValue () const noexcept {
		const int left = trackLeft ();
		const int span = trackRight () - left;
		if (span <= 0)
			return left;
		return left + MulDiv (value - kMinOpacityPercent, span, kMaxOpacityPercent - kMinOpacityPercent);
	}

	int valueFromClientX (int x) const noexcept {
		const int left = trackLeft ();
		const int right = trackRight ();
		if (x <= left)
			return kMinOpacityPercent;
		if (x >= right)
			return kMaxOpacityPercent;
		const int pct = kMinOpacityPercent + MulDiv (x - left, kMaxOpacityPercent - kMinOpacityPercent, right - left);
		return pct < kMinOpacityPercent ? kMinOpacityPercent : (pct > kMaxOpacityPercent ? kMaxOpacityPercent : pct);
	}

	void applyUserValue (int newValue) {
		Melder_clip (kMinOpacityPercent, & newValue, kMaxOpacityPercent);
		if (newValue == value)
			return;
		value = newValue;
		if (hwnd) {
			InvalidateRect (hwnd, nullptr, FALSE);
			UpdateWindow (hwnd);
		}
		if (changedCallback)
			changedCallback (boss, value);
	}
};

static void paintOpacityPanel (GuiOpacityPanel me, HDC targetDc) {
	RECT rc { };
	GetClientRect (my hwnd, & rc);
	const int w = rc.right - rc.left;
	const int h = rc.bottom - rc.top;
	if (w <= 0 || h <= 0)
		return;

	HDC dc = CreateCompatibleDC (targetDc);
	HBITMAP bmp = CreateCompatibleBitmap (targetDc, w, h);
	HBITMAP oldBmp = (HBITMAP) SelectObject (dc, bmp);

	// 1. Card background (and 1px border fallback when DWM rounded corners are unavailable on Win10)
	HBRUSH bgBrush = CreateSolidBrush (RGB (252, 253, 255));
	if (my hasDwmRoundCorners) {
		FillRect (dc, & rc, bgBrush);
	} else {
		HPEN borderPen = CreatePen (PS_SOLID, 1, RGB (208, 213, 221));
		HPEN oldPen = (HPEN) SelectObject (dc, borderPen);
		HBRUSH oldBrush = (HBRUSH) SelectObject (dc, bgBrush);
		Rectangle (dc, 0, 0, w, h);
		SelectObject (dc, oldBrush);
		SelectObject (dc, oldPen);
		DeleteObject (borderPen);
	}
	DeleteObject (bgBrush);

	SetBkMode (dc, TRANSPARENT);

	// 2. Top row: title label + clickable percentage readout pill (click resets to 100%)
	const int padX = scaleDpi (14, my dpi);
	const int topY = scaleDpi (10, my dpi);
	const int rowH = scaleDpi (24, my dpi);

	wchar_t pctText [16];
	swprintf (pctText, 16, L"%d%%", my value);

	HFONT oldFont = (HFONT) SelectObject (dc, my boldFont ());
	SIZE pctSize { };
	GetTextExtentPoint32W (dc, pctText, (int) wcslen (pctText), & pctSize);
	const int badgeW = (pctSize.cx + scaleDpi (16, my dpi) > scaleDpi (48, my dpi))
		? (pctSize.cx + scaleDpi (16, my dpi))
		: scaleDpi (48, my dpi);
	SetRect (& my badgeRect, w - padX - badgeW, topY, w - padX, topY + rowH);

	const bool badgeInteractive = (my value < 100);
	const COLORREF badgeBg = (badgeInteractive && my hoverBadge) ? RGB (224, 238, 249) : RGB (241, 245, 249);
	const COLORREF badgeBorder = (badgeInteractive && my hoverBadge) ? RGB (153, 209, 255) : RGB (226, 232, 240);
	const COLORREF badgeFg = (badgeInteractive && my hoverBadge) ? RGB (0, 103, 192) : RGB (15, 23, 42);

	{
		HBRUSH pillBrush = CreateSolidBrush (badgeBg);
		HPEN pillPen = CreatePen (PS_SOLID, 1, badgeBorder);
		HBRUSH prevB = (HBRUSH) SelectObject (dc, pillBrush);
		HPEN prevP = (HPEN) SelectObject (dc, pillPen);
		const int pillRadius = scaleDpi (10, my dpi);
		RoundRect (dc, my badgeRect.left, my badgeRect.top, my badgeRect.right, my badgeRect.bottom, pillRadius, pillRadius);
		SelectObject (dc, prevB);
		SelectObject (dc, prevP);
		DeleteObject (pillBrush);
		DeleteObject (pillPen);
	}

	SetTextColor (dc, badgeFg);
	DrawTextW (dc, pctText, -1, & my badgeRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

	SelectObject (dc, my normalFont ());
	SetTextColor (dc, RGB (31, 41, 55));
	RECT titleRect { padX, topY, my badgeRect.left - scaleDpi (8, my dpi), topY + rowH };
	conststring32 translatedLabel = praat_translate (U"Window opacity");
	DrawTextW (dc, Melder_peek32toW (translatedLabel), -1, & titleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

	// 3. Slider track (inactive + active Fluent Blue #0067C0)
	const int tLeft = my trackLeft ();
	const int tRight = my trackRight ();
	const int tY = my trackY ();
	const int halfTrack = scaleDpi (3, my dpi);
	const int trackCorner = halfTrack * 2;
	const int thumbX = my thumbXFromValue ();

	{
		HBRUSH trackBrush = CreateSolidBrush (RGB (226, 232, 240));
		HPEN trackPen = CreatePen (PS_SOLID, 1, RGB (226, 232, 240));
		HBRUSH prevB = (HBRUSH) SelectObject (dc, trackBrush);
		HPEN prevP = (HPEN) SelectObject (dc, trackPen);
		RoundRect (dc, tLeft, tY - halfTrack, tRight, tY + halfTrack, trackCorner, trackCorner);
		SelectObject (dc, prevB);
		SelectObject (dc, prevP);
		DeleteObject (trackBrush);
		DeleteObject (trackPen);
	}

	if (thumbX > tLeft) {
		HBRUSH activeBrush = CreateSolidBrush (RGB (0, 103, 192));
		HPEN activePen = CreatePen (PS_SOLID, 1, RGB (0, 103, 192));
		HBRUSH prevB = (HBRUSH) SelectObject (dc, activeBrush);
		HPEN prevP = (HPEN) SelectObject (dc, activePen);
		RoundRect (dc, tLeft, tY - halfTrack, thumbX, tY + halfTrack, trackCorner, trackCorner);
		SelectObject (dc, prevB);
		SelectObject (dc, prevP);
		DeleteObject (activeBrush);
		DeleteObject (activePen);
	}

	// 4. Fluent circular thumb with inner white core
	const int rOuter = my thumbRadius ();
	const COLORREF outerCol = my isDragging ? RGB (0, 86, 163)
		: (my hoverThumb ? RGB (16, 118, 210) : RGB (0, 103, 192));
	{
		HBRUSH thumbBrush = CreateSolidBrush (outerCol);
		HPEN thumbPen = CreatePen (PS_SOLID, 1, outerCol);
		HBRUSH prevB = (HBRUSH) SelectObject (dc, thumbBrush);
		HPEN prevP = (HPEN) SelectObject (dc, thumbPen);
		Ellipse (dc, thumbX - rOuter, tY - rOuter, thumbX + rOuter + 1, tY + rOuter + 1);
		SelectObject (dc, prevB);
		SelectObject (dc, prevP);
		DeleteObject (thumbBrush);
		DeleteObject (thumbPen);
	}
	{
		const int rInner = my isDragging ? scaleDpi (3, my dpi)
			: (my hoverThumb ? scaleDpi (5, my dpi) : scaleDpi (4, my dpi));
		HBRUSH coreBrush = CreateSolidBrush (RGB (255, 255, 255));
		HPEN corePen = CreatePen (PS_SOLID, 1, RGB (255, 255, 255));
		HBRUSH prevB = (HBRUSH) SelectObject (dc, coreBrush);
		HPEN prevP = (HPEN) SelectObject (dc, corePen);
		Ellipse (dc, thumbX - rInner, tY - rInner, thumbX + rInner + 1, tY + rInner + 1);
		SelectObject (dc, prevB);
		SelectObject (dc, prevP);
		DeleteObject (coreBrush);
		DeleteObject (corePen);
	}

	SelectObject (dc, oldFont);

	BitBlt (targetDc, 0, 0, w, h, dc, 0, 0, SRCCOPY);
	SelectObject (dc, oldBmp);
	DeleteObject (bmp);
	DeleteDC (dc);
}

static LRESULT CALLBACK opacityPanelWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_NCCREATE) {
		CREATESTRUCTW *cs = reinterpret_cast <CREATESTRUCTW *> (lParam);
		SetWindowLongPtrW (hwnd, GWLP_USERDATA, reinterpret_cast <LONG_PTR> (cs -> lpCreateParams));
		return TRUE;
	}
	GuiOpacityPanel me = reinterpret_cast <GuiOpacityPanel> (GetWindowLongPtrW (hwnd, GWLP_USERDATA));
	if (! me)
		return DefWindowProcW (hwnd, msg, wParam, lParam);

	switch (msg) {
		case WM_ERASEBKGND:
			return 1;

		case WM_PAINT: {
			PAINTSTRUCT ps { };
			HDC hdc = BeginPaint (hwnd, & ps);
			paintOpacityPanel (me, hdc);
			EndPaint (hwnd, & ps);
			return 0;
		}

		case WM_MOUSEMOVE: {
			if (! my trackingLeave) {
				TRACKMOUSEEVENT tme { sizeof (TRACKMOUSEEVENT), TME_LEAVE, hwnd, 0 };
				if (TrackMouseEvent (& tme))
					my trackingLeave = true;
			}
			const int x = GET_X_LPARAM (lParam);
			const int y = GET_Y_LPARAM (lParam);
			if (my isDragging) {
				my applyUserValue (my valueFromClientX (x));
				return 0;
			}
			POINT pt { x, y };
			const int dx = x - my thumbXFromValue ();
			const int dy = y - my trackY ();
			const int hitR = my thumbRadius () + scaleDpi (4, my dpi);
			const bool newHoverThumb = (dx * dx + dy * dy <= hitR * hitR) || (y >= scaleDpi (38, my dpi));
			const bool newHoverBadge = (my value < 100) && PtInRect (& my badgeRect, pt) != FALSE;
			if (newHoverThumb != my hoverThumb || newHoverBadge != my hoverBadge) {
				my hoverThumb = newHoverThumb;
				my hoverBadge = newHoverBadge;
				InvalidateRect (hwnd, nullptr, FALSE);
			}
			return 0;
		}

		case WM_MOUSELEAVE: {
			my trackingLeave = false;
			if (my hoverThumb || my hoverBadge) {
				my hoverThumb = false;
				my hoverBadge = false;
				InvalidateRect (hwnd, nullptr, FALSE);
			}
			return 0;
		}

		case WM_LBUTTONDOWN: {
			const int x = GET_X_LPARAM (lParam);
			const int y = GET_Y_LPARAM (lParam);
			POINT pt { x, y };
			if (PtInRect (& my badgeRect, pt)) {
				my applyUserValue (100);
				return 0;
			}
			if (y >= scaleDpi (34, my dpi)) {
				my isDragging = true;
				SetCapture (hwnd);
				InvalidateRect (hwnd, nullptr, FALSE);
				my applyUserValue (my valueFromClientX (x));
			}
			return 0;
		}

		case WM_LBUTTONDBLCLK: {
			const int y = GET_Y_LPARAM (lParam);
			if (y >= scaleDpi (34, my dpi))
				my applyUserValue (100);
			return 0;
		}

		case WM_LBUTTONUP: {
			if (my isDragging) {
				my isDragging = false;
				ReleaseCapture ();
				InvalidateRect (hwnd, nullptr, FALSE);
			}
			return 0;
		}

		case WM_CAPTURECHANGED: {
			if (my isDragging) {
				my isDragging = false;
				InvalidateRect (hwnd, nullptr, FALSE);
			}
			return 0;
		}

		case WM_MOUSEWHEEL: {
			const int delta = GET_WHEEL_DELTA_WPARAM (wParam);
			if (delta != 0) {
				const int step = (LOWORD (wParam) & (MK_SHIFT | MK_CONTROL)) ? 1 : 5;
				const int ticks = delta / WHEEL_DELTA;
				const int dir = (ticks != 0) ? ticks : (delta > 0 ? 1 : -1);
				my applyUserValue (my value + dir * step);
			}
			return 0;
		}

		case WM_KEYDOWN: {
			const int fineStep = (GetKeyState (VK_SHIFT) < 0 || GetKeyState (VK_CONTROL) < 0) ? 1 : 5;
			switch (wParam) {
				case VK_LEFT:
				case VK_DOWN:
					my applyUserValue (my value - fineStep);
					return 0;
				case VK_RIGHT:
				case VK_UP:
					my applyUserValue (my value + fineStep);
					return 0;
				case VK_NEXT:
					my applyUserValue (my value - 10);
					return 0;
				case VK_PRIOR:
					my applyUserValue (my value + 10);
					return 0;
				case VK_HOME:
					my applyUserValue (kMinOpacityPercent);
					return 0;
				case VK_END:
					my applyUserValue (kMaxOpacityPercent);
					return 0;
				case VK_ESCAPE:
				case VK_RETURN:
					GuiOpacityPanel_dismiss (me);
					if (my ownerHwnd && IsWindow (my ownerHwnd))
						SetActiveWindow (my ownerHwnd);
					return 0;
				default:
					break;
			}
			break;
		}

		case WM_DPICHANGED: {
			const int newDpi = HIWORD (wParam);
			my ensureFontsForDpi (newDpi);
			const RECT *suggested = reinterpret_cast <const RECT *> (lParam);
			if (suggested) {
				SetWindowPos (hwnd, nullptr, suggested -> left, suggested -> top,
					suggested -> right - suggested -> left, suggested -> bottom - suggested -> top,
					SWP_NOZORDER | SWP_NOACTIVATE);
			}
			InvalidateRect (hwnd, nullptr, FALSE);
			return 0;
		}

		case WM_ACTIVATE: {
			if (LOWORD (wParam) == WA_INACTIVE && my isShowing) {
				POINT pt { };
				GetCursorPos (& pt);
				const bool leftDown = (GetAsyncKeyState (VK_LBUTTON) & 0x8000) != 0;
				if (leftDown && PtInRect (& my anchorRect, pt)) {
					my closedByAnchorClick = true;
					my closedByAnchorTick = GetTickCount64 ();
				}
				GuiOpacityPanel_dismiss (me);
				return 0;
			}
			break;
		}

		default:
			break;
	}
	return DefWindowProcW (hwnd, msg, wParam, lParam);
}

static void ensureOpacityPanelClassRegistered () {
	static bool registered = false;
	if (registered)
		return;
	WNDCLASSEXW wc { };
	wc.cbSize = sizeof (WNDCLASSEXW);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_DROPSHADOW;
	wc.lpfnWndProc = opacityPanelWndProc;
	wc.hInstance = theGui.instance;
	wc.hCursor = LoadCursor (nullptr, IDC_ARROW);
	wc.hbrBackground = nullptr;
	wc.lpszClassName = kOpacityPanelClassName;
	RegisterClassExW (& wc);
	registered = true;
}

GuiOpacityPanel GuiOpacityPanel_create (GuiWindow owner, int initialOpacityPercent,
	GuiOpacityPanel_ChangedCallback changedCallback, Thing boss)
{
	ensureOpacityPanelClassRegistered ();
	GuiOpacityPanel me = Melder_calloc (struct structGuiOpacityPanel, 1);
	my owner = owner;
	my ownerHwnd = (owner && owner -> d_xmShell) ? owner -> d_xmShell -> window : nullptr;
	my changedCallback = changedCallback;
	my boss = boss;
	Melder_clip (kMinOpacityPercent, & initialOpacityPercent, kMaxOpacityPercent);
	my value = initialOpacityPercent;
	my dpi = 96;
	my ensureFontsForDpi (queryWindowDpi (my ownerHwnd));

	my hwnd = CreateWindowExW (
		WS_EX_TOOLWINDOW,
		kOpacityPanelClassName,
		L"",
		WS_POPUP,
		0, 0, my cardWidth (), my cardHeight (),
		my ownerHwnd,
		nullptr,
		theGui.instance,
		me
	);
	if (my hwnd) {
		#ifndef DWMWA_WINDOW_CORNER_PREFERENCE
		#define DWMWA_WINDOW_CORNER_PREFERENCE 33
		#endif
		#ifndef DWMWCP_ROUND
		#define DWMWCP_ROUND 2
		#endif
		#ifndef DWMWA_BORDER_COLOR
		#define DWMWA_BORDER_COLOR 34
		#endif
		DWORD cornerPref = DWMWCP_ROUND;
		my hasDwmRoundCorners = SUCCEEDED (DwmSetWindowAttribute (
			my hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, & cornerPref, sizeof (cornerPref)));
		COLORREF borderCol = RGB (208, 213, 221);
		DwmSetWindowAttribute (my hwnd, DWMWA_BORDER_COLOR, & borderCol, sizeof (borderCol));
	}
	return me;
}

void GuiOpacityPanel_dispose (GuiOpacityPanel *p_me) noexcept {
	if (! p_me || ! *p_me)
		return;
	GuiOpacityPanel me = *p_me;
	*p_me = nullptr;
	my isShowing = false;
	if (my hwnd) {
		SetWindowLongPtrW (my hwnd, GWLP_USERDATA, 0);
		if (IsWindow (my hwnd))
			DestroyWindow (my hwnd);
		my hwnd = nullptr;
	}
	my releaseCustomFonts ();
	Melder_free (me);
}

void GuiOpacityPanel_show (GuiOpacityPanel me, int anchorLeft, int anchorRight, int anchorTop, int anchorBottom) {
	if (! me || ! my hwnd)
		return;
	my closedByAnchorClick = false;
	SetRect (& my anchorRect, anchorLeft, anchorTop, anchorRight, anchorBottom);
	my ensureFontsForDpi (queryWindowDpi (my ownerHwnd ? my ownerHwnd : my hwnd));

	const int w = my cardWidth ();
	const int h = my cardHeight ();
	int x = anchorLeft;
	int y = anchorBottom + scaleDpi (2, my dpi);

	HMONITOR mon = MonitorFromRect (& my anchorRect, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi { sizeof (MONITORINFO) };
	if (mon && GetMonitorInfoW (mon, & mi)) {
		const int margin = scaleDpi (6, my dpi);
		if (x + w > mi.rcWork.right - margin)
			x = mi.rcWork.right - margin - w;
		if (x < mi.rcWork.left + margin)
			x = mi.rcWork.left + margin;
		if (y + h > mi.rcWork.bottom - margin)
			y = anchorTop - h - scaleDpi (2, my dpi);
		if (y < mi.rcWork.top + margin)
			y = mi.rcWork.top + margin;
	}

	my isShowing = true;
	SetWindowPos (my hwnd, HWND_TOP, x, y, w, h, SWP_SHOWWINDOW);
	InvalidateRect (my hwnd, nullptr, FALSE);
	SetActiveWindow (my hwnd);
	SetFocus (my hwnd);
}

void GuiOpacityPanel_dismiss (GuiOpacityPanel me) noexcept {
	if (! me)
		return;
	my isShowing = false;
	if (my isDragging) {
		my isDragging = false;
		if (GetCapture () == my hwnd)
			ReleaseCapture ();
	}
	my hoverThumb = false;
	my hoverBadge = false;
	if (my hwnd && IsWindow (my hwnd))
		ShowWindow (my hwnd, SW_HIDE);
}

void GuiOpacityPanel_setValue (GuiOpacityPanel me, int opacityPercent) {
	if (! me)
		return;
	Melder_clip (kMinOpacityPercent, & opacityPercent, kMaxOpacityPercent);
	my value = opacityPercent;
	if (my hwnd && my isShowing)
		InvalidateRect (my hwnd, nullptr, FALSE);
}

bool GuiOpacityPanel_isShowing (GuiOpacityPanel me) noexcept {
	return me && my isShowing && my hwnd && IsWindowVisible (my hwnd);
}

bool GuiOpacityPanel_steppedAsideForAnchorClick (GuiOpacityPanel me) noexcept {
	if (! me || ! my closedByAnchorClick)
		return false;
	my closedByAnchorClick = false;
	return (GetTickCount64 () - my closedByAnchorTick) <= 500;
}

#else

GuiOpacityPanel GuiOpacityPanel_create (GuiWindow, int, GuiOpacityPanel_ChangedCallback, Thing) {
	return nullptr;
}
void GuiOpacityPanel_dispose (GuiOpacityPanel *p_me) noexcept {
	if (p_me)
		*p_me = nullptr;
}
void GuiOpacityPanel_show (GuiOpacityPanel, int, int, int, int) { }
void GuiOpacityPanel_dismiss (GuiOpacityPanel) noexcept { }
void GuiOpacityPanel_setValue (GuiOpacityPanel, int) { }
bool GuiOpacityPanel_isShowing (GuiOpacityPanel) noexcept { return false; }
bool GuiOpacityPanel_steppedAsideForAnchorClick (GuiOpacityPanel) noexcept { return false; }

#endif

bool GuiWindow_setDirty (GuiWindow me, bool dirty) {
	#if gtk
		(void) dirty;
		return false;
	#elif motif
		(void) dirty;
		return false;
	#elif cocoa
		[my d_cocoaShell   setDocumentEdited: dirty];
		return true;
	#else
		(void) dirty;
		return false;
	#endif
}

/* End of file GuiWindow.cpp */
