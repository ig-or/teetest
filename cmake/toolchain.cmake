

#  based on https://github.com/apmorton/teensy-template

INCLUDE(CMakeForceCompiler)
message(STATUS ${CMAKE_GENERATOR})
#string(COMPARE EQUAL ${CMAKE_GENERATOR} "Ninja" tbProto)
#if (tbProto)#
#	message(STATUS "creating a TinkerBell project")
#else()
#	message(STATUS "creating a VS project")
#endif()

set(CMAKE_SYSTEM_NAME Generic)
#set (CMAKE_SYSTEM_VERSION )
set(CMAKE_SYSTEM_PROCESSOR arm)

set(atools "$ENV{TEENSY_TOOLS}")
set(gcc_path "$ENV{GCC_ARM}")
message(STATUS "atools =  ${atools}; gcc path = ${gcc_path}")
#SET(ASM_OPTIONS "-x assembler-with-cpp")

SET(CMAKE_FIND_ROOT_PATH  ${TOOLSPATH}/arm)
set(CMAKE_TRY_COMPILE_TARGET_TYPE   STATIC_LIBRARY)


set(COMPILERPATH  "${gcc_path}/bin")

set(esfx)
if (WIN32)
set(esfx .exe)
endif()

set(CMAKE_AR  ${COMPILERPATH}/arm-none-eabi-ar${esfx})
set(CMAKE_C_COMPILER "${COMPILERPATH}/arm-none-eabi-gcc${esfx}")
set(CMAKE_CXX_COMPILER "${COMPILERPATH}/arm-none-eabi-g++${esfx}")
set(CMAKE_ASM_COMPILER "${COMPILERPATH}/arm-none-eabi-gcc${esfx}")

#set(CMAKE_ASM_COMPILER "\"${COMPILERPATH}/arm-none-eabi-as${esfx}")
#set(CMAKE_ASM_NASM_COMPILER "\"${COMPILERPATH}/arm-none-eabi-as${esfx}")
#set(CMAKE_ASM_MASM_COMPILER "\"${COMPILERPATH}/arm-none-eabi-as${esfx}")
#set(CMAKE_ASM-ATT_COMPILER "\"${COMPILERPATH}/arm-none-eabi-as${esfx}")
set(CMAKE_LINKER  "${COMPILERPATH}/arm-none-eabi-ld${esfx}")

set(CMAKE_OBJCOPY  "${COMPILERPATH}/arm-none-eabi-objcopy${esfx}"  CACHE INTERNAL "")
set(CMAKE_OBJDUMP "${COMPILERPATH}/arm-none-eabi-objdump${esfx}"  CACHE INTERNAL "")
set(CMAKE_SIZE  "${COMPILERPATH}/arm-none-eabi-size${esfx}"  CACHE INTERNAL "")
set(CMAKE_RANLIB "${COMPILERPATH}/arm-none-eabi-gcc-ranlib${esfx}"  CACHE INTERNAL "")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Use these lines for Teensy 4.1
#MCU = IMXRT1062
#MCU_LD = imxrt1062_t41.ld
#MCU_DEF = ARDUINO_TEENSY41
set(MCU IMXRT1062)
set(MCU_LD imxrt1062_t41.ld)
set(MCU_DEF ARDUINO_TEENSY41)
set(TEENSY_CORE_SPEED 600000000)
set(AARDUINO 10816)

# options needed by many Arduino libraries to configure for Teensy model
#OPTIONS += -D__$(MCU)__ -DARDUINO=10813 -DTEENSYDUINO=154 -D$(MCU_DEF)
set(CMAKE_VERBOSE_MAKEFILE ON  CACHE INTERNAL "")

set(CDEFS "-DUSB_SERIAL -DLAYOUT_US_ENGLISH -DUSING_MAKEFILE -DXM_ARM -D__arm__ -DF_CPU=${TEENSY_CORE_SPEED} -D__${MCU}__  -DARDUINO=${AARDUINO} -DTEENSYDUINO=155 -D${MCU_DEF}")
#add_definitions(-D__MK66FX1M0__)



set (BUILDDIR  build)
set (BUILD_DIR  build)
#set (COREPATH "${CMAKE_CURRENT_LIST_DIR}/pjrc/teensy4")
set (COREPATH "${CMAKE_CURRENT_LIST_DIR}/../pjrc/teensy4")

# for Cortex M7 with single & double precision FPU
set(CPUOPTIONS " -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -mthumb ")  #-ffast-math
#set(CPPFLAGS  "${CPPFLAGS}  -mcpu=cortex-m7  -march=armv7e-m -mfloat-abi=hard -mfpu=fpv5-d16  -ffast-math ")

