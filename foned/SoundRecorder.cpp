/* SoundRecorder.cpp
 *
 * Copyright (C) 1992-2026 Paul Boersma
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
/* Linux code originally by Darryl Purnell, Pretoria */
/* GTK conversion includes work by Franz Brauße */

/* This source file describes interactive sound recorders for the following systems:
 *     MacOS
 *     Linux
 *     Windows
 * Because the behaviour of these sound recorders is partly similar, partly different,
 * this would seem a good candidate for object-oriented programming
 * (one audio manager and several audio drivers).
 * However, the places where sound recorders are similar and where they are different,
 * are hard to predict. For this reason, everything is done with platform #ifdefs.
 */

#include <errno.h>
#include "SoundRecorder.h"
#include "Sound_and_Spectrum.h"
#include "machine.h"
#include "EditorM.h"
#include "GuiP.h"
#if defined (macintosh)
	#include "pa_mac_core.h"
#endif

#include "enums_getText.h"
#include "SoundRecorder_enums.h"
#include "enums_getValue.h"
#include "SoundRecorder_enums.h"

Thing_implement (SoundRecorder, Editor, 0);

#include "Prefs_define.h"
#include "SoundRecorder_prefs.h"
#include "Prefs_install.h"
#include "SoundRecorder_prefs.h"
#include "Prefs_copyToInstance.h"
#include "SoundRecorder_prefs.h"

static struct {
	integer bufferSizeInMegabytes;
} preferences;

void SoundRecorder_preferences () {
	Preferences_addInteger (U"SoundRecorder.bufferSizeInMegabytes", & preferences.bufferSizeInMegabytes, 60);
}

integer SoundRecorder_getBufferSizePref_MB () {
	return preferences.bufferSizeInMegabytes;
}
void SoundRecorder_setBufferSizePref_MB (integer size) {
	preferences.bufferSizeInMegabytes = Melder_clipped (1_integer, size, 1000_integer);
}

#define step 1000

/* For those systems that do not have a pollable audio control panel, */
/* the settings are saved only here, so that they are remembered across */
/* subsequent creations of a SoundRecorder. Also, this is then the way */
/* in which two simultaneously open SoundRecorders would communicate. */

static struct {
	int inputSource;   // 1 = microphone, 2 = line, 3 = digital
	int leftGain, rightGain;   // 0..255
	double sampleRate;
} theControlPanel =
#if defined (linux)
	{ 1, 200, 200, 44100.0 };
#elif defined (macintosh)
	{ 1, 26, 26, 44100.0 };
#else
	{ 1, 26, 26, 44100.0 };
#endif

/********** ERROR HANDLING **********/

#if defined (_WIN32)
static void win_fillFormat (SoundRecorder me) {
	my waveFormat. nSamplesPerSec = (int) theControlPanel. sampleRate;
	my waveFormat. nChannels = my numberOfChannels;
	my waveFormat. wFormatTag = WAVE_FORMAT_PCM;
	my waveFormat. wBitsPerSample = 16;
	my waveFormat. nBlockAlign = my waveFormat. nChannels * my waveFormat. wBitsPerSample / 8;
	my waveFormat. nAvgBytesPerSec = my waveFormat. nBlockAlign * my waveFormat. nSamplesPerSec;
	my waveFormat. cbSize = 0;
}
static void win_fillHeader (SoundRecorder me, int which) {
	my waveHeader [which]. dwFlags = 0;
	my waveHeader [which]. lpData =
			( which == 0 ? (char *) my recordBuffer.asArgumentToFunctionThatExpectsZeroBasedArray() :
			  which == 1 ? (char *) my buffertje1: (char *) my buffertje2 );
	my waveHeader [which]. dwBufferLength =
			( which == 0 ? my nmax * my waveFormat. nChannels * 2 : 1000 * my waveFormat. nChannels * 2 );
	my waveHeader [which]. dwLoops = 0;
	my waveHeader [which]. lpNext = nullptr;
	my waveHeader [which]. reserved = 0;
}
static void win_waveInCheck (SoundRecorder me) {
	wchar_t messageText [MAXERRORLENGTH];
	MMRESULT err;
	if (my err == MMSYSERR_NOERROR) return;
	err = waveInGetErrorText (my err, messageText, MAXERRORLENGTH);
	if (err == MMSYSERR_NOERROR)
		Melder_throw (Melder_peekWto32 (messageText));
	else if (err == MMSYSERR_BADERRNUM)
		Melder_throw (U"Error number ", my err, U" out of range.");
	else if (err == MMSYSERR_NODRIVER)
		Melder_throw (U"No sound driver present.");
	else if (err == MMSYSERR_NOMEM)
		Melder_throw (U"Out of memory.");
	else
		Melder_throw (U"Unknown sound error.");
}
static void win_waveInOpen (SoundRecorder me) {
	try {
		my err = waveInOpen (& my hWaveIn, WAVE_MAPPER, & my waveFormat, 0, 0, CALLBACK_NULL);
		win_waveInCheck (me);
		if (! (Melder_debug == 8))
			waveInReset (my hWaveIn);
	} catch (MelderError) {
		Melder_throw (U"Audio input not opened.");
	}
}
static void win_waveInPrepareHeader (SoundRecorder me, int which) {
	try {
		my err = waveInPrepareHeader (my hWaveIn, & my waveHeader [which], sizeof (WAVEHDR));
		win_waveInCheck (me);
	} catch (MelderError) {
		Melder_throw (U"Audio input: cannot prepare header.\nQuit some other programs or go to \"Sound input prefs\" in the Preferences menu.");
	}
}
static void win_waveInAddBuffer (SoundRecorder me, int which) {
	try {
		my err = waveInAddBuffer (my hWaveIn, & my waveHeader [which], sizeof (WAVEHDR));
		win_waveInCheck (me);
	} catch (MelderError) {
		Melder_throw (U"Audio input: cannot add buffer.");
	}
}
static void win_waveInStart (SoundRecorder me) {
	try {
		my err = waveInStart (my hWaveIn);   // asynchronous
		win_waveInCheck (me);
	} catch (MelderError) {
		Melder_throw (U"Audio input not started.");
	}
}
static void win_waveInStop (SoundRecorder me) {
	try {
		my err = waveInStop (my hWaveIn);
		win_waveInCheck (me);
	} catch (MelderError) {
		Melder_throw (U"Audio input not stopped.");
	}
}
static void win_waveInReset (SoundRecorder me) {
	try {
		my err = waveInReset (my hWaveIn);
		win_waveInCheck (me);
	} catch (MelderError) {
		Melder_throw (U"Audio input not reset.");
	}
}
static void win_waveInUnprepareHeader (SoundRecorder me, int which) {
	try {
		my err = waveInUnprepareHeader (my hWaveIn, & my waveHeader [which], sizeof (WAVEHDR));
		win_waveInCheck (me);
	} catch (MelderError) {
		Melder_throw (U"Audio input: cannot unprepare header.");
	}
}
static void win_waveInClose (SoundRecorder me) {
	try {
		my err = waveInClose (my hWaveIn);
		my hWaveIn = 0;
		win_waveInCheck (me);
	} catch (MelderError) {
		Melder_throw (U"Audio input not closed.");
	}
}
#endif

static void closePortAudioStream (SoundRecorder me) {
	if (my portaudioStream) {
		try {
			Pa_StopStream (my portaudioStream);
		} catch (...) { }
		try {
			Pa_CloseStream (my portaudioStream);
		} catch (...) { }
		my portaudioStream = nullptr;
	}
}

static void stopRecording (SoundRecorder me) {	
	if (! my recording) return;
	try {
		my recording = false;
		if (! my synchronous) {
			if (my inputUsesPortAudio) {
				// Keep portaudioStream in hot standby mode for zero-latency restart
			} else {
				#if defined (_WIN32)
					MMTIME mmtime;
					mmtime. wType = TIME_BYTES;
					my nsamp = 0;
					if (waveInGetPosition (my hWaveIn, & mmtime, sizeof (MMTIME)) == MMSYSERR_NOERROR)
						my nsamp = mmtime. u.cb / (sizeof (short) * my numberOfChannels);
					win_waveInReset (me);
					if (my nsamp == 0)
						my nsamp = my waveHeader [0]. dwBytesRecorded / (sizeof (short) * my numberOfChannels);
					if (my nsamp > my nmax)
						my nsamp = my nmax;
					win_waveInUnprepareHeader (me, 0);
					win_waveInClose (me);
				#endif
			}
		}
	} catch (MelderError) {
		Melder_flushError (U"Cannot stop recording.");
	}
}

