/* manual_SpeechRecognizer.cpp
 *
 * Copyright (C) 2026 Anastasia Shchupak
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

void manual_SpeechRecognizer_init (ManPages me);
void manual_SpeechRecognizer_init (ManPages me) {

MAN_PAGES_BEGIN R"~~~(
################################################################################
"Speech recognition"
© Anastasia Shchupak 2026-06-01

There are two speech recognition tools available in Praat: automatic transcription (turning speech
into text) and automatic speaker diarization (finding out who speaks when). This tutorial
describes how you can use these tools in Praat. It assumes that you are familiar with the @Intro,
especially with @@Intro 7. Annotation@.

Transcription is performed with @@whisper.cpp@ and it requires at least one external model
installed on your computer; diarization is performed with an adapted @@pyannote.audio@
diarization pipeline and it works without any external models. Diarization can also be performed
together with transcription, so that transcribed text is divided between speakers.

After you have read the chapters ##1. Automatic transcription# and ##2. Automatic speaker
diarization#, you might be interested in what can be done to improve the performance of these
tools in Praat; you can find this information in ##3. Performance#.

1. Automatic transcription
==========================
Transcription in Praat turns speech in a @Sound object into written text. Usually you might want
this text to be placed into a @TextGrid, aligning it with the Sound (this is described in ##1.2.
Transcribing into a TextGrid#). But you can also obtain it as plain text (see ##1.3. Transcribing
into plain text#). Either way, you first need a Whisper model installed, and how to do that is
explained in ##1.1. Installing Whisper models#.

1.1. Installing Whisper models
==============================
To transcribe, you need at least one Whisper model installed. In this case, installation
means downloading a model and placing it into a specific folder on your computer, where Praat
can later find it. In your Praat @@preferences folder@, create a folder called `models`, and
inside it another one called `whispercpp`. If you are on Windows, you will now have a folder called
something like `C:\Users\Your Name\Praat\models\whispercpp`.

Models can be downloaded from `https://huggingface.co/ggerganov/whisper.cpp`. This
page contains Whisper models in ggml format (files with extension `.bin`), which is what whisper
.cpp uses. Note that the original Whisper models, by OpenAI, are distributed in PyTorch format
(files with extension `.pt`) and cannot be used in Praat directly: they first need to be
converted to ggml format. The files on the page above are results of such conversions.

The list of available models is quite long, and you might wonder how to choose one. You will
probably want to experiment with different models to find a good balance between speed and
accuracy for your specific task, but below you can read a brief overview to help you get
started. If you want to learn more about the performance of different models, please read ##3.
Performance#.

A model is English-only if its name contains `.en` (e.g. `ggml-small.en.bin`); all other models are
multilingual. Size-wise, the models range from `tiny` (about 75 MB) through `base`, `small` and
`medium` to `large` (about 2.9 GB). The large models have three different versions: `large-v1`,
`large-v2` and `large-v3`. The later versions are improvements over the earlier ones. Generally
speaking, larger models tend to be more accurate but use more disk space and memory, and take
longer to transcribe. So, `base` or `small` can be good starting points.

Models whose names end in `-q5_0`, `-q5_1` or `-q8_0` are %quantized: their weights are stored with
fewer bits (5 or 8 instead of 16), so they take less disk space and memory, and run faster than the
normal (non-quantized) models with the corresponding names. There is also `large-v3-turbo`,
a reduced version of `large-v3` that has fewer decoder layers: it is about half the size and thus
faster than the original `large-v3`. These optimized model variants were created to offer better
speed at the cost of reduced accuracy.

Once you have decided which model you want, download its `.bin` file and place it in your newly
created `whispercpp` folder. Any `.bin` file placed in this folder will be found by Praat.
In fact, if you want to experiment with different models, you can download
as many of them as you like; Praat will let you select one before you start transcription.

1.2. Transcribing into a TextGrid
=================================
You need a @Sound and a @TextGrid for this Sound (transcription modifies an existing TextGrid;
it does not create one). The TextGrid should have at least one interval tier: transcription is
run on one selected interval, so you need an interval tier to contain this interval. To
transcribe the whole Sound, you can use an interval tier without internal boundaries and thus
consisting of only one interval spanning the whole Sound (interval 1).

There are two ways to start transcription into a TextGrid:
1. from the @TextGridEditor;
2. from the @@Objects window@, when you select the TextGrid and the Sound together and choose
@@TextGrid & Sound: Transcribe interval...@ from the @@Dynamic menu@.

These two ways achieve exactly the same result, and which one you use is a matter of preference.
They differ only in their interface. In the TextGridEditor, you adjust the settings via the separate
command ##Transcription settings...# (see below) and select the interval by clicking on it. Via
##TextGrid & Sound: Transcribe interval...#, all the settings appear in this command window, which
also has two extra settings: ##Tier number# and ##Interval number#.

To perform transcription from the TextGridEditor, you need to open the TextGrid together with the
Sound (by selecting both the TextGrid and the Sound before clicking ##View & Edit#). Then the
#Interval menu has two actions related to transcription: ##Transcribe interval# and
##Transcription settings...#.

Before running transcription, you should check and adapt the ##Transcription settings...#. These
settings are preserved across transcription runs and across Praat sessions, so you can skip
this step later when you want to reuse the settings from the last time you transcribed.

Two main settings for transcription are ##Whisper model# and #Language:
1. ##Whisper model# lists all the models you installed in ##1.1. Installing Whisper models#; now is
the time to choose the model that you want to use. If you find yourself at this step with an empty
model list, this means something went wrong at the installation step. You can go back
to ##1.1. Installing Whisper models# and check the folder names and that the downloaded models are
there. After that, reopen ##Transcription settings...#; your installed model(s) will appear in
the list.
2. #Language is the language you want to recognize; you can select it from the list of all
available languages or keep the default ##Autodetect language# (note that for an English-only model
you are not allowed to select a language other than #English or the default ##Autodetect
language#).

This tutorial does not describe all transcription settings, because they are described in
@@transcription with whisper.cpp@. However, two of them, ##Include words# and ##Include
diarization#, influence how the TextGrid is modified, so they are explained below.

The structure of the resulting TextGrid depends on the combination of ##Include words# and ##Include
diarization# settings. There are four possible combinations of these two settings. Let’s assume
that the interval selected for transcription belongs to tier %Mary and walk through all
four combinations.

1. If both settings are off: the interval is split into many intervals, one interval per sentence,
each of them containing the text of the sentence (sentence boundaries come from the punctuation
that whisper.cpp returns). No new tiers are inserted.

2. If only ##Include words# is on: the original interval is split as in case 1, plus a tier called
%%Mary/word% is added just below %Mary, with one interval per word.

3. If only ##Include diarization# is on: %Mary is renamed to %%Mary/sp1%, and if diarization
detected more than one speaker, then more tiers are added so that there are as many tiers as
detected speakers: %%Mary/sp2%, %%Mary/sp3%, and so on, one tier per detected speaker. Each
speaker’s tier contains the intervals with the sentences or parts of sentences spoken by that
speaker. It is possible that a sentence is not completely spoken by one speaker. In this case the
 sentence is shown in pieces. For example, speaker 1 started this sentence, but halfway through it,
speaker 2 took over and finished it. Then the interval containing the first part of the sentence on
%%Mary/sp1% will end with “...”, and the interval containing the end of the sentence on
%%Mary/sp2% will start with “...”, so you can see it is the same sentence split between two
speakers.

4. If both settings are on: in addition to a sentence tier, each speaker also gets a word tier
(%%Mary/sp1/w% for speaker 1, %%Mary/sp2/w% for speaker 2, ...), which is inserted directly after
their sentence tier.

With the transcription settings configured, you are now ready to start the transcription. Select
the interval you want to transcribe and choose ##Transcribe interval# from the #Interval menu.
This runs the transcription, which takes some time; how long it takes depends on the length of
the selected interval, the Whisper model, and whether diarization is included. Note that you
cannot transcribe an interval belonging to a tier whose name already contains a slash. This
is to prevent endlessly growing names of the derived tiers.

1.3. Transcribing into plain text
=================================
You can also transcribe a whole @Sound into plain text. For this you need a @SpeechRecognizer
object. You can create one by choosing @@Create SpeechRecognizer...@ from the @@New menu@
in the @@Objects window@. A command window will appear containing two settings: ##Whisper model#
and #Language, both of which work exactly as in ##1.2. Transcribing into a TextGrid#. After you
click #OK, you will find a new #SpeechRecognizer object in the object list.

To transcribe, select the SpeechRecognizer object and the Sound object together and choose
##SpeechRecognizer & Sound: Transcribe# from the @@Dynamic menu@. The result of transcription will
be written to the @@Info window@. Note that the ##Allow silences# setting is not available here:
@@speech activity detection with Silero VAD|Silero VAD@ is always on, with default settings.

2. Automatic speaker diarization
================================
Speaker diarization in Praat detects different speakers in a @Sound. For each detected speaker, it
produces %%speech segments%, which are time intervals during which this speaker is active.
Diarization can be done as part of transcription or standalone. In both cases, it modifies an
existing @TextGrid, producing one interval tier per detected speaker, with that speaker’s speech
segments as intervals.

Speaker diarization as part of transcription is described in ##1.2. Transcribing into a TextGrid#.
The rest of this chapter is about standalone diarization.

As with transcription, to perform diarization, you need a @Sound and a @TextGrid for this
Sound (diarization modifies an existing TextGrid; it does not create one). The TextGrid should
have at least one interval tier: diarization is run on one selected interval, so you need an
interval tier to contain this interval. To diarize the whole Sound, you can use an interval
tier without internal boundaries and thus consisting of only one interval spanning the whole
Sound (interval 1).

There are two ways to start diarization:
1. from the @TextGridEditor;
2. from the @@Objects window@, when you select the TextGrid and the Sound together and choose
@@TextGrid & Sound: Diarize interval...@ from the @@Dynamic menu@.

These two ways achieve exactly the same result, and which one you use is a matter of preference.
They differ only in their interface. In the TextGridEditor, you adjust the settings via the separate
command ##Diarization settings...# (see below) and select the interval by clicking on it. Via
##TextGrid & Sound: Diarize interval...#, all the settings appear in this command window, which
also has two extra settings: ##Tier number# and ##Interval number#.

To perform diarization from the TextGridEditor, you need to open the TextGrid together with the
Sound (by selecting both the TextGrid and the Sound before clicking ##View & Edit#). Then the
#Interval menu has two actions related to diarization: ##Diarize interval# and
##Diarization settings...#.

Before running diarization, you should check and adapt the ##Diarization settings...#. These
settings are preserved across diarization runs and across Praat sessions, so you can skip
this step later when you want to reuse the settings from the last time you diarized.

This tutorial does not describe the diarization settings, because they are described in
@@speaker diarization with adapted pyannote.audio@.

With the diarization settings configured, you are now ready to start the diarization. Select
the interval you want to diarize and choose ##Diarize interval# from the #Interval menu.
This runs the diarization, which takes some time; how long it takes depends on the length of
the selected interval. Note that you cannot run diarization on an interval belonging to a tier
whose name already contains a slash. This is to prevent endlessly growing names of the derived
tiers.

The resulting TextGrid has the following structure. Let’s assume that the interval selected for
diarization belongs to tier %Mary. The tier %Mary is renamed to %%Mary/sp1%. Additional speakers
get new tiers %%Mary/sp2%, %%Mary/sp3%, etc. Each speaker’s tier contains alternating intervals:
non-speech and speech intervals labelled with the configured settings ##Non-speech interval
label# and ##Speech interval label# respectively.

3. Performance
==============


################################################################################
"speech activity detection with Silero VAD"
© Anastasia Shchupak 2026-06-01

Praat uses the @@whisper.cpp@ implementation of the @@Silero VAD@ speech activity detector.
The pre-trained Silero VAD model weights have been converted to ggml format and compiled into Praat,
so no external model files are required. The sound is automatically resampled to 16 kHz
(the sampling frequency expected by the Silero VAD model) before being processed by the model.

Purpose
=======
to detect which parts of a sound contain speech. The output is a list of speech segments,
each defined by a start and an end time.

Algorithm
=========
The Silero VAD model processes the sound in fixed frames of 512 samples (32 ms, since the sound
is resampled to 16 kHz). For each frame, it outputs a probability that the frame contains speech.
Based on this output, the list of speech segments is constructed as described below. This
description reflects the whisper.cpp implementation, which differs slightly from Silero’s
original implementation.

A speech segment begins at the first frame whose speech probability exceeds the ##Speech probability
threshold#. The segment continues as long as the speech probability stays above a lower threshold
(called the “negative threshold” in the Silero source code, equal to the speech probability
threshold minus 0.15). The segment ends only when the probability stays below the negative
threshold for at least ##Min. gap between speech segments#. When a segment ends, it is kept only
if it is longer than ##Min. speech segment#.

After all segments have been formed, segments separated by a gap shorter than 0.2 s are merged
(this is hardcoded in whisper.cpp and not configurable). After that, padding is applied: each
segment is extended on both sides by ##Padding around speech segments#. If padding would cause
two segments to overlap, they instead meet at the midpoint of the gap between them.

The result is a list of speech segments.

Settings
========
##Speech probability threshold (0-1)
:   determines the sensitivity of the speech detector. Higher values make the detector less
	sensitive, meaning that a frame requires a higher speech probability to be considered part of
	a speech segment. This reduces false positives (non-speech incorrectly classified as speech),
	but may cause some speech to be missed. Lower values make the detector more sensitive. The
	default of 0.5 works well for most use cases.

##Min. gap between speech segments (s)
:   the minimum duration of a gap between two speech segments. You might want to increase this value
	if short silences within speech (e.g. plosive closures) are splitting speech into multiple
	segments. Notranscriptionte that gaps shorter than 0.2 s are removed from the output (with their adjacent
	speech segments merged), regardless of this setting.

##Min. speech segment (s)
:   the minimum duration of a speech segment. Shorter segments are discarded.

##Padding around speech segments (s)
:   extends each detected speech segment by this amount on both sides. You might want to increase
	this value if speech onsets and offsets are being clipped.

Availability in Praat
=====================
Silero VAD speech activity detection is available in Praat:
- as part of transcription, running just before it to remove non-speech regions from the
  analysed sound (see @@transcription with whisper.cpp@);
- standalone, producing a new @TextGrid with non-speech and speech intervals (see @@Sound: To
  TextGrid (speech activity, Silero)...@).

################################################################################
"Sound: To TextGrid (speech activity, Silero)..."
© Anastasia Shchupak 2026-06-01

A command that creates a @TextGrid with one interval tier from every selected @Sound object.
The interval tier contains non-speech and speech intervals with boundaries determined by the
Silero VAD model (for the algorithm and settings, see @@speech activity detection with Silero VAD@).
The labels of the intervals are specified by the settings ##Non-speech interval label# and ##Speech
interval label#.

Settings
========
##Speech probability threshold (0-1)#

##Min. gap between speech segments (s)#

##Min. speech segment (s)#

##Padding around speech segments (s)
:   see @@speech activity detection with Silero VAD@.

,

##Non-speech interval label
:	the label assigned to intervals classified as non-speech in the resulting TextGrid.

##Speech interval label
:	the label assigned to intervals classified as speech in the resulting TextGrid.

################################################################################
"transcription with whisper.cpp"
© Anastasia Shchupak 2026-06-01

Praat can perform automatic transcription of a sound using @@whisper.cpp@. To do this, at least
one Whisper model must be installed on your computer. You can find the details about how to
install models and how to use transcription in the @@Speech recognition@ tutorial. The sound is
automatically resampled to 16 kHz (the sampling frequency expected by whisper.cpp) before being
transcribed. This page documents the transcription settings.

Behaviour
=========
When transcription is run on a @TextGrid interval, it modifies the TextGrid: intervals are split,
tiers may be added or renamed. The exact resulting TextGrid structure depends on the combination
of the ##Include words# and ##Include diarization# settings. See the @@Speech recognition@ tutorial
for the details of the transcription output under different combinations of these settings.

Settings
========
##Whisper model
:	determines which Whisper model is used.
	The list is populated with the `.bin` files found in the `whispercpp` subfolder of the
	`models` folder in the Praat preferences folder. See the @@Speech recognition@ tutorial for
	details on how to install models.

##Language
:	determines the language to be used for transcription.
	Choose ##Autodetect language# to let the model detect the language automatically. If you know
	the language you want to use for transcription, selecting it explicitly may improve
	transcription accuracy. Note that English-only models (those with ##.en# in the name) can only
	be used with ##Autodetect language# or ##English#.

##Include words
:	if on, each transcribed word is given a start and an end time, computed using whisper.cpp’s
	internal dynamic time warping (DTW) algorithm.

##Include diarization
:	if on, speaker diarization is run alongside transcription (see @@speaker diarization with
	adapted pyannote.audio@). The results of both are later combined to attribute portions of
	transcribed speech to different speakers.

##Allow silences
:	if on, @@speech activity detection with Silero VAD@ runs before transcription to identify
	speech regions. Only those regions are then passed to the Whisper model. This generally
	improves both speed and accuracy of transcription. Speed is improved by reducing the length of
	the sound sent to the model, and accuracy by preventing the model from hallucinating text for
	silent regions.

,

##Speech probability threshold (0-1)#

##Min. gap between speech segments (s)#

##Min. speech segment (s)#

##Padding around speech segments (s)
:   see @@speech activity detection with Silero VAD@.

,

##Fixed number of speakers...#

##... or range of numbers of speakers#

##Allow speakers to overlap#

##Clustering threshold (0-2)#

##Segmentation step (0-1)
:   see @@speaker diarization with adapted pyannote.audio@.

Availability in Praat
=====================
Transcription with whisper.cpp is available in two ways in Praat:
- if you select a @Sound together with its @TextGrid and choose
  @@TextGrid & Sound: Transcribe interval...|Transcribe interval...@;
- via ##Transcribe interval# from the #Interval menu in the @TextGridEditor.
  The settings for this command are set via ##Transcription settings...# in the same menu
  and are remembered across Praat sessions.

################################################################################
"TextGrid & Sound: Transcribe interval..."
© Anastasia Shchupak 2026-06-01

This command takes a specified interval of the @TextGrid, transcribes the corresponding part
of the @Sound, and writes the result back into the TextGrid.

Settings
========
##Tier number
:	the number of the tier containing the interval to be transcribed.

##Interval number
:	the number of the interval to be transcribed.

The remaining settings in this dialog control the transcription itself; see @@transcription with
whisper.cpp@ for their meaning.

################################################################################
"speaker diarization with adapted pyannote.audio"
© Anastasia Shchupak 2026-06-01

Praat performs speaker diarization using a C++/ggml adaptation of @@pyannote.audio@’s
`pyannote/speaker-diarization-3.1` pipeline. The two neural models used by the pipeline,
`pyannote/segmentation-3.0` (for segmentation) and `wespeaker-voxceleb-resnet34-LM` (for
speaker embedding, see @@WeSpeaker@), have been converted to ggml format and compiled into Praat,
so no external model files are required. The sound is automatically resampled to 16 kHz
(the sampling frequency expected by both models) before being processed.

Speaker diarization in Praat always modifies an existing TextGrid, producing one tier per
detected speaker. Diarization can be done as part of transcription (so that transcribed text is
split between speaker tiers) or standalone (so that each speaker tier contains the intervals
labelled as non-speech or speech). See @@Speech recognition@ tutorial for details on how to use it.
Its settings and algorithm are described below.

Purpose
=======
to detect which parts of a sound contain speech and to attribute each part to one or more
speakers. The output is a list of segments, each segment defined by a start time, an end time and
a speaker identifier. Speaker identifiers are natural numbers (1, 2, 3, ...); they are arbitrary
labels that roughly follow the order in which the speakers first appear.

Settings
========
##Fixed number of speakers...
:   if set to a positive integer, the algorithm aims to produce exactly this many speakers. Takes
	precedence over the ##... or range of numbers of speakers# setting below. This is a target,
	not a guarantee: if the sound does not have that many distinct voices, fewer speakers may be
	produced (and a warning is shown).

##... or range of numbers of speakers
:   a minimum and a maximum for the number of speakers the algorithm may produce. You can set only
	the minimum, only the maximum, or both; a value of 0 for any of the bounds leaves that bound
	unconstrained. Note that if ##Fixed number of speakers...# is set, this setting will be ignored.

##Allow speakers to overlap
:   if on, two speakers may be active at the same moment (not more than two though, as this is the
	limit of the segmentation model). If off, every moment is attributed to a single speaker, the
	one most active at that moment, according to the segmentation model.

##Clustering threshold (0-2)
:	if two groups at stage 3 of the algorithm (see below) are closer than this threshold, then they
	are merged. Lower values produce more speakers, higher values produce fewer speakers. When the
	number of speakers is constrained by ##Fixed number of speakers...# or by ##... or range of
	numbers of speakers#, this threshold no longer determines how many speakers are found
	(however, it still influences the process of group forming).

##Segmentation step (0-1)
:   the spacing between consecutive overlapping 10-second chunks, as a fraction of the chunk length
	(see stage 1 of the algorithm). A value of 1.0 means no overlap; lower values mean more
	overlap. Less overlap is faster (but generally less accurate), so this is the setting to
	lower when you want to speed diarization up.

Algorithm
=========
The algorithm has four stages. It is a port of @@pyannote.audio@s `pyannote/speaker-diarization-3.1`
pipeline (see @@Bredin (2023)@) with some adaptations.

##1. Segmentation#. The sound is divided into overlapping 10-second chunks. The distance between
the starts of two consecutive chunks is a fraction of the chunk length defined by ##Segmentation
step (0-1)#. For example, a segmentation step of 0.1 makes this distance 1 second, so that
consecutive chunks have a 90\%  overlap.

Each chunk is then sent to the segmentation model, which divides it into 589 frames (each frame
spanning about 0.017 s) and assigns a label to each frame in the following way. The model assumes
that there are at most three speakers in a chunk, of whom at most two can be active in the same
frame. For every frame, it gives a probability to each of seven possible combinations of active
speakers: {∅}, {1}, {2}, {3}, {1, 2}, {1, 3}, {2, 3}. The most probable combination is taken as
that frame’s label. The speaker numbers 1, 2 and 3 are local to each chunk: speaker 1 in one chunk
is not necessarily the same person as speaker 1 in another. Linking these local speakers across
chunks is the task of the remaining stages.

##2. Speaker embeddings#. For each chunk, every speaker who actually speaks in it is turned into
an %embedding (a 256-dimensional vector) by the embedding model. The model takes as input an
audio piece assembled from the parts of the chunk where the speaker is active, and outputs that
vector. The model works in such a way that two embeddings from the same speaker (from different
chunks) tend to be closer to each other than two embeddings from two different speakers. This
property of the model makes the next stage possible.

##3. Clustering#. The previous stage produces one embedding for each active speaker in each chunk.
Of these, only the ones backed by enough non-overlapping speech are treated as “reliable”; the
rest are set aside for now.

The reliable embeddings from all chunks are L2-normalized and grouped by similarity using
agglomerative hierarchical clustering: the two most similar groups are merged repeatedly,
stopping when the closest remaining groups are further apart than ##Clustering threshold (0-2)#.
Similarity between two groups is measured by the distance between their centres (more similar =
less distance). Because the embeddings are L2-normalized, the distance between any two of
them lies between 0 (identical) and 2 (opposite).

Each final group is one speaker; the number of groups can be influenced by ##Fixed number of
speakers...# and ##... or range of numbers of speakers# settings. Finally, the unreliable
embeddings (those not used to form the groups) are attached to their nearest groups. In this way,
each chunk-local speaker is assigned to a global speaker.

##4. Reconstruction#. Each chunk consists of frames, and because the chunks overlap, each frame
on the global timeline is covered by several chunks. Such a frame receives, for every speaker,
the average of the activations the covering chunks gave that speaker (using the global speaker
numbers from stage 3). It is then assigned to the speaker with the highest average activation
(or, where overlap is allowed and two people are talking, to the two highest). Finally, for each
speaker, every run of consecutive frames in which that speaker is active becomes one %segment. The
result is a list of segments, each attributed to one speaker.

Availability in Praat
=====================
For diarization as part of transcription, see @@transcription with whisper.cpp@.

Standalone diarization is available in two ways:
- if you select a @Sound together with its @TextGrid and choose
  @@TextGrid & Sound: Diarize interval...|Diarize interval...@;
- via ##Diarize interval# from the #Interval menu in the @TextGridEditor. The settings for this
  command are set via ##Diarization settings...# in the same menu and are remembered across Praat
  sessions.

################################################################################
"TextGrid & Sound: Diarize interval..."
© Anastasia Shchupak 2026-06-01

This command takes a specified interval of the @TextGrid, runs diarization on the corresponding part
of the @Sound, and writes the result back into the TextGrid.

Settings
========
##Tier number
:	the number of the tier containing the interval to be diarized.

##Interval number
:	the number of the interval to be diarized.

The remaining settings in this dialog control the speaker diarization itself;
see @@speaker diarization with adapted pyannote.audio@ for their meaning.

################################################################################
"SpeechRecognizer"
© Anastasia Shchupak 2026-03-15

One of the @@types of objects@ in Praat. It performs @@transcription with whisper.cpp@ on a
@Sound object. If you are new to speech recognition in Praat, see the @@Speech recognition@
tutorial first.

Commands
========

Creation:
,	@@Create SpeechRecognizer...@

Transcription:
,	@@SpeechRecognizer & Sound: Transcribe@

################################################################################
"Create SpeechRecognizer..."
© Anastasia Shchupak 2026-03-15

Creates the @SpeechRecognizer object.
If you are new to speech recognition in Praat, see the @@Speech recognition@ tutorial first.

Settings
========
##Whisper model
##Language
:	for both settings see @@transcription with whisper.cpp@.

################################################################################
"SpeechRecognizer & Sound: Transcribe"
© Anastasia Shchupak 2026-03-15

@@transcription with whisper.cpp|Transcribes@ selected @Sound using selected @SpeechRecognizer
object and writes the result of transcription to the @@Info window@.

The sound is automatically resampled to 16 kHz (the sampling frequency expected by @@whisper.cpp@)
before being processed.

The transcription uses the @@speech activity detection with Silero VAD@ built into @@whisper.cpp@
to skip non-speech parts of the sound, which improves both speed and accuracy.

The result is a flat text string containing the full transcription.

################################################################################
"Silero VAD"
© Anastasia Shchupak 2026-06-01

Silero VAD is a pre-trained speech activity detector created by Silero Team; see
@@Silero Team (2024)@ for the model and documentation. VAD stands for “voice activity detection”,
but we use “speech activity detection” instead, because Silero VAD detects unvoiced parts of speech
as well as voiced ones. The Silero VAD model is compiled into Praat (see @Acknowledgments).

For how speech activity detection with Silero VAD is used in Praat, see the @@Speech recognition@
tutorial. For the algorithm and settings, see @@speech activity detection with Silero VAD@.

################################################################################
"whisper.cpp"
© Anastasia Shchupak 2026-06-01

Whisper is an automatic speech recognition (ASR) system by OpenAI for transcribing speech
into text; see @@Radford et al. (2022)@ for the model architecture, training and evaluation.
OpenAI’s Whisper implementation is in Python (PyTorch).

Praat uses whisper.cpp, a lightweight C/C++ port of Whisper built on top of the ggml tensor library
for machine learning, developed by Georgi Gerganov and many other contributors (see
@Acknowledgments). The original OpenAI Whisper models must be converted to ggml format for use with
whisper.cpp.

For how transcription is used in Praat, see the @@Speech recognition@ tutorial.
For the transcription settings, see @@transcription with whisper.cpp@.

################################################################################
"pyannote.audio"
© Anastasia Shchupak 2026-06-01

pyannote.audio is an automatic speaker diarization toolkit developed by Hervé Bredin and
collaborators (see @@Plaquet & Bredin (2023)@ and @@Bredin (2023)@).

Praat contains a C++/ggml adaptation of its `pyannote/speaker-diarization-3.1` pipeline.
The pipeline uses two neural models: `pyannote/segmentation-3.0` by pyannote.audio for segmentation
and `wespeaker-voxceleb-resnet34-LM` by @@WeSpeaker@ for speaker embedding.
The `pyannote/segmentation-3.0` weights have been converted to ggml format
and embedded into Praat (see @Acknowledgments).

For how speaker diarization is used in Praat, see the @@Speech recognition@ tutorial.
For the diarization settings, see @@speaker diarization with adapted pyannote.audio@.

################################################################################
"WeSpeaker"
© Anastasia Shchupak 2026-06-01

WeSpeaker is a speaker embedding learning toolkit developed by WeNet Community
(see @@Wang et al. (2023)@ and @@Wang et al. (2024)@). Praat uses one of its pretrained models,
the `wespeaker-voxceleb-resnet34-LM` embedding model, as part of @@pyannote.audio@’s
`pyannote/speaker-diarization-3.1` pipeline.

`wespeaker-voxceleb-resnet34-LM` was trained on the VoxCeleb2 dataset
(see @@Chung, Nagrani & Zisserman (2018)@ and @@Nagrani, Chung & Zisserman (2017)@).
The model weights have been converted to ggml format and embedded into Praat (see @Acknowledgments).
Praat contains a C++/ggml port of WeSpeaker’s ResNet34 architecture with TSTP pooling,
used for inference on this model.

################################################################################
"Silero Team (2024)"
© Anastasia Shchupak 2026-06-01

Silero Team (2024). Silero VAD: pre-trained enterprise-grade voice activity detector (VAD),
number detector and language classifier [Computer software]. Version 6.2.0.

Available on `https://github.com/snakers4/silero-vad`.

################################################################################
"Radford et al. (2022)"
© Anastasia Shchupak 2026-06-01

Alec Radford, Jong Wook Kim, Tao Xu, Greg Brockman, Christine McLeavey & Ilya Sutskever (2022):
“Robust speech recognition via large-scale weak supervision.”

Available on `https://cdn.openai.com/papers/whisper.pdf`.

################################################################################
"Plaquet & Bredin (2023)"
© Anastasia Shchupak 2026-06-01

Alexis Plaquet & Hervé Bredin (2023): “Powerset multi-class cross entropy loss for neural
speaker diarization.” %%Proc. Interspeech 2023%.

Available on `https://www.isca-archive.org/interspeech_2023/plaquet23_interspeech.html`.

################################################################################
"Bredin (2023)"
© Anastasia Shchupak 2026-06-01

Hervé Bredin (2023): “pyannote.audio 2.1 speaker diarization pipeline: principle, benchmark,
and recipe.” %%Proc. Interspeech 2023%.

Available on `https://www.isca-archive.org/interspeech_2023/bredin23_interspeech.html`.

################################################################################
"Wang et al. (2023)"
© Anastasia Shchupak 2026-06-01

Hongji Wang, Chengdong Liang, Shuai Wang, Zhengyang Chen, Binbin Zhang, Xu Xiang, Yanlei Deng
& Yanmin Qian (2023): “Wespeaker: a research and production oriented speaker embedding learning
toolkit.” %%ICASSP 2023 - 2023 IEEE International Conference on Acoustics, Speech and Signal
Processing (ICASSP)%, 1–5.

Available on `https://doi.org/10.48550/arXiv.2210.17016`.

################################################################################
"Wang et al. (2024)"
© Anastasia Shchupak 2026-06-01

Shuai Wang, Zhengyang Chen, Bing Han, Hongji Wang, Chengdong Liang, Binbin Zhang, Xu Xiang,
Wen Ding, Johan Rohdin, Anna Silnova, Yanmin Qian & Haizhou Li (2024): “Advancing speaker
embedding learning: Wespeaker toolkit for research and production.” %%Speech Communication%
##162#: 103104.

Available on `https://doi.org/10.1016/j.specom.2024.103104`.

################################################################################
"Chung, Nagrani & Zisserman (2018)"
© Anastasia Shchupak 2026-06-01

Joon Son Chung, Arsha Nagrani & Andrew Zisserman (2018): “VoxCeleb2: deep speaker recognition.”
%%Proc. Interspeech 2018%.

Available on `https://www.isca-archive.org/interspeech_2018/chung18b_interspeech.html`.

################################################################################
"Nagrani, Chung & Zisserman (2017)"
© Anastasia Shchupak 2026-06-01

Arsha Nagrani, Joon Son Chung & Andrew Zisserman (2017): “VoxCeleb: a large-scale speaker
identification dataset.” %%Proc. Interspeech 2017%.

Available on `https://www.isca-archive.org/interspeech_2017/nagrani17_interspeech.html`.

################################################################################
)~~~"
MAN_PAGES_END

}

/* End of file manual_SpeechRecognizer.cpp */
