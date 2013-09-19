
# set_cpp
#  Marks the current folder as containing C++ modules, additionally enabling
#  specific C++ language features as specified (all of these default to off):
#
#  WITH_RUNTIME
#   Links with the C++ runtime. Enable this for modules which use new/delete or
#   RTTI, but do not require STL. This is the right choice if you see undefined
#   references to operator new/delete, vector constructor/destructor iterator,
#   type_info::vtable, ...
#   Note: this only affects linking, so cannot be used for static libraries.
#  WITH_RTTI
#   Enables run-time type information. Enable this if the module uses typeid or
#   dynamic_cast. You will probably need to enable WITH_RUNTIME as well, if
#   you're not already using STL.
#  WITH_EXCEPTIONS
#   Enables C++ exception handling. Enable this if the module uses try/catch or
#   throw. You might also need this if you use a standard operator new (the one
#   without nothrow).
#  WITH_STL
#   Enables standard C++ headers and links to the Standard Template Library.
#   Use this for modules using anything from the std:: namespace, e.g. maps,
#   strings, vectors, etc.
#   Note: this affects both compiling (via include directories) and
#         linking (by adding STL). Implies WITH_RUNTIME.
#   FIXME: WITH_STL is currently also required for runtime headers such as
#          <new> and <exception>. This is not a big issue because in stl-less
#          environments you usually don't want those anyway; but we might want
#          to have modules like this in the future.
#
# Examples:
#  set_cpp()
#   Enables the C++ language, but will cause errors if any runtime or standard
#   library features are used. This should be the default for C++ in kernel
#   mode or otherwise restricted environments.
#   Note: this is required to get libgcc (for multiplication/division) linked
#         in for C++ modules, and to set the correct language for precompiled
#         header files, so it IS required even with no features specified.
#  set_cpp(WITH_RUNTIME)
#   Links with the C++ runtime, so that e.g. custom operator new implementations
#   can be used in a restricted environment. This is also required for linking
#   with libraries (such as ATL) which have RTTI enabled, even if the module in
#   question does not use WITH_RTTI.
#  set_cpp(WITH_RTTI WITH_EXCEPTIONS WITH_STL)
#   The full package. This will adjust compiler and linker so that all C++
#   features can be used.
macro(set_cpp)
    cmake_parse_arguments(__cppopts "WITH_RUNTIME;WITH_RTTI;WITH_EXCEPTIONS;WITH_STL" "" "" ${ARGN})
    if(__cppopts_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "set_cpp: unparsed arguments ${__cppopts_UNPARSED_ARGUMENTS}")
    endif()

    if(__cppopts_WITH_RUNTIME)
        set(CPP_USE_RT 1)
    endif()
    if(__cppopts_WITH_RTTI)
        if(MSVC)
            replace_compile_flags("/GR-" "/GR")
        else()
            replace_compile_flags_language("-fno-rtti" "-frtti" "CXX")
        endif()
    endif()
    if(__cppopts_WITH_EXCEPTIONS)
        if(MSVC)
            replace_compile_flags("/EHs-c-" "/EHsc")
        else()
            replace_compile_flags_language("-fno-exceptions" "-fexceptions" "CXX")
        endif()
    endif()
    if(__cppopts_WITH_STL)
        set(CPP_USE_STL 1)
        if(MSVC)
            add_definitions(-DNATIVE_CPP_INCLUDE=${REACTOS_SOURCE_DIR}/include/c++)
            include_directories(${REACTOS_SOURCE_DIR}/include/c++/stlport)
        endif()
    endif()

    set(IS_CPP 1)
endmacro()

function(add_dependency_node _node)
    if(GENERATE_DEPENDENCY_GRAPH)
        get_target_property(_type ${_node} TYPE)
        if(_type MATCHES SHARED_LIBRARY OR ${_node} MATCHES ntoskrnl)
            file(APPEND ${REACTOS_BINARY_DIR}/dependencies.graphml "    <node id=\"${_node}\"/>\n")
        endif()
     endif()
endfunction()

