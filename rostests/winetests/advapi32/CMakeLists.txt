
list(APPEND SOURCE
    cred.c
    crypt.c
    crypt_lmhash.c
    crypt_md4.c
    crypt_md5.c
    crypt_sha.c
    eventlog.c
    lsa.c
    registry.c
    security.c
    service.c
    testlist.c)

add_executable(advapi32_winetest ${SOURCE})
set_module_type(advapi32_winetest win32cui)
add_importlibs(advapi32_winetest advapi32 ole32 msvcrt kernel32)
add_cd_file(TARGET advapi32_winetest DESTINATION reactos/bin FOR all)
