#ifndef _melder_i18n_h_
#define _melder_i18n_h_
/* melder_i18n.h
 *
 * High-performance flat UTF-32 hash table and zero-intrusion i18n interface
 * for Praat Simplified/Traditional Chinese localization.
 *
 * Design:
 *   - Contiguous char32_t arena + power-of-2 open-addressing hash table
 *   - ~15-30 ns lock-free, zero-allocation exact lookup
 *   - Context disambiguation (msgctxt)
 *   - Manual paragraph overlay indexed by (pageTitle, parIndex) with FNV-1a
 *     hash verification + (pageTitle, enHash) fallback for upstream resilience
 *   - Bilingual script alias resolution (English-first, Chinese fallback)
 */

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

extern int g_language_choice;   // 0 = English, 1 = Simplified Chinese, 2 = Traditional Chinese
extern int g_dpi_mode;

static inline uint64_t MelderI18n_fnv1a_u32 (std::u32string_view s, uint64_t seed = 14695981039346656037ULL) noexcept {
	for (char32_t ch : s) {
		seed ^= static_cast <uint64_t> (ch);
		seed *= 1099511628211ULL;
	}
	return seed ? seed : 1ULL;
}

static inline uint64_t MelderI18n_mixHash (uint64_t a, uint64_t b) noexcept {
	uint64_t h = a ^ (b + 0x9e3779b97f4a7c15ULL + (a << 6) + (a >> 2));
	return h ? h : 1ULL;
}

struct MelderI18nSlot {
	uint64_t hash = 0;   // 0 == empty
	uint32_t keyOff = 0, keyLen = 0;
	uint32_t ctxOff = 0, ctxLen = 0;
	uint32_t valOff = 0, valLen = 0;
};

class MelderI18nTable {
	std::vector <char32_t> arena_;
	std::vector <MelderI18nSlot> slots_;
	uint32_t mask_ = 0;
	size_t count_ = 0;

	uint32_t appendString (std::u32string_view s) {
		const uint32_t off = static_cast <uint32_t> (arena_.size());
		arena_.insert (arena_.end(), s.begin(), s.end());
		arena_.push_back (U'\0');
		return off;
	}

	void rehash (size_t newCap) {
		std::vector <MelderI18nSlot> next (newCap);
		const uint32_t newMask = static_cast <uint32_t> (newCap - 1);
		for (const auto& sl : slots_) {
			if (sl.hash == 0)
				continue;
			uint32_t idx = static_cast <uint32_t> (sl.hash) & newMask;
			while (next [idx].hash != 0)
				idx = (idx + 1) & newMask;
			next [idx] = sl;
		}
		slots_.swap (next);
		mask_ = newMask;
	}

	static inline uint64_t makeHash (std::u32string_view key, std::u32string_view ctx) noexcept {
		return MelderI18n_fnv1a_u32 (key, ctx.empty() ? 14695981039346656037ULL : MelderI18n_fnv1a_u32 (ctx));
	}

public:
	void clear () {
		arena_.clear();
		slots_.clear();
		mask_ = 0;
		count_ = 0;
	}

	void reserve (size_t expectedEntries, size_t expectedChars = 0) {
		size_t cap = 64;
		while (cap < expectedEntries * 2)
			cap <<= 1;
		if (slots_.size() < cap)
			rehash (cap);
		if (expectedChars > 0)
			arena_.reserve (expectedChars);
	}

	void insert (std::u32string_view key, std::u32string_view ctx, std::u32string_view val, bool overwrite = true) {
		if (key.empty())
			return;
		if (slots_.empty())
			rehash (64);
		else if ((count_ + 1) * 10 >= slots_.size() * 7)
			rehash (slots_.size() * 2);

		const uint64_t h = makeHash (key, ctx);
		uint32_t idx = static_cast <uint32_t> (h) & mask_;
		for (;;) {
			auto& sl = slots_ [idx];
			if (sl.hash == 0) {
				sl.hash = h;
				sl.keyOff = appendString (key);
				sl.keyLen = static_cast <uint32_t> (key.size());
				sl.ctxOff = ctx.empty() ? 0 : appendString (ctx);
				sl.ctxLen = static_cast <uint32_t> (ctx.size());
				sl.valOff = appendString (val);
				sl.valLen = static_cast <uint32_t> (val.size());
				++ count_;
				return;
			}
			if (sl.hash == h &&
				std::u32string_view (& arena_ [sl.keyOff], sl.keyLen) == key &&
				(sl.ctxLen == ctx.size() && (sl.ctxLen == 0 || std::u32string_view (& arena_ [sl.ctxOff], sl.ctxLen) == ctx)))
			{
				if (overwrite) {
					sl.valOff = appendString (val);
					sl.valLen = static_cast <uint32_t> (val.size());
				}
				return;
			}
			idx = (idx + 1) & mask_;
		}
	}

