#define MyAppName "VrmEditor"
#define MyAppVersion "1.3.2"
#define MyAppURL "https://github.com/ousttrue/vrmeditor/releases"

[Setup]
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
AppName={#MyAppName}
AppVersion={#MyAppVersion}
WizardStyle=modern
DefaultDirName={autopf}\{#MyAppName}
DefaultGroupName={#MyAppName}
Compression=lzma2
SolidCompression=yes
ChangesAssociations=yes

[Types]
;Name: "full"; Description: "Full installation"
;Name: "compact"; Description: "Compact installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "gltf"; Description: "Extension association"; Types: custom
Name: "glb"; Description: "Extension association"; Types: custom
Name: "vrm"; Description: "Extension association"; Types: custom

[Files]
Source: "prefix/bin/vrmeditor.exe"; DestDir: "{app}/bin"
Source: "prefix/bin/*.dll"; DestDir: "{app}/bin"
Source: "prefix/shaders/*"; DestDir: "{app}/shaders"
Source: "prefix/threejs_shader_chunks/*"; DestDir: "{app}/threejs_shader_chunks"

[Registry]
Root: HKCR; Subkey: "{#MyAppName}"; ValueType: string; ValueName: ""; ValueData: "Vrm Editor"; Flags: uninsdeletekey
Root: HKCR; Subkey: "{#MyAppName}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\bin\vrmeditor.EXE,0"
Root: HKCR; Subkey: "{#MyAppName}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\bin\vrmeditor.EXE"" ""%1"""
Root: HKCR; Subkey: ".gltf"; ValueType: string; ValueName: ""; ValueData: "{#MyAppName}"; Flags: uninsdeletevalue; Components: gltf
Root: HKCR; Subkey: ".glb"; ValueType: string; ValueName: ""; ValueData: "{#MyAppName}"; Flags: uninsdeletevalue; Components: glb
Root: HKCR; Subkey: ".vrm"; ValueType: string; ValueName: ""; ValueData: "{#MyAppName}"; Flags: uninsdeletevalue; Components: vrm