function(add_dependency_edge _source _target)
    if(GENERATE_DEPENDENCY_GRAPH)
        get_target_property(_type ${_source} TYPE)
        if(_type MATCHES SHARED_LIBRARY)
            file(APPEND ${REACTOS_BINARY_DIR}/dependencies.graphml "    <edge source=\"${_source}\" target=\"${_target}\"/>\n")
        endif()
    endif()
endfunction()

function(add_dependency_header)
    file(APPEND ${REACTOS_BINARY_DIR}/dependencies.graphml "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<graphml>\n  <graph id=\"ReactOS dependencies\" edgedefault=\"directed\">\n")
endfunction()

function(add_dependency_footer)
    add_dependency_node(ntdll)
    file(APPEND ${REACTOS_BINARY_DIR}/dependencies.graphml "  </graph>\n</graphml>\n")
endfunction()

function(add_message_headers _type)
    if(${_type} STREQUAL UNICODE)
        set(_flag "-U")
    else()
        set(_flag "-A")
    endif()
    foreach(_in_FILE ${ARGN})
        get_filename_component(FILE ${_in_FILE} NAME_WE)
        macro_mc(${_flag} ${FILE})
        add_custom_command(
            OUTPUT ${REACTOS_BINARY_DIR}/include/reactos/${FILE}.rc ${REACTOS_BINARY_DIR}/include/reactos/${FILE}.h
            COMMAND ${COMMAND_MC} ${MC_FLAGS}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE}.mc)
        set_source_files_properties(
            ${REACTOS_BINARY_DIR}/include/reactos/${FILE}.h ${REACTOS_BINARY_DIR}/include/reactos/${FILE}.rc
            PROPERTIES GENERATED TRUE)
        add_custom_target(${FILE} ALL DEPENDS ${REACTOS_BINARY_DIR}/include/reactos/${FILE}.h ${REACTOS_BINARY_DIR}/include/reactos/${FILE}.rc)
    endforeach()
endfunction()

function(add_link)
    cmake_parse_arguments(_LINK "MINIMIZE" "NAME;PATH;CMD_LINE_ARGS;ICON;GUID" "" ${ARGN})
    if(NOT _LINK_NAME OR NOT _LINK_PATH)
        message(FATAL_ERROR "You must provide name and path")
    endif()

    if(_LINK_CMD_LINE_ARGS)
        set(_LINK_CMD_LINE_ARGS -c ${_LINK_CMD_LINE_ARGS})
    endif()

    if(_LINK_ICON)
        set(_LINK_ICON -i ${_LINK_ICON})
    endif()

    if(_LINK_GUID)
        set(_LINK_GUID -g ${_LINK_GUID})
    endif()

    if(_LINK_MINIMIZE)
        set(_LINK_MINIMIZE "-m")
    endif()

    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_LINK_NAME}.lnk
        COMMAND native-mkshelllink -o ${CMAKE_CURRENT_BINARY_DIR}/${_LINK_NAME}.lnk ${_LINK_CMD_LINE_ARGS} ${_LINK_ICON} ${_LINK_GUID} ${_LINK_MINIMIZE} ${_LINK_PATH}
        DEPENDS native-mkshelllink)
    set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${_LINK_NAME}.lnk PROPERTIES GENERATED TRUE)
endfunction()

macro(dir_to_num dir var)
    if(${dir} STREQUAL reactos/system32)
        set(${var} 1)
    elseif(${dir} STREQUAL reactos/system32/drivers)
        set(${var} 2)
    elseif(${dir} STREQUAL reactos/Fonts)
        set(${var} 3)
    elseif(${dir} STREQUAL reactos)
        set(${var} 4)
    elseif(${dir} STREQUAL reactos/system32/drivers/etc)
        set(${var} 5)
    elseif(${dir} STREQUAL reactos/inf)
        set(${var} 6)
    elseif(${dir} STREQUAL reactos/bin)
        set(${var} 7)
    elseif(${dir} STREQUAL reactos/bin/data)
        set(${var} 8)
    elseif(${dir} STREQUAL reactos/media)
        set(${var} 9)
    elseif(${dir} STREQUAL reactos/Microsoft.NET)
        set(${var} 10)
    elseif(${dir} STREQUAL reactos/Microsoft.NET/Framework)
        set(${var} 11)
    elseif(${dir} STREQUAL reactos/Microsoft.NET/Framework/v1.0.3705)
        set(${var} 12)
    elseif(${dir} STREQUAL reactos/Microsoft.NET/Framework/v1.1.4322)
        set(${var} 13)
    elseif(${dir} STREQUAL reactos/Microsoft.NET/Framework/v2.0.50727)
        set(${var} 14)
    elseif(${dir} STREQUAL reactos/Resources)
        set(${var} 15)
    elseif(${dir} STREQUAL reactos/Resources/Themes)
        set(${var} 16)
    elseif(${dir} STREQUAL reactos/system32/wbem)
        set(${var} 17)
    else()
        message(FATAL_ERROR "Wrong destination: ${dir}")
    endif()
