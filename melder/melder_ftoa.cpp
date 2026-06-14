/* melder_ftoa.cpp
 *
 * Copyright (C) 1992-2008,2010-2012,2014-2026 Paul Boersma
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

#include "melder.h"

/********** NUMBER TO STRING CONVERSION **********/

inline constexpr int NUMBER_OF_BUFFERS = 32;   // = maximum number of arguments to a function call
inline constexpr int extraRoom = 2;
inline constexpr int MAXIMUM_NUMERIC_STRING_LENGTH = 1200;   // = sign + 326 + point + 60 + e + sign + 3 + null byte + ("·10^^^" - "e"), times 3, + i, + 2 extra

static char   buffers8  [NUMBER_OF_BUFFERS] [MAXIMUM_NUMERIC_STRING_LENGTH + 1];
static char32 buffers32 [NUMBER_OF_BUFFERS] [MAXIMUM_NUMERIC_STRING_LENGTH + 1];
static int ibuffer = 0;

#define CONVERT_BUFFER_TO_CHAR32 \
	char32 *q = buffers32 [ibuffer]; \
	while (*p != '\0') \
		* q ++ = (char32) (char8) * p ++; /* change signedness before extending (should be unnecessary, because all characters should be below 128) */ \
	*q = U'\0'; \
	return buffers32 [ibuffer];

const char * Melder8_integer (int64 value) {
	if (++ ibuffer == NUMBER_OF_BUFFERS)
		ibuffer = 0;
	if (sizeof (long_not_integer) == 8) {
		const int n = snprintf (buffers8 [ibuffer],MAXIMUM_NUMERIC_STRING_LENGTH+1, "%ld", (long_not_integer) value);   // cast to identical type, to make compiler happy
		Melder_assert (n > 0);
		Melder_assert (n <= MAXIMUM_NUMERIC_STRING_LENGTH);
	} else if (sizeof (long long) == 8) {
		/*
			There are buggy platforms (namely 32-bit Mingw on Windows XP) that support long long and %lld but that convert
			the argument to a 32-bit long.
			There are also buggy platforms (namely 32-bit gcc on Linux) that support long long and %I64d but that convert
			the argument to a 32-bit long.
		*/
		static const char *formatString = nullptr;
		if (! formatString) {
			char tryBuffer [MAXIMUM_NUMERIC_STRING_LENGTH + 1];
			formatString = "%lld";
			snprintf (tryBuffer,MAXIMUM_NUMERIC_STRING_LENGTH+1, formatString, 1000000000000LL);
			if (! strequ (tryBuffer, "1000000000000")) {
				formatString = "%I64d";
				snprintf (tryBuffer,MAXIMUM_NUMERIC_STRING_LENGTH+1, formatString, 1000000000000LL);
				if (! strequ (tryBuffer, "1000000000000"))
					Melder_crash (U"Found no way to print 64-bit integers on this machine.");
			}
		}
		const int n = snprintf (buffers8 [ibuffer],MAXIMUM_NUMERIC_STRING_LENGTH+1, formatString, value);
		Melder_assert (n > 0);
		Melder_assert (n <= MAXIMUM_NUMERIC_STRING_LENGTH);
	} else {
		Melder_crash (U"Neither long nor long long is 8 bytes on this machine.");
	}
	return buffers8 [ibuffer];
}
conststring32 Melder_integer (int64 value) {
	const char *p = Melder8_integer (value);
	CONVERT_BUFFER_TO_CHAR32
}

