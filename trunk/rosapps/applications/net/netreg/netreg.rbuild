<module name="netreg" type="win32cui" installbase="system32" installname="netreg.exe"  stdlib="host">
	<include base="netreg">.</include>
	<library>user32</library>
	<library>ws2_32</library>
	<library>advapi32</library>
	<compilerflag compiler="cxx" compilerset="gcc">-Wno-non-virtual-dtor</compilerflag>
	<file>netreg.cpp</file>
	<file>netreg.rc</file>
</module>
