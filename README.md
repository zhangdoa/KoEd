# KoEd
[![GPL-3.0 licensed](https://img.shields.io/badge/license-GPL--3.0-brightgreen.svg)](LICENSE.md)

## How to use
```shell
KoEd.exe -[option] "[libraryName].nicnt"
```

1. Extract data section files

2. Modify
[libraryName]_ProductHints.xml contains product related info
[libraryName]_Wallpaper.png is the embedded rack wallpaper
[libraryName]_LibInfo.xml contains instrument and sample related info

3. Repack
Please keep all data section file name unchanged before repack

### Avaliable options
```shell
-e //extract data section files from .nicnt file
-c //create .nicnt file from unpacked data section files
```

## Dependencies
- CMake 3.10+

## How to build
Run BuildWin-VS15.bat on Windows