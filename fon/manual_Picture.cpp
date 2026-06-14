/* manual_Picture.cpp
 *
 * Copyright (C) 1992-2023,2025 Paul Boersma
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

#include "ManPagesM.h"
#include "../kar/UnicodeData.h"

static void drawOneIpaSymbol (Graphics g, double x, double y, conststring32 symbol) {
	char32 buffer [30], *p = & buffer [0];
	const double fontSize = Graphics_inqFontSize (g);
	Graphics_rectangle (g, x - 0.5, x + 0.5, y - 0.5, y + 0.5);
	if (! symbol) return;
	Graphics_setTextAlignment (g, Graphics_CENTRE, Graphics_HALF);
	Graphics_setFont (g, kGraphics_font::TIMES);
	Graphics_setFontSize (g, fontSize * 3.0 / 2.0);
	Graphics_text (g, x, y + 0.25, symbol);
	while (*symbol) {
		if (*symbol == '\\') { *p++ = '\\'; *p++ = 'b'; *p++ = 's'; }   // visualize backslash (\bs)
		else { *p++ = '\\'; *p++ = *symbol; *p++ = ' '; }   // visualize special symbols (% ^ _ #)
		symbol ++;
	}
	*p = '\0';   // trailing null byte
	Graphics_setFont (g, kGraphics_font::HELVETICA);
	Graphics_setFontSize (g, fontSize * 5.0 / 6.0);
	Graphics_text (g, x, y - 0.25, buffer);
	Graphics_setFontSize (g, fontSize);
}

static void draw_IPA_consonant_chart (Graphics graphics) {
	Graphics_setWindow (graphics, -0.8, 20.0, 6.0, 19.5);
	Graphics_setFontStyle (graphics, Graphics_ITALIC);
	Graphics_setTextRotation (graphics, 60.0);
	Graphics_setTextAlignment (graphics, Graphics_LEFT, Graphics_HALF);
	Graphics_text (graphics, 4, 17.6, U"bilabial");
	Graphics_text (graphics, 5, 17.6, U"labiodental");
	Graphics_text (graphics, 6, 17.6, U"dental");
	Graphics_text (graphics, 7, 17.6, U"alveolar");
	Graphics_text (graphics, 8, 17.6, U"alv. lateral");
	Graphics_text (graphics, 9, 17.6, U"postalveolar");
	Graphics_text (graphics, 10, 17.6, U"retroflex");
	Graphics_text (graphics, 11, 17.6, U"alveolo-palatal");
	Graphics_text (graphics, 12, 17.6, U"palatal");
	Graphics_text (graphics, 13, 17.6, U"labial-palatal");
	Graphics_text (graphics, 14, 17.6, U"labial-velar");
	Graphics_text (graphics, 15, 17.6, U"velar");
	Graphics_text (graphics, 16, 17.6, U"uvular");
	Graphics_text (graphics, 17, 17.6, U"pharyngeal");
	Graphics_text (graphics, 18, 17.6, U"epiglottal");
	Graphics_text (graphics, 19, 17.6, U"glottal");
	Graphics_setTextRotation (graphics, 0.0);
	Graphics_setTextAlignment (graphics, Graphics_RIGHT, Graphics_HALF);
	Graphics_text (graphics, 3.3, 17, U"voiceless plosive");
	Graphics_text (graphics, 3.3, 16, U"voiced plosive");
	Graphics_text (graphics, 3.3, 15, U"nasal");
	Graphics_text (graphics, 3.3, 14, U"voiceless fricative");
	Graphics_text (graphics, 3.3, 13, U"voiced fricative");
	Graphics_text (graphics, 3.3, 12, U"approximant");
	Graphics_text (graphics, 3.3, 11, U"trill");
	Graphics_text (graphics, 3.3, 10, U"tap or flap");
	Graphics_text (graphics, 3.3, 9, U"lateral approx.");
	Graphics_text (graphics, 3.3, 8, U"implosive");
	Graphics_text (graphics, 3.3, 7, U"click");
	Graphics_setFontStyle (graphics, Graphics_NORMAL);
	{
		static const struct { double x, y; conststring32 string; } symbols [] = {
{ 4, 17, U"p" }, { 4, 16, U"b" }, { 4, 15, U"m" }, { 4, 14, U"\\ff" }, { 4, 13, U"\\bf" }, { 4, 11, U"\\bc" }, { 4, 8, U"\\b^" }, { 4, 7, U"\\O." },
{ 5, 15, U"\\mj" }, { 5, 14, U"f" }, { 5, 13, U"v" }, { 5, 12, U"\\vs" }, { 5, 10, U"\\V^" },
{ 6, 14, U"\\tf" }, { 6, 13, U"\\dh" }, { 6, 7, U"\\|1" },
{ 7, 17, U"t" }, { 7, 16, U"d" }, { 7, 15, U"n" }, { 7, 14, U"s" }, { 7, 13, U"z" }, { 7, 12, U"\\rt" }, { 7, 11, U"r" }, { 7, 10, U"\\fh" }, { 7, 9, U"l" }, { 7, 8, U"\\d^" },
{ 8, 17, U"t^l" }, { 8, 16, U"d^l" }, { 8, 14, U"\\l-" }, { 8, 13, U"\\lz" }, { 8, 12, U"l" }, { 8, 10, U"\\rl" }, { 8, 9, U"l" }, { 8, 7, U"\\|2" },
{ 9, 14, U"\\sh" }, { 9, 13, U"\\zh" }, { 9, 7, U"\\|-" },
{ 10, 17, U"\\t." }, { 10, 16, U"\\d." }, { 10, 15, U"\\n." }, { 10, 14, U"\\s." }, { 10, 13, U"\\z." }, { 10, 12, U"\\r." }, { 10, 10, U"\\f." }, { 10, 9, U"\\l." }, { 10, 7, U"!" },
{ 11, 14, U"\\cc" }, { 11, 13, U"\\zc" },
{ 12, 17, U"c" }, { 12, 16, U"\\j-" }, { 12, 15, U"\\nj" }, { 12, 14, U"\\c," }, { 12, 13, U"\\jc" }, { 12, 12, U"j" }, { 12, 9, U"\\yt" }, { 12, 8, U"\\j^" },
{ 13, 12, U"\\ht" },
{ 14, 14, U"\\wt" }, { 14, 12, U"w" },
{ 15, 17, U"k" }, { 15, 16, U"\\gs" }, { 15, 15, U"\\ng" }, { 15, 14, U"x" }, { 15, 13, U"\\gf" }, { 15, 12, U"\\ml" }, { 15, 9, U"\\lc" }, { 15, 8, U"\\g^" },
{ 16, 17, U"q" }, { 16, 16, U"\\gc" }, { 16, 15, U"\\nc" }, { 16, 14, U"\\cf" }, { 16, 13, U"\\ri" }, { 16, 11, U"\\rc" }, { 16, 8, U"\\G^" },
{ 17, 14, U"\\h-" }, { 17, 13, U"\\9e" },
{ 18, 17, U"\\?-" }, { 18, 14, U"\\hc" }, { 18, 13, U"\\9-" },
{ 19, 17, U"\\?g" }, { 19, 14, U"h" }, { 19, 13, U"\\h^" },
		{ 0, 0, nullptr } };
		for (int i = 0; symbols [i]. string != nullptr; i ++)
			drawOneIpaSymbol (graphics, symbols [i]. x, symbols [i]. y, symbols [i]. string);
	}
}

static void draw_IPA_vowel_chart (Graphics graphics) {
	Graphics_setWindow (graphics, -0.7, 19.0, -0.5, 7.5);
	Graphics_setFontStyle (graphics, Graphics_ITALIC);
	Graphics_setTextRotation (graphics, 60.0);
	Graphics_setTextAlignment (graphics, Graphics_LEFT, Graphics_HALF);
	Graphics_text (graphics, 4.5, 6.6, U"front");
	Graphics_text (graphics, 7, 6.6, U"central");
	Graphics_text (graphics, 9.5, 6.6, U"back");
	Graphics_setTextRotation (graphics, 0);
	Graphics_setTextAlignment (graphics, Graphics_RIGHT, Graphics_HALF);
	Graphics_text (graphics, 3.3, 6, U"close");
	Graphics_text (graphics, 3.3, 5, U"close centralized");
	Graphics_text (graphics, 3.3, 4, U"close-mid");
	Graphics_text (graphics, 3.3, 2, U"open-mid");
	Graphics_text (graphics, 3.3, 0, U"open");
	Graphics_setFontStyle (graphics, Graphics_NORMAL);
	{
		static const struct { double x, y; conststring32 string; } symbols [] = {
{ 4, 6, U"i" }, { 5, 6, U"y" }, { 6.5, 6, U"\\i-" }, { 7.5, 6, U"\\u-" }, { 9, 6, U"\\mt" }, { 10, 6, U"u" },
{ 4.2, 5, U"\\ic" }, { 5.2, 5, U"\\yc" }, { 9.8, 5, U"\\hs" },
{ 4, 4, U"e" }, { 5, 4, U"\\o/" }, { 6.5, 4, U"\\e-" }, { 7.5, 4, U"\\o-" }, { 9, 4, U"\\rh" }, { 10, 4, U"o" },
{ 7, 3, U"\\sw" }, { 7, 1, U"\\at" },
{ 4, 2, U"\\ef" }, { 5, 2, U"\\oe" }, { 6.5, 2, U"\\er" }, { 7.5, 2, U"\\kb" }, { 9, 2, U"\\vt" }, { 10, 2, U"\\ct" },
{ 4, 1, U"\\ae" },
{ 4, 0, U"a" }, { 5, 0, U"\\Oe" }, { 9, 0, U"\\as" }, { 10, 0, U"\\ab" },
		{ 0, 0, nullptr } };
		for (int i = 0; symbols [i]. string != nullptr; i ++)
			drawOneIpaSymbol (graphics, symbols [i]. x, symbols [i]. y, symbols [i]. string);
	}
}

void manual_Picture_init (ManPages me);
void manual_Picture_init (ManPages me) {

MAN_PAGES_BEGIN
R"~~~(
################################################################################
"Marks bottom every..."
© Paul Boersma 2023

参见 @@Marks left/right/top/bottom every...@。

Examples of use:
================
################################################################################
"Text bottom..."
© Paul Boersma 2023

参见 @@Text left/right/top/bottom...@。

Examples of use:
================
################################################################################
"Text top..."
© Paul Boersma 2023

参见 @@Text left/right/top/bottom...@。

Examples of use:
================

################################################################################
"World menu"
© Paul Boersma 2023

One of the menus of the @@Picture window@.

Purpose
=======
To draw text, lines, curves or shapes.

The coordinates for commands in the ##World menu# are to be specified as world coordinates,
i.e. in the same coordinates as the latest drawing (e.g. seconds horizontally
and pascals vertically) or as specified by the latest @@Axes...@ command.

################################################################################
"Draw rectangle..."
© Paul Boersma 2023

A command in the @@World menu@ of the @@Picture window@,
to draw a rectangle bounded by the %x and %y points given,
in the current colour and with the current line width and line type.

The coordinates %x and %y are to be specified as world coordinates,
i.e. in the same coordinates as the latest drawing (e.g. seconds horizontally
and pascals vertically) or as specified by the latest @@Axes...@ command.

################################################################################
"Draw line..."
© Paul Boersma 2023

A command in the @@World menu@ of the @@Picture window@,
to draw a line between the given %x and %y points,
in the current colour and with the current line width and line type.

The coordinates %x and %y are to be specified as world coordinates,
i.e. in the same coordinates as the latest drawing (e.g. seconds horizontally
and pascals vertically) or as specified by the latest @@Axes...@ command.

################################################################################
"Paint rectangle..."
© Paul Boersma 2023

A command in the @@World menu@ of the @@Picture window@,
to fill a rectangle bounded by the given %x and %y points,
with the given colour.

The coordinates %x and %y are to be specified as world coordinates,
i.e. in the same coordinates as the latest drawing (e.g. seconds horizontally
and pascals vertically) or as specified by the latest @@Axes...@ command.

################################################################################
"Draw rounded rectangle..."
© Paul Boersma 2023

A command in the @@World menu@ of the @@Picture window@,
to draw a rectangle bounded by the %x and %y points given,
and with the given radius for rounding the corners (in millimetres),
in the current colour and with the current line width and line type.

The coordinates %x and %y are to be specified as world coordinates,
i.e. in the same coordinates as the latest drawing (e.g. seconds horizontally
and pascals vertically) or as specified by the latest @@Axes...@ command.

################################################################################
"Paint rounded rectangle..."
© Paul Boersma 2023

A command in the @@World menu@ of the @@Picture window@,
to fill a rectangle bounded by the given %x and %y points,
and with the given radius for rounding the corners (in millimetres),
with the given colour.

The coordinates %x and %y are to be specified as world coordinates,
i.e. in the same coordinates as the latest drawing (e.g. seconds horizontally
and pascals vertically) or as specified by the latest @@Axes...@ command.

################################################################################
"Black"
© Paul Boersma 2023

A command in the @@Pen menu@ of the @@Picture window@,
to set the colour of subsequent drawings to black.

################################################################################
"Blue"
© Paul Boersma 2023

A command in the @@Pen menu@ of the @@Picture window@,
to set the colour of subsequent drawings to blue.

################################################################################
"Colour..."
© Paul Boersma 2023

A command in the @@Pen menu@ of the @@Picture window@,
to set the colour of subsequent drawings.

You can specify the colour as one of the colour names in the Pen menu,
or as a grey number betwene 0.0 (black) and 1.0 (white), or as a RGB sequence.

In an RGB sequence you specify
a red value between 0.0 (no red) and 1.0 (brightest red),
a green value between 0.0 (no green) and 1.0 (brightest green),
and a blue value between 0.0 (no blue) and 1.0 (brightest blue).
For instance, {0,1,0} is green, {1,1,1} is white, or { 1, 0.5, 0.5 } is pink.

################################################################################
"Cyan"
© Paul Boersma 2023

A command in the @@Pen menu@ of the @@Picture window@,
to set the colour of subsequent drawings to cyan.

################################################################################
"Pink"
© Paul Boersma 2023

A command in the @@Pen menu@ of the @@Picture window@,
to set the colour of subsequent drawings to pink.

################################################################################
"Green"
© Paul Boersma 2023

A command in the @@Pen menu@ of the @@Picture window@,
to set the colour of subsequent drawings to green.

################################################################################
"Grey"
© Paul Boersma 2023

A command in the @@Pen menu@ of the @@Picture window@,
to set the colour of subsequent drawings to grey.

################################################################################
"Lime"
© Paul Boersma 2023

A command in the @@Pen menu@ of the @@Picture window@,
to set the colour of subsequent drawings to lime.

################################################################################
"Magenta"
© Paul Boersma 2023

A command in the @@Pen menu@ of the @@Picture window@,
to set the colour of subsequent drawings to magenta.

################################################################################
"Maroon"
© Paul Boersma 2023

A command in the @@Pen menu@ of the @@Picture window@,
to set the colour of subsequent drawings to maroon.

################################################################################
"Navy"
© Paul Boersma 2023

A command in the @@Pen menu@ of the @@Picture window@,
to set the colour of subsequent drawings to navy blue.

################################################################################
"Olive"
© Paul Boersma 2023

A command in the @@Pen menu@ of the @@Picture window@,
to set the colour of subsequent drawings to olive green.

################################################################################
"Purple"
© Paul Boersma 2023

A command in the @@Pen menu@ of the @@Picture window@,
to set the colour of subsequent drawings to purple.

################################################################################
"Red"
© Paul Boersma 2023

A command in the @@Pen menu@ of the @@Picture window@,
to set the colour of subsequent drawings to red.

################################################################################
"Silver"
© Paul Boersma 2023

A command in the @@Pen menu@ of the @@Picture window@,
to set the colour of subsequent drawings to silver (light grey).

################################################################################
"Teal"
© Paul Boersma 2023

A command in the @@Pen menu@ of the @@Picture window@,
to set the colour of subsequent drawings to teal.

################################################################################
"Yellow"
© Paul Boersma 2023

A command in the @@Pen menu@ of the @@Picture window@,
to set the colour of subsequent drawings to yellow.

################################################################################
"Helvetica"
© Paul Boersma 2023

A command in the @@Font menu@ of the @@Picture window@,
to set the font of subsequent drawings to Helvetica,
or to s similar sans-serif font.

################################################################################
"Palatino"
© Paul Boersma 2023

A command in the @@Font menu@ of the @@Picture window@,
to set the font of subsequent drawings to Palatino,
or to a similar serif font.

################################################################################
"Times"
© Paul Boersma 2023

A command in the @@Font menu@ of the @@Picture window@,
to set the font of subsequent drawings to Times,
or to a similar serif font.

################################################################################
"Courier"
© Paul Boersma 2023

A command in the @@Font menu@ of the @@Picture window@,
to set the font of subsequent drawings to Courier,
or to a similar fixed-width font.

################################################################################
"10"
© Paul Boersma 2023

A command in the @@Font menu@ of the @@Picture window@,
to set the font size of subsequent drawings to 10 points.

################################################################################
"12"
© Paul Boersma 2023

A command in the @@Font menu@ of the @@Picture window@,
to set the font size of subsequent drawings to 12 points.

################################################################################
"14"
© Paul Boersma 2023

A command in the @@Font menu@ of the @@Picture window@,
to set the font size of subsequent drawings to 14 points.

################################################################################
"18"
© Paul Boersma 2023

A command in the @@Font menu@ of the @@Picture window@,
to set the font size of subsequent drawings to 18 points.

################################################################################
"24"
© Paul Boersma 2023

A command in the @@Font menu@ of the @@Picture window@,
to set the font size of subsequent drawings to 24 points.

################################################################################
"Font size..."
© Paul Boersma 2023

A command in the @@Font menu@ of the @@Picture window@,
to set the font size of subsequent drawings to a specific number of points.

################################################################################
"Special symbols"
© Paul Boersma 1997,1999,2003,2006,2007,2008,2025

When drawing text into the @@Picture window@ or into the @@TextGridEditor@,
you can use %%backslash trigraphs% to display various kinds of special symbols.
You can usually also type the characters themselves, if your computer has an input method for them.


欧洲语言符号
============

To get the symbol "\a^" (a-circumflex), you can type `\a^`, i.e.,
a sequence of backslash + a + circumflex (this is three characters in total, hence the term %trigraph).
In this way, you can get a hundred non-ASCII symbols that are used in the alphabets of many European languages:

- \a^ `\a^`   \e^ `\e^`   \i^ `\i^`   \o^ `\o^`   \u^ `\u^`      \A^ `\A^`   \E^ `\E^`   \I^ `\I^`   \O^ `\O^`   \U^ `\U^`
- \a` `\a```   \e` `\e```   \i` `\i```   \o` `\o```   \u` `\u```      \A` `\A```   \E` `\E```   \I` `\I```   \O` `\O```   \U` `\U```
- \a" `\a"`   \e" `\e"`   \i" `\i"`   \o" `\o"`   \u" `\u"`      \A" `\A"`   \E" `\E"`   \I" `\I"`   \O" `\O"`   \U" `\U"`      \y" `\y"`   \Y" `\Y"`
- \a' `\a'`   \e' `\e'`   \i' `\i'`   \o' `\o'`   \u' `\u'`      \A' `\A'`   \E' `\E'`   \I' `\I'`   \O' `\O'`   \U' `\U'`      \y' `\y'`   \Y' `\Y'`
- \c' `\c'`   \n' `\n'`   \s' `\s'`   \z' `\z'`      \C' `\C'`   \N' `\N'`   \S' `\S'`   \Z' `\Z'`
- \o: `\o:`   \u: `\u:`      \O: `\O:`   \U: `\U:`
- \a~ `\a~`   \n~ `\n~`   \o~ `\o~`      \A~ `\A~`   \N~ `\N~`   \O~ `\O~`
- \c< `\c<`   \d< `\d<`   \e< `\e<`   \g< `\g<`   \n< `\n<`   \r< `\r<`   \s< `\s<`   \t< `\t<`   \z< `\z<`
- \C< `\C<`   \D< `\D<`   \E< `\E<`   \G< `\G<`   \N< `\N<`   \R< `\R<`   \S< `\S<`   \T< `\T<`   \Z< `\Z<`
- \d- `\d-`   \D- `\D-`
- \ao `\ao`   \uo `\uo`      \Ao `\Ao`   \Uo `\Uo`
- \a; `\a;`   \e; `\e;`      \A; `\A;`   \E; `\E;`
- \c, `\c,`   \s, `\s,`   \t, `\t,`      \C, `\C,`   \S, `\S,`   \T, `\T,`
- \ae `\ae`   \o/ `\o/`      \Ae `\Ae`   \O/ `\O/`
- \l/ `\l/`   \ss `\ss`   \th `\th`   \z! `\z!`      \L/ `\L/`   \Th `\Th`   \Z! `\Z!`
- \!d `\!d`   \?d `\?d`
- \eu `\eu` %euro,   \Lp `\Lp` %%pound sterling%,   \Y= `\Y=` %yen,   \fd `\fd` %florin,   \c/ `\c/` %cent
- \SS `\SS` %section,   \|| `\||` %pilcrow (%paragraph)
- \co `\co` %copyright,   \re `\re` %registered,   \tm `\tm` %trademark
- \a_ `\a_` %%feminine ordinal%,   \o_ `\o_` %%masculine ordinal%
- \<< `\<<` %%left-pointing guillemet%,   \>> `\>>` %%right-pointing guillemet%


数学符号
========
- \.c `\.c` %%middle dot%,   \xx `\xx` %multiplication,   \:- `\:-` %division,   \/d `\/d` %%division slash%
- \dg `\dg` %degree,   \'p `\'p` %prime (%minute),   \"p `\"p` %%double prime% (%second)
- \-m `\-m` %minus,   \-- `\--` %%en-dash%,   \+- `\+-` %%plus-minus%
- \<_ `\<_` %%less than or equal to%,   \>_ `\>_` %%greater than or equal to%,   \=/ `\=/` %%not equal to%
- \no `\no` %%logical not%,   \an `\an` %%logical and%,   \or `\or` %%logical or%
- \At `\At` %%for all%,   \Er `\Er` %%there exists%,   \.3 `\.3` %therefore
- \oc `\oc` %%proportional to%,   \=3 `\=3` %%defined as% (or %%congruent modulo%),   \~~ `\~~` %%approximately equal to%
- \Vr `\Vr` %%square root%
- \<- `\<-`,   \-> `\->`,   \<> `\<>`
- \<= `\<=`,   \=> `\=>`,   \eq `\eq`
- \^| `\^|`,   \=~ `\=~` %%congruent to%,   \_| `\_|`
- \NE `\NE`,   \SE `\SE`,   \SW `\SW`,   \NW `\NW`
- \oo `\oo` %infinity,   \Tt `\Tt` %%up tack% (%%perpendicular to%)
- \O| `\O|` %%empty set%,   \ni `\ni` %intersection, \uu `\uu` %union,   \c= `\c=` %%subset of%,   \e= `\e=` %%element of%
- \dd `\dd` %%partial differential%
- \ox `\ox` %%circled times%,   \o+ `\o+` %%circled plus%
- \su `\su` %summation,   \in `\in` %integral


希腊字母
========

To get \ep\up\ro\et\ka\al, you type `\ep\up\ro\et\ka\al`.
|  %alpha  | \al `\al` | \Al `\Al`
 |  %beta  | \be `\be` | \Be `\Be`
|  %gamma  | \ga `\ga` | \Ga `\Ga`
|  %delta  | \de `\de` | \De `\De`
| %epsilon | \ep `\ep` | \Ep `\Ep`
 |  %zeta  | \ze `\ze` | \Ze `\Ze`
|   %eta   | \et `\et` | \Et `\Et`
|  %theta  | \te `\te` | \Te `\Te` | \t2 `\t2`
 |  %iota  | \io `\io` | \Io `\Io`
|  %kappa  | \ka `\ka` | \Ka `\Ka`
 | %lambda | \la `\la` | \La `\La`
 |   %mu   | \mu `\mu` | \Mu `\Mu`
 |   %nu   | \nu `\nu` | \Nu `\Nu`
 |   %xi   | \xi `\xi` | \Xi `\Xi`
| %omicron | \on `\on` | \On `\On`
 |   %pi   | \pi `\pi` | \Pi `\Pi` | \o2 `\o2`
|   %rho   | \ro `\ro` | \Ro `\Ro`
|  %sigma  | \si `\si` | \Si `\Si` | \s2 `\s2`
|   %tau   | \ta `\ta` | \Ta `\Ta`
| %upsilon | \up `\up` | \Up `\Up`
|   %phi   | \fi `\fi` | \Fi `\Fi` | \f2 `\f2`
|   %chi   | \ci `\ci` | \Ci `\Ci`
|   %psi   | \ps `\ps` | \Ps `\Ps`
|  %omega  | \om `\om` | \Om `\Om`


希伯来字母
==========

To get an alef, you type `\?+`.
在 Windows 上，您必须安装国际化支持才能获得正确的从左到右/从右到左顺序。
|  %alef  | \?+ `\?+`
 |  %bet  | \B+ `\B+`
 | %gimel | \G+ `\G+`
 | %dalet | \D+ `\D+`
|   %he   | \H+ `\H+`
 |  %vav  | \V+ `\V+`
 | %zayin | \Z+ `\Z+`
 |  %het  | \X+ `\X+`
 |  %tet  | \Y+ `\Y+`
 |  %yod  | \J+ `\J+`
 |  %kaf  | \K+ `\K+` | \K% `\K%`
 | %lamed | \L+ `\L+`
 |  %mem  | \M+ `\M+` | \M% `\M%`
 |  %nun  | \N+ `\N+` | \N% `\N%`
| %samekh | \S+ `\S+`
|  %ayin  | \9+ `\9+`
|   %pe   | \P+ `\P+` | \P% `\P%`
 | %tsadi | \C+ `\C+` | \C% `\C%`
 |  %qof  | \Q+ `\Q+`
|  %resh  | \R+ `\R+`
|  %shin  | \W+ `\W+`
 |  %tav  | \T+ `\T+`
 | %hiriq | \?+\hI `\?+\hI`
 | %segol | \?+\sE `\?+\sE` | %tsere | \?+\cE `\?+\cE`
| %qamats | \?+\qA `\?+\qA` | %patah | \?+\pA `\?+\pA`
 | %holam | \?+\hO `\?+\hO`
| %qubuts | \?+\qU `\?+\qU`
| %dagesh or %maliq | \dq `\dq`

Combinations:
| %%vav holam | \vO `\vO`
| %shuruq | \sU `\sU`


语音符号
========

See @@Phonetic symbols@


其他/杂项
========
, \bs `\bs` %backslash,   \bu `\bu` %bullet
, \cl `\cl` (%club),   \di `\di` (%diamond),   \he `\he` (%heart),   \sp `\sp` (%spade)
, \pf `\pf` %%pointing finger%,   \f5 `\f5` %%flower-five%


另请参阅
========

@@Text styles@

################################################################################
"Text styles"
© Paul Boersma 1996,2023,2025

在 @@Picture window|Picture 窗口@ 或编辑器中绘制文本时，您可以使用常规正体（Roman）以外的文本样式。

斜体、粗体、上标、下标
======================

使用以下符号，您可以引入以特殊样式绘制的文本片段：
, `%`：其后的一个字母将为斜体。
, `#`：其后的一个字母将为粗体。
, `#%`：其后的一个字母将为粗斜体。
, `^`：其后的一个字母将为上标：`%m%c^2` 得到 %m%c^2。
, `_`：其后的一个字母将为下标。例如：%F_0 输入为 `%F_0`。
, `%%`：其后的字母将为斜体，直到遇到下一个 `%`：
	, `Now %%you% try` 得到：Now %%you% try。
, 对于 `##`、`^^` 和 `__` 也是如此。
, `\s{...}`：小号字体：
	, `W\s{ARP}` 得到：W\s{ARP}

要绘制 \%、\#、\^ 或 \_ 符号本身，您应分别输入“`\% `”、“`\# `”、“`\^ `”或“`\_ `”：即一个反斜杠、该符号和一个空格。

另请参阅
========

@@Special symbols@

################################################################################
"Phonetic symbols"
© Paul Boersma 2004,2005,2007,2009,2025

To draw phonetic symbols in the @@Picture window@ or in the @TextGridEditor,
make sure that you have installed the Charis and/or Doulos SIL font, for instance from `sil.org` or from `praat.org`.
You can either type the symbols directly (if your computer has an input method for them), or use backslash trigraphs as described in:

- @@Phonetic symbols: consonants
- @@Phonetic symbols: vowels
- @@Phonetic symbols: diacritics

################################################################################
"Phonetic symbols: diacritics"
© Paul Boersma 2004,2005,2006,2007,2008,2009,2011,2013,2021,2025,2026

要在 @@Picture window|Picture 窗口@ 或 @TextGridEditor 中绘制语音附加符号（diacritics），请确保您已安装 Charis 和/或 Doulos SIL 字体，例如从 `sil.org` 或 `praat.org` 下载。
您可以直接输入这些符号本身（如果您的计算机装有相应的输入法），也可以使用下表中的反斜杠三合字符（trigraphs）。

字母下方的附加符号：
, n\|v `n\|v`（%%combining vertical line below%，下方竖线）：成音节辅音
, b\0v `b\0v`（%%combining ring below%，下方圆圈）：清音（例如：弱清塞音、清鼻音或清无擦通音/近似音）
, s\vv `s\vv`（%%combining caron below%，下方抑扬符）：浊音
, o\Tv `o\Tv`（%%combining down tack below%，下方低舌位符）：舌位低移元音；或将擦音变为无擦通音
, o\T^ `o\T^`（%%combining up tack below%，下方高舌位符）：舌位高移元音；或将无擦通音变为擦音
, o\T( `o\T(`（%%combining left tack below%，下方左对齐符）：舌根前移
, o\T) `o\T)`（%%combining right tack below%，下方右对齐符）：舌根后移
, e\-v `e\-v`（%%combining macron below%，下方长音符）：舌位后移
, s\.v `s\.v`（%%combining dot below%，下方点）：舌尖后齿龈音（非标准 IPA 卷舌音，在需要与舌尖硬腭音区分时使用；参见 @@Ladefoged & Maddieson (1996)@）
, o\+v `o\+v`（%%combining plus sign below%，下方加号）：舌位前移
, o\:v `o\:v`（%%combining diaeresis below%，下方分音符）：气声/弛声
, o\~v `o\~v`（%%combining tilde below%，下方波浪号）：嘎裂声/喉燥音
, d\Nv `d\Nv`（%%combining bridge below%，下方桥型符）：齿音（相对于齿龈音）
, d\Uv `d\Uv`（%%combining inverted bridge below%，下方倒桥符）：舌尖音
, d\Dv `d\Dv`（%%combining square below%，下方方框符）：舌叶音
, u\nv `u\nv`（%%combining inverted breve below%，下方倒圆音符）：非音节
, e\3v `e\3v`（%%combining right half ring below%，下方右半环）：微圆唇
, u\cv `u\cv`（%%combining left half ring below%，下方左半环）：微展唇/不圆唇
, d\mv `d\mv`（%%combining seagull below%，下方海鸥符）：舌唇音
, a\_ub `a\_ub`  下联结符（联诵，如果在您的转录中空格不代表停顿）

字母上方的附加符号：
, \gf\0^ `\gf\0^`（%%combining ring above%，上方圆圈）：清音
, \ef\'' `\ef\''`（%%combining double acute accent%，双锐音符）：超高调
, \ef\'^ `\ef\'^`（%%combining acute accent%，锐音符）：高调
, \ef\-^ `\ef\-^`（%%combining macron%，长音符）：中调（或左右）
, \ef\`^ `\ef\``^`（%%combining grave accent%，重音符）：低调
, \ef\`` `\ef\`````（%%combining double grave accent%，双重音符）：超低调
, \ef\-' `\ef\-'`（%%combining macron\--acute%，长音\--锐音）：高升调
, \ef\'- `\ef\'-`（%%combining acute\--macron%，锐音\--长音）：高降调
, \ef\`- `\ef\``-`（%%combining grave\--macron%，重音\--长音）：低升调
, \ef\-` `\ef\-```（%%combining macron\--grave%，长音\--重音）：低降调
, \ef\rf `\ef\rf`（%%combining grave\--acute\--grave%，重\--锐\--重）：升降调
, \ef\fr `\ef\fr`（%%combining acute\--grave\--acute%，锐\--重\--锐）：降升调
, \ef\~^ `\ef\~^`（%%combining tilde%，上方波浪号）：鼻音化
, \ef\v^ `\ef\v^`（%%combining caron%，抑扬符/楔形号）：升调
, \ef\^^ `\ef\^^`（%%combining circumflex accent%，扬抑符）：降调
, o\:^ `o\:^`（%%combining diaeresis%，分音符）：央化
, e\x^ `e\x^`（%%combining x above%，上方 x）：中央化
, \ef\N^ `\ef\N^`（%%combining breve%，短音符）：超短音
, k\lip `k\lip`,   t\lis `t\lis`（双倒短音符/合音）：协同发音，或单一音段
, m\LIb `m\LIb`,   k\LIx `k\LIx`（下方双短音符/合音）：协同发音，或单一音段

贯穿字母的附加符号（覆盖型）：
, d\// `d\//`  (%%combining long solidus overlay%): ?
, d\-/ `d\-/`  (%%combining long stroke overlay%): ?
, d\~/ `d\~/`  (%%combining tilde overlay%): 软腭化或咽音化，主要用于 “l”，
  虽然 l\~/ 可以更容易地输入为 \l~（否则，软腭化可以更清晰地表示为例如
  d\^g、d\^M 或 d\^G，咽音化表示为 d\^9）。


行内前缀：
, \'1 `\'1`  主重音
, \'2 `\'2`  次重音
, \|u `\|u`（%%modifier letter raised up arrow%，上标上箭头）：升阶
, \|d `\|d`（%%modifier letter raised down arrow%，上标下箭头）：降阶
, \NE `\NE`（%%north east arrow%，东北箭头）：全局上升
, \SE `\SE`（%%south east arrow%，东南箭头）：全局下降

行内后缀：
, \:f `\:f`  音标长音号
, \.f `\.f`  半长音号
, t\cn `t\cn`（%%combining left angle above%，左上角标）：无释放塞音（无声除阻）
, \er\hr `\er\hr`（%%combining rhotic hook%，卷舌钩）：卷舌化元音（儿化元音）
, p\ap `p\ap`  撇号（用于喷音）
, \-5 `\-5`  超高声调
, \-4 `\-4`  高声调
, \-3 `\-3`  中声调
, \-2 `\-2`  低声调
, \-1 `\-1`  超低声调
, （最后这五个可以组合，例如 \-5\-1 降调，\-3\-5 高升，
  \-2\-5\-2 升降，或 \-2\-1\-3 低降升）

其他行内符号：
, \|f `\|f`  音标竖线

上标：
, t\^h `t\^h`  送气
, b\^H `b\^H`  浊送气（气声）
, t\^j `t\^j`  腭化
, t\^g `t\^g`,   t\^M `t\^M`,   t\^G `t\^G`   软腭化
, k\^w `k\^w`  圆唇化
, t\^Y `t\^Y`  圆唇腭化
, a\^? `a\^?`  喉壁化/喉音化
, t\^9 `t\^9`  咽音化
, t\^l `t\^l`  边音释放/边音除阻
, t\^n `t\^n`,   p\^m `p\^m`,   k\^N `k\^N`   鼻音释放/鼻音除阻
, t\^s `t\^s`,   k\^x `k\^x`,   p\^f `p\^f`   塞擦音化
, t\^y `t\^y`（已弃用的美式标音法中的腭化）

双字符（合字）：
, \ts `\ts`  t\--s 合字
, \tS `\tS`  tesh 合字
, \dz `\dz`  d\--z 合字
, \dZ `\dZ`  dezh 合字

################################################################################
)~~~"
MAN_PAGES_END


MAN_BEGIN (U"Phonetic symbols: consonants", U"ppgb", 20090804)  // 2025
NORMAL (U"要在 @@Picture window|Picture 窗口@ 或 @TextGridEditor 中绘制辅音语音符号，请确保已安装 Charis 和/或 Doulos SIL 字体，例如从 `sil.org` 或 `praat.org` 下载。您可以直接输入这些符号（如果您的电脑装有相应的输入法），也可以使用下表中的反斜杠序列。")
PICTURE (6.0, 7.0, draw_IPA_consonant_chart)
NORMAL (U"其他辅音符号：")
LIST_ITEM (U"\\l~ \\bsl~（带有波浪号的 l）：软腭化 %l")
LIST_ITEM (U"\\hj \\bshj（带有钩头的 heng）：瑞典语圆唇齿龈后与软腭擦音")
ENTRY (U"如何记忆编码")
NORMAL (U"对于大多数编码，首字母指示与之最相似的英文字母。第二个字母可以是 %t（%turned，旋转）、%c（%capital 顶端或 %curled 卷曲）、%s（%script 手写体）、-（%barred 带杠）、%l（%with leg 带腿）、%i（%inverted 倒转）或 %j（%left tail 左尾）。一些语音符号与希腊字母相似，但有带有衬线的特殊语音（%f）版本（\\ff, \\bf, \\gf），或在其他方面略有不同（\\tf, \\cf）。\\ng（%engma）、\\dh（%eth）、\\sh（%esh）和 \\zh（%yogh）的编码是传统的替代拼写。卷舌音的第二个位置有一个点，因为另一种传统的拼写是在它们下方写一个点。\\fh 的编码是 %fishhook（鱼钩）的缩写。")
MAN_END

MAN_BEGIN (U"Phonetic symbols: vowels", U"ppgb", 20090804)  // 2025
NORMAL (U"要在 @@Picture window|Picture 窗口@ 或 @TextGridEditor 中绘制元音语音符号，请确保已安装 Charis 和/或 Doulos SIL 字体，例如从 `sil.org` 或 `praat.org` 下载。您可以直接输入这些符号（如果您的电脑装有相应的输入法），也可以使用下表中的反斜杠序列。")
PICTURE (6.0, 5.0, draw_IPA_vowel_chart)
NORMAL (U"其他元音符号包括：")
LIST_ITEM (U"\\sr \\bssr（带有右钩的 schwa）：卷舌化 schwa")
NORMAL (U"在 EPS 文件中不可用（即仅能通过复制粘贴或在 PDF 文件中发布）：")
LIST_ITEM (U"\\id \\bsid 成音节齿无擦通音")
LIST_ITEM (U"\\ir \\bsir 成音节卷舌（或舌尖后齿龈）无擦通音")
ENTRY (U"如何记忆编码")
NORMAL (U"对于大多数编码，首字母指示与之最相似的英文字母。第二个字母可以是 %t（%turned，旋转）、%c（%capital，顶端）、%s（%script，手写体）、%r（%reversed，反转）、-（%barred 带杠或 %retracted 后缩）或 /（%slashed 斜杠）。符号 \\ef 是希腊字母的语音版本。\\sw、\\rh、\\hs 和 \\kb 的编码是 %schwa、%%ram's horn%（公羊角）、%horseshoe（马蹄铁）和 %%kidney bean%（四季豆/腰豆）的缩写。")
MAN_END

MAN_BEGIN (U"Axes...", U"ppgb", 20140107)
INTRO (U"@@Picture window|Picture 窗口@ 的 #Margins（边距）和 #World（世界）菜单中的命令之一。")
ENTRY (U"目的")
NORMAL (U"查看和更改水平和垂直轴的当前世界坐标。")
ENTRY (U"用法")
NORMAL (U"坐标轴通常会通过动态菜单中的每个绘制操作进行更改，即通过标题为 ##Draw...# 和 ##Paint...# 的对象特定绘制命令（Picture 窗口中的绘制命令，如 ##Paint rectangle...#，不会改变坐标轴）。")
NORMAL (U"如果您的数据不在对象中，您可以使用 ##Axes...# 命令。")
ENTRY (U"示例")
NORMAL (U"以下脚本将绘制一个人的元音三角形：")
CODE (U"# Put F1 (between 300 and 800 Hz) along the horizontal axis,")
CODE (U"# and F2 (between 600 and 3600 Hz) along the vertical axis.")
CODE (U"\\#{Axes:} 300, 800, 600, 3600")
CODE (U"# Draw a rectangle inside the current viewport (selected area),")
CODE (U"# with text in the margins, and tick marks in steps of 100 Hz along the F1 axis,")
CODE (U"# and in steps of 200 Hz along the F2 axis.")
CODE (U"Draw inner box")
CODE (U"Text top: \"no\", \"Daniël's Dutch vowel triangle\"")
CODE (U"Text bottom: \"yes\", \"%F_1 (Hz)\"")
CODE (U"Text left: \"yes\", \"%F_2 (Hz)\"")
CODE (U"Marks bottom every: 1, 100, \"yes\", \"yes\", \"yes\", \"\"")
CODE (U"Marks left every: 1, 200, \"yes\", \"yes\", \"yes\", \"\"")
CODE (U"# Draw large phonetic symbols at the vowel points.")
CODE (U"Text special: 340, \"Centre\", 688, \"Half\", \"Times\", 24, \"0\", \"u\"")
CODE (U"Text special: 481, \"Centre\", 1195, \"Half\", \"Times\", 24, \"0\", \"ø\"")
CODE (U"# Etcetera")
NORMAL (U"此示例将在边距中绘制文本“Daniël's Dutch vowel triangle”、“%F_1 (Hz)”和“%F_2 (Hz)”，并在绘制区域内的适当位置绘制文本“u”和“ø”。")
MAN_END

MAN_BEGIN (U"Copy to clipboard", U"ppgb", 20120430)   /* Not Unix. */
INTRO (U"Windows 和 Macintosh 上 @@Picture window|Picture 窗口@ 的 File（文件）菜单中的命令之一。")
NORMAL (U"它将图画的选中部分复制到剪贴板中。然后您可以将其粘贴（#Paste）到任何支持显示图片的程序中。")
ENTRY (U"行为")
NORMAL (U"虽然所有的图像数据都会写入剪贴板，但只有与 Picture 窗口选中部分（%viewport，即视口）相对应的部分才可见。")
ENTRY (U"用法")
NORMAL (U"在 Windows 上，如果您有 PostScript 打印机，您可能希望改用 @@Save as EPS file...@（在转换为 PDF 或打印后检查效果）。如果图片太大，例如声谱图似乎无法装入剪贴板，您可以尝试改用 @@Save as Windows metafile...@。")
NORMAL (U"在 Mac 上，图片将以 PDF 格式放入剪贴板，能够处理 PDF 图片的程序（如 Microsoft^\\re Word\\tm 2008）可以识别它；图片的质量将与您使用 @@Save as PDF file...@ 并读取生成的 PDF 文件完全一样好。请注意，如果您将文档保存为 ##.doc# 文件，Word 将把 PDF 图片转换为中等质量的 300-dpi 位图；因此，您应该将文档保存为 ##.docx# 文件。如果您无法使用 ##.docx# 文件（例如，您的出版商尚不接受该文件类型），请考虑改用 @@Save as EPS file...@。在较旧的程序中（如 Microsoft^\\re Word\\tm 2004），您无法粘贴 Praat 的 PDF 图片；请改用 @@Save as EPS file...@。")
MAN_END