	const char32_t* probe (std::u32string_view key, std::u32string_view ctx = U"") const noexcept {
		if (slots_.empty() || key.empty())
			return nullptr;
		const uint64_t h = makeHash (key, ctx);
		uint32_t idx = static_cast <uint32_t> (h) & mask_;
		for (;;) {
			const auto& sl = slots_ [idx];
			if (sl.hash == 0)
				return nullptr;
			if (sl.hash == h &&
				sl.keyLen == key.size() &&
				sl.ctxLen == ctx.size() &&
				std::u32string_view (& arena_ [sl.keyOff], sl.keyLen) == key &&
				(sl.ctxLen == 0 || std::u32string_view (& arena_ [sl.ctxOff], sl.ctxLen) == ctx))
			{
				return & arena_ [sl.valOff];
			}
			idx = (idx + 1) & mask_;
		}
	}

	size_t size () const noexcept { return count_; }
};

struct MelderManualSlot {
	uint64_t hash = 0;      // 0 == empty
	uint64_t enHash = 0;    // FNV-1a of upstream English paragraph (0 == skip hash check)
	uint32_t titleOff = 0, titleLen = 0;
	int32_t  parIndex = 0;
	uint32_t valOff = 0, valLen = 0;
};

class MelderManualI18nTable {
	std::vector <char32_t> arena_;
	std::vector <MelderManualSlot> byPos_;
	std::vector <MelderManualSlot> byHash_;
	uint32_t posMask_ = 0;
	uint32_t hashMask_ = 0;
	size_t count_ = 0;

	uint32_t appendString (std::u32string_view s) {
		const uint32_t off = static_cast <uint32_t> (arena_.size());
		arena_.insert (arena_.end(), s.begin(), s.end());
		arena_.push_back (U'\0');
		return off;
	}

	static void rehashVec (std::vector <MelderManualSlot>& vec, uint32_t& mask, size_t newCap) {
		std::vector <MelderManualSlot> next (newCap);
		const uint32_t newMask = static_cast <uint32_t> (newCap - 1);
		for (const auto& sl : vec) {
			if (sl.hash == 0)
				continue;
			uint32_t idx = static_cast <uint32_t> (sl.hash) & newMask;
			while (next [idx].hash != 0)
				idx = (idx + 1) & newMask;
			next [idx] = sl;
		}
		vec.swap (next);
		mask = newMask;
	}

public:
	void clear () {
		arena_.clear();
		byPos_.clear();
		byHash_.clear();
		posMask_ = hashMask_ = 0;
		count_ = 0;
	}

	void reserve (size_t expectedEntries, size_t expectedChars = 0) {
		size_t cap = 64;
		while (cap < expectedEntries * 2)
			cap <<= 1;
		if (byPos_.size() < cap)
			rehashVec (byPos_, posMask_, cap);
		if (byHash_.size() < cap)
			rehashVec (byHash_, hashMask_, cap);
		if (expectedChars > 0)
			arena_.reserve (expectedChars);
	}

