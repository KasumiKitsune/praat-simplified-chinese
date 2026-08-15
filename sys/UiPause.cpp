/* UiPause.cpp
 *
 * Copyright (C) 2009-2020,2022-2026 Paul Boersma
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

#include "UiPause.h"
#include "praatP.h"

static autoUiForm thePauseForm;
static int thePauseForm_clicked = 0;
static int theCancelContinueButton = 0;
static Interpreter thePauseForm_interpreterReference;  // TODO: this should be a weak_ptr to the InterpreterStack
static structMelderFolder thePauseForm_savedFolder;   // TODO: probably remove
static Editor thePauseForm_savedEditorReference;

void UiPause_interpreterGoesAway (Interpreter interpreter) {
	if (interpreter == thePauseForm_interpreterReference)
		thePauseForm_interpreterReference = nullptr;   // undangle
}

static void thePauseFormOkCallback (UiForm /* sendingForm */, integer /* narg */, Stackel /* args */,
	conststring32 /* sendingString */, Interpreter /* interpreter */,
	conststring32 /* invokingButtonTitle */, bool /* modified */, void *closure, Editor optionalEditor)
{
	if (! thePauseForm_interpreterReference) {   // interpreter was destroyed?
		GuiThing_hide (thePauseForm -> d_dialogForm);   // BUG: memory leak
		thePauseForm_savedEditorReference = nullptr;
		thePauseForm. releaseToUser();
		return;
	}
	if (! thePauseForm)   // BUG: perhaps there was a mistake in the script
		return;
	Melder_assert (thePauseForm_interpreterReference);
	Melder_assert (Thing_isa (thePauseForm_interpreterReference, classInterpreter));
	if (thePauseForm_interpreterReference -> optionalDynamicEnvironmentEditor() != thePauseForm_savedEditorReference) {
		Melder_assert (thePauseForm_savedEditorReference);
		Melder_assert (! thePauseForm_interpreterReference -> optionalDynamicEnvironmentEditor());
		Melder_assert (thePauseForm_interpreterReference -> optionalDynamicEditorEnvironmentClassName());
				// testing the assumption that the environment can be lost but never added during pause
		Melder_assert (thePauseForm);
		if (thePauseForm -> d_dialogForm) {
			GuiThing_hide (thePauseForm -> d_dialogForm);   // BUG: memory leak
			//thePauseForm_interpreterReference -> isInSecondPass = false;   // TODO: corrected? and what about isHalted?
		}
		thePauseForm. releaseToUser();   // undangle (will be autodestroyed when unmanaged)
		Melder_assert (! thePauseForm);
		const integer lineNumber = thePauseForm_interpreterReference -> lineNumber;
		Interpreter_stop (thePauseForm_interpreterReference);
		Melder_flushError (U"Cannot continue after pause, because the ", thePauseForm_interpreterReference -> optionalDynamicEditorEnvironmentClassName(), U" has been closed.",
				U"\nScript interrupted at line ", lineNumber, U".");   // a top-level error message
		return;
	}
	/*
		Get the data from the pause form.
	*/
	Melder_assert (thePauseForm);
	thePauseForm_clicked = UiForm_getClickedContinueButton (thePauseForm.get());
	if (thePauseForm_clicked != theCancelContinueButton)
		UiForm_Interpreter_addVariables (thePauseForm.get(), (Interpreter) closure);   // 'closure', not 'interpreter' or 'theInterpreter'!
	Melder_assert (thePauseForm_clicked >= 1 && thePauseForm_clicked <= 10);
	autostring32 clickedText = Melder_dup (thePauseForm -> continueTexts [thePauseForm_clicked].get());   // very safe
	if (thePauseForm -> d_dialogForm) {
		GuiThing_hide (thePauseForm -> d_dialogForm);   // BUG: memory leak
		//thePauseForm_interpreterReference -> isInSecondPass = false;   // TODO: corrected? and what about isHalted?
	}
	thePauseForm. releaseToUser();   // undangle (will be autodestroyed when unmanaged)
	Melder_assert (! thePauseForm);
	/*
		Resume the interpreter.
	*/
	//Melder_setCurrentFolder (& thePauseForm_savedFolder);   // TODO: remove
	Melder_assert (thePauseForm_interpreterReference);
	try {
		autoPraatBackground background;
		Melder_assert (thePauseForm_interpreterReference -> owningInterpreterStack);
		thePauseForm_interpreterReference -> owningInterpreterStack -> resumeFromTop ();
	} catch (MelderError) {
		/*
			These errors will often not be reached, because Interpreter_resume contains some Melder_flushError().
		*/
		if (thePauseForm_clicked == theCancelContinueButton)
			Melder_throw (U"This happened after you cancelled the pause form.");
		else
			Melder_throw (U"This happened after you clicked “", clickedText.get(), U"” in the pause form.");
	}
}
static void thePauseFormCancelCallback (UiForm /* dia */, void * /* closure */) {
	if (! thePauseForm_interpreterReference) {   // interpreter was destroyed?
		GuiThing_hide (thePauseForm -> d_dialogForm);   // BUG: memory leak
		thePauseForm_savedEditorReference = nullptr;
		thePauseForm. releaseToUser();
		return;
	}
	if (! thePauseForm)   // BUG: perhaps there was a mistake in the script
		return;
	Melder_assert (thePauseForm_interpreterReference);
	Melder_assert (Thing_isa (thePauseForm_interpreterReference, classInterpreter));
	/*
		We arrive here if the user clicked the close box of the dialog window, or if they clicked Stop.
	*/
	Melder_assert (thePauseForm);
	if (thePauseForm -> d_dialogForm) {
		GuiThing_hide (thePauseForm -> d_dialogForm);   // BUG: memory leak
		//thePauseForm_interpreterReference -> isInSecondPass = false;   // TODO: corrected? and what about isHalted?
	}
	thePauseForm. releaseToUser();   // undangle (will be autodestroyed when unmanaged)
	Melder_assert (! thePauseForm);
	if (theCancelContinueButton != 0) {
		/*
			The pause window apparently contains a "Cancel"-like button, so it doesn’t contain a Stop button.
			Hence, if we arrive here, the user must have clicked the close box of the dialog window.
			We divert this click to the "Cancel"-like button,
			and it will be the script’s responsibility to handle it.
		*/
		thePauseForm_clicked = theCancelContinueButton;   // there was
		if (thePauseForm_interpreterReference -> optionalDynamicEnvironmentEditor() != thePauseForm_savedEditorReference) {
			Melder_assert (thePauseForm_savedEditorReference);
			Melder_assert (! thePauseForm_interpreterReference -> optionalDynamicEnvironmentEditor());
			Melder_assert (thePauseForm_interpreterReference -> optionalDynamicEditorEnvironmentClassName());
					// testing the assumption that the environment can be lost but never added during pause
			const integer lineNumber = thePauseForm_interpreterReference -> lineNumber;
			Interpreter_stop (thePauseForm_interpreterReference);
			Melder_flushError (U"Cannot continue after pause, because the ", thePauseForm_interpreterReference -> optionalDynamicEditorEnvironmentClassName(), U" has been closed.",
					U"\nScript interrupted at line ", lineNumber, U".");   // a top-level error message
		}
		/*
			Resume the interpreter.
		*/
		//Melder_setCurrentFolder (& thePauseForm_savedFolder);   // TODO: remove
		try {
			autoPraatBackground background;
			Melder_assert (thePauseForm_interpreterReference -> owningInterpreterStack);
			thePauseForm_interpreterReference -> owningInterpreterStack -> resumeFromTop ();
		} catch (MelderError) {
			Melder_flushError (U"This happened after you stopped the pause form.");
		}
	} else {
		/*
			The pause window apparently contains no "Cancel"-like button, so it must contain a Stop button.
			Perform the normal action for the Stop button, which is to post a message about interruption.
			A click in the close box of the dialog window performs the same action as the Stop button.
		*/
		const integer lineNumber = thePauseForm_interpreterReference -> lineNumber;
		Interpreter_stop (thePauseForm_interpreterReference);
		Melder_flushError (U"You interrupted the script at line ", lineNumber, U".");   // a top-level error message
	}
}
void UiPause_begin (GuiWindow topShell, Editor optionalPauseWindowOwningEditor, conststring32 title, Interpreter interpreter) {
	Melder_assert (interpreter);
	if (interpreter -> isInSecondPass)
		return;   // this can happen in `pauseScript()` and in `pause`.
	if (thePauseForm)
		Melder_throw (Melder_upperCaseAppName(), U" cannot have more than one pause form at a time.");
	thePauseForm = UiForm_create (topShell, optionalPauseWindowOwningEditor, Melder_cat (U"Pause: ", title),
		thePauseFormOkCallback, interpreter,   // pass interpreter as closure!
		nullptr, nullptr
	);
}
void UiPause_real (conststring32 label, conststring32 defaultValue) {
	if (! thePauseForm)
		Melder_throw (U"The function “real” should be between a “beginPause” and an “endPause”.");
	UiForm_addReal (thePauseForm.get(), nullptr, nullptr, label, defaultValue);
}
void UiPause_positive (conststring32 label, conststring32 defaultValue) {
	if (! thePauseForm)
		Melder_throw (U"The function “positive” should be between a “beginPause” and an “endPause”.");
	UiForm_addPositive (thePauseForm.get(), nullptr, nullptr, label, defaultValue);
}
void UiPause_integer (conststring32 label, conststring32 defaultValue) {
	if (! thePauseForm)
		Melder_throw (U"The function “integer” should be between a “beginPause” and an “endPause”.");
	UiForm_addInteger (thePauseForm.get(), nullptr, nullptr, label, defaultValue);
}
void UiPause_natural0 (conststring32 label, conststring32 defaultValue) {
	if (! thePauseForm)
		Melder_throw (U"The function “natural0” should be between a “beginPause” and an “endPause”.");
	UiForm_addNatural0 (thePauseForm.get(), nullptr, nullptr, label, defaultValue);
}
void UiPause_natural1 (conststring32 label, conststring32 defaultValue) {
	if (! thePauseForm)
		Melder_throw (U"The function “natural1” should be between a “beginPause” and an “endPause”.");
	UiForm_addNatural1 (thePauseForm.get(), nullptr, nullptr, label, defaultValue);
}
void UiPause_natural (conststring32 label, conststring32 defaultValue) {
	if (! thePauseForm)
		Melder_throw (U"The function “natural” should be between a “beginPause” and an “endPause”.");
	UiForm_addNatural1 (thePauseForm.get(), nullptr, nullptr, label, defaultValue);
}
void UiPause_word (conststring32 label, conststring32 defaultValue) {
	if (! thePauseForm)
		Melder_throw (U"The function “word” should be between a “beginPause” and an “endPause”.");
	UiForm_addWord (thePauseForm.get(), nullptr, nullptr, label, defaultValue);
}
void UiPause_sentence (conststring32 label, conststring32 defaultValue) {
	if (! thePauseForm)
		Melder_throw (U"The function “sentence” should be between a “beginPause” and an “endPause”.");
	UiForm_addSentence (thePauseForm.get(), nullptr, nullptr, label, defaultValue);
}
void UiPause_text (conststring32 label, conststring32 defaultValue, integer numberOfLines) {
	if (! thePauseForm)
		Melder_throw (U"The function “text” should be between a “beginPause” and an “endPause”.");
	UiForm_addText (thePauseForm.get(), nullptr, nullptr, label, defaultValue, numberOfLines);
}
void UiPause_boolean (conststring32 label, bool defaultValue) {
	if (! thePauseForm)
		Melder_throw (U"The function “boolean” should be between a “beginPause” and an “endPause”.");
	UiForm_addBoolean (thePauseForm.get(), nullptr, nullptr, label, defaultValue);
}
void UiPause_infile (conststring32 label, conststring32 defaultValue, integer numberOfLines) {
	if (! thePauseForm)
		Melder_throw (U"The function “infile” should be between a “beginPause” and an “endPause”.");
	UiForm_addInfile (thePauseForm.get(), nullptr, nullptr, label, defaultValue, numberOfLines);
}
void UiPause_outfile (conststring32 label, conststring32 defaultValue, integer numberOfLines) {
	if (! thePauseForm)
		Melder_throw (U"The function “outfile” should be between a “beginPause” and an “endPause”.");
	UiForm_addOutfile (thePauseForm.get(), nullptr, nullptr, label, defaultValue, numberOfLines);
}
void UiPause_folder (conststring32 label, conststring32 defaultValue, integer numberOfLines) {
	if (! thePauseForm)
		Melder_throw (U"The function “folder” should be between a “beginPause” and an “endPause”.");
	UiForm_addFolder (thePauseForm.get(), nullptr, nullptr, label, defaultValue, numberOfLines);
}
void UiPause_realvector (conststring32 label, kUi_realVectorFormat defaultFormat, conststring32 defaultValue, integer numberOfLines) {
	if (! thePauseForm)
		Melder_throw (U"The function “realvector” should be between a “beginPause” and an “endPause”.");
	UiForm_addRealVector (thePauseForm.get(), nullptr, nullptr, label, defaultFormat, defaultValue, numberOfLines);
}
void UiPause_positivevector (conststring32 label, kUi_realVectorFormat defaultFormat, conststring32 defaultValue, integer numberOfLines) {
	if (! thePauseForm)
		Melder_throw (U"The function “positivevector” should be between a “beginPause” and an “endPause”.");
	UiForm_addRealVector (thePauseForm.get(), nullptr, nullptr, label, defaultFormat, defaultValue, numberOfLines);
}
void UiPause_integervector (conststring32 label, kUi_integerVectorFormat defaultFormat, conststring32 defaultValue, integer numberOfLines) {
	if (! thePauseForm)
		Melder_throw (U"The function “integervector” should be between a “beginPause” and an “endPause”.");
	UiForm_addIntegerVector (thePauseForm.get(), nullptr, nullptr, label, defaultFormat, defaultValue, numberOfLines);
}
void UiPause_naturalvector (conststring32 label, kUi_integerVectorFormat defaultFormat, conststring32 defaultValue, integer numberOfLines) {
	if (! thePauseForm)
		Melder_throw (U"The function “naturalvector” should be between a “beginPause” and an “endPause”.");
	UiForm_addIntegerVector (thePauseForm.get(), nullptr, nullptr, label, defaultFormat, defaultValue, numberOfLines);
}
void UiPause_choice (conststring32 label, int defaultValue) {
	if (! thePauseForm)
		Melder_throw (U"The function “choice” should be between a “beginPause” and an “endPause”.");
	UiForm_addChoice (thePauseForm.get(), nullptr, nullptr, nullptr, label, defaultValue, 1);
}
void UiPause_optionmenu (conststring32 label, int defaultValue) {
	if (! thePauseForm)
		Melder_throw (U"The function “optionmenu” should be between a “beginPause” and an “endPause”.");
	UiForm_addOptionMenu (thePauseForm.get(), nullptr, nullptr, nullptr, label, defaultValue, 1);
}
void UiPause_option (conststring32 optionText) {
	if (! thePauseForm)
		Melder_throw (U"The function “option” should be between a “beginPause” and an “endPause”.");
	UiOption option = UiForm_addOption (thePauseForm.get(), optionText);
	if (! option) {
		thePauseForm. reset();
		Melder_throw (U"Found the function “option” without a preceding “choice” or “optionmenu”.");
	}
}
void UiPause_heading (conststring32 label) {
	if (! thePauseForm)
		Melder_throw (U"The function “comment” should be between a “beginPause” and an “endPause”.");
	UiForm_addHeading (thePauseForm.get(), nullptr, label);
}
void UiPause_comment (conststring32 label) {
	//if (thePauseForm_secondPass)
	//	return;   // this can happen in `pauseScript()` and in `pause`. TODO: is this needed?
	if (! thePauseForm)
		Melder_throw (U"The function “comment” should be between a “beginPause” and an “endPause”.");
	UiForm_addComment (thePauseForm.get(), nullptr, label);
}
void UiPause_caption (conststring32 label) {
	if (! thePauseForm)
		Melder_throw (U"The function “caption” should be between a “beginPause” and an “endPause”.");
	UiForm_addCaption (thePauseForm.get(), nullptr, label);
}

