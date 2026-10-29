/* Native panel rendering at explicit logical DPIs; this does not emulate monitor transitions. */
#include "../../sys/GuiP.h"
#include "../../sys/praat.h"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>

static UINT layoutDpi = 96;
static UINT layoutDpiForWindow (HWND) { return layoutDpi; }
#define GetDpiForWindow layoutDpiForWindow
#include "../../foned/EditorAppearancePanel.cpp"
#undef GetDpiForWindow

static void pump (DWORD milliseconds = 400) {
	const ULONGLONG end = GetTickCount64 () + milliseconds;
	do {
		MSG message;
		while (PeekMessageW (& message, nullptr, 0, 0, PM_REMOVE)) {
			/* Capture can queue a move for the physical pointer; these checks send their own coordinates. */
			if (message.message == WM_MOUSEMOVE)
				continue;
			TranslateMessage (& message);
			DispatchMessageW (& message);
		}
		Sleep (1);
	} while (GetTickCount64 () < end);
}

static bool capture (HWND window, const wchar_t *path);

static std::vector <BYTE> pixelsOnScreen (HWND window) {
	RECT rectangle;
	GetWindowRect (window, & rectangle);
	const int width = rectangle.right - rectangle.left, height = rectangle.bottom - rectangle.top;
	BITMAPINFO info {};
	info.bmiHeader.biSize = sizeof (BITMAPINFOHEADER);
	info.bmiHeader.biWidth = width;
	info.bmiHeader.biHeight = - height;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	HDC screen = GetDC (nullptr), memory = CreateCompatibleDC (screen);
	void *bits;
	HBITMAP bitmap = CreateDIBSection (screen, & info, DIB_RGB_COLORS, & bits, nullptr, 0);
	HGDIOBJ oldBitmap = SelectObject (memory, bitmap);
	BitBlt (memory, 0, 0, width, height, screen, rectangle.left, rectangle.top, SRCCOPY);
	GdiFlush ();
	const BYTE *first = (const BYTE *) bits;
	std::vector <BYTE> result (first, first + width * height * 4);
	SelectObject (memory, oldBitmap);
	DeleteObject (bitmap);
	DeleteDC (memory);
	ReleaseDC (nullptr, screen);
	return result;
}

static bool outlineIsCrisp (EditorAppearancePanel panel, int &softPixels, int &minimumSolidPixels, int &roundedPixels) {
	RECT rectangle;
	GetClientRect (panel -> chip [1], & rectangle);
	const int width = rectangle.right, height = rectangle.bottom;
	const auto pixels = pixelsOnScreen (panel -> chip [1]);
	const auto matches = [] (const BYTE *pixel, int red, int green, int blue) {
		return std::abs ((int) pixel [2] - red) <= 1 && std::abs ((int) pixel [1] - green) <= 1 &&
			std::abs ((int) pixel [0] - blue) <= 1;
	};
	const int sampleCount = MulDiv (6, panel -> dpi, 96);
	minimumSolidPixels = sampleCount;
	for (int side = 0; side < 4; side ++) {
		int solid = 0;
		for (int offset = 0; offset < sampleCount; offset ++) {
			const int x = side == 2 ? offset : side == 3 ? width - 1 - offset : width / 2;
			const int y = side == 0 ? offset : side == 1 ? height - 1 - offset : height / 2;
			const BYTE *pixel = & pixels [(y * width + x) * 4];
			const bool stroke = matches (pixel, 124, 130, 137);
			solid += stroke;
			if (! stroke && ! matches (pixel, 255, 255, 255) && ! matches (pixel, 238, 240, 242))
				softPixels ++;
		}
		minimumSolidPixels = std::min (minimumSolidPixels, solid);
	}
	const int corner = MulDiv (10, panel -> dpi, 96);
	for (int y = 0; y < corner; y ++)
		for (int x = 0; x < corner; x ++) {
			const BYTE *pixel = & pixels [(y * width + x) * 4];
			if (! matches (pixel, 124, 130, 137) && ! matches (pixel, 255, 255, 255) && ! matches (pixel, 238, 240, 242))
				roundedPixels ++;
		}
	return softPixels == 0 && minimumSolidPixels > 0 && roundedPixels > 3;
}

static bool sliderExtremaAreCrisp (EditorAppearancePanel panel, int &maximumFade) {
	const EditorAppearancePanel_state original = panel -> state;
	RECT client;
	GetClientRect (panel -> slider, & client);
	const int width = client.right, height = client.bottom;
	for (int position : { 20, 64, 100 }) {
		const EditorAppearancePanel_state state { original.palette, position };
		EditorAppearancePanel_setState (panel, & state);
		pump (30);
		DwmFlush ();
		RECT thumb;
		SendMessageW (panel -> slider, TBM_GETTHUMBRECT, 0, (LPARAM) & thumb);
		const float cx = (thumb.left + thumb.right) / 2.0f, cy = (thumb.top + thumb.bottom) / 2.0f;
		const float radius = 9.0f * panel -> dpi / 96.0f;
		const auto pixels = pixelsOnScreen (panel -> slider);
		for (int side = 0; side < 4; side ++) {
			int closestInk = 255;
			for (int a = -2; a <= 2; a ++)
				for (int n = -2; n <= 2; n ++) {
					const int x = (int) (cx + (side == 2 ? - radius : side == 3 ? radius : a) + (side >= 2 ? n : 0));
					const int y = (int) (cy + (side == 0 ? - radius : side == 1 ? radius : a) + (side < 2 ? n : 0));
					if (x < 0 || x >= width || y < 0 || y >= height)
						continue;
					const BYTE *pixel = & pixels [(y * width + x) * 4];
					const int difference = std::max ({ std::abs ((int) pixel [2] - 55),
						std::abs ((int) pixel [1] - 59), std::abs ((int) pixel [0] - 64) });
					closestInk = std::min (closestInk, difference);
				}
			maximumFade = std::max (maximumFade, closestInk);
		}
	}
	EditorAppearancePanel_setState (panel, & original);
	pump (30);
	return maximumFade <= 2;
}

static bool sliderRepaintIsStable (EditorAppearancePanel panel) {
	RECT thumb, channel;
	SendMessageW (panel -> slider, TBM_GETTHUMBRECT, 0, (LPARAM) & thumb);
	SendMessageW (panel -> slider, TBM_GETCHANNELRECT, 0, (LPARAM) & channel);
	const int y = (thumb.top + thumb.bottom) / 2;
	POINT pointer { (thumb.left + thumb.right) / 2, y };
	ClientToScreen (panel -> slider, & pointer);
	SetCursorPos (pointer.x, pointer.y);
	pump (20);
	SendMessageW (panel -> slider, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM ((thumb.left + thumb.right) / 2, y));
	bool stable = true;
	for (int step = 0; step < 48; step ++) {
		const double position = step < 24 ? (23 - step) / 28.0 : (step - 24) / 28.0;
		const int x = channel.left + (int) ((channel.right - channel.left - 1) * position);
		SendMessageW (panel -> slider, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM (x, y));
		UpdateWindow (panel -> slider);
		pump (8);
		if (step == 23 || step == 47) {
			DwmFlush ();
			const auto dragged = pixelsOnScreen (panel -> slider);
			wchar_t filename [180];
			swprintf (filename, L"test-records/appearance-panel-native/drag-%d-%d.png", panel -> dpi, step);
			capture (panel -> slider, filename);
			InvalidateRect (panel -> slider, nullptr, FALSE);
			UpdateWindow (panel -> slider);
			DwmFlush ();
			stable &= dragged == pixelsOnScreen (panel -> slider);
		}
	}
	SendMessageW (panel -> slider, WM_LBUTTONUP, 0, MAKELPARAM (thumb.left, y));
	pump (60);
	return stable;
}

/*
	Interaction evidence. The panel is driven with the messages a real press produces, so the shared
	geometry, the pointer distance, the cancel paths and the number of reports can be read from the
	panel's own state in one process. Real mouse input, the capture chain and the compositor are
	checked by tools/verify-native-appearance.ps1; nothing here claims to have measured those.
*/
static EditorAppearancePanel thePanel;
static int previewCount, commitCount;
static std::vector <int> previewed, committed;
static std::vector <std::pair <int, int>> notifications;   // native notification code, position
static int lastNotificationCode = -1;
static int refusedPercent = -1;   // the percentage the fake editor below refuses to put in effect
static bool refuseOnPreview = true;

