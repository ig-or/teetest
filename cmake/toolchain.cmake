

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

set(COMPILERPATH  "${gcc_path}/bin")

set(CMAKE_C_COMPILER "${COMPILERPATH}/arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "${COMPILERPATH}/arm-none-eabi-g++")
set(CMAKE_ASM_COMPILER "${COMPILERPATH}/arm-none-eabi-gcc")

#set(CMAKE_ASM_COMPILER "\"${COMPILERPATH}/arm-none-eabi-as.exe")
#set(CMAKE_ASM_NASM_COMPILER "\"${COMPILERPATH}/arm-none-eabi-as.exe")
#set(CMAKE_ASM_MASM_COMPILER "\"${COMPILERPATH}/arm-none-eabi-as.exe")
#set(CMAKE_ASM-ATT_COMPILER "\"${COMPILERPATH}/arm-none-eabi-as.exe")

set(CMAKE_LINKER  "${COMPILERPATH}/arm-none-eabi-ld")
#set(CMAKE_AR )

set(CMAKE_OBJCOPY  "${COMPILERPATH}/arm-none-eabi-objcopy")
set(CMAKE_OBJDUMP "${COMPILERPATH}/arm-none-eabi-objdump")
set(CMAKE_SIZE  "${COMPILERPATH}/arm-none-eabi-size")
set(CMAKE_RANLIB "${COMPILERPATH}/arm-none-eabi-gcc-ranlib")

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

add_definitions(-DUSB_SERIAL -DLAYOUT_US_ENGLISH -DUSING_MAKEFILE 
	-DXM_ARM  
	-D__arm__ 
	-DF_CPU=${TEENSY_CORE_SPEED} 
	-D__$(MCU)__  -DARDUINO=${AARDUINO} -DTEENSYDUINO=155 -D${MCU_DEF}
)
#add_definitions(-D__MK66FX1M0__)



set (BUILDDIR  build)
set (BUILD_DIR  build)
set (COREPATH "${CMAKE_CURRENT_LIST_DIR}/pjrc/teensy4")

# for Cortex M7 with single & double precision FPU
set(CPUOPTIONS " -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -mthumb  -ffast-math ")
#set(CPPFLAGS  "${CPPFLAGS}  -mcpu=cortex-m7  -march=armv7e-m -mfloat-abi=hard -mfpu=fpv5-d16  -ffast-math ")

# use this for a smaller, no-float printf
#SPECS = --specs=nano.specs


# CPPFLAGS = compiler options for C and C++
#set(CPPFLAGS " -Wall -g -Os -mthumb -ffunction-sections -fdata-sections -MMD ") # -Wno-switch " ) #  -Wdouble-promotion " )
set(CPPFLAGS " -Wall -g -O2 ${CPUOPTIONS} -MMD -I. -ffunction-sections -fdata-sections ")

set(CPPFLAGS  "${CPPFLAGS}  -Wno-switch -funwind-tables  -fasynchronous-unwind-tables ")

# compiler options for C++ only
#set(CXXFLAGS  " -felide-constructors  -fno-rtti  -fsized-deallocation")  #  -fno-exceptions
set(CXXFLAGS  " -std=gnu++14 -felide-constructors -fno-exceptions -fpermissive -fno-rtti -Wno-error=narrowing ")


# compiler options for C only
#set (CFLAGS )

# linker options
#set (LDFLAGS " -Os -Wl,--gc-sections -mthumb ") # -LC:/programs/teensy/tools/arm/arm-none-eabi/lib")
set(LDSCRIPT  "\"${COREPATH}/${MCU_LD}\"")
#set (LDFLAGS " -O2 -Wl,--gc-sections,--relax ${SPECS} ${CPUOPTIONS} -T${LDSCRIPT} ")
set (LDFLAGS " -O2 -Wl,--gc-sections,--relax -T${LDSCRIPT} ")
#set(LDFLAGS "${LDFLAGS}   -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -T${LDSCRIPT} " ) # -u _scanf_float  -u _printf_float")
#if (tbProto)
#	set(LDFLAGS "${LDFLAGS}  -u _scanf_float  -u _printf_float ")
#endif()

# additional libraries to link
set(LIBS   " -larm_cortexM7lfsp_math -lm -lstdc++ ")

SET(CMAKE_ASM_FLAGS "${CPPFLAGS}" )
set(CMAKE_C_FLAGS "${CPPFLAGS}")
set(CMAKE_CXX_FLAGS "${CPPFLAGS} ${CXXFLAGS}")
set(CMAKE_EXE_LINKER_FLAGS ${LDFLAGS})

message (STATUS "LDFLAGS = ${LDFLAGS}")
message (STATUS "COREPATH = ${COREPATH}")
message (STATUS "LDSCRIPT = ${LDSCRIPT}")
message (STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message (STATUS "CPPFLAGS = ${CPPFLAGS}")

#ASMFLAGS, ASM_NASMFLAGS, ASM_MASMFLAGS or ASM-ATTFLAGS

set (asmFlags  " ${CPPFLAGS} -x assembler-with-cpp")

set(ASMFLAGS  "${asmFlags}"  )
set(ASM_NASMFLAGS  "${asmFlags}"  )
set(ASM_MASMFLAGS  "${asmFlags}"  )
set(ASM_ATTFLAGS  "${asmFlags}"  )

set(CMAKE_ASM_FLAGS  "${asmFlags}"  )
set(CMAKE_NASM_FLAGS  "${asmFlags}" )
set(CMAKE_MASM_FLAGS  "${asmFlags}" )
set(CMAKE_ATT_FLAGS  "${asmFlags} " )

set(BUILD_SHARED_LIBS OFF)

