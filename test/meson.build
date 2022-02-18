tests = {
	'assert' : { 'should_fail' : true },
	'signal' : { 'should_fail' : true },
}

foreach name, config : tests
	exe = executable(name,
		include_directories : headers,
		link_with : libcover,
		sources : config.get('sources', name + '.c')
	)

	test(name, exe,
		args : [ '-tap', '-' ],
		protocol : 'tap',
		should_fail : config.get('should_fail', false)
	)
endforeach