static void recordPreview (void * /* boss */, integer percent) {
	previewCount ++;
	previewed.push_back ((int) percent);
	if (refuseOnPreview && (int) percent == refusedPercent) {
		/* This is what FunctionEditor does when the system refuses: write the real value back. */
		const EditorAppearancePanel_state restored { thePanel -> state. palette, refusedPercent - 3 };
		EditorAppearancePanel_setState (thePanel, & restored);
	}
}

static void recordCommit (void * /* boss */, integer percent) {
	commitCount ++;
	committed.push_back ((int) percent);
	if ((int) percent == refusedPercent) {
		const EditorAppearancePanel_state restored { thePanel -> state. palette, refusedPercent - 3 };
		EditorAppearancePanel_setState (thePanel, & restored);
	}
}

/* A second window procedure in front of the panel, so the native notifications can be counted. */
static LRESULT CALLBACK spyingProcedure (HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_HSCROLL) {
		lastNotificationCode = (int) LOWORD (wParam);
		notifications.push_back ({ (int) LOWORD (wParam),
				(int) SendMessageW (thePanel -> slider, TBM_GETPOS, 0, 0) });
	}
	return panelProcedure (window, message, wParam, lParam);
}

static void startSpying (EditorAppearancePanel panel) {
	notifications.clear ();
	SetWindowLongPtrW (panel -> window, GWLP_WNDPROC, (LONG_PTR) spyingProcedure);
}

static void stopSpying (EditorAppearancePanel panel) {
	SetWindowLongPtrW (panel -> window, GWLP_WNDPROC, (LONG_PTR) panelProcedure);
}

static void resetReports () {
	previewCount = 0;
	commitCount = 0;
	previewed.clear ();
	committed.clear ();
	refusedPercent = -1;
	refuseOnPreview = true;
}

static const char *notificationName (int code) {
	static const struct { int code; const char *name; } names [] = {
		{ TB_LINEUP, "TB_LINEUP" }, { TB_LINEDOWN, "TB_LINEDOWN" }, { TB_PAGEUP, "TB_PAGEUP" },
		{ TB_PAGEDOWN, "TB_PAGEDOWN" }, { TB_THUMBPOSITION, "TB_THUMBPOSITION" },
		{ TB_THUMBTRACK, "TB_THUMBTRACK" }, { TB_TOP, "TB_TOP" }, { TB_BOTTOM, "TB_BOTTOM" },
		{ TB_ENDTRACK, "TB_ENDTRACK" }
	};
	for (const auto &entry : names)
		if (entry.code == code)
			return entry.name;
	return "none";
}

static int appliedValue (EditorAppearancePanel panel) {
	return (int) SendMessageW (panel -> slider, TBM_GETPOS, 0, 0);
}

static int firstNumberIn (HWND window) {
	wchar_t text [80];
	GetWindowTextW (window, text, 79);
	for (int i = 0; text [i]; i ++)
		if (text [i] >= L'0' && text [i] <= L'9') {
			int value = 0;
			while (text [i] >= L'0' && text [i] <= L'9')
				value = value * 10 + (text [i ++] - L'0');
			return value;
		}
	return -1;
}

static int readoutPercent (EditorAppearancePanel panel) {
	return firstNumberIn (panel -> percent);
}

static int accessiblePercent (EditorAppearancePanel panel) {
	return firstNumberIn (panel -> slider);   // "不透明度 64 %"
}

static void applyValue (EditorAppearancePanel panel, int percent) {
	const EditorAppearancePanel_state state { panel -> state. palette, percent };
	EditorAppearancePanel_setState (panel, & state);
	clearSliderDrag (panel);
}

static bool openPanel (EditorAppearancePanel panel, HWND owner, int percent) {
	const EditorAppearancePanel_state state { panel -> state. palette, percent };
	const bool shown = EditorAppearancePanel_show (panel, owner, 150, 100, 40, 24, & state);
	pump (60);
	return shown;
}

/* The pointer position that the measured travel maps onto a percentage. */
static float pointerForValue (EditorAppearancePanel panel, int percent) {
	return panel -> travelLeft + (panel -> travelRight - panel -> travelLeft) * (percent - MINIMUM_OPACITY)
			/ (float) (MAXIMUM_OPACITY - MINIMUM_OPACITY);
}

/*
	One device pixel cannot express a tenth of a percent, so the drawn handle may sit half a
	percentage step plus one pixel away from the pointer; that is the bound the design sets.
*/
static double quantisationBound (EditorAppearancePanel panel) {
	return (panel -> travelRight - panel -> travelLeft) /
			(double) (MAXIMUM_OPACITY - MINIMUM_OPACITY) / 2.0 + 1.0;
}

static double pointerDistanceInExcess (EditorAppearancePanel panel, int pointerX) {
	const double wanted = (double) pointerX - panel -> grabOffset;
	const double drawn = std::max (panel -> travelLeft, std::min (pointerForValue (panel, appliedValue (panel)),
			panel -> travelRight));
	return std::abs (drawn - wanted) - quantisationBound (panel);
}

template <typename Type> std::string number (Type value) {
	std::ostringstream text;
	text << value;
	return text. str ();
}

static std::string number (double value) {
	std::ostringstream text;
	text.precision (2);
	text << std::fixed << value;
	return text. str ();
}

static bool expect (bool condition, std::vector <std::string> &failures, const std::string &description) {
	if (! condition)
		failures.push_back (description);
	return condition;
}

/* Event log: every press, movement and release, with everything the design asks to distinguish. */
static std::ofstream eventLog;
static bool firstEvent = true;
static const char *currentCheck = "";

static void openEventLog () {
	eventLog.open ("test-records/appearance-panel-native/drag-events.json");
	if (eventLog.is_open ())
		eventLog << "{\"method\":\"messages of a left-button press, move and release sent to the panel's own "
			"controls in one process; real input, capture delivery and compositing are recorded by "
			"tools/verify-native-appearance.ps1\",\"events\":[";
}

static void writeEvent (const char *kind, EditorAppearancePanel panel, int pointerX, int pointerY,
		const std::string &note) {
	if (! eventLog.is_open ())
		return;
	const SliderGeometry geometry = sliderGeometry (panel);
	RECT thumb, channel;
	SendMessageW (panel -> slider, TBM_GETTHUMBRECT, 0, (LPARAM) & thumb);
	SendMessageW (panel -> slider, TBM_GETCHANNELRECT, 0, (LPARAM) & channel);
	wchar_t readout [16];
	GetWindowTextW (panel -> percent, readout, 15);
	wchar_t line [1100];
	swprintf (line, L"{\"dpi\":%u,\"check\":\"%hs\",\"kind\":\"%hs\",\"pointerX\":%d,\"pointerY\":%d,"
			L"\"hit\":\"%hs\",\"nativeThumbRect\":[%d,%d,%d,%d],\"nativeChannelRect\":[%d,%d,%d,%d],"
			L"\"clientRect\":[%d,%d,%d,%d],\"drawnCentreX\":%.2f,\"drawnCentreY\":%.2f,"
			L"\"grabRect\":[%d,%d,%d,%d],\"grabOffset\":%.2f,\"controlValue\":%d,\"readout\":\"%ls\","
			L"\"accessibleNameValue\":%d,\"capture\":\"0x%p\",\"focus\":\"0x%p\",\"dragging\":\"%hs\","
			L"\"notification\":\"%hs\",\"previews\":%d,\"commits\":%d,\"note\":\"%hs\"}",
			(unsigned) panel -> dpi, currentCheck, kind, pointerX, pointerY,
			PtInRect (& geometry.grab, POINT { (LONG) pointerX, (LONG) pointerY }) ? "handle" : "channel",
			(int) thumb.left, (int) thumb.top, (int) thumb.right, (int) thumb.bottom,
			(int) channel.left, (int) channel.top, (int) channel.right, (int) channel.bottom,
			(int) geometry.client.left, (int) geometry.client.top, (int) geometry.client.right,
			(int) geometry.client.bottom,
			geometry.centreX, geometry.centreY,
			(int) geometry.grab.left, (int) geometry.grab.top, (int) geometry.grab.right,
			(int) geometry.grab.bottom, panel -> grabOffset, appliedValue (panel), readout,
			accessiblePercent (panel), (void *) GetCapture (), (void *) GetFocus (),
			panel -> dragging ? "true" : "false", notificationName (lastNotificationCode),
			previewCount, commitCount, note.c_str ());
	const int needed = WideCharToMultiByte (CP_UTF8, 0, line, -1, nullptr, 0, nullptr, nullptr);
	std::vector <char> narrow ((size_t) std::max (needed, 1));
	WideCharToMultiByte (CP_UTF8, 0, line, -1, narrow.data (), (int) narrow.size (), nullptr, nullptr);
	if (! firstEvent)
		eventLog << ',';
	firstEvent = false;
	eventLog << narrow.data ();
}

