#ifndef _melder_warning_h_
#define _melder_warning_h_
/* melder_warning.h
 *
 * Copyright (C) 1992-2018,2025 Paul Boersma
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

/*
	SYNOPSIS (2018-08-08)

	Melder_warning (args...);

		Gives a warning to stderr (batch) or to a "Warning" dialog.
		Use sparingly, because it interrupts the user's workflow.
*/

#include "melder_error.h"

namespace MelderWarning {
	extern int _depth;
	extern MelderString _buffer;
	using Proc = void (*) (conststring32 message);
	void _defaultProc (conststring32 message);
	extern Proc _p_currentProc;
}

template <typename... Arg>
void Melder_warning (const Arg... arg) {
	if (MelderWarning::_depth < 0)
		return;
	MelderString_copy (& MelderWarning::_buffer, Melder_translate (MelderArg { arg }. _arg)...);
	(*MelderWarning::_p_currentProc) (MelderWarning::_buffer.string);
}

void Melder_warningOff ();
void Melder_warningOn ();

class autoMelderWarningOff {
public:
	autoMelderWarningOff () { Melder_warningOff (); }
	~autoMelderWarningOff () { Melder_warningOn (); }
};

void Melder_setWarningProc (MelderWarning::Proc p_proc);

//#define Melder_warning_once(...)  \
//	do { static bool hasWarned = false; if (! hasWarned) { Melder_warning (__VA_ARGS__); hasWarned = true; } } while (0)

template <int invocationSite>
struct MelderWarningOnce {
	inline static std::once_flag flag;
};

template <int invocationSite, typename... Args>
inline void Melder_warning_once_template (Args&&... args)
{
	std::call_once (MelderWarningOnce <invocationSite> :: flag, [&] {
		Melder_warning (std::forward <Args> (args)...);
	});
}

#define Melder_warning_once(...)  Melder_warning_once_template <__COUNTER__> (__VA_ARGS__)

#endif // !_melder_warning_h_