MAN_BEGIN (U"Draw inner box", U"ppgb", 19970330)
INTRO (U"@@Picture window|Picture 窗口@ 的 #Margins（边距）菜单中的命令之一。")
ENTRY (U"目的")
NORMAL (U"在绘制区域内绘制一个矩形，在四周留出边距（@margins）以绘制文本和标记。")
ENTRY (U"行为")
NORMAL (U"边距的宽度取决于当前的字体大小。")
MAN_END

MAN_BEGIN (U"Encapsulated PostScript", U"ppgb", 20110129)
INTRO (U"一种可以轻松导入到字处理器和绘图程序中的 PostScript 文件。在 Praat 中，您可以通过选择 @@Save as EPS file...@ 来创建封装的 PostScript (EPS) 文件。")
MAN_END

MAN_BEGIN (U"Erase all", U"ppgb", 19980825)
INTRO (U"@@Picture window|Picture 窗口@ 的 #Edit（编辑）菜单中的命令之一。")
NORMAL (U"它会擦除您的所有绘图。")
MAN_END

MAN_BEGIN (U"Font menu", U"ppgb", 20101125)
INTRO (U"@@Picture window|Picture 窗口@ 的菜单之一。")
NORMAL (U"它允许您选择后续绘制中使用的文本字体，以及大小（字符高度）。")
ENTRY (U"大小")
NORMAL (U"您可以直接从此菜单中选择 10、12、14、18 或 24 中的任何尺寸，或者在 ##Font size...# 表单中填入任何其他尺寸。")
NORMAL (U"边距的宽度取决于当前的字体大小，因此如果您想更改字体大小，请在开始绘图之前进行更改。")
ENTRY (U"字体")
NORMAL (U"通过这些命令，您可以设置后续文本将以何种字体绘制：Times、Helvetica、Palatino 或 Courier。")
NORMAL (U"您可以将 Symbol 和 IPA 字母表与普通罗马字母表混合使用，并使用反斜杠 + 双字符序列来获得 @@special symbols|特殊符号@（参见 @@phonetic symbols|语音符号@）。")
NORMAL (U"例如，您可以通过输入 \\bse\\\" 获得 \\e\"，或者通过输入 \\bsss 获得 \\ss；您可以通过输入 \\bsep 获得 \\ep，或者通过输入 \\bsct 获得 \\ct（即倒转的 c）。")
ENTRY (U"样式")
NORMAL (U"菜单中没有针对文本样式（粗体、斜体）的命令，但您仍然可以在 Picture 窗口中使用它们：参见 @@text styles|文本样式@。")
MAN_END

