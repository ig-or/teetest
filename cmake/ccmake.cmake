
# collect all the subdirectories from curdir into flist:
MACRO(SUBDIRLIST flist curdir)
	FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
	#message(STATUS "curdir = ${curdir};  children = ${children}")

	FOREACH(child ${children})
		if(${child} IN_LIST SKIP_LIBS_LIST)
			message(STATUS "library ${child} skipped")
		else()
			set (theLib ${curdir}/${child})
			IF(IS_DIRECTORY ${theLib})
				if (EXISTS "${theLib}/utility") # 
					list(APPEND ${flist} "${theLib}/utility")
				endif()
				if (EXISTS "${theLib}/src") # in thei case, we need only what is inside
					list(APPEND ${flist} "${theLib}/src")
					SUBDIRLIST(sub_dirs ${theLib}/src)
					FOREACH(subdir ${sub_dirs})
						list(APPEND  ${flist} ${subdir} )
					ENDFOREACH()
				else()
					list(APPEND ${flist} ${theLib})
				endif()
			ENDIF()
		endif()
	ENDFOREACH()
ENDMACRO()

# collect source files from the sourceFilesPath, into the groupName 
macro (addSourceFiles groupName sourceFilesPath)
	if(IS_ABSOLUTE ${sourceFilesPath})
		set(sfp ${sourceFilesPath})
	else()
		set(sfp ${CMAKE_SOURCE_DIR}/${sourceFilesPath})
	endif()
	
	set(extList "cpp" "cc" "c" "h" "hpp" "hh" "S" "s") # we need only those kind of files
	set(flist ${ARGN}) # since we cannot use ARGN directly since its not a 'variable'
	set(files)
	if (flist)   #    we have particular  files to use:
		foreach(f ${flist})
			set(find_a_file FALSE)
			foreach(e ${extList})
				set(fn "${sfp}/${f}.${e}")
				#message(STATUS "looking for the file ${fn}")
				if (EXISTS ${fn})
					list(APPEND files ${fn})
					set(find_a_file TRUE)
					
					#if (e STREQUAL "S")
					#	set_property(SOURCE ${fn} PROPERTY LANGUAGE C)
					#	message(STATUS "file ${fn}  also added!")
					#endif()
					#message(STATUS "file ${fn}  also added!")
					#message(STATUS "OK!")
				endif()
			endforeach()
			if(NOT find_a_file) 
				message(ERROR " cannot find file  ${f} in ${sfp}")
			endif()
		endforeach()
	else()   # use all the files
		set(exprList)
		foreach(ext ${extList})
			list(APPEND exprList "${sfp}/*.${ext}")
		endforeach()
		FILE(GLOB files  LIST_DIRECTORIES false  ${exprList})
		
		foreach (ff ${files})
			#message(STATUS "file ${ff} added!!!!")
			get_filename_component(cur_ext ${ff} EXT)
			if (cur_ext STREQUAL ".S")
				#set_property(SOURCE ${ff} PROPERTY LANGUAGE C)
				message(STATUS "file ${ff}  added!")
			endif()
		endforeach()
		
	endif()
	#message (STATUS ${files})
	if (files)
		list(APPEND INC_DIR_LIST ${sfp})
		list(APPEND group_${groupName}_files ${files}) # add files to the 'group file list'
		if (NOT ${groupName} IN_LIST ourGroupNames)
			list(APPEND ourGroupNames ${groupName})
		endif()
		list(APPEND ${PROJ_SRC_FILES}  ${files}) # add files to the 'global file list'
	else()
		message(ERROR "cannot find files in  ${sfp} ")
	endif()
endmacro()



