
include_directories(${REACTOS_BINARY_DIR}/include/reactos)
add_library(iologmsg SHARED iologmsg.rc)
set_module_type(iologmsg win32dll ENTRYPOINT 0 UNICODE)
add_dependencies(iologmsg ntiologc)
add_cd_file(TARGET iologmsg DESTINATION reactos/system32 FOR all)