	void insert (std::u32string_view pageTitle, int32_t parIndex, uint64_t enHash, std::u32string_view zhText) {
		if (pageTitle.empty() || zhText.empty())
			return;
		if (byPos_.empty()) {
			rehashVec (byPos_, posMask_, 64);
			rehashVec (byHash_, hashMask_, 64);
		} else if ((count_ + 1) * 10 >= byPos_.size() * 7) {
			rehashVec (byPos_, posMask_, byPos_.size() * 2);
			rehashVec (byHash_, hashMask_, byHash_.size() * 2);
		}

		const uint32_t titleOff = appendString (pageTitle);
		const uint32_t titleLen = static_cast <uint32_t> (pageTitle.size());
		const uint32_t valOff = appendString (zhText);
		const uint32_t valLen = static_cast <uint32_t> (zhText.size());
		const uint64_t titleHash = MelderI18n_fnv1a_u32 (pageTitle);

		// Primary index: (pageTitle, parIndex)
		const uint64_t posH = MelderI18n_mixHash (titleHash, static_cast <uint64_t> (static_cast <uint32_t> (parIndex)));
		uint32_t idx = static_cast <uint32_t> (posH) & posMask_;
		for (;;) {
			auto& sl = byPos_ [idx];
			if (sl.hash == 0) {
				sl = { posH, enHash, titleOff, titleLen, parIndex, valOff, valLen };
				++ count_;
				break;
			}
			if (sl.hash == posH && sl.parIndex == parIndex && sl.titleLen == titleLen &&
				std::u32string_view (& arena_ [sl.titleOff], sl.titleLen) == pageTitle)
			{
				sl.enHash = enHash;
				sl.valOff = valOff;
				sl.valLen = valLen;
				break;
			}
			idx = (idx + 1) & posMask_;
		}

		// Secondary fallback index: (pageTitle, enHash)
		if (enHash != 0) {
			const uint64_t hashH = MelderI18n_mixHash (titleHash, enHash);
			uint32_t hidx = static_cast <uint32_t> (hashH) & hashMask_;
			for (;;) {
				auto& sl = byHash_ [hidx];
				if (sl.hash == 0) {
					sl = { hashH, enHash, titleOff, titleLen, parIndex, valOff, valLen };
					break;
				}
				if (sl.hash == hashH && sl.enHash == enHash && sl.titleLen == titleLen &&
					std::u32string_view (& arena_ [sl.titleOff], sl.titleLen) == pageTitle)
				{
					sl.valOff = valOff;
					sl.valLen = valLen;
					break;
				}
				hidx = (hidx + 1) & hashMask_;
			}
		}
	}

	const char32_t* probe (std::u32string_view pageTitle, int32_t parIndex, std::u32string_view enText) const noexcept {
		if (byPos_.empty() || pageTitle.empty())
			return nullptr;
		const uint64_t titleHash = MelderI18n_fnv1a_u32 (pageTitle);
		const uint64_t enHash = enText.empty() ? 0 : MelderI18n_fnv1a_u32 (enText);

		// 1. Primary O(1) lookup by (pageTitle, parIndex) + enHash verification
		const uint64_t posH = MelderI18n_mixHash (titleHash, static_cast <uint64_t> (static_cast <uint32_t> (parIndex)));
		uint32_t idx = static_cast <uint32_t> (posH) & posMask_;
		for (;;) {
			const auto& sl = byPos_ [idx];
			if (sl.hash == 0)
				break;
			if (sl.hash == posH && sl.parIndex == parIndex && sl.titleLen == pageTitle.size() &&
				std::u32string_view (& arena_ [sl.titleOff], sl.titleLen) == pageTitle)
			{
				if (sl.enHash == 0 || sl.enHash == enHash)
					return & arena_ [sl.valOff];
				break;
			}
			idx = (idx + 1) & posMask_;
		}

		// 2. Fallback O(1) lookup by (pageTitle, enHash) if upstream shifted paragraph index
		if (enHash != 0 && ! byHash_.empty()) {
			const uint64_t hashH = MelderI18n_mixHash (titleHash, enHash);
			uint32_t hidx = static_cast <uint32_t> (hashH) & hashMask_;
			for (;;) {
				const auto& sl = byHash_ [hidx];
				if (sl.hash == 0)
					break;
				if (sl.hash == hashH && sl.enHash == enHash && sl.titleLen == pageTitle.size() &&
					std::u32string_view (& arena_ [sl.titleOff], sl.titleLen) == pageTitle)
				{
					return & arena_ [sl.valOff];
				}
				hidx = (hidx + 1) & hashMask_;
			}
		}
		return nullptr;
	}

	size_t size () const noexcept { return count_; }
};

void Melder_i18n_init () noexcept;
const char32_t* Melder_trCtx (const char32_t* key, const char32_t* ctx = U"") noexcept;
inline const char32_t* Melder_tr (const char32_t* key) noexcept {
	return Melder_trCtx (key, U"");
}
const char32_t* Melder_trManualParagraph (const char32_t* pageTitle, int64_t parIndex, const char32_t* enText) noexcept;
const char32_t* Melder_i18n_resolveScriptAlias (const char32_t* text) noexcept;
bool Melder_i18n_matchesTranslation (const char32_t* userInput, const char32_t* canonicalEn) noexcept;

#endif
