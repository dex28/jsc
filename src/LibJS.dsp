# Microsoft Developer Studio Project File - Name="LibJS" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=LibJS - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "LibJS.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "LibJS.mak" CFG="LibJS - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "LibJS - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "LibJS - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "LibJS - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBJS_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBJS_EXPORTS" /D "LIBJS_IMPLEMENTATION" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 odbc32.lib /nologo /dll /machine:I386 /out:"../../RBin/LibJS.dll"

!ELSEIF  "$(CFG)" == "LibJS - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBJS_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "LIBJS_EXPORTS" /D "JS_DEBUG_NEW" /D "LIBJS_IMPLEMENTATION" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 odbc32.lib /nologo /dll /debug /machine:I386 /out:"../../Bin/LibJS.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "LibJS - Win32 Release"
# Name "LibJS - Win32 Debug"
# Begin Group "VM"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ByteCode.cpp
# End Source File
# Begin Source File

SOURCE=.\IOStream.cpp
# End Source File
# Begin Source File

SOURCE=.\Object.cpp
# End Source File
# Begin Source File

SOURCE=.\Variant.cpp
# End Source File
# Begin Source File

SOURCE=.\VM.cpp
# End Source File
# Begin Source File

SOURCE=.\VMcpu.cpp
# End Source File
# Begin Source File

SOURCE=.\VMcpu.h
# End Source File
# Begin Source File

SOURCE=.\VMdebug.cpp
# End Source File
# Begin Source File

SOURCE=.\VMheap.cpp
# End Source File
# End Group
# Begin Group "Builtins"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\a_Builtin.cpp
# End Source File
# Begin Source File

SOURCE=.\b0_Core.cpp
# End Source File
# Begin Source File

SOURCE=.\b1_Array.cpp
# End Source File
# Begin Source File

SOURCE=.\b1_Bool.cpp
# End Source File
# Begin Source File

SOURCE=.\b1_Func.cpp
# End Source File
# Begin Source File

SOURCE=.\b1_Number.cpp
# End Source File
# Begin Source File

SOURCE=.\b1_Object.cpp
# End Source File
# Begin Source File

SOURCE=.\b1_String.cpp
# End Source File
# Begin Source File

SOURCE=.\b2_Date.cpp
# End Source File
# Begin Source File

SOURCE=.\b2_Dir.cpp
# End Source File
# Begin Source File

SOURCE=.\b2_File.cpp
# End Source File
# Begin Source File

SOURCE=.\b2_Math.cpp
# End Source File
# Begin Source File

SOURCE=.\b2_System.cpp
# End Source File
# Begin Source File

SOURCE=.\b2_VM.cpp
# End Source File
# Begin Source File

SOURCE=.\b4_MD5.cpp
# End Source File
# Begin Source File

SOURCE=.\b4_SQL.cpp
# End Source File
# End Group
# Begin Group "Misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\DynaLoad.cpp
# End Source File
# Begin Source File

SOURCE=.\MemAlloc.cpp
# End Source File
# End Group
# Begin Group "Compiler"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Asm.cpp
# End Source File
# Begin Source File

SOURCE=.\AsmBC.cpp
# End Source File
# Begin Source File

SOURCE=.\AsmOpt.cpp
# End Source File
# Begin Source File

SOURCE=.\CodeGen.cpp
# End Source File
# Begin Source File

SOURCE=.\Compiler.cpp
# End Source File
# Begin Source File

SOURCE=.\ContBreak.cpp
# End Source File
# Begin Source File

SOURCE=.\CountLocals.cpp
# End Source File
# Begin Source File

SOURCE=.\Lexer.cpp
# End Source File
# Begin Source File

SOURCE=.\Namespace.cpp
# End Source File
# Begin Source File

SOURCE=.\Parser.cpp
# End Source File
# End Group
# Begin Group "Lib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\CRC32.cpp
# End Source File
# Begin Source File

SOURCE=.\DES.cpp
# End Source File
# Begin Source File

SOURCE=.\DES.h
# End Source File
# Begin Source File

SOURCE=.\DLList.h
# End Source File
# Begin Source File

SOURCE=.\MD5.cpp
# End Source File
# Begin Source File

SOURCE=.\MD5.h
# End Source File
# Begin Source File

SOURCE=.\ODBC.cpp
# End Source File
# Begin Source File

SOURCE=.\ODBC.h
# End Source File
# Begin Source File

SOURCE=.\Rand48.cpp
# End Source File
# Begin Source File

SOURCE=.\Win32.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\JS.h
# End Source File
# Begin Source File

SOURCE=.\JSC.h
# End Source File
# Begin Source File

SOURCE=.\JSconfig.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\LibJS.rc
# End Source File
# End Group
# End Target
# End Project
