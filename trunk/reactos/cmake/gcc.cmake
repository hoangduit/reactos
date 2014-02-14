
# Show a note about ccache build
if(ENABLE_CCACHE)
    message("-- Enabling ccache build - done")
endif()

# PDB style debug info
if(NOT DEFINED SEPARATE_DBG)
    set(SEPARATE_DBG FALSE)
endif()

if(NOT DEFINED USE_PSEH3)
    set(USE_PSEH3 1)
endif()

if(USE_PSEH3)
    add_definitions(-D_USE_PSEH3=1)
endif()

if(NOT DEFINED USE_DUMMY_PSEH)
    set(USE_DUMMY_PSEH 0)
endif()

if(USE_DUMMY_PSEH)
    add_definitions(-D_USE_DUMMY_PSEH=1)
endif()

# Compiler Core
add_compile_flags("-pipe -fms-extensions -fno-strict-aliasing")
if(GCC_VERSION VERSION_GREATER 4.7)
    add_compile_flags("-mstackrealign")
endif()

add_compile_flags_language("-fno-rtti -fno-exceptions" "CXX")

#bug
#file(TO_NATIVE_PATH ${REACTOS_SOURCE_DIR} REACTOS_SOURCE_DIR_NATIVE)
#workaround
set(REACTOS_SOURCE_DIR_NATIVE ${REACTOS_SOURCE_DIR})
 if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
string(REPLACE "/" "\\" REACTOS_SOURCE_DIR_NATIVE ${REACTOS_SOURCE_DIR})
endif()
add_compile_flags("-fdebug-prefix-map=\"${REACTOS_SOURCE_DIR_NATIVE}\"=ReactOS")

# Debugging
if(SEPARATE_DBG)
    add_compile_flags("-gdwarf-2 -g2")
else()
    add_compile_flags("-gdwarf-2 -gstrict-dwarf -femit-struct-debug-detailed=none -feliminate-unused-debug-symbols")
endif()

# For some reason, cmake sets -fPIC, and we don't want it
if(DEFINED CMAKE_SHARED_LIBRARY_ASM_FLAGS)
    string(REPLACE "-fPIC" "" CMAKE_SHARED_LIBRARY_ASM_FLAGS ${CMAKE_SHARED_LIBRARY_ASM_FLAGS})
endif()

# Tuning
if(ARCH STREQUAL "i386")
    add_compile_flags("-march=${OARCH} -mtune=${TUNE}")
else()
    add_compile_flags("-march=${OARCH}")
endif()

# Warnings, errors
add_compile_flags("-Werror -Wall -Wpointer-arith")
add_compile_flags("-Wno-char-subscripts -Wno-multichar -Wno-unused-value -Wno-maybe-uninitialized")
add_compile_flags("-Wno-error=unused-but-set-variable -Wno-error=narrowing")
add_compile_flags("-Wtype-limits -Wno-error=type-limits")

if(ARCH STREQUAL "amd64")
    add_compile_flags("-Wno-format")
elseif(ARCH STREQUAL "arm")
    add_compile_flags("-Wno-attributes")
endif()

# Optimizations
if(OPTIMIZE STREQUAL "1")
    add_compile_flags("-Os -ftracer")
elseif(OPTIMIZE STREQUAL "2")
    add_compile_flags("-Os")
elseif(OPTIMIZE STREQUAL "3")
    add_compile_flags("-Og")
elseif(OPTIMIZE STREQUAL "4")
    add_compile_flags("-O1")
elseif(OPTIMIZE STREQUAL "5")
    add_compile_flags("-O2")
elseif(OPTIMIZE STREQUAL "6")
    add_compile_flags("-O3")
elseif(OPTIMIZE STREQUAL "7")
    add_compile_flags("-Ofast")
endif()

# Link-time code generation
if(LTCG)
    add_compile_flags("-flto -ffat-lto-objects")
endif()

if(ARCH STREQUAL "i386")
    add_compile_flags("-mpreferred-stack-boundary=3 -fno-set-stack-executable -fno-optimize-sibling-calls -fno-omit-frame-pointer")
    # FIXME: this doesn't work. CMAKE_BUILD_TYPE is always "Debug"
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_flags("-momit-leaf-frame-pointer")
    endif()
elseif(ARCH STREQUAL "amd64")
    add_compile_flags("-mpreferred-stack-boundary=4")
endif()

# Other
if(ARCH STREQUAL "amd64")
    add_definitions(-U_X86_ -UWIN32)
elseif(ARCH STREQUAL "arm")
    add_definitions(-U_UNICODE -UUNICODE)
    add_definitions(-D__MSVCRT__) # DUBIOUS
endif()

add_definitions(-D_inline=__inline)

# alternative arch name
if(ARCH STREQUAL "amd64")
    set(ARCH2 x86_64)
