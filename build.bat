@ECHO ON

RMDIR /Q /S build
MKDIR build
PUSHD build

Echo batch1 folder is: %~dp0

@echo off
CALL %~dp0\env\Scripts\activate

conan install .. --build=missing
cmake .. -G "Visual Studio 17 2022" -A "x64"
cmake --build . --config Release

bin\dbconnector.exe