const char * Melder8_bigInteger (int64 value) {
	if (value == INT64_MIN)
		return "-9,223,372,036,854,775,808";
	if (++ ibuffer == NUMBER_OF_BUFFERS)
		ibuffer = 0;
	static_assert (MAXIMUM_NUMERIC_STRING_LENGTH > 200);   // so that we can be sure that even the longest int64 will fit
	char *pout = buffers8 [ibuffer];
	pout [0] = '\0';
	if (value < 0) {
		pout += snprintf (pout,100, "-");   // "100" for performance and simplicity; the onus is upon us to have this algorithm right
		value = - value;
	}
	char * const startOfDigits = pout;
	const int quintillions =  int (value / 1'000'000'000'000'000'000LL);
	value -=                quintillions * 1'000'000'000'000'000'000LL;
	const int quadrillions =  int (value / 1'000'000'000'000'000LL);
	value -=                quadrillions * 1'000'000'000'000'000LL;
	const int trillions =     int (value / 1'000'000'000'000LL);
	value -=                   trillions * 1'000'000'000'000LL;
	const int billions =      int (value / 1'000'000'000LL);   // cast to silence warning; the onus is upon us to have this algorithm right
	value -=                    billions * 1'000'000'000LL;
	const int millions =      int (value / 1'000'000LL);
	value -=                    millions * 1'000'000LL;
	const int thousands =     int (value / 1'000LL);
	value -=                   thousands * 1'000LL;
	const int units =         int (value);
	if (quintillions)
		pout += snprintf (pout,100, "%d,", quintillions);   // "100" for performance and simplicity; the onus is upon us to have this algorithm right
	if (quadrillions || pout > startOfDigits)
		pout += snprintf (pout,100, pout > startOfDigits ? "%03d," : "%d,", quadrillions);
	if (trillions || pout > startOfDigits)
		pout += snprintf (pout,100, pout > startOfDigits ? "%03d," : "%d,", trillions);
	if (billions || pout > startOfDigits)
		pout += snprintf (pout,100, pout > startOfDigits ? "%03d," : "%d,", billions);
	if (millions || pout > startOfDigits)
		pout += snprintf (pout,100, pout > startOfDigits ? "%03d," : "%d,", millions);
	if (thousands || pout > startOfDigits)
		pout += snprintf (pout,100, pout > startOfDigits ? "%03d," : "%d,", thousands);
	pout += snprintf (pout,100, pout > startOfDigits ? "%03d" : "%d", units);
	Melder_post (pout - buffers8 [ibuffer] <= 27);   // negative quintillions can go up to 27 characters
	return buffers8 [ibuffer];
}
conststring32 Melder_bigInteger (int64 value) {
	const char *p = Melder8_bigInteger (value);
	CONVERT_BUFFER_TO_CHAR32
}

const char * Melder8_boolean (bool value) {
	return value ? "yes" : "no";
}
conststring32 Melder_boolean (bool value) {
	return value ? U"yes" : U"no";
}

const char * Melder8_onoff (bool value) {
	return value ? "on" : "off";
}
conststring32 Melder_onoff (bool value) {
	return value ? U"on" : U"off";
}

const char * Melder8_kleenean (kleenean valueK) {
	return valueK ? "yes" : ! valueK ? "no": "unknown";
}
conststring32 Melder_kleenean (kleenean valueK) {
	return valueK ? U"yes" : ! valueK ? U"no": U"unknown";
}

inline static int common_snprintf_double (const mutablestring8 buffer, const int bufferSize, const double value) {
	Melder_pre (bufferSize > 0);   // make sure that it can be cast to size_t
	int numberOfPrintedCharacters = snprintf (buffer, size_t (bufferSize), "%.15g", value);   // guarded cast
	if (strtod (buffer, nullptr) != value) {
		numberOfPrintedCharacters = snprintf (buffer, size_t (bufferSize), "%.16g", value);   // guarded cast
		if (strtod (buffer, nullptr) != value)
			numberOfPrintedCharacters = snprintf (buffer, size_t (bufferSize), "%.17g", value);   // guarded cast
	}
	Melder_post (numberOfPrintedCharacters > 0);
	Melder_post (numberOfPrintedCharacters < bufferSize);   // truncation is a fatal error
	return numberOfPrintedCharacters;
}
inline static int common_snprintf_single (const mutablestring8 buffer, const int bufferSize, const double value) {
	Melder_pre (bufferSize > 0);   // make sure that it can be cast to size_t
	const int numberOfPrintedCharacters = snprintf (buffer, size_t (bufferSize), "%.9g", value);   // guarded cast
	Melder_post (numberOfPrintedCharacters > 0);
	Melder_post (numberOfPrintedCharacters < bufferSize);   // truncation is a fatal error
	return numberOfPrintedCharacters;
}
inline static int common_snprintf_half (const mutablestring8 buffer, const int bufferSize, const double value) {
	Melder_pre (bufferSize > 0);   // make sure that it can be cast to size_t
	const int numberOfPrintedCharacters = snprintf (buffer, size_t (bufferSize), "%.4g", value);   // guarded cast
	Melder_post (numberOfPrintedCharacters > 0);
	Melder_post (numberOfPrintedCharacters < bufferSize);   // truncation is a fatal error
	return numberOfPrintedCharacters;
}

/*@praat
	assert string$ (1000000000000) = "1000000000000"
	assert string$ (undefined) = "--undefined--"
@*/
const char * Melder8_double (double value) {
	if (++ ibuffer == NUMBER_OF_BUFFERS)   // should come before any return!
		ibuffer = 0;
	if (isundef (value))
		return "--undefined--";
	common_snprintf_double (buffers8 [ibuffer], MAXIMUM_NUMERIC_STRING_LENGTH+1, value);
	return buffers8 [ibuffer];
}
conststring32 Melder_double (double value) {
	const char *p = Melder8_double (value);
	CONVERT_BUFFER_TO_CHAR32
}

const char * Melder8_double_overtlyReal (double value) {
	if (++ ibuffer == NUMBER_OF_BUFFERS)   // should come before any return!
		ibuffer = 0;
	if (isundef (value))
		return "--undefined--";
	constexpr int roomForDotZero = 2;
	common_snprintf_double (buffers8 [ibuffer], MAXIMUM_NUMERIC_STRING_LENGTH+1 - roomForDotZero, value);
	if (! strchr (buffers8 [ibuffer], '.') && ! strchr (buffers8 [ibuffer], 'e') && ! strchr (buffers8 [ibuffer], 'E'))
		strcat (buffers8 [ibuffer], ".0");   // dot zero
	return buffers8 [ibuffer];
}
conststring32 Melder_double_overtlyReal (double value) {
	const char *p = Melder8_double_overtlyReal (value);
	CONVERT_BUFFER_TO_CHAR32
}

const char * Melder8_single (double value) {
	if (++ ibuffer == NUMBER_OF_BUFFERS)   // should come before any return!
		ibuffer = 0;
	if (isundef (value))
		return "--undefined--";
	common_snprintf_single (buffers8 [ibuffer], MAXIMUM_NUMERIC_STRING_LENGTH+1, value);
	return buffers8 [ibuffer];
}
conststring32 Melder_single (double value) {
	const char *p = Melder8_single (value);
	CONVERT_BUFFER_TO_CHAR32
}

const char * Melder8_half (double value) {
	if (++ ibuffer == NUMBER_OF_BUFFERS)   // should come before any return!
		ibuffer = 0;
	if (isundef (value))
		return "--undefined--";
	common_snprintf_half (buffers8 [ibuffer], MAXIMUM_NUMERIC_STRING_LENGTH+1, value);
	return buffers8 [ibuffer];
}
conststring32 Melder_half (double value) {
	const char *p = Melder8_half (value);
	CONVERT_BUFFER_TO_CHAR32
}

const char * Melder8_fixed (double value, integer precision) {
	if (++ ibuffer == NUMBER_OF_BUFFERS)   // should come before any return!
		ibuffer = 0;
	if (isundef (value))
		return "--undefined--";
	if (value == 0.0)
		return "0";
	if (precision > 60)
		precision = 60;
	const int minimumPrecision = - (int) floor (log10 (fabs (value)));
	const int numberOfPrintedCharacters = snprintf (buffers8 [ibuffer], MAXIMUM_NUMERIC_STRING_LENGTH+1, "%.*f",
			(int) (minimumPrecision > precision ? minimumPrecision : precision), value);
	Melder_post (numberOfPrintedCharacters > 0);
	Melder_post (numberOfPrintedCharacters < MAXIMUM_NUMERIC_STRING_LENGTH+1);
	return buffers8 [ibuffer];
}
conststring32 Melder_fixed (double value, integer precision) {
	const char *p = Melder8_fixed (value, precision);
	CONVERT_BUFFER_TO_CHAR32
}

const char * Melder8_fixedExponent (double value, integer exponent, integer precision) {
	if (++ ibuffer == NUMBER_OF_BUFFERS)   // should come before any return!
		ibuffer = 0;
	const double factor = pow (10.0, exponent);
	if (isundef (value))
		return "--undefined--";
	if (value == 0.0)
		return "0";
	if (precision > 60)
		precision = 60;
	value /= factor;
	const int minimumPrecision = - (int) floor (log10 (fabs (value)));
	const int numberOfPrintedCharacters = snprintf (buffers8 [ibuffer], MAXIMUM_NUMERIC_STRING_LENGTH+1, "%.*fE%d",
			(int) (minimumPrecision > precision ? minimumPrecision : precision), value, (int) exponent);
	Melder_post (numberOfPrintedCharacters > 0);
	Melder_post (numberOfPrintedCharacters < MAXIMUM_NUMERIC_STRING_LENGTH+1);
	return buffers8 [ibuffer];
}
conststring32 Melder_fixedExponent (double value, integer exponent, integer precision) {
	const char *p = Melder8_fixedExponent (value, exponent, precision);
	CONVERT_BUFFER_TO_CHAR32
}

static const char * common_Melder8_percent (bool graphical, double value, integer precision) {
	if (++ ibuffer == NUMBER_OF_BUFFERS)   // should come before any return!
		ibuffer = 0;
	if (isundef (value))
		return "--undefined--";
	if (value == 0.0)
		return "0";
	if (precision > 60)
		precision = 60;
	value *= 100.0;
	const int minimumPrecision = - (int) floor (log10 (fabs (value)));
	const int numberOfPrintedCharacters = snprintf (buffers8 [ibuffer], MAXIMUM_NUMERIC_STRING_LENGTH+1, graphical ? "%.*f\\%% " : "%.*f%%",
			(int) (minimumPrecision > precision ? minimumPrecision : precision), value);
	Melder_post (numberOfPrintedCharacters > 0);
	Melder_post (numberOfPrintedCharacters < MAXIMUM_NUMERIC_STRING_LENGTH+1);
	return buffers8 [ibuffer];
}
const char * Melder8_percent (double value, integer precision) {
	return common_Melder8_percent (false, value, precision);
}
conststring32 Melder_percent (double value, integer precision) {
	const char *p = Melder8_percent (value, precision);
	CONVERT_BUFFER_TO_CHAR32
}
const char * Melder8_graphicalPercent (double value, integer precision) {
	return common_Melder8_percent (true, value, precision);
}
conststring32 Melder_graphicalPercent (double value, integer precision) {
	const char *p = Melder8_graphicalPercent (value, precision);
	CONVERT_BUFFER_TO_CHAR32
}

const char * Melder8_hexadecimal (integer value, integer precision) {
	if (++ ibuffer == NUMBER_OF_BUFFERS)   // should come before any return!
		ibuffer = 0;
	if (value < 0)
		return "--undefined--";
	if (precision > 60)
		precision = 60;
	const integer integerValue = Melder_iround (value);
	const int numberOfPrintedCharacters = snprintf (buffers8 [ibuffer], MAXIMUM_NUMERIC_STRING_LENGTH+1, "%.*llX",
			(int) precision, (unsigned long long) integerValue);
	Melder_post (numberOfPrintedCharacters > 0);
	Melder_post (numberOfPrintedCharacters < MAXIMUM_NUMERIC_STRING_LENGTH+1);
	return buffers8 [ibuffer];
}
conststring32 Melder_hexadecimal (integer value, integer precision) {
	const char *p = Melder8_hexadecimal (value, precision);
	CONVERT_BUFFER_TO_CHAR32
}

const char * Melder8_dcomplex (dcomplex value) {
	if (++ ibuffer == NUMBER_OF_BUFFERS)   // should come before any return!
		ibuffer = 0;
	if (isundef (value.real()) || isundef (value.imag()))
		return "--undefined--";
	constexpr int roomForAdditionOrSubtractionSymbol = 1;
	constexpr int roomForISymbol = 1;
	constexpr int totalRoom = roomForAdditionOrSubtractionSymbol + roomForISymbol;
	static_assert (totalRoom <= extraRoom);
	const int numberOfPrintedCharacters = common_snprintf_double (buffers8 [ibuffer], MAXIMUM_NUMERIC_STRING_LENGTH/2+1 - roomForAdditionOrSubtractionSymbol, value.real());
	char *p = buffers8 [ibuffer] + numberOfPrintedCharacters;
	* (p ++) = ( value.imag() < 0.0 ? '-' : '+' );
	common_snprintf_double (p, MAXIMUM_NUMERIC_STRING_LENGTH/2+1 - roomForISymbol, fabs (value.imag()));
	strcat (buffers8 [ibuffer], "i");
	return buffers8 [ibuffer];
}
conststring32 Melder_dcomplex (dcomplex value) {
	const char *p = Melder8_dcomplex (value);
	CONVERT_BUFFER_TO_CHAR32
}

const char * Melder8_scomplex (dcomplex value) {
	if (++ ibuffer == NUMBER_OF_BUFFERS)   // should come before any return!
		ibuffer = 0;
	if (isundef (value.real()) || isundef (value.imag()))
		return "--undefined--";
	constexpr int roomForAdditionOrSubtractionSymbol = 1;
	constexpr int roomForISymbol = 1;
	constexpr int totalRoom = roomForAdditionOrSubtractionSymbol + roomForISymbol;
	static_assert (totalRoom <= extraRoom);
	const int numberOfPrintedCharacters = common_snprintf_single (buffers8 [ibuffer], MAXIMUM_NUMERIC_STRING_LENGTH/2+1 - roomForAdditionOrSubtractionSymbol, value.real());
	char *p = buffers8 [ibuffer] + numberOfPrintedCharacters;
	* (p ++) = ( value.imag() < 0.0 ? '-' : '+' );
	common_snprintf_single (p, MAXIMUM_NUMERIC_STRING_LENGTH/2+1 - roomForISymbol, fabs (value.imag()));
	strcat (buffers8 [ibuffer], "i");
	return buffers8 [ibuffer];
}
conststring32 Melder_scomplex (dcomplex value) {
	const char *p = Melder8_scomplex (value);
	CONVERT_BUFFER_TO_CHAR32
}

static conststring32 commmon_Melder_graphicalFloat (conststring32 number) {
	if (++ ibuffer == NUMBER_OF_BUFFERS)   // should come before any return!
		ibuffer = 0;
	if (! str32chr (number, 'e')) {
		str32cpy (buffers32 [ibuffer], number);
	} else {
		char32 *b = buffers32 [ibuffer];
		const char32 *n = & number [0];
		while (*n != U'e')
			*(b++) = *(n++);
		*b = U'\0';
		if (number [0] == U'1' && number [1] == U'e') {
			str32cpy (buffers32 [ibuffer], U"10^^");
			b = buffers32 [ibuffer] + 4;
		} else {
			str32cat (buffers32 [ibuffer], U"·10^^");
			b += 5;
		}
		Melder_assert (*n == U'e');
		if (*++n == U'+')
			n ++;   // ignore leading plus sign in exponent
		if (*n == U'-')
			*(b++) = *(n++);   // copy sign of negative exponent
		while (*n == U'0')
			n ++;   // ignore leading zeroes in exponent
		while (*n >= U'0' && *n <= U'9')
			*(b++) = *(n++);
		*(b++) = U'^';
		while (*n != U'\0')
			*(b++) = *(n++);
		*b = U'\0';
	}
	return buffers32 [ibuffer];
}
conststring32 Melder_graphicalHalf (double number) {
	return commmon_Melder_graphicalFloat (Melder_half (number));
}
conststring32 Melder_graphicalSingle (double number) {
	return commmon_Melder_graphicalFloat (Melder_single (number));
}
conststring32 Melder_graphicalDouble (double number) {
	return commmon_Melder_graphicalFloat (Melder_double (number));
}

const char * Melder8_naturalLogarithm (double lnNumber) {
	if (++ ibuffer == NUMBER_OF_BUFFERS)   // should come before any return!
		ibuffer = 0;
	//if (lnNumber == -INFINITY) return "0";   // this would have been nice, but cannot be relied upon
	if (isundef (lnNumber))
		return "--undefined--";
	const double log10Number = lnNumber * NUMlog10e;
	if (log10Number < -41.0) {
		/* mutable adjust */ integer ceiling = (integer) ceil (log10Number);
		const double remainder = log10Number - ceiling;
		double remainder10 = pow (10.0, remainder);
		while (remainder10 < 1.0) {
			remainder10 *= 10.0;
			ceiling --;
		}
		constexpr int roomForExponent = 20;
		const int numberOfPrintedCharacters = common_snprintf_double (buffers8 [ibuffer], MAXIMUM_NUMERIC_STRING_LENGTH+1 - roomForExponent, remainder10);
		snprintf (buffers8 [ibuffer] + numberOfPrintedCharacters, roomForExponent+1, "e-%td", ceiling);   // BUG: this just doesn't work on Windows
	} else {
		return Melder8_double (exp (lnNumber));
	}
	return buffers8 [ibuffer];
}
conststring32 Melder_naturalLogarithm (double lnNumber) {
	const char *p = Melder8_naturalLogarithm (lnNumber);
	CONVERT_BUFFER_TO_CHAR32
}

const char * Melder8_pointer (const void *pointer) {
	if (++ ibuffer == NUMBER_OF_BUFFERS)
		ibuffer = 0;
	snprintf (buffers8 [ibuffer],MAXIMUM_NUMERIC_STRING_LENGTH+1, "%p", pointer);
	return buffers8 [ibuffer];
}
conststring32 Melder_pointer (const void *pointer) {
	const char *p = Melder8_pointer (pointer);
	CONVERT_BUFFER_TO_CHAR32
}

conststring32 Melder_character (char32 kar) {
	if (++ ibuffer == NUMBER_OF_BUFFERS)
		ibuffer = 0;
	buffers32 [ibuffer] [0] = kar;
	buffers32 [ibuffer] [1] = U'\0';
	return buffers32 [ibuffer];
}

const char * Melder8_colour (MelderColour colour) {
	if (++ ibuffer == NUMBER_OF_BUFFERS)   // should come before any return!
		ibuffer = 0;
	if (isundef (colour.red) || isundef (colour.green) || isundef (colour.blue))
		return "{--undefined--,--undefined--,--undefined--}";
	char *p = & buffers8 [ibuffer] [0];
	constexpr int roomForBracesAndCommas = 4;
	constexpr int roomPerBraceOrComma = 2;   // rounded up; although this reserves 6 characters instead of the maximum of 2, this is possible because there won't be a "·10^^^" (saving 3*5) or "i"
	strcpy (p ++, "{");
	/* mutable reuse */ int numberOfPrintedCharacters = common_snprintf_double (p, MAXIMUM_NUMERIC_STRING_LENGTH/3+1 - roomPerBraceOrComma, colour.red);
	p += numberOfPrintedCharacters;
	strcpy (p ++, ",");
	numberOfPrintedCharacters = common_snprintf_double (p, MAXIMUM_NUMERIC_STRING_LENGTH/3+1 - roomPerBraceOrComma, colour.green);
	p += numberOfPrintedCharacters;
	strcpy (p ++, ",");
	numberOfPrintedCharacters = common_snprintf_double (p, MAXIMUM_NUMERIC_STRING_LENGTH/3+1 - roomPerBraceOrComma, colour.blue);
	p += numberOfPrintedCharacters;
	strcpy (p, "}");
	return buffers8 [ibuffer];
}
conststring32 Melder_colour (MelderColour colour) {
	const char *p = Melder8_colour (colour);
	CONVERT_BUFFER_TO_CHAR32
}

/********** TENSOR TO STRING CONVERSION **********/

#define NUMBER_OF_TENSOR_BUFFERS  3
static MelderString theTensorBuffers [NUMBER_OF_TENSOR_BUFFERS];
static int iTensorBuffer { 0 };

conststring32 Melder_VEC (constVECVU const& value, const bool horizontal) {
	if (++ iTensorBuffer == NUMBER_OF_TENSOR_BUFFERS)
		iTensorBuffer = 0;
	MelderString *string = & theTensorBuffers [iTensorBuffer];
	MelderString_empty (string);
	if (! NUMisEmpty (value))
		for (integer i = 1; i <= value.size; i ++)
			MelderString_append (string, value [i], horizontal ? U' ' : U'\n');
	return string -> string;
}
conststring32 Melder_fixed (constVECVU const& value, integer precision, const bool horizontal) {
	if (++ iTensorBuffer == NUMBER_OF_TENSOR_BUFFERS)
		iTensorBuffer = 0;
	MelderString *string = & theTensorBuffers [iTensorBuffer];
	MelderString_empty (string);
	if (! NUMisEmpty (value))
		for (integer i = 1; i <= value.size; i ++)
			MelderString_append (string, Melder_fixed (value [i], precision), horizontal ? U' ' : U'\n');
	return string -> string;
}

conststring32 Melder_MAT (constMATVU const& value) {
	if (++ iTensorBuffer == NUMBER_OF_TENSOR_BUFFERS)
		iTensorBuffer = 0;
	MelderString *string = & theTensorBuffers [iTensorBuffer];
	MelderString_empty (string);
	if (! NUMisEmpty (value)) {
		for (integer irow = 1; irow <= value.nrow; irow ++) {
			for (integer icol = 1; icol <= value.ncol; icol ++) {
				MelderString_append (string, value [irow] [icol]);
				if (icol < value.ncol)
					MelderString_appendCharacter (string, U' ');
			}
			if (irow < value.nrow)
				MelderString_appendCharacter (string, U'\n');
		}
	}
	return string -> string;
}
conststring32 Melder_fixed (constMATVU const& value, integer precision) {
	if (++ iTensorBuffer == NUMBER_OF_TENSOR_BUFFERS)
		iTensorBuffer = 0;
	MelderString *string = & theTensorBuffers [iTensorBuffer];
	MelderString_empty (string);
	if (! NUMisEmpty (value)) {
		for (integer irow = 1; irow <= value.nrow; irow ++) {
			for (integer icol = 1; icol <= value.ncol; icol ++) {
				MelderString_append (string, Melder_fixed (value [irow] [icol], precision));
				if (icol < value.ncol)
					MelderString_appendCharacter (string, U' ');
			}
			if (irow < value.nrow)
				MelderString_appendCharacter (string, U'\n');
		}
	}
	return string -> string;
}

conststring32 Melder_STRVEC (constSTRVEC const& value) {
	if (++ iTensorBuffer == NUMBER_OF_TENSOR_BUFFERS)
		iTensorBuffer = 0;
	MelderString *string = & theTensorBuffers [iTensorBuffer];
	MelderString_empty (string);
	if (! NUMisEmpty (value))
		for (integer i = 1; i <= value.size; i ++)
			MelderString_append (string, value [i], U'\n');
	return string -> string;
}

/********** STRING TO STRING CONVERSION **********/

static MelderString thePadBuffers [NUMBER_OF_BUFFERS];
static int iPadBuffer { 0 };

conststring32 Melder_padLeft (const conststring32 string, const integer width, const conststring32 pad) {
	if (++ iPadBuffer == NUMBER_OF_BUFFERS)
		iPadBuffer = 0;
	const integer length = Melder_length (string);
	Melder_assert (length >= 0);
	if (width <= length)
		return string;
	Melder_assert (width > 0);
	const integer tooShort = width - length;
			// guarded subtraction (length cannot be negative, so no overflow, and width is positive, so no underflow)
	Melder_assert (tooShort > 0);
	const integer padLength = Melder_length (pad);
	Melder_require (padLength > 0,
		U"Empty pad string.");
	MelderString_empty (& thePadBuffers [iPadBuffer]);
	/* mutable cycle */ integer ipad = 0;
	for (integer i = 0; i < tooShort; i ++) {
		Melder_assert (ipad >= 0);
		Melder_assert (ipad < padLength);
		MelderString_appendCharacter (& thePadBuffers [iPadBuffer], pad [ipad]);
		if (++ ipad >= padLength)
			ipad = 0;
	}
	MelderString_append (& thePadBuffers [iPadBuffer], string);
	return thePadBuffers [iPadBuffer].string;
}

conststring32 Melder_padRight (const conststring32 string, const integer width, const conststring32 pad) {
	if (++ iPadBuffer == NUMBER_OF_BUFFERS)
		iPadBuffer = 0;
	const integer length = Melder_length (string);
	Melder_assert (length >= 0);
	if (width <= length)
		return string;
	Melder_assert (width > 0);
	const integer tooShort = width - length;
			// guarded subtraction (length cannot be negative, so no overflow, and width is positive, so no underflow)
	Melder_assert (tooShort > 0);
	const integer padLength = Melder_length (pad);
	Melder_require (padLength > 0,
		U"Empty pad string.");
	MelderString_copy (& thePadBuffers [iPadBuffer], string);
	/* mutable cycle */ integer ipad = padLength - 1 - ((tooShort - 1) % padLength);
	for (integer i = 0; i < tooShort; i ++) {
		Melder_assert (ipad >= 0);
		Melder_assert (ipad < padLength);
		MelderString_appendCharacter (& thePadBuffers [iPadBuffer], pad [ipad]);
		if (++ ipad >= padLength)
			ipad = 0;
	}
	return thePadBuffers [iPadBuffer].string;
}

conststring32 Melder_truncateLeft (const conststring32 string, const integer width) {
	if (++ iPadBuffer == NUMBER_OF_BUFFERS)
		iPadBuffer = 0;
	Melder_require (width >= 0,
		U"Can never truncate a string down to ", width, U" characters.");
	const integer length = Melder_length (string);   // BUG: this can be slow if used in a loop, e.g. if 10'000'000 -> 1000
	Melder_assert (length >= 0);
	if (length <= width)
		return string;
	const integer tooLong = length - width;
			// guarded subtraction (length cannot be negative, so no underflow, and width cannot be negative, so no overflow)
	Melder_assert (tooLong > 0);
	MelderString_ncopy (& thePadBuffers [iPadBuffer], string + tooLong, width);
	return thePadBuffers [iPadBuffer].string;
}

conststring32 Melder_truncateRight (const conststring32 string, const integer width) {
	if (++ iPadBuffer == NUMBER_OF_BUFFERS)
		iPadBuffer = 0;
	Melder_require (width >= 0,
		U"Can never truncate a string down to ", width, U" characters.");
	const integer length = Melder_length (string);   // BUG: this can be slow if used in a loop, e.g. if 10'000'000 -> 1000
	Melder_assert (length >= 0);
	if (length <= width)
		return string;
	const integer tooLong = length - width;
			// guarded subtraction (length cannot be negative, so no underflow, and width is nonnegative, so no overflow)
	Melder_assert (tooLong > 0);
	MelderString_ncopy (& thePadBuffers [iPadBuffer], string, width);
	return thePadBuffers [iPadBuffer].string;
}

conststring32 Melder_padOrTruncateLeft (const conststring32 string, const integer width, const conststring32 pad) {
	if (++ iPadBuffer == NUMBER_OF_BUFFERS)
		iPadBuffer = 0;
	Melder_require (width >= 0,
		U"Can never truncate a string down to ", width, U" characters.");
	const integer length = Melder_length (string);
	Melder_assert (length >= 0);
	const integer tooLong = length - width;
			// guarded subtraction (length cannot be negative, so no underflow or INTEGER_MIN, and width cannot be negative, so no overflow)
	if (tooLong == 0)
		return string;
	if (tooLong < 0) {
		Melder_assert (tooLong > INTEGER_MIN);
		const integer tooShort = - tooLong;   // guarded integer sign flip
		Melder_assert (tooShort > 0);
		const integer padLength = Melder_length (pad);
		Melder_require (padLength > 0,
			U"Empty pad string.");
		MelderString_empty (& thePadBuffers [iPadBuffer]);
		/* mutable cycle */ integer ipad = 0;
		for (integer i = 0; i < tooShort; i ++) {
			Melder_assert (ipad >= 0);
			Melder_assert (ipad < padLength);
			MelderString_appendCharacter (& thePadBuffers [iPadBuffer], pad [ipad]);
			if (++ ipad >= padLength)
				ipad = 0;
		}
		MelderString_append (& thePadBuffers [iPadBuffer], string);
	} else {
		Melder_assert (tooLong <= length);
		MelderString_ncopy (& thePadBuffers [iPadBuffer], string + tooLong, width);
	}
	return thePadBuffers [iPadBuffer].string;
}

conststring32 Melder_padOrTruncateRight (conststring32 string, integer width, const conststring32 pad) {
	if (++ iPadBuffer == NUMBER_OF_BUFFERS)
		iPadBuffer = 0;
	Melder_require (width >= 0,
		U"Can never truncate a string down to ", width, U" characters.");
	const integer length = Melder_length (string);
	Melder_assert (length >= 0);
	const integer tooLong = length - width;
			// guarded subtraction (length cannot be negative, so no underflow or INTEGER_MIN, and width cannot be negative, so no overflow)
	if (tooLong == 0)
		return string;
	if (tooLong < 0) {
		Melder_assert (tooLong > INTEGER_MIN);
		const integer tooShort = - tooLong;   // guarded integer sign flip
		Melder_assert (tooShort > 0);
		const integer padLength = Melder_length (pad);
		Melder_require (padLength > 0,
			U"Empty pad string.");
		MelderString_copy (& thePadBuffers [iPadBuffer], string);
		/* mutable cycle */ integer ipad = padLength - 1 - ((tooShort - 1) % padLength);
		for (integer i = 0; i < tooShort; i ++) {
			Melder_assert (ipad >= 0);
			Melder_assert (ipad < padLength);
			MelderString_appendCharacter (& thePadBuffers [iPadBuffer], pad [ipad]);
			if (++ ipad >= padLength)
				ipad = 0;
		}
	} else {
		Melder_assert (tooLong <= length);
		MelderString_ncopy (& thePadBuffers [iPadBuffer], string, width);
	}
	return thePadBuffers [iPadBuffer].string;
}

/* End of file melder_ftoa.cpp */