void structSoundRecorder :: v9_destroy () noexcept {
	our recording = false;
	MelderAudio_stopPlaying (MelderAudio_IMPLICIT);   // must also occur before freeing our buffer
	#if cocoa
		if (our d_cocoaTimer)
			CFRunLoopTimerInvalidate (our d_cocoaTimer);
	#elif gtk
		g_idle_remove_by_data (this);
	#elif motif
		if (our workProcId)
			XtRemoveWorkProc (our workProcId);
	#endif

	if (our inputUsesPortAudio) {
		closePortAudioStream (this);
	} else {
		#if defined (_WIN32)
			if (our hWaveIn != 0) {
				waveInReset (our hWaveIn);
				waveInUnprepareHeader (our hWaveIn, & our waveHeader [0], sizeof (WAVEHDR));
				waveInClose (our hWaveIn);
			}
		#elif defined (macintosh)
		#elif defined (UNIX)
			if (our fd != -1)
				close (our fd);
		#endif
	}
	our SoundRecorder_Parent :: v9_destroy ();
}

static void showMaximum (SoundRecorder me, int channel, double maximum) {
	maximum /= 32768.0;
	Graphics_setWindow (my graphics.get(),
		my numberOfChannels == 1 || channel == 1 ? 0.0 : -1.0,
		my numberOfChannels == 1 || channel == 2 ? 1.0 : 2.0,
		-0.1, 1.1);
	Graphics_setGrey (my graphics.get(), 0.9);
	Graphics_fillRectangle (my graphics.get(), 0.0, 1.0, maximum, 1.0);
	Graphics_setColour (my graphics.get(), Melder_GREEN);
	if (maximum < 0.75) {
		Graphics_fillRectangle (my graphics.get(), 0.0, 1.0, 0.0, maximum);
	} else {
		Graphics_fillRectangle (my graphics.get(), 0.0, 1.0, 0.0, 0.75);
		Graphics_setColour (my graphics.get(), Melder_YELLOW);
		if (maximum < 0.92) {
			Graphics_fillRectangle (my graphics.get(), 0.0, 1.0, 0.75, maximum);
		} else {
			Graphics_fillRectangle (my graphics.get(), 0.0, 1.0, 0.75, 0.92);
			Graphics_setColour (my graphics.get(), Melder_RED);
			Graphics_fillRectangle (my graphics.get(), 0.0, 1.0, 0.92, maximum);
		}
	}
}

static integer getSelectedTakeIndex (SoundRecorder me) {
	if (! my takeList || my recordedSounds.size == 0)
		return 0;
	autoINTVEC selected = GuiList_getSelectedPositions (my takeList);
	if (selected.size == 0)
		return 0;
	return selected [1];
}

static void showMeter (SoundRecorder me, const short *buffertje, integer nsamp) {
	Melder_assert (my graphics);
	Graphics_clearWs (my graphics.get());
	if (nsamp < 1) {
		integer sel = getSelectedTakeIndex (me);
		Sound sound = nullptr;
		if (sel >= 1 && sel <= my recordedSounds.size)
			sound = my recordedSounds.at [sel];

		if (sound) {
			double x1w, x2w, y1w, y2w;
			Graphics_inqWsWindow (my graphics.get(), & x1w, & x2w, & y1w, & y2w);
			Graphics_setViewport (my graphics.get(), x1w, x2w, y1w, y2w);
			Graphics_setWindow (my graphics.get(), 0.0, 1.0, 0.0, 1.0);
			Graphics_setColour (my graphics.get(), Melder_WHITE);
			Graphics_fillRectangle (my graphics.get(), 0.0, 1.0, 0.0, 1.0);
			Graphics_setColour (my graphics.get(), Melder_BLACK);

			const double totalDuration = sound -> xmax - sound -> xmin;
			const double ROW_DURATION = 10.0;
			int numRows = (int) ceil (totalDuration / ROW_DURATION);
			if (numRows < 1)
				numRows = 1;
			if (numRows > 3)
				numRows = 3;

			for (int row = 0; row < numRows; row ++) {
				double tmin = sound -> xmin + row * ROW_DURATION;
				double tmax = sound -> xmin + (row + 1) * ROW_DURATION;
				if (tmax > sound -> xmax)
					tmax = sound -> xmax;
				if (tmin < sound -> xmax) {
					double yBottom = y1w + (y2w - y1w) * (double)(numRows - 1 - row) / numRows;
					double yTop = y1w + (y2w - y1w) * (double)(numRows - row) / numRows;
					Graphics_setViewport (my graphics.get(), x1w, x2w, yBottom, yTop);
					Sound_draw (sound, my graphics.get(), tmin, tmax, 0.0, 0.0, true, U"curve");
				}
			}

			if (totalDuration > 30.0) {
				Graphics_setViewport (my graphics.get(), x1w, x2w, y1w, y2w);
				Graphics_setWindow (my graphics.get(), 0.0, 1.0, 0.0, 1.0);
				Graphics_setTextAlignment (my graphics.get(), Graphics_RIGHT, Graphics_TOP);
				Graphics_setColour (my graphics.get(), Melder_RED);
				Graphics_text (my graphics.get(), 0.98, 0.98, Melder_cat (U"(前30秒预览 / 总长 ", Melder_fixed (totalDuration, 1), U"s)"));
			}

			Graphics_setViewport (my graphics.get(), x1w, x2w, y1w, y2w);
			return;
		}

		Graphics_setWindow (my graphics.get(), 0.0, 1.0, 0.0, 1.0);
		#if defined (macintosh)
			Graphics_setColour (my graphics.get(), Melder_WHITE);
			Graphics_fillRectangle (my graphics.get(), 0.2, 0.8, 0.3, 0.7);
		#endif
		Graphics_setTextAlignment (my graphics.get(), Graphics_CENTRE, Graphics_HALF);
		Graphics_setColour (my graphics.get(), Melder_BLACK);
		Graphics_text (my graphics.get(), 0.5, 0.5, praat_translate (U"Not recording."));
		return;
	}
	if (my instancePref_meter_which() == kSoundRecorder_meter::INTENSITY) {
		int leftMaximum = 0, rightMaximum = 0;
		if (my numberOfChannels == 1) {
			for (integer i = 0; i < nsamp; i ++) {
				int value = buffertje [i];
				if (abs (value) > leftMaximum)
					leftMaximum = abs (value);
			}
		} else {
			for (integer i = 0; i < nsamp; i ++) {
				int left = buffertje [2 * i], right = buffertje [2 * i + 1];
				if (abs (left) > leftMaximum)
					leftMaximum = abs (left);
				if (abs (right) > rightMaximum)
					rightMaximum = abs (right);
			}
		}
		if (my lastLeftMaximum > 30000) {
			int leak = my lastLeftMaximum - Melder_ifloor (2000000.0 / theControlPanel. sampleRate);
			if (leftMaximum < leak)
				leftMaximum = leak;
		}
		showMaximum (me, 1, leftMaximum);
		my lastLeftMaximum = leftMaximum;
		if (my numberOfChannels == 2) {
			if (my lastRightMaximum > 30000) {
				int leak = my lastRightMaximum - Melder_ifloor (2000000.0 / theControlPanel. sampleRate);
				if (rightMaximum < leak)
					rightMaximum = leak;
			}
			showMaximum (me, 2, rightMaximum);
			my lastRightMaximum = rightMaximum;
		}
	} else if (my instancePref_meter_which() == kSoundRecorder_meter::CENTRE_OF_GRAVITY_VERSUS_INTENSITY) {
		autoSound sound = Sound_create (my numberOfChannels,
				0.0, nsamp / theControlPanel. sampleRate,
				nsamp, 1.0 / theControlPanel. sampleRate, 0.5 / theControlPanel. sampleRate);
		const short *p = & buffertje [0];
		for (integer isamp = 1; isamp <= nsamp; isamp ++)
			for (integer ichan = 1; ichan <= my numberOfChannels; ichan ++)
				sound -> z [ichan] [isamp] = * (p ++) / 32768.0;
		Sound_multiplyByWindow (sound.get(), kSound_windowShape::KAISER_2);
		const double intensity = Sound_getIntensity_dB (sound.get());
		autoSpectrum spectrum = Sound_to_Spectrum (sound.get(), true);
		const double centreOfGravity = Spectrum_getCentreOfGravity (spectrum.get(), 1.0);
		trace (nsamp, U" samples, intensity ", intensity, U" dB, centre of gravity ", centreOfGravity, U" Hz");
		Graphics_setWindow (my graphics.get(),
			my instancePref_meter_centreOfGravity_minimum(), my instancePref_meter_centreOfGravity_maximum(),
			my instancePref_meter_intensity_minimum(), my instancePref_meter_intensity_maximum()
		);
		Graphics_setColour (my graphics.get(), Melder_WHITE);
		Graphics_fillRectangle (my graphics.get(),
			my instancePref_meter_centreOfGravity_minimum(), my instancePref_meter_centreOfGravity_maximum(),
			my instancePref_meter_intensity_minimum(), my instancePref_meter_intensity_maximum()
		);
		Graphics_setColour (my graphics.get(), Melder_BLACK);
		Graphics_fillCircle_mm (my graphics.get(), centreOfGravity, intensity, 3.0);
	}
}