static void pressAt (EditorAppearancePanel panel, float x, float y, const std::string &note) {
	const int pointerX = (int) std::lround (x), pointerY = (int) std::lround (y);
	SendMessageW (panel -> slider, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM (pointerX, pointerY));
	pump (20);
	writeEvent ("press", panel, pointerX, pointerY, note);
}

static void dragTo (EditorAppearancePanel panel, float x, float y, bool record, const std::string &note = "") {
	const int pointerX = (int) std::lround (x), pointerY = (int) std::lround (y);
	SendMessageW (panel -> slider, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM (pointerX, pointerY));
	if (record)
		writeEvent ("drag", panel, pointerX, pointerY, note);
}

static void releaseAt (EditorAppearancePanel panel, float x, float y, const std::string &note) {
	const int pointerX = (int) std::lround (x), pointerY = (int) std::lround (y);
	SendMessageW (panel -> slider, WM_LBUTTONUP, 0, MAKELPARAM (pointerX, pointerY));
	pump (30);
	writeEvent ("release", panel, pointerX, pointerY, note);
}

/* Keep the physical pointer off the panel: nothing but the sent messages should move the handle. */
static void moveTheRealPointerAway (EditorAppearancePanel panel) {
	RECT window;
	GetWindowRect (panel -> window, & window);
	SetCursorPos (window.right + 300, window.bottom + 300);
	pump (20);
}

/* The travel was measured once for this layout, and the handle is drawn with exactly that travel. */
static bool checkGeometry (EditorAppearancePanel panel, std::vector <std::string> &failures) {
	bool passed = true;
	const float travel = panel -> travelRight - panel -> travelLeft;
	passed &= expect (travel > 1.0f, failures, "travel measured at layout time, got " + number (travel));
	applyValue (panel, MINIMUM_OPACITY);
	const SliderGeometry low = sliderGeometry (panel);
	passed &= expect (std::abs (low.centreX - panel -> travelLeft) <= 1.0, failures,
			"drawn centre at 20 % is the left end of the travel (" + number (low.centreX) + " / " +
			number (panel -> travelLeft) + ")");
	applyValue (panel, MAXIMUM_OPACITY);
	const SliderGeometry high = sliderGeometry (panel);
	passed &= expect (std::abs (high.centreX - panel -> travelRight) <= 1.0, failures,
			"drawn centre at 100 % is the right end of the travel (" + number (high.centreX) + " / " +
			number (panel -> travelRight) + ")");
	applyValue (panel, 60);
	const SliderGeometry geometry = sliderGeometry (panel);
	RECT probeThumb, probeChannel;
	SendMessageW (panel -> slider, TBM_GETTHUMBRECT, 0, (LPARAM) & probeThumb);
	SendMessageW (panel -> slider, TBM_GETCHANNELRECT, 0, (LPARAM) & probeChannel);
	std::cout << "   geometry " << number (panel -> dpi) << " dpi: client " << number ((int) geometry.client.right)
			<< "x" << number ((int) geometry.client.bottom) << ", channel " << number ((int) probeChannel.left)
			<< ".." << number ((int) probeChannel.right) << " y " << number ((int) probeChannel.top) << ".."
			<< number ((int) probeChannel.bottom) << ", thumb y " << number ((int) probeThumb.top) << ".."
			<< number ((int) probeThumb.bottom) << ", centre " << number (geometry.centreX) << "," <<
			number (geometry.centreY) << ", grab " << number ((int) geometry.grab.left) << ".." <<
			number ((int) geometry.grab.right) << " y " << number ((int) geometry.grab.top) << ".." <<
			number ((int) geometry.grab.bottom) << '\n';
	const float ink = geometry.radius + geometry.strokeWidth / 2.0f + 0.5f;
	passed &= expect (geometry.grab.left <= (LONG) std::floor (geometry.centreX - ink) &&
			geometry.grab.right >= (LONG) std::ceil (geometry.centreX + ink), failures,
			"the grab area covers the whole visible circle (radius " + number (geometry.radius) +
			", grab " + number (geometry.grab.left) + ".." + number (geometry.grab.right) + ")");
	passed &= expect (geometry.grab.right - geometry.grab.left >= 2 * scaled (GRAB_WIDTH / 2, panel -> dpi),
			failures, "the grab area is at least 24 logical pixels wide (" +
			number (geometry.grab.right - geometry.grab.left) + " device pixels at " +
			number (panel -> dpi) + " dpi)");
	/*
		The row is 32 logical pixels high and the native control keeps its client area inside that, so
		the band a press can grab the handle with is as tall as the row's usable height and covers the
		circle from top to bottom: no part of the visible circle falls outside it.
	*/
	passed &= expect (geometry.grab.top <= (LONG) std::floor (geometry.centreY - ink) &&
			geometry.grab.bottom >= (LONG) std::ceil (geometry.centreY + ink), failures,
			"the grab area does not reach the top and bottom of the drawn circle (grab " +
			number (geometry.grab.top) + ".." + number (geometry.grab.bottom) + ", circle " +
			number (geometry.centreY - ink) + ".." + number (geometry.centreY + ink) + ")");
	passed &= expect (geometry.grab.bottom - geometry.grab.top >= std::min (2 * scaled (GRAB_HEIGHT / 2, panel -> dpi),
			(int) (geometry.client.bottom - geometry.client.top)), failures,
			"the grab area is " + number (geometry.grab.bottom - geometry.grab.top) +
			" device pixels high, shorter than the row allows (" + number ((int) geometry.client.bottom) +
			" usable device pixels at " + number (panel -> dpi) + " dpi)");
	passed &= expect (geometry.grab.left >= geometry.client.left && geometry.grab.right <= geometry.client.right &&
			geometry.grab.top >= geometry.client.top && geometry.grab.bottom <= geometry.client.bottom,
			failures, "the grab area stays inside the control");
	panel -> grabOffset = 0.0f;
	const int middle = (int) std::lround ((panel -> travelLeft + panel -> travelRight) / 2.0f);
	passed &= expect (sliderPercentForPointerX (panel, middle) == (MINIMUM_OPACITY + MAXIMUM_OPACITY) / 2,
			failures, "the middle of the travel is 60 % (got " + number (sliderPercentForPointerX (panel, middle)) + ")");
	passed &= expect (sliderPercentForPointerX (panel, -3000) == MINIMUM_OPACITY &&
			sliderPercentForPointerX (panel, 3000) == MAXIMUM_OPACITY, failures,
			"positions beyond the ends clamp to 20 % and 100 %");
	return passed;
}

