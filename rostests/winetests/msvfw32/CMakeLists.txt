
add_definitions(-DUSE_WINE_TODOS)
add_executable(msvfw32_winetest drawdib.c msvfw.c testlist.c)
set_module_type(msvfw32_winetest win32cui)
add_importlibs(msvfw32_winetest gdi32 advapi32 msvfw32 msvcrt kernel32)
add_cd_file(TARGET msvfw32_winetest DESTINATION reactos/bin FOR all)
