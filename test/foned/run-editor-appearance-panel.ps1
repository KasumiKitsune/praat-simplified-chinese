param([string] $Compiler = 'C:/msys64/clang64/bin/clang.exe')
$ErrorActionPreference = 'Stop'
$root = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$stage = Join-Path $root 'scratch/editor-appearance-test'
New-Item -ItemType Directory -Force -Path $stage, (Join-Path $root 'test-records/appearance-panel-native') | Out-Null
$includes = @('kar','melder','sys','dwsys','stat','fon','foned','dwtools','LPC','external/portaudio','external/flac','external/mp3','external/espeak') |
    ForEach-Object { '-I' + (Join-Path $root $_) }
$libraries = @('fon/libfon.a','artsynth/libartsynth.a','FFNet/libFFNet.a','gram/libgram.a','EEG/libEEG.a','LPC/libLPC.a',
    'dwtools/libdwtools.a','sensors/libsensors.a','foned/libfoned.a','fon/libfon.a','stat/libstat.a','dwsys/libdwsys.a',
    'sys/libsys.a','melder/libmelder.a','kar/libkar.a','external/espeak/libespeak.a','external/portaudio/libportaudio.a',
    'external/flac/libflac.a','external/lame/liblame.a','external/mp3/libmp3.a','external/glpk/libglpk.a',
    'external/clapack/libclapack.a','external/gsl/libgsl.a','external/num/libnum.a','external/vorbis/libvorbis.a',
    'external/opusfile/libopusfile.a','external/whispercpp/libwhisper.a','external/blake3/libblake3.a','external/zlib/libzlib.a') |
    ForEach-Object { Join-Path $root $_ }
$object = Join-Path $stage 'panel-test.o'
$exe = Join-Path $stage 'panel-test.exe'
& $Compiler --driver-mode=g++ -c -std=gnu++17 -municode -D_FILE_OFFSET_BITS=64 -march=x86-64-v3 -O2 -Wshadow @includes (Join-Path $PSScriptRoot 'editor_appearance_panel.cpp') -o $object
if ($LASTEXITCODE -ne 0) { throw 'Panel test compilation failed.' }
& $Compiler --driver-mode=g++ -o $exe $object (Join-Path $root 'main/praat_win.o') @libraries -lwinmm -lwsock32 -lcomctl32 -lole32 -lgdi32 -lgdiplus -lcomdlg32 -lwinspool -lshell32 -luxtheme -ldwmapi -static -lc++ -lc++abi
if ($LASTEXITCODE -ne 0) { throw 'Panel test linking failed; build Praat first.' }
Push-Location $root
try { & $exe; if ($LASTEXITCODE -ne 0) { throw 'Panel layout or interaction checks failed.' } }
finally { Pop-Location }