else()
    set(ARCH2 ${ARCH})
endif()

if(SEPARATE_DBG)
    # PDB style debug puts all dwarf debug info in a separate dbg file
    message(STATUS "Building separate debug symbols")
    file(MAKE_DIRECTORY ${REACTOS_BINARY_DIR}/symbols)
    if(CMAKE_GENERATOR STREQUAL "Ninja")
        set(SYMBOL_FILE <TARGET_PDB>)
    else()
        set(SYMBOL_FILE <TARGET>.gdb)
    endif()
    set(OBJCOPY ${CMAKE_OBJCOPY})
    set(CMAKE_C_LINK_EXECUTABLE
        "<CMAKE_C_COMPILER> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>"
        "${OBJCOPY} --only-keep-debug <TARGET> ${REACTOS_BINARY_DIR}/symbols/${SYMBOL_FILE}"
        "${OBJCOPY} --strip-debug <TARGET>")
    set(CMAKE_CXX_LINK_EXECUTABLE
        "<CMAKE_CXX_COMPILER> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>"
        "${OBJCOPY} --only-keep-debug <TARGET> ${REACTOS_BINARY_DIR}/symbols/${SYMBOL_FILE}"
        "${OBJCOPY} --strip-debug <TARGET>")
    set(CMAKE_C_CREATE_SHARED_LIBRARY
        "<CMAKE_C_COMPILER> <CMAKE_SHARED_LIBRARY_C_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>"
        "${OBJCOPY} --only-keep-debug <TARGET> ${REACTOS_BINARY_DIR}/symbols/${SYMBOL_FILE}"
        "${OBJCOPY} --strip-debug <TARGET>")
    set(CMAKE_CXX_CREATE_SHARED_LIBRARY
        "<CMAKE_CXX_COMPILER> <CMAKE_SHARED_LIBRARY_CXX_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>"
        "${OBJCOPY} --only-keep-debug <TARGET> ${REACTOS_BINARY_DIR}/symbols/${SYMBOL_FILE}"
        "${OBJCOPY} --strip-debug <TARGET>")
    set(CMAKE_RC_CREATE_SHARED_LIBRARY
        "<CMAKE_C_COMPILER> <CMAKE_SHARED_LIBRARY_C_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>"
        "${OBJCOPY} --only-keep-debug <TARGET> ${REACTOS_BINARY_DIR}/symbols/${SYMBOL_FILE}"
        "${OBJCOPY} --strip-debug <TARGET>")
else()
    # Normal rsym build
    get_target_property(RSYM native-rsym IMPORTED_LOCATION_NOCONFIG)
    set(CMAKE_C_LINK_EXECUTABLE
        "<CMAKE_C_COMPILER> ${CMAKE_C_FLAGS} <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>"
        "${RSYM} -s ${REACTOS_SOURCE_DIR} <TARGET> <TARGET>")
    set(CMAKE_CXX_LINK_EXECUTABLE
        "<CMAKE_CXX_COMPILER> ${CMAKE_CXX_FLAGS} <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>"
        "${RSYM} -s ${REACTOS_SOURCE_DIR} <TARGET> <TARGET>")
    set(CMAKE_C_CREATE_SHARED_LIBRARY
        "<CMAKE_C_COMPILER> ${CMAKE_C_FLAGS} <CMAKE_SHARED_LIBRARY_C_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>"
        "${RSYM} -s ${REACTOS_SOURCE_DIR} <TARGET> <TARGET>")
    set(CMAKE_CXX_CREATE_SHARED_LIBRARY
        "<CMAKE_CXX_COMPILER> ${CMAKE_CXX_FLAGS} <CMAKE_SHARED_LIBRARY_CXX_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_CXX_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>"
        "${RSYM} -s ${REACTOS_SOURCE_DIR} <TARGET> <TARGET>")
    set(CMAKE_RC_CREATE_SHARED_LIBRARY
        "<CMAKE_C_COMPILER> ${CMAKE_C_FLAGS} <CMAKE_SHARED_LIBRARY_C_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>")
endif()

set(CMAKE_EXE_LINKER_FLAGS "-nostdlib -Wl,--enable-auto-image-base,--disable-auto-import,--disable-stdcall-fixup")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS_INIT} -Wl,--disable-stdcall-fixup")

SET(CMAKE_C_COMPILE_OBJECT "${CCACHE} <CMAKE_C_COMPILER> <DEFINES> -Wa,--compress-debug-sections <FLAGS> -o <OBJECT> -c <SOURCE>")
SET(CMAKE_CXX_COMPILE_OBJECT "${CCACHE} <CMAKE_CXX_COMPILER>  <DEFINES> <FLAGS> -o <OBJECT> -c <SOURCE>")
set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> -Wa,--compress-debug-sections -x assembler-with-cpp -o <OBJECT> -I${REACTOS_SOURCE_DIR}/include/asm -I${REACTOS_BINARY_DIR}/include/asm <FLAGS> <DEFINES> -D__ASM__ -c <SOURCE>")