MAN_BEGIN (U"Logarithmic marks left/right/top/bottom...", U"ppgb", 19970330)
INTRO (U"@@Picture window|Picture 窗口@ 的 #Margins（边距）菜单中的四个命令。")
ENTRY (U"目的")
NORMAL (U"沿着对数轴在绘制区域周围的边距（@margins）中绘制每个数量级十倍频程（decade）指定数量的标记。")
ENTRY (U"设置")
TERM (U"%%Marks per decade")
DEFINITION (U"每个十倍频程将绘制的标记数。")
TERM (U"%%Write numbers")
DEFINITION (U"如果开启，将在边距中写入实数，表示您最近一次水平或垂直方向绘制的定义域或值域。")
TERM (U"%%Draw ticks")
DEFINITION (U"如果开启，将在边距中绘制短线段。")
TERM (U"%%Draw dotted lines")
DEFINITION (U"如果开启，虚线将贯穿您的绘图。")
ENTRY (U"行为")
LIST_ITEM (U"如果您的垂直对数轴运行范围为 10 到 100，并且 %%Marks per decade% 为 1，则仅在 10 和 100 处绘制标记；")
LIST_ITEM (U"如果 %%Marks per decade% 为 2，则在 10、30 和 100 处绘制标记；")
LIST_ITEM (U"如果为 3，则在 10、20、50 和 100 处绘制标记；")
LIST_ITEM (U"如果为 4，则在 10、20、30、50 和 100 处绘制标记；")
LIST_ITEM (U"如果为 5，则在 10、20、30、50、70 和 100 处绘制标记；")
LIST_ITEM (U"如果为 6，则在 10、15、20、30、50、70 和 100 处绘制标记；")
LIST_ITEM (U"如果为 7（最大值），则在 10、15、20、30、40、50、70 和 100 处绘制标记。")
MAN_END

