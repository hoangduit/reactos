
set_cpp(WITH_RUNTIME)
include_directories(${REACTOS_SOURCE_DIR}/lib/3rdparty/cardlib)

list(APPEND SOURCE
    solcreate.cpp
    solgame.cpp
    solitaire.cpp
    solitaire.h)

add_executable(sol ${SOURCE} rsrc.rc)
target_link_libraries(sol cardlib)
set_module_type(sol win32gui UNICODE)
add_importlibs(sol advapi32 comctl32 user32 gdi32 msvcrt kernel32)
add_pch(sol solitaire.h SOURCE)
add_cd_file(TARGET sol DESTINATION reactos/system32 FOR all)
