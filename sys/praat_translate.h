#ifndef _praat_translate_h_
#define _praat_translate_h_

#include "../melder/melder_i18n.h"

using char32 = char32_t;

const char32* praat_translate (const char32* text);
const char32* praat_translate_manual (const char32* text);

/*
 * Lightweight live GUI i18n binding registry.
 * Automatically unbound in structGuiThing::v9_destroy().
 */
enum class GuiI18nKind : uint8_t {
	Label = 0,
	Button,
	CheckButton,
	RadioButton,
	Menu,
	MenuItem,
	Shell,
	OptionMenu
};

void GuiI18n_bind (void *guiThing, GuiI18nKind kind, const char32* enKey, const char32* ctx = U"") noexcept;
void GuiI18n_bindOption (void *guiOptionMenu, int position1Based, const char32* enKey) noexcept;
void GuiI18n_unbind (void *guiThing) noexcept;
void GuiI18n_refreshAll () noexcept;
bool GuiI18n_isRefreshing () noexcept;

#endif