set(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> -O coff <FLAGS> -DRC_INVOKED -D__WIN32__=1 -D__FLAT__=1 ${I18N_DEFS} <DEFINES> <SOURCE> <OBJECT>")
set(CMAKE_DEPFILE_FLAGS_RC "--preprocessor \"${MINGW_TOOLCHAIN_PREFIX}gcc${MINGW_TOOLCHAIN_SUFFIX} -E -xc-header -MMD -MF <DEPFILE> -MT <OBJECT>\" ")

# Optional 3rd parameter: stdcall stack bytes
function(set_entrypoint MODULE ENTRYPOINT)
    if(${ENTRYPOINT} STREQUAL "0")
        add_target_link_flags(${MODULE} "-Wl,-entry,0")
    elseif(ARCH STREQUAL "i386")
        set(_entrysymbol _${ENTRYPOINT})
        if(${ARGC} GREATER 2)
            set(_entrysymbol ${_entrysymbol}@${ARGV2})
        endif()
        add_target_link_flags(${MODULE} "-Wl,-entry,${_entrysymbol}")
    else()
        add_target_link_flags(${MODULE} "-Wl,-entry,${ENTRYPOINT}")
    endif()
endfunction()

function(set_subsystem MODULE SUBSYSTEM)
    add_target_link_flags(${MODULE} "-Wl,--subsystem,${SUBSYSTEM}")
endfunction()

function(set_image_base MODULE IMAGE_BASE)
    add_target_link_flags(${MODULE} "-Wl,--image-base,${IMAGE_BASE}")
endfunction()

function(set_module_type_toolchain MODULE TYPE)
    if(CPP_USE_STL)
        if((${TYPE} STREQUAL "kernelmodedriver") OR (${TYPE} STREQUAL "wdmdriver"))
            message(FATAL_ERROR "Use of STL in kernelmodedriver or wdmdriver type module prohibited")
        endif()
        target_link_libraries(${MODULE} -lstdc++ -lsupc++ -lgcc -lmingwex)
    elseif(CPP_USE_RT)
        target_link_libraries(${MODULE} -lsupc++ -lgcc)
    elseif(IS_CPP)
        target_link_libraries(${MODULE} -lgcc)
    endif()

    if((${TYPE} STREQUAL "kernelmodedriver") OR (${TYPE} STREQUAL "wdmdriver"))
        add_target_link_flags(${MODULE} "-Wl,--exclude-all-symbols,-file-alignment=0x1000,-section-alignment=0x1000")
        if(${TYPE} STREQUAL "wdmdriver")
            add_target_link_flags(${MODULE} "-Wl,--wdmdriver")
        endif()
    endif()
endfunction()

function(add_delay_importlibs _module)
    get_target_property(_module_type ${_module} TYPE)
    if(_module_type STREQUAL "STATIC_LIBRARY")
        message(FATAL_ERROR "Cannot add delay imports to a static library")
    endif()
    foreach(_lib ${ARGN})
        target_link_libraries(${_module} lib${_lib}_delayed)
    endforeach()
    target_link_libraries(${_module} delayimp)
endfunction()

if(NOT ARCH STREQUAL "i386")
    set(DECO_OPTION "-@")
endif()

function(generate_import_lib _libname _dllname _spec_file)
    # generate the def for the import lib
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_libname}_implib.def
        COMMAND native-spec2def -n=${_dllname} -a=${ARCH2} --implib -d=${CMAKE_CURRENT_BINARY_DIR}/${_libname}_implib.def ${CMAKE_CURRENT_SOURCE_DIR}/${_spec_file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_spec_file} native-spec2def)
    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_libname}_implib.def PROPERTIES EXTERNAL_OBJECT TRUE)

    #create normal importlib
    _add_library(${_libname} STATIC EXCLUDE_FROM_ALL ${CMAKE_CURRENT_BINARY_DIR}/${_libname}_implib.def)
    set_target_properties(${_libname} PROPERTIES LINKER_LANGUAGE "IMPLIB" PREFIX "")

    #create delayed importlib
    _add_library(${_libname}_delayed STATIC EXCLUDE_FROM_ALL ${CMAKE_CURRENT_BINARY_DIR}/${_libname}_implib.def)
    set_target_properties(${_libname}_delayed PROPERTIES LINKER_LANGUAGE "IMPLIB_DELAYED" PREFIX "")
endfunction()