/*
	Every device pixel of the travel is visited, from three starts that are not multiples of ten.
	The value must move one percent at a time, must reach every percentage, and must sit at the same
	value when the same pixel is visited again on the way back.
*/
static bool checkContinuity (EditorAppearancePanel panel, std::vector <std::string> &failures) {
	bool passed = true;
	for (const int start : { 37, 64, 87 }) {
		resetReports ();
		applyValue (panel, start);
		const SliderGeometry geometry = sliderGeometry (panel);
		const float row = geometry.centreY;
		pressAt (panel, geometry.centreX, row, "press at the drawn centre, start " + std::string (number (start)) + " %");
		std::map <int, int> forward;
		int largestStep = 0;
		double largestDistance = 0.0;
		int missing = 0;
		bool firstPixel = true;
		int previousValue = 0;
		for (int x = (int) std::lround (pointerForValue (panel, 21));
				x <= (int) std::lround (pointerForValue (panel, 99)); x ++) {
			dragTo (panel, (float) x, row, false);
			const int value = appliedValue (panel);
			forward [x] = value;
			if (! firstPixel)
				largestStep = std::max (largestStep, std::abs (value - previousValue));
			largestDistance = std::max (largestDistance, - pointerDistanceInExcess (panel, x));
			previousValue = value;
			firstPixel = false;
		}
		for (int value = 21; value <= 99; value ++) {
			bool reached = false;
			for (const auto &step : forward)
				reached |= step.second == value;
			missing += reached ? 0 : 1;
		}
		int lagging = 0;
		for (int x = (int) std::lround (pointerForValue (panel, 99));
				x >= (int) std::lround (pointerForValue (panel, 21)); x --) {
			dragTo (panel, (float) x, row, false);
			lagging += appliedValue (panel) == forward [x] ? 0 : 1;
		}
		const int ended = appliedValue (panel);
		releaseAt (panel, (float) std::lround (pointerForValue (panel, 21)), row, "end of the round trip");
		passed &= expect (largestStep <= 1, failures, "start " + number (start) + ": the largest step between two "
				"device pixels is " + number (largestStep) + " percent (a fixed ten percent catch-up would be 10)");
		passed &= expect (missing == 0, failures, "start " + number (start) + ": " + number (missing) +
				" percentages between 21 and 99 cannot be stopped on");
		passed &= expect (lagging == 0, failures, "start " + number (start) + ": " + number (lagging) +
				" pixels of the way back disagree with the way there");
		passed &= expect (largestDistance <= quantisationBound (panel) + 0.01, failures, "start " + number (start) +
				": the pointer is " + number (largestDistance) + " device pixels from the drawn handle, bound " +
				number (quantisationBound (panel)));
		passed &= expect (commitCount == (ended == start ? 0 : 1), failures, "start " + number (start) +
				": one slow round trip commits " + number (commitCount) + " times");
		writeEvent ("sweep", panel, (int) std::lround (pointerForValue (panel, 21)), (int) row,
				"round trip from " + number (start) + " %; pixels " + number ((int) forward.size ()) +
				", largest step " + number (largestStep) + ", largest pointer distance " + number (largestDistance));
	}
	return passed;
}

/* A fast move across the whole groove must land on the value of its position at once, ends included. */
static bool checkFastTravelAndEndpoints (EditorAppearancePanel panel, std::vector <std::string> &failures) {
	bool passed = true;
	resetReports ();
	applyValue (panel, 45);
	const float row = sliderGeometry (panel).centreY;
	pressAt (panel, sliderGeometry (panel).centreX, row, "press before a fast sweep");
	int lag = 0, changedAsks = 0, before = 45;
	for (const int target : { 90, 20, 100, 47, 20, 100 }) {
		dragTo (panel, pointerForValue (panel, target), row, true, "fast move to " + number (target) + " %");
		lag += appliedValue (panel) == target ? 0 : 1;
		changedAsks += target != before ? 1 : 0;
		before = target;
	}
	for (const int target : { 20, 100 }) {
		const int beyond = target == MINIMUM_OPACITY ?
				(int) std::lround (panel -> travelLeft) - 900 : (int) std::lround (panel -> travelRight) + 900;
		dragTo (panel, (float) beyond, row, true, "beyond the end of the travel");
		lag += appliedValue (panel) == target ? 0 : 1;
		changedAsks += target != before ? 1 : 0;
		before = target;
	}
	const int ended = appliedValue (panel);
	releaseAt (panel, (float) std::lround (panel -> travelRight) + 900, row, "released beyond the right end");
	passed &= expect (lag == 0, failures, "a fast sweep lags on " + number (lag) + " of the asked-for positions");
	passed &= expect (ended == MAXIMUM_OPACITY, failures, "the travel ends are reachable and stay clamped (got " +
			number (ended) + " %)");
	passed &= expect (commitCount == 1, failures, "one fast sweep commits " + number (commitCount) + " times");
	passed &= expect (previewCount == changedAsks, failures, "the fast sweep reported " + number (previewCount) +
			" previews for " + number (changedAsks) + " position changes");
	return passed;
}

/*
	Centre, the four sides of the circle, the four diagonal edges and the corners of the grab area:
	a press there changes nothing, and a drag from there keeps the horizontal distance that the press
	left between the pointer and the drawn circle.
*/
static bool checkHandlePressesKeepTheOffset (EditorAppearancePanel panel, std::vector <std::string> &failures) {
	/* The centre of the circle, its four sides and its four diagonal edges: nine grab points. */
	static const float horizontal [] = { 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.707f, -0.707f, 0.707f, -0.707f };
	static const float vertical [] = { 0.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.707f, 0.707f, -0.707f, -0.707f };
	bool passed = true;
	int tested = 0;
	for (int point = 0; point < 9; point ++) {
		applyValue (panel, 64);
		resetReports ();
		const SliderGeometry geometry = sliderGeometry (panel);
		const float x = geometry.centreX + geometry.radius * horizontal [point];
		const float y = geometry.centreY + geometry.radius * vertical [point];
		const int pointerX = (int) std::lround (x), pointerY = (int) std::lround (y);
		if (! PtInRect (& geometry.grab, POINT { pointerX, pointerY })) {
			passed &= expect (false, failures, "the edge of the circle at " + number (pointerX) + "," +
					number (pointerY) + " lies outside the grab area");
			continue;
		}
		tested += 1;
		pressAt (panel, x, y, "grab at the drawn circle");
		passed &= expect (appliedValue (panel) == 64 && previewCount == 0 && commitCount == 0, failures,
				"a press at " + number (pointerX) + "," + number (pointerY) + " jumps the value to " +
				number (appliedValue (panel)) + " %");
		passed &= expect (panel -> dragging, failures, "a press at " + number (pointerX) + "," +
				number (pointerY) + " does not grab the handle");
		passed &= expect (std::abs (panel -> grabOffset - (x - geometry.centreX)) <= 0.5, failures,
				"a press at " + number (pointerX) + "," + number (pointerY) + " keeps no offset (grabOffset " +
				number (panel -> grabOffset) + " want " + number (x - geometry.centreX) + ")");
		/* Continue: the handle follows the pointer minus that offset, so it never snaps to the pointer. */
		dragTo (panel, x + 4, y, true, "four device pixels further, offset preserved");
		passed &= expect (pointerDistanceInExcess (panel, pointerX + 4) <= 0.0, failures,
				"the drag from " + number (pointerX) + "," + number (pointerY) + " snaps the handle to the pointer");
		releaseAt (panel, x + 4, y, "release of the grabbed circle");
		passed &= expect (commitCount == (appliedValue (panel) == 64 ? 0 : 1), failures,
				"the drag from " + number (pointerX) + "," + number (pointerY) + " commits " +
				number (commitCount) + " times");
	}
	return expect (tested == 9, failures, "only " + number (tested) + " of the nine points on the handle were pressed") && passed;
}

/* A press on the groove goes to that position at once and continues as a drag; holding does nothing. */
static bool checkChannelPressPositionsAndKeepsHolding (EditorAppearancePanel panel,
		std::vector <std::string> &failures) {
	bool passed = true;
	startSpying (panel);
	for (const int target : { 22, 35, 49, 78, 91, 99 }) {
		resetReports ();
		applyValue (panel, 64);
		const SliderGeometry geometry = sliderGeometry (panel);
		const float x = pointerForValue (panel, target);
		const int pointerX = (int) std::lround (x);
		if (PtInRect (& geometry.grab, POINT { pointerX, (int) std::lround (geometry.centreY) })) {
			passed &= expect (false, failures, "the point of " + number (target) + " % at " + number (pointerX) +
					" lies inside the grab area, so it is not a channel press");
			continue;
		}
		notifications.clear ();
		pressAt (panel, x, geometry.centreY, "press on the groove");
		passed &= expect (appliedValue (panel) == target, failures, "a press at " + number (pointerX) +
				" positions the handle on " + number (appliedValue (panel)) + " % instead of " + number (target));
		passed &= expect (previewCount == 1 && commitCount == 0, failures, "a press on the groove reports " +
				number (previewCount) + " previews and " + number (commitCount) + " commits");
		passed &= expect (notifications.empty (), failures, "the press on the groove also reached the native "
				"control: " + number ((int) notifications.size ()) + " notifications");
		/* Still held, without a movement: no paging timer may chase the handle. */
		pump (400);
		passed &= expect (appliedValue (panel) == target && previewCount == 1 && commitCount == 0 &&
				panel -> dragging && GetCapture () == panel -> slider, failures,
				"holding still for 400 ms changed the value to " + number (appliedValue (panel)) +
				" with " + number ((int) notifications.size ()) + " native notifications");
		/* The same press continues as a drag. */
		const int continued = target >= 60 ? target - 12 : target + 12;
		dragTo (panel, pointerForValue (panel, continued), geometry.centreY, true,
				"the groove press continues as a drag");
		passed &= expect (appliedValue (panel) == continued, failures, "the drag after a groove press reached " +
				number (appliedValue (panel)) + " % instead of " + number (continued));
		releaseAt (panel, pointerForValue (panel, continued), geometry.centreY, "release of the continued drag");
		passed &= expect (commitCount == 1, failures, "a groove press and its drag commit " +
				number (commitCount) + " times");
	}
	stopSpying (panel);
	return passed;
}

