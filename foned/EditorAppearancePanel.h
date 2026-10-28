#ifndef _EditorAppearancePanel_h_
#define _EditorAppearancePanel_h_
/* EditorAppearancePanel.h
 *
 * Copyright (C) 2026 yjzxkxdn, 2026 Paul Boersma
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

#include "EditorAppearance.h"

/*
	A panel that belongs to one editor window and holds the two appearance settings of that window:
	the colour scheme and the whole-window opacity. The panel is a native top-level popup of its own,
	so it stays readable even when the window it belongs to is drawn at 20 % opacity.
	On platforms other than Windows there is no panel; `create` returns null and the editor keeps
	its existing menu behaviour.
*/

struct structEditorAppearancePanel;
typedef struct structEditorAppearancePanel *EditorAppearancePanel;

typedef struct EditorAppearancePanel_callbacks {
	void *boss;
	void (*changePalette) (void *boss, kEditorAppearancePalette palette);
		/* Called when the user picks a colour scheme; the value is already in effect. */
	void (*previewOpacity) (void *boss, integer percent);
		/*
			Called for every percentage the held-down pointer asks for, and never twice for the same one:
			only the compositing of the window changes, nothing is recomputed or drawn again. If that
			percentage cannot be put in effect, the callback brings back the value that is in effect with
			`EditorAppearancePanel_setState`; the panel takes that as the end of the drag.
		*/
	void (*commitOpacity) (void *boss, integer percent);
		/*
			Called once per finished operation — the release of a drag, or one key — with the percentage
			the user settled on. Nothing is committed for a press that changed no value, and a native end
			notification that repeats a percentage already in effect is not committed a second time.
		*/
} EditorAppearancePanel_callbacks;

typedef struct EditorAppearancePanel_state {
	kEditorAppearancePalette palette;
	integer opacityPercent;   // 20 through 100
} EditorAppearancePanel_state;

EditorAppearancePanel EditorAppearancePanel_create (const EditorAppearancePanel_callbacks *callbacks);
void EditorAppearancePanel_dispose (EditorAppearancePanel me);

bool EditorAppearancePanel_show (EditorAppearancePanel me, void *ownerWindow,
		int anchorLeft, int anchorTop, int anchorWidth, int anchorHeight,
		const EditorAppearancePanel_state *state);
	/*
		Open the panel just below the rectangle that its menu entry occupies in the owner's menu bar,
		laid out for the DPI of that monitor and clipped to its work area.
		Return value: false if the panel could not be opened.
	*/
void EditorAppearancePanel_dismiss (EditorAppearancePanel me);
	/* Hide the panel, keeping the values the user has set. */
void EditorAppearancePanel_setState (EditorAppearancePanel me, const EditorAppearancePanel_state *state);
	/* Bring the controls in line with the window state, for example after another menu changed it. */
bool EditorAppearancePanel_isShowing (EditorAppearancePanel me);
bool EditorAppearancePanel_steppedAsideForAClick (EditorAppearancePanel me);
	/*
		True for a moment after the panel hid itself because a click somewhere else took its keyboard
		away. The menu-bar entry uses this to recognise the second half of that same click.
	*/

/* End of file EditorAppearancePanel.h */
#endif