static bool tooManySamplesInBufferToReturnToGui (SoundRecorder me) {
	(void) me;
	return false;
}

static integer getMyNsamp (SoundRecorder me) {
	volatile integer nsamp = my nsamp;   // Prevent inlining.
	return nsamp;
}

#if cocoa
	#define WORKPROC_RETURN  void
	#define WORKPROC_ARGS  CFRunLoopTimerRef /*timer*/, void *void_SoundRecorder
#elif gtk
	#define WORKPROC_RETURN  gboolean
	#define WORKPROC_ARGS  void *void_SoundRecorder
#else
	#define WORKPROC_RETURN  bool
	#define WORKPROC_ARGS  void *void_SoundRecorder
#endif

static WORKPROC_RETURN workProc (WORKPROC_ARGS) {
	SoundRecorder me = static_cast <SoundRecorder> (void_SoundRecorder);
	try {
		short buffertje [step*2];
		int stepje = 0;

		/*
			Determine global audio parameters (may have been changed by an external control panel):
			1. input source;
			2. left and right gain;
			3. sampling frequency.
		*/
		if (my inputUsesPortAudio) {
		} else {
		}

		/*
			Set the buttons according to the audio parameters.
		*/
		bool hasTakes = (my recordedSounds.size > 0);
		if (my recordButton)
			GuiThing_setSensitive (my recordButton, ! my recording);
		if (my stopButton)
			GuiThing_setSensitive (my stopButton, my recording);
		if (my playButton)
			GuiThing_setSensitive (my playButton, ! my recording && my nsamp > 0);
		if (my playTakeButton)
			GuiThing_setSensitive (my playTakeButton, ! my recording && hasTakes);
		if (my renameTakeButton)
			GuiThing_setSensitive (my renameTakeButton, ! my recording && hasTakes);
		if (my deleteTakeButton)
			GuiThing_setSensitive (my deleteTakeButton, ! my recording && hasTakes);
		if (my publishAllButton)
			GuiThing_setSensitive (my publishAllButton, ! my recording && (hasTakes || my nsamp > 0));
		if (my applyButton)
			GuiThing_setSensitive (my applyButton, ! my recording && (hasTakes || my nsamp > 0));
		if (my okButton)
			GuiThing_setSensitive (my okButton, ! my recording && (hasTakes || my nsamp > 0));
		if (my monoButton && my numberOfChannels == 1)
			GuiRadioButton_set (my monoButton);
		if (my stereoButton && my numberOfChannels == 2)
			GuiRadioButton_set (my stereoButton);
		for (integer i = 1; i <= SoundRecorder_IFSAMP_MAX; i ++)
			if (my fsamps [i]. button && theControlPanel. sampleRate == my fsamps [i]. fsamp)
				GuiRadioButton_set (my fsamps [i]. button);
		if (my devices [theControlPanel. inputSource]. button)
			GuiRadioButton_set (my devices [theControlPanel. inputSource]. button);
		if (my monoButton)   GuiThing_setSensitive (my monoButton,   ! my recording);
		if (my stereoButton) GuiThing_setSensitive (my stereoButton, ! my recording);
		for (integer i = 1; i <= SoundRecorder_IFSAMP_MAX; i ++)
			if (my fsamps [i]. button)
				GuiThing_setSensitive (my fsamps [i]. button, ! my recording);
		for (integer i = 1; i <= SoundRecorder_IDEVICE_MAX; i ++)
			if (my devices [i]. button)
				GuiThing_setSensitive (my devices [i]. button, ! my recording);

		/*Graphics_setGrey (my graphics, 0.9);
		Graphics_fillRectangle (my graphics, 0.0, 1.0, 0.0, 32768.0);
		Graphics_setGrey (my graphics, 0.9);
		Graphics_fillRectangle (my graphics, 0.0, 1.0, 0.0, 32768.0);*/

		if (my synchronous) {
			/*
				Read some samples into 'buffertje'.
			*/
			do {
				if (my inputUsesPortAudio) {
					/*
						Asynchronous recording: do nothing.
					*/
				} else {
					#if defined (macintosh) || defined (_WIN32)
						/*
							Asynchronous recording on these systems: do nothing.
						*/
					#else
						// linux
						if (my fd != -1)
							stepje = read (my fd, (void *) buffertje, step * (sizeof (short) * my numberOfChannels)) / (sizeof (short) * my numberOfChannels);
					#endif
				}

				if (my recording)
					memcpy (& my recordBuffer [1 + my nsamp * my numberOfChannels], buffertje, stepje * (sizeof (short) * my numberOfChannels));
				//showMeter (me, buffertje, stepje);
				if (my recording) {
					my nsamp += stepje;
					if (my nsamp > my nmax - step)
						my recording = false;
					GuiScale_setValue (my progressScale, 1000.0 * ((double) my nsamp / (double) my nmax));
				}
			} while (my recording && tooManySamplesInBufferToReturnToGui (me));
		} else {
			if (my recording) {
				/*
					We have to know how far the buffer has been filled.
					However, the buffer may be filled at interrupt time,
					so that the buffer may be being filled during this workproc.
					So we ask for the buffer filling just once, namely here at the beginning.
				*/
				my lastSample = 0;
				if (my inputUsesPortAudio) {
					/*
						The buffer filling is contained in my nsamp,
						which has been set during interrupt time and may again be updated behind our backs during this workproc.
						So we do it in such a way that the compiler cannot ask for my nsamp twice.
					*/
					my lastSample = getMyNsamp (me);
					Pa_Sleep (10);
				} else {
					#if defined (_WIN32)
						MMTIME mmtime;
						mmtime. wType = TIME_BYTES;
						if (waveInGetPosition (my hWaveIn, & mmtime, sizeof (MMTIME)) == MMSYSERR_NOERROR)
							my lastSample = mmtime. u.cb / (sizeof (short) * my numberOfChannels);
					#elif defined (macintosh)
					#endif
				}
				my firstSample = my lastSample - 3000;
				Melder_clipLeft (0_integer, & my firstSample);
				GuiScale_setValue (my progressScale, 1000.0 * ((double) my lastSample / (double) my nmax));
				Graphics_updateWs (my graphics.get());
			} else {
				#if defined (_WIN32)
					if (my inputUsesPortAudio) {
						Pa_Sleep (10);
					}
				#endif
			}
		}
	} catch (MelderError) {
		Melder_flushError ();
	}
	#if cocoa
		return;
	#elif gtk
		return true;
	#else
		return false;
	#endif
}

static int portaudioStreamCallback (
    const void *input, void * /* output */,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* /* timeInfo */,
    PaStreamCallbackFlags /* statusFlags */,
    void *void_SoundRecorder)
{
	/*
		This procedure may be called at interrupt time.
		It therefore accesses only data that is constant during recording,
		namely me, my buffer, my numberOfChannels, and my nmax.
		The only thing it changes is my nsamp;
		the workProc will therefore have to take some care in accessing my nsamp (see there).
	*/
	SoundRecorder me = static_cast <SoundRecorder> (void_SoundRecorder);
	if (! my recording)
		return paContinue;

	if (Melder_debug == 20)
		Melder_casual (U"The PortAudio stream callback receives ", frameCount, U" frames.");
	Melder_assert (my nsamp <= my nmax);
	uinteger samplesLeft = my nmax - my nsamp;
	if (samplesLeft > 0) {
		uinteger dsamples = samplesLeft > frameCount ? frameCount : samplesLeft;
		if (Melder_debug == 20)
			Melder_casual (U"play ", dsamples, U" ", Pa_GetStreamCpuLoad (my portaudioStream));
		memcpy (& my recordBuffer [1 + my nsamp * my numberOfChannels], input, 2 * dsamples * my numberOfChannels);
		my nsamp += dsamples;
		if (my nsamp >= my nmax) {
			my recording = false;
			return paContinue;
		}
	} else {
		my nsamp = my nmax;
		my recording = false;
		return paContinue;
	}
	return paContinue;
}

