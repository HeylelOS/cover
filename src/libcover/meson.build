libcover = library('cover',
	dependencies : libdl,
	include_directories : headers,
	install : true,
	override_options : [ 'b_lundef=false' ], # Required to lazily acquire cover_suite
	sources : [
		'backends/output.c',
		'backends.c',
		'suite.c',
	]
)

pkgconfig.generate(libcover, description : 'Test-suite framework for C.')
