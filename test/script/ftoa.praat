#
# test script ftoa.praat
# Paul Boersma, 10 June 2026
#

pi40$ = "3.1415926535897932384626433832795028841971"   ; 41-digit approximation
assert fixed$ (pi, 40) = "3.1415926535897931159979634685441851615906"   ; 16-digit approximation
assert number (fixed$ (pi, 40)) = number (pi40$)

assert double$ (pi) = "3.141592653589793"


assert half$ (pi) = "3.142"

assert graphicalHalf$ (pi) = "3.142"

assert naturalLogarithm$ (-40) = "4.248354255291589e-18"
assert naturalLogarithm$ (-41) = "1.5628821893349888e-18"
assert index ({ "5.74952226429356e-19", "5.749522264293559e-19" }, naturalLogarithm$ (-42))
assert naturalLogarithm$ (-43) = "2.1151310375910805e-19"
assert index ({ "9.602680054508676e-24", "9.602680054508677e-24" }, naturalLogarithm$ (-53))
assert naturalLogarithm$ (-63) = "4.359610000063081e-28"
assert naturalLogarithm$ (-73) = "1.9792598779469045e-32"
assert naturalLogarithm$ (-83) = "8.985825944049381e-37"
assert naturalLogarithm$ (-93) = "4.0795586671775603e-41"
assert naturalLogarithm$ (-94) = "1.5007857627073948e-41"
assert index ({ "5.521082277028581e-42", "5.52108227702858e-42" }, naturalLogarithm$ (-95))
assert index ({ "3.720075976020842e-44", "3.7200759760208406e-44" }, naturalLogarithm$ (-100))
assert index ({ "1.8521167695179883e-45", "1.852116769517988e-45" }, naturalLogarithm$ (-103))
assert index ({ "5.075958897549367e-435", "5.075958897549366e-435" }, naturalLogarithm$ (-1000))
x$ = naturalLogarithm$ (-10000)
assert left$ (x$, 17) = "1.135483865314535"
assert right$ (x$, 6) = "e-4343"
x$ = naturalLogarithm$ (-100000)
assert left$ (x$, 17) = "3.562949565332927"
assert right$ (x$, 7) = "e-43430"
assert naturalLogarithm$ (-1000000) = "3.296831477975111e-434295"
x$ = naturalLogarithm$ (-1e9)
assert left$ (x$, 16) = "1.24953427447586"
assert right$ (x$, 11) = "e-434294482"
if praat_64bit
	assert naturalLogarithm$ (-1e12) = "5.599753956953867e-434294481904"
	assert naturalLogarithm$ (-1e15) = "1.5399265260594919e-434294481903252"
else
	assert naturalLogarithm$ (-1e12) = "0"
	assert naturalLogarithm$ (-1e15) = "0"
endif
assert naturalLogarithm$ (-1e16) = "0"
assert naturalLogarithm$ (-1e17) = "0"
assert naturalLogarithm$ (-1e18) = "0"
assert naturalLogarithm$ (-1e19) = "0"   ; wrong


writeInfoLine: "OK"