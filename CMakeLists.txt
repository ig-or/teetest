
cmake_minimum_required (VERSION 3.16)

#cmake_policy(SET CMP0057 NEW)

set(PROJECT_NAME teetest)

PROJECT(${PROJECT_NAME})


set(PROJ_SRC_FILES  SL_${PROJECT_NAME})
set(CMAKE_COLOR_MAKEFILE TRUE)
set(CMAKE_VERBOSE_MAKEFILE ON)

SET(cdir "${CMAKE_SOURCE_DIR}")
set(xqt "${cdir}/xqtools")
INCLUDE("${cdir}/cmake/options.cmake")
include("${xqt}/buildsys/ccmake.cmake")

#INCLUDE("${CMAKE_SOURCE_DIR}/cmake/ccmake.cmake")

#if (tbProto)
ENABLE_LANGUAGE(ASM  C CXX)
message(STATUS "adding ASM language")
#else()
#	message(STATUS "adding no ASM language")
#endif()

set(SKIP_LIBS_LIST)

#option(WITH_WASHING_MACHINE "use washing machine" OFF)
#option(WITH_PIANO_SUPPORT "use piano keyboard" ON)
#option(WITH_BLUETOOTH4LN100 "connect to LN100 via bluetooth" ON)
#o#ption(WITH_SAFE_MODE "enable additional checking" OFF)
#option(WITH_SDFAT_LIB "enable SDFAT library" ON)
#option(WITH_THREADING "enable threading library" ON)
#option(WITH_HG1120_IMU "use great Honeywell IMU" OFF)