/* Above the row, below it, outside the panel: the drag keeps tracking the horizontal position. */
static bool checkDragLeavesTheRowAndThePanel (EditorAppearancePanel panel, std::vector <std::string> &failures) {
	bool passed = true;
	resetReports ();
	applyValue (panel, 58);
	const SliderGeometry geometry = sliderGeometry (panel);
	pressAt (panel, geometry.centreX, geometry.centreY, "press before leaving the row");
	const int left = (int) std::lround (pointerForValue (panel, 44));
	dragTo (panel, (float) left, geometry.centreY - 400, true, "400 pixels above the row");
	passed &= expect (appliedValue (panel) == 44 && panel -> dragging, failures,
			"above the row the value is " + number (appliedValue (panel)) + " %");
	dragTo (panel, (float) left, geometry.centreY + 900, true, "900 pixels below the row");
	passed &= expect (appliedValue (panel) == 44 && panel -> dragging, failures,
			"below the row the value is " + number (appliedValue (panel)) + " %");
	dragTo (panel, - 3000.0f, 5000.0f, true, "far outside the panel, left");
	passed &= expect (appliedValue (panel) == MINIMUM_OPACITY && panel -> dragging, failures,
			"far outside on the left the value is " + number (appliedValue (panel)) + " %");
	dragTo (panel, 3000.0f, 5000.0f, true, "far outside the panel, right");
	passed &= expect (appliedValue (panel) == MAXIMUM_OPACITY, failures,
			"far outside on the right the value is " + number (appliedValue (panel)) + " %");
	const int back = (int) std::lround (pointerForValue (panel, 73));
	dragTo (panel, (float) back, 5000, true, "back inside the travel while still outside the panel");
	passed &= expect (appliedValue (panel) == 73, failures, "the way back reaches " +
			number (appliedValue (panel)) + " % instead of 73");
	releaseAt (panel, (float) back, 5000, "released outside the panel");
	passed &= expect (! panel -> dragging && GetCapture () == nullptr, failures,
			"a release outside the panel leaves a hanging drag");
	passed &= expect (commitCount == 1, failures, "a drag that leaves the panel commits " +
			number (commitCount) + " times");
	const int previewsAfterRelease = previewCount, commitsAfterRelease = commitCount, valueAfterRelease = appliedValue (panel);
	for (int x = back; x < back + 40; x += 7)
		dragTo (panel, (float) x, 5000, false);
	pump (40);
	return expect (previewCount == previewsAfterRelease && commitCount == commitsAfterRelease &&
			appliedValue (panel) == valueAfterRelease, failures,
			"movements after the release changed the value to " + number (appliedValue (panel))) && passed;
}

/* Escape closes the panel; the value it left is kept, the drag is over, and the panel can be grabbed again. */
static bool checkEscapeCancelsAndReopens (EditorAppearancePanel panel, HWND owner,
		std::vector <std::string> &failures) {
	bool passed = true;
	resetReports ();
	applyValue (panel, 48);
	const SliderGeometry geometry = sliderGeometry (panel);
	pressAt (panel, geometry.centreX, geometry.centreY, "press before Escape");
	dragTo (panel, pointerForValue (panel, 61), geometry.centreY, true, "dragged to 61 %");
	SendMessageW (panel -> slider, WM_KEYDOWN, VK_ESCAPE, 0);
	pump (40);
	const bool hidden = ! IsWindowVisible (panel -> window) && ! IsWindowVisible (panel -> chrome);
	passed &= expect (hidden, failures, "Escape left either the panel or its plate visible");
	passed &= expect (! panel -> dragging && GetCapture () == nullptr, failures,
			"Escape left a drag or a capture behind");
	passed &= expect (appliedValue (panel) == 61, failures, "Escape moved the value to " +
			number (appliedValue (panel)) + " % instead of keeping 61");
	passed &= expect (commitCount == 1, failures, "Escape commits " + number (commitCount) + " times for one drag");
	const bool reopened = openPanel (panel, owner, 61);
	passed &= expect (reopened && IsWindowVisible (panel -> window), failures, "the panel did not open again");
	const SliderGeometry again = sliderGeometry (panel);
	pressAt (panel, again.centreX, again.centreY, "first press after reopening");
	passed &= expect (panel -> dragging && GetCapture () == panel -> slider, failures,
			"after reopening the first press does not grab the handle");
	releaseAt (panel, again.centreX, again.centreY, "release without movement");
	passed &= expect (commitCount == 1, failures, "reopening and grabbing without movement commits " +
			number (commitCount) + " times");
	resetReports ();
	pressAt (panel, again.centreX, again.centreY, "press before the window is hidden");
	dragTo (panel, pointerForValue (panel, 73), again.centreY, true, "73 % before hiding");
	ShowWindow (panel -> window, SW_HIDE);
	pump (40);
	passed &= expect (! panel -> dragging && GetCapture () == nullptr, failures,
			"hiding the panel left a drag or a capture behind");
	passed &= expect (appliedValue (panel) == 73 && commitCount == 1, failures,
			"hiding the panel did not finish once at its last applied value");
	dragTo (panel, pointerForValue (panel, 88), again.centreY, false);
	passed &= expect (appliedValue (panel) == 73 && commitCount == 1, failures,
			"a hidden panel kept following the pointer");
	passed &= expect (openPanel (panel, owner, 73), failures, "the hidden panel did not reopen");
	return passed;
}

/* The capture going away — taken, cancelled, or released by ourselves — ends the drag exactly once. */
static bool checkCaptureLossEndsTheDrag (EditorAppearancePanel panel, HWND owner,
		std::vector <std::string> &failures) {
	bool passed = true;
	/* Another window takes the pointer. */
	resetReports ();
	applyValue (panel, 50);
	SliderGeometry geometry = sliderGeometry (panel);
	pressAt (panel, geometry.centreX, geometry.centreY, "press before the capture is taken");
	dragTo (panel, pointerForValue (panel, 55), geometry.centreY, true, "dragged to 55 %");
	SetCapture (owner);
	pump (40);
	passed &= expect (! panel -> dragging, failures, "a stolen capture left the drag running");
	passed &= expect (GetCapture () == owner, failures, "the panel took the capture back after it was stolen");
	passed &= expect (appliedValue (panel) == 55, failures, "a stolen capture recomputed the value to " +
			number (appliedValue (panel)) + " %");
	passed &= expect (commitCount == 1, failures, "a stolen capture commits " + number (commitCount) + " times");
	ReleaseCapture ();
	pump (30);
	passed &= expect (commitCount == 1, failures, "the capture notification after a steal commits again (" +
			number (commitCount) + ")");
	/* The system cancels the modal input state. */
	resetReports ();
	applyValue (panel, 50);
	geometry = sliderGeometry (panel);
	pressAt (panel, geometry.centreX, geometry.centreY, "press before WM_CANCELMODE");
	dragTo (panel, pointerForValue (panel, 70), geometry.centreY, true, "dragged to 70 %");
	SendMessageW (panel -> slider, WM_CANCELMODE, 0, 0);
	pump (40);
	passed &= expect (! panel -> dragging && GetCapture () == nullptr, failures, "WM_CANCELMODE left a hanging drag");
	passed &= expect (appliedValue (panel) == 70 && commitCount == 1, failures, "WM_CANCELMODE kept " +
			number (appliedValue (panel)) + " % with " + number (commitCount) + " commits");
	/* We release the capture ourselves: the notification that follows must not report a second change. */
	resetReports ();
	applyValue (panel, 50);
	geometry = sliderGeometry (panel);
	pressAt (panel, geometry.centreX, geometry.centreY, "press before releasing the capture ourselves");
	dragTo (panel, pointerForValue (panel, 40), geometry.centreY, true, "dragged to 40 %");
	ReleaseCapture ();
	pump (40);
	passed &= expect (! panel -> dragging, failures, "releasing the capture left the drag running");
	passed &= expect (commitCount == 1, failures, "releasing the capture ourselves commits " +
			number (commitCount) + " times");
	dragTo (panel, pointerForValue (panel, 88), geometry.centreY, false);
	SendMessageW (panel -> slider, WM_LBUTTONUP, 0, MAKELPARAM ((int) std::lround (geometry.centreX),
			(int) std::lround (geometry.centreY)));
	pump (40);
	return expect (commitCount == 1 && appliedValue (panel) == 40, failures,
			"the button released after a lost capture changed the value to " + number (appliedValue (panel)) +
			" % with " + number (commitCount) + " commits") && passed;
}

