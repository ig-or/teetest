#
# 	ethernet client console
#

cmake_minimum_required (VERSION 3.21)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../xqtools/buildsys/")
include(ccmake)
set(PROJECT_NAME console)

set (NEED_BOOST  program_options date_time system filesystem thread chrono log iostreams)
commonStart()
set(NO_VERSION_INFO 1)

add_compile_definitions(tstype=int)

#add_definitions(-DNOMINMAX)
#add_definitions(-D_WIN32_WINNT=0x0A00)
#add_definitions(-DUNICODE -D_UNICODE)

#IF(WIN32)

#endif()

addSourceFiles("main" "." console inproc eth_client)
#addSourceFiles("src" "../src" )
addSourceFiles("core" "../xqtools/core/src" rbuf xmroundbuf xmutils xstdef xmatrixplatform)
addSourceFiles("matrix" "../xqtools/matrix/src" xmatrix_imp xmatrix2)
addSourceFiles("message" "../xqtools/message/src" xmessage xparser xq_parser datafileloader xqdatatxtfile xmessagepack xmessagesend)


#addOtherLib("Winmm.lib")
#addOtherLib("ws2_32.lib")

programEnd()