int UiPause_end (int numberOfContinueButtons, int defaultContinueButton, int cancelContinueButton,
	conststring32 continueText1, conststring32 continueText2, conststring32 continueText3,
	conststring32 continueText4, conststring32 continueText5, conststring32 continueText6,
	conststring32 continueText7, conststring32 continueText8, conststring32 continueText9,
	conststring32 continueText10, Interpreter interpreter)
{
	//TRACE
	Melder_assert (interpreter);
	trace (U"is in second Pass? ", interpreter -> isInSecondPass);
	if (interpreter -> isInSecondPass) {
		Melder_assert (interpreter == thePauseForm_interpreterReference);
		Melder_assert (! thePauseForm);
		interpreter -> isInSecondPass = false;   // TODO: needed?
		return thePauseForm_clicked;
	} else {
		if (! thePauseForm)
			Melder_throw (U"Found the function “endPause” without a preceding “beginPause”.");
		thePauseForm_interpreterReference = interpreter;
		thePauseForm_savedEditorReference = interpreter -> optionalDynamicEnvironmentEditor();
		UiForm_setPauseForm (thePauseForm.get(), numberOfContinueButtons, defaultContinueButton, cancelContinueButton,
			continueText1, continueText2, continueText3, continueText4, continueText5,
			continueText6, continueText7, continueText8, continueText9, continueText10,
			thePauseFormCancelCallback
		);
		theCancelContinueButton = cancelContinueButton;
		UiForm_finish (thePauseForm.get());
		//if (theCurrentPraatApplication -> batch) goto end;
		thePauseForm_clicked = 0;
		Melder_assert (interpreter -> owningInterpreterStack);
		interpreter -> owningInterpreterStack -> haltAll ();
		/*
			Put the pause form on the screen.
		*/
		UiForm_destroyWhenUnmanaged (thePauseForm.get());
		UiForm_do (thePauseForm.get(), false);
		return 0;
	}
}