MAN_BEGIN (U"Margins", U"ppgb", 19970405)
INTRO (U"@@Picture window|Picture 窗口@ 中大多数图画周围的空间。")
ENTRY (U"世界坐标")
NORMAL (U"通过 #Margins 菜单中的命令，您可以在最近绘制的图画周围的边距中绘制文本、刻度线、数字或矩形，或者在最近的图画中绘制贯穿的虚线或在内部绘制文本。")
NORMAL (U"您以世界坐标来指定这些事物的位置，即引用您最近一次绘制的自然坐标系的坐标。")
NORMAL (U"您可以在图画周围标记的数字也引用这些坐标。例如，在使用 ##Spectrum: Draw...# 绘制频谱后，您可以通过分别选择 ##One mark bottom...# 或 ##One mark left...# 并输入“2000”或“60”，从而在 2000 Hz 或 60 dB 处绘制一条虚线。")
ENTRY (U"用法")
NORMAL (U"边距命令适用于所有在其周围留出边距的图画，例如 ##Sound: Draw...#、##Spectrogram: Paint...#、##Polygon: Paint...# 等。然而，它们无法正常用于绘制声道形状的命令，例如 ##Art & Speaker: Draw...# 和 ##Artword & Speaker: Draw...#，因为这些只能正确绘制到正方形视口中。")
ENTRY (U"有限的有效性")
NORMAL (U"边距命令仅对您最近做出的绘制有效（除非您撤销 @Undo 该绘制）。")
ENTRY (U"边距大小")
NORMAL (U"边距的大小取决于字体大小，因此在进行绘制之前，请务必选择好您要使用的字体大小。您可以使用 @@Font menu|字体菜单@ 来设置字体大小。")
MAN_END

