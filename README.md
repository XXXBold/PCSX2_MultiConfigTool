# PCSX2_MultiConfigTool

This tool can be used to manage diffrent configs and create Shortcuts for direct starting games with PCSX2 https://github.com/PCSX2/pcsx2.

The tool will not modify your original PCSX2 data without your knowledge, if you don't want to use it anymore, just delete it and continue using PCSX2 as before.

There are pre-built binaries available for Windows (32 and 64 bit) and for linux (GTK-based, 64 bit). Windows' .exe are both standalones, for linux you have to install the libwxgtk3.0-gtk3 runtime to run the binary (On Ubuntu/Mint: sudo apt install libwxgtk3.0-gtk3-0v5) or install wxwidgets >=3.0.4  dev files to build yourself.
Find the binaries here: https://github.com/XXXBold/PCSX2_MultiConfigTool/releases.

To change the GUI, use https://github.com/wxFormBuilder/wxFormBuilder for editing PCSX2Tool.fbp and use the generated C++ Source in the Project.

Let me know if you have suggestions, wishes, find a bug or if you need help or more information.

## Releases
- 0.1: First Release
- 0.2-beta: Added Some new options
- 0.3-beta: Added support for linux, updated to new codebase
- 0.4-beta: Improved UI Controls, enhanced shortcut creation (Overwrite-confirm dialog), ask to set executable bit for linux version directly
- 0.5-beta: Added more options for creating shortcuts, Updated icon, is now shown correctly, added license, for linux: Update to wxWidgets based on GTK3

## TODOs
- Add build instructions
- code cleanup

## Credits & References
- wxWidgets Library: http://www.wxwidgets.org/
- wxFormBuilder: https://github.com/wxFormBuilder/wxFormBuilder