void UiPause_pauseScript (GuiWindow topShell, Editor optionalPauseWindowOwningEditor, Interpreter interpreter, conststring32 text) {
	Melder_assert (interpreter);
	if (interpreter -> isInSecondPass) {
		Melder_assert (interpreter == thePauseForm_interpreterReference);
		Melder_assert (! thePauseForm);
		interpreter -> isInSecondPass = false;
	} else {
		if (thePauseForm)
			Melder_throw (Melder_upperCaseAppName(), U" cannot have more than one pause form at a time.");
		thePauseForm = UiForm_create (topShell, optionalPauseWindowOwningEditor, U"Pause: stop or continue",
			thePauseFormOkCallback, interpreter,   // pass interpreter as closure!
			nullptr, nullptr
		);
		UiForm_addComment (thePauseForm.get(), nullptr, text);
		thePauseForm_interpreterReference = interpreter;
		thePauseForm_savedEditorReference = interpreter -> optionalDynamicEnvironmentEditor();
		UiForm_setPauseForm (thePauseForm.get(), 1, 1, 0,
			U"Continue", nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
			thePauseFormCancelCallback
		);
		theCancelContinueButton = 0;
		UiForm_finish (thePauseForm.get());
		//if (theCurrentPraatApplication -> batch) goto end;
		thePauseForm_clicked = 0;
		Melder_assert (interpreter -> owningInterpreterStack);
		interpreter -> owningInterpreterStack -> haltAll ();
		/*
			Put the pause form on the screen.
		*/
		UiForm_destroyWhenUnmanaged (thePauseForm.get());
		UiForm_do (thePauseForm.get(), false);
	}
}

void UiPause_cleanUp () {
	#if defined (macintosh) || 1
		thePauseForm. reset();   // TODO: this works only if pause windows aren't children of editor windows
	#else
		thePauseForm. releaseToUser();   // BUG: potential memory leak (but it can be a child of an editor window)
	#endif
	thePauseForm_clicked = 0;
	theCancelContinueButton = 0;
	thePauseForm_interpreterReference = nullptr;
	//thePauseForm_secondPass = false;   // TODO: probably remove
	MelderFolder_setToNull (& thePauseForm_savedFolder);   // TODO: probably remove
	thePauseForm_savedEditorReference = nullptr;
}

/* End of file UiPause.cpp */