MAN_BEGIN (U"Marks left/right/top/bottom...", U"ppgb", 19970330)
INTRO (U"@@Picture window|Picture 窗口@ 的 #Margins（边距）菜单中的四个命令。")
ENTRY (U"目的")
NORMAL (U"在绘制区域周围的边距（@margins）中绘制任意数量的等间距标记。")
ENTRY (U"设置")
TERM (U"##Number of marks")
DEFINITION (U"将绘制的等间距标记的数量（2个或更多）；在定义域或值域的起点和终点总会留有标记。")
TERM (U"##Write numbers")
DEFINITION (U"如果开启，将在边距中写入实数，表示您最近一次水平或垂直方向绘制的定义域或值域。")
TERM (U"##Draw ticks")
DEFINITION (U"如果开启，将在边距中绘制短线段。")
TERM (U"##Draw dotted lines")
DEFINITION (U"如果开启，虚线将贯穿您的绘图。")
ENTRY (U"示例")
NORMAL (U"如果您绘制一个时间域在 0 到 1 秒之间、振幅范围在 -1 到 1 之间的 Sound，选择 ##Marks left...# 数量为 3 且 %%Draw dotted lines%（绘制虚线）开启，将在 -1、0 和 1 处得到水平标记和水平虚线；选择 ##Marks bottom...# 数量为 6 且 %%Draw dotted lines% 关闭，将在 0、0.2、0.4、0.6、0.8 和 1 处得到垂直标记。")
MAN_END

