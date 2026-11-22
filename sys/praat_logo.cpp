/* praat_logo.cpp
 *
 * Copyright (C) 1996-2024 Paul Boersma
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

#include "praatP.h"
#include "Picture.h"
#include "GraphicsP.h"
#include "GuiP.h"
#include "praat_translate.h"
#include "praat_zh_version.h"
#include "praat_zh_logo_data.h"

#include <atomic>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#if defined (_WIN32)
	#include <windows.h>
	#include <shellapi.h>
	#include <objidl.h>
	#include <gdiplus.h>
#elif defined (macintosh)
	#include <dispatch/dispatch.h>
	#include <CoreGraphics/CoreGraphics.h>
	#include <unistd.h>
#elif defined (UNIX)
	#include <unistd.h>
	#if gtk
		#include <gtk/gtk.h>
	#endif
	#if cairo
		#include <cairo/cairo.h>
	#endif
#endif

static void logo_defaultDraw (Graphics g) {
	Graphics_setColour (g, Melder_MAGENTA);
	Graphics_fillRectangle (g, 0.0, 1.0, 0.0, 1.0);
	Graphics_setGrey (g, 0.5);
	Graphics_fillRectangle (g, 0.05, 0.95, 0.1, 0.9);
	Graphics_setTextAlignment (g, Graphics_CENTRE, Graphics_HALF);
	Graphics_setColour (g, Melder_YELLOW);
	Graphics_setFont (g, kGraphics_font::TIMES);
	Graphics_setFontSize (g, 24);
	Graphics_setFontStyle (g, Graphics_ITALIC);
	Graphics_setUnderscoreIsSubscript (g, false);   // because program names may contain underscores
	Graphics_text (g, 0.5, 0.6, Melder_upperCaseAppName());
	Graphics_setFontStyle (g, 0);
	Graphics_setFontSize (g, 12);
	Graphics_text (g, 0.5, 0.25, U"\\s{Built on the} %%Praat shell%\\s{,© Paul Boersma, 1992-2024");
}

static struct {
	double width_mm, height_mm;
	void (*draw) (Graphics g);
	GuiDialog dia;
	GuiForm form;
	GuiDrawingArea drawingArea;
	autoGraphics graphics;
} theLogo = { 90, 40, logo_defaultDraw };

void praat_setLogo (double width_mm, double height_mm, void (*draw) (Graphics g)) {
	theLogo.width_mm = width_mm;
	theLogo.height_mm = height_mm;
	theLogo.draw = draw;
}

static void gui_drawingarea_cb_expose (Thing /* me */, GuiDrawingArea_ExposeEvent /* event */) {
	theLogo.draw (theLogo.graphics.get());
}

static void gui_drawingarea_cb_mouse (Thing /* me */, GuiDrawingArea_MouseEvent event) {
	if (event -> isClick ())
		GuiThing_hide (theLogo.form);
}

static void gui_cb_goAway (Thing /* boss */) {
 	GuiThing_hide (theLogo.form);
}

void praat_showLogo () {
	if (theCurrentPraatApplication -> batch || ! theLogo.draw)
		return;
	if (! theLogo.dia) {
		int width  = theLogo.width_mm  / 25.4 * Gui_getResolution (nullptr);
		int height = theLogo.height_mm / 25.4 * Gui_getResolution (nullptr);
		theLogo.dia = GuiDialog_create (theCurrentPraatApplication -> topShell, 100, 100, width, height,
				U"About", gui_cb_goAway, nullptr, GuiDialog_Modality::MODELESS);
		theLogo.form = theLogo.dia;
		theLogo.drawingArea = GuiDrawingArea_createShown (theLogo.form, 0, width, 0, height,
				gui_drawingarea_cb_expose, gui_drawingarea_cb_mouse, nullptr, nullptr, nullptr, nullptr, 0);
		/*
			Note about ordering the following three statements (2021-01-20).
			On some platforms (e.g. on macOS 10.10, but not 10.15), showing entails immediate drawing.
			So the Graphics has to exist before that.
			(It is possible that the *native* graphics *context* is created later.)
		*/
		theLogo.graphics = Graphics_create_xmdrawingarea (theLogo.drawingArea);
		GuiThing_show (theLogo.form);
		GuiThing_show (theLogo.dia);
	} else {
		GuiThing_show (theLogo.form);
		GuiThing_show (theLogo.dia);
	}
}

/********** Parallel About Chinese Edition dialog & online update checker **********/

namespace {

enum class UpdateCheckStatus {
	Idle,
	Checking,
	UpdateAvailable,
	UpToDate,
	Error
};

struct UpdateCheckJob {
	std::atomic <bool> finished { false };
	UpdateCheckStatus status = UpdateCheckStatus::Error;
	std::string latestTag;
	std::string downloadUrl;
};

struct ChineseAboutDialogState {
	GuiDialog dialog = nullptr;
	GuiDrawingArea logoArea = nullptr;
	autoGraphics logoGraphics;
	GuiLabel titleLabel = nullptr;
	GuiLabel versionLabel = nullptr;
	GuiLabel upstreamLabel = nullptr;
	GuiLabel platformLabel = nullptr;
	GuiLabel maintainerLabel = nullptr;
	GuiLabel licenseLabel = nullptr;
	GuiLabel homepageLabel = nullptr;
	GuiLabel statusLabel = nullptr;
	GuiButton checkUpdateButton = nullptr;
	GuiButton homepageButton = nullptr;
	GuiButton downloadButton = nullptr;
	GuiButton closeButton = nullptr;