/* Still grabs, repeated pixels, and the native end notifications of a key: none of them reports twice. */
static bool checkReportsComeOncePerOperation (EditorAppearancePanel panel,
		std::vector <std::string> &failures) {
	bool passed = true;
	startSpying (panel);
	resetReports ();
	applyValue (panel, 58);
	SliderGeometry geometry = sliderGeometry (panel);
	pressAt (panel, geometry.centreX, geometry.centreY, "still grab");
	releaseAt (panel, geometry.centreX, geometry.centreY, "release on the same pixel");
	passed &= expect (previewCount == 0 && commitCount == 0, failures, "a still grab reported " +
			number (previewCount) + " previews and " + number (commitCount) + " commits");
	resetReports ();
	applyValue (panel, 64);
	geometry = sliderGeometry (panel);
	pressAt (panel, geometry.centreX, geometry.centreY, "press before a stuttering drag");
	for (int repetition = 0; repetition < 3; repetition ++)
		dragTo (panel, pointerForValue (panel, 70), geometry.centreY, repetition == 0, "same pixel, three times");
	dragTo (panel, pointerForValue (panel, 66), geometry.centreY, true, "back to 66 %");
	dragTo (panel, pointerForValue (panel, 70), geometry.centreY, true, "again to 70 %");
	releaseAt (panel, pointerForValue (panel, 70), geometry.centreY, "release at 70 %");
	passed &= expect (previewCount == 3, failures, "the stuttering drag previews " + number (previewCount) +
			" values instead of 3");
	passed &= expect (commitCount == 1, failures, "the stuttering drag commits " + number (commitCount) + " times");
	int repeated = 0;
	for (size_t i = 1; i < previewed.size (); i ++)
		repeated += previewed [i] == previewed [i - 1] ? 1 : 0;
	passed &= expect (repeated == 0, failures, number (repeated) + " previews repeat the value before them");
	/* The notifications a native end of track sends: the value is already in effect. */
	const int previewsAfterDrag = previewCount, commitsAfterDrag = commitCount;
	for (const int code : { TB_THUMBTRACK, TB_THUMBPOSITION, TB_ENDTRACK, TB_PAGEUP, TB_BOTTOM }) {
		SendMessageW (panel -> window, WM_HSCROLL, MAKEWPARAM (code, 0), (LPARAM) panel -> slider);
		pump (10);
	}
	passed &= expect (previewCount == previewsAfterDrag && commitCount == commitsAfterDrag, failures,
			"native end notifications after a finished drag report " + number (previewCount - previewsAfterDrag) +
			" previews and " + number (commitCount - commitsAfterDrag) + " commits more");
	/* Holding the button while the value returns to where the operation started is not a change. */
	resetReports ();
	applyValue (panel, 64);
	geometry = sliderGeometry (panel);
	pressAt (panel, geometry.centreX, geometry.centreY, "press before a round trip to the same value");
	dragTo (panel, pointerForValue (panel, 71), geometry.centreY, true, "out to 71 %");
	dragTo (panel, pointerForValue (panel, 64), geometry.centreY, true, "back to 64 %");
	releaseAt (panel, pointerForValue (panel, 64), geometry.centreY, "release on the starting value");
	passed &= expect (previewCount == 2 && commitCount == 0, failures, "a round trip back to 64 % reports " +
			number (previewCount) + " previews and " + number (commitCount) + " commits");
	stopSpying (panel);
	return passed;
}

/* The keyboard is still the native control's: one percent, ten percent, the ends, one commit each. */
static bool checkKeyboardAndAccessibility (EditorAppearancePanel panel, std::vector <std::string> &failures) {
	bool passed = true;
	startSpying (panel);
	resetReports ();
	applyValue (panel, 64);
	const LONG styles = (LONG) GetWindowLongPtrW (panel -> slider, GWL_STYLE);
	passed &= expect ((styles & WS_TABSTOP) != 0, failures, "the slider is not a tab stop");
	passed &= expect (GetNextDlgTabItem (panel -> window, panel -> chip [2], FALSE) == panel -> slider,
			failures, "Tab from the last colour chip does not go to the slider");
	const int rangeMinimum = (int) SendMessageW (panel -> slider, TBM_GETRANGEMIN, 0, 0);
	const int rangeMaximum = (int) SendMessageW (panel -> slider, TBM_GETRANGEMAX, 0, 0);
	passed &= expect (rangeMinimum == MINIMUM_OPACITY && rangeMaximum == MAXIMUM_OPACITY,
			failures, "the native range is " + number (rangeMinimum) + ".." + number (rangeMaximum));
	const struct { int key; int want; const char *name; } keys [] = {
		{ VK_RIGHT, 65, "Right" }, { VK_LEFT, 64, "Left" }, { VK_NEXT, 74, "PageDown" },
		{ VK_PRIOR, 64, "PageUp" }, { VK_HOME, MINIMUM_OPACITY, "Home" }, { VK_END, MAXIMUM_OPACITY, "End" }
	};
	for (const auto &key : keys) {
		resetReports ();
		notifications.clear ();
		SendMessageW (panel -> slider, WM_KEYDOWN, key.key, 0);
		SendMessageW (panel -> slider, WM_KEYUP, key.key, 0);
		pump (30);
		const int value = appliedValue (panel);
		passed &= expect (value == key.want, failures, std::string (key.name) + " reaches " + number (value) +
				" % instead of " + number (key.want));
		passed &= expect (commitCount == 1, failures, std::string (key.name) + " commits " + number (commitCount) +
				" times for one key");
		passed &= expect (previewCount == 0, failures, std::string (key.name) + " previews " +
				number (previewCount) + " times");
		passed &= expect (readoutPercent (panel) == value && accessiblePercent (panel) == value, failures,
				std::string (key.name) + ": the read-out shows " + number (readoutPercent (panel)) +
				" and the accessible name " + number (accessiblePercent (panel)) + ", the control has " + number (value));
		passed &= expect (! notifications.empty (), failures, std::string (key.name) + " sends no native notification");
		writeEvent ("key", panel, -1, -1, std::string (key.name) + " -> " + number (value) +
				" %, notifications " + number ((int) notifications.size ()) + ", last " +
				notificationName (lastNotificationCode));
	}
	stopSpying (panel);
	return passed;
}

/* A percentage the window cannot show ends the drag; the panel keeps showing the value that is in effect. */
static bool checkRefusalEndsTheDrag (EditorAppearancePanel panel, std::vector <std::string> &failures) {
	bool passed = true;
	/* Refused while previewing: the write-back is the end of this drag. */
	resetReports ();
	applyValue (panel, 41);
	refusedPercent = 44;
	refuseOnPreview = true;
	SliderGeometry geometry = sliderGeometry (panel);
	pressAt (panel, geometry.centreX, geometry.centreY, "press before a refused percentage");
	dragTo (panel, pointerForValue (panel, 44), geometry.centreY, true, "the refused 44 %");
	passed &= expect (! panel -> dragging, failures, "a refused preview leaves the drag running");
	passed &= expect (appliedValue (panel) == 41 && readoutPercent (panel) == 41, failures,
			"a refused preview leaves the value at " + number (appliedValue (panel)) + " % and the read-out at " +
			number (readoutPercent (panel)));
	passed &= expect (previewCount == 1 && commitCount == 1, failures, "a refused preview reports " +
			number (previewCount) + " previews and " + number (commitCount) + " commits");
	dragTo (panel, pointerForValue (panel, 60), geometry.centreY, false);
	releaseAt (panel, pointerForValue (panel, 60), geometry.centreY, "the button after a refused drag");
	passed &= expect (previewCount == 1 && commitCount == 1 && appliedValue (panel) == 41, failures,
			"the rest of a refused drag still changes the value to " + number (appliedValue (panel)) + " %");
	/* Refused only at the end: the finished change is reported once, then the real value comes back. */
	resetReports ();
	applyValue (panel, 41);
	refusedPercent = 44;
	refuseOnPreview = false;
	geometry = sliderGeometry (panel);
	pressAt (panel, geometry.centreX, geometry.centreY, "press before a refused commit");
	dragTo (panel, pointerForValue (panel, 44), geometry.centreY, true, "44 % while dragging");
	passed &= expect (panel -> dragging && appliedValue (panel) == 44, failures,
			"a commit-only refusal stops the drag too early");
	releaseAt (panel, pointerForValue (panel, 44), geometry.centreY, "release on the refused value");
	passed &= expect (commitCount == 1, failures, "a refused commit happens " + number (commitCount) + " times");
	passed &= expect (appliedValue (panel) == 41 && readoutPercent (panel) == 41, failures,
			"after a refused commit the panel shows " + number (appliedValue (panel)) + " % instead of the value in effect");
	/* A refusal does not break the following operation. */
	resetReports ();
	refusedPercent = -1;
	geometry = sliderGeometry (panel);
	pressAt (panel, geometry.centreX, geometry.centreY, "press after the refusals");
	dragTo (panel, pointerForValue (panel, 55), geometry.centreY, true, "55 % after a refusal");
	releaseAt (panel, pointerForValue (panel, 55), geometry.centreY, "release after the refusals");
	passed &= expect (appliedValue (panel) == 55 && commitCount == 1, failures, "the drag after a refusal reaches " +
			number (appliedValue (panel)) + " % with " + number (commitCount) + " commits");
	return passed;
}

