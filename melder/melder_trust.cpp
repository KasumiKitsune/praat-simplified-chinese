/* melder_trust.cpp
 *
 * Copyright (C) 2024,2026 Paul Boersma
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

#include "Interpreter.h"
#include "Script.h"
#include "Notebook.h"

void MelderTrust::_defaultProc (void *void_interpreter, conststring32 message) {
	Interpreter interpreter = (Interpreter) void_interpreter;
	if (interpreter) {
		Script script = interpreter -> scriptReference;
		Notebook notebook = interpreter -> notebookReference;
		if (Melder_appVersion() < 7000)
			return;   // no trust checking before Praat 7.0
		if (script && ! script -> trusted)
			Melder_throw (U"The following potentially dangerous action was requested by the script “", script -> string.get(),
				U"” but is not allowed without --FULL-TRUST:\n\n", message,
				U"\n\nUse --FULL-TRUST to prevent this message, but of course only if you indeed trust"
				" the intentions and skills of the authors of the script and of the scripts or notebooks it calls."
			);
		if (notebook && ! notebook -> trusted)
			Melder_throw (U"The following potentially dangerous action was requested by the notebook “", notebook -> string.get(),
				U"” but is not allowed without --FULL-TRUST:\n\n", message,
				U"\n\nUse --FULL-TRUST to prevent this message, but of course only if you indeed trust"
				" the intentions and skills of the authors of the notebook and of the scripts or notebooks it calls."
			);
		if (! script && ! notebook)
			Melder_throw (U"The following potentially dangerous action was requested"
				" but is not allowed without --FULL-TRUST:\n\n", message,
				U"\n\nUse --FULL-TRUST to prevent this message, but of course only if you indeed trust"
				" the intentions and skills of the authors of the script or notebook and of the scripts or notebooks it calls."
			);
	}
}

MelderTrust::Proc MelderTrust::_p_currentProc = & MelderTrust::_defaultProc;

MelderString MelderTrust::_buffer;

void Melder_setTrustProc (MelderTrust::Proc p_proc) {
	MelderTrust::_p_currentProc = ( p_proc ? p_proc : & MelderTrust::_defaultProc );
}

/* End of file melder_trust.cpp */