# Cute little hack to produce import libs
set(CMAKE_IMPLIB_CREATE_STATIC_LIBRARY "${CMAKE_DLLTOOL} --def <OBJECTS> --kill-at --output-lib=<TARGET>")
set(CMAKE_IMPLIB_DELAYED_CREATE_STATIC_LIBRARY "${CMAKE_DLLTOOL} --def <OBJECTS> --kill-at --output-delaylib=<TARGET>")
function(spec2def _dllname _spec_file)
    # do we also want to add importlib targets?
    if(${ARGC} GREATER 2)
        if(${ARGN} STREQUAL "ADD_IMPORTLIB")
            set(__add_importlib TRUE)
        else()
            message(FATAL_ERROR "Wrong argument passed to spec2def, ${ARGN}")
        endif()
    endif()

    # get library basename
    get_filename_component(_file ${_dllname} NAME_WE)

    # error out on anything else than spec
    if(NOT ${_spec_file} MATCHES ".*\\.spec")
        message(FATAL_ERROR "spec2def only takes spec files as input.")
    endif()

    # generate exports def and stubs C file for the module
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_file}.def ${CMAKE_CURRENT_BINARY_DIR}/${_file}_stubs.c
        COMMAND native-spec2def -n=${_dllname} -a=${ARCH2} -d=${CMAKE_CURRENT_BINARY_DIR}/${_file}.def -s=${CMAKE_CURRENT_BINARY_DIR}/${_file}_stubs.c ${CMAKE_CURRENT_SOURCE_DIR}/${_spec_file}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_spec_file} native-spec2def)

    if(__add_importlib)
        generate_import_lib(lib${_file} ${_dllname} ${_spec_file})
    endif()
endfunction()

macro(macro_mc FLAG FILE)
    set(COMMAND_MC ${CMAKE_MC_COMPILER} ${FLAG} -b ${CMAKE_CURRENT_SOURCE_DIR}/${FILE}.mc -r ${REACTOS_BINARY_DIR}/include/reactos -h ${REACTOS_BINARY_DIR}/include/reactos)
endmacro()

#pseh lib, needed with mingw
set(PSEH_LIB "pseh")

# Macros
if(PCH)
    macro(add_pch _target _pch _sources)
        # When including x.h GCC looks for x.h.gch first
        set(_pch_final_name "${_target}_pch.h")
        set(_gch ${CMAKE_CURRENT_BINARY_DIR}/${_pch_final_name}.gch)

        if(IS_CPP)
            set(_pch_language CXX)
        else()
            set(_pch_language C)
        endif()

        # Build the precompiled header
        # HEADER_FILE_ONLY FALSE: force compiling the header
        # EXTERNAL_SOURCE TRUE: don't use the .gch file when linking
        set_source_files_properties(${_pch} PROPERTIES
            HEADER_FILE_ONLY FALSE
            LANGUAGE ${_pch_language}
            EXTERNAL_SOURCE TRUE
            OBJECT_LOCATION ${_gch})

        if(ENABLE_CCACHE)
            set(_ccache_flag "-fpch-preprocess")
        endif()

        # Include the gch in the specified source files, skipping the pch file itself
        list(REMOVE_ITEM ${_sources} ${_pch})
        foreach(_src ${${_sources}})
            set_property(SOURCE ${_src} APPEND_STRING PROPERTY COMPILE_FLAGS " ${_ccache_flag} -Winvalid-pch -Werror=invalid-pch -include ${_pch_final_name}")
            set_property(SOURCE ${_src} APPEND PROPERTY OBJECT_DEPENDS ${_gch})
        endforeach()
    endmacro()
else()
    macro(add_pch _target _pch _sources)
    endmacro()
endif()

function(CreateBootSectorTarget _target_name _asm_file _binary_file _base_address)
    set(_object_file ${_binary_file}.o)

    add_custom_command(
        OUTPUT ${_object_file}
        COMMAND ${CMAKE_ASM_COMPILER} -x assembler-with-cpp -o ${_object_file} -I${REACTOS_SOURCE_DIR}/include/asm -I${REACTOS_BINARY_DIR}/include/asm -D__ASM__ -c ${_asm_file}
        DEPENDS ${_asm_file})

    add_custom_command(
        OUTPUT ${_binary_file}
        COMMAND native-obj2bin ${_object_file} ${_binary_file} ${_base_address}
        # COMMAND objcopy --output-target binary --image-base 0x${_base_address} ${_object_file} ${_binary_file}
        DEPENDS ${_object_file} native-obj2bin)

    set_source_files_properties(${_object_file} ${_binary_file} PROPERTIES GENERATED TRUE)

    add_custom_target(${_target_name} ALL DEPENDS ${_binary_file})

endfunction()

function(allow_warnings __module)
    add_target_compile_flags(${__module} "-Wno-error")
endfunction()

macro(add_asm_files _target)
    list(APPEND ${_target} ${ARGN})
endmacro()

