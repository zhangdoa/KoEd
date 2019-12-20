mkdir Build
cd Build
cmake -G "Visual Studio 15 Win64" ../Source
cmake -G "Visual Studio 15 Win64" ../Source
msbuild KoEd.sln /property:Configuration="Debug" /m
msbuild KoEd.sln /property:Configuration="Release" /m
pause