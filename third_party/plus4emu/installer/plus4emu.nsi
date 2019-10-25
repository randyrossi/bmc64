;NSIS Modern User Interface
;Basic Example Script
;Written by Joost Verburg

  SetCompressor /SOLID /FINAL LZMA

;--------------------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;General

  ;Use "makensis.exe /DWIN64" to create x64 installer
  !ifndef WIN64
  ;Name and file
  Name "plus4emu"
  OutFile "plus4emu-1.2.11_beta-x86.exe"
  ;Default installation folder
  InstallDir "$PROGRAMFILES\plus4emu"
  !else
  Name "plus4emu (x64)"
  OutFile "plus4emu-1.2.11_beta-x64.exe"
  InstallDir "$PROGRAMFILES64\plus4emu"
  !endif

  RequestExecutionLevel admin

  ;Get installation folder from registry if available
  InstallDirRegKey HKLM "Software\plus4emu\InstallDirectory" ""

;--------------------------------
;Variables

  Var MUI_TEMP
  Var STARTMENU_FOLDER

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "..\COPYING"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM"
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\plus4emu"
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  !insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER
  !insertmacro MUI_PAGE_INSTFILES

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "plus4emu" SecMain

  SectionIn RO

  SetOutPath "$INSTDIR"

  File "..\COPYING"
  File "..\licenses\LICENSE.FLTK"
  File "..\licenses\LICENSE.Lua"
  File "..\licenses\LICENSE.PortAudio"
  File "..\licenses\LICENSE.SDL"
  File "..\licenses\LICENSE.dotconf"
  File "..\licenses\LICENSE.libsndfile"
  File "/oname=news.txt" "..\NEWS"
  File "/oname=readme.txt" "..\README"
  File "..\plus4emu.exe"
  File "..\makecfg.exe"
  File "..\tapconv.exe"
  !ifndef WIN64
  File "C:\mingw32\bin\libgcc_s_dw2-1.dll"
  File "C:\mingw32\bin\libsndfile-1.dll"
  File "C:\mingw32\bin\libstdc++-6.dll"
  File "C:\mingw32\bin\lua51.dll"
  File "C:\mingw32\bin\lua53.dll"
  File "C:\mingw32\bin\portaudio_x86.dll"
  File "C:\mingw32\bin\SDL.dll"
  !else
  File "C:\mingw64\bin\libgcc_s_seh-1.dll"
  File "C:\mingw64\bin\libsndfile-1.dll"
  File "C:\mingw64\bin\libstdc++-6.dll"
  File "C:\mingw64\bin\lua51.dll"
  File "C:\mingw64\bin\lua53.dll"
  File "C:\mingw64\bin\portaudio_x64.dll"
  File "C:\mingw64\bin\SDL.dll"
  !endif

  SetOutPath "$INSTDIR\config"

  SetOutPath "$INSTDIR\demo"

  SetOutPath "$INSTDIR\disk"

  File "..\disk\disk.zip"

  SetOutPath "$INSTDIR\progs"

  SetOutPath "$INSTDIR\roms"

  File /nonfatal "..\roms\1526_07c.rom"
  File /nonfatal "..\roms\3plus1.rom"
  File /nonfatal "..\roms\dos1541.rom"
  File /nonfatal "..\roms\dos15412.rom"
  File /nonfatal "..\roms\dos1551.rom"
  File /nonfatal "..\roms\dos1581.rom"
  File /nonfatal "..\roms\mps801.rom"
  File /nonfatal "..\roms\p4_basic.rom"
  File "..\roms\p4fileio.rom"
  File /nonfatal "..\roms\p4kernal.rom"
  File /nonfatal "..\roms\p4_ntsc.rom"

  SetOutPath "$INSTDIR\tape"

  ;Store installation folder
  WriteRegStr HKLM "Software\plus4emu\InstallDirectory" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    SetShellVarContext all

    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER\GUI themes"
    SetOutPath "$INSTDIR"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\plus4emu - OpenGL mode.lnk" "$INSTDIR\plus4emu.exe" '-opengl'
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\plus4emu - software mode.lnk" "$INSTDIR\plus4emu.exe" '-no-opengl'
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\GUI themes\plus4emu - GL - Win2000 theme.lnk" "$INSTDIR\plus4emu.exe" '-opengl -colorscheme 1'
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\GUI themes\plus4emu - GL - plastic theme.lnk" "$INSTDIR\plus4emu.exe" '-opengl -colorscheme 2'
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\GUI themes\plus4emu - GL - Gtk+ theme.lnk" "$INSTDIR\plus4emu.exe" '-opengl -colorscheme 3'
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\GUI themes\plus4emu - Software - Win2000 theme.lnk" "$INSTDIR\plus4emu.exe" '-no-opengl -colorscheme 1'
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\GUI themes\plus4emu - Software - plastic theme.lnk" "$INSTDIR\plus4emu.exe" '-no-opengl -colorscheme 2'
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\GUI themes\plus4emu - Software - Gtk+ theme.lnk" "$INSTDIR\plus4emu.exe" '-no-opengl -colorscheme 3'
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\README.lnk" "$INSTDIR\readme.txt"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Reinstall configuration files.lnk" "$INSTDIR\makecfg.exe" '"$INSTDIR"'
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_END

  ExecWait '"$INSTDIR\makecfg.exe" "$INSTDIR"'

