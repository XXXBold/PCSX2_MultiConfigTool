# PCSX2_MultiConfigTool

This tool can be used to manage diffrent configs and create Shortcuts for direct starting games with PCSX2.

The tool will not modify your original PCSX2 data without your knowledge, if you don't want to use it anymore, just delete it and continue using PCSX2 as before.

There are pre-built binaries available for Windows (64 bit, 32-bit no longer maintained after v0.5. Let me know if someone still needs 32-bit) and for linux (GTK-based, 64 bit). Windows' .exe are both standalones, for linux you have to install the libwxgtk3.0-gtk3 runtime to run the binary (On Ubuntu/Mint: sudo apt install libwxgtk3.0-gtk3-0v5) or install wxwidgets >=3.0.4  dev files to build yourself.
Find the binaries here: https://github.com/XXXBold/PCSX2_MultiConfigTool/releases.

To change the GUI, use https://github.com/wxFormBuilder/wxFormBuilder for editing resources/PCSX2Tool.fbp and use the generated C++ Source in the Project.

Let me know if you have suggestions, wishes, find a bug or if you need help or more information.

## Build instructions
From Version >=0.6, the buildsystem was changed to meson. There are scripts available to invoke the building process for both Linux and Windows. For Windows, MSYS2 is needed in order to use the script.

### Linux (Ubuntu 18.04 and similar)
- Install Packages from repo: `sudo apt install build-essential libwxgtk3.0-gtk3-dev python3-pip ninja-build`
- Install meson via Python pip: `pip3 install meson`
- Relog to recreate path variable for meson
- Navigate to folder containing this project
- Execute `./build_linux.sh`, which should build everything

### Windows/MSYS2
- Download MSYS2 from https://www.msys2.org/ (preferable the x86_64 setup, it's also able to build 32-bit executables, if desired).
- Run setup to install MSYS2
- Execute MSYS2 using the shortcut "MSYS2 Mingw 64-bit"
- Update the Packages: `pacman -Suyy`, if requested, close the programm. Open again, run `pacman -Suyy` a second time (Can take a while). Packages should now be up to date.
- Install the necessary Packages: `pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-wxWidgets ninja-vim mingw-w64-x86_64-meson` This will take a while.
- Within the MSYS2 (MinGW64)-shell, navigate to your local folder, containing this project.
- Execute `./build_windows.sh`, which should build everything

## Releases
- 0.7-beta: Improved window (re) sizeing, sometimes not all UI-Elements were visible, improved process spawn handling (Now throws error if executable path doesn't exist), changed executable name to lowercase.
- 0.6-beta: Added Better handling of PCSX2-Process spawning, minor UI fixes, some cleanups. Reorganized files, meson is now used as the build system. Buildscripts are now available for both platforms.
- 0.5-beta: Added more options for creating shortcuts, Updated icon, is now shown correctly, added license, for linux: Update to wxWidgets based on GTK3
- 0.4-beta: Improved UI Controls, enhanced shortcut creation (Overwrite-confirm dialog), ask to set executable bit for linux version directly
- 0.3-beta: Added support for linux, updated to new codebase
- 0.2-beta: Added Some new options
- 0.1: First Release



## TODOs
- code cleanup

## Credits & References
- PCSX2 Playstation 2 Emulator: https://pcsx2.net/ / https://github.com/PCSX2/pcsx2
- wxWidgets Library: http://www.wxwidgets.org/
- wxFormBuilder: https://github.com/wxFormBuilder/wxFormBuilder
- meson: https://mesonbuild.com/index.html
- MSYS2: https://www.msys2.org/