# use this for a smaller, no-float printf
#SPECS = --specs=nano.specs

#set(files bootdata.c)

# CPPFLAGS = compiler options for C and C++
#set(CPPFLAGS " -Wall -g -O2 -mthumb -ffunction-sections -fdata-sections -MMD ") # -Wno-switch " ) #  -Wdouble-promotion " )
set(CPPFLAGS " -Wall -Os -fstack-usage ${CPUOPTIONS} -MMD -I. -ffunction-sections -fdata-sections ")

set(CPPFLAGS  " ${CPPFLAGS} ${CDEFS} ") #  -Wno-switch -funwind-tables  -fasynchronous-unwind-tables ")

# compiler options for C++ only
#set(CXXFLAGS  " -felide-constructors  -fno-rtti  -fsized-deallocation")  #  -fno-exceptions
set(CXXFLAGS  " -std=gnu++14 -felide-constructors -fno-exceptions -fpermissive -fno-rtti -Wno-error=narrowing ")


# compiler options for C only
#set (CFLAGS )
# additional libraries to link
set(LIBS   "-larm_cortexM7lfsp_math -lm -lstdc++" )

# linker options
#set (LDFLAGS " -Os -Wl,--gc-sections -mthumb ") # -LC:/programs/teensy/tools/arm/arm-none-eabi/lib")
set(LDSCRIPT  "\"${COREPATH}/${MCU_LD}\"")
#set (LDFLAGS " -O2 -Wl,--gc-sections,--relax ${SPECS} ${CPUOPTIONS} -T${LDSCRIPT} ")
set (LDFLAGS "-Os -Wl,--gc-sections,--relax -T${LDSCRIPT} ${LIBS}") # -fPIC
#set (LDFLAGS "${LDFLAGS} \"-L${gcc_path}/lib/gcc/arm-none-eabi/10.3.1/thumb/v7e-m+fp/hard\"")
#set (LDFLAGS "${LDFLAGS} \"-L${gcc_path}/arm-none-eabi/lib/thumb/v7e-m+fp/hard\"")
set (LDFLAGS "${LDFLAGS} \"-L${atools}/arm/arm-none-eabi/lib/armv7e-m/fpu/fpv5-d16\"")
set (LDFLAGS "${LDFLAGS} \"-L${atools}/arm/arm-none-eabi/lib\"")
#set(LDFLAGS "${LDFLAGS}   -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -T${LDSCRIPT} " ) # -u _scanf_float  -u _printf_float")
#if (tbProto)
#	set(LDFLAGS "${LDFLAGS}  -u _scanf_float  -u _printf_float ")
#endif()

set(CMAKE_C_FLAGS "${CPPFLAGS}"  CACHE INTERNAL "" FORCE)
set(CMAKE_CXX_FLAGS "${CPPFLAGS} ${CXXFLAGS}"  CACHE INTERNAL "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS ${LDFLAGS}  CACHE INTERNAL "" FORCE)

#add_compile_options(${COREPATH}/bootdata.c)

message (STATUS "COREPATH = ${COREPATH}")
message (STATUS "LDSCRIPT = ${LDSCRIPT}")
message (STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message (STATUS "CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")
message (STATUS "CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
message (STATUS "CMAKE_EXE_LINKER_FLAGS = ${CMAKE_EXE_LINKER_FLAGS}")

#ASMFLAGS, ASM_NASMFLAGS, ASM_MASMFLAGS or ASM-ATTFLAGS
set (asmFlags  " ${CPPFLAGS} -x assembler-with-cpp")

set(ASMFLAGS  "${asmFlags}"   CACHE INTERNAL "" FORCE)
set(ASM_NASMFLAGS  "${asmFlags}"  CACHE INTERNAL "" FORCE )
set(ASM_MASMFLAGS  "${asmFlags}"   CACHE INTERNAL "" FORCE)
set(ASM_ATTFLAGS  "${asmFlags}"   CACHE INTERNAL "" FORCE)

set(CMAKE_ASM_FLAGS  "${asmFlags}"  CACHE INTERNAL "" FORCE )
set(CMAKE_NASM_FLAGS  "${asmFlags}"  CACHE INTERNAL "" FORCE)
set(CMAKE_MASM_FLAGS  "${asmFlags}"  CACHE INTERNAL "" FORCE)
set(CMAKE_ATT_FLAGS  "${asmFlags} " CACHE INTERNAL "" FORCE )

set(BUILD_SHARED_LIBS OFF)