endmacro()

function(add_cd_file)
    cmake_parse_arguments(_CD "NO_CAB" "DESTINATION;NAME_ON_CD;TARGET" "FILE;FOR" ${ARGN})
    if(NOT (_CD_TARGET OR _CD_FILE))
        message(FATAL_ERROR "You must provide a target or a file to install!")
    endif()

    if(NOT _CD_DESTINATION)
        message(FATAL_ERROR "You must provide a destination")
    elseif(${_CD_DESTINATION} STREQUAL root)
        set(_CD_DESTINATION "")
    endif()

    if(NOT _CD_FOR)
        message(FATAL_ERROR "You must provide a cd name (or \"all\" for all of them) to install the file on!")
    endif()

    #get file if we need to
    if(NOT _CD_FILE)
        get_target_property(_CD_FILE ${_CD_TARGET} LOCATION_${CMAKE_BUILD_TYPE})
    endif()

    #do we add it to all CDs?
    if(_CD_FOR STREQUAL all)
        set(_CD_FOR "bootcd;livecd;regtest")
    endif()

    #do we add it to bootcd?
    list(FIND _CD_FOR bootcd __cd)
    if(NOT __cd EQUAL -1)
        #whether or not we should put it in reactos.cab or directly on cd
        if(_CD_NO_CAB)
            #directly on cd
            foreach(item ${_CD_FILE})
                if(_CD_NAME_ON_CD)
                    #rename it in the cd tree
                    set(__file ${_CD_NAME_ON_CD})
                else()
                    get_filename_component(__file ${item} NAME)
                endif()
                set_property(GLOBAL APPEND PROPERTY BOOTCD_FILE_LIST "${_CD_DESTINATION}/${__file}=${item}")
            endforeach()
            if(_CD_TARGET)
                #manage dependency
                add_dependencies(bootcd ${_CD_TARGET})
            endif()
        else()
            #add it in reactos.cab
            dir_to_num(${_CD_DESTINATION} _num)
            file(RELATIVE_PATH __relative_file ${REACTOS_SOURCE_DIR} ${_CD_FILE})
            file(APPEND ${REACTOS_BINARY_DIR}/boot/bootdata/packages/reactos.dff.dyn "\"${__relative_file}\" ${_num}\n")
            unset(__relative_file)
            if(_CD_TARGET)
                #manage dependency
                add_dependencies(reactos_cab ${_CD_TARGET})
            endif()
        endif()
    endif() #end bootcd

    #do we add it to livecd?
    list(FIND _CD_FOR livecd __cd)
    if(NOT __cd EQUAL -1)
        #manage dependency
        if(_CD_TARGET)
            add_dependencies(livecd ${_CD_TARGET})
        endif()
        foreach(item ${_CD_FILE})
            if(_CD_NAME_ON_CD)
                #rename it in the cd tree
                set(__file ${_CD_NAME_ON_CD})
            else()
                get_filename_component(__file ${item} NAME)
            endif()
            set_property(GLOBAL APPEND PROPERTY LIVECD_FILE_LIST "${_CD_DESTINATION}/${__file}=${item}")
        endforeach()
    endif() #end livecd

    #do we add it to regtest?
    list(FIND _CD_FOR regtest __cd)
    if(NOT __cd EQUAL -1)
        #whether or not we should put it in reactos.cab or directly on cd
        if(_CD_NO_CAB)
            #directly on cd
            foreach(item ${_CD_FILE})
                if(_CD_NAME_ON_CD)
                    #rename it in the cd tree
                    set(__file ${_CD_NAME_ON_CD})
                else()
                    get_filename_component(__file ${item} NAME)
                endif()
                set_property(GLOBAL APPEND PROPERTY BOOTCDREGTEST_FILE_LIST "${_CD_DESTINATION}/${__file}=${item}")
            endforeach()
            if(_CD_TARGET)
                #manage dependency
                add_dependencies(bootcdregtest ${_CD_TARGET})
            endif()
        else()
            #add it in reactos.cab
            #dir_to_num(${_CD_DESTINATION} _num)
            #file(APPEND ${REACTOS_BINARY_DIR}/boot/bootdata/packages/reactos.dff.dyn "${_CD_FILE} ${_num}\n")
            #if(_CD_TARGET)
            #    #manage dependency
            #    add_dependencies(reactos_cab ${_CD_TARGET})
            #endif()
        endif()
    endif() #end bootcd
