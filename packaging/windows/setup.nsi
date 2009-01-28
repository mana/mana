CRCCheck on
SetCompress off
SetCompressor /SOLID lzma

!define SRCDIR "..\.."

;--- (and without !defines ) ---
!System "upx\upx.exe --best --crp-ms=999999 --compress-icons=0 --nrv2d ${SRCDIR}\aethyra.exe"

; HM NIS Edit helper defines
!define PRODUCT_NAME "Aethyra"
!define PRODUCT_VERSION "0.0.27.1"
!define PRODUCT_PUBLISHER "Aethyra Development Team"
!define PRODUCT_WEB_SITE "http://aethyra.org"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\aethyra.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; MUI 1.67 compatible ------
!include "MUI.nsh"

; MUI Settings
!define MUI_ABORTWARNING
;!define MUI_ICON "${NSISDIR}\Contrib\Graphics\Icons\win-install.ico"
!define MUI_ICON "${SRCDIR}\data\icons\aethyra.ico"
;!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\win-uninstall.ico"
!define MUI_UNICON "${SRCDIR}\data\icons\aethyra.ico"

;Language Selection Dialog Settings
;Remember the installer language
!define MUI_LANGDLL_REGISTRY_ROOT "HKCU"
!define MUI_LANGDLL_REGISTRY_KEY "Software\Aethyra"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

!define MUI_WELCOMEFINISHPAGE_BITMAP "setup_welcome.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "setup_welcome.bmp"

; Welcome page

!define MUI_WELCOMEPAGE_TITLE_3LINES
!insertmacro MUI_PAGE_WELCOME
; License page
!insertmacro MUI_PAGE_LICENSE "${SRCDIR}\COPYING"
; Directory page
!insertmacro MUI_PAGE_DIRECTORY
; Components page
!insertmacro MUI_PAGE_COMPONENTS
; Instfiles page
!insertmacro MUI_PAGE_INSTFILES
; Finish page
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_FUNCTION Runaethyra
!define MUI_FINISHPAGE_SHOWREADME 'notepad.exe "$\"$INSTDIR\README$\""'
!define MUI_PAGE_CUSTOMFUNCTION_PRE changeFinishImage
!define MUI_FINISHPAGE_LINK "Visit the Aethyra website for the latest news, FAQs and support"
!define MUI_FINISHPAGE_LINK_LOCATION "http://aethyra.org"
!insertmacro MUI_PAGE_FINISH

Function Runaethyra
SetOutPath $INSTDIR
Exec "$INSTDIR\aethyra.exe"
FunctionEnd

Function changeFinishImage
!insertmacro MUI_INSTALLOPTIONS_WRITE "ioSpecial.ini" "Field 1" "Text" "$PLUGINSDIR\setup_finish.bmp"
FunctionEnd

; Uninstaller pages

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!define MUI_FINISHPAGE_TITLE_3LINES
!insertmacro MUI_UNPAGE_FINISH

;Languages
!insertmacro MUI_LANGUAGE "English" # first language is the default language
!insertmacro MUI_LANGUAGE "French"
!insertmacro MUI_LANGUAGE "German"
!insertmacro MUI_LANGUAGE "Spanish"
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "TradChinese"
!insertmacro MUI_LANGUAGE "Japanese"
!insertmacro MUI_LANGUAGE "Korean"
!insertmacro MUI_LANGUAGE "Italian"
!insertmacro MUI_LANGUAGE "Dutch"
!insertmacro MUI_LANGUAGE "Danish"
!insertmacro MUI_LANGUAGE "Swedish"
!insertmacro MUI_LANGUAGE "Norwegian"
!insertmacro MUI_LANGUAGE "Finnish"
!insertmacro MUI_LANGUAGE "Greek"
!insertmacro MUI_LANGUAGE "Russian"
!insertmacro MUI_LANGUAGE "Portuguese"
!insertmacro MUI_LANGUAGE "PortugueseBR"
!insertmacro MUI_LANGUAGE "Polish"
!insertmacro MUI_LANGUAGE "Ukrainian"
!insertmacro MUI_LANGUAGE "Czech"
!insertmacro MUI_LANGUAGE "Slovak"
!insertmacro MUI_LANGUAGE "Croatian"
!insertmacro MUI_LANGUAGE "Bulgarian"
!insertmacro MUI_LANGUAGE "Hungarian"
!insertmacro MUI_LANGUAGE "Thai"
!insertmacro MUI_LANGUAGE "Romanian"
!insertmacro MUI_LANGUAGE "Latvian"
!insertmacro MUI_LANGUAGE "Macedonian"
!insertmacro MUI_LANGUAGE "Estonian"
!insertmacro MUI_LANGUAGE "Turkish"
!insertmacro MUI_LANGUAGE "Lithuanian"
!insertmacro MUI_LANGUAGE "Catalan"
!insertmacro MUI_LANGUAGE "Slovenian"
!insertmacro MUI_LANGUAGE "Serbian"
!insertmacro MUI_LANGUAGE "SerbianLatin"
!insertmacro MUI_LANGUAGE "Arabic"
!insertmacro MUI_LANGUAGE "Farsi"
!insertmacro MUI_LANGUAGE "Hebrew"
!insertmacro MUI_LANGUAGE "Indonesian"
!insertmacro MUI_LANGUAGE "Mongolian"
!insertmacro MUI_LANGUAGE "Luxembourgish"
!insertmacro MUI_LANGUAGE "Albanian"
!insertmacro MUI_LANGUAGE "Breton"
!insertmacro MUI_LANGUAGE "Belarusian"
!insertmacro MUI_LANGUAGE "Icelandic"
!insertmacro MUI_LANGUAGE "Malay"
!insertmacro MUI_LANGUAGE "Bosnian"
!insertmacro MUI_LANGUAGE "Kurdish"