static bool sliderInteractionChecks (EditorAppearancePanel panel, HWND owner,
		std::vector <std::string> &failures) {
	moveTheRealPointerAway (panel);
	bool passed = true;
	currentCheck = "geometryAndTravel";
	passed &= checkGeometry (panel, failures);
	currentCheck = "continuityFromNonRoundStarts";
	passed &= checkContinuity (panel, failures);
	currentCheck = "fastTravelAndEndpoints";
	passed &= checkFastTravelAndEndpoints (panel, failures);
	currentCheck = "handlePressesKeepTheOffset";
	passed &= checkHandlePressesKeepTheOffset (panel, failures);
	currentCheck = "channelPressPositionsAndKeepsHolding";
	passed &= checkChannelPressPositionsAndKeepsHolding (panel, failures);
	currentCheck = "dragLeavesTheRowAndThePanel";
	passed &= checkDragLeavesTheRowAndThePanel (panel, failures);
	currentCheck = "escapeCancelsAndReopens";
	passed &= checkEscapeCancelsAndReopens (panel, owner, failures);
	currentCheck = "captureLossEndsTheDrag";
	passed &= checkCaptureLossEndsTheDrag (panel, owner, failures);
	currentCheck = "reportsComeOncePerOperation";
	passed &= checkReportsComeOncePerOperation (panel, failures);
	currentCheck = "keyboardAndAccessibility";
	passed &= checkKeyboardAndAccessibility (panel, failures);
	currentCheck = "refusalEndsTheDrag";
	passed &= checkRefusalEndsTheDrag (panel, failures);
	currentCheck = "none";
	return passed;
}

static LRESULT CALLBACK ownerProcedure (HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_ERASEBKGND) {
		RECT rectangle;
		GetClientRect (window, & rectangle);
		HBRUSH background = CreateSolidBrush (RGB (88, 95, 104));
		FillRect ((HDC) wParam, & rectangle, background);
		DeleteObject (background);
		return 1;
	}
	return DefWindowProcW (window, message, wParam, lParam);
}

static bool capture (HWND window, const wchar_t *path) {
	RECT rectangle;
	GetWindowRect (window, & rectangle);
	const int width = rectangle.right - rectangle.left, height = rectangle.bottom - rectangle.top;
	HDC screen = GetDC (nullptr), memory = CreateCompatibleDC (screen);
	HBITMAP bitmap = CreateCompatibleBitmap (screen, width, height);
	HGDIOBJ oldBitmap = SelectObject (memory, bitmap);
	BitBlt (memory, 0, 0, width, height, screen, rectangle.left, rectangle.top, SRCCOPY);
	SelectObject (memory, oldBitmap);
	bool saved = false;
	{
		Gdiplus::Bitmap image (bitmap, nullptr);
		UINT count, size;
		Gdiplus::GetImageEncodersSize (& count, & size);
		std::vector <BYTE> storage (size);
		auto encoders = (Gdiplus::ImageCodecInfo *) storage.data ();
		if (Gdiplus::GetImageEncoders (count, size, encoders) == Gdiplus::Ok)
			for (UINT i = 0; i < count; i ++)
				if (wcscmp (encoders [i].MimeType, L"image/png") == 0)
					saved = image.Save (path, & encoders [i].Clsid, nullptr) == Gdiplus::Ok;
	}
	DeleteObject (bitmap);
	DeleteDC (memory);
	ReleaseDC (nullptr, screen);
	return saved;
}

static std::string jsonEscape (const std::string &text) {
	std::string result;
	for (const char c : text)
		if (c == '"' || c == '\\')
			result += '\\', result += c;
		else if ((unsigned char) c >= 0x20)
			result += c;
	return result;
}

