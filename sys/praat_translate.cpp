/* praat_translate.cpp
 *
 * Phase 3 zero-intrusion localization engine:
 *   - High-performance flat UTF-32 hash tables (MelderI18nTable / MelderManualI18nTable)
 *   - Embedded UTF-8 TSV data (sys/praat_i18n_data.h) + optional runtime locales/*.tsv override
 *   - One-time startup Simplified -> Traditional Chinese table generation
 *   - Bilingual script alias resolution (English-first, Chinese fallback)
 *   - GuiI18nBinding live widget registry for instant hot-refresh on language switch
 */

#include "praat_translate.h"
#include "praat_i18n_data.h"
#include "Gui.h"
#include "melder.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#if defined (_WIN32)
	#include <windows.h>
#elif defined (__APPLE__)
	#include <mach-o/dyld.h>
#elif defined (__linux__)
	#include <unistd.h>
#endif

int g_language_choice = 1;   // 0 = English, 1 = Simplified Chinese, 2 = Traditional Chinese
#if ! defined (_WIN32)
int g_dpi_mode = 0;
#endif

namespace {

std::once_flag g_init_flag;
MelderI18nTable g_ui_zh_CN;
MelderI18nTable g_ui_zh_TW;
MelderI18nTable g_alias_zh_to_en;
MelderManualI18nTable g_manual_zh_CN;
MelderManualI18nTable g_manual_zh_TW;

std::mutex g_fallback_cache_mutex;
std::unordered_map <std::u32string, std::u32string> g_fallback_caches [3];   // 0=EN, 1=zh_CN, 2=zh_TW

std::mutex g_audit_mutex;
std::unordered_map <std::u32string, size_t> g_audit_counts;

// ==================== UTF-8 <-> UTF-32 & TSV Helpers ====================

char32_t convertS2TChar (char32_t ch) noexcept {
	const uint32_t cp = static_cast <uint32_t> (ch);
	size_t lo = 0, hi = kPraatS2TTable_size;
	while (lo < hi) {
		const size_t mid = lo + ((hi - lo) >> 1);
		const uint32_t cur = kPraatS2TTable [mid].simp;
		if (cur == cp)
			return static_cast <char32_t> (kPraatS2TTable [mid].trad);
		if (cur < cp)
			lo = mid + 1;
		else
			hi = mid;
	}
	return ch;
}

std::u32string convertS2TString (std::u32string_view s) {
	std::u32string out;
	out.reserve (s.size());
	for (char32_t ch : s)
		out.push_back (convertS2TChar (ch));
	return out;
}

std::u32string decodeUtf8AndUnescape (std::string_view sv) {
	std::u32string out;
	out.reserve (sv.size());
	const size_t n = sv.size();
	size_t i = 0;
	while (i < n) {
		const unsigned char c = static_cast <unsigned char> (sv [i]);
		if (c == '\\' && i + 1 < n) {
			const char nxt = sv [i + 1];
			if (nxt == 'n')      { out.push_back (U'\n'); i += 2; continue; }
			else if (nxt == 't') { out.push_back (U'\t'); i += 2; continue; }
			else if (nxt == 'r') { out.push_back (U'\r'); i += 2; continue; }
			else if (nxt == '\\'){ out.push_back (U'\\'); i += 2; continue; }
		}
		if (c < 0x80) {
			out.push_back (static_cast <char32_t> (c));
			++ i;
		} else if ((c & 0xE0) == 0xC0 && i + 1 < n) {
			char32_t cp = ((c & 0x1F) << 6) | (static_cast <unsigned char> (sv [i + 1]) & 0x3F);
			out.push_back (cp);
			i += 2;
		} else if ((c & 0xF0) == 0xE0 && i + 2 < n) {
			char32_t cp = ((c & 0x0F) << 12) |
				((static_cast <unsigned char> (sv [i + 1]) & 0x3F) << 6) |
				(static_cast <unsigned char> (sv [i + 2]) & 0x3F);
			out.push_back (cp);
			i += 3;
		} else if ((c & 0xF8) == 0xF0 && i + 3 < n) {
			char32_t cp = ((c & 0x07) << 18) |
				((static_cast <unsigned char> (sv [i + 1]) & 0x3F) << 12) |
				((static_cast <unsigned char> (sv [i + 2]) & 0x3F) << 6) |
				(static_cast <unsigned char> (sv [i + 3]) & 0x3F);
			out.push_back (cp);
			i += 4;
		} else {
			out.push_back (static_cast <char32_t> (c));
			++ i;
		}
	}
	return out;
}

uint64_t parseHex64 (std::string_view sv) noexcept {
	uint64_t v = 0;
	for (char c : sv) {
		v <<= 4;
		if (c >= '0' && c <= '9')      v |= static_cast <uint64_t> (c - '0');
		else if (c >= 'a' && c <= 'f') v |= static_cast <uint64_t> (c - 'a' + 10);
		else if (c >= 'A' && c <= 'F') v |= static_cast <uint64_t> (c - 'A' + 10);
	}
	return v;
}

void parseAliasesTsv (std::string_view payload) {
	size_t pos = 0;
	const size_t n = payload.size();
	while (pos < n) {
		size_t eol = payload.find ('\n', pos);
		if (eol == std::string_view::npos)
			eol = n;
		std::string_view line = payload.substr (pos, eol - pos);
		if (! line.empty() && line.back() == '\r')
			line.remove_suffix (1);
		pos = (eol < n) ? (eol + 1) : n;
		if (line.empty() || line [0] == '#')
			continue;
		const size_t tab = line.find ('\t');
		if (tab == std::string_view::npos)
			continue;
		std::u32string zh = decodeUtf8AndUnescape (line.substr (0, tab));
		std::u32string en = decodeUtf8AndUnescape (line.substr (tab + 1));
		if (! zh.empty() && ! en.empty()) {
			g_alias_zh_to_en.insert (zh, U"", en, true);
			std::u32string zhTw = convertS2TString (zh);
			if (zhTw != zh)
				g_alias_zh_to_en.insert (zhTw, U"", en, false);
		}
	}
}

void parseUiTsv (std::string_view payload, bool overwrite) {
	size_t pos = 0;
	const size_t n = payload.size();
	while (pos < n) {
		size_t eol = payload.find ('\n', pos);
		if (eol == std::string_view::npos)
			eol = n;
		std::string_view line = payload.substr (pos, eol - pos);
		if (! line.empty() && line.back() == '\r')
			line.remove_suffix (1);
		pos = (eol < n) ? (eol + 1) : n;
		if (line.empty() || line [0] == '#')
			continue;
		const size_t t1 = line.find ('\t');
		if (t1 == std::string_view::npos)
			continue;
		const size_t t2 = line.find ('\t', t1 + 1);
		if (t2 == std::string_view::npos)
			continue;
		std::u32string ctx = decodeUtf8AndUnescape (line.substr (0, t1));
		std::u32string en  = decodeUtf8AndUnescape (line.substr (t1 + 1, t2 - t1 - 1));
		std::u32string zh  = decodeUtf8AndUnescape (line.substr (t2 + 1));
		if (en.empty() || zh.empty())
			continue;
		g_ui_zh_CN.insert (en, ctx, zh, overwrite);
		std::u32string zhTw = convertS2TString (zh);
		g_ui_zh_TW.insert (en, ctx, zhTw, overwrite);
		// Populate reverse alias table without overriding curated disambiguations
		if (ctx.empty()) {
			g_alias_zh_to_en.insert (zh, U"", en, false);
			if (zhTw != zh)
				g_alias_zh_to_en.insert (zhTw, U"", en, false);
		}
	}
}

void parseManualsTsv (std::string_view payload) {
	size_t pos = 0;
	const size_t n = payload.size();
	while (pos < n) {
		size_t eol = payload.find ('\n', pos);
		if (eol == std::string_view::npos)
			eol = n;
		std::string_view line = payload.substr (pos, eol - pos);
		if (! line.empty() && line.back() == '\r')
			line.remove_suffix (1);
		pos = (eol < n) ? (eol + 1) : n;
		if (line.empty() || line [0] == '#')
			continue;
		const size_t t1 = line.find ('\t');
		if (t1 == std::string_view::npos) continue;
		const size_t t2 = line.find ('\t', t1 + 1);
		if (t2 == std::string_view::npos) continue;
		const size_t t3 = line.find ('\t', t2 + 1);
		if (t3 == std::string_view::npos) continue;

		std::u32string title = decodeUtf8AndUnescape (line.substr (0, t1));
		int32_t parIdx = 0;
		for (char c : line.substr (t1 + 1, t2 - t1 - 1)) {
			if (c >= '0' && c <= '9')
				parIdx = parIdx * 10 + (c - '0');
		}
		const uint64_t enHash = parseHex64 (line.substr (t2 + 1, t3 - t2 - 1));
		std::u32string zh = decodeUtf8AndUnescape (line.substr (t3 + 1));
		if (title.empty() || parIdx <= 0 || zh.empty())
			continue;
		g_manual_zh_CN.insert (title, parIdx, enHash, zh);
		g_manual_zh_TW.insert (title, parIdx, enHash, convertS2TString (zh));
	}
}

bool readFileUtf8 (const std::string& path, std::string& out) {
	#if defined (_WIN32)
		int wlen = MultiByteToWideChar (CP_UTF8, 0, path.c_str(), -1, nullptr, 0);
		if (wlen <= 0) return false;
		std::wstring wpath (static_cast <size_t> (wlen), L'\0');
		MultiByteToWideChar (CP_UTF8, 0, path.c_str(), -1, & wpath [0], wlen);
		HANDLE h = CreateFileW (wpath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (h == INVALID_HANDLE_VALUE) return false;
		LARGE_INTEGER sz;
		if (! GetFileSizeEx (h, & sz) || sz.QuadPart <= 0 || sz.QuadPart > 64LL * 1024 * 1024) {
			CloseHandle (h);
			return false;
		}
		out.resize (static_cast <size_t> (sz.QuadPart));
		DWORD got = 0;
		BOOL ok = ReadFile (h, & out [0], static_cast <DWORD> (sz.QuadPart), & got, nullptr);
		CloseHandle (h);
		if (! ok) { out.clear(); return false; }
		out.resize (got);
		return true;
	#else
		std::ifstream ifs (path, std::ios::binary);
		if (! ifs) return false;
		out.assign ((std::istreambuf_iterator <char> (ifs)), std::istreambuf_iterator <char> ());
		return ! out.empty();
	#endif
}

std::string getExecutableDirUtf8 () {
	#if defined (_WIN32)
		std::vector <wchar_t> buf (32768);
		DWORD len = GetModuleFileNameW (nullptr, buf.data(), static_cast <DWORD> (buf.size()));
		if (len == 0 || len >= buf.size()) return {};
		std::wstring wdir (buf.data(), len);
		size_t slash = wdir.find_last_of (L"\\/");
		if (slash != std::wstring::npos) wdir.resize (slash);
		int u8len = WideCharToMultiByte (CP_UTF8, 0, wdir.c_str(), -1, nullptr, 0, nullptr, nullptr);
		if (u8len <= 1) return {};
		std::string dir (static_cast <size_t> (u8len - 1), '\0');
		WideCharToMultiByte (CP_UTF8, 0, wdir.c_str(), -1, & dir [0], u8len, nullptr, nullptr);
		return dir;
	#elif defined (__APPLE__)
		uint32_t sz = 0;
		_NSGetExecutablePath (nullptr, & sz);
		if (sz == 0) return {};
		std::string p (sz, '\0');
		if (_NSGetExecutablePath (& p [0], & sz) != 0) return {};
		p.resize (std::strlen (p.c_str()));
		size_t slash = p.find_last_of ('/');
		if (slash != std::string::npos) p.resize (slash);
		return p;
	#elif defined (__linux__)
		char buf [4096];
		ssize_t len = readlink ("/proc/self/exe", buf, sizeof (buf) - 1);
		if (len <= 0) return {};
		buf [len] = '\0';
		std::string p (buf);
		size_t slash = p.find_last_of ('/');
		if (slash != std::string::npos) p.resize (slash);
		return p;
	#else
		return {};
	#endif
}

void loadExternalLocaleDir (const std::string& dir) {
	if (dir.empty())
		return;
	std::string content;
	if (readFileUtf8 (dir + "/locales/zh_CN_aliases.tsv", content))
		parseAliasesTsv (content);
	if (readFileUtf8 (dir + "/locales/zh_CN.tsv", content))
		parseUiTsv (content, true);
	if (readFileUtf8 (dir + "/locales/zh_CN_manuals.tsv", content))
		parseManualsTsv (content);
}

void initTablesOnce () noexcept {
	try {
		g_ui_zh_CN.reserve (6500, 180000);
		g_ui_zh_TW.reserve (6500, 180000);
		g_alias_zh_to_en.reserve (9000, 200000);
		g_manual_zh_CN.reserve (3400, 260000);
		g_manual_zh_TW.reserve (3400, 260000);

		// 1. Load curated aliases first so they win reverse-disambiguation priority
		parseAliasesTsv (std::string_view (reinterpret_cast <const char*> (kPraatI18nAliasesTsv), kPraatI18nAliasesTsv_size));
		// 2. Load embedded UI & Manual tables
		parseUiTsv (std::string_view (reinterpret_cast <const char*> (kPraatI18nUiTsv), kPraatI18nUiTsv_size), true);
		parseManualsTsv (std::string_view (reinterpret_cast <const char*> (kPraatI18nManualsTsv), kPraatI18nManualsTsv_size));
		// 3. Optional external runtime override (adjacent locales/ folder or macOS Resources/locales/)
		const std::string exeDir = getExecutableDirUtf8 ();
		if (! exeDir.empty()) {
			loadExternalLocaleDir (exeDir);
			#if defined (__APPLE__)
				loadExternalLocaleDir (exeDir + "/../Resources");
			#endif
		}
	} catch (...) {
	}
}

inline bool isPureAscii (std::u32string_view s) noexcept {
	for (char32_t c : s) {
		if (c >= 0x80)
			return false;
	}
	return true;
}

const char32_t* storeFallback (int cacheIdx, std::u32string_view key, std::u32string val) noexcept {
	try {
		std::lock_guard <std::mutex> lock (g_fallback_cache_mutex);
		auto& cache = g_fallback_caches [cacheIdx];
		auto [it, ok] = cache.emplace (std::u32string (key), std::move (val));
		return it -> second.c_str();
	} catch (...) {
		return nullptr;
	}
}

// ==================== Audit Miss Logger (opt-in via PRAAT_ZH_AUDIT=1) ====================

void appendAuditUtf8 (std::string& out, char32_t cp) {
	if (cp <= 0x7F) {
		out.push_back (static_cast <char> (cp));
	} else if (cp <= 0x7FF) {
		out.push_back (static_cast <char> (0xC0 | (cp >> 6)));
		out.push_back (static_cast <char> (0x80 | (cp & 0x3F)));
	} else if (cp <= 0xFFFF) {
		out.push_back (static_cast <char> (0xE0 | (cp >> 12)));
		out.push_back (static_cast <char> (0x80 | ((cp >> 6) & 0x3F)));
		out.push_back (static_cast <char> (0x80 | (cp & 0x3F)));
	} else {
		out.push_back (static_cast <char> (0xF0 | (cp >> 18)));
		out.push_back (static_cast <char> (0x80 | ((cp >> 12) & 0x3F)));
		out.push_back (static_cast <char> (0x80 | ((cp >> 6) & 0x3F)));
		out.push_back (static_cast <char> (0x80 | (cp & 0x3F)));
	}
}

void auditMiss (std::u32string_view key, const char* context) noexcept {
	const char* auditEnv = std::getenv ("PRAAT_ZH_AUDIT");
	if (! auditEnv || std::strcmp (auditEnv, "1") != 0 || key.empty())
		return;
	bool hasAlpha = false;
	for (char32_t c : key) {
		if ((c >= U'A' && c <= U'Z') || (c >= U'a' && c <= U'z')) {
			hasAlpha = true;
			break;
		}
	}
	if (! hasAlpha)
		return;
	try {
		std::lock_guard <std::mutex> lock (g_audit_mutex);
		g_audit_counts [std::u32string (key)] ++;
		const char* fileEnv = std::getenv ("PRAAT_ZH_AUDIT_FILE");
		const std::string path = (fileEnv && fileEnv [0]) ? fileEnv : "praat-zh-missing.tsv";
		std::ofstream out (path, std::ios::binary | std::ios::trunc);
		if (! out) return;
		out << "context\tsource\tcount\n";
		for (const auto& kv : g_audit_counts) {
			std::string escaped;
			for (char32_t c : kv.first) {
				if (c == U'\t') escaped += "\\t";
				else if (c == U'\r') escaped += "\\r";
				else if (c == U'\n') escaped += "\\n";
				else if (c == U'\\') escaped += "\\\\";
				else appendAuditUtf8 (escaped, c);
			}
			out << context << "\t" << escaped << "\t" << kv.second << "\n";
		}
	} catch (...) {
	}
}

// ==================== Forward Lookup Core ====================

const char32_t* probeForwardExact (const MelderI18nTable& tbl, std::u32string_view key, std::u32string_view ctx) noexcept {
	if (! ctx.empty()) {
		if (const char32_t* hit = tbl.probe (key, ctx))
			return hit;
	}
	return tbl.probe (key, U"");
}

bool trySynthesizeForward (const MelderI18nTable& tbl, std::u32string_view key, std::u32string_view ctx, bool toTrad, std::u32string& out) {
	// 1. Trailing colon ':' -> look up base + append fullwidth colon '：'
	if (key.size() > 1 && key.back() == U':') {
		std::u32string_view base = key.substr (0, key.size() - 1);
		if (const char32_t* hit = probeForwardExact (tbl, base, ctx)) {
			out.assign (hit);
			if (! out.empty()) {
				const char32_t last = out.back();
				if (last != U':' && last != U'\uff1a' && last != U'?' && last != U'\uff1f' &&
					last != U'.' && last != U'\u3002' && last != U'!' && last != U'\uff01')
				{
					out.push_back (U'\uff1a');
				}
			}
			return true;
		}
	}

	// 2. Leading indentation spaces or plugin superscript '+' ('\u207A')
	size_t prefixLen = 0;
	while (prefixLen < key.size() && (key [prefixLen] == U' ' || key [prefixLen] == U'\u207A'))
		++ prefixLen;
	if (prefixLen > 0 && prefixLen < key.size()) {
		std::u32string_view inner = key.substr (prefixLen);
		if (const char32_t* hit = probeForwardExact (tbl, inner, ctx)) {
			out.assign (key.substr (0, prefixLen));
			out.append (hit);
			return true;
		}
		std::u32string innerSynth;
		if (trySynthesizeForward (tbl, inner, ctx, toTrad, innerSynth)) {
			out.assign (key.substr (0, prefixLen));
			out.append (innerSynth);
			return true;
		}
	}

	// 3. Dynamic templates: "Undo <X>", "Redo <X>", "Quit <X>", "<Class> info", "class part \"<X>\":"
	if (key.size() > 5 && key.substr (0, 5) == U"Undo ") {
		std::u32string_view rest = key.substr (5);
		const char32_t* restTr = probeForwardExact (tbl, rest, U"");
		out = toTrad ? U"撤銷 " : U"撤销 ";
		out.append (restTr ? std::u32string_view (restTr) : rest);
		return true;
	}
	if (key.size() > 5 && key.substr (0, 5) == U"Redo ") {
		std::u32string_view rest = key.substr (5);
		const char32_t* restTr = probeForwardExact (tbl, rest, U"");
		out = U"重做 ";
		out.append (restTr ? std::u32string_view (restTr) : rest);
		return true;
	}
	if (key.size() > 5 && key.substr (0, 5) == U"Quit ") {
		out = U"退出 ";
		out.append (key.substr (5));
		return true;
	}
	if (key.size() > 5 && key.substr (key.size() - 5) == U" info") {
		std::u32string_view cls = key.substr (0, key.size() - 5);
		const char32_t* clsTr = probeForwardExact (tbl, cls, U"");
		out.assign (clsTr ? std::u32string_view (clsTr) : cls);
		out.append (toTrad ? U" 資訊" : U" 信息");
		return true;
	}
	if (key.size() > 14 && key.substr (0, 12) == U"class part \"" && key.substr (key.size() - 2) == U"\":") {
		out = toTrad ? U"類部分 \"" : U"类部分 \"";
		out.append (key.substr (12));
		return true;
	}
	return false;
}

// ==================== GuiI18nBinding Live Registry ====================

struct GuiI18nEntry {
	GuiI18nKind kind = GuiI18nKind::Label;
	std::u32string enKey;
	std::u32string ctx;
	std::vector <std::u32string> optionKeys;   // 0-based index -> position1Based - 1
};

std::unordered_map <void*, GuiI18nEntry> g_gui_bindings;
bool g_is_refreshing = false;

} // anonymous namespace

// ==================== Public C++ API ====================

void Melder_i18n_init () noexcept {
	std::call_once (g_init_flag, initTablesOnce);
}

const char32_t* Melder_trCtx (const char32_t* key, const char32_t* ctx) noexcept {
	if (! key || ! *key)
		return key;
	Melder_i18n_init ();

	const std::u32string_view kv (key);
	const std::u32string_view cv (ctx ? ctx : U"");

	// English mode: pure ASCII zero-overhead return; non-ASCII checks reverse alias table
	if (g_language_choice == 0) {
		if (isPureAscii (kv))
			return key;
		if (const char32_t* hit = g_alias_zh_to_en.probe (kv, U""))
			return hit;
		size_t prefixLen = 0;
		while (prefixLen < kv.size() && (kv [prefixLen] == U' ' || kv [prefixLen] == U'\u207A'))
			++ prefixLen;
		if (prefixLen > 0 && prefixLen < kv.size()) {
			if (const char32_t* hit = g_alias_zh_to_en.probe (kv.substr (prefixLen), U"")) {
				{
					std::lock_guard <std::mutex> lock (g_fallback_cache_mutex);
					auto it = g_fallback_caches [0].find (std::u32string (kv));
					if (it != g_fallback_caches [0].end())
						return it -> second.c_str();
				}
				std::u32string combined (kv.substr (0, prefixLen));
				combined.append (hit);
				if (const char32_t* cached = storeFallback (0, kv, std::move (combined)))
					return cached;
			}
		}
		return key;
	}

	const bool toTrad = (g_language_choice == 2);
	const MelderI18nTable& tbl = toTrad ? g_ui_zh_TW : g_ui_zh_CN;

	// Fast path (~15-30 ns, zero allocation, lock-free)
	if (const char32_t* hit = probeForwardExact (tbl, kv, cv))
		return hit;

	// Check fallback cache (for colon/prefix/template/S2T-only strings)
	const int cacheIdx = toTrad ? 2 : 1;
	{
		std::lock_guard <std::mutex> lock (g_fallback_cache_mutex);
		auto it = g_fallback_caches [cacheIdx].find (std::u32string (kv));
		if (it != g_fallback_caches [cacheIdx].end())
			return it -> second.c_str();
	}

	std::u32string synthesized;
	if (trySynthesizeForward (tbl, kv, cv, toTrad, synthesized)) {
		if (const char32_t* cached = storeFallback (cacheIdx, kv, std::move (synthesized)))
			return cached;
	}

	// Traditional Chinese passthrough for already-Chinese strings not in English key map
	if (toTrad && ! isPureAscii (kv)) {
		std::u32string trad = convertS2TString (kv);
		if (trad != kv) {
			if (const char32_t* cached = storeFallback (cacheIdx, kv, std::move (trad)))
				return cached;
		}
		return key;
	}

	auditMiss (kv, "ui");
	return key;
}

const char32_t* Melder_trManualParagraph (const char32_t* pageTitle, int64_t parIndex, const char32_t* enText) noexcept {
	if (! enText || ! *enText || g_language_choice == 0)
		return enText;
	if (! pageTitle || ! *pageTitle || parIndex <= 0)
		return enText;
	Melder_i18n_init ();

	const MelderManualI18nTable& tbl = (g_language_choice == 2) ? g_manual_zh_TW : g_manual_zh_CN;
	if (const char32_t* hit = tbl.probe (std::u32string_view (pageTitle), static_cast <int32_t> (parIndex), std::u32string_view (enText)))
		return hit;
	return enText;
}

const char32_t* Melder_i18n_resolveScriptAlias (const char32_t* text) noexcept {
	if (! text || ! *text)
		return nullptr;
	const std::u32string_view sv (text);
	if (isPureAscii (sv))
		return nullptr;
	Melder_i18n_init ();

	if (const char32_t* hit = g_alias_zh_to_en.probe (sv, U""))
		return hit;
	// Normalize fullwidth colon '：' -> ':' or ellipsis '…' -> '...'
	if (sv.back() == U'\uff1a' || sv.find (U'\u2026') != std::u32string_view::npos) {
		std::u32string norm;
		norm.reserve (sv.size() + 2);
		for (char32_t ch : sv) {
			if (ch == U'\uff1a') norm.push_back (U':');
			else if (ch == U'\u2026') norm.append (U"...");
			else norm.push_back (ch);
		}
		if (const char32_t* hit = g_alias_zh_to_en.probe (norm, U""))
			return hit;
	}
	return nullptr;
}

bool Melder_i18n_matchesTranslation (const char32_t* userInput, const char32_t* canonicalEn) noexcept {
	if (! userInput || ! canonicalEn || ! *userInput || ! *canonicalEn)
		return false;
	const std::u32string_view u (userInput);
	const std::u32string_view en (canonicalEn);
	if (u == en)
		return true;
	if (isPureAscii (u))
		return false;
	Melder_i18n_init ();
	if (const char32_t* zhCn = g_ui_zh_CN.probe (en, U"")) {
		if (u == std::u32string_view (zhCn))
			return true;
	}
	if (const char32_t* zhTw = g_ui_zh_TW.probe (en, U"")) {
		if (u == std::u32string_view (zhTw))
			return true;
	}
	return false;
}

const char32* praat_translate (const char32* text) {
	return Melder_trCtx (text, U"");
}

const char32* praat_translate_manual (const char32* text) {
	return Melder_trCtx (text, U"");
}

// ==================== GuiI18nBinding Implementation ====================

bool GuiI18n_isRefreshing () noexcept {
	return g_is_refreshing;
}

void GuiI18n_bind (void *guiThing, GuiI18nKind kind, const char32* enKey, const char32* ctx) noexcept {
	if (! guiThing || g_is_refreshing)
		return;
	if (! enKey || ! *enKey) {
		g_gui_bindings.erase (guiThing);
		return;
	}
	try {
		auto& entry = g_gui_bindings [guiThing];
		entry.kind = kind;
		entry.enKey.assign (enKey);
		entry.ctx.assign (ctx ? ctx : U"");
	} catch (...) {
	}
}

void GuiI18n_bindOption (void *guiOptionMenu, int position1Based, const char32* enKey) noexcept {
	if (! guiOptionMenu || g_is_refreshing || position1Based <= 0 || ! enKey)
		return;
	try {
		auto& entry = g_gui_bindings [guiOptionMenu];
		entry.kind = GuiI18nKind::OptionMenu;
		const size_t idx = static_cast <size_t> (position1Based - 1);
		if (entry.optionKeys.size() <= idx)
			entry.optionKeys.resize (idx + 1);
		entry.optionKeys [idx].assign (enKey);
	} catch (...) {
	}
}

void GuiI18n_unbind (void *guiThing) noexcept {
	if (! guiThing)
		return;
	g_gui_bindings.erase (guiThing);
}

void GuiI18n_refreshAll () noexcept {
	if (g_is_refreshing || g_gui_bindings.empty())
		return;
	g_is_refreshing = true;
	try {
		std::vector <void*> snapshot;
		snapshot.reserve (g_gui_bindings.size());
		for (const auto& kv : g_gui_bindings)
			snapshot.push_back (kv.first);

		for (void* ptr : snapshot) {
			auto it = g_gui_bindings.find (ptr);
			if (it == g_gui_bindings.end())
				continue;
			const GuiI18nEntry entry = it -> second;   // copy for safety during widget call
			switch (entry.kind) {
				case GuiI18nKind::Label:
					GuiLabel_setText (static_cast <GuiLabel> (ptr), entry.enKey.c_str());
					break;
				case GuiI18nKind::Button:
					GuiButton_setText (static_cast <GuiButton> (ptr), entry.enKey.c_str());
					break;
				case GuiI18nKind::CheckButton:
					GuiCheckButton_setText (static_cast <GuiCheckButton> (ptr), entry.enKey.c_str());
					break;
				case GuiI18nKind::RadioButton:
					GuiRadioButton_setText (static_cast <GuiRadioButton> (ptr), entry.enKey.c_str());
					break;
				case GuiI18nKind::Menu:
					GuiMenu_setTitle (static_cast <GuiMenu> (ptr), entry.enKey.c_str());
					break;
				case GuiI18nKind::MenuItem:
					GuiMenuItem_setText (static_cast <GuiMenuItem> (ptr), entry.enKey.c_str());
					break;
				case GuiI18nKind::Shell:
					GuiShell_setTitle (static_cast <GuiShell> (ptr), entry.enKey.c_str());
					break;
				case GuiI18nKind::OptionMenu: {
					GuiOptionMenu om = static_cast <GuiOptionMenu> (ptr);
					for (size_t i = 0; i < entry.optionKeys.size(); ++ i) {
						if (! entry.optionKeys [i].empty())
							GuiOptionMenu_setOptionText (om, static_cast <int> (i + 1), entry.optionKeys [i].c_str());
					}
					break;
				}
			}
		}
	} catch (...) {
	}
	g_is_refreshing = false;
}