!insertmacro MUI_RESERVEFILE_LANGDLL

ReserveFile "setup_finish.bmp"

; MUI end ------

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "aethyra-${PRODUCT_VERSION}-win32.exe"
InstallDir "$PROGRAMFILES\Aethyra"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails show
ShowUnInstDetails show

Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY
  InitPluginsDir
  File /oname=$PLUGINSDIR\setup_finish.bmp "setup_finish.bmp"
FunctionEnd

Section "Core files (required)" SecCore
  SectionIn RO
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  CreateDirectory "$SMPROGRAMS\Aethyra"
  CreateShortCut "$SMPROGRAMS\Aethyra\Aethyra.lnk" "$INSTDIR\aethyra.exe"
  CreateShortCut "$DESKTOP\Aethyra.lnk" "$INSTDIR\aethyra.exe"
  CreateDirectory "$INSTDIR\data"
  CreateDirectory "$INSTDIR\data\fonts"
  CreateDirectory "$INSTDIR\data\graphics"
  CreateDirectory "$INSTDIR\data\help"
  CreateDirectory "$INSTDIR\data\icons"
  CreateDirectory "$INSTDIR\data\graphics\gui"
  CreateDirectory "$INSTDIR\data\graphics\images"
  CreateDirectory "$INSTDIR\docs"

  SetOverwrite ifnewer
  SetOutPath "$INSTDIR"

  File "${SRCDIR}\aethyra.exe"
  File "${SRCDIR}\*.dll"
  File "${SRCDIR}\AUTHORS"
  File "${SRCDIR}\COPYING"
  File "${SRCDIR}\NEWS"
  File "${SRCDIR}\README"
  SetOutPath "$INSTDIR\data\fonts"
  File "${SRCDIR}\data\fonts\*.ttf"
  SetOutPath "$INSTDIR\data\graphics\gui"
  File "${SRCDIR}\data\graphics\gui\*.png"
  File "${SRCDIR}\data\graphics\gui\*.xml"
  SetOutPath "$INSTDIR\data\graphics\images"
  File /x minimap_*.png ${SRCDIR}\data\graphics\images\*.png
  SetOutPath "$INSTDIR\data\help"
  File "${SRCDIR}\data\help\*.txt"
  SetOutPath "$INSTDIR\data\icons\"
  File "${SRCDIR}\data\icons\aethyra.ico"
  SetOutPath "$INSTDIR\docs"
  File "${SRCDIR}\docs\FAQ.txt"
SectionEnd

Section "Music" SecMusic
  CreateDirectory "$INSTDIR\data\music"
  SetOutPath "$INSTDIR\data\music"
  File /nonfatal "${SRCDIR}\data\music\*.ogg"
SectionEnd

Section "Translations" SecTrans
  CreateDirectory "$INSTDIR\translations"
  SetOutPath "$INSTDIR\translations"
  File /nonfatal /r "${SRCDIR}\translations"
SectionEnd

;Package descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} "The core program files."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMusic} "Background music."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecTrans} "Translations for the user interface into 23 different languages. Uncheck this component to leave it in English."
!insertmacro MUI_FUNCTION_DESCRIPTION_END



Section -AdditionalIcons
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
  CreateShortCut "$SMPROGRAMS\Aethyra\Website.lnk" "$INSTDIR\${PRODUCT_NAME}.url"
  CreateShortCut "$SMPROGRAMS\Aethyra\Readme.lnk" "notepad.exe" "$INSTDIR\README"
  CreateShortCut "$SMPROGRAMS\Aethyra\FAQ.lnk" "$INSTDIR\docs\FAQ.txt"
  CreateShortCut "$SMPROGRAMS\Aethyra\Uninstall.lnk" "$INSTDIR\uninst.exe"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\aethyra.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\aethyra.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
SectionEnd

Function un.onInit
  !insertmacro MUI_UNGETLANGUAGE
FunctionEnd

Section Uninstall
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aethyra"

  Delete "$INSTDIR\*.*"

  Delete "$SMPROGRAMS\Aethyra\Uninstall.lnk"
  Delete "$DESKTOP\Aethyra.lnk"
  Delete "$SMPROGRAMS\Aethyra\Aethyra.lnk"
  Delete "$SMPROGRAMS\Aethyra\Website.lnk"
  Delete "$SMPROGRAMS\Aethyra\Readme.lnk"
  Delete "$SMPROGRAMS\Aethyra\FAQ.lnk"

  RMDir "$SMPROGRAMS\Aethyra"
  RMDir /r "$INSTDIR\data"
  RMDir /r "$INSTDIR\docs"
  RMDir /r "$INSTDIR\po"
  RMDir /r "$INSTDIR\updates"
  RMDir "$INSTDIR"

  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd
