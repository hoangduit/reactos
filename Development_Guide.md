

# Overview #
This guide will assume you have an out-of-the-box installation of Windows or ReactOS.  Starting from scratch, building ReactOS on Visual Studio requires some Development Tools, the ReactOS source, and some configuration.  This guide will assume a certain level of technical ability on your part, as this is a guide to compiling an operating system.

<br>
Also, please note that building ReactOS with Visual Studio is still a work in process.  ReactOS built with Visual Studio cannot yet successfully boot.<br>
<br>
<h1>Install Development Tools</h1>
The tools you'll need are MS Visual Studio, an SVN Client, a Virtual Machine (not mandatory <i>per se</i>, but HIGHLY recommended), and a couple other utilities.<br>
<br>
<h2>Install Visual Studio 2012</h2>
<blockquote>Download link: <a href='http://www.microsoft.com/visualstudio/eng/downloads'>http://www.microsoft.com/visualstudio/eng/downloads</a></blockquote>

<blockquote>We support only MS Visual Studio 2012.  Other versions have not been tested and are not expected to work.  MS Visual Studio 2012 can be found at <a href='http://www.microsoft.com/visualstudio/eng/downloads'>http://www.microsoft.com/visualstudio/eng/downloads</a>.  Windows 8 users may want <b>(need?)</b> to install Visual Studio Express 2012 for Windows 8 instead.  Note that the Express version is free, and all 2012 versions require the Microsoft .NET 4.5 framework.  Visual Studio Express requires a free registration within 30 days of installation.</blockquote>

<blockquote>There is a flaw in VS2012 related to .NET 4.5, and the fix can be found at <a href='http://www.microsoft.com/en-us/download/details.aspx?id=36020'>http://www.microsoft.com/en-us/download/details.aspx?id=36020</a>.  On first run, let VS2012 update itself with Update 3.  It's a shame that isn't included in the initial download as the update is about 2GB when installed.  Get comfortable or go do something else while this is happening.  You may want to run Windows Update after installation to ensure all related components are up to date.</blockquote>

<h2>Install SVN Client</h2>

An SVN (Apache Subversion) client will allow you to access the source of ReactOS.<br>
<br>
An SVN, client (named after the command name <i>svn</i>) is a versioning and revision control system that allows developers to control the changes made to the source code. If you have commit access, then you can commit your changes back to the ReactOS SVN repository.  We recommend and use TortoiseSVN, and this guide will assume it's the client you've installed. You can use another SVN client, but you'll need to configure it yourself.<br>
<br>
<h3>Install TortoiseSVN</h3>

<blockquote>Download link: <a href='http://tortoisesvn.net/downloads.html'>http://tortoisesvn.net/downloads.html</a></blockquote>

<blockquote>Make sure to download the right architecture (x86 or x64) for your system.  Windows XP users may need to update MSI installer (TortoiseSVN has a link).  Also note that TortoiseSVN comes with only language.  Additional language packs are available for download as well.</blockquote>

<blockquote>TortoiseSVN does not install command line utilities by default, but this option can be selected during install.  Having the command line utilities installed is optional.</blockquote>

...<br>
<br>
<b>SECTION INCOMPLETE?</b>

<h2>Install a Virtual Machine</h2>

<blockquote>A Virtual Machine (VM) will let you test the compiled operating system without modifying or risking data loss on your actual PC.  There are several great options to choose from.  We recommend VirtualBox.</blockquote>

No matter which virtual machine software you decide to use, there is one concept to keep in mind when configuring a virtual machine. A virtual machine really is that - a generally-configurable machine, and not simply a full computer that happens to have a blank hard disk. In other words, a virtual machine is a motherboard that must be populated with other hardware components that you select. This subtle difference in perception, computer-with-blank-hard disk versus mother-board-that-can-be-populated-hardware-components, will assist you in configuring your virtual machine. For example, just as with a real computer, a virtual machine allows you to add virtual mass-storage hardware controllers, such as IDE or SATA interfaces, to which you might attach virtual hard disks or CD-ROM drives, after which might you leave a CD "inserted" into the virtual CD-ROM drive.<br>
<br>
<h3>Install VirtualBox</h3>

Download link: <a href='https://www.virtualbox.org/wiki/Downloads'>https://www.virtualbox.org/wiki/Downloads</a>

<blockquote>For Windows, Linux, OS X, and Solaris.  A full-featured and easy-to-use virtual machine.  This is the recommended virtual machine to use.  The <a href='VirtualBox.md'>VirtualBox Setup Guide</a> has much more detailed setup information, but ultimately you'll be creating a virtual machine on your computer with 256 MB or 512 MB of RAM and a decent hard disk.  After you've <a href='Setup_Guide#Building_the_ReactOS_Source.md'>created your ISO</a>, you'll load that into the virtual machine and boot it.</blockquote>

<h4>Setting up VirtualBox</h4>
This guide is for setting up VirtualBox to use with the .ISO file compiled using Visual Studio 2012.  To start, open VirtualBox.  Click on the blue New icon at the top and name your new virtual machine.  It can be whatever you want - I chose "VS2012 Build".<br>
<img src='https://reactos.googlecode.com/svn/wiki/VBox%20Setup%201%20-%20en.png' />

After that, select the amount of RAM you want your virtual machine to have.  ReactOS doesn't need much at all, so 256MB or 512MB should be more than enough.  What good is a virtual machine with no storage?  Choose "create a virtual hard drive now" and click Create.<br>
<img src='https://reactos.googlecode.com/svn/wiki/VBox%20Setup%202%20-%20en.png' />

