
add_definitions(-DMKHIVE_HOST)

include_directories(
    ${REACTOS_SOURCE_DIR}/lib/inflib
    ${REACTOS_SOURCE_DIR}/lib/cmlib
    ${REACTOS_SOURCE_DIR}/lib/rtl)

list(APPEND SOURCE
    binhive.c
    cmi.c
    cmindex.c
    cmname.c
    mkhive.c
    reginf.c
    registry.c
    rtl.c)

add_executable(mkhive ${SOURCE})
target_link_libraries(mkhive unicode cmlibhost inflibhost)
