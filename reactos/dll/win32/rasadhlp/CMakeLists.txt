
include_directories(${REACTOS_SOURCE_DIR}/include/reactos/winsock)
spec2def(rasadhlp.dll rasadhlp.spec)

list(APPEND SOURCE
    autodial.c
    init.c
    winsock.c
    precomp.h
    ${CMAKE_CURRENT_BINARY_DIR}/rasadhlp_stubs.c)

add_library(rasadhlp SHARED
    ${SOURCE}
    ${CMAKE_CURRENT_BINARY_DIR}/rasadhlp.def)

set_module_type(rasadhlp win32dll)
add_importlibs(rasadhlp ws2_32 msvcrt kernel32 ntdll)
add_pch(rasadhlp precomp.h SOURCE)
add_cd_file(TARGET rasadhlp DESTINATION reactos/system32 FOR all)