MAN_BEGIN (U"Marks left/right/top/bottom every...", U"ppgb", 19970330)
INTRO (U"@@Picture window|Picture 窗口@ 的 #Margins（边距）菜单中的四个命令。")
ENTRY (U"目的")
NORMAL (U"在绘制区域周围的边距（@margins）中绘制若干等间距的标记。")
ENTRY (U"设置")
TERM (U"##Units")
DEFINITION (U"数字所使用的单位，相对于标准单位。例如，如果您想使用毫秒而不是秒（秒始终是标准单位）来表示时间，则 #Units 应为 0.001。")
TERM (U"##Distance")
DEFINITION (U"将绘制的等间距标记之间的间距，以 #Units 为单位表示。例如，如果您希望每 20 毫秒有一个标记，且 %Units 是 0.001，则此参数应填 20。")
TERM (U"##Write numbers")
DEFINITION (U"如果开启，将在边距中写入实数，表示您最近一次水平或垂直方向绘制的定义域或值域。")
TERM (U"##Draw ticks")
DEFINITION (U"如果开启，将在边距中绘制短线段。")
TERM (U"##Draw dotted lines")
DEFINITION (U"如果开启，虚线将贯穿您的绘图。")
MAN_END

MAN_BEGIN (U"One logarithmic mark left/right/top/bottom...", U"ppgb", 19970330)
INTRO (U"@@Picture window|Picture 窗口@ 的 #Margins（边距）菜单中的四个命令。")
ENTRY (U"目的")
NORMAL (U"沿着对数轴在绘制区域周围的四个边距（@margins）之一中绘制一个标记。")
ENTRY (U"设置")
TERM (U"##Position")
DEFINITION (U"标记的 %x（顶端或底端）或 %y（左端或右端）位置，表示为最近一次在水平或垂直方向上绘制的对数定义域或值域。")
TERM (U"##Write number")
DEFINITION (U"如果开启，将在边距中的 #Position 处写入一个等于 #Position 的实数。")
TERM (U"##Draw tick")
DEFINITION (U"如果开启，将在边距中的 #Position 处绘制一条短线段。")
TERM (U"##Draw dotted line")
DEFINITION (U"如果开启，一条虚线将贯穿您的绘图，其位置等于 #Position。")
TERM (U"##Draw text")
DEFINITION (U"如果不为空，此文本将绘制在边距中，其位置等于 #Position。")
ENTRY (U"示例")
NORMAL (U"在对数范围 100 到 400 Hz 之间绘制音高（基频）后，选择 ##One logarithmic mark left...# 位置为 200 且 %%Draw dotted line% 开启，将在 200 的 %y 位置为您绘制一个水平标记“200”和一条水平虚线，该位置恰好处于 100 和 400 Hz 的正中间。")
MAN_END

MAN_BEGIN (U"One mark left/right/top/bottom...", U"ppgb", 19970330)
INTRO (U"@@Picture window|Picture 窗口@ 的 #Margins（边距）菜单中的四个命令。")
ENTRY (U"目的")
NORMAL (U"在绘制区域周围的四个边距（@margins）之一中绘制一个标记。")
ENTRY (U"设置")
TERM (U"##Position")
DEFINITION (U"标记的 %x（顶端或底端）或 %y（左端或右端）位置，表示为最近一次在水平或垂直方向上绘制的定义域或值域。")
TERM (U"##Write number")
DEFINITION (U"如果开启，将在边距中的 #Position 处写入一个等于 #Position 的实数。")
TERM (U"##Draw tick")
DEFINITION (U"如果开启，将在边距中的 #Position 处绘制一条短线段。")
TERM (U"##Draw dotted line")
DEFINITION (U"如果开启，一条虚线将贯穿您的绘图，其位置等于 #Position。")
TERM (U"##Draw text")
DEFINITION (U"如果不为空，此文本将绘制在边距中，其位置等于 #Position。")
ENTRY (U"示例")
NORMAL (U"如果将声音绘制到 -1 到 1 的振幅范围，选择 ##One mark left...# 位置为 0.0 且 %%Draw dotted line%（绘制虚线）开启，将在 0 的 y 位置为您绘制水平标记“0”和水平虚线。")
MAN_END

MAN_BEGIN (U"Pen menu", U"ppgb", 20091215)
INTRO (U"@@Picture window|Picture 窗口@ 的菜单之一。")
ENTRY (U"目的")
NORMAL (U"选择后续线条和文本绘制中使用的线型和颜色（@colour）。")
ENTRY (U"行为")
NORMAL (U"@@Draw inner box|绘制内边框@ 所使用的线型（实线）以及 ##Mark...# 命令中虚线的线型将不受影响。")
NORMAL (U"@Margins 菜单中的命令将始终以黑色绘制。")
MAN_END

MAN_BEGIN (U"Colour", U"ppgb", 20091215)
INTRO (U"当您从 @@Pen menu|画笔菜单@ 选择 ##Colour...# 或在 ##World menu|世界菜单# 中选择任何 #Paint 命令时弹出的窗口中，您可以看到以下字段：")
CODE (U"Colour (0-1, name, or {r,g,b})")
NORMAL (U"这意味着您可以通过以下三种方式之一在此处指定颜色：")
TERM (U"\\bu 介于 0 和 1 之间的灰度值：")
DEFINITION (U"值为 0 表示黑色，1 表示白色，0.5 表示灰色，0.75 表示银色，依此类推。")
TERM (U"\\bu 颜色名称：")
DEFINITION (U"您可以从 Black、White、Red、Green、Blue、Yellow、Cyan、Magenta、Maroon、Lime、Navy、Teal、Purple、Olive、Pink、Silver、Grey 中进行选择，即画笔菜单中的任何颜色。您可以用大写（Red）或小写（red）来书写这些颜色。")
TERM (U"\\bu RGB 颜色：")
DEFINITION (U"您可以指定红-绿-蓝三色值，作为花括号括起来并用逗号隔开的三个介于 0 和 1 之间的数值，例如 {0.8,0.1,0.2} 是带红色的颜色。")
MAN_END

MAN_BEGIN (U"Picture window", U"ppgb", 20140325)
INTRO (U"Praat 的两个主窗口之一。")
TERM (U"File（文件）菜单")
LIST_ITEM (U"\\bu @@Save as PDF file...")
LIST_ITEM (U"\\bu @@Save as PNG file...")
LIST_ITEM (U"\\bu @@Save as EPS file...")
LIST_ITEM (U"\\bu @@Save as Windows metafile...@")
LIST_ITEM (U"\\bu @@Read from Praat picture file...@, @@Save as Praat picture file...")
LIST_ITEM (U"\\bu @@PostScript settings...")
LIST_ITEM (U"\\bu @@Print...")
TERM (U"Edit（编辑）菜单")
LIST_ITEM (U"\\bu @@Undo@")
LIST_ITEM (U"\\bu @@Copy to clipboard@")
LIST_ITEM (U"\\bu @@Erase all@")
TERM (U"@Margins（边距）菜单")
LIST_ITEM (U"\\bu @@Draw inner box")
LIST_ITEM (U"\\bu @@Text left/right/top/bottom...")
LIST_ITEM (U"\\bu @@Marks left/right/top/bottom every...")
LIST_ITEM (U"\\bu @@One mark left/right/top/bottom...")
LIST_ITEM (U"\\bu @@Marks left/right/top/bottom...")
LIST_ITEM (U"\\bu @@Logarithmic marks left/right/top/bottom...")
LIST_ITEM (U"\\bu @@One logarithmic mark left/right/top/bottom...")
LIST_ITEM (U"\\bu @@Axes...")
TERM (U"World（世界）菜单")
LIST_ITEM (U"\\bu @@Text...")
LIST_ITEM (U"\\bu @@Axes...")
TERM (U"Select（选择）菜单")
LIST_ITEM (U"\\bu @@Select inner viewport...@, @@Select outer viewport...@, @@Viewport text...")
TERM (U"@@Pen menu|画笔菜单@")
TERM (U"@@Font menu|字体菜单@")
MAN_END

MAN_BEGIN (U"PostScript settings...", U"ppgb", 20201229)
INTRO (U"许多窗口的 File 菜单中的命令之一。PostScript 设置会影响打印（@Printing）以及保存为 @@Encapsulated PostScript|封装的 PostScript@ (EPS) 文件。")
ENTRY (U"设置")
TERM (U"##Allow direct PostScript printing#（允许直接 PostScript 打印，仅限 Windows）")
DEFINITION (U"这决定了如果您的打印机是 PostScript 打印机，Praat 是否向该打印机打印显式的 PostScript 命令。这通常是您想要的。然而，如果您发现在 #Print 窗口中选择的某些选项似乎不受支持（例如：缩放、一纸双面打印等），您可以关闭此开关；Praat 将发送 Windows 本地绘制命令，打印机驱动程序将尝试把它们转换为 PostScript。如果您的打印机不支持 PostScript，此开关将被忽略。在 Macintosh 上，此开关被忽略，因为所有打印都以 PDF 格式进行。在 Unix 上，此开关是多余的，因为所有打印都直接以 PostScript 进行。")
TERM (U"##Grey resolution（灰色分辨率）")
DEFINITION (U"您可以从两种图像质量中进行选择：")
LIST_ITEM1 (U"\\bu 灰色图的最佳质量（每英寸 106 点），从打印机直接打印时效果最佳；")
LIST_ITEM1 (U"\\bu 复印适用质量，每英寸的点数较少（85 点），在复印后效果最佳。")
DEFINITION (U"您对灰色分辨率的选择会影响直接 PostScript 打印以及保存到 @@Encapsulated PostScript|封装的 PostScript@ 文件。")
TERM (U"##Paper size#（纸张大小，仅限 Unix）")
DEFINITION (U"您可以选择 A4（210 \\xx 297 毫米）、A3（297 \\xx 420 毫米）或 US Letter（8.5 \\xx 11 码）。此选择仅适用于 Unix；在 Windows 上，您在 ##Print...# 窗口中选择纸张大小；在 Macintosh 上，您在 ##Page setup...# 窗口中选择纸张大小。")
TERM (U"##Orientation#（方向，仅限 Unix）")
DEFINITION (U"您可以选择纵向（%portrait，例如高 297 毫米，宽 210 毫米）或横向（%landscape，例如高 210 毫米，宽 297 毫米）。此选择仅适用于 Unix；在 Windows 上，您在 ##Print...# 窗口中选择方向；在 Macintosh 上，您在 ##Page setup...# 窗口中选择方向。")
TERM (U"##Magnification#（放大倍数，仅限 Unix）")
DEFINITION (U"打印图像的相对大小；通常为 1.0。此选择仅适用于 Unix；在 Windows 上，您在 ##Print...# 窗口中选择缩放比例；在 Macintosh 上，您在 ##Page setup...# 窗口中选择缩放比例。")
TERM (U"##Print command#（打印命令，仅限 Unix）")
DEFINITION (U"在 Unix 上打印时，将在 “/tmp” 文件夹中创建一个 temporary PostScript^\\re 文件；它将具有类似于 “picXXXXXX” 的名称，并在打印后自动删除。此文件将随打印命令发送到打印机，通常类似于 `lp -c %s`，其中 `%s` 代表文件名。")
MAN_END

