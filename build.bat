@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
"c:\Program Files\git\usr\bin\sh.exe" --login toolchain\install.sh toolchain
toolchain\host\bin\ninja.exe -f msvc.ninja