endfunction()

function(create_iso_lists)
    get_property(_filelist GLOBAL PROPERTY BOOTCD_FILE_LIST)
    string(REPLACE ";" "\n" _filelist "${_filelist}")
    file(APPEND ${REACTOS_BINARY_DIR}/boot/bootcd.lst "${_filelist}")
    unset(_filelist)

    get_property(_filelist GLOBAL PROPERTY LIVECD_FILE_LIST)
    string(REPLACE ";" "\n" _filelist "${_filelist}")
    file(APPEND ${REACTOS_BINARY_DIR}/boot/livecd.lst "${_filelist}")
    unset(_filelist)

    get_property(_filelist GLOBAL PROPERTY BOOTCDREGTEST_FILE_LIST)
    string(REPLACE ";" "\n" _filelist "${_filelist}")
    file(APPEND ${REACTOS_BINARY_DIR}/boot/bootcdregtest.lst "${_filelist}")
    unset(_filelist)
endfunction()

# Create module_clean targets
function(add_clean_target _target)
    set(_clean_working_directory ${CMAKE_CURRENT_BINARY_DIR})
    if(CMAKE_GENERATOR STREQUAL "Unix Makefiles" OR CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
        set(_clean_command make clean)
    elseif(CMAKE_GENERATOR STREQUAL "NMake Makefiles")
        set(_clean_command nmake /nologo clean)
    elseif(CMAKE_GENERATOR STREQUAL "Ninja")
        set(_clean_command ninja -t clean ${_target})
        set(_clean_working_directory ${REACTOS_BINARY_DIR})
    endif()
    add_custom_target(${_target}_clean
        COMMAND ${_clean_command}
        WORKING_DIRECTORY ${_clean_working_directory}
        COMMENT "Cleaning ${_target}")
endfunction()

if(NOT MSVC_IDE)
    function(add_library name)
        _add_library(${name} ${ARGN})
        add_clean_target(${name})
    endfunction()

    function(add_executable name)
        _add_executable(${name} ${ARGN})
        add_clean_target(${name})
    endfunction()
elseif(USE_FOLDER_STRUCTURE)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    string(LENGTH ${CMAKE_SOURCE_DIR} CMAKE_SOURCE_DIR_LENGTH)

    function(add_custom_target name)
        _add_custom_target(${name} ${ARGN})
        string(SUBSTRING ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR_LENGTH} -1 CMAKE_CURRENT_SOURCE_DIR_RELATIVE)
        set_property(TARGET "${name}" PROPERTY FOLDER "${CMAKE_CURRENT_SOURCE_DIR_RELATIVE}")
    endfunction()

    function(add_library name)
        _add_library(${name} ${ARGN})
        get_target_property(_target_excluded ${name} EXCLUDE_FROM_ALL)
        if(_target_excluded AND ${name} MATCHES "^lib.*")
            set_property(TARGET "${name}" PROPERTY FOLDER "Importlibs")
        else()
            string(SUBSTRING ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR_LENGTH} -1 CMAKE_CURRENT_SOURCE_DIR_RELATIVE)
            set_property(TARGET "${name}" PROPERTY FOLDER "${CMAKE_CURRENT_SOURCE_DIR_RELATIVE}")
        endif()
    endfunction()

    function(add_executable name)
        _add_executable(${name} ${ARGN})
        string(SUBSTRING ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR_LENGTH} -1 CMAKE_CURRENT_SOURCE_DIR_RELATIVE)
        set_property(TARGET "${name}" PROPERTY FOLDER "${CMAKE_CURRENT_SOURCE_DIR_RELATIVE}")
    endfunction()
