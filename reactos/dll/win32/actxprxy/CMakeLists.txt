
add_definitions(-D__WINESRC__)
include_directories(${REACTOS_SOURCE_DIR}/include/reactos/wine)
add_definitions(-DWINE_REGISTER_DLL -DPROXY_DELEGATION)
spec2def(actxprxy.dll actxprxy.spec)

add_rpcproxy_files(
    actxprxy_activscp.idl
    actxprxy_comcat.idl
    actxprxy_docobj.idl
    actxprxy_hlink.idl
    actxprxy_htiface.idl
    actxprxy_htiframe.idl
    actxprxy_objsafe.idl
    actxprxy_ocmm.idl
    actxprxy_servprov.idl
    actxprxy_shldisp.idl
    actxprxy_shobjidl.idl
    actxprxy_urlhist.idl)

list(APPEND SOURCE
    usrmarshal.c
    actxprxy.rc
    ${CMAKE_CURRENT_BINARY_DIR}/actxprxy_activscp_p.c
    ${CMAKE_CURRENT_BINARY_DIR}/actxprxy_comcat_p.c
    ${CMAKE_CURRENT_BINARY_DIR}/actxprxy_docobj_p.c
    ${CMAKE_CURRENT_BINARY_DIR}/actxprxy_hlink_p.c
    ${CMAKE_CURRENT_BINARY_DIR}/actxprxy_htiface_p.c
    ${CMAKE_CURRENT_BINARY_DIR}/actxprxy_htiframe_p.c
    ${CMAKE_CURRENT_BINARY_DIR}/actxprxy_objsafe_p.c
    ${CMAKE_CURRENT_BINARY_DIR}/actxprxy_ocmm_p.c
    ${CMAKE_CURRENT_BINARY_DIR}/actxprxy_servprov_p.c
    ${CMAKE_CURRENT_BINARY_DIR}/actxprxy_shldisp_p.c
    ${CMAKE_CURRENT_BINARY_DIR}/actxprxy_shobjidl_p.c
    ${CMAKE_CURRENT_BINARY_DIR}/actxprxy_urlhist_p.c
    ${CMAKE_CURRENT_BINARY_DIR}/proxy.dlldata.c
    ${CMAKE_CURRENT_BINARY_DIR}/actxprxy.def)

add_library(actxprxy SHARED ${SOURCE})
set_module_type(actxprxy win32dll)
target_link_libraries(actxprxy uuid wine ${PSEH_LIB})
add_importlibs(actxprxy rpcrt4 ole32 oleaut32 msvcrt kernel32 ntdll)
add_cd_file(TARGET actxprxy DESTINATION reactos/system32 FOR all)