SectionEnd

Section "Utilities" SecUtils

  SetOutPath "$INSTDIR"

  File "..\compress.exe"
  File "..\p4fliconv.exe"
  File "..\p4fliconv_gui.exe"
  File "..\p4sconv.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    SetShellVarContext all

    ;Create shortcuts
    SetOutPath "$INSTDIR"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Image converter utility.lnk" "$INSTDIR\p4fliconv_gui.exe"

  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

Section "Development library" SecPlus4Library

  SetOutPath "$INSTDIR\plus4lib"

  File "..\plus4lib\plus4emu.def"
  File "..\plus4lib\plus4emu.dll"
  File "..\plus4lib\plus4emu.h"
  File "..\plus4lib\sample.c"
  !ifndef WIN64
  File "C:\mingw32\bin\libgcc_s_dw2-1.dll"
  File "C:\mingw32\bin\libsndfile-1.dll"
  File "C:\mingw32\bin\libstdc++-6.dll"
  !else
  File "C:\mingw64\bin\libgcc_s_seh-1.dll"
  File "C:\mingw64\bin\libsndfile-1.dll"
  File "C:\mingw64\bin\libstdc++-6.dll"
  !endif

SectionEnd

Section "Source code" SecSrc

  SetOutPath "$INSTDIR\src"

  File "..\COPYING"
  File "..\NEWS"
  File "..\README"
  File "..\SConstruct"
  File "..\*.sh"
  File "..\.gitignore"

  SetOutPath "$INSTDIR\src\Fl_Native_File_Chooser"

  File "..\Fl_Native_File_Chooser\CREDITS"
  File "..\Fl_Native_File_Chooser\README.txt"
  File "..\Fl_Native_File_Chooser\*.cxx"

  SetOutPath "$INSTDIR\src\Fl_Native_File_Chooser\FL"

  File "..\Fl_Native_File_Chooser\FL\*.H"

  SetOutPath "$INSTDIR\src\config"

  File "..\config\*.cfg"

  SetOutPath "$INSTDIR\src\disk"

  File "..\disk\*.zip"

  SetOutPath "$INSTDIR\src\gui"

  File "..\gui\*.fl"
  File /x "*_fl.cpp" "..\gui\*.cpp"
  File /x "*_fl.hpp" "..\gui\*.hpp"

  SetOutPath "$INSTDIR\src\installer"

  File "..\installer\*.nsi"
  File "..\installer\*.fl"
  File "..\installer\makecfg.cpp"
  File "..\installer\plus4emu"

  SetOutPath "$INSTDIR\src\licenses"

  File "..\licenses\LICENSE.*"

  SetOutPath "$INSTDIR\src\msvc"

  SetOutPath "$INSTDIR\src\msvc\include"

  File "..\msvc\include\*.h"

  SetOutPath "$INSTDIR\src\patches"

  File "..\patches\*.patch"

  SetOutPath "$INSTDIR\src\plus4emu.app"

  SetOutPath "$INSTDIR\src\plus4emu.app\Contents"

  File "..\plus4emu.app\Contents\Info.plist"
  File "..\plus4emu.app\Contents\PkgInfo"

  SetOutPath "$INSTDIR\src\plus4emu.app\Contents\MacOS"

  SetOutPath "$INSTDIR\src\plus4emu.app\Contents\Resources"

  File "..\plus4emu.app\Contents\Resources\plus4emu.icns"

  SetOutPath "$INSTDIR\src\plus4lib"

  File "..\plus4lib\*.c"
  File "..\plus4lib\*.cpp"
  File "..\plus4lib\*.h"

  SetOutPath "$INSTDIR\src\resid"

  File "..\resid\AUTHORS"
  File "..\resid\COPYING"
  File "..\resid\ChangeLog"
  File "..\resid\NEWS"
  File "..\resid\README"
  File "..\resid\THANKS"
  File "..\resid\TODO"
  File "..\resid\*.cpp"
  File "..\resid\*.hpp"

  SetOutPath "$INSTDIR\src\resource"

  File "..\resource\Makefile"
  File "..\resource\Read_me.txt"
  File "..\resource\plus4emu.rc"
  File "..\resource\*.desktop"
  File "..\resource\*.ico"
  File "..\resource\*.png"

  SetOutPath "$INSTDIR\src\roms"

  File "..\roms\*.rom"

  SetOutPath "$INSTDIR\src\src"

  File "..\src\*.c"
  File "..\src\*.cpp"
  File "..\src\*.h"
  File "..\src\*.hpp"
  File "..\src\*.py"

  SetOutPath "$INSTDIR\src\util"

  File "..\util\*.c"
  File "..\util\*.cpp"

  SetOutPath "$INSTDIR\src\util\compress"

  File "..\util\compress\*.cpp"
  File "..\util\compress\*.hpp"
  File "..\util\compress\*.py"
  File "..\util\compress\*.s"

  SetOutPath "$INSTDIR\src\util\p4fliconv"

  File "..\util\p4fliconv\*.fl"
  File /x "*_fl.cpp" "..\util\p4fliconv\*.cpp"
  File /x "*_fl.hpp" "..\util\p4fliconv\*.hpp"
  File "..\util\p4fliconv\*.s"