static void ensurePortAudioStream (SoundRecorder me) {
	if (my portaudioStream)
		return;
	PaStreamParameters streamParameters = { };
	integer preferredDevIndex = MelderAudio_getInputDeviceIndex ();
	if (preferredDevIndex >= 0 && preferredDevIndex < Pa_GetDeviceCount ()) {
		streamParameters. device = (PaDeviceIndex) preferredDevIndex;
		const PaDeviceInfo *devInfo = Pa_GetDeviceInfo (streamParameters. device);
		if (devInfo)
			streamParameters. suggestedLatency = devInfo -> defaultLowInputLatency;
	} else if (theControlPanel. inputSource >= 1 && theControlPanel. inputSource <= my numberOfInputDevices) {
		streamParameters. device = my deviceIndices [theControlPanel. inputSource];
		streamParameters. suggestedLatency = my deviceInfos [theControlPanel. inputSource] -> defaultLowInputLatency;
	} else {
		streamParameters. device = Pa_GetDefaultInputDevice ();
		if (streamParameters. device != paNoDevice) {
			const PaDeviceInfo *devInfo = Pa_GetDeviceInfo (streamParameters. device);
			if (devInfo)
				streamParameters. suggestedLatency = devInfo -> defaultLowInputLatency;
		}
	}
	streamParameters. channelCount = my numberOfChannels;
	streamParameters. sampleFormat = paInt16;
	#if defined (macintosh)
		PaMacCoreStreamInfo macCoreStreamInfo = { };
		macCoreStreamInfo. size = sizeof (PaMacCoreStreamInfo);
		macCoreStreamInfo. hostApiType = paCoreAudio;
		macCoreStreamInfo. version = 0x01;
		macCoreStreamInfo. flags = paMacCoreChangeDeviceParameters | paMacCoreFailIfConversionRequired;
		streamParameters. hostApiSpecificStreamInfo = & macCoreStreamInfo;
	#endif
	if (Melder_debug == 20)
		Melder_casual (U"Before Pa_OpenStream");
	PaError err = Pa_OpenStream (& my portaudioStream, & streamParameters, nullptr,
		theControlPanel. sampleRate, 0, paNoFlag, portaudioStreamCallback, (void *) me);
	if (Melder_debug == 20)
		Melder_casual (U"Pa_OpenStream returns ", (int) err);
	if (err) {
		conststring32 errorText = Melder_peek8to32_u (Pa_GetErrorText (err));
		if (Melder_equ (errorText, U"Invalid number of channels")) {
			if (my numberOfChannels == 1)
				Melder_throw (U"You are trying to record in mono, but your microphone does not seem to support that.\nPerhaps you could try to record in stereo instead.");
			else
				Melder_throw (U"You are trying to record in stereo, but you do not seem to have a stereo microphone.\nPerhaps you could try to record in mono instead.");
		} else {
			Melder_throw (U"Error opening audio input stream: ", errorText, U".");
		}
	}
	err = Pa_StartStream (my portaudioStream);
	if (Melder_debug == 20)
		Melder_casual (U"Pa_StartStream returns ", (int) err);
	if (err)
		Melder_throw (U"Error starting audio input stream: ", Melder_peek8to32_u (Pa_GetErrorText (err)), U".");
}

static void startRecording (SoundRecorder me) {
	try {
		if (my recording)
			return;
		my nsamp = 0;
		my lastLeftMaximum = 0;
		my lastRightMaximum = 0;
		if (! my synchronous) {
			if (my inputUsesPortAudio) {
				ensurePortAudioStream (me);
				my recording = true;
			} else {
				#if defined (_WIN32)
					win_fillFormat (me);
					win_fillHeader (me, 0);
					win_waveInOpen (me);
					win_waveInPrepareHeader (me, 0);
					win_waveInAddBuffer (me, 0);
					win_waveInStart (me);
				#elif defined (macintosh)
				#endif
				my recording = true;
			}
		}
		Graphics_updateWs (my graphics.get());
	} catch (MelderError) {
		my recording = false;
		Graphics_updateWs (my graphics.get());
		Melder_flushError (U"The recording was not started.");
	}
}

static void gui_button_cb_record (SoundRecorder me, GuiButtonEvent /* event */) {
	startRecording (me);
}

static autoSound createRecordedSound (SoundRecorder me) {
	if (my nsamp <= 0)
		return autoSound();
	double fsamp = theControlPanel. sampleRate;
	if (fsamp <= 0.0)
		fsamp = 44100.0;   // safe
	autoSound sound;
	try {
		sound = Sound_createSimple (my numberOfChannels, (double) my nsamp / fsamp, fsamp);
	} catch (MelderError) {
		Melder_flushError (U"Could not create Sound object.");
		return autoSound();
	}
	if (my numberOfChannels == 1) {
		for (integer i = 1; i <= my nsamp; i ++)
			sound -> z [1] [i] = my recordBuffer [i] * (1.0 / 32768);
	} else {
		for (integer i = 1; i <= my nsamp; i ++) {
			sound -> z [1] [i] = my recordBuffer [i + i - 1] * (1.0 / 32768);
			sound -> z [2] [i] = my recordBuffer [i + i] * (1.0 / 32768);
		}
	}
	return sound;
}

static void addCurrentRecordingToTakes (SoundRecorder me) {
	if (my nsamp <= 0)
		return;
	autoSound sound = createRecordedSound (me);
	if (! sound)
		return;

	autostring32 baseName;
	if (my soundName)
		baseName = GuiText_getString (my soundName);
	if (! baseName || Melder_length (baseName.get()) == 0)
		baseName = Melder_dup (U"untitled");

	char32 takeName [300];
	Melder_sprint (takeName, 300, baseName.get(), U"_", my takeIndex ++);
	Thing_setName (sound.get(), takeName);

	my recordedSounds.addItem_move (sound.move());

	if (my takeList) {
		GuiList_insertItem (my takeList, takeName, my recordedSounds.size);
		GuiList_selectItem (my takeList, my recordedSounds.size);
	}
}

#if defined (_WIN32)
static WNDPROC g_originalHoldRecordWndProc = nullptr;
static LRESULT CALLBACK holdRecordWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	SoundRecorder me = (SoundRecorder) GetWindowLongPtr (hwnd, GWLP_USERDATA);
	if (me) {
		if (msg == WM_LBUTTONDOWN) {
			my isHoldingRecord = true;
			startRecording (me);
			if (my holdRecordButton)
				GuiButton_setText (my holdRecordButton, praat_translate (U"● Recording... (release to stop)"));
		} else if (msg == WM_LBUTTONUP || msg == WM_CAPTURECHANGED) {
			if (my isHoldingRecord) {
				my isHoldingRecord = false;
				stopRecording (me);
				addCurrentRecordingToTakes (me);
				if (my holdRecordButton)
					GuiButton_setText (my holdRecordButton, praat_translate (U"Hold to Record (release to stop)"));
				Graphics_updateWs (my graphics.get());
			}
		}
	}
	return CallWindowProc (g_originalHoldRecordWndProc, hwnd, msg, wParam, lParam);
}
#endif

static void gui_list_cb_takeSelectionChanged (SoundRecorder me, GuiList_SelectionChangedEvent /* event */) {
	integer index = getSelectedTakeIndex (me);
	if (index >= 1 && index <= my recordedSounds.size) {
		Sound sound = my recordedSounds.at [index];
		if (sound && my soundName)
			GuiText_setString (my soundName, Thing_getName (sound));
	}
	if (! my recording && my graphics) {
		showMeter (me, nullptr, 0);
		Graphics_updateWs (my graphics.get());
	}
}

static void gui_list_cb_takeDoubleClick (SoundRecorder me, GuiList_DoubleClickEvent /* event */) {
	integer index = getSelectedTakeIndex (me);
	if (index >= 1 && index <= my recordedSounds.size) {
		Sound sound = my recordedSounds.at [index];
		if (sound)
			Sound_play (sound, nullptr, nullptr);
	}
}

static void gui_button_cb_takePlay (SoundRecorder me, GuiButtonEvent /* event */) {
	integer index = getSelectedTakeIndex (me);
	if (index >= 1 && index <= my recordedSounds.size) {
		Sound sound = my recordedSounds.at [index];
		if (sound)
			Sound_play (sound, nullptr, nullptr);
	}
}

