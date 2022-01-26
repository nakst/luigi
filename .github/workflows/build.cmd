@echo off
setlocal enabledelayedexpansion
where /Q cl.exe || (
set __VSCMD_ARG_NO_LOGO=1
  for /f "tokens=*" %%i in ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -requires Microsoft.VisualStudio.Workload.NativeDesktop -property installationPath') do set VS=%%i
    if "!VS!" equ "" (
      echo ERROR: Visual Studio installation not found
      exit /b 1
    )  
  call "!VS!\VC\Auxiliary\Build\vcvarsall.bat" amd64 || exit /b 1
)
cl luigi_example.c -DUI_WINDOWS -luser32