endif()

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    macro(to_win_path _cmake_path _native_path)
        string(REPLACE "/" "\\" ${_native_path} "${_cmake_path}")
    endmacro()

    # yeah the parameter mess sucks, but thats what works...
    function(concatenate_files _file1 _target2 _output)
        get_target_property(_file2 ${_target2} LOCATION)
        to_win_path("${_file1}" _real_file1)
        to_win_path("${_file2}" _real_file2)
        to_win_path("${_output}" _real_output)
        add_custom_command(
            OUTPUT ${_output}
            COMMAND cmd.exe /C "copy /Y /B ${_real_file1} + ${_real_file2} ${_real_output} > nul"
            DEPENDS ${_file1}
            DEPENDS ${_target2})
    endfunction()
else()
    macro(concatenate_files _file1 _target2 _output)
        get_target_property(_file2 ${_target2} LOCATION)
        add_custom_command(
            OUTPUT ${_output}
            COMMAND cat ${_file1} ${_file2} > ${_output}
            DEPENDS ${_file1}
            DEPENDS ${_target2})
    endmacro()
endif()

function(add_importlibs _module)
    add_dependency_node(${_module})
    foreach(LIB ${ARGN})
        if("${LIB}" MATCHES "msvcrt")
            add_target_compile_definitions(${_module} _DLL __USE_CRTIMP)
            target_link_libraries(${_module} msvcrtex)
        endif()
        target_link_libraries(${_module} lib${LIB})
        add_dependencies(${_module} lib${LIB})
        add_dependency_edge(${_module} ${LIB})
    endforeach()
endfunction()