static void gui_button_cb_takeRename (SoundRecorder me, GuiButtonEvent /* event */) {
	integer index = getSelectedTakeIndex (me);
	if (index < 1 || index > my recordedSounds.size)
		return;
	Sound sound = my recordedSounds.at [index];
	if (! sound)
		return;

	const int diaWidth = 320, diaHeight = 140;
	GuiDialog dia = GuiDialog_create (my windowForm, 250, 180, diaWidth, diaHeight,
			U"Rename recorded take", nullptr, nullptr, GuiDialog_Modality::BLOCKING);
	GuiLabel_createShown (dia, 20, diaWidth - 20, 15, 35, U"New name:", 0);
	GuiText text = GuiText_createShown (dia, 20, diaWidth - 20, 40, 68, 0);
	GuiText_setString (text, Thing_getName (sound));

	int btnWidth = 90, btnY = 80;
	GuiButton_createShown (dia, diaWidth - 20 - btnWidth, diaWidth - 20, btnY, btnY + Gui_PUSHBUTTON_HEIGHT,
			U"OK", nullptr, nullptr, GuiButton_DEFAULT);
	GuiButton_createShown (dia, diaWidth - 30 - 2 * btnWidth, diaWidth - 30 - btnWidth, btnY, btnY + Gui_PUSHBUTTON_HEIGHT,
			U"Cancel", nullptr, nullptr, GuiButton_CANCEL);

	GuiThing_show (dia);
	const integer clicked = GuiDialog_run (dia);
	if (clicked == 1) {   // OK button
		autostring32 newName = GuiText_getString (text);
		if (newName && Melder_length (newName.get()) > 0) {
			Thing_setName (sound, newName.get());
			GuiList_replaceItem (my takeList, newName.get(), index);
			GuiList_selectItem (my takeList, index);
			if (my soundName)
				GuiText_setString (my soundName, newName.get());
		}
	}
	GuiThing_hide (dia);
	forget (dia);
}

static void gui_button_cb_takeDelete (SoundRecorder me, GuiButtonEvent /* event */) {
	integer index = getSelectedTakeIndex (me);
	if (index < 1 || index > my recordedSounds.size)
		return;
	my recordedSounds.removeItem (index);
	GuiList_deleteItem (my takeList, index);
	if (my recordedSounds.size > 0) {
		integer newSelect = index <= my recordedSounds.size ? index : my recordedSounds.size;
		GuiList_selectItem (my takeList, newSelect);
		Sound sound = my recordedSounds.at [newSelect];
		if (sound && my soundName)
			GuiText_setString (my soundName, Thing_getName (sound));
	}
	if (! my recording && my graphics) {
		showMeter (me, nullptr, 0);
		Graphics_updateWs (my graphics.get());
	}
}

static void gui_button_cb_publishAll (SoundRecorder me, GuiButtonEvent /* event */) {
	if (my recordedSounds.size == 0) {
		if (my nsamp > 0) {
			autoSound sound = createRecordedSound (me);
			if (sound) {
				if (my soundName) {
					autostring32 name = GuiText_getString (my soundName);
					Thing_setName (sound.get(), name.get());
				}
				Editor_broadcastPublication (me, sound.move());
			}
		}
		return;
	}
	for (integer i = 1; i <= my recordedSounds.size; i ++) {
		Sound sound = my recordedSounds.at [i];
		if (sound) {
			autoSound soundCopy = Data_copy (sound);
			Editor_broadcastPublication (me, soundCopy.move());
		}
	}
}

static void gui_button_cb_holdRecord (SoundRecorder /* me */, GuiButtonEvent /* event */) {
	// Push-to-talk is handled by window subclassing
}

static void gui_button_cb_stop (SoundRecorder me, GuiButtonEvent /* event */) {
	stopRecording (me);
	if (my isHoldingRecord) {
		my isHoldingRecord = false;
		if (my holdRecordButton)
			GuiButton_setText (my holdRecordButton, praat_translate (U"Hold to Record (release to stop)"));
	}
	addCurrentRecordingToTakes (me);
	Graphics_updateWs (my graphics.get());
}

static void gui_button_cb_play (SoundRecorder me, GuiButtonEvent /* event */) {
	if (my recording)
		return;
	integer index = getSelectedTakeIndex (me);
	if (index >= 1 && index <= my recordedSounds.size) {
		Sound sound = my recordedSounds.at [index];
		if (sound) {
			Sound_play (sound, nullptr, nullptr);
			return;
		}
	}
	if (my nsamp > 0) {
		MelderAudio_play16 (my recordBuffer.asArgumentToFunctionThatExpectsZeroBasedArray(),
				theControlPanel. sampleRate, my nsamp, my numberOfChannels, nullptr, nullptr);
	}
}

static void publish (SoundRecorder me) {
	if (my recordedSounds.size > 0) {
		integer sel = getSelectedTakeIndex (me);
		if (sel >= 1 && sel <= my recordedSounds.size) {
			Sound sound = my recordedSounds.at [sel];
			if (sound) {
				autoSound soundCopy = Data_copy (sound);
				Editor_broadcastPublication (me, soundCopy.move());
				return;
			}
		} else {
			for (integer i = 1; i <= my recordedSounds.size; i ++) {
				Sound sound = my recordedSounds.at [i];
				if (sound) {
					autoSound soundCopy = Data_copy (sound);
					Editor_broadcastPublication (me, soundCopy.move());
				}
			}
			return;
		}
	}
	if (my nsamp == 0)
		return;
	autoSound sound = createRecordedSound (me);
	if (! sound)
		return;
	if (my soundName) {
		autostring32 name = GuiText_getString (my soundName);
		Thing_setName (sound.get(), name.get());
	}
	Editor_broadcastPublication (me, sound.move());
}

static void gui_button_cb_cancel (SoundRecorder me, GuiButtonEvent /* event */) {
	stopRecording (me);
	forget (me);
}

static void gui_button_cb_apply (SoundRecorder me, GuiButtonEvent /* event */) {
	stopRecording (me);
	publish (me);
}

static void gui_button_cb_ok (SoundRecorder me, GuiButtonEvent /* event */) {
	stopRecording (me);
	if (my recordedSounds.size > 0) {
		for (integer i = 1; i <= my recordedSounds.size; i ++) {
			Sound sound = my recordedSounds.at [i];
			if (sound) {
				autoSound soundCopy = Data_copy (sound);
				Editor_broadcastPublication (me, soundCopy.move());
			}
		}
	} else if (my nsamp > 0) {
		publish (me);
	}
	forget (me);
}

static void initialize (SoundRecorder me) {
	try {
		if (my inputUsesPortAudio) {
			#if defined (macintoshXXX)
				my fsamps [SoundRecorder_IFSAMP_8000]. canDo = false;
				my fsamps [SoundRecorder_IFSAMP_11025]. canDo = false;
				my fsamps [SoundRecorder_IFSAMP_12000]. canDo = false;
				my fsamps [SoundRecorder_IFSAMP_16000]. canDo = false;
				my fsamps [SoundRecorder_IFSAMP_22050]. canDo = false;
				my fsamps [SoundRecorder_IFSAMP_24000]. canDo = false;
				my fsamps [SoundRecorder_IFSAMP_32000]. canDo = false;
				my fsamps [SoundRecorder_IFSAMP_64000]. canDo = false;
			#else
				// Accept all standard sample rates.
				(void) me;
			#endif
		} else {
			#if defined (macintosh)
			#elif defined (_WIN32)
				(void) me;
			#elif defined (linux) && ! defined (NO_AUDIO)
				int sampleRate = (int) theControlPanel. sampleRate, sampleSize = 16;
				int channels = my numberOfChannels, stereo = ( my numberOfChannels == 2 ), val;
				#if __BYTE_ORDER == __BIG_ENDIAN
					int format = AFMT_S16_BE;
				#else
					int format = AFMT_S16_LE;
				#endif
				int fd_mixer;
				my fd = open ("/dev/dsp", O_RDONLY);
				if (my fd == -1) {
					if (errno == EBUSY)
						Melder_throw (U"Audio device already in use.");
					else
						Melder_throw (U"Cannot open audio device.\n"
							U"Please switch on PortAudio in the Sound Recording Preferences.");
				}
				ioctl (my fd, SNDCTL_DSP_RESET, nullptr);
				ioctl (my fd, SNDCTL_DSP_SPEED, & sampleRate);
				ioctl (my fd, SNDCTL_DSP_SAMPLESIZE, & sampleSize);
				ioctl (my fd, SNDCTL_DSP_CHANNELS, (val = channels, & val));
				if (channels == 1 && val == 2) {
					close (my fd);
					Melder_throw (U"This sound card does not support mono.");
				}
				ioctl (my fd, SNDCTL_DSP_STEREO, & stereo);
				ioctl (my fd, SNDCTL_DSP_SETFMT, & format);
				fd_mixer = open ("/dev/mixer", O_WRONLY);		
				if (fd_mixer == -1) {
					Melder_throw (U"Cannot open /dev/mixer.");
				} else {
					int dev_mask = theControlPanel. inputSource == 2 ? SOUND_MASK_LINE : SOUND_MASK_MIC;
					if (ioctl (fd_mixer, SOUND_MIXER_WRITE_RECSRC, & dev_mask) == -1) {
						close (fd_mixer);
						Melder_throw (U"Can't set recording device in mixer.");
					}
					close (fd_mixer);
				}
			#endif
		}
	} catch (MelderError) {
		Melder_throw (U"16-bit audio recording not initialized.");
	}
}