SectionEnd

Section "Associate .prg and .p00 files with plus4emu" SecAssocPRG

  WriteRegStr HKCR ".prg" "" "Plus4Emu.PRGFile"
  WriteRegStr HKCR ".p00" "" "Plus4Emu.PRGFile"
  WriteRegStr HKCR "Plus4Emu.PRGFile" "" "Plus/4 program"
  WriteRegStr HKCR "Plus4Emu.PRGFile\DefaultIcon" "" "$INSTDIR\plus4emu.exe,4"
  WriteRegStr HKCR "Plus4Emu.PRGFile\shell" "" "open"
  WriteRegStr HKCR "Plus4Emu.PRGFile\shell\open\command" "" '"$INSTDIR\plus4emu.exe" -prg "%1"'

SectionEnd

Section "Associate .d64 and .d81 files with plus4emu" SecAssocDisk

  WriteRegStr HKCR ".d64" "" "Plus4Emu.DiskFile"
  WriteRegStr HKCR ".d81" "" "Plus4Emu.DiskFile"
  WriteRegStr HKCR "Plus4Emu.DiskFile" "" "Plus/4 disk image"
  WriteRegStr HKCR "Plus4Emu.DiskFile\DefaultIcon" "" "$INSTDIR\plus4emu.exe,1"
  WriteRegStr HKCR "Plus4Emu.DiskFile\shell" "" "open"
  WriteRegStr HKCR "Plus4Emu.DiskFile\shell\open\command" "" '"$INSTDIR\plus4emu.exe" -disk "%1"'

SectionEnd

Section "Associate .tap files with plus4emu" SecAssocTape

  WriteRegStr HKCR ".tap" "" "Plus4Emu.TapeFile"
  WriteRegStr HKCR "Plus4Emu.TapeFile" "" "Plus/4 tape image"
  WriteRegStr HKCR "Plus4Emu.TapeFile\DefaultIcon" "" "$INSTDIR\plus4emu.exe,4"
  WriteRegStr HKCR "Plus4Emu.TapeFile\shell" "" "open"
  WriteRegStr HKCR "Plus4Emu.TapeFile\shell\open\command" "" '"$INSTDIR\plus4emu.exe" -tape "%1"'

SectionEnd

Section "Create desktop shortcuts" SecDesktop

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    SetShellVarContext all

    ;Create shortcuts
    SetOutPath "$INSTDIR"

    CreateShortCut "$DESKTOP\plus4emu.lnk" "$INSTDIR\plus4emu.exe" '-opengl'
    SectionGetFlags ${SecUtils} $0
    IntOp $0 $0 & ${SF_SELECTED}
    IntCmp $0 0 noImgConv
    CreateShortCut "$DESKTOP\p4fliconv.lnk" "$INSTDIR\p4fliconv_gui.exe"
