
list(APPEND SOURCE
    cabinet.cxx
    dfp.cxx
    main.cxx
    mszip.cxx
    raw.cxx)

include_directories(${REACTOS_SOURCE_DIR}/include/reactos/libs/zlib)
add_executable(cabman ${SOURCE})
target_link_libraries(cabman zlibhost)
