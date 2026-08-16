#ifndef _PythonScriptEditor_h_
#define _PythonScriptEditor_h_
/* PythonScriptEditor.h
 *
 * Python script editor window for Praat
 */

#include "TextEditor.h"

Thing_define (PythonScriptEditor, TextEditor) {
	void v_nameChanged ()
		override;
	bool v_scriptable ()
		override { return false; }
	void v_createMenus ()
		override;
	void v_createMenuItems_help (EditorMenu menu)
		override;
	conststring32 v_extension () const
		override { return U".py"; }
};

void PythonScriptEditor_init (PythonScriptEditor me,
	conststring32 initialText
);
autoPythonScriptEditor PythonScriptEditor_createFromText (
	conststring32 initialText
);

#endif