	UpdateCheckStatus updateStatus = UpdateCheckStatus::Idle;
	std::string latestTag;
	std::string latestUrl;
	std::shared_ptr <UpdateCheckJob> activeJob;
	#if defined (_WIN32)
		UINT_PTR pollTimerId = 0;
		HFONT titleFont = nullptr;
	#endif
};

static ChineseAboutDialogState theChineseAbout;

static conststring32 trZh (conststring32 enText, conststring32 zhSimplified) {
	if (g_language_choice == 0)
		return enText;
	return praat_translate (zhSimplified);
}

static conststring32 getBuiltPlatformText () {
	[[maybe_unused]] constexpr bool isArm64 =
		#if defined (__aarch64__) || defined (_M_ARM64_)
			true;
		#else
			false;
		#endif
	[[maybe_unused]] constexpr int x64_version =
		#if defined (__AVX2__) && defined (__FMA__) && defined (__F16C__)
			3;
		#else
			1;
		#endif
	#if defined (macintosh)
		return isArm64 ? U"ARM64 macOS" : U"Intel64 macOS";
	#elif defined (_WIN32)
		return isArm64 ? U"ARM64 Windows" :
			sizeof (void *) == 4 ? U"x86 Windows" :
			x64_version == 3 ? U"x64 (AVX2) Windows" :
			U"x64 Windows";
	#elif defined (linux)
		return isArm64 ? U"ARM64 Linux" : U"x64 Linux";
	#else
		return U"Desktop";
	#endif
}

static void openUrlInBrowser (const std::string& url) {
	if (url.rfind ("https://", 0) != 0 && url.rfind ("http://", 0) != 0)
		return;
	#if defined (_WIN32)
		int wlen = MultiByteToWideChar (CP_UTF8, 0, url.c_str (), -1, nullptr, 0);
		if (wlen > 0) {
			std::vector <wchar_t> wurl (static_cast <size_t> (wlen));
			MultiByteToWideChar (CP_UTF8, 0, url.c_str (), -1, wurl.data (), wlen);
			ShellExecuteW (nullptr, L"open", wurl.data (), nullptr, nullptr, SW_SHOWNORMAL);
		}
	#elif defined (macintosh)
		pid_t pid = fork ();
		if (pid == 0) {
			execlp ("open", "open", url.c_str (), static_cast <char *> (nullptr));
			_exit (127);
		}
	#elif defined (UNIX)
		pid_t pid = fork ();
		if (pid == 0) {
			execlp ("xdg-open", "xdg-open", url.c_str (), static_cast <char *> (nullptr));
			_exit (127);
		}
	#endif
}

static std::string extractJsonStringField (const std::string& json, const char *fieldName) {
	const std::string needle = std::string ("\"") + fieldName + "\"";
	size_t pos = json.find (needle);
	while (pos != std::string::npos) {
		size_t colon = pos + needle.size ();
		while (colon < json.size () && (json [colon] == ' ' || json [colon] == '\t' || json [colon] == '\r' || json [colon] == '\n'))
			++ colon;
		if (colon < json.size () && json [colon] == ':') {
			size_t q = colon + 1;
			while (q < json.size () && (json [q] == ' ' || json [q] == '\t' || json [q] == '\r' || json [q] == '\n'))
				++ q;
			if (q < json.size () && json [q] == '"') {
				++ q;
				std::string out;
				while (q < json.size ()) {
					char c = json [q ++];
					if (c == '\\' && q < json.size ()) {
						char esc = json [q ++];
						if (esc == '"' || esc == '\\' || esc == '/')
							out.push_back (esc);
						else if (esc == 'n')
							out.push_back ('\n');
						else if (esc == 't')
							out.push_back ('\t');
					} else if (c == '"') {
						return out;
					} else {
						out.push_back (c);
					}
				}
			}
		}
		pos = json.find (needle, pos + needle.size ());
	}
	return {};
}

struct ParsedZhVersion {
	std::vector <int> baseParts;
	std::vector <int> zhParts;
};

static std::vector <int> parseNumericSequence (const std::string& s) {
	std::vector <int> nums;
	int cur = 0;
	bool inNum = false;
	for (unsigned char ch : s) {
		if (ch >= '0' && ch <= '9') {
			cur = cur * 10 + (ch - '0');
			inNum = true;
		} else if (inNum) {
			nums.push_back (cur);
			cur = 0;
			inNum = false;
		}
	}
	if (inNum)
		nums.push_back (cur);
	return nums;
}

static ParsedZhVersion parseZhVersionTag (const std::string& tag) {
	ParsedZhVersion out;
	std::string lower = tag;
	for (char& c : lower)
		if (c >= 'A' && c <= 'Z')
			c = static_cast <char> (c - 'A' + 'a');
	size_t zhPos = lower.find ("zh");
	if (zhPos != std::string::npos) {
		out.baseParts = parseNumericSequence (lower.substr (0, zhPos));
		out.zhParts   = parseNumericSequence (lower.substr (zhPos + 2));
	} else {
		out.baseParts = parseNumericSequence (lower);
	}
	return out;
}

// Returns > 0 if remoteTag > localTag, 0 if equal, < 0 if remoteTag < localTag
static int compareZhVersionTags (const std::string& localTag, const std::string& remoteTag) {
	const ParsedZhVersion a = parseZhVersionTag (localTag);
	const ParsedZhVersion b = parseZhVersionTag (remoteTag);
	const size_t maxBase = std::max (a.baseParts.size (), b.baseParts.size ());
	for (size_t i = 0; i < maxBase; ++ i) {
		int va = (i < a.baseParts.size ()) ? a.baseParts [i] : 0;
		int vb = (i < b.baseParts.size ()) ? b.baseParts [i] : 0;
		if (vb != va)
			return (vb > va) ? 1 : -1;
	}
	const size_t maxZh = std::max (a.zhParts.size (), b.zhParts.size ());
	for (size_t i = 0; i < maxZh; ++ i) {
		int va = (i < a.zhParts.size ()) ? a.zhParts [i] : 0;
		int vb = (i < b.zhParts.size ()) ? b.zhParts [i] : 0;
		if (vb != va)
			return (vb > va) ? 1 : -1;
	}
	return 0;
}

static bool extractTagFromReleaseUrl (const std::string& url, std::string& tagOut) {
	const std::string marker = "/releases/tag/";
	size_t pos = url.find (marker);
	if (pos == std::string::npos)
		return false;
	pos += marker.size ();
	size_t end = url.find_first_of ("/?#", pos);
	tagOut = (end == std::string::npos) ? url.substr (pos) : url.substr (pos, end - pos);
	return ! tagOut.empty ();
}

#if defined (_WIN32)
static bool winInetFetchUrl (const wchar_t *url, std::string& responseOut, std::string& finalUrlOut, bool headOnly) {
	using HINTERNET_t = void *;
	using PFN_InternetOpenW = HINTERNET_t (WINAPI *) (LPCWSTR, DWORD, LPCWSTR, LPCWSTR, DWORD);
	using PFN_InternetSetOptionW = BOOL (WINAPI *) (HINTERNET_t, DWORD, LPVOID, DWORD);
	using PFN_InternetOpenUrlW = HINTERNET_t (WINAPI *) (HINTERNET_t, LPCWSTR, LPCWSTR, DWORD, DWORD, DWORD_PTR);
	using PFN_HttpQueryInfoW = BOOL (WINAPI *) (HINTERNET_t, DWORD, LPVOID, LPDWORD, LPDWORD);
	using PFN_InternetQueryOptionW = BOOL (WINAPI *) (HINTERNET_t, DWORD, LPVOID, LPDWORD);
	using PFN_InternetReadFile = BOOL (WINAPI *) (HINTERNET_t, LPVOID, DWORD, LPDWORD);
	using PFN_InternetCloseHandle = BOOL (WINAPI *) (HINTERNET_t);

	constexpr DWORD kOpenTypePreconfig         = 0;
	constexpr DWORD kOptionConnectTimeout      = 2;
	constexpr DWORD kOptionSendTimeout         = 5;
	constexpr DWORD kOptionReceiveTimeout      = 6;
	constexpr DWORD kOptionUrl                 = 34;
	constexpr DWORD kFlagReload                = 0x80000000;
	constexpr DWORD kFlagSecure                = 0x00800000;
	constexpr DWORD kFlagNoCacheWrite          = 0x04000000;
	constexpr DWORD kFlagPragmaNocache         = 0x00000100;
	constexpr DWORD kHttpQueryStatusCode       = 19;
	constexpr DWORD kHttpQueryFlagNumber       = 0x20000000;

	HMODULE mod = LoadLibraryW (L"wininet.dll");
	if (! mod)
		return false;

	auto pOpen        = reinterpret_cast <PFN_InternetOpenW>        (GetProcAddress (mod, "InternetOpenW"));
	auto pSetOption   = reinterpret_cast <PFN_InternetSetOptionW>   (GetProcAddress (mod, "InternetSetOptionW"));
	auto pOpenUrl     = reinterpret_cast <PFN_InternetOpenUrlW>     (GetProcAddress (mod, "InternetOpenUrlW"));
	auto pQueryInfo   = reinterpret_cast <PFN_HttpQueryInfoW>       (GetProcAddress (mod, "HttpQueryInfoW"));
	auto pQueryOption = reinterpret_cast <PFN_InternetQueryOptionW> (GetProcAddress (mod, "InternetQueryOptionW"));
	auto pReadFile    = reinterpret_cast <PFN_InternetReadFile>     (GetProcAddress (mod, "InternetReadFile"));
	auto pCloseHandle = reinterpret_cast <PFN_InternetCloseHandle>  (GetProcAddress (mod, "InternetCloseHandle"));

	if (! pOpen || ! pOpenUrl || ! pQueryInfo || ! pQueryOption || ! pReadFile || ! pCloseHandle) {
		FreeLibrary (mod);
		return false;
	}

	bool ok = false;
	HINTERNET_t hSession = pOpen (L"Praat-ZH-Updater/1.0", kOpenTypePreconfig, nullptr, nullptr, 0);
	if (hSession) {
		if (pSetOption) {
			DWORD connectTimeout = 4500;
			DWORD ioTimeout = 6000;
			pSetOption (hSession, kOptionConnectTimeout, & connectTimeout, sizeof (connectTimeout));
			pSetOption (hSession, kOptionSendTimeout, & ioTimeout, sizeof (ioTimeout));
			pSetOption (hSession, kOptionReceiveTimeout, & ioTimeout, sizeof (ioTimeout));
		}
		const DWORD flags = kFlagReload | kFlagSecure | kFlagNoCacheWrite | kFlagPragmaNocache;
		HINTERNET_t hUrl = pOpenUrl (hSession, url, nullptr, 0, flags, 0);
		if (hUrl) {
			DWORD statusCode = 0;
			DWORD statusSize = sizeof (statusCode);
			bool statusOk = pQueryInfo (hUrl, kHttpQueryStatusCode | kHttpQueryFlagNumber, & statusCode, & statusSize, nullptr)
				&& statusCode >= 200 && statusCode < 400;

			wchar_t finalUrlW [1024] = { };
			DWORD finalUrlBytes = sizeof (finalUrlW);
			if (pQueryOption (hUrl, kOptionUrl, finalUrlW, & finalUrlBytes)) {
				int utf8Len = WideCharToMultiByte (CP_UTF8, 0, finalUrlW, -1, nullptr, 0, nullptr, nullptr);
				if (utf8Len > 1) {
					finalUrlOut.resize (static_cast <size_t> (utf8Len - 1));
					WideCharToMultiByte (CP_UTF8, 0, finalUrlW, -1, finalUrlOut.data (), utf8Len, nullptr, nullptr);
				}
			}

			if (statusOk) {
				if (headOnly) {
					ok = ! finalUrlOut.empty ();
				} else {
					responseOut.clear ();
					char buf [4096];
					DWORD bytesRead = 0;
					while (pReadFile (hUrl, buf, sizeof (buf), & bytesRead) && bytesRead > 0) {
						responseOut.append (buf, bytesRead);
						if (responseOut.size () > 131072)
							break;
					}
					ok = ! responseOut.empty ();
				}
			}
			pCloseHandle (hUrl);
		}
		pCloseHandle (hSession);
	}
	FreeLibrary (mod);
	return ok;
}
#else
static bool posixCurlFetchText (const char *cmd, std::string& responseOut) {
	FILE *pipe = popen (cmd, "r");
	if (! pipe)
		return false;
	responseOut.clear ();
	char buf [4096];
	size_t n = 0;
	while ((n = fread (buf, 1, sizeof (buf), pipe)) > 0) {
		responseOut.append (buf, n);
		if (responseOut.size () > 131072)
			break;
	}
	int rc = pclose (pipe);
	return (rc == 0 && ! responseOut.empty ());
}
#endif

static bool fetchLatestReleaseInfo (std::string& tagOut, std::string& urlOut) {
	tagOut.clear ();
	urlOut.clear ();
	#if defined (_WIN32)
		// Source 1: GitHub releases/latest redirect (zero API rate-limit, always real-time, uses system proxy via WinINet)
		{
			std::string dummy, finalUrl;
			if (winInetFetchUrl (PRAAT_ZH_LATEST_URL_W, dummy, finalUrl, true) &&
			    extractTagFromReleaseUrl (finalUrl, tagOut))
			{
				urlOut = finalUrl;
				return true;
			}
		}
		// Source 2: jsDelivr CDN mirror of docs/version.json (direct access in China without proxy)
		{
			std::string body, finalUrl;
			if (winInetFetchUrl (PRAAT_ZH_CDN_VER_URL_W, body, finalUrl, false)) {
				tagOut = extractJsonStringField (body, "tag_name");
				urlOut = extractJsonStringField (body, "html_url");
				if (! tagOut.empty ()) {
					if (urlOut.empty ())
						urlOut = Melder_peek32to8 (PRAAT_ZH_RELEASES_URL);
					return true;
				}
			}
		}
		// Source 3: GitHub Pages docs/version.json fallback
		{
			std::string body, finalUrl;
			if (winInetFetchUrl (PRAAT_ZH_PAGES_VER_URL_W, body, finalUrl, false)) {
				tagOut = extractJsonStringField (body, "tag_name");
				urlOut = extractJsonStringField (body, "html_url");
				if (! tagOut.empty ()) {
					if (urlOut.empty ())
						urlOut = Melder_peek32to8 (PRAAT_ZH_RELEASES_URL);
					return true;
				}
			}
		}
		return false;
	#else
		// Source 1: GitHub releases/latest redirect URL (zero API rate-limit)
		{
			std::string finalUrl;
			const std::string cmd = "curl -fsSLI --max-time 5 -A Praat-ZH-Updater/1.0 -o /dev/null -w \"%{url_effective}\" \"" PRAAT_ZH_LATEST_URL_8 "\" 2>/dev/null";
			if (posixCurlFetchText (cmd.c_str (), finalUrl) &&
			    extractTagFromReleaseUrl (finalUrl, tagOut))
			{
				urlOut = finalUrl;
				return true;
			}
		}
		// Source 2 & 3: jsDelivr CDN -> GitHub Pages static version.json
		const char *jsonUrls [] = { PRAAT_ZH_CDN_VER_URL_8, PRAAT_ZH_PAGES_VER_URL_8 };
		for (const char *u : jsonUrls) {
			std::string body;
			const std::string cmd = std::string ("curl -fsSL --max-time 6 -A Praat-ZH-Updater/1.0 \"") + u + "\" 2>/dev/null";
			if (posixCurlFetchText (cmd.c_str (), body)) {
				tagOut = extractJsonStringField (body, "tag_name");
				urlOut = extractJsonStringField (body, "html_url");
				if (! tagOut.empty ()) {
					if (urlOut.empty ())
						urlOut = Melder_peek32to8 (PRAAT_ZH_RELEASES_URL);
					return true;
				}
			}
		}
		return false;
	#endif
}

#if gdi
static Gdiplus::Bitmap * getCachedWinLogoBitmap () {
	static Gdiplus::Bitmap *s_bitmap = nullptr;
	static bool s_attempted = false;
	if (s_attempted)
		return s_bitmap;
	s_attempted = true;

	static ULONG_PTR s_gdiplusToken = 0;
	static bool s_gdiplusStarted = false;
	if (! s_gdiplusStarted) {
		Gdiplus::GdiplusStartupInput input;
		if (Gdiplus::GdiplusStartup (& s_gdiplusToken, & input, nullptr) == Gdiplus::Ok)
			s_gdiplusStarted = true;
	}
	if (! s_gdiplusStarted)
		return nullptr;

	HGLOBAL hMem = GlobalAlloc (GMEM_MOVEABLE, kPraatZhLogoPngSize);
	if (! hMem)
		return nullptr;
	void *pMem = GlobalLock (hMem);
	if (! pMem) {
		GlobalFree (hMem);
		return nullptr;
	}
	std::memcpy (pMem, kPraatZhLogoPng, kPraatZhLogoPngSize);
	GlobalUnlock (hMem);

	IStream *stream = nullptr;
	if (SUCCEEDED (CreateStreamOnHGlobal (hMem, TRUE, & stream)) && stream) {
		std::unique_ptr <Gdiplus::Bitmap> decoded (Gdiplus::Bitmap::FromStream (stream));
		if (decoded && decoded -> GetLastStatus () == Gdiplus::Ok) {
			const UINT w = decoded -> GetWidth ();
			const UINT h = decoded -> GetHeight ();
			if (w > 0 && h > 0) {
				Gdiplus::Bitmap *cloned = decoded -> Clone (0, 0, static_cast <INT> (w), static_cast <INT> (h), PixelFormat32bppPARGB);
				if (cloned && cloned -> GetLastStatus () == Gdiplus::Ok)
					s_bitmap = cloned;
				else
					delete cloned;
			}
		}
		stream -> Release ();
	} else {
		GlobalFree (hMem);
	}
	return s_bitmap;
}
#elif quartz
static CGImageRef getCachedMacLogoImage () {
	static CGImageRef s_image = nullptr;
	static bool s_attempted = false;
	if (s_attempted)
		return s_image;
	s_attempted = true;
	CGDataProviderRef provider = CGDataProviderCreateWithData (nullptr, kPraatZhLogoPng, kPraatZhLogoPngSize, nullptr);
	if (provider) {
		s_image = CGImageCreateWithPNGDataProvider (provider, nullptr, true, kCGRenderingIntentDefault);
		CGDataProviderRelease (provider);
	}
	return s_image;
}
#elif cairo
struct CairoPngReadState {
	const unsigned char *data;
	size_t size;
	size_t offset;
};

static cairo_status_t cairoReadLogoPng (void *closure, unsigned char *out, unsigned int length) {
	auto *st = static_cast <CairoPngReadState *> (closure);
	if (st -> offset + length > st -> size)
		return CAIRO_STATUS_READ_ERROR;
	std::memcpy (out, st -> data + st -> offset, length);
	st -> offset += length;
	return CAIRO_STATUS_SUCCESS;
}

static cairo_surface_t * getCachedCairoLogoSurface () {
	static cairo_surface_t *s_surface = nullptr;
	static bool s_attempted = false;
	if (s_attempted)
		return s_surface;
	s_attempted = true;
	CairoPngReadState st { kPraatZhLogoPng, kPraatZhLogoPngSize, 0 };
	cairo_surface_t *sfc = cairo_image_surface_create_from_png_stream (cairoReadLogoPng, & st);
	if (sfc && cairo_surface_status (sfc) == CAIRO_STATUS_SUCCESS)
		s_surface = sfc;
	else if (sfc)
		cairo_surface_destroy (sfc);
	return s_surface;
}
#endif

static void gui_chineseAbout_logoExpose (Thing /* me */, GuiDrawingArea_ExposeEvent /* event */) {
	Graphics g = theChineseAbout.logoGraphics.get ();
	if (! g)
		return;
	Graphics_setWindow (g, 0.0, 1.0, 0.0, 1.0);
	Graphics_setColour (g, Melder_WHITE);
	Graphics_fillRectangle (g, 0.0, 1.0, 0.0, 1.0);

	GraphicsScreen gs = static_cast <GraphicsScreen> (g);
	#if gdi
		HDC hdc = gs -> d_gdiGraphicsContext;
		if (! hdc)
			return;
		int w = 136, h = 136;
		if (gs -> d_winWindow) {
			RECT rc { };
			if (GetClientRect (gs -> d_winWindow, & rc) && rc.right > rc.left && rc.bottom > rc.top) {
				w = rc.right - rc.left;
				h = rc.bottom - rc.top;
			}
		}
		Gdiplus::Graphics dcplus (hdc);
		dcplus.SetSmoothingMode (Gdiplus::SmoothingModeHighQuality);
		dcplus.SetInterpolationMode (Gdiplus::InterpolationModeHighQualityBicubic);
		dcplus.SetPixelOffsetMode (Gdiplus::PixelOffsetModeHighQuality);

		COLORREF bgColor = GetSysColor (COLOR_BTNFACE);
		if (HBRUSH bgBrush = theWinGuiBackgroundBrush ()) {
			LOGBRUSH lb { };
			if (GetObject (bgBrush, sizeof (lb), & lb) == sizeof (lb))
				bgColor = lb.lbColor;
		}
		Gdiplus::SolidBrush outerBrush (Gdiplus::Color (255, GetRValue (bgColor), GetGValue (bgColor), GetBValue (bgColor)));
		dcplus.FillRectangle (& outerBrush, 0, 0, w, h);

		if (Gdiplus::Bitmap *bmp = getCachedWinLogoBitmap ()) {
			const int pad = 4;
			const int side = std::max (16, std::min (w, h) - pad * 2);
			const int dx = (w - side) / 2;
			const int dy = (h - side) / 2;
			dcplus.DrawImage (bmp, Gdiplus::Rect (dx, dy, side, side));
		}
	#elif quartz
		if (gs -> d_macGraphicsContext) {
			CGImageRef img = getCachedMacLogoImage ();
			if (img) {
				const double x1 = gs -> d_x1DC, x2 = gs -> d_x2DC;
				const double y1 = gs -> d_y1DC, y2 = gs -> d_y2DC;
				const double width = std::abs (x2 - x1);
				const double height = std::abs (y1 - y2);
				const double pad = 10.0;
				const double side = std::max (16.0, std::min (width, height) - pad * 2.0);
				const double dx = std::min (x1, x2) + (width - side) * 0.5;
				const double topY = std::min (y1, y2) + (height - side) * 0.5;
				CGContextSaveGState (gs -> d_macGraphicsContext);
				CGContextSetInterpolationQuality (gs -> d_macGraphicsContext, kCGInterpolationHigh);
				CGContextTranslateCTM (gs -> d_macGraphicsContext, 0.0, topY + side);
				CGContextScaleCTM (gs -> d_macGraphicsContext, 1.0, -1.0);
				CGContextDrawImage (gs -> d_macGraphicsContext, CGRectMake (dx, 0.0, side, side), img);
				CGContextRestoreGState (gs -> d_macGraphicsContext);
			}
		}
	#elif cairo
		if (gs -> d_cairoGraphicsContext) {
			cairo_surface_t *sfc = getCachedCairoLogoSurface ();
			if (sfc) {
				const double width = std::abs (static_cast <double> (gs -> d_x2DC - gs -> d_x1DC));
				const double height = std::abs (static_cast <double> (gs -> d_y1DC - gs -> d_y2DC));
				const double pad = 10.0;
				const double side = std::max (16.0, std::min (width, height) - pad * 2.0);
				const double sw = cairo_image_surface_get_width (sfc);
				const double sh = cairo_image_surface_get_height (sfc);
				if (sw > 0 && sh > 0) {
					cairo_save (gs -> d_cairoGraphicsContext);
					cairo_translate (gs -> d_cairoGraphicsContext, (width - side) * 0.5, (height - side) * 0.5);
					cairo_scale (gs -> d_cairoGraphicsContext, side / sw, side / sh);
					cairo_set_source_surface (gs -> d_cairoGraphicsContext, sfc, 0.0, 0.0);
					cairo_paint (gs -> d_cairoGraphicsContext);
					cairo_restore (gs -> d_cairoGraphicsContext);
				}
			}
		}
	#endif
}

static void refreshChineseAboutTexts () {
	if (! theChineseAbout.dialog)
		return;

	GuiShell_setTitle (theChineseAbout.dialog, trZh (U"About Praat Chinese Edition", U"关于 Praat 中文版"));

	if (theChineseAbout.titleLabel)
		GuiLabel_setText (theChineseAbout.titleLabel, trZh (U"Praat Chinese Edition", U"Praat 中文版"));

	if (theChineseAbout.versionLabel) {
		conststring32 edition = trZh (PRAAT_ZH_EDITION_EN, PRAAT_ZH_EDITION_ZH);
		GuiLabel_setText (theChineseAbout.versionLabel, Melder_cat (
			trZh (U"Release: ", U"发行版本："), PRAAT_ZH_VERSION_STR, U"  ·  ", edition));
	}

	if (theChineseAbout.upstreamLabel) {
		GuiLabel_setText (theChineseAbout.upstreamLabel, Melder_cat (
			trZh (U"Upstream Core: ", U"上游内核："), U"Praat ", Melder_appVersionSTR (),
			U" (", Melder_appYear (), U"-", Melder_appMonth (), U"-", Melder_appDay (), U")"));
	}

	if (theChineseAbout.platformLabel) {
		GuiLabel_setText (theChineseAbout.platformLabel, Melder_cat (
			trZh (U"Platform: ", U"运行平台："), getBuiltPlatformText ()));
	}

	if (theChineseAbout.maintainerLabel) {
		GuiLabel_setText (theChineseAbout.maintainerLabel, Melder_cat (
			trZh (U"Localization: ", U"汉化维护："), PRAAT_ZH_MAINTAINER_STR));
	}

	if (theChineseAbout.licenseLabel) {
		GuiLabel_setText (theChineseAbout.licenseLabel, trZh (
			U"License: GPLv3 · Based on Praat by Paul Boersma & David Weenink",
			U"开源许可：GPLv3 · 基于 Paul Boersma & David Weenink 原作"));
	}

	if (theChineseAbout.homepageLabel) {
		GuiLabel_setText (theChineseAbout.homepageLabel, Melder_cat (
			trZh (U"Homepage: ", U"项目主页："), PRAAT_ZH_HOMEPAGE_URL));
	}

	if (theChineseAbout.statusLabel) {
		const conststring32 latestTag32 = Melder_peek8to32_u (theChineseAbout.latestTag.c_str ());
		switch (theChineseAbout.updateStatus) {
			case UpdateCheckStatus::Idle:
				GuiLabel_setText (theChineseAbout.statusLabel, trZh (
					U"Click \"Check for Updates\" to check the latest release online.",
					U"点击“检查更新”可在线查询最新发行版本。"));
				break;
			case UpdateCheckStatus::Checking:
				GuiLabel_setText (theChineseAbout.statusLabel, trZh (
					U"Checking for updates...",
					U"正在连接服务器检查最新版本号，请稍候..."));
				break;
			case UpdateCheckStatus::UpdateAvailable:
				GuiLabel_setText (theChineseAbout.statusLabel, Melder_cat (
					trZh (U"New version available: ", U"发现新版本："), latestTag32,
					trZh (U" (Click \"Releases\" to download)", U"（请点击“下载页面”获取更新）")));
				break;
			case UpdateCheckStatus::UpToDate:
				GuiLabel_setText (theChineseAbout.statusLabel, Melder_cat (
					trZh (U"You are up to date (", U"当前已是最新版本（"),
					PRAAT_ZH_VERSION_STR,
					theChineseAbout.latestTag.empty () ? U"" : Melder_cat (
						trZh (U", latest online: ", U"，线上最新："), latestTag32),
					trZh (U").", U"）。")));
				break;
			case UpdateCheckStatus::Error:
				GuiLabel_setText (theChineseAbout.statusLabel, trZh (
					U"Update check failed. Check your network or visit Homepage.",
					U"检查更新失败，请检查网络连接或点击“项目主页”手动查看。"));
				break;
		}
	}

	if (theChineseAbout.checkUpdateButton) {
		GuiButton_setText (theChineseAbout.checkUpdateButton,
			theChineseAbout.updateStatus == UpdateCheckStatus::Checking
				? trZh (U"Checking...", U"检查中...")
				: trZh (U"Check for Updates", U"检查更新"));
		GuiThing_setSensitive (theChineseAbout.checkUpdateButton,
			theChineseAbout.updateStatus != UpdateCheckStatus::Checking);
	}
	if (theChineseAbout.homepageButton)
		GuiButton_setText (theChineseAbout.homepageButton, trZh (U"Homepage", U"项目主页"));
	if (theChineseAbout.downloadButton)
		GuiButton_setText (theChineseAbout.downloadButton,
			theChineseAbout.updateStatus == UpdateCheckStatus::UpdateAvailable
				? trZh (U"Download Update", U"下载新版")
				: trZh (U"Releases", U"下载页面"));
	if (theChineseAbout.closeButton)
		GuiButton_setText (theChineseAbout.closeButton, trZh (U"Close", U"关闭"));
}

static void applyCompletedUpdateJob (const std::shared_ptr <UpdateCheckJob>& job) {
	if (! job || theChineseAbout.activeJob != job)
		return;
	theChineseAbout.updateStatus = job -> status;
	theChineseAbout.latestTag = job -> latestTag;
	theChineseAbout.latestUrl = job -> downloadUrl;
	theChineseAbout.activeJob.reset ();
	refreshChineseAboutTexts ();
}

#if defined (_WIN32)
static VOID CALLBACK winUpdatePollTimerProc (HWND /* hwnd */, UINT /* msg */, UINT_PTR idEvent, DWORD /* dwTime */) {
	auto job = theChineseAbout.activeJob;
	if (! job) {
		if (theChineseAbout.pollTimerId) {
			KillTimer (nullptr, theChineseAbout.pollTimerId);
			theChineseAbout.pollTimerId = 0;
		}
		return;
	}
	if (job -> finished.load (std::memory_order_acquire)) {
		if (theChineseAbout.pollTimerId) {
			KillTimer (nullptr, theChineseAbout.pollTimerId);
			theChineseAbout.pollTimerId = 0;
		} else if (idEvent) {
			KillTimer (nullptr, idEvent);
		}
		applyCompletedUpdateJob (job);
	}
}
#elif defined (UNIX) && gtk
static gboolean gtkApplyUpdateJob (gpointer userData) {
	auto *holder = static_cast <std::shared_ptr <UpdateCheckJob> *> (userData);
	if (holder) {
		applyCompletedUpdateJob (*holder);
		delete holder;
	}
	return G_SOURCE_REMOVE;
}
#endif

static void gui_chineseAbout_cb_checkUpdate (Thing /* boss */, GuiButtonEvent /* event */) {
	if (theChineseAbout.activeJob)
		return;

	auto job = std::make_shared <UpdateCheckJob> ();
	theChineseAbout.activeJob = job;
	theChineseAbout.updateStatus = UpdateCheckStatus::Checking;
	refreshChineseAboutTexts ();

	const std::string localTag (Melder_peek32to8 (PRAAT_ZH_VERSION_STR));

	#if defined (_WIN32)
		if (theChineseAbout.pollTimerId) {
			KillTimer (nullptr, theChineseAbout.pollTimerId);
			theChineseAbout.pollTimerId = 0;
		}
		theChineseAbout.pollTimerId = SetTimer (nullptr, 0, 100, winUpdatePollTimerProc);
	#endif

	std::thread ([job, localTag] () {
		std::string remoteTag, remoteUrl;
		if (fetchLatestReleaseInfo (remoteTag, remoteUrl)) {
			job -> latestTag = remoteTag;
			job -> downloadUrl = remoteUrl;
			job -> status = (compareZhVersionTags (localTag, remoteTag) > 0)
				? UpdateCheckStatus::UpdateAvailable
				: UpdateCheckStatus::UpToDate;
		} else {
			job -> status = UpdateCheckStatus::Error;
		}
		job -> finished.store (true, std::memory_order_release);
		#if defined (macintosh)
			dispatch_async (dispatch_get_main_queue (), ^{
				applyCompletedUpdateJob (job);
			});
		#elif defined (UNIX) && gtk
			g_idle_add (gtkApplyUpdateJob, new std::shared_ptr <UpdateCheckJob> (job));
		#endif
	}).detach ();
}

static void gui_chineseAbout_cb_homepage (Thing /* boss */, GuiButtonEvent /* event */) {
	openUrlInBrowser (Melder_peek32to8 (PRAAT_ZH_HOMEPAGE_URL));
}

static void gui_chineseAbout_cb_download (Thing /* boss */, GuiButtonEvent /* event */) {
	if (! theChineseAbout.latestUrl.empty ())
		openUrlInBrowser (theChineseAbout.latestUrl);
	else
		openUrlInBrowser (Melder_peek32to8 (PRAAT_ZH_RELEASES_URL));
}

static void gui_chineseAbout_cb_close (Thing /* boss */, GuiButtonEvent /* event */) {
	if (theChineseAbout.dialog)
		GuiThing_hide (theChineseAbout.dialog);
}

static void gui_chineseAbout_cb_goAway (Thing /* boss */) {
	if (theChineseAbout.dialog)
		GuiThing_hide (theChineseAbout.dialog);
}

} // anonymous namespace