function(set_module_type MODULE TYPE)
    cmake_parse_arguments(__module "UNICODE" "IMAGEBASE" "ENTRYPOINT" ${ARGN})

    if(__module_UNPARSED_ARGUMENTS)
        message(STATUS "set_module_type : unparsed arguments ${__module_UNPARSED_ARGUMENTS}, module : ${MODULE}")
    endif()

    # Set subsystem. Also take this as an occasion
    # to error out if someone gave a non existing type
    if((${TYPE} STREQUAL nativecui) OR (${TYPE} STREQUAL nativedll) OR (${TYPE} STREQUAL kernelmodedriver) OR (${TYPE} STREQUAL wdmdriver))
        set(__subsystem native)
    elseif(${TYPE} STREQUAL win32cui)
        set(__subsystem console)
    elseif(${TYPE} STREQUAL win32gui)
        set(__subsystem windows)
    elseif(NOT ((${TYPE} STREQUAL win32dll) OR (${TYPE} STREQUAL win32ocx)
            OR (${TYPE} STREQUAL cpl) OR (${TYPE} STREQUAL module)))
        message(FATAL_ERROR "Unknown type ${TYPE} for module ${MODULE}")
    endif()

    if(DEFINED __subsystem)
        set_subsystem(${MODULE} ${__subsystem})
    endif()

    #set unicode definitions
    if(__module_UNICODE)
        add_target_compile_definitions(${MODULE} UNICODE _UNICODE)
    endif()

    # set entry point
    if(__module_ENTRYPOINT OR (__module_ENTRYPOINT STREQUAL "0"))
        list(GET __module_ENTRYPOINT 0 __entrypoint)
        list(LENGTH __module_ENTRYPOINT __length)
        if(${__length} EQUAL 2)
            list(GET __module_ENTRYPOINT 1 __entrystack)
        elseif(NOT ${__length} EQUAL 1)
            message(FATAL_ERROR "Wrong arguments for ENTRYPOINT parameter of set_module_type : ${__module_ENTRYPOINT}")
        endif()
        unset(__length)
    elseif(${TYPE} STREQUAL nativecui)
        set(__entrypoint NtProcessStartup)
        set(__entrystack 4)
    elseif(${TYPE} STREQUAL win32cui)
        if(__module_UNICODE)
            set(__entrypoint wmainCRTStartup)
        else()
            set(__entrypoint mainCRTStartup)
        endif()
    elseif(${TYPE} STREQUAL win32gui)
        if(__module_UNICODE)
            set(__entrypoint wWinMainCRTStartup)
        else()
            set(__entrypoint WinMainCRTStartup)
        endif()
    elseif((${TYPE} STREQUAL win32dll) OR (${TYPE} STREQUAL win32ocx)
            OR (${TYPE} STREQUAL cpl))
        set(__entrypoint DllMainCRTStartup)
        set(__entrystack 12)
    elseif((${TYPE} STREQUAL kernelmodedriver) OR (${TYPE} STREQUAL wdmdriver))
        set(__entrypoint DriverEntry)
        set(__entrystack 8)
    elseif(${TYPE} STREQUAL nativedll)
        set(__entrypoint DllMain)
        set(__entrystack 12)
    elseif(${TYPE} STREQUAL module)
        set(__entrypoint 0)
    endif()

    if(DEFINED __entrypoint)
        if(DEFINED __entrystack)
            set_entrypoint(${MODULE} ${__entrypoint} ${__entrystack})
        else()
            set_entrypoint(${MODULE} ${__entrypoint})
        endif()
    endif()

    #set base address
    if(__module_IMAGEBASE)
        set_image_base(${MODULE} __module_IMAGEBASE)
    elseif(${TYPE} STREQUAL win32dll)
        if(DEFINED baseaddress_${MODULE})
            set_image_base(${MODULE} ${baseaddress_${MODULE}})
        else()
            message(STATUS "${MODULE} has no base address")
        endif()
    elseif((${TYPE} STREQUAL kernelmodedriver) OR (${TYPE} STREQUAL wdmdriver))
        set_image_base(${MODULE} 0x00010000)
    endif()

    # Now do some stuff which is specific to each type
    if((${TYPE} STREQUAL kernelmodedriver) OR (${TYPE} STREQUAL wdmdriver))
        add_dependencies(${MODULE} bugcodes)
        set_target_properties(${MODULE} PROPERTIES SUFFIX ".sys")
    endif()

    if(${TYPE} STREQUAL win32ocx)
        set_target_properties(${MODULE} PROPERTIES SUFFIX ".ocx")
    endif()

    if(${TYPE} STREQUAL cpl)
        set_target_properties(${MODULE} PROPERTIES SUFFIX ".cpl")
    endif()

    # do compiler specific stuff
    set_module_type_toolchain(${MODULE} ${TYPE})
endfunction()

function(preprocess_file __in __out)
    set(__arg ${__in})
    foreach(__def in ${ARGN})
        list(APPEND __arg -D${__def})
    endforeach()
    if(MSVC)
        add_custom_command(OUTPUT ${_out}
            COMMAND ${CMAKE_C_COMPILER} /EP ${__arg}
            DEPENDS ${__in})
    else()
        add_custom_command(OUTPUT ${_out}
            COMMAND ${CMAKE_C_COMPILER} -E ${__arg}
            DEPENDS ${__in})
    endif()
endfunction()

function(get_includes OUTPUT_VAR)
    get_directory_property(_includes INCLUDE_DIRECTORIES)
    foreach(arg ${_includes})
        list(APPEND __tmp_var -I${arg})
    endforeach()
    set(${OUTPUT_VAR} ${__tmp_var} PARENT_SCOPE)
endfunction()

function(get_defines OUTPUT_VAR)
    get_directory_property(_defines COMPILE_DEFINITIONS)
    foreach(arg ${_defines})
        list(APPEND __tmp_var -D${arg})
    endforeach()
    set(${OUTPUT_VAR} ${__tmp_var} PARENT_SCOPE)
endfunction()

if(NOT MSVC AND (CMAKE_VERSION VERSION_GREATER 2.8.7))
    function(add_object_library _target)
        add_library(${_target} OBJECT ${ARGN})
    endfunction()
else()
    function(add_object_library _target)
        add_library(${_target} ${ARGN})
    endfunction()
endif()