MAN_BEGIN (U"Print...", U"ppgb", 20001010)
INTRO (U"@@Picture window|Picture 窗口@ 的 File（文件）菜单中的命令之一。")
NORMAL (U"通过此命令，您可以立即将整个图画发送到打印机。详情请参阅 @Printing 教程。")
MAN_END

MAN_BEGIN (U"Printing", U"ppgb", 20120430)
ENTRY (U"1a. 在 Windows 上打印")
NORMAL (U"在 Windows 上，在 PostScript 打印机上将获得最佳效果，因为这些打印机具有用于图像（例如声谱图）和旋转文本的内置功能。如果 PostScript 打印机可用，Praat 通常会直接将 PostScript 命令写入该打印机（如果您想关闭此功能，请参见 @@PostScript settings...@）。Praat 还支持非 PostScript 打印机，例如大多数彩色喷墨打印机。")
NORMAL (U"如果您没有 PostScript 打印机，但仍想要 PostScript 质量，您可以将图画保存为 EPS 文件（@@Save as EPS file...@）。然后，您可以使用免费提供的 GhostView^\\tm 程序查看此文件（您可以从 `http://pages.cs.wisc.edu/~ghost/` 下载），或者使用 GhostView 或 Adobe^\\re Acrobat^\\tm Distiller^\\tm 将其转换为 PDF（后者比 GhostView 更可靠但价格昂贵）。")
ENTRY (U"1b. 在 Macintosh 上打印")
NORMAL (U"在 Mac 上，Praat 将使用 PDF 格式进行打印，无论是打印到 PostScript 还是非 PostScript 打印机。")
ENTRY (U"1c. 在 Linux 上打印")
NORMAL (U"在 Linux 上，当您让 Praat 打印图画或手册页时，Praat 会将图画写入一个临时 PostScript 文件，并使用 %%print command%（打印命令）将该文件发送到打印机，您可以通过 @@PostScript settings...@ 更改该命令。您不需要 PostScript 打印机来直接打印 PostScript，因为 #lpr 程序会将 PostScript 文件发送给 GhostScript 程序（这是所有现代 Linux 发行版的一部分）。打印命令通常为 `lpr %s`。通过更改打印命令（使用 @@PostScript settings...@），您可以将其更改为更复杂的命令。例如，如果您想节约纸张并在单张纸上打印两页，您可以将其更改为 `cat %s | mpage -2 -o -f -m0 | lpr`。")
ENTRY (U"2. 使用字处理器进行间接打印")
NORMAL (U"如果您将图画保存为 EPS 文件，您将能够将其作为图片插入到您常用的字处理器（Microsoft^\\re Word^\\tm, LaTeX...）中。参见 @@Save as EPS file...@。")
NORMAL (U"在 Mac 上，最好将图画保存为 PDF 文件，Microsoft^\\re Word^\\tm（使用 `.docx` 格式时）和 LaTeX 都可以处理该文件。参见 @@Save as PDF file...@。")
ENTRY (U"3. 通过剪贴板进行间接打印")
NORMAL (U"在 Windows 上，通过 @@Copy to clipboard|复制到剪贴板@ 或 @@Save as Windows metafile...@ 导入到您的字处理器中的图画可以正常打印，尽管质量不如 EPS 文件。")
NORMAL (U"在 Mac 上，通过 @@Copy to clipboard|复制到剪贴板@ 导入到您的字处理器（2006年后版本）中的图画的打印效果与 PDF 文件一样好（请在 Microsoft Word 中使用 `.docx` 格式）。")
MAN_END

MAN_BEGIN (U"Read from Praat picture file...", U"ppgb", 20110129)
INTRO (U"@@Picture window|Picture 窗口@ 的 File（文件）菜单中的命令之一。")
ENTRY (U"目的")
NORMAL (U"读取您早前使用 @@Save as Praat picture file...@ 保存的图片。")
ENTRY (U"行为")
NORMAL (U"图片将被绘制在 Picture 窗口中当前可见的所有内容之上。")
ENTRY (U"用法")
NORMAL (U"借助此命令，您可以将图片从 Unix 机器传输到 Macintosh。Macintosh 版本的 Praat 可以将图片写入带有屏幕预览的 @@Encapsulated PostScript|封装的 PostScript@ 文件中。")
MAN_END

MAN_BEGIN (U"Text...", U"ppgb", 19970330)
INTRO (U"@@Picture window|Picture 窗口@ 的 #World（世界）菜单中的命令之一。")
ENTRY (U"目的")
NORMAL (U"在绘制区域内部写入文本。")
ENTRY (U"范围")
NORMAL (U"这适用于所有在其周围留出边距（@margins）的绘制。")
ENTRY (U"设置")
TERM (U"%x")
DEFINITION (U"水平位置，表示为最近一次绘制的水平定义域。")
TERM (U"%y")
DEFINITION (U"垂直位置，表示为最近一次绘制的垂直值域或定义域。")
TERM (U"%%Horizontal alignment")
DEFINITION (U"决定文本相对于 %x 的水平对齐方式。")
TERM (U"%%Vertical alignment")
DEFINITION (U"决定文本相对于 %y 的垂直对齐方式。")
TERM (U"%%Text")
DEFINITION (U"将以您使用 @@Font menu|字体菜单@ 设置的当前字体和字体大小进行绘制。")
ENTRY (U"用法")
NORMAL (U"通过 ##Text...# 命令，您可以使用所有 @@special symbols|特殊符号@ 和 @@text styles|文本样式@。")
MAN_END

MAN_BEGIN (U"Insert picture from file...", U"ppgb", 20140608)
INTRO (U"@@Picture window|Picture 窗口@ 的 #World（世界）菜单中的命令之一。")
ENTRY (U"目的")
NORMAL (U"将图片文件（PNG；在 Mac 和 Windows 上也支持 JPEG 或 TIFF；其他图片格式也可能适用）绘制到 Picture 窗口（或 Demo 窗口）中。")
ENTRY (U"设置")
TERM (U"##File name")
DEFINITION (U"图片文件的名称。如果在脚本中使用此命令，您可以使用相对路径名，例如 `pictures/myface.png` or `~/Desktop/hello.png`。")
TERM (U"##From x")
TERM (U"##To x")
DEFINITION (U"图片将出现的水平位置（使用世界坐标）。如果先前没有通过 #Draw 或 #Paint 命令隐式设置世界坐标，请使用 @@Axes...|Axes...@ 命令来设置它们。如果将 ##From x# 和 ##To x# 设置为相同的值，图片将在水平方向上居中于该 %x 值，并自动获得一个保持原始图片宽高比（宽度与高度之比）的宽度。")
TERM (U"##From y")
TERM (U"##To y")
DEFINITION (U"图片将出现的垂直位置（使用世界坐标）。如果先前没有通过 #Draw 或 #Paint 命令隐式设置世界坐标，请使用 @@Axes...|Axes...@ 命令来设置它们。如果将 ##From y# 和 ##To y# 设置为相同的值，图片将在垂直方向上居中于该 %y 值，并自动获得一个保持原始图片宽高比（宽度与高度之比）的高度。")
ENTRY (U"行为")
NORMAL (U"根据上述说明，您会发现如果 ##From x# 不等于 ##To x# 且 ##From y# 不等于 ##To y#，图片的宽高比可能会与原始图片不同。为了保持原始宽高比，您可以让 ##To x# 等于 ##From x#，或者让 ##To y# 等于 ##From y#。")
NORMAL (U"要在与原始图像相同的像素数上显示图片，请让 ##To x# 等于 ##From x# 且 ##To y# 等于 ##From y#。Praat 将显示具有原始宽度和高度（像素）的图片，并居中于给定的 %x 和 %y 值。")
ENTRY (U"用法")
NORMAL (U"您通常会在脚本中使用此命令，且经常在运行演示窗口（@@Demo window@）的脚本中使用。")
MAN_END

MAN_BEGIN (U"Text left/right/top/bottom...", U"ppgb", 19970330)
INTRO (U"@@Picture window|Picture 窗口@ 的 #Margins（边距）菜单中的四个命令。")
ENTRY (U"目的")
NORMAL (U"在绘制区域周围的边距（@margins）中写入文本。")
ENTRY (U"行为")
NORMAL (U"文本将沿边缘居中。左侧或右侧的文本将旋转 90 度，并分别自下而上或自上而下书写。")
MAN_END

MAN_BEGIN (U"Undo", U"ppgb", 20091215)
INTRO (U"@@Picture window|Picture 窗口@ 的 #Edit（编辑）菜单中的命令之一。")
NORMAL (U"它会擦除您最近创建的图画，这可能来自对象窗口（Objects window）中的命令，或来自世界（World）菜单和边距（@Margins）菜单中的绘制命令。")
ENTRY (U"行为")
NORMAL (U"此命令将擦除一些绘图，但它不会改变您在 #Pen（画笔）和 #Font（字体）菜单中的设置（线型、线宽、字体、字体大小和颜色 @colour）。")
NORMAL (U"世界窗口将恢复为倒数第二次绘制后的状态，以便您可以正常使用 @Margins 菜单，就好像最近一次的绘制从未发生过一样。")
MAN_END

