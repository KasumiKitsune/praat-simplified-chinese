# meson.build of the external library "blake3"
# Paul Boersma, 27 June 2026

sources = '''
	blake3.c blake3_dispatch.c blake3_portable.c'''.split()

libblake3 = static_library (
	'blake3',
	sources : sources,
	include_directories : [melder_inc]
)

libblake3_dep = declare_dependency (
	include_directories : include_directories('.'), link_with: libblake3
)
