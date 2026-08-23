#ifndef _PreferencesDialog_h_
#define _PreferencesDialog_h_
/* PreferencesDialog.h
 *
 * Unified Preferences Dialog for Praat
 */

#include "Gui.h"
#include "praat.h"

void PRAAT_preferencesDialog (GuiWindow parentWindow);
extern "C" void PRAAT_preferences (UiForm, integer, Stackel, conststring32, Interpreter, conststring32, bool, void *, Editor);

#endif