static void gui_radiobutton_cb_input (SoundRecorder me, GuiRadioButtonEvent event) {
	Melder_casual (U"SoundRecorder:"
		U" setting the input source from ", theControlPanel. inputSource,
		U" to ", event -> position, U".");
	theControlPanel. inputSource = event -> position;

	/* Set system's input source. */
	if (my inputUsesPortAudio) {
		closePortAudioStream (me);
	} else {
		#if defined (_WIN32)
			// deferred to the start of recording
		#elif defined (macintosh)
			//SPBCloseDevice (my refNum);
			try {
				initialize (me);
			} catch (MelderError) {
				Melder_flushError ();
			}
		#elif defined (linux) && ! defined (NO_AUDIO)
			int fd_mixer = open ("/dev/mixer", O_WRONLY);		
			if (fd_mixer == -1)
				Melder_flushError (U"(Sound_record:) Cannot open /dev/mixer.");
			int dev_mask = theControlPanel.inputSource == 2 ? SOUND_MASK_LINE : SOUND_MASK_MIC;
			if (ioctl (fd_mixer, SOUND_MIXER_WRITE_RECSRC, & dev_mask) == -1)
				Melder_flushError (U"(Sound_record:) Can't set recording device in mixer");
			close (fd_mixer);
		#endif
	}
}

static void gui_radiobutton_cb_fsamp (SoundRecorder me, GuiRadioButtonEvent event) {
	if (my recording)
		return;
	try {
		double fsamp = undefined;
		for (integer i = 1; i <= SoundRecorder_IFSAMP_MAX; i ++)
			if (event -> toggle == my fsamps [i]. button)
				fsamp = my fsamps [i]. fsamp;
		Melder_assert (isdefined (fsamp));
		/*
			If we push the 48000 button while the sampling frequency is 22050,
			we first get a message that the 22050 button has changed,
			and then we get a message that the 48000 button has changed.
			So the following will work (it used to be different with old Motif versions on Linux):
		*/
		Melder_casual (U"SoundRecorder:"
				U" setting the sample rate from ", (integer) theControlPanel. sampleRate,
				U" to ", (integer) fsamp, U" Hz.");
		if (fsamp == theControlPanel. sampleRate)
			return;
		/*
			Now we know, hopefully, that the message is from the button that was clicked,
			not the one that was unset by the radio box, so we can take action.
		*/
		theControlPanel. sampleRate = fsamp;
		/*
			Set the system's sampling frequency.
			On some systems, we cannot do this without closing the audio device,
			and reopening it with a new sampling frequency.
		*/
		if (my inputUsesPortAudio) {
			closePortAudioStream (me);
		} else {
			#if defined (_WIN32)
				// deferred to the start of recording
			#elif defined (macintosh)
				//SPBCloseDevice (my refNum);
				initialize (me);
			#elif defined (linux) && ! defined (NO_AUDIO)
				close (my fd);
				initialize (me);
			#endif
		}
	} catch (MelderError) {
		Melder_throw (U"Sampling frequency not changed.");
	}
}

static void gui_drawingarea_cb_expose (SoundRecorder me, GuiDrawingArea_ExposeEvent event) {
	if (! my graphics)
		return;   // could be the case in the very beginning
	if (my recording)
		showMeter (me, & my recordBuffer [1 + my firstSample * my numberOfChannels], my lastSample - my firstSample);
	else
		showMeter (me, nullptr, 0);
}

static void gui_drawingarea_cb_resize (SoundRecorder me, GuiDrawingArea_ResizeEvent event) {
	if (! my graphics)
		return;   // could be the case in the very beginning
	Graphics_setWsViewport (my graphics.get(), 0, event -> width, 0, event -> height);
	Graphics_setWsWindow (my graphics.get(), 0, event -> width, 0, event -> height);
	Graphics_setViewport (my graphics.get(), 0, event -> width, 0, event -> height);
	Graphics_updateWs (my graphics.get());
}

void structSoundRecorder :: v_createChildren ()
{
	integer y = Machine_getMenuBarBottom () + 15;

	/*
		Takes list (Left region: x: 10 ~ 280).
	*/
	GuiLabel_createShown (our windowForm, 10, 280, y, y + Gui_LABEL_HEIGHT, U"Recorded takes:", 0);
	our takeList = GuiList_createShown (our windowForm, 10, 280, y + Gui_LABEL_HEIGHT + 2, -215, false, U" Takes ");
	GuiList_setSelectionChangedCallback (our takeList, gui_list_cb_takeSelectionChanged, this);
	GuiList_setDoubleClickCallback (our takeList, gui_list_cb_takeDoubleClick, this);

	// 2x2 Action buttons below take list (Row 1: y = -205 ~ -180; Row 2: y = -175 ~ -150)
	our playTakeButton = GuiButton_createShown (our windowForm, 10, 140, -205, -180,
			U"Play", gui_button_cb_takePlay, this, 0);
	our renameTakeButton = GuiButton_createShown (our windowForm, 150, 280, -205, -180,
			U"Rename...", gui_button_cb_takeRename, this, 0);
	our deleteTakeButton = GuiButton_createShown (our windowForm, 10, 140, -175, -150,
			U"Delete", gui_button_cb_takeDelete, this, 0);
	our publishAllButton = GuiButton_createShown (our windowForm, 150, 280, -175, -150,
			U"Save all", gui_button_cb_publishAll, this, 0);

	/*
		Meter box (Center: x: 290 ~ -170).
	*/
	GuiLabel_createShown (our windowForm, 290, -170, y, y + Gui_LABEL_HEIGHT, U"Meter", GuiLabel_CENTRE);
	our meter = GuiDrawingArea_createShown (our windowForm, 290, -170, y + Gui_LABEL_HEIGHT + 2, -145,
		gui_drawingarea_cb_expose, nullptr,
		nullptr, gui_drawingarea_cb_resize, nullptr, this, GuiDrawingArea_BORDER
	);

	/*
		Sampling frequency (Right: x: -160 ~ -10).
	*/
	GuiLabel_createShown (our windowForm, -160, -10, y, y + Gui_LABEL_HEIGHT, U"Sampling frequency:", 0);
	GuiRadioGroup_begin ();
	for (integer i = 1; i <= SoundRecorder_IFSAMP_MAX; i ++) {
		if (our fsamps [i]. canDo) {
			double fsamp = our fsamps [i]. fsamp;
			y += Gui_RADIOBUTTON_HEIGHT + Gui_RADIOBUTTON_SPACING;
			our fsamps [i]. button = GuiRadioButton_createShown (our windowForm,
				-150, -10, y, y + Gui_RADIOBUTTON_HEIGHT,
				Melder_cat (fsamp == Melder_roundDown (fsamp) ? Melder_integer ((integer) fsamp) : Melder_fixed (fsamp, 5), U" Hz"),
				gui_radiobutton_cb_fsamp, this, fsamp == theControlPanel. sampleRate ? GuiRadioButton_SET : 0
			);
		}
	}
	GuiRadioGroup_end ();

	GuiLabel_createShown (our windowForm, 10, 130, -128, -108, U"Buffer usage:", 0);
	our progressScale = GuiScale_createShown (our windowForm,
			135, -10, -126, -110,
			0, 1000, 0, 0);

	/*
		Control buttons row (y = 65).
	*/
	y = 65;
	our recordButton = GuiButton_createShown (our windowForm, 10, 75, -y - Gui_PUSHBUTTON_HEIGHT, -y,
			U"Record", gui_button_cb_record, this, 0);
	our holdRecordButton = GuiButton_createShown (our windowForm, 80, 260, -y - Gui_PUSHBUTTON_HEIGHT, -y,
			U"Hold to Record (release to stop)", gui_button_cb_holdRecord, this, 0);
	#if defined (_WIN32)
		if (our holdRecordButton && our holdRecordButton -> d_widget) {
			HWND hwnd = our holdRecordButton -> d_widget -> window;
			if (hwnd) {
				SetWindowLongPtr (hwnd, GWLP_USERDATA, (LONG_PTR) this);
				g_originalHoldRecordWndProc = (WNDPROC) SetWindowLongPtr (hwnd, GWLP_WNDPROC, (LONG_PTR) holdRecordWndProc);
			}
		}
	#endif
	our stopButton = GuiButton_createShown (our windowForm, 265, 330, -y - Gui_PUSHBUTTON_HEIGHT, -y,
			U"Stop", gui_button_cb_stop, this, 0);
	if (inputUsesPortAudio) {
		our playButton = GuiButton_createShown (our windowForm, 335, 400, -y - Gui_PUSHBUTTON_HEIGHT, -y,
				U"Play", gui_button_cb_play, this, 0);
	} else {
		#if defined (_WIN32) || defined (macintosh)
			our playButton = GuiButton_createShown (our windowForm, 335, 400, -y - Gui_PUSHBUTTON_HEIGHT, -y,
					U"Play", gui_button_cb_play, this, 0);
		#endif
	}
	
	GuiLabel_createShown (our windowForm, -240, -170, -y - 2 - Gui_TEXTFIELD_HEIGHT, -y - 2, U"Name:", GuiLabel_RIGHT);
	our soundName = GuiText_createShown (our windowForm, -160, -10, -y - 2 - Gui_TEXTFIELD_HEIGHT, -y - 2, 0);
	GuiText_setString (our soundName, U"untitled");

	/*
		Bottom row (y = 20).
	*/
	y = 20;
	our cancelButton = GuiButton_createShown (our windowForm, -410, -330, -y - Gui_PUSHBUTTON_HEIGHT, -y,
			U"Close", gui_button_cb_cancel, this, 0);
	our applyButton = GuiButton_createShown (our windowForm, -320, -185, -y - Gui_PUSHBUTTON_HEIGHT, -y,
			U"Save to list", gui_button_cb_apply, this, GuiButton_DEFAULT);
	our okButton = GuiButton_createShown (our windowForm, -175, -10, -y - Gui_PUSHBUTTON_HEIGHT, -y,
			U"Save to list & Close", gui_button_cb_ok, this, 0);
}