MAN_BEGIN (U"Select inner viewport...", U"ppgb", 20041108)
INTRO (U"@@Picture window|Picture 窗口@ 的 #Select（选择）菜单中的命令之一。")
ENTRY (U"目的")
NORMAL (U"决定您下一次绘制将发生在哪里。")
ENTRY (U"视口（viewport）")
NORMAL (U"“视口”是 Picture 窗口中您下一次绘制将要发生的部分。它的边界是粉红色的。")
NORMAL (U"内部视口（%inner viewport）不包括边距，而外部视口（%outer viewport）包括（参见 @@Select outer viewport...@）。")
NORMAL (U"通常情况下，您可以通过用鼠标拖动划过 Picture 窗口来选择视口。然而，在以下情况下您会使用此显式命令：")
LIST_ITEM (U"\\bu 在脚本中；")
LIST_ITEM (U"\\bu 如果您需要一个不能用半英寸（halves of an inch）的倍数表示的视口。")
MAN_END

MAN_BEGIN (U"Select outer viewport...", U"ppgb", 20041108)
INTRO (U"@@Picture window|Picture 窗口@ 的 #Select（选择）菜单中的命令之一。")
ENTRY (U"目的")
NORMAL (U"决定您下一次绘制将发生在哪里。")
ENTRY (U"视口（viewport）")
NORMAL (U"“视口”是 Picture 窗口中您下一次绘制将要发生的部分。它的边界是粉红色的。")
NORMAL (U"外部视口（%outer viewport）包括边距，而内部视口（%inner viewport）不包括（参见 @@Select inner viewport...@）。")
NORMAL (U"通常情况下，您可以通过用鼠标拖动划过 Picture 窗口来选择视口。然而，在以下情况下您会使用此显式命令：")
LIST_ITEM (U"\\bu 在脚本中；")
LIST_ITEM (U"\\bu 如果您需要一个不能用半英寸（halves of an inch）的倍数表示的视口。")
MAN_END

MAN_BEGIN (U"Viewport text...", U"ppgb", 19970330)
INTRO (U"@@Picture window|Picture 窗口@ 的 #Select（选择）菜单中的命令之一。")
ENTRY (U"目的")
NORMAL (U"在视口内的九个不同位置写入文本，旋转角度在 0 到 360 度之间。")
ENTRY (U"设置：")
TERM (U"##Horizontal alignment")
DEFINITION (U"决定文本的水平对齐方式：")
LIST_ITEM (U"    \\bu #Left（左对齐）表示紧贴视口的左边缘；")
LIST_ITEM (U"    \\bu #Right（右对齐）表示紧贴视口的右边缘；")
LIST_ITEM (U"    \\bu #Centre（居中）表示在视口中水平居中。")
TERM (U"##Vertical alignment")
DEFINITION (U"决定文本的垂直对齐方式：")
LIST_ITEM (U"    \\bu #Top（顶对齐）表示紧贴视口的顶部；")
LIST_ITEM (U"    \\bu #Bottom（底对齐）表示紧贴视口的底部；")
LIST_ITEM (U"    \\bu #Half（半对齐）表示在视口中垂直居中。")
TERM (U"##Text")
DEFINITION (U"将以您使用 @@Font menu|字体菜单@ 设置的当前字体和字体大小进行绘制。")
ENTRY (U"行为")
NORMAL (U"对于旋转的文本，对齐设置不仅将决定视口内的位置，还将决定旋转坐标系中的对齐方式。这偶尔会带来意想不到的效果；因此，如果您希望有几个相互对齐的旋转文本，您应该通过改变视口来做，而不是通过改变对齐方式。")
ENTRY (U"用法")
NORMAL (U"您可以使用所有 @@special symbols|特殊符号@ 和 @@text styles|文本样式@。")
MAN_END

MAN_BEGIN (U"Save as EPS file...", U"ppgb", 20140325)
INTRO (U"@@Picture window|Picture 窗口@ 的 File（文件）菜单中的命令之一。")
NORMAL (U"它将图画保存为 @@Encapsulated PostScript|封装的 PostScript@ (EPS) 文件，该文件可以被许多其他程序（如 Microsoft^\\re Word^\\tm）导入。")
ENTRY (U"用法")
NORMAL (U"EPS 文件正在逐渐被淘汰，主要是因为它们不能真正支持国际化文本。在 Macintosh 和 Linux 上，几乎总是改用 @@Save as PDF file...@ 或者（在 Mac 上）使用 @@Copy to clipboard|复制到剪贴板@ 更好。在尚不能真正支持 PDF 文件的 Windows 上，您可能会发现 EPS 文件有时会比 @@Copy to clipboard|复制到剪贴板@ 产生更好的质量；您也可以在 Windows 上查看 @@Save as PNG file...@。要在 Word 中导入 EPS 文件，请选择 #Insert（插入）\\-> #Picture（图片）\\-> ##From file...#。")
ENTRY (U"行为")
NORMAL (U"尽管 Picture 窗口的所有内容都会写入 EPS 文件，但在 Word（或其他程序）中可能只有您在 Picture 窗口中选中的部分（%viewport，即视口）才可见。")
ENTRY (U"设置")
NORMAL (U"EPS 图片将以您使用 @@PostScript settings...|PostScript 设置...@ 指定的灰色分辨率和字体进行保存。")
MAN_END

MAN_BEGIN (U"Save as PDF file...", U"ppgb", 20140325)
INTRO (U"Macintosh 和 Linux 上 @@Picture window|Picture 窗口@ 的 File（文件）菜单中的命令之一。")
NORMAL (U"它将图画保存为 PDF 文件，该文件可以被其他几种程序（如现代版本的 Microsoft^\\re Word\\tm）导入。")
ENTRY (U"PDF 意味着尽可能最高的质量")
NORMAL (U"通过 PDF 图片，您可以在字处理器文档中使用高质量的图形。在 Mac 上，其质量与使用 @@Copy to clipboard|复制到剪贴板@ 完全相同。")
NORMAL (U"在 Windows 上，请改用 @@Save as PNG file...@ 或 @@Save as EPS file...@。")
ENTRY (U"行为")
NORMAL (U"尽管 Picture 窗口的所有内容都会写入 PDF 文件，但在 Word（或其他程序）中只有您在 Picture 窗口中选中的部分（%viewport，即视口）才会可见。")
ENTRY (U"用法")
NORMAL (U"要在 Word 中导入 PDF 文件，请选择 #Insert \\-> #Picture \\-> ##From file...#。Word 将创建一张与 Picture 窗口原始选中部分（%viewport，即视口）大小相同的图片。")
MAN_END

MAN_BEGIN (U"Save as PNG file...", U"ppgb", 20140325)
INTRO (U"所有平台上 @@Picture window|Picture 窗口@ 的 File（文件）菜单中的命令之一。")
NORMAL (U"它将图画保存为 PNG（“ping”）图像文件，该文件可以被其他几种程序（如 Microsoft^\\re Word\\tm）导入。对于分辨率，您可以在 600 像素/英寸（即使在打印时也具有极佳质量）和 300 dpi（对于所有网站都足够，即使是在视网膜显示屏上）之间进行选择。")
ENTRY (U"在文本编辑器中的用法")
NORMAL (U"在 Windows 上，PNG 文件有时可能会为您提供可以获得的最佳质量，尽管您也应该尝试 @@Save as EPS file...@ 和 @@Copy to clipboard|复制到剪贴板@。在 Macintosh 或 Linux 上，@@Save as PDF file...@ 或 @@Copy to clipboard|复制到剪贴板@ 几乎总是更好。")
ENTRY (U"用于出版的用法")
NORMAL (U"一些出版商不接受 PDF 图片。在这种情况下，他们可能会接受 600-dpi 或 300-dpi 的 PNG 图片。如果他们仅接受 TIFF 图片，那么您可以使用任何图形转换器程序轻松地将 PNG 图片转换为 TIFF 图片。")
ENTRY (U"行为")
NORMAL (U"只有您选中的 Picture 窗口部分的内客（%viewport，即视口）才会被写入 PNG 文件。")
ENTRY (U"用法")
NORMAL (U"要在 Word 中导入 PNG 文件，请选择 #Insert \\-> #Picture \\-> ##From file...#。Word 将创建一张与 Picture 窗口原始选中部分（%viewport，即视口）大小相同的图片。")
MAN_END

MAN_BEGIN (U"Save as Praat picture file...", U"ppgb", 20110129)
INTRO (U"@@Picture window|Picture 窗口@ 的 File（文件）菜单中的命令之一。")
NORMAL (U"它以一种稍后可以通过 @@Read from Praat picture file...@ 导入到 Praat 中的格式保存图片。")
ENTRY (U"用法")
NORMAL (U"借助此命令，您可以在计算机之间甚至不同平台之间传输 Picture 窗口的内容，例如从 Macintosh 传输到 Windows 计算机。")
MAN_END

MAN_BEGIN (U"Save as Windows metafile...", U"ppgb", 20140325)
INTRO (U"如果您在 Windows 上，@@Picture window|Picture 窗口@ 的 File（文件）菜单中的命令之一。")
NORMAL (U"它将图片的选中部分保存为“增强型图元文件”（.EMF）格式，该格式可以被许多 Windows 程序导入，如 Adobe^\\re Illustrator^\\tm 或 Microsoft^\\re Word^\\tm。")
ENTRY (U"行为")
NORMAL (U"尽管所有的图片数据都将写入该文件，但在另一个程序中可能只有您在 Picture 窗口中选中的部分（%viewport，即视口）才会可见。")
ENTRY (U"用法")
NORMAL (U"您不会非常频繁地使用此命令，因为通常更容易通过 @@Copy to clipboard|复制到剪贴板@ 命令将选中内容复制到剪贴板，并“粘贴”到另一个程序中。如果剪贴板太大以至于另一个程序无法读取，或者如果您想将图片传输到另一台计算机，您可以使用图元文件来代替剪贴板。")
MAN_END

}

/* End of file manual_Picture.cpp */