noImgConv:

  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecMain ${LANG_ENGLISH} "plus4emu binaries"
  LangString DESC_SecUtils ${LANG_ENGLISH} "Install image converter and compressor utilities"
  LangString DESC_SecPlus4Library ${LANG_ENGLISH} "Install plus4emu C interface (header, DLL file, and sample code)"
  LangString DESC_SecSrc ${LANG_ENGLISH} "plus4emu source code"
  LangString DESC_SecAssocPRG ${LANG_ENGLISH} "Associate .prg and .p00 files with plus4emu"
  LangString DESC_SecAssocDisk ${LANG_ENGLISH} "Associate .d64 and .d81 files with plus4emu"
  LangString DESC_SecAssocTape ${LANG_ENGLISH} "Associate .tap files with plus4emu"
  LangString DESC_SecDesktop ${LANG_ENGLISH} "Create desktop shortcuts"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} $(DESC_SecMain)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecUtils} $(DESC_SecUtils)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPlus4Library} $(DESC_SecPlus4Library)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecSrc} $(DESC_SecSrc)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecAssocPRG} $(DESC_SecAssocPRG)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecAssocDisk} $(DESC_SecAssocDisk)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecAssocTape} $(DESC_SecAssocTape)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} $(DESC_SecDesktop)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  SetShellVarContext all

  Delete "$INSTDIR\COPYING"
  Delete "$INSTDIR\LICENSE.FLTK"
  Delete "$INSTDIR\LICENSE.Lua"
  Delete "$INSTDIR\LICENSE.PortAudio"
  Delete "$INSTDIR\LICENSE.SDL"
  Delete "$INSTDIR\LICENSE.dotconf"
  Delete "$INSTDIR\LICENSE.libsndfile"
  Delete "$INSTDIR\compress.exe"
  Delete "$INSTDIR\news.txt"
  Delete "$INSTDIR\readme.txt"
  Delete "$INSTDIR\p4fliconv.exe"
  Delete "$INSTDIR\p4fliconv_gui.exe"
  Delete "$INSTDIR\p4sconv.exe"
  Delete "$INSTDIR\plus4emu.exe"
  Delete "$INSTDIR\libgcc_s_dw2-1.dll"
  Delete "$INSTDIR\libgcc_s_seh-1.dll"
  Delete "$INSTDIR\libsndfile-1.dll"
  Delete "$INSTDIR\libstdc++-6.dll"
  Delete "$INSTDIR\lua51.dll"
  Delete "$INSTDIR\lua53.dll"
  Delete "$INSTDIR\makecfg.exe"
  Delete "$INSTDIR\portaudio_x64.dll"
  Delete "$INSTDIR\portaudio_x86.dll"
  Delete "$INSTDIR\SDL.dll"
  Delete "$INSTDIR\tapconv.exe"
  Delete "$INSTDIR\config\P4_Keyboard_US.cfg"
  Delete "$INSTDIR\config\P4_Keyboard_HU.cfg"
  Delete "$INSTDIR\config\P4_16k_PAL.cfg"
  Delete "$INSTDIR\config\P4_16k_NTSC.cfg"
  Delete "$INSTDIR\config\P4_64k_PAL.cfg"
  Delete "$INSTDIR\config\P4_64k_NTSC.cfg"
  Delete "$INSTDIR\config\P4_16k_PAL_3PLUS1.cfg"
  Delete "$INSTDIR\config\P4_16k_NTSC_3PLUS1.cfg"
  Delete "$INSTDIR\config\P4_64k_PAL_3PLUS1.cfg"
  Delete "$INSTDIR\config\P4_64k_NTSC_3PLUS1.cfg"
  Delete "$INSTDIR\config\P4_16k_PAL_FileIO.cfg"
  Delete "$INSTDIR\config\P4_16k_PAL_TapeTurbo.cfg"
  Delete "$INSTDIR\config\P4_16k_NTSC_FileIO.cfg"
  Delete "$INSTDIR\config\P4_16k_NTSC_TapeTurbo.cfg"
  Delete "$INSTDIR\config\P4_64k_PAL_FileIO.cfg"
  Delete "$INSTDIR\config\P4_64k_PAL_TapeTurbo.cfg"
  Delete "$INSTDIR\config\P4_64k_NTSC_FileIO.cfg"
  Delete "$INSTDIR\config\P4_64k_NTSC_TapeTurbo.cfg"
  Delete "$INSTDIR\config\P4_16k_PAL_3PLUS1_FileIO.cfg"
  Delete "$INSTDIR\config\P4_16k_PAL_3PLUS1_TapeTurbo.cfg"
  Delete "$INSTDIR\config\P4_16k_NTSC_3PLUS1_FileIO.cfg"
  Delete "$INSTDIR\config\P4_16k_NTSC_3PLUS1_TapeTurbo.cfg"
  Delete "$INSTDIR\config\P4_64k_PAL_3PLUS1_FileIO.cfg"
  Delete "$INSTDIR\config\P4_64k_PAL_3PLUS1_TapeTurbo.cfg"
  Delete "$INSTDIR\config\P4_64k_NTSC_3PLUS1_FileIO.cfg"
  Delete "$INSTDIR\config\P4_64k_NTSC_3PLUS1_TapeTurbo.cfg"
  Delete "$INSTDIR\config\shaders\ntsc.glsl"
  Delete "$INSTDIR\config\shaders\pal.glsl"
  RMDir "$INSTDIR\config\shaders"
  RMDir "$INSTDIR\config"
  RMDir "$INSTDIR\demo"
  Delete "$INSTDIR\disk\disk.zip"
  RMDir "$INSTDIR\disk"
  Delete "$INSTDIR\plus4lib\plus4emu.h"
  Delete "$INSTDIR\plus4lib\plus4emu.def"
  Delete "$INSTDIR\plus4lib\plus4emu.dll"
  Delete "$INSTDIR\plus4lib\sample.c"
  Delete "$INSTDIR\plus4lib\libgcc_s_dw2-1.dll"
  Delete "$INSTDIR\plus4lib\libgcc_s_seh-1.dll"
  Delete "$INSTDIR\plus4lib\libsndfile-1.dll"
  Delete "$INSTDIR\plus4lib\libstdc++-6.dll"
  Delete "$INSTDIR\plus4lib\mingwm10.dll"
  RMDir "$INSTDIR\plus4lib"
  RMDir "$INSTDIR\progs"
  Delete "$INSTDIR\roms\1526_07c.rom"
  Delete "$INSTDIR\roms\3plus1.rom"
  Delete "$INSTDIR\roms\3plus1hi.rom"
  Delete "$INSTDIR\roms\3plus1lo.rom"
  Delete "$INSTDIR\roms\dos1541.rom"
  Delete "$INSTDIR\roms\dos15412.rom"
  Delete "$INSTDIR\roms\dos1551.rom"
  Delete "$INSTDIR\roms\dos1581.rom"
  Delete "$INSTDIR\roms\mps801.rom"
  Delete "$INSTDIR\roms\p4_basic.rom"
  Delete "$INSTDIR\roms\p4fileio.rom"
  Delete "$INSTDIR\roms\p4kernal.rom"
  Delete "$INSTDIR\roms\p4_ntsc.rom"
  RMDir "$INSTDIR\roms"
  RMDir /r "$INSTDIR\src"
  RMDir "$INSTDIR\tape"

  Delete "$DESKTOP\plus4emu.lnk"
  Delete "$DESKTOP\p4fliconv.lnk"

  !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP

  Delete "$SMPROGRAMS\$MUI_TEMP\plus4emu - OpenGL mode.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\plus4emu - software mode.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\GUI themes\plus4emu - GL - Win2000 theme.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\GUI themes\plus4emu - GL - plastic theme.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\GUI themes\plus4emu - GL - Gtk+ theme.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\GUI themes\plus4emu - Software - Win2000 theme.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\GUI themes\plus4emu - Software - plastic theme.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\GUI themes\plus4emu - Software - Gtk+ theme.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\plus4emu - GL - Win2000 theme.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\plus4emu - GL - plastic theme.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\Image converter utility.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\README.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\Reinstall configuration files.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"

  ;Delete empty start menu parent diretories
  StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP\GUI themes"

  startMenuDeleteLoop:
    ClearErrors
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."

    IfErrors startMenuDeleteLoopDone

    StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
  startMenuDeleteLoopDone:

  DeleteRegKey /ifempty HKLM "Software\plus4emu\InstallDirectory"
  DeleteRegKey /ifempty HKLM "Software\plus4emu"

  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"

SectionEnd

