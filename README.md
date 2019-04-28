# PCSX2_MultiConfigTool

This tool can be used to manage diffrent configs and create Shortcuts for direct starting games with PCSX2 https://github.com/PCSX2/pcsx2.

The tool will not modify your original PCSX2 data without your knowledge, if you don't want to use it anymore, just delete it and continue using PCSX2 as before.

To build it for Windows, I'm using MSYS2/Mingw.
For Linux, install the packet "libwxgtk3.0-dev" via packet manager and use GCC to build.
To edit the GUI, use https://github.com/wxFormBuilder/wxFormBuilder for editing PCSX2Tool.fbp and use the generated C++ Source in the Project.

If you occur any problems, or have ideas for improvments, let me know.

I'll put precompiled versions here: https://github.com/XXXBold/PCSX2_MultiConfigTool/releases. 

Tell me if you need help or more information.

## Releases
- 0.1: First Release
- 0.2-beta: Added Some new options
- 0.3-beta: Added support for linux, updated to new codebase
- 0.4-beta: Improved UI Controls, enhanced shortcut creation (Overwrite-confirm dialog), ask to set executable bit for linux version directly

## TODOs
- Add build instructions
- code cleanup

## Credits & References
- wxWidgets Library: http://www.wxwidgets.org/
