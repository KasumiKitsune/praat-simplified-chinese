/* PythonScriptEditor.cpp
 *
 * Python script editor window for Praat
 */

#include "PythonScriptEditor.h"
#include "praatP.h"
#include "EditorM.h"
#include "praat_python.h"
#include "praat_translate.h"

Thing_implement (PythonScriptEditor, TextEditor, 0);

static void menu_cb_run (PythonScriptEditor me, EDITOR_ARGS) {
	try {
		autostring32 text = GuiText_getString (my textWidget);
		if (! text || text [0] == U'\0')
			Melder_throw (U"No Python script text to run.");
		if (! MelderFile_isNull (& my file)) {
			praat_runPythonScriptFile (MelderFile_peekPath (& my file));
		} else {
			praat_runPythonScriptText (text.get());
		}
	} catch (MelderError) {
		Melder_flushError (U"Python script execution failed.");
	}
}

static void menu_cb_runSelection (PythonScriptEditor me, EDITOR_ARGS) {
	try {
		autostring32 selectedText = GuiText_getSelection (my textWidget);
		if (! selectedText || selectedText [0] == U'\0')
			Melder_throw (U"No Python script text selected.");
		praat_runPythonScriptText (selectedText.get());
	} catch (MelderError) {
		Melder_flushError (U"Python script selection execution failed.");
	}
}

static void menu_cb_insertTemplate (PythonScriptEditor me, EDITOR_ARGS) {
	autostring32 tmpl = praat_python_getScriptTemplate ();
	GuiText_setString (my textWidget, tmpl.get());
}

static void menu_cb_settings (PythonScriptEditor /* me */, EDITOR_ARGS) {
	EDITOR_FORM (U"Python settings", U"Python settings...")
		TEXTFIELD (pythonPath, U"Python executable path", praat_python_getExecutablePath (), 1)
	EDITOR_OK
	EDITOR_DO
		praat_python_setExecutablePath (pythonPath);
	EDITOR_END
}

static void menu_cb_tutorial (PythonScriptEditor /* me */, EDITOR_ARGS) {
	MelderInfo_open ();
	MelderInfo_write (praat_python_getTutorialDoc ().get());
	MelderInfo_close ();
}

static void menu_cb_apiRef (PythonScriptEditor /* me */, EDITOR_ARGS) {
	MelderInfo_open ();
	MelderInfo_write (praat_python_getApiDoc ().get());
	MelderInfo_close ();
}

static void menu_cb_exampleBatch (PythonScriptEditor me, EDITOR_ARGS) {
	GuiText_setString (my textWidget, praat_python_getExampleBatch ().get());
}

static void menu_cb_exampleTextGrid (PythonScriptEditor me, EDITOR_ARGS) {
	GuiText_setString (my textWidget, praat_python_getExampleTextGrid ().get());
}

static void menu_cb_exampleSound (PythonScriptEditor me, EDITOR_ARGS) {
	GuiText_setString (my textWidget, praat_python_getExampleSound ().get());
}

void structPythonScriptEditor :: v_nameChanged () {
	const bool dirtinessAlreadyShown = GuiWindow_setDirty (our windowForm, our dirty);
	static MelderString buffer;
	MelderString_copy (& buffer, MelderFile_isNull (& our file) ? U"untitled.py" : MelderFile_peekPath (& our file));
	if (our dirty && ! dirtinessAlreadyShown)
		MelderString_append (& buffer, U" (modified)");
	GuiShell_setTitle (our windowForm, buffer.string);
}

void structPythonScriptEditor :: v_createMenus () {
	PythonScriptEditor_Parent :: v_createMenus ();
	Editor_addMenu (this, U"Run", 0);
	Editor_addCommand (this, U"Run", U"Run", 'R', menu_cb_run);
	Editor_addCommand (this, U"Run", U"Run selection", 'T', menu_cb_runSelection);
	Editor_addCommand (this, U"Run", U"-- template --", 0, nullptr);
	Editor_addCommand (this, U"Run", U"Insert Praat Python template", 0, menu_cb_insertTemplate);
	Editor_addCommand (this, U"Run", U"-- settings --", 0, nullptr);
	Editor_addCommand (this, U"Run", U"Python settings...", 0, menu_cb_settings);
}

void structPythonScriptEditor :: v_createMenuItems_help (EditorMenu menu) {
	PythonScriptEditor_Parent :: v_createMenuItems_help (menu);
	EditorMenu_addCommand (menu, U"-- python help --", 0, nullptr);
	EditorMenu_addCommand (menu, U"Python scripting tutorial", 0, menu_cb_tutorial);
	EditorMenu_addCommand (menu, U"Praat Python API reference", 0, menu_cb_apiRef);
	EditorMenu_addCommand (menu, U"-- examples --", 0, nullptr);
	EditorMenu_addCommand (menu, U"Example: Batch Pitch & Formants", 0, menu_cb_exampleBatch);
	EditorMenu_addCommand (menu, U"Example: TextGrid Analysis", 0, menu_cb_exampleTextGrid);
	EditorMenu_addCommand (menu, U"Example: Generate Sound", 0, menu_cb_exampleSound);
}

void PythonScriptEditor_init (PythonScriptEditor me, conststring32 initialText) {
	TextEditor_init (me, initialText ? initialText : praat_python_getScriptTemplate().get());
}

autoPythonScriptEditor PythonScriptEditor_createFromText (conststring32 initialText) {
	try {
		autoPythonScriptEditor me = Thing_new (PythonScriptEditor);
		PythonScriptEditor_init (me.get(), initialText);
		return me;
	} catch (MelderError) {
		Melder_throw (U"Python Script window not created.");
	}
}