void praat_chineseAbout_languageChanged () {
	refreshChineseAboutTexts ();
}

void praat_showChineseAbout () {
	if (theCurrentPraatApplication -> batch)
		return;

	if (! theChineseAbout.dialog) {
		const int dialogWidth = 530;
		const int dialogHeight = 310;
		theChineseAbout.dialog = GuiDialog_create (theCurrentPraatApplication -> topShell,
			120, 120, dialogWidth, dialogHeight,
			trZh (U"About Praat Chinese Edition", U"关于 Praat 中文版"),
			gui_chineseAbout_cb_goAway, nullptr, GuiDialog_Modality::MODELESS);

		theChineseAbout.logoArea = GuiDrawingArea_createShown (theChineseAbout.dialog,
			20, 156, 20, 156,
			gui_chineseAbout_logoExpose, nullptr, nullptr, nullptr, nullptr, nullptr, 0);
		#if defined (_WIN32)
			if (theChineseAbout.logoArea && theChineseAbout.logoArea -> d_widget && theChineseAbout.logoArea -> d_widget -> window) {
				HWND logoHwnd = theChineseAbout.logoArea -> d_widget -> window;
				LONG_PTR style = GetWindowLongPtrW (logoHwnd, GWL_STYLE);
				LONG_PTR exStyle = GetWindowLongPtrW (logoHwnd, GWL_EXSTYLE);
				SetWindowLongPtrW (logoHwnd, GWL_STYLE, style & ~static_cast <LONG_PTR> (WS_BORDER));
				SetWindowLongPtrW (logoHwnd, GWL_EXSTYLE, exStyle & ~static_cast <LONG_PTR> (WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));
				SetWindowPos (logoHwnd, nullptr, 0, 0, 0, 0,
					SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
			}
		#endif
		theChineseAbout.logoGraphics = Graphics_create_xmdrawingarea (theChineseAbout.logoArea);

		const int infoLeft = 174;
		const int infoRight = dialogWidth - 20;
		theChineseAbout.titleLabel = GuiLabel_createShown (theChineseAbout.dialog,
			infoLeft, infoRight, 20, 48, U"", 0);
		theChineseAbout.versionLabel = GuiLabel_createShown (theChineseAbout.dialog,
			infoLeft, infoRight, 52, 72, U"", 0);
		theChineseAbout.upstreamLabel = GuiLabel_createShown (theChineseAbout.dialog,
			infoLeft, infoRight, 76, 96, U"", 0);
		theChineseAbout.platformLabel = GuiLabel_createShown (theChineseAbout.dialog,
			infoLeft, infoRight, 100, 120, U"", 0);
		theChineseAbout.maintainerLabel = GuiLabel_createShown (theChineseAbout.dialog,
			infoLeft, infoRight, 124, 144, U"", 0);
		theChineseAbout.licenseLabel = GuiLabel_createShown (theChineseAbout.dialog,
			20, infoRight, 170, 192, U"", 0);
		theChineseAbout.homepageLabel = GuiLabel_createShown (theChineseAbout.dialog,
			20, infoRight, 196, 218, U"", 0);
		theChineseAbout.statusLabel = GuiLabel_createShown (theChineseAbout.dialog,
			20, infoRight, 224, 248, U"", 0);

		#if defined (_WIN32)
			if (theChineseAbout.titleLabel && theChineseAbout.titleLabel -> d_widget && theChineseAbout.titleLabel -> d_widget -> window) {
				if (! theChineseAbout.titleFont) {
					theChineseAbout.titleFont = CreateFontW (
						-20, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
						DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
						CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei UI");
				}
				if (theChineseAbout.titleFont)
					SendMessageW (theChineseAbout.titleLabel -> d_widget -> window, WM_SETFONT,
						reinterpret_cast <WPARAM> (theChineseAbout.titleFont), TRUE);
			}
		#endif

		const int btnTop = dialogHeight - 44;
		const int btnBottom = btnTop + Gui_PUSHBUTTON_HEIGHT;
		theChineseAbout.checkUpdateButton = GuiButton_createShown (theChineseAbout.dialog,
			20, 146, btnTop, btnBottom, U"Check for Updates",
			gui_chineseAbout_cb_checkUpdate, nullptr, GuiButton_DEFAULT);
		theChineseAbout.homepageButton = GuiButton_createShown (theChineseAbout.dialog,
			158, 268, btnTop, btnBottom, U"Homepage",
			gui_chineseAbout_cb_homepage, nullptr, 0);
		theChineseAbout.downloadButton = GuiButton_createShown (theChineseAbout.dialog,
			280, 390, btnTop, btnBottom, U"Releases",
			gui_chineseAbout_cb_download, nullptr, 0);
		theChineseAbout.closeButton = GuiButton_createShown (theChineseAbout.dialog,
			414, dialogWidth - 20, btnTop, btnBottom, U"Close",
			gui_chineseAbout_cb_close, nullptr, GuiButton_CANCEL);

		refreshChineseAboutTexts ();
		GuiThing_show (theChineseAbout.dialog);
	} else {
		refreshChineseAboutTexts ();
		GuiThing_show (theChineseAbout.dialog);
	}
}

/* End of file praat_logo.cpp */
