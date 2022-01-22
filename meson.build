project('cover', 'c', license : 'BSD-3-Clause')

##########################
# Dependencies and tools #
##########################

pkgconfig = import('pkgconfig')

cc = meson.get_compiler('c')
libdl = cc.find_library('dl', required : false)

###########
# Targets #
###########

subdir('include')
subdir('src/libcover')

############
# Examples #
############

subdir('examples')