static void writeAudioFile (SoundRecorder me, MelderFile file, int audioFileType) {
	try {
		MelderFile_writeAudioFile (file, audioFileType, my recordBuffer.asArgumentToFunctionThatExpectsZeroBasedArray(),
				Melder_iround (theControlPanel. sampleRate), my nsamp, my numberOfChannels, 16);
	} catch (MelderError) {
		Melder_throw (U"Audio file not written.");
	}
}

static void menu_cb_writeWav (SoundRecorder me, EDITOR_ARGS) {
	EDITOR_FORM_SAVE (U"Save as WAV file", nullptr)
		autostring32 name = GuiText_getString (my soundName);
		Melder_sprint (defaultName,300, name.get(), U".wav");
	EDITOR_DO_SAVE
		writeAudioFile (me, file, Melder_WAV);
	EDITOR_END
}

static void menu_cb_writeAifc (SoundRecorder me, EDITOR_ARGS) {
	EDITOR_FORM_SAVE (U"Save as AIFC file", nullptr)
		autostring32 name = GuiText_getString (my soundName);
		Melder_sprint (defaultName,300, name.get(), U".aifc");
	EDITOR_DO_SAVE
		writeAudioFile (me, file, Melder_AIFC);
	EDITOR_END
}

static void menu_cb_writeNextSun (SoundRecorder me, EDITOR_ARGS) {
	EDITOR_FORM_SAVE (U"Save as NeXT/Sun file", nullptr)
		autostring32 name = GuiText_getString (my soundName);
		Melder_sprint (defaultName,300, name.get(), U".au");
	EDITOR_DO_SAVE
		writeAudioFile (me, file, Melder_NEXT_SUN);
	EDITOR_END
}

static void menu_cb_writeNist (SoundRecorder me, EDITOR_ARGS) {
	EDITOR_FORM_SAVE (U"Save as NIST file", nullptr)
		autostring32 name = GuiText_getString (my soundName);
		Melder_sprint (defaultName,300, name.get(), U".nist");
	EDITOR_DO_SAVE
		writeAudioFile (me, file, Melder_NIST);
	EDITOR_END
}

static void updateMenus (SoundRecorder me) {
	GuiMenuItem_check (my meterIntensityButton,
			my instancePref_meter_which() == kSoundRecorder_meter::INTENSITY);
	GuiMenuItem_check (my meterCentreOfGravityVersusIntensityButton,
			my instancePref_meter_which() == kSoundRecorder_meter::CENTRE_OF_GRAVITY_VERSUS_INTENSITY);
}

static void menu_cb_intensity (SoundRecorder me, EDITOR_ARGS) {
	my setInstancePref_meter_which (kSoundRecorder_meter::INTENSITY);
	updateMenus (me);
}
static void menu_cb_centreOfGravityVersusIntensity (SoundRecorder me, EDITOR_ARGS) {
	my setInstancePref_meter_which (kSoundRecorder_meter::CENTRE_OF_GRAVITY_VERSUS_INTENSITY);
	updateMenus (me);
}

static void menu_cb_SoundRecorder_help (SoundRecorder, EDITOR_ARGS) { Melder_help (U"SoundRecorder"); }

void structSoundRecorder :: v_createMenus () {
	SoundRecorder_Parent :: v_createMenus ();
	Editor_addCommand (this, U"File", U"Save as WAV file...", 0, menu_cb_writeWav);
	Editor_addCommand (this, U"File", U"Save as AIFC file...", 0, menu_cb_writeAifc);
	Editor_addCommand (this, U"File", U"Save as NeXT/Sun file...", 0, menu_cb_writeNextSun);
	Editor_addCommand (this, U"File", U"Save as NIST file...", 0, menu_cb_writeNist);
	Editor_addCommand (this, U"File", U"-- write --", 0, nullptr);
	Editor_addMenu (this, U"Meter", 0);
	our meterIntensityButton =
		Editor_addCommand (this, U"Meter", U"Intensity", GuiMenu_RADIO_FIRST, menu_cb_intensity);
	our meterCentreOfGravityVersusIntensityButton =
		Editor_addCommand (this, U"Meter", U"Centre of gravity ~ intensity", GuiMenu_RADIO_NEXT, menu_cb_centreOfGravityVersusIntensity);
}

void structSoundRecorder :: v_createMenuItems_help (EditorMenu menu) {
	SoundRecorder_Parent :: v_createMenuItems_help (menu);
	EditorMenu_addCommand (menu, U"SoundRecorder help", '?', menu_cb_SoundRecorder_help);
}

