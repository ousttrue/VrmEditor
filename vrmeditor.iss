#define MyAppURL "https://github.com/ousttrue/vrmeditor/releases"
#include "builddir/config.h"

[Setup]
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64
AppName={#PACKAGE}
AppVersion={#PACKAGE_VERSION}
WizardStyle=modern
DefaultDirName={autopf}\{#PACKAGE}
DefaultGroupName={#PACKAGE}
Compression=lzma2
SolidCompression=yes
ChangesAssociations=yes

[Types]
;Name: "full"; Description: "Full installation"
;Name: "compact"; Description: "Compact installation"
Name: "custom"; Description: "Custom installation"; Flags: iscustom

[Components]
Name: "gltf"; Description: ".gltf Extension association"; Types: custom
Name: "glb"; Description: ".glb Extension association"; Types: custom
Name: "vrm"; Description: ".vrm Extension association"; Types: custom

[Files]
Source: "prefix\bin\vrmeditor.exe"; DestDir: "{app}\bin"
Source: "prefix\bin\*.dll"; DestDir: "{app}\bin"
Source: "prefix\shaders\*"; DestDir: "{app}\shaders"
Source: "prefix\shaders\khronos\*"; DestDir: "{app}\shaders\khronos"
Source: "prefix\threejs_shader_chunks\*"; DestDir: "{app}\threejs_shader_chunks"

[Registry]
Root: HKCR; Subkey: "{#PACKAGE}"; ValueType: string; ValueName: ""; ValueData: "Vrm Editor"; Flags: uninsdeletekey
Root: HKCR; Subkey: "{#PACKAGE}\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\bin\vrmeditor.EXE,0"
Root: HKCR; Subkey: "{#PACKAGE}\shell\open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\bin\vrmeditor.EXE"" ""%1"""

Root: HKCU; Subkey: ".gltf"; Flags: deletekey; Components: gltf
Root: HKCR; Subkey: ".gltf"; ValueType: string; ValueName: ""; ValueData: "{#PACKAGE}"; Flags: uninsdeletevalue; Components: gltf

Root: HKCU; Subkey: ".glb"; Flags: deletekey; Components: glb
Root: HKCR; Subkey: ".glb"; ValueType: string; ValueName: ""; ValueData: "{#PACKAGE}"; Flags: uninsdeletevalue; Components: glb

Root: HKCU; Subkey: ".vrm"; Flags: deletekey; Components: vrm
Root: HKCR; Subkey: ".vrm"; ValueType: string; ValueName: ""; ValueData: "{#PACKAGE}"; Flags: uninsdeletevalue; Components: vrm

