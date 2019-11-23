SET SOURCE_DIR=%cd%
c:\Qt\5.12.3\msvc2017_64\bin\qtenv2.bat | EXIT /b 1
call "%ProgramFiles(x86)%\Microsoft Visual Studio\2019\Community\Common7\Tools\vsdevcmd" -arch=x64
cd %SOURCE_DIR%
rm -rf build
mkdir build
cd build
cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ../
cmake --build . --config Release