#set(L_LIST LIBS_${PROJECT_NAME})
set(BUILD_DIR ${CMAKE_BINARY_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BUILD_DIR}/debug")
set(ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${BUILD_DIR}/debug")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG "${BUILD_DIR}/debug")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BUILD_DIR}/release")
set(ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${BUILD_DIR}/release")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "${BUILD_DIR}/release")

add_definitions(-DOURPROJECTNAME=${PROJECT_NAME}) # -DENCODER_USE_INTERRUPTS -DENCODER_OPTIMIZE_INTERRUPTS)
#add_definitions(-DNOMINMAX)
add_definitions(-DUSE_SINGLE__PRECISION_NUMBERS -DUSE_IMU_INDEX=1 -DXMPRINTF_TO_FILE)

add_definitions(-DLWIP_PROVIDE_ERRNO=1 -DLWIP_IPV4=1) # for QNEthernet
add_compile_options( -std=c++14 )
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 ")

set(ourGroupNames)
#SUBDIRLIST(pjrc_libs ${CMAKE_SOURCE_DIR}/pjrc/libraries)
#SUBDIRLIST(arduino_libs ${CMAKE_SOURCE_DIR}/arduino)
set (foldersList  
	test/blink 
	src
	nav
	pjrc/teensy4 
	${CMAKE_SOURCE_DIR}/pjrc/libraries
#	${arduino_libs}
) #  pjrc/teensy)

#set(INC_DIR_LIST ${COREPATH}) # C:/programs/teensy/tools/arm/arm-none-eabi/include)

FOREACH(i ${foldersList})
	#message(STATUS ${i})
	addSourceFiles0(${i} ${i})
ENDFOREACH()

# add some of the matrix and 'common' files:
addSourceFiles(matrix "${xqt}/matrix/src" 
	xmatrix_imp xmatrix2 xmfilter xmgsl
)
	
addSourceFiles(core "${xqt}/core/src"
  rbuf xmroundbuf xmutils xstdef xmatrixplatform
)
 
addSourceFiles(core "${xqt}/message/src"
  xmessage xmessagesend
)

# add all the libraries:
#set(libFoldersList)
#SBDIRLIST(libFoldersList ${cdir}/libraries) # 
#FOREACH(i ${libFoldersList})
#	message(STATUS "adding library folder ${i}")
#	addSourceFiles0(library ${i})
#ENDFOREACH()

#addSourceFiles0(library ${cdir}/libraries)

#message(STATUS "ourGroupNames = " ${ourGroupNames}) 

# process all the groups:
foreach(ourGroup  ${ourGroupNames})
	source_group(${ourGroup} FILES ${group_${ourGroup}_files})
	#message(" group ${ourGroup} created; ")
	#message(${group_${ourGroup}_files})
	#message("")
endforeach()

#message(STATUS "INC_DIR_LIST = ")
foreach(dir  ${INC_DIR_LIST})
	message(STATUS "  -I    ${dir}")
endforeach()



#message(STATUS "PROJ_SRC_FILES = ${${PROJ_SRC_FILES}}")

ADD_EXECUTABLE(${PROJECT_NAME} ${${PROJ_SRC_FILES}})
INCLUDE_DIRECTORIES(${INC_DIR_LIST}) 
#SET_TARGET_PROPERTIES( ${PROJECT_NAME}
#	PROPERTIES    DEBUG_OUTPUT_NAME "${PROJECT_NAME}d.elf"    RELEASE_OUTPUT_NAME "${PROJECT_NAME}.elf"
#	ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${BUILD_DIR}/debug"  ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${BUILD_DIR}/release"
#)
SET_TARGET_PROPERTIES( ${PROJECT_NAME}
	PROPERTIES    
	DEBUG_OUTPUT_NAME "${PROJECT_NAME}d.elf"    
	RELEASE_OUTPUT_NAME "${PROJECT_NAME}.elf"
	OUTPUT_NAME "${PROJECT_NAME}.elf"

#	ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${BUILD_DIR}/debug"  
#	ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${BUILD_DIR}/release"
#	ARCHIVE_OUTPUT_DIRECTORY "${BUILD_DIR}/release"
)

#target_link_libraries(${PROJECT_NAME}  ${LIBS})

#set(atools $ENV{TEENSY_TOOLS})

if (WIN32)
	add_custom_command(TARGET ${PROJECT_NAME}  POST_BUILD 
		COMMAND echo  "-------teensy post buld started---------" 
		COMMAND ${CMAKE_SIZE} ARGS "${BUILD_DIR}/${PROJECT_NAME}.elf"
		COMMAND ${CMAKE_OBJCOPY} ARGS -O ihex -R .eeprom ${BUILD_DIR}/${PROJECT_NAME}.elf ${BUILD_DIR}/${PROJECT_NAME}.hex
		COMMAND  ${atools}/teensy_post_compile.exe 
			ARGS -file=${PROJECT_NAME} -path=${BUILD_DIR} -tools="${atools}"
		COMMAND echo  "teensy post buld completed" 
		BYPRODUCTS ${BUILD_DIR}/${PROJECT_NAME}.hex
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		COMMENT "---------teensy post build--------------" 
		VERBATIM 
		USES_TERMINAL
	)
else()
	add_custom_command(TARGET ${PROJECT_NAME}  POST_BUILD 
		COMMAND ${CMAKE_SIZE} ARGS "${BUILD_DIR}/${PROJECT_NAME}.elf"
		COMMAND ${CMAKE_OBJCOPY} ARGS -O ihex -R .eeprom ${BUILD_DIR}/${PROJECT_NAME}.elf ${BUILD_DIR}/${PROJECT_NAME}.hex
		#COMMAND  ${atools}/teensy_post_compile 
		#	ARGS -file=${PROJECT_NAME} -path=${BUILD_DIR} -tools="${atools}"
		#COMMAND echo  "teensy post buld completed" 
		BYPRODUCTS ${BUILD_DIR}/${PROJECT_NAME}.hex
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
		COMMENT "makeing HEX file" 
		VERBATIM 
		USES_TERMINAL
	)

endif()

add_custom_target(
	run ${atools}/teensy_reboot
	COMMENT   "loading the code into the teensy board" VERBATIM USES_TERMINAL
	WORKING_DIRECTORY "${atools}"
)
