autoSoundRecorder SoundRecorder_create (int numberOfChannels) {
	try {
		autoSoundRecorder me = Thing_new (SoundRecorder);
		my inputUsesPortAudio =
			#if defined (_WIN32)
				MelderAudio_getInputSoundSystem () == kMelder_inputSoundSystem::MME_VIA_PORTAUDIO;
			#elif defined (macintosh)
				MelderAudio_getInputSoundSystem () == kMelder_inputSoundSystem::COREAUDIO_VIA_PORTAUDIO;
			#elif defined (raspberrypi)
				MelderAudio_getInputSoundSystem () == kMelder_inputSoundSystem::JACK_VIA_PORTAUDIO;
			#else
				MelderAudio_getInputSoundSystem () == kMelder_inputSoundSystem::ALSA_OR_JACK_VIA_PORTAUDIO;
			#endif

		if (my inputUsesPortAudio) {
		} else {
			#if defined (_WIN32)
				UINT numberOfDevices = waveInGetNumDevs ();
				if (numberOfDevices == 0)
					Melder_throw (U"No sound input devices available.");
				WAVEINCAPS caps;
				MMRESULT err = waveInGetDevCaps (WAVE_MAPPER, & caps, sizeof (WAVEINCAPS));
				if (numberOfChannels == 2 && caps. wChannels < 2)
					Melder_throw (U"Your computer does not support stereo sound input.");
				/* BUG: should we ask whether 16 bit is supported? */
				for (UINT i = 0; i < numberOfDevices; i ++) {
					waveInGetDevCaps (i, & caps, sizeof (WAVEINCAPS));
					/*Melder_casual (U"Name of device ", i, U": ", Melder_peek16to32 (aps. szPname));*/
				}
			#elif defined (macintosh)
				SInt32 soundFeatures;
				if (Gestalt (gestaltSoundAttr, & soundFeatures) ||
						! (soundFeatures & (1 << gestaltSoundIOMgrPresent)) ||
						! (soundFeatures & (1 << gestaltBuiltInSoundInput)) ||
						! (soundFeatures & (1 << gestaltHasSoundInputDevice)))
					Melder_throw (U"Your computer does not support sound input.");
				if (! (soundFeatures & (1 << gestalt16BitSoundIO)) ||   // hardware
					! (soundFeatures & (1 << gestaltStereoInput)) ||   // hardware
					! (soundFeatures & (1 << gestalt16BitAudioSupport)))   // software
					Melder_throw (U"Your computer does not support stereo sound input.");
			#endif
		}
		my numberOfChannels = numberOfChannels;
		if (sizeof (short) != 2)
			Melder_throw (U"Long shorts!!!!!");
		if (my inputUsesPortAudio) {
			my synchronous = false;
		} else {
			#if defined (macintosh) || defined (_WIN32)
				my synchronous = false;
			#else
				my synchronous = true;
			#endif
		}
		/*
			Allocate the maximum buffer.
		*/
		Melder_clip (1_integer, & preferences.bufferSizeInMegabytes, 1000_integer);
		if (NUMisEmpty (my recordBuffer.get())) {
			integer nmax_bytes_pref = preferences.bufferSizeInMegabytes * 1000000;
			integer nmax_bytes = ( my inputUsesPortAudio ? nmax_bytes_pref :
				#if defined (_WIN32)
					66150000 );   // the maximum physical buffer on Windows XP; shorter than in Windows 98, alas.
				#else
					nmax_bytes_pref );
				#endif
			my nmax = nmax_bytes / (sizeof (short) * numberOfChannels);
			for (;;) {
				try {
					my recordBuffer = newvectorzero <short> (my nmax * numberOfChannels);
					break;   // success
				} catch (MelderError) {
					if (my nmax < 100000) {
						throw;   // failure, with error message
					} else {
						Melder_clearError ();
						my nmax /= 2;   // retry with less application memory
					}
				}
			}
		}

		/*
			Count the number of input devices and sources.
		*/
		if (my inputUsesPortAudio) {
			if (! MelderAudio_hasBeenInitialized) {
				PaError err = Pa_Initialize ();
				if (Melder_debug == 20)
					Melder_casual (U"init ", Melder_peek8to32_u (Pa_GetErrorText (err)));
				MelderAudio_hasBeenInitialized = true;
				if (Melder_debug == 20) {
					PaHostApiIndex hostApiCount = Pa_GetHostApiCount ();
					Melder_casual (U"host API count ", hostApiCount);
					for (PaHostApiIndex iHostApi = 0; iHostApi < hostApiCount; iHostApi ++) {
						const PaHostApiInfo *hostApiInfo = Pa_GetHostApiInfo (iHostApi);
						PaHostApiTypeId type = hostApiInfo -> type;
						Melder_casual (U"host API ", iHostApi, U": ", type, U", \"", Melder_peek8to32_u (hostApiInfo -> name), U"\" ", hostApiInfo -> deviceCount);
					}
					PaHostApiIndex defaultHostApi = Pa_GetDefaultHostApi ();
					Melder_casual (U"default host API ", defaultHostApi);
					PaDeviceIndex deviceCount = Pa_GetDeviceCount ();
					Melder_casual (U"device count ", deviceCount);
				}
			}
			PaDeviceIndex deviceCount = Pa_GetDeviceCount ();
			for (PaDeviceIndex idevice = 0; idevice < deviceCount; idevice ++) {
				const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo (idevice);
				if (Melder_debug == 20)
					Melder_casual (
						U"Device \"", Melder_peek8to32_u (deviceInfo -> name),
						U"\", input ", deviceInfo -> maxInputChannels,
						U", output ", deviceInfo -> maxOutputChannels,
						U", sample rate ", deviceInfo -> defaultSampleRate
					);
				if (deviceInfo -> maxInputChannels > 0 && my numberOfInputDevices < SoundRecorder_IDEVICE_MAX) {
					my devices [++ my numberOfInputDevices]. canDo = true;
					#if defined (macintosh) || defined (_WIN32)   // only one host API
						str32ncpy (my devices [my numberOfInputDevices]. fullName, Melder_peek8to32_u (deviceInfo -> name), SoundRecorder_Device :: maximumLengthOfFullName);
					#else
						const PaHostApiIndex hostApiIndex = deviceInfo -> hostApi;
						const PaHostApiInfo * const hostApiInfo = Pa_GetHostApiInfo (hostApiIndex);
						Melder_assert (hostApiInfo);
						const conststring8 shortHostApiName8 = ( strstr (hostApiInfo -> name, "JACK") == hostApiInfo -> name ? "JACK" : hostApiInfo -> name );
						str32ncpy (my devices [my numberOfInputDevices]. fullName,
							Melder_cat (U"(", Melder_peek8to32_u (shortHostApiName8), U":) ", Melder_peek8to32_u (deviceInfo -> name)),
							SoundRecorder_Device :: maximumLengthOfFullName
						);
					#endif
					my devices [my numberOfInputDevices]. fullName [SoundRecorder_Device :: maximumLengthOfFullName] = U'\0';
					my deviceInfos [my numberOfInputDevices] = deviceInfo;
					my deviceIndices [my numberOfInputDevices] = idevice;
				}
			}
			if (my numberOfInputDevices == 0)
				Melder_throw (U"No input devices available.");
			PaDeviceIndex prefDevice = (PaDeviceIndex) MelderAudio_getInputDeviceIndex ();
			if (prefDevice != paNoDevice) {
				for (integer i = 1; i <= my numberOfInputDevices; i ++) {
					if (my deviceIndices [i] == prefDevice) {
						theControlPanel. inputSource = i;
						break;
					}
				}
			}
		} else {
			#if defined (macintosh)
			#elif defined (_WIN32)
				// No device info: use Windows mixer.
			#else
				my devices [1]. canDo = true;
				str32cpy (my devices [1]. fullName, U"Microphone");
				my devices [2]. canDo = true;
				str32cpy (my devices [2]. fullName, U"Line");
			#endif
		}

		/*
			Sampling frequency constants.
		*/
		my fsamps [SoundRecorder_IFSAMP_8000]. fsamp = 8000.0;
		my fsamps [SoundRecorder_IFSAMP_9800]. fsamp = 9800.0;
		my fsamps [SoundRecorder_IFSAMP_11025]. fsamp = 11025.0;
		my fsamps [SoundRecorder_IFSAMP_12000]. fsamp = 12000.0;
		my fsamps [SoundRecorder_IFSAMP_16000]. fsamp = 16000.0;
		my fsamps [SoundRecorder_IFSAMP_22050]. fsamp = 22050.0;
		my fsamps [SoundRecorder_IFSAMP_22254]. fsamp = 22254.54545;
		my fsamps [SoundRecorder_IFSAMP_24000]. fsamp = 24000.0;
		my fsamps [SoundRecorder_IFSAMP_32000]. fsamp = 32000.0;
		my fsamps [SoundRecorder_IFSAMP_44100]. fsamp = 44100.0;
		my fsamps [SoundRecorder_IFSAMP_48000]. fsamp = 48000.0;
		my fsamps [SoundRecorder_IFSAMP_64000]. fsamp = 64000.0;
		my fsamps [SoundRecorder_IFSAMP_96000]. fsamp = 96000.0;
		my fsamps [SoundRecorder_IFSAMP_192000]. fsamp = 192000.0;

		/*
			The default set of possible sampling frequencies, to be modified in the initialize () procedure.
		*/
		for (integer i = 1; i <= SoundRecorder_IFSAMP_MAX; i ++)
			my fsamps [i]. canDo = true;   // optimistic: can do all, except two:
		my fsamps [SoundRecorder_IFSAMP_9800]. canDo = false;   // sgi only
		my fsamps [SoundRecorder_IFSAMP_22254]. canDo = false;   // old Mac only

		/*
			Initialize system-dependent structures.
			On all systems: stereo 16-bit linear encoding.
			Some systems take initial values from the system control panel
			(automatically in the workProc), other systems from theControlPanel.
		*/
		initialize (me.get());

		autoMelderString title;
		MelderString_copy (& title, U"SoundRecorder (", ( numberOfChannels == 1 ? U"mono" : U"stereo" ), U")");
		my takeIndex = 1;
		my isHoldingRecord = false;
		Editor_init (me.get(), 100, 100, 840, 620, title.string, nullptr);
		my graphics = Graphics_create_xmdrawingarea (my meter);
		Melder_assert (my graphics);

struct structGuiDrawingArea_ResizeEvent event { my meter, 0 };
event. width  = GuiControl_getWidth  (my meter);
event. height = GuiControl_getHeight (my meter);
gui_drawingarea_cb_resize (me.get(), & event);

		#if cocoa
			CFRunLoopTimerContext context = { 0, me.get(), nullptr, nullptr, nullptr };
			my d_cocoaTimer = CFRunLoopTimerCreate (nullptr, CFAbsoluteTimeGetCurrent () + 0.02,
					0.02, 0, 0, workProc, & context);
			CFRunLoopAddTimer (CFRunLoopGetCurrent (), my d_cocoaTimer, kCFRunLoopCommonModes);
		#elif gtk
			g_idle_add (workProc, me.get());
		#elif motif
			my workProcId = XtAddWorkProc (workProc, me.get());
		#endif
		updateMenus (me.get());
		if (my inputUsesPortAudio) {
			try {
				ensurePortAudioStream (me.get());
			} catch (MelderError) {
				Melder_clearError ();
			}
		}
		return me;
	} catch (MelderError) {
		Melder_throw (U"SoundRecorder not created.");
	}
}

/* End of file SoundRecorder.cpp */
