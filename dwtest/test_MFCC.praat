# test_MFCC.praat
# Paul Boersma, 29 May 2026, David Weenink, 2026

appendInfo: "test_MFCC.praat:"

sound = Create Sound from formula: "sineWithNoise", 1, 0, 1, 44100, "1/2 * sin(2*pi*377*x) + randomGauss(0,0.1)"
mfcc1 = To MFCC: 12, 0.015, 0.005, 100, 100, 0
mfcc2 = Copy: "mfcc2"
selectObject: mfcc1, mfcc2

# The Cross-correlate and the Convolve commands would crash Praat before 20260603"

soundcc = Cross-correlate: "peak 0.99", "zero"
selectObject: mfcc1, mfcc2
soundcon = Convolve: "peak 0.99", "zero"

removeObject: sound, mfcc1, mfcc2, soundcc, soundcon

appendInfoLine: " OK"