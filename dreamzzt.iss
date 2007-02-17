; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
AppName=DreamZZT
AppVerName=DreamZZT 3.0.6.1
AppPublisher=Sam Steele
AppPublisherURL=http://www.c99.org/
AppSupportURL=http://forums.c99.org/
AppUpdatesURL=http://dev.c99.org/DreamZZT/
DefaultDirName={pf}\DreamZZT
DefaultGroupName=DreamZZT
OutputBaseFilename=dreamzzt-3.0.6.1-setup
Compression=lzma
SolidCompression=true
MinVersion=4.1.2222,5.0.2195
LicenseFile=COPYING
ChangesAssociations=true
AppVersion=3.0.6.1
UninstallDisplayIcon={app}\DreamZZT.exe
UninstallDisplayName=DreamZZT 3.0.6.1
VersionInfoVersion=3.0.6.1
VersionInfoCompany=Sam Steele
VersionInfoDescription=DreamZZT 3.0.6.1
VersionInfoTextVersion=3.0.6.1
VersionInfoCopyright=Copyright (C) 2000 - 2007 Sam Steele, All Rights Reserved.
InternalCompressLevel=max
InfoBeforeFile=ChangeLog

[Languages]
Name: english; MessagesFile: compiler:Default.isl

[Tasks]
Name: desktopicon; Description: {cm:CreateDesktopIcon}; GroupDescription: {cm:AdditionalIcons}; Flags: unchecked
Name: registerzzt; Description: Associate DreamZZT with .ZZT files; Flags: checkedonce; GroupDescription: Register file types:
Name: registersav; Description: Associate DreamZZT with .SAV files; Flags: checkedonce; GroupDescription: Register file types:

[Files]
Source: COPYING; DestDir: {app}
Source: ChangeLog; DestDir: {app}
Source: Release\DreamZZT.exe; DestDir: {app}; Flags: ignoreversion
Source: resources\town.zzt; DestDir: {app}; Flags: ignoreversion
Source: resources\tutorial.zzt; DestDir: {app}; Flags: ignoreversion
Source: resources\zzt-ascii.png; DestDir: {app}; Flags: ignoreversion
Source: win32\alut.dll; DestDir: {app}; Flags: ignoreversion
Source: win32\OpenAL32.dll; DestDir: {app}; Flags: ignoreversion
Source: win32\wrap_oal.dll; DestDir: {app}; Flags: ignoreversion
Source: win32\libcurl.dll; DestDir: {app}; Flags: ignoreversion; Tasks: 

[INI]

[Icons]
Name: {group}\DreamZZT; Filename: {app}\DreamZZT.exe; WorkingDir: {app}; IconFilename: {app}\DreamZZT.exe; IconIndex: 0; Comment: DreamZZT - Open Source ZZT Engine
Name: {group}\{cm:ProgramOnTheWeb,DreamZZT}; Filename: http://dev.c99.org/DreamZZT/; Tasks: ; Languages: 
Name: {group}\{cm:UninstallProgram,DreamZZT}; Filename: {uninstallexe}
Name: {userdesktop}\DreamZZT; Filename: {app}\DreamZZT.exe; Tasks: desktopicon; WorkingDir: {app}; IconFilename: {app}\DreamZZT.exe; Comment: DreamZZT - Open Source ZZT Engine; IconIndex: 0
Name: {group}\Forums; Filename: http://forums.c99.org/
Name: {group}\Report a Bug; Filename: http://dev.c99.org/DreamZZT/newticket

[Run]
Filename: {app}\DreamZZT.exe; Description: {cm:LaunchProgram,DreamZZT}; Flags: nowait postinstall skipifsilent

[UninstallDelete]
Type: files; Name: {app}\DreamZZT.url
Type: files; Name: {app}\Forums.url
Type: files; Name: {app}\BugReport.url

[Registry]
Root: HKCR; SubKey: .zzt; ValueType: string; ValueData: ZZTGame; Flags: uninsdeletekey; Tasks: registerzzt
Root: HKCR; SubKey: ZZTGame; ValueType: string; ValueData: DreamZZT Game; Flags: uninsdeletekey; Tasks: registerzzt
Root: HKCR; SubKey: ZZTGame\Shell\Open\Command; ValueType: string; ValueData: """{app}\DreamZZT.exe"" ""%1"""; Flags: uninsdeletevalue; Tasks: registerzzt
Root: HKCR; Subkey: ZZTGame\DefaultIcon; ValueType: string; ValueData: {app}\DreamZZT.exe,1; Flags: uninsdeletevalue; Tasks: registerzzt
Root: HKCR; SubKey: .sav; ValueType: string; ValueData: ZZTSaveGame; Flags: uninsdeletekey; Tasks: registerzzt
Root: HKCR; SubKey: ZZTSaveGame; ValueType: string; ValueData: DreamZZT Saved Game; Flags: uninsdeletekey; Tasks: registerzzt
Root: HKCR; SubKey: ZZTSaveGame\Shell\Open\Command; ValueType: string; ValueData: """{app}\DreamZZT.exe"" ""%1"""; Flags: uninsdeletevalue; Tasks: registerzzt
Root: HKCR; Subkey: ZZTSaveGame\DefaultIcon; ValueType: string; ValueData: {app}\DreamZZT.exe,1; Flags: uninsdeletevalue; Tasks: registerzzt
