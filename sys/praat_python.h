#ifndef _praat_python_h_
#define _praat_python_h_
/* praat_python.h
 *
 * Python scripting subsystem for Praat
 */

#include "melder.h"

void praat_python_initPreferences ();

conststring32 praat_python_getExecutablePath ();
void praat_python_setExecutablePath (conststring32 path);

// Run a Python script file (.py)
void praat_runPythonScriptFile (conststring32 filePath, conststring32 optionalWorkingDir = nullptr);

// Run Python code text directly
void praat_runPythonScriptText (conststring32 scriptText, conststring32 optionalScriptDirectory = nullptr);

// Documentation & templates
autostring32 praat_python_getScriptTemplate ();
autostring32 praat_python_getTutorialDoc ();
autostring32 praat_python_getApiDoc ();
autostring32 praat_python_getExampleBatch ();
autostring32 praat_python_getExampleTextGrid ();
autostring32 praat_python_getExampleSound ();

#endif
