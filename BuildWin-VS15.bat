mkdir Build
cd Build
cmake -G "Visual Studio 15 Win64" ../source
cmake -G "Visual Studio 15 Win64" ../source
msbuild KoEd.sln
pause