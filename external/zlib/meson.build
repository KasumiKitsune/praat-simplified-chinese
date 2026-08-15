# meson.build of the external library "zlib"
# Paul Boersma, 18 July 2026

sources = '''
	adler32.c crc32.c gzread.c gzlib.c gzclose.c inflate.c inffast.c inftrees.c zutil.c'''.split()

libzlib = static_library (
	'zlib',
	sources : sources,
	include_directories : [melder_inc]
)

libzlib_dep = declare_dependency (
	include_directories : include_directories('.'), link_with: libzlib
)