A virtual hard disk is simply a file on your hard drive that will act like a hard disk for your virtual machine.  There are several virtual hard disk formats to choose from (VDI, VMDK, VHD, HDD, QED, and QCOW).  It's okay if you don't know what they are - it really doesn't matter.  If you have Windows 7, you may want to try the .VHD format because Windows 7 can mount it as a hard drive and let you interact with it directly (useful for copying files to & from the virtual machine).<br>
<br>
Now you need to choose how the virtual hard drive will behave on your physical hard disk.  Fixed Size means it will always be the size that the virtual hard drive reports.  Dynamically Allocated means the virtual hard disk file will only take up as much space as it needs to (so empty parts of the virtual drive don't use any space on your physical hard disk).  You can choose either option.  In this example I chose Dynamically Allocated.<br>
<img src='https://reactos.googlecode.com/svn/wiki/VBox%20Setup%203%20-%20en.png' />

Next, name your virtual hard drive.  It doesn't matter what you call it.  The one in this example is called "VS2012 Build".  Now choose a size for the hard disk.  If you chose a Fixed Size drive, you'll likely want a smaller drive (1GB is plenty).  If you chose Dynamically Allocated, then the default size is fine since it won't be taking up that much space anyway.  Press Create.<br>
<br>
<h4>Testing your Virtual Machine</h4>
You're all done with setup!  Select your virtual machine from the list, and click on the green arrow icon, Start.  Your virtual machine will begin to boot.<br>
<br>
<h2>Install Archiver/Data Compression utility</h2>

<h3>Free</h3>

Link: <a href='http://www.7-zip.org/'>http://www.7-zip.org/</a>

<h3>Commercial</h3>

Link: <a href='http://www.rarlab.com/'>http://www.rarlab.com/</a>

Link: <a href='http://www.winzip.com/'>http://www.winzip.com/</a>

<h2>Install Dependency Walker</h2>
<blockquote>Download link: <a href='http://www.dependencywalker.com/'>http://www.dependencywalker.com/</a></blockquote>

<h2>Install WinFlashTool</h2>
<blockquote>Download link: <a href='http://winflashtool.sysprogs.com/'>http://winflashtool.sysprogs.com/</a></blockquote>

<h2>Install Remote Debuggers</h2>
<h3>Install Microsoft Visual Studio 2012 Remote Debugger</h3>
<h3>Install Remote Tools for Visual Studio 2012 Update 1</h3>
<h3>Install VisualDDK</h3>

<h1>Consult JIRA</h1>

JIRA is a bug tracking, issue tracking and project management software.<br>
<br>
URL: <a href='http://jira.reactos.org/'>http://jira.reactos.org/</a>

<h1>Pull Source Code From Repository</h1>
You can get the source code via command line or with the TortoiseSVN Browser.<br>
<br>
<h2>Pull Source Code Using SVN GUI</h2>
...<br>
<br>
When finished, your ReactOS source folder will have a green icon over it, like this:<br>
<img src='https://reactos.googlecode.com/svn/wiki/SVN%20Sync%206%20-%20en.png' />

This is the opposite making a commit.  If your source is outdated, or if you want to discard all changes you have made to your copy of the source, then you want to perform an SVN Update.  <b>Do not redownload the entire source.</b>  That is a waste of time and bandwidth. Navigate to your ReactOS source and right-click it.  Select "SVN Update" as in this screenshot:<br>
<img src='https://reactos.googlecode.com/svn/wiki/SVN%20Sync%202%20-%20en.png' />

Your copy of the source will begin to sync with the online version.  This window will notify you of the progress:<br>
<img src='https://reactos.googlecode.com/svn/wiki/SVN%20Sync%203%20-%20en.png' />

When finished, your ReactOS source will again have the green "no changes have been made" icon over it.<br>
<br>
<blockquote>Open the TortoiseSVN Browser.  It's likely located at Start -> TortoiseSVN -> TortoiseSVN Repository Browser.  Navigate to <code>https://reactos.googlecode.com/svn</code>.  In the left-hand pane, right click on the URL and select Checkout.</blockquote>

<blockquote><img src='https://reactos.googlecode.com/svn/wiki/SVN%20Browser%201%20-%20en.png' /></blockquote>

<blockquote>Set "Checkout directory" to the folder you'd like to put the Repository Source in.  Make sure that Checkout Depth is set to Fully Recursive.  Click on OK.</blockquote>

<blockquote><img src='https://reactos.googlecode.com/svn/wiki/SVN%20Browser%202%20-%20en.png' /></blockquote>

<blockquote>The Checkout process will begin.  This will take a while, so be patient.</blockquote>

<blockquote><img src='https://reactos.googlecode.com/svn/wiki/SVN%20Browser%203%20-%20en.png' /></blockquote>

<blockquote>Once it's finished, you're ready to fire up Visual Studio!</blockquote>

<h2>Pull Source Code Using SVN Command Line</h2>
<blockquote><i>(This is written for TortoiseSVN, but will likely work for other SVN command line utilities as well.)</i></blockquote>

<blockquote>Open up a Command Prompt and navigate to where you'd like to put the source code.</blockquote>

<blockquote>If you're a member and you'd like commit access, use the following command (replace <<name@email.com>> with your Google Code email account).  Note that it's <b>https</b>, not http.<br>
<code>svn checkout https://reactos.googlecode.com/svn/trunk/ reactos --username &lt;&lt;name@email.com&gt;&gt;</code></blockquote>

<blockquote>If you're not a member, or you'd like to download anonymously, you can get a read-only copy with the following command.  This time it's <b>http</b>.<br>
<code>svn checkout http://reactos.googlecode.com/svn/trunk/ reactos-read-only</code></blockquote>

<blockquote>A recent checkout clocked in at 661MB, so this may take a while depending on your internet connection.  You only need to do this once.  After your initial download, all changes will be Updates or Commits.  See <a href='TortoiseSVN.md'>TortoiseSVN</a>.</blockquote>

<h2>Special Files That Are Pulled</h2>

Ideally, we would be able to abstain from perturbing the official ReactOS file hierarchy to achieve an OS build, augmenting the hierarchy with one <b><code>.sln</code></b> file and many <b><code>.vcxproj</code></b> files. Unfortunately, total abstention is not possible because at least two <b><code>.h</code></b> files are generated at compile-time by <b><code>cmake</code></b>, and as stated previously, one of the primary reasons for creating our break-away project was to eliminate any dependency on <b><code>cmake</code></b>. To solve this dilemma, we simply copy, manually, <b><code>cmake</code></b>-generated header files to the locations where the <b><code>.c</code></b> files expect them to be, in our version of the ReactOS repository here at Google Code. Then, when the repository is pulled using SVN, these files will be included in the download. These files are as follows:<br>
<br>
<h3>buildno.h</h3>

Multiple files in ReactOS include the <b><code>cmake</code></b>-generated <b><code>buildno.h</code></b> to know what build number of ReactOS is being built. This file is normally generated by <b><code>cmake</code></b> to be located at:<br>
<br>
<b><code>reactos\include\reactos\buildno.h</code></b>

As can be seen by the following actual <b><code>buildno.h</code></b> that we use, it simply contains version information for the build of the OS:<br>
<br>
<pre><code>/* Do not edit - Machine generated */<br>
#ifndef _INC_REACTOS_BUILDNO<br>
#define _INC_REACTOS_BUILDNO<br>
#define KERNEL_VERSION_BUILD	20130406<br>
#define KERNEL_VERSION_BUILD_HEX	0x0<br>
#define KERNEL_VERSION_BUILD_STR	"20130406-rUNKNOWN"<br>
#define KERNEL_VERSION_BUILD_RC	"20130406-rUNKNOWN\0"<br>
#define KERNEL_RELEASE_RC	"0.4-SVN\0"<br>
#define KERNEL_RELEASE_STR	"0.4-SVN"<br>
#define KERNEL_VERSION_RC	"0.4-SVN\0"<br>
#define KERNEL_VERSION_STR	"0.4-SVN"<br>
#define REACTOS_DLL_VERSION_MAJOR	42<br>
#define REACTOS_DLL_RELEASE_RC	"42.4-SVN\0"<br>
#define REACTOS_DLL_RELEASE_STR	"42.4-SVN"<br>
#define REACTOS_DLL_VERSION_RC	"42.4-SVN\0"<br>
#define REACTOS_DLL_VERSION_STR	"42.4-SVN"<br>
#endif<br>
/* EOF */<br>
</code></pre>

<b><code>buildno.h</code></b> is generated by <b><code>cmake</code></b> from its companion file, <b><code>buildno.h.cmake</code></b>, located at:<br>
<br>
<b><code>reactos\include\reactos\buildno.h.cmake</code></b>

...whose text is as follows:<br>
<br>
<pre><code>/* Do not edit - Machine generated */<br>
#ifndef _INC_REACTOS_BUILDNO<br>
#define _INC_REACTOS_BUILDNO<br>
#define KERNEL_VERSION_BUILD	@KERNEL_VERSION_BUILD@<br>
#define KERNEL_VERSION_BUILD_HEX	0x@KERNEL_VERSION_BUILD_HEX@<br>
#define KERNEL_VERSION_BUILD_STR	"@KERNEL_VERSION_BUILD@@REVISION@"<br>
#define KERNEL_VERSION_BUILD_RC	"@KERNEL_VERSION_BUILD@@REVISION@\0"<br>
#define KERNEL_RELEASE_RC	"@KERNEL_VERSION@\0"<br>
#define KERNEL_RELEASE_STR	"@KERNEL_VERSION@"<br>
#define KERNEL_VERSION_RC	"@KERNEL_VERSION@\0"<br>
#define KERNEL_VERSION_STR	"@KERNEL_VERSION@"<br>
#define REACTOS_DLL_VERSION_MAJOR	@REACTOS_DLL_VERSION_MAJOR@<br>
#define REACTOS_DLL_RELEASE_RC	"@DLL_VERSION_STR@\0"<br>
#define REACTOS_DLL_RELEASE_STR	"@DLL_VERSION_STR@"<br>
#define REACTOS_DLL_VERSION_RC	"@DLL_VERSION_STR@\0"<br>
#define REACTOS_DLL_VERSION_STR	"@DLL_VERSION_STR@"<br>
#endif<br>
/* EOF */<br>
</code></pre>

<h3>version.h</h3>

Multiple files in ReactOS include the <b><code>cmake</code></b>-generated <b><code>version.h</code></b> to know what version of ReactOS is being built. This file is normally generated by <b><code>cmake</code></b> to be located at:<br>
<br>
<b><code>reactos\include\reactos\version.h</code></b>

As can be seen by the following actual <b><code>version.h</code></b> that we use, it simply contains version information for the OS:<br>
<br>
<pre><code>/*<br>
 * COPYRIGHT:   See COPYING in the top level directory<br>
 * PROJECT:     ReactOS kernel<br>
 * FILE:        include/internal/version.h<br>
 * PURPOSE:     Defines the current version<br>
 * PROGRAMMER:  David Welch (welch@mcmail.com)<br>
 * REVISIONS:<br>
 * 	1999-11-06 (ea)<br>
 * 		Moved from include/internal in include/reactos<br>
 *		to be used by buildno.<br>
 *	2002-01-17 (ea)<br>
 *		KERNEL_VERSION removed. Use<br>
 *		reactos/buildno.h:KERNEL_VERSION_STR instead.<br>
 */<br>
<br>
#ifndef __VERSION_H<br>
#define __VERSION_H<br>
<br>
#define KERNEL_VERSION_MAJOR 0<br>
#define KERNEL_VERSION_MINOR 4<br>
#define KERNEL_VERSION_PATCH_LEVEL 0<br>
<br>
#define COPYRIGHT_YEAR "2013"<br>
<br>
/* KERNEL_VERSION_BUILD_TYPE is L"SVN", L"RC1", L"RC2" or L"" (for the release) */<br>
#define KERNEL_VERSION_BUILD_TYPE "SVN"<br>
<br>
<br>
#endif<br>
<br>
/* EOF */<br>
</code></pre>

<b><code>version.h</code></b> is generated by <b><code>cmake</code></b> from its companion file, <b><code>version.h.cmake</code></b>, located at:<br>
<br>
<b><code>reactos\include\reactos\version.h.cmake</code></b>

...whose text is as follows:<br>
<br>
<pre><code>/*<br>
 * COPYRIGHT:   See COPYING in the top level directory<br>
 * PROJECT:     ReactOS kernel<br>
 * FILE:        include/internal/version.h<br>
 * PURPOSE:     Defines the current version<br>
 * PROGRAMMER:  David Welch (welch@mcmail.com)<br>
 * REVISIONS:<br>
 * 	1999-11-06 (ea)<br>
 * 		Moved from include/internal in include/reactos<br>
 *		to be used by buildno.<br>
 *	2002-01-17 (ea)<br>
 *		KERNEL_VERSION removed. Use<br>
 *		reactos/buildno.h:KERNEL_VERSION_STR instead.<br>
 */<br>
<br>
#ifndef __VERSION_H<br>
#define __VERSION_H<br>
<br>
#define KERNEL_VERSION_MAJOR @KERNEL_VERSION_MAJOR@<br>
#define KERNEL_VERSION_MINOR @KERNEL_VERSION_MINOR@<br>
#define KERNEL_VERSION_PATCH_LEVEL @KERNEL_VERSION_PATCH_LEVEL@<br>
<br>
#define COPYRIGHT_YEAR "@COPYRIGHT_YEAR@"<br>
<br>
/* KERNEL_VERSION_BUILD_TYPE is L"SVN", L"RC1", L"RC2" or L"" (for the release) */<br>
#define KERNEL_VERSION_BUILD_TYPE "@KERNEL_VERSION_BUILD_TYPE@"<br>
<br>
<br>
#endif<br>
<br>
/* EOF */<br>
</code></pre>

<h1>Load ReactOS.sln</h1>
Open up Visual Studio, and load a new project (<b>.sln</b> files are called called solution files).  Navigate to the folder you put the source code in.  Navigate further to reactos\reactos\reactos.sln and open it.  This is an operating system, remember, so expect this to take a couple minutes.  Once it's done, you're ready to make some changes!<br>
<br>
If you want to perform a test of compiling ReactOS, there's a couple changes you need to make.  Go to Build -> Configuration Manager.  Make sure that Active solution configuration is set to "Debug" and the Active solution platform to "x86_32".<br>
<br>
<img src='https://reactos.googlecode.com/svn/wiki/VS%20Setup%202%20-%20en.png' />

<h1>Create Visual Studio Projects</h1>

<h2>Cheat Sheet</h2>

The following cheat sheet allows you to provide immediate, valuable contribution to this project without understanding the entirety of Visual Studio IDE options or the intricacies of the ReactOS source code hierarchy.<br>
<br>
<ol><li>Create a <b><code>.vcxproj</code></b> file corresponding to an empty project, making sure that the <b><code>.vcxproj</code></b> file falls within the correct directory. For example, if you are creating the <b><code>.vcxproj</code></b> file for <b><code>calc.exe</code></b>, the <b><code>.vcxproj</code></b> should have the path:<br><b><code>reactos\base\applications\calc\calc.vcxproj</code></b><br>Actually, it does not matter whether you create an empty project, a <b><code>.DLL</code></b> project, an <b><code>.EXE</code></b> project, or whatever, because you can easily change the project type later without consequence.<br>
</li><li>Define <b><code>Configuration Properties-&gt;General-&gt;Configuration Type</code></b> appropriately, depending upon whether you are creating the <b><code>.vcxproj</code></b> file for a <b><code>.EXE</code></b>, <b><code>.LIB</code></b>, <b><code>.DLL</code></b>, etc. Note that <b><code>.SYS</code></b> executables are nothing more than <b><code>.DLL</code></b>'s with a few bits tweaked in their headers.<br>
</li><li>Define <b><code>Configuration Properties-&gt;General-&gt;Output Directory</code></b>, otherwise known as <b><code>$(OutDir)</code></b>, to be <b><code>build\x86_32\$(Configuration)</code></b>.<br>
</li><li>Define <b><code>Configuration Properties-&gt;General-&gt;Intermediate Directory</code></b>, otherwise known as <b><code>$(IntDir)</code></b>, to be <b><code>build\x86_32\$(Configuration)</code></b>.<br>
</li><li>Define <b><code>Configuration Properties-&gt;C/C++-&gt;Preprocessor-&gt;Ignore Standard Include Paths</code></b> to be <b><code>Yes (/X)</code></b>.<br>
</li><li>Define <b><code>Configuration Properties-&gt;Linker-&gt;Input-&gt;Additional Dependencies</code></b>, to be empty, if appropriate.<br>
</li><li>Define <b><code>Configuration Properties-&gt;Linker-&gt;Input-&gt;Ignore All Default Libraries</code></b> to be <b><code>YES (/NODEFAULTLIB)</code></b>, if appropriate.<br>
</li><li>Define <b><code>Configuration Properties-&gt;Librarian-&gt;General-&gt;Additional Dependencies</code></b>, to be empty, if appropriate.<br>
</li><li>Define <b><code>Configuration Properties-&gt;Librarian-&gt;General-&gt;Ignore All Default Libraries</code></b> to be <b><code>YES (/NODEFAULTLIB)</code></b>, if appropriate.<br>
</li><li>After the <b><code>.vcxproj</code></b> has been completed, you will see that <b><code>BUILD-&gt;Configuration Manager-&gt;Active solution platforms</code></b> now includes a 4th platform: <b><code>Win32</code></b>. This is bad. We do not use the name <b><code>Win32</code></b> to denote the Intel 32-bit platform. Instead, we use <b><code>x86_32</code></b>. Delete the <b><code>Win32</code></b> platform that was just created. Be careful not to accidentally delete <b><code>x86_32</code></b>, <b><code>x86_64</code></b>, or <b><code>ARM</code></b>!!!<br>
</li><li>At this point, you have done enough to take a break. Use TortoiseSVN to check-in your new project so that your new <b><code>ReactOS.sln</code></b>, which has been modified due to your creating the new project, can be seen by everyone else.</li></ol>

<h2>CMakeLists.txt</h2>
<h3>A Command-Line Tool</h3>
<h3>Message Codes (bugcodes.h)</h3>
<h3>ks386.inc</h3>
<h3>Boot Sector Image</h3>
<h3>Free Loader</h3>
<h3>An Import Library</h3>
<h3>NTOSKRNL.EXE</h3>
<h3>HAL.DLL</h3>
<h3>KDCOM.DLL</h3>
<h3>Bootvid.DLL</h3>
<h3>Registry Hives</h3>
<h3>A Kernel-Mode Driver</h3>
<h3>A User-Mode DLL</h3>
<h3>A User-Mode EXE</h3>
Our example for creating a user-mode <b><code>.EXE</code></b> is <b><code>calc.exe</code></b>.<br>
Use <b><code>Explorer.exe</code></b> to locate the source code for <b><code>calc.exe</code></b>. It is the directory at:<br>
<b><code>reactos\base\applications\calc</code></b>:<br>
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/1.png' />
Open its <b><code>CMakeLists.txt</code></b> file to determine what we should do to create <b><code>calc.vcxproj</code></b>:<br>
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/2.png' /><br>
Looking at the <b><code>CMakLists.txt</code></b> file, we make the following determinations:<br>
<pre><code>add_definitions(-DDISABLE_HTMLHELP_SUPPORT=1)<br>
</code></pre>
…means that we will have to add a <b><code>#define</code></b> to the IDE settings that effectively does this for all <b><code>.c/.cpp/etc.</code></b> files:<br>
<pre><code>#define DDISABLE_HTMLHELP_SUPPORT 1<br>
</code></pre>
<pre><code>add_executable(calc<br>
</code></pre>
…shows us that there are seven source files that will be compiled to generate <b><code>calc.exe</code></b>. They are:<br>
<pre><code>    about.c<br>
    convert.c<br>
    function.c<br>
    rpn.c<br>
    utl.c<br>
    winmain.c<br>
    resource.rc<br>
</code></pre>
<pre><code>set_module_type(calc win32gui UNICODE)<br>
</code></pre>
…tells us that the output of the build of the project will be a Windows 32-bit <b><code>.EXE</code></b> that has a GUI (unlike a console application). It will use <b><code>UNICODE</code></b>, so throughout the source code, <b><code>TCHAR</code></b> will be defined as <b><code>WCHAR</code></b> and not <b><code>CHAR</code></b>.<br>
<pre><code>add_importlibs(calc advapi32 user32 gdi32 msvcrt kernel32)<br>
</code></pre>
…tells us that <b><code>calc.exe</code></b> will depend upon several <b><code>.DLL</code></b>’s to execute, and therefore must be linked with their corresponding <b><code>.LIB</code></b> files at build time.<br>
<pre><code>if(MSVC)<br>
    add_importlibs(calc ntdll)<br>
endif()<br>
</code></pre>
…tells us that, if our compiler is Microsoft Visual C, then <b><code>calc.exe</code></b> will also depend on <b><code>ntdll.dll</code></b>, and therefore, must be linked with the import library that corresponds to <b><code>ntdll.dll</code></b>.<br>
<pre><code>add_pch(calc calc.h)<br>
</code></pre>

…tells us that the header file that would be used to generate a pre-compiled header (<b><code>.pch</code></b>) file is <b><code>calc.h</code></b>. Since <b><code>calc.exe</code></b> is a very small program in terms of number of lines of code, we forego utilization the pre-compiled header compilation optimization.<br>
<pre><code>add_cd_file(TARGET calc DESTINATION reactos/system32 FOR all)<br>
</code></pre>
..tells us that, after <b><code>calc.exe</code></b> is built, if we choose to place it on the OS CD, it should be copied to:<br>
<b><code>reactos/system32</code></b><br>
We must remember to modify the <b><code>CD_Live</code></b> project accordingly, so that <b><code>calc.exe</code></b> is included in the CD-generation process when building <b><code>CD_Live</code></b>.<br>
Normally, we would be able to add <b><code>calc.vcxproj</code></b> to <b><code>reactos.sln</code></b> by right-clicking in <b><code>Solution Explorer</code></b> the node in the hierarchy that is to be the parent of <b><code>calc.vcxproj</code></b>. However, with Visual Studio 2012 Express, you will find that Microsoft has disallowed choosing the initial location within the hierarchy, and instead, forces the generation of <b><code>calc</code></b> at an arbitrary location: as a child of the root of the solution.<br>
No matter. Right-click on the root of the solution hierarchy in <b><code>Solution Explorer</code></b> and do <b><code>Add-&gt;New Project…</code></b>
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/3.png' /><br>
A dialog box will give you the choice of which type of Visual C++ project you would like to add. Since <b><code>calc.exe</code></b> is a Win32 GUI executable, choosing <b><code>Win32 Project</code></b> might seem appropriate, but it turns out that it does not matter which of these you choose, as the project type can be easily changed later. For purpose of illustration, we choose <b><code>Empty Project</code></b>.<br>
We choose the name <b><code>calc</code></b>, which causes Visual Studio to choose the name of the <b><code>.vcxproj</code></b> file to be <b><code>calc.vcxproj</code></b>.<br>
We very carefully choose the <b><code>Location:</code></b> on disk where the <b><code>calc.vcxproj</code></b> file should be placed by Visual Studio. Visual Studio knows that the name of the project is <b><code>calc</code></b>, so it creates a directory called <b><code>calc</code></b> somewhere, no matter what we do. We cannot stop it from creating a directory. Therefore, the <b><code>Location:</code></b> option should be the parent directory of the directory <b><code>calc</code></b> that will be created. When trying to create the <b><code>calc</code></b> directory, Visual Studio will see that it already exists, and proceed as if it did not. Then, <b><code>calc.vcxproj</code></b> will be placed inside the <b><code>calc</code></b> directory. <br>This is the process that we keep in mind as we overlay <b><code>.vcxproj</code></b> files onto the extant ReactOS source code hierarchy: Visual Studio would normally attempt to create a directory that, in general, will already exist.<br>
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/4.png' /><br>
After creating <b><code>calc</code></b>, we use <b><code>Explorer.exe</code></b> to verify that the <b><code>calc</code></b> direcotry and other Visual Studio files are as they should be:<br>
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/5.png' /><br>
The new project, <b><code>calc</code></b>, now appears in <b><code>Solution Explorer</code></b>, but because we are using a free version of Visual Studio 2012, Microsoft successfully annoys us by placing <b><code>calc</code></b> project as the child of the root of the solution:<br>
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/6.png' /><br>
The root of the solution is obviously not the correct location. The correct parent for <b><code>calc</code></b> is a <b><code>Solution Explorer</code></b> folder called <b><code>applications</code></b>. In other words, <b><code>calc</code></b> should be a sibling of <b><code>atactl</code></b>:<br>
<br>
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/7.png' />
Normally, with a paid version of Visual Studio, we would be able to:<br>
<ol><li>Right-click on <b><code>calc</code></b>.<br>
</li><li>Select <b><code>cut</code></b>.<br>
</li><li>Right-click on <b><code>applications</code></b>.<br>
</li><li>Select <b><code>paste</code></b>.<br></li></ol>

…and <b><code>calc</code></b> would be moved to its correct location. However, cut-and-paste of projects is not supported in VS2012 Express, so we must drag-and-drop <b><code>calc</code></b> to <b><code>applications</code></b>:<br>
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/8.png' />

At this point, <b><code>calc.vcxproj</code></b> exists on disk, but <b><code>calc.vcxproj</code></b> does not reference any source files <b><code>(.c/.h/etc.)</code></b>. We must add the files to the project. We expand-out <b><code>calc</code></b> to see the default solution filters for <b><code>calc</code></b>. There are three of them, plus the pseudo-filter <b><code>External Dependencies</code></b> that can neither be created nor deleted by us:<br>

<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/9.png' />

<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/10.png' />

<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/11.png' />
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/12.png' />
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/13.png' />

<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/14.png' />
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/15.png' />
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/16.png' />
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/17.png' />
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/18.png' />
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/19.png' />


<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/20.png' />


<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/21.png' />

<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/22.png' />


<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/23.png' />


<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/24.png' />


<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/25.png' />
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Create_Project_For_User_Mode_EXE/26.png' />
<h2>Project Properties</h2>
<h3>Common Properties</h3>
<h4>Framework and References</h4>

While creating <b><code>atactl.vcxproj</code></b>, the following linker error occurs.<br>
<br>
<pre><code>1&gt;------ Build started: Project: atactl, Configuration: Debug Win32 ------<br>
1&gt;LINK : error LNK2001: unresolved external symbol _mainCRTStartup<br>
1&gt;build\x86_32\Debug\atactl.exe : fatal error LNK1120: 1 unresolved externals<br>
========== Build: 0 succeeded, 1 failed, 20 up-to-date, 0 skipped ==========<br>
</code></pre>

We know that <b><code>_mainCRTStartup</code></b> has to do with the C run-time library. The <b><code>CMakeLists.txt</code></b> file that corresponds to <b><code>atactl.vcxproj</code></b> is as follows:<br>
<br>
<pre><code><br>
set_cpp()<br>
<br>
add_definitions(-DUSER_MODE)<br>
include_directories(${REACTOS_SOURCE_DIR}/drivers/storage/ide/uniata)<br>
add_executable(atactl atactl.cpp atactl.rc)<br>
set_module_type(atactl win32cui)<br>
add_importlibs(atactl advapi32 msvcrt kernel32 ntdll)<br>
<br>
if(NOT MSVC)<br>
    # FIXME: http://www.cmake.org/Bug/view.php?id=12998<br>
    #allow_warnings(atactl)<br>
    set_source_files_properties(atactl.cpp PROPERTIES COMPILE_FLAGS "-Wno-error")<br>
endif()<br>
<br>
add_cd_file(TARGET atactl DESTINATION reactos/system32 FOR all)<br>
</code></pre>

We check our <b><code>References</code></b> to make sure that we have not forgotten to link in a CRT-related library:<br>
<br>
All looks good, yet the error is still there, and the file where <b><code>_mainCRTStartup</code></b> is defined is not in the only possible supplicant: <b><code>msvcrt.dll</code></b>.<br>
<br>
It turns out that there is a bug in the <b><code>CMakeLists.txt</code></b> scheme. Apparently, a dependence upon <b><code>msvcrt.dll</code></b> implies a dependence upon <b><code>msvcrtex.lib</code></b>, which we specify, to solve the problem:<br>
<br>
<br>
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Project_Properties/References-_mainCRTStartup.png' />




<h3>Configuration Properties</h3>
<h4>General</h4>
<h5>Output Directory</h5>
<h5>Intermediate Directory</h5>
<h5>Configuration Type</h5>
<h5>Character Set</h5>
<h4>Debugging</h4>
<h4>VC++ Directories</h4>
<h4>C/C++</h4>
<h5>General</h5>
<h6>Additional Include Directories</h6>
<h6>Resolve #using References</h6>
<h6>Debug Information Format</h6>

1>advapi32_stubs.obj : warning LNK4075: ignoring '/EDITANDCONTINUE' due to '/OPT:LBR' specification<br>
<h6>Common Language RunTime Support</h6>
<h6>Consume Windows Runtime Extension</h6>
<h6>Suppress Startup Banner</h6>
<h6>Warning Level</h6>
<h6>Treat Warnings As Errors</h6>
<h6>Multi-processor Compilation</h6>
User Unicode For Assembler Listing<br>
<h5>Optimization</h5>
<h6>Optimization</h6>
<h6>Inline Function Expansion</h6>
<h6>Enable Intrinsic Functions</h6>
<h6>Favor Size Or Speed</h6>
<h6>Omit Frame Pointers</h6>
<h6>Enable Fiber-Safe Optimizations</h6>
<h6>Whole Program Optimization</h6>
<h5>Preprocessor</h5>
<h6>Preprocessors Definitions</h6>

<pre><code>1&gt;------ Build started: Project: msvcrt, Configuration: Debug Win32 ------<br>
1&gt;  stubs.c<br>
1&gt;c:\users\john\desktop\google code\reactos\reactos\dll\win32\msvcrt\stubs.c(53): error C3861: '_inp': identifier not found<br>
1&gt;c:\users\john\desktop\google code\reactos\reactos\dll\win32\msvcrt\stubs.c(59): error C3861: '_inpw': identifier not found<br>
1&gt;c:\users\john\desktop\google code\reactos\reactos\dll\win32\msvcrt\stubs.c(65): error C3861: '_inpd': identifier not found<br>
1&gt;c:\users\john\desktop\google code\reactos\reactos\dll\win32\msvcrt\stubs.c(73): error C3861: '_outp': identifier not found<br>
1&gt;c:\users\john\desktop\google code\reactos\reactos\dll\win32\msvcrt\stubs.c(80): error C3861: '_outpw': identifier not found<br>
1&gt;c:\users\john\desktop\google code\reactos\reactos\dll\win32\msvcrt\stubs.c(87): error C3861: '_outpd': identifier not found<br>
</code></pre>

<pre><code>int MSVCRT__inp(<br>
   unsigned short port)<br>
{<br>
    return _inp(port);<br>
}<br>
</code></pre>

conio.h shows:<br>
<br>
<pre><code>#if defined(_M_IX86) || defined(_M_X64)<br>
int __cdecl _inp(unsigned short);<br>
unsigned short __cdecl _inpw(unsigned short);<br>
unsigned long __cdecl _inpd(unsigned short);<br>
#endif /* _M_IX86 || _M_X64 */<br>
</code></pre>

<pre><code>_inp=__inbyte<br>
_inpw=__inword<br>
_inpd=__indword<br>
</code></pre>

Don’t forget to define WIN32_LEAN_AND_MEAN to speed-up edit-time parsing.<br>
Undefine Preprocessor Definitions<br>
Undefine All Preprocessor Definitions<br>
Ignore Standard Include Paths<br>
Preprocess to a File<br>
Preprocess Suppress Line Numbers<br>
Keep Comments<br>
<br>
<h5>Code Generation</h5>
<h6>Enable String Pooling</h6>
<h6>Enable Minimal Rebuild</h6>
<h6>Enable C++ Exceptions</h6>
<h6>Smaller Type Check</h6>
<h6>Basic Runtime Checks</h6>
<h6>Runtime Library</h6>

While creating <b><code>ftfd.vcxproj</code></b>, the following linker errors occurred:<br>
<br>
<pre><code>&gt;freetype.lib(ftbase.obj) : error LNK2019: unresolved external symbol __imp__strrchr referenced in function _raccess_make_file_name<br>
1&gt;freetype.lib(sfnt.obj) : error LNK2019: unresolved external symbol __imp__memchr referenced in function _tt_face_find_bdf_prop<br>
1&gt;freetype.lib(ttobjs.obj) : error LNK2019: unresolved external symbol __imp__strstr referenced in function _tt_check_trickyness_family<br>
1&gt;build\x86_32\Debug\ftfd.dll : fatal error LNK1120: 4 unresolved externals<br>
========== Build: 0 succeeded, 1 failed, 8 up-to-date, 0 skipped ==========<br>
</code></pre>

Since <b><code>ftfd.dll</code></b> does not dynamically link with any module that could possibly export <b><code>memchr</code></b>, something is obviously wrong with importation. Choosing the function <b><code>_tt_face_find_bdf_prop</code></b> arbitrarily for investigation, we discover that it uses a macro <b><code>ft_memchr</code></b> in the file:<br>
<br>
<b><code>reactos\lib\3rdparty\freetype\src\sfnt\ttbdf.c</code></b>:<br>
<br>
<pre><code><br>
          switch ( type &amp; 0x0F )<br>
          {<br>
          case 0x00:  /* string */<br>
          case 0x01:  /* atoms */<br>
            /* check that the content is really 0-terminated */<br>
            if ( value &lt; bdf-&gt;strings_size &amp;&amp;<br>
                 ft_memchr( bdf-&gt;strings + value, 0, bdf-&gt;strings_size ) )<br>
            {<br>
              aprop-&gt;type   = BDF_PROPERTY_TYPE_ATOM;<br>
              aprop-&gt;u.atom = (const char*)bdf-&gt;strings + value;<br>
              error         = SFNT_Err_Ok;<br>
              goto Exit;<br>
            }<br>
            break;<br>
</code></pre>

<b><code>reactos\lib\3rdparty\freetype\src\sfnt\ttbdf.c</code></b>

...includes...<br>
<br>
<b><code>reactos\lib\3rdparty\freetype\include\freetype\config\ftstdlib.h</code></b>, which makes the following definitions:<br>
<br>
<pre><code>#define ft_memchr   memchr<br>
#define ft_memcmp   memcmp<br>
#define ft_memcpy   memcpy<br>
#define ft_memmove  memmove<br>
#define ft_memset   memset<br>
#define ft_strcat   strcat<br>
#define ft_strcmp   strcmp<br>
#define ft_strcpy   strcpy<br>
#define ft_strlen   strlen<br>
#define ft_strncmp  strncmp<br>
#define ft_strncpy  strncpy<br>
#define ft_strrchr  strrchr<br>
#define ft_strstr   strstr<br>
</code></pre>

Intellisense tells us that <b><code>memchr</code></b> has the following declaration:<br>
<br>
<pre><code>  _Must_inspect_result_<br>
  _CRTIMP<br>
  _CONST_RETURN<br>
  void*<br>
  __cdecl<br>
  memchr(<br>
    _In_reads_bytes_opt_(_MaxCount) const void *_Buf,<br>
    _In_ int _Val,<br>
    _In_ size_t _MaxCount);<br>
</code></pre>

Using Intellisense to hunt down <b><code>_CRTIMP</code></b> reveals its declaration as:<br>
<br>
<pre><code>#ifndef _CRTIMP<br>
 #ifdef CRTDLL /* Defined for ntdll, crtdll, msvcrt, etc */<br>
  #define _CRTIMP<br>
 #elif defined(_DLL)<br>
  #define _CRTIMP __declspec(dllimport)<br>
 #else /* !CRTDLL &amp;&amp; !_DLL */<br>
  #define _CRTIMP<br>
 #endif /* CRTDLL || _DLL */<br>
#endif /* !_CRTIMP */<br>
</code></pre>

And we see that <b><code>_DLL</code></b> is defined, even though <b><code>freetype.lib</code></b> is being built as a static library! This is abnormal, so we inspect the compiler command-line for <b><code>freetype.lib</code></b>:<br>
<br>
<pre><code>/X /GS- /analyze- /W3 /Zc:wchar_t /I"include" /I"." /I"C:\Users\John\Desktop\Google Code\reactos\reactos\\include\crt" /I"C:\Users\John\Desktop\Google Code\reactos\reactos\\include\psdk" /ZI /Gm /Od /Fd"build\x86_32\Debug\vc110.pdb" /fp:precise /D "__REACTOS__" /D "DBG=1" /D "_SEH_ENABLE_TRACE" /D "_WINKD_=1" /D "WINVER=0x502" /D "_WIN32_IE=0x600" /D "_WIN32_WINNT=0x502" /D "_WIN32_WINDOWS=0x502" /D "_SETUPAPI_VER=0x502" /D "_M_IX86" /D "_X86_" /D "__i386__" /D "i386" /D "USE_COMPILER_EXCEPTIONS" /D "_USE_32BIT_TIME_T" /D "WIN32" /D "_WINDOWS" /D "inline=__inline" /D "__STDC__=1" /D "LANGUAGE_EN_US" /D "__NTDRIVER__" /D "FT2_BUILD_LIBRARY" /D "CMAKE_INTDIR=\"Debug\"" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /arch:IA32 /Gd /Oy- /MDd /Fa"build\x86_32\Debug\" /EHsc /nologo /Fo"build\x86_32\Debug\" /Fp"build\x86_32\Debug\freetype.pch"<br>
</code></pre>

None of those command-line arguments indicate that <b><code>_DLL</code></b> is defined. This is very puzzling until we vaguely remember something about <b><code>_DLL</code></b> being implicitly defined depending on how we define another option: <b><code>Runtime Library</code></b> on <a href='http://msdn.microsoft.com/en-us/library/2kzt1wy3.aspx'>MSDN</a>:<br>
<br>
<b><code>/MD</code></b>
<blockquote>Causes your application to use the multithread- and DLL-specific   version of the run-time library. Defines <i>MT and</i>DLL and causes the compiler to place the library name MSVCRT.lib into the .obj file.<br>
Applications compiled with this option are statically linked to MSVCRT.lib. This library provides a layer of code that allows the linker to resolve external references. The actual working code is contained in MSVCR100.DLL, which must be available at run time to applications linked with MSVCRT.lib.</blockquote>

The solution is to change the compiler settings for the <b><code>Runtime Library</code></b> setting, even though we are not using a run-time library, to prevent <b><code>_DLL</code></b> from being implicitly defined:<br>
<br>
This...<br>
<br>
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Project_Properties/Run-Time-Library-DLL.png' />

...should be changed to this...<br>
<br>
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Project_Properties/Run-Time-Library-No-DLL.png' />

While creating <b><code>msvcrt.vcxproj</code></b>, the following linker warnings occurred:<br>
<br>
<pre><code>1&gt;     Creating library build\x86_32\Debug\msvcrt.lib and object build\x86_32\Debug\msvcrt.exp<br>
1&gt;dllmain.obj : warning LNK4217: locally defined symbol __strdup imported in function _DllMain@12<br>
1&gt;dllmain.obj : warning LNK4217: locally defined symbol __wcsdup imported in function _DllMain@12<br>
1&gt;  msvcrt.vcxproj -&gt; C:\Users\John\Desktop\Google Code\reactos\reactos\dll\win32\msvcrt\build\x86_32\Debug\msvcrt.dll<br>
</code></pre>

The source of these errors is, again, the phantom <b><code>_DLL</code></b> problem as just described, and the solution is the same.<br>
<br>
<h6>Struct Member Alignment</h6>
<h6>Security Check</h6>
While creating <b><code>atactl.vcxproj</code></b>, the following linker errors occurred:<br>
<br>
<pre><code>1&gt;------ Build started: Project: atactl, Configuration: Debug Win32 ------<br>
1&gt;  atactl.cpp<br>
1&gt;atactl.obj : error LNK2019: unresolved external symbol ___report_rangecheckfailure referenced in function "unsigned char __cdecl ata_bblk(int,int,int)" (?ata_bblk@@YAEHHH@Z)<br>
1&gt;atactl.obj : error LNK2019: unresolved external symbol @__security_check_cookie@4 referenced in function "unsigned char __cdecl ata_adapter_info(int,int)" (?ata_adapter_info@@YAEHH@Z)<br>
1&gt;atactl.obj : error LNK2019: unresolved external symbol ___security_cookie referenced in function "unsigned char __cdecl ata_adapter_info(int,int)" (?ata_adapter_info@@YAEHH@Z)<br>
</code></pre>

The references to <i>security cookies</i> probably has something to with the <b><code>security check</code></b> option. It appears that security check is currently enabled:<br>
<br>
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Project_Properties/security_check_enabled.png' />

Disabling it removes the compiler-generated references:<br>
<br>
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Project_Properties/security_check_disabled.png' />

Disabling it also resolves the <b><code>___report_rangecheckfailure</code></b> error.<br>
<br>
<h6>Enable Function-Level Linking</h6>
<h6>Enable Enhanced Instruction Set</h6>
<h6>Enable Parallel Code Generation</h6>
<h6>Enable Enhanded Instruction Set</h6>
<h6>Floating Point Model</h6>
<h6>Enable Floating Point Exceptions</h6>
<h6>Create Hotpatchable Image</h6>


<h5>Language</h5>
<h6>Disable Language Extensions</h6>
<h6>Treat WChart_t As Built in Typye</h6>
<h6>Force Conformance in For Loop Scope</h6>
<h6>Enable Run-Time Type Information</h6>
<h6>Open MP Support</h6>

<h5>Precompiled Headers</h5>
<h6>Precompiled Header</h6>
<h6>Precompiled Header File</h6>
<h6>Precompiled Header Output File</h6>

<h5>Output Files</h5>
<h6>Expand Attributed Source</h6>
<h6>Assembler Output</h6>
<h6>Use Unicode For Assembler Listing</h6>
<h6>ASM List Location</h6>
<h6>Object File Name</h6>
<h6>Program Database File Name</h6>
<h6>Generate XML Document Files</h6>
<h6>XML Documentation File Name</h6>

<h5>Browse Information</h5>
<h6>Enable Browse Information</h6>
<h6>Browse Information File</h6>

<h5>Advanced</h5>
<h6>Calling Convention</h6>
<h6>Compile As</h6>
<h6>Disable Specific Warnings</h6>
<h6>Forced Include File</h6>
While creating <b><code>imm.vcxproj</code></b>, the following compiler errors occurred:<br>
<br>
<pre><code>1&gt;------ Build started: Project: imm32, Configuration: Debug Win32 ------<br>
1&gt;  imm.c<br>
1&gt;reactos\dll\win32\imm32\imm.c(58): error C2061: syntax error : identifier 'typeof'<br>
1&gt;reactos\dll\win32\imm32\imm.c(59): error C2143: syntax error : missing '{' before '*'<br>
1&gt;reactos\dll\win32\imm32\imm.c(60): error C2143: syntax error : missing '{' before '*'<br>
</code></pre>

The offending code in <b><code>imm.c</code></b> shows us that <b><code>typeof</code></b> is being called by the macro <b><code>MAKE_FUNCPTR</code></b>:<br>
<br>
<pre><code>#define MAKE_FUNCPTR(f) typeof(f) * p##f<br>
typedef struct _tagImmHkl{<br>
    struct list entry;<br>
    HKL         hkl;<br>
    HMODULE     hIME;<br>
    IMEINFO     imeInfo;<br>
    WCHAR       imeClassName[17]; /* 16 character max */<br>
    ULONG       uSelected;<br>
    HWND        UIWnd;<br>
<br>
    /* Function Pointers */<br>
    MAKE_FUNCPTR(ImeInquire);<br>
    MAKE_FUNCPTR(ImeConfigure);<br>
    MAKE_FUNCPTR(ImeDestroy);<br>
    MAKE_FUNCPTR(ImeEscape);<br>
    MAKE_FUNCPTR(ImeSelect);<br>
</code></pre>

Intellisense tells us that <b><code>typeof</code></b> is not defined, and we know that <b><code>typeof</code></b> is certainly not a valid <b><code>C</code></b> keyword. Fortunately, we had the good-sense to use the <b><code>CmakeLists.txt</code></b> file as our guide, and remember seeing something like this in the <b><code>CmakeLists.txt</code></b> file:<br>
<br>
<pre><code>if(MSVC)<br>
    # FIXME: http://www.cmake.org/Bug/view.php?id=12998<br>
    #add_target_compile_flags(imm32 "/FIwine/typeof.h")<br>
    set_source_files_properties(${SOURCE} PROPERTIES COMPILE_FLAGS "/FIwine/typeof.h")<br>
endif()<br>
</code></pre>

The <b><code>"/FIwine/typeof.h"</code></b> tells us that we need to force the compiler to do an inclusion of <b><code>typeof.h</code></b> into all <code>.c</code> files in the project, which undoubtedly contains a definition for <b><code>typeof</code></b>:<br>
<br>
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Project_Properties/force_include.png' />



<h6>Force #using File</h6>
<h6>Show Includes</h6>
<h6>Use Full Paths</h6>
<h6>Omit Default Library Name</h6>
<h6>Internal Compiler Error Reporting</h6>
<h6>Treat Specific Warnings As Errors</h6>


<h5>Command Line</h5>
<h4>Linker</h4>
<h5>General</h5>
<h6>Output File</h6>
<h6>Show Progress</h6>
<h6>Version</h6>
<h6>Enable Incremental Linking</h6>
<h6>Suppress Startup Banner</h6>
<h6>Ignore Import Library</h6>
<h6>Register Output</h6>
<h6>Per-user Redirection</h6>
<h6>Additional Library Directories</h6>
<h6>Link Library Dependencies</h6>
<h6>Use Library Dependency Inputs</h6>
<h6>Link Status</h6>
<h6>Prevent Dll Binding</h6>
<h6>Treat Linker Warning As Errors</h6>
<h6>Force File Output</h6>
<h6>Create Hot Patchable Image</h6>
<h6>Specify Section Attributes</h6>
<h5>Input</h5>
<h6>Additional Dependencies</h6>
<h6>Ignore All Default Libraries</h6>
<h6>Ignore Specific Default Libraries</h6>
<h6>Module Definition File</h6>
<h6>Add Module to Assembly</h6>
<h6>Embed Managed Resource File</h6>
<h6>Force Symbol References</h6>
<h6>Delay Loaded Dlls</h6>
<h6>Assembly Link Resource</h6>
<h5>Manifest File</h5>
<h6>Generate Manifest</h6>
<h6>Manifest File</h6>
<h6>Allow Isolation</h6>
<h6>Enable User Account control (UAC)</h6>
<h6>UAC Execution Level</h6>
<h6>UAC Bypass UI Protection</h6>

<h5>Debugging</h5>
<h6>Generate Debug Info</h6>
<h6>Generate Program Database File</h6>
<h6>Strip Private Symbols</h6>
<h6>Generate Map File</h6>
<h6>Map File Name</h6>
<h6>Map Exports</h6>
<h6>Debuggable Assembly</h6>
<h5>System</h5>
<h6>Sub System</h6>
<h6>Minimum Required Version</h6>
<h6>Heap Reserve Size</h6>
<h6>Heap Commit Size</h6>
<h6>Stack Reserve Size</h6>
<h6>Stack Commit Size</h6>
<h6>Enable Large Addresses</h6>
<h6>Terminal Server</h6>
<h6>Swap Run From CD</h6>
<h6>Swap Run From Network</h6>
<h6>Driver</h6>
<h5>Optimization</h5>
<h6>References</h6>
<h6>Enable COMDAT Folding</h6>
<h6>Function Order</h6>
<h6>Profile Guided  Database</h6>
<h6>Link Time Code Generation</h6>
<h5>Embedded IDL</h5>
<h6>MIDL Commands</h6>
<h6>Ignore Embedded IDL</h6>
<h6>Merged IDL Base File Name</h6>
<h6>Type Library</h6>
<h6>Typelib Resource ID</h6>
<h5>Windows Metadata</h5>
<h5>Advanced</h5>
<h6>Entry Point</h6>

While creating <b><code>msimg32.vcxpro</code></b>, the following linker error occurs:<br>
<br>
<pre><code>1&gt;------ Build started: Project: msimg32, Configuration: Debug Win32 ------<br>
1&gt;     Creating library build\x86_32\Debug\msimg32.lib and object build\x86_32\Debug\msimg32.exp<br>
1&gt;LINK : error LNK2001: unresolved external symbol __DllMainCRTStartup@12<br>
1&gt;build\x86_32\Debug\msimg32.dll : fatal error LNK1120: 1 unresolved externals<br>
========== Build: 0 succeeded, 1 failed, 26 up-to-date, 0 skipped ==========<br>
</code></pre>

In other words, the linker is unhappy because it it needs the symbol, <b><code>__DllMainCRTStartup@12</code></b>, to finish the linking process. We check the <b><code>CMakeLists.txt</code></b> file that corresponds to <b><code>msimg32.vcxproj</code></b>...<br>
<pre><code><br>
include_directories(${REACTOS_SOURCE_DIR}/include/reactos/wine)<br>
add_definitions(-D__WINESRC__)<br>
<br>
spec2def(msimg32.dll msimg32.spec ADD_IMPORTLIB)<br>
<br>
list(APPEND SOURCE<br>
    msimg32_main.c<br>
    ${CMAKE_CURRENT_BINARY_DIR}/msimg32.def)<br>
<br>
add_library(msimg32 SHARED ${SOURCE})<br>
<br>
set_module_type(msimg32 win32dll)<br>
target_link_libraries(msimg32 wine)<br>
add_importlibs(msimg32 gdi32 msvcrt kernel32 ntdll)<br>
add_cd_file(TARGET msimg32 DESTINATION reactos/system32 FOR all)<br>
</code></pre>

...and see that it only prescribes compilation/linking of one <b><code>.c</code></b> file, namely <b><code>msimg32_main.c</code></b>, which, itself, is quite simple, and certainly does not contain any references to anything like <b><code>DllMainCRTStartup</code></b>:<br>
<br>
<pre><code><br>
#define WIN32_NO_STATUS<br>
<br>
#include &lt;stdarg.h&gt;<br>
<br>
#include &lt;windef.h&gt;<br>
#include &lt;winbase.h&gt;<br>
//#include "wingdi.h"<br>
//#include "winerror.h"<br>
#include &lt;wine/debug.h&gt;<br>
<br>
WINE_DEFAULT_DEBUG_CHANNEL(msimg32);<br>
<br>
/***********************************************************************<br>
 *           DllInitialize (MSIMG32.@)<br>
 *<br>
 * MSIMG32 initialisation routine.<br>
 */<br>
BOOL WINAPI DllMain( HINSTANCE inst, DWORD reason, LPVOID reserved )<br>
{<br>
    if (reason == DLL_PROCESS_ATTACH) DisableThreadLibraryCalls( inst );<br>
    return TRUE;<br>
}<br>
<br>
<br>
/******************************************************************************<br>
 *           vSetDdrawflag   (MSIMG32.@)<br>
 */<br>
void WINAPI vSetDdrawflag(void)<br>
{<br>
    static unsigned int vDrawflag=1;<br>
    FIXME("stub: vSetDrawFlag %u\n", vDrawflag);<br>
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);<br>
}<br>
</code></pre>

However, the <b><code>CMakeLists.txt</code></b> file <i>does</i> prescribe a dependency upon <b><code>msvcrt</code></b>, and, implicitly, <b><code>msvcrtex</code></b>, and we know that <b><code>msvcrtex</code></b> contains a file, <b><code>crtdll.c</code></b> that defines a some functions that are somewhat like <b><code>DllMainCRTStartup</code></b>:<br>
<br>
<pre><code><br>
static WINBOOL __DllMainCRTStartup (HANDLE, DWORD, LPVOID);<br>
<br>
WINBOOL WINAPI DllMainCRTStartup (HANDLE, DWORD, LPVOID);<br>
int __mingw_init_ehandler (void);<br>
<br>
WINBOOL WINAPI<br>
DllMainCRTStartup (HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)<br>
{<br>
  mingw_app_type = 0;<br>
  if (dwReason == DLL_PROCESS_ATTACH)<br>
    {<br>
      __security_init_cookie ();<br>
#ifdef _WIN64<br>
      __mingw_init_ehandler ();<br>
#endif<br>
    }<br>
  return __DllMainCRTStartup (hDllHandle, dwReason, lpreserved);<br>
}<br>
<br>
__declspec(noinline) WINBOOL<br>
__DllMainCRTStartup (HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)<br>
</code></pre>

The only problem is that the functions in <b><code>crtdll.c</code></b> are either preceded by no underbars or two underbars. Therefore, after decoration, these functions in <b><code>crtdll.c</code></b> will have either one or three underbars, as can be seen in the dump of the symbols from <b><code>msvcrtex</code></b> using the <b><code>DUMPBIN.EXE</code></b> utility:<br>
<br>
<pre><code>035 00000000 UNDEF  notype       External     | __imp__Sleep@4<br>
036 00000000 UNDEF  notype ()    External     | _free<br>
037 00000000 UNDEF  notype ()    External     | _malloc<br>
038 00000000 UNDEF  notype       External     | __imp___amsg_exit<br>
039 00000000 UNDEF  notype ()    External     | __encode_pointer<br>
03A 00000000 UNDEF  notype ()    External     | __decode_pointer<br>
03B 00000000 UNDEF  notype ()    External     | ___security_init_cookie<br>
03C 00000000 UNDEF  notype ()    External     | _DllMain@12<br>
03D 00000000 SECTB  notype ()    External     | __CRT_INIT@12<br>
03E 00000000 UNDEF  notype ()    External     | __initterm<br>
03F 00000000 UNDEF  notype ()    External     | ___main<br>
040 00000000 UNDEF  notype ()    External     | __pei386_runtime_relocator<br>
041 00000000 UNDEF  notype ()    External     | _DllEntryPoint@12<br>
042 00000000 SECTD  notype ()    Static       | ___DllMainCRTStartup<br>
043 00000000 SECT7  notype ()    External     | _DllMainCRTStartup@12<br>
044 0000012B SECTD  notype       Label        | $i__leave$14<br>
045 00000000 UNDEF  notype       External     | ___native_startup_state<br>
046 00000000 UNDEF  notype       External     | ___native_startup_lock<br>
047 00000000 UNDEF  notype       External     | ___native_dllmain_reason<br>
</code></pre>

We count the number of underbars in the linker error above very carefully to see that it wants only <b>TWO</b> underbars. Something is very puzzling here. No where in our entire source code is there a function that could generate a function <b><code>DllMainCRTStartup</code></b> that is preceded by exactly two underbars, and we have not indicated any kind of weird options to the linker to cause it to want <b><code>DllMainCRTStartup</code></b> that is preceded by exactly two underbars.<br>
<br>
This is what is happening:<br>
<br>
The linker knows that what we are trying to generate, <b><code>imm32.dll</code></b> is a DLL, and, per <a href='http://msdn.microsoft.com/en-us/library/988ye33t.aspx'>MSDN documentation</a>, unless we use the <b><code>/ENTRY</code></b> linker option, the linker will determine for itself that that the entry point into the final DLL will be a function having the undecorated signature <b><code>_DllMainCRTStartup</code></b>:<br>
<br>
Notice that this function has <b>ONE</b> underbar. Therefore, after decoration, the final symbol will be <b><code>__DllMainCRTStartup@12</code></b>, and the problem is solved: the linker is the culprit.<br>
<br>
We fix this issue by telling the linker to use our (eventually) single-underbar <b><code>_DllMainCRTStartup@12</code></b> function that is defined in <b><code>crtdll.c</code></b>:<br>
<br>
<img src='https://reactos.googlecode.com/svn/wiki/Development_Guide/Create_Visual_Studio_Projects/Project_Properties/Entry_Point.png' />

Notice how the linker expects us to omit the underbar in the IDE option.<br>
<br>
<h6>No Entry Point</h6>
<h6>Set Checksum</h6>
<h6>Base Address</h6>
<h6>Randomized Base Address</h6>
<h6>Fixed Base Address</h6>
<h6>Data Execution Prevention (DEP)</h6>
<h6>Tunr Off Assembly Generation</h6>
<h6>Unload delay loaded DLL</h6>
<h6>Nobind delay loaded DLL</h6>
<h6>Import Library</h6>
<h6>Merge Sections</h6>
<h6>Target Machine</h6>
<h6>Profile</h6>
<h6>CLR Thread Attribute</h6>
<h6>CLR Image Type</h6>
<h6>Key File</h6>
<h6>Key Container</h6>
<h6>Delay Sign</h6>
<h6>CLR Unmanaged Code Check</h6>
<h6>Detect One Definition Rule violations</h6>
<h6>Error Reporting</h6>
<h6>Section Alignment</h6>
<h6>Preserve Last Error Code for PInvoke Calls</h6>
<h6>Image Has Safe Exception Handlers</h6>
<h5>All Options</h5>
<h5>Command-Line</h5>
<h3>Librarian</h3>
<h5>General</h5>
<h6>Output File</h6>
<h6>Additional Dependencies</h6>
<h6>Additional Library Directories</h6>
<h6>Suppress Startup Banner</h6>
<h6>Module Definition File Name</h6>
<h6>Ignore All Default Libraries</h6>
<h6>Export Named Functions</h6>
<h6>Force Symbol References</h6>
Use Unicode Response Files<br>
<h6>Link Library Dependencies</h6>
<h6>Error Reporting</h6>
<h6>Treat Lib Warning As Errors</h6>
<h6>Target Machine</h6>
<h6>Sub System</h6>
<h6>Minimum Required Version</h6>
<h6>Remove Objects</h6>
<h6>Verbose</h6>
<h6>Name</h6>
<h6>Link Time Code Generation</h6>
<h5>Command Line</h5>
<h4>Manifest Tool</h4>
<h5>General</h5>
<h6>Suppress Startup Banner</h6>
<h6>Verbose Output</h6>
<h6>Assembly Identity</h6>
<h5>Input and Output</h5>
<h6>Additional Manifest Files</h6>
<h6>Input Resource Manifests</h6>
<h6>Embed Manifest</h6>
<h6>Output Manifest File</h6>
<h6>Manifest Resource File</h6>
<h6>Generate Catalog Files</h6>
<h6>Generated Manifest From Managed Assembly</h6>
<h6>Suppress Dependency Element</h6>
<h6>Generate Category Tags</h6>
<h6>Enable DPI Awareness</h6>

<h5>Isolated COM</h5>
<h6>Type Library File</h6>
<h6>Registrar Script File</h6>
<h6>Component File Name</h6>
<h6>Replacements File</h6>

<h5>Advanced</h5>
<h6>Update File Hashes</h6>
<h6>Update File Hashes Search Path</h6>
Command Line<br>
<h4>Resources</h4>
<h5>General</h5>
When a .rc files contains a #include, the resource compiler will sometimes complain that it cannot find the file that is being included, even though the programmer has specified the path to the included file in the IDE settings. The reason is that the programmer is in error – he did, indeed, specify a path, but for C/C++ files, not for .rc files. The .rc files in question must have their #include’s updated accordingly.<br>
<br>
<h6>Preprocess Definitions</h6>
<h6>Use Undefine Preprocessor Definitions</h6>
<h6>Culture</h6>
<h6>Additional Include Directories</h6>
<h6>Ignore Standard Include Paths</h6>
<h6>Show Progress</h6>
<h6>Suppress Startup Banner</h6>
<h6>Resource File Name</h6>
<h6>Null Terminate Strings</h6>
<h5>Command Line</h5>
<h4>XML Document Generator</h4>
<h5>General</h5>
<h6>Suppress Startup Banner</h6>
<h6>Additional Document File</h6>
<h6>Output Document File</h6>
<h6>Document Library Dependencies</h6>
<h5>Command Line</h5>
<h4>Browse Information</h4>
<h5>General</h5>
<h6>Suppress Startup Banner</h6>
<h6>Output File</h6>
<h6>Preserve SBR Files</h6>
<h5>Command Line</h5>
<h4>Build Events</h4>
<h5>Pre-Build Event</h5>
<h6>Command Line</h6>
<h6>Description</h6>
<h6>Use In Build</h6>
<h5>Pre-Link Event</h5>
<h6>Command Line</h6>
<h6>Description</h6>
<h6>Use In Build</h6>
<h5>Post-Build Event</h5>
<h6>Command Line</h6>
<h6>Description</h6>
<h6>Use In Build</h6>
<h4>Custom Build Step</h4>
<h5>General</h5>
<h4>Custom Build Tool</h4>
<h4>Code Analysis</h4>

<h2>Create Project To Generate CD</h2>
<h2>Create Project To Generate SD Card</h2>
<h1>Develop</h1>
<h2>Edit</h2>
<h3>Edit C/C++ Code</h3>
<h3>Edit Language Files</h3>
<h2>Build</h2>
<h3>Build According to Platform</h3>
<h4>Build For x86_32</h4>
<h4>Build For x86_64</h4>
<h4>Build For ARM</h4>
<h3>Build According to Configuration</h3>
<h4>Build For Debug</h4>
<h4>Build For Release</h4>
<h3>Build A Single File</h3>
<h3>Build A Group Of Files</h3>
<h3>Build A Single Project</h3>
<h3>Build A Group Of Projects</h3>
<h3>Build A ReactOS Image</h3>

Once you've got your Active Solution configured, all you have to do is hit F7 to build the Debug x86_32 version of ReactOS.sln (solution).  Expect this step to take a while to complete.  When it's done compiling, you'll find the <b><code>ReactOS_CD_Live.ISO</code></b> at:<br>
<b><code>reactos\reactos\_IMAGES\CD_Live\build\x86_32\Debug\ReactOS_CD_Live.ISO</code></b>
<h3>Build Everything</h3>
<h3>Clean</h3>
<h3>Rebuild</h3>
<h2>Deploy</h2>
<h3>Deploy CD</h3>
If you haven't yet created the ReactOS ISO, go back to wherever you left off in the <a href='Setup_Guide.md'>Setup Guide</a> and finish the process.  Once you've got your ISO, open up VirtualBox again.<br>
<br>
Now click the yellow gear button at the top, Settings.  Go to Storage and then choose "Empty" (your virtual machine's CD drive).  Click the disc icon on the far right to access the drop-down menu, and select "Choose a virtual CD/DVD disk file..."  Choose the .ISO file you built with VS2012.  This puts the ReactOS Boot Disc into the virtual machine.  Press OK.<br>
<img src='https://reactos.googlecode.com/svn/wiki/VBox%20Setup%204%20-%20en.png' />
<h3>Deploy SD Card</h3>
<h2>Debug</h2>
When things don't work according to plan, it's time to debug your code.  <b>Blah, blah, blah finish this later - PuTTY config for debug logs</b>

Currently, ReactOS built with Visual Studio doesn't yet boot.  This is a screenshot of the BSOD that occurs once it's booted as far as it can:<br>
<blockquote><img src='https://reactos.googlecode.com/svn/wiki/Good%20BSOD.png' />
<h3>Debug User-Mode Executable</h3>
<h3>Debug Kernel-Mode Executable</h3>
<h1>Save ReactOS.sln</h1>
<h1>Push Source Code To Respository</h1>
<h2>SVN Commits</h2>
If you have made changes to the ReactOS source and you want to send those changes upstream to the ReactOS SVN repository, then you want to make a <i>commit</i>.  Note that commits made here only affect this database; you are not directly submitting commits to the official ReactOS trunk located at <a href='http://www.ReactOS.org/'>http://www.ReactOS.org/</a>.</blockquote>

To submit a commit, it is best to not have Visual Studio running.  Navigate Windows Explorer to your ReactOS source folder.  It will look something like this:<br>
<img src='https://reactos.googlecode.com/svn/wiki/SVN%20Sync%201%20-%20en.png' />

Note the red icon on the folder.  This indicates that you have made changes to your source since you last committed/synced.  Right-click on the folder and select SVN Commit.  (Note in this screen shot that a different option is highlighted.)<br>
<img src='https://reactos.googlecode.com/svn/wiki/SVN%20Sync%202%20-%20en.png' />


<h2>Resolving Conflicts</h2>
Sometimes you will try to update or commit a file that has been changed online and changed by you.  This results in a conflict, which looks like this:<br>
<img src='https://reactos.googlecode.com/svn/wiki/SVN%20Sync%204%20-%20en.png' />

To resolve the conflict, you must decide to use the online copy ("theirs") , or your own copy ("mine").  You can also compare the two, and even open your copy to make changes as needed.  These are your available options:<br>
<img src='https://reactos.googlecode.com/svn/wiki/SVN%20Sync%205%20-%20en.png' />

In this example, I chose to discard my copy and use "theirs".<br>
<br>
When finished dealing with conflicts, you will again have the green icon over your ReactOS source folder.<br>
<br>
<h1>Update JIRA</h1>
<h1>Known Errors</h1>