int main (int, char *[]) {
	SetProcessDpiAwarenessContext (DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	theGui.instance = GetModuleHandleW (nullptr);
	INITCOMMONCONTROLSEX controls { sizeof (controls), ICC_BAR_CLASSES };
	InitCommonControlsEx (& controls);
	WNDCLASSW ownerClass {};
	ownerClass.lpfnWndProc = ownerProcedure;
	ownerClass.hInstance = theGui.instance;
	ownerClass.lpszClassName = L"AppearancePanelTestOwner";
	RegisterClassW (& ownerClass);
	HWND owner = CreateWindowExW (0, ownerClass.lpszClassName, L"Appearance panel layout test",
		WS_POPUP | WS_VISIBLE, 100, 80, 1000, 850, nullptr, nullptr, theGui.instance, nullptr);
	EditorAppearancePanel_callbacks callbacks { nullptr, nullptr, recordPreview, recordCommit };
	EditorAppearancePanel panel = EditorAppearancePanel_create (& callbacks);
	if (! owner || ! panel)
		return 2;
	thePanel = panel;
	openEventLog ();
	std::ofstream report ("test-records/appearance-panel-native/layout.json");
	report << "{\"method\":\"native controls; test-only DPI provider; monitor transitions not covered\",\"monitorDpi\":"
		<< GetDpiForWindow (owner) << ",\"layouts\":[";
	bool allPassed = true;
	for (const UINT dpi : { 96u, 120u, 144u, 192u }) {
		layoutDpi = dpi;
		std::vector <std::string> failures;
		EditorAppearancePanel_state state { kEditorAppearancePalette::MIST_BLUE_GREY, 100 };
		bool passed = EditorAppearancePanel_show (panel, owner, 150, 100, 40, 24, & state);
		pump ();
		const SliderGeometry firstHandle = sliderGeometry (panel);
		passed &= expect (std::abs (firstHandle.centreX - panel -> travelRight) <= 1.0f,
				failures, "the fresh fully opaque handle is not at its displayed endpoint");
		pressAt (panel, firstHandle.centreX, firstHandle.centreY, "first press at the initial handle");
		releaseAt (panel, firstHandle.centreX, firstHandle.centreY, "first release without moving");
		passed &= expect (appliedValue (panel) == 100, failures,
				"the first click at the initial handle changed the value without movement");
		state.opacityPercent = 64;
		EditorAppearancePanel_setState (panel, & state);
		pump (20);
		RECT window;
		GetWindowRect (panel -> window, & window);
		passed &= window.right - window.left == MulDiv (328, dpi, 96);
		passed &= window.bottom - window.top == MulDiv (248, dpi, 96);
		RECT chrome;
		GetWindowRect (panel -> chrome, & chrome);
		const bool chromeAligned = window.left - chrome.left == MulDiv (18, dpi, 96) &&
			window.top - chrome.top == MulDiv (18, dpi, 96) &&
			chrome.right - window.right == MulDiv (18, dpi, 96) &&
			chrome.bottom - window.bottom == MulDiv (18, dpi, 96);
		const LONG_PTR chromeStyle = GetWindowLongPtrW (panel -> chrome, GWL_EXSTYLE);
		passed &= chromeAligned && IsWindowVisible (panel -> chrome) &&
			(chromeStyle & WS_EX_NOACTIVATE) && (chromeStyle & WS_EX_TRANSPARENT);
		RECT previous {};
		for (int slot = 0; slot < 3; slot ++) {
			RECT chip;
			GetWindowRect (panel -> chip [slot], & chip);
			passed &= chip.right - chip.left == MulDiv (88, dpi, 96);
			passed &= chip.bottom - chip.top == MulDiv (82, dpi, 96);
			passed &= chip.left >= window.left && chip.right <= window.right;
			if (slot > 0)
				passed &= chip.top == previous.top && chip.left - previous.right == MulDiv (8, dpi, 96);
			HDC dc = GetDC (panel -> chip [slot]);
			HGDIOBJ font = SelectObject (dc, panel -> font);
			SIZE text;
			GetTextExtentPoint32W (dc, chipTitle (slot), (int) wcslen (chipTitle (slot)), & text);
			passed &= text.cx <= chip.right - chip.left - MulDiv (16, dpi, 96);
			passed &= text.cy <= MulDiv (19, dpi, 96);
			SelectObject (dc, font);
			ReleaseDC (panel -> chip [slot], dc);
			previous = chip;
		}
		RECT slider, percent;
		GetWindowRect (panel -> slider, & slider);
		GetWindowRect (panel -> percent, & percent);
		passed &= percent.bottom <= slider.top && percent.right == slider.right;
		passed &= slider.right - slider.left == MulDiv (280, dpi, 96);
		passed &= slider.top - previous.bottom == MulDiv (56, dpi, 96);
		passed &= std::abs (window.bottom - slider.bottom - MulDiv (24, dpi, 96)) <= 1;
		const auto panelPixels = pixelsOnScreen (panel -> window);
		int blendedCornerPixels = 0;
		const int corner = MulDiv (14, dpi, 96), panelWidth = window.right - window.left;
		for (int y = 0; y < corner; y ++)
			for (int x = 0; x < corner; x ++) {
				const BYTE red = panelPixels [(y * panelWidth + x) * 4 + 2];
				if (red > 110 && red < 245)
					blendedCornerPixels ++;
			}
		const bool smoothCorner = blendedCornerPixels >= 8;
		passed &= smoothCorner;
		int softOutlinePixels = 0, minimumSolidOutlinePixels = 0, roundedOutlinePixels = 0;
		const bool crispOutline = outlineIsCrisp (panel, softOutlinePixels, minimumSolidOutlinePixels, roundedOutlinePixels);
		passed &= crispOutline;
		int maximumThumbFade = 0;
		const bool crispThumb = sliderExtremaAreCrisp (panel, maximumThumbFade);
		passed &= crispThumb;
		wchar_t filename [160];
		swprintf (filename, L"test-records/appearance-panel-native/panel-%u.png", dpi);
		passed &= capture (panel -> window, filename);
		swprintf (filename, L"test-records/appearance-panel-native/frame-%u.png", dpi);
		passed &= capture (panel -> chrome, filename);
		const bool repaintStable = sliderRepaintIsStable (panel);
		passed &= repaintStable;
		for (int choice = 0; choice < 9; choice ++) {
			SendMessageW (panel -> chip [choice % 3], BM_CLICK, 0, 0);
			pump (10);
		}
		bool chipsStable = true;
		for (HWND chip : panel -> chip) {
			const auto switched = pixelsOnScreen (chip);
			InvalidateRect (chip, nullptr, FALSE);
			UpdateWindow (chip);
			DwmFlush ();
			chipsStable &= switched == pixelsOnScreen (chip);
		}
		passed &= chipsStable;
		SendMessageW (panel -> chip [2], BM_CLICK, 0, 0);
		passed &= panel -> state.palette == kEditorAppearancePalette::CLAY_GREY;
		SendMessageW (panel -> slider, WM_KEYDOWN, VK_HOME, 0);
		passed &= SendMessageW (panel -> slider, TBM_GETPOS, 0, 0) == 20;
		SendMessageW (panel -> slider, WM_KEYDOWN, VK_END, 0);
		passed &= SendMessageW (panel -> slider, TBM_GETPOS, 0, 0) == 100;
		SendMessageW (panel -> slider, WM_KEYDOWN, VK_ESCAPE, 0);
		pump (20);
		const bool dismissedTogether = ! IsWindowVisible (panel -> window) && ! IsWindowVisible (panel -> chrome);
		passed &= dismissedTogether;
		/* The same panel, driven as a press and a drag drive it. */
		currentCheck = "none";
		resetReports ();
		openPanel (panel, owner, 64);
		const float travel = panel -> travelRight - panel -> travelLeft;
		const bool interaction = sliderInteractionChecks (panel, owner, failures);
		passed &= interaction;
		allPassed &= passed;
		if (dpi != 96)
			report << ',';
		report << "{\"dpi\":" << dpi << ",\"width\":" << window.right - window.left
			<< ",\"height\":" << window.bottom - window.top << ",\"cornerBlendPixels\":" << blendedCornerPixels
			<< ",\"repaintStable\":" << (repaintStable ? "true" : "false")
			<< ",\"chipsStable\":" << (chipsStable ? "true" : "false")
			<< ",\"softOutlinePixels\":" << softOutlinePixels
			<< ",\"minimumSolidOutlinePixels\":" << minimumSolidOutlinePixels
			<< ",\"roundedOutlinePixels\":" << roundedOutlinePixels
			<< ",\"crispOutline\":" << (crispOutline ? "true" : "false")
			<< ",\"maximumThumbFade\":" << maximumThumbFade
			<< ",\"crispThumb\":" << (crispThumb ? "true" : "false")
			<< ",\"chromeAligned\":" << (chromeAligned ? "true" : "false")
			<< ",\"dismissedTogether\":" << (dismissedTogether ? "true" : "false")
			<< ",\"handleTravel\":" << number (travel)
			<< ",\"quantisationBoundInPixels\":" << number (quantisationBound (panel))
			<< ",\"interactionPassed\":" << (interaction ? "true" : "false")
			<< ",\"failures\":[";
		for (size_t i = 0; i < failures.size (); i ++)
			report << (i ? "," : "") << '"' << jsonEscape (failures [i]) << '"';
		report << "],\"passed\":" << (passed ? "true" : "false") << '}';
		std::cout << dpi << " dpi layout: " << (passed ? "passed" : "FAILED")
			<< "; corner blend pixels " << blendedCornerPixels << "; drag repaint " << repaintStable
			<< "; soft outline pixels " << softOutlinePixels << "; thumb fade " << maximumThumbFade
			<< "; handle travel " << travel << " px; interaction " << (interaction ? "passed" : "FAILED")
			<< "; failures " << failures.size () << '\n';
		for (const std::string &failure : failures)
			std::cout << "   " << failure << '\n';
	}
	/* A panel that goes away with a held button reports nothing: the editor is already closing. */
	currentCheck = "dragDoesNotOutliveTheWindows";
	resetReports ();
	openPanel (panel, owner, 50);
	{
		const SliderGeometry geometry = sliderGeometry (panel);
		pressAt (panel, geometry.centreX, geometry.centreY, "press before destruction");
		dragTo (panel, pointerForValue (panel, 67), geometry.centreY, true, "dragged to 67 %");
	}
	const int previewsBeforeDestroy = previewCount;
	const int commitsBeforeDestroy = commitCount;
	const HWND lastChrome = panel -> chrome;
	const bool chromeAlive = lastChrome && IsWindow (lastChrome);
	EditorAppearancePanel_dispose (panel);   // v9_destroy takes the panel away while the button is held
	const bool destroyed = ! IsWindow (lastChrome);
	const bool destroyIsQuiet = chromeAlive && destroyed && commitCount == commitsBeforeDestroy &&
			previewCount == previewsBeforeDestroy;
	allPassed &= destroyed;
	allPassed &= destroyIsQuiet;
	eventLog << "]}";
	eventLog.close ();
	report << "],\"events\":\"test-records/appearance-panel-native/drag-events.json\""
		<< ",\"destroyIsQuiet\":" << (destroyIsQuiet ? "true" : "false")
		<< ",\"previewsBeforeDestroy\":" << previewsBeforeDestroy
		<< ",\"commitsDuringDestroy\":" << commitCount
		<< ",\"chromeDestroyed\":" << (destroyed ? "true" : "false")
		<< ",\"passed\":" << (allPassed ? "true" : "false") << '}';
	DestroyWindow (owner);
	return allPassed ? 0 : 1;
}
