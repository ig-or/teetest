
cmake_minimum_required (VERSION 3.16)
set(PROJECT_NAME utest)

set(cdir "${CMAKE_SOURCE_DIR}")
set(bdir "${CMAKE_SOURCE_DIR}/../..")
set(xqt "${bdir}/xqtools")
set(NO_VERSION_INFO 1)
include("${xqt}/buildsys/ccmake.cmake")
commonStart()


#add_definitions(-DNOMINMAX)
add_definitions(-DUSE_SINGLE__PRECISION_NUMBERS -DPCTEST)

addSourceFiles("main" "." 	"utest.cpp" )	
# add some of the matrix and 'common' files:
addSourceFiles(matrix "${xqt}/matrix/src" 
	 xmatrix_imp xmatrix2 xmfilter xmgsl
)
	
addSourceFiles(core "${xqt}/core/src"
  rbuf xmroundbuf xmutils xstdef xmatrixplatform
)

addSourceFiles(core "${bdir}/src"
  motordriver
)
 
#addSourceFiles(msg "xqtools/message/src"
#  xmessage xmessagesend)

programEnd()





