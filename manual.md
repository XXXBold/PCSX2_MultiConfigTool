# User Manual for PCSX2_MultiConfigTool

## This Document describes how to use the PCSX2 MultiConfigTool

At the First Start, you need to set up some Paths to make it work correctly.
- Games-Path: 
  The Path where you've stored the .iso Files.
- PCSX2-Exe-Path: 
  Locate your Executable (pcsx2.exe), This Path will be used when Starting PCSX2 over the tool or when you create a new Game Shortcut.
- Configs Path (PCSX2):
  Your current (default) "inis" Folder, created and used per default from PCSX2. This will be the "Default" entry in your configs List.
- Configs Path (User):  
  The Path where you want to store new Configurations, this must be an empty Folder, or, if you used the Tool before, the previous used location.
  
After Selecting all Paths, you can Manage your configs. The "Default" entry refers to your original PCSX2 Config, which cannot be deleted or renamed by the tool.
If you click "Create new Config", an Input will open, allowing you to specify the new config's Name. 
After "Save", the data from your previously selected config are copied to the new config.

You can now Select your new Config and Start PCSX2 and edit your settings as needed. If you're done, Goto "File->Create New Game Shortcut" to create a new Game Shortcut.
For more Options, goto "Options->Game Shortcuts" (>=V0.2) to select additional parameters for new created shortcuts. Attention: These settings will not apply to already existing shortcuts.

### Options
- "Disable PCSX2-GUI on Startup": (--nogui) Disable the PCSX2 GUI if started, useful if you want to just start your game & play.
- "Start in Fullscreen": (--fullscreen) Start game directly in fullscreen
- "Use Full Boot on Startup": (--fullboot) Disable fast boot, perform a full PS2-Boot.
#### from 0.5-beta
- "Force PCSX2-Console to be visible": (--console) Force the PCSX2 Console to show up
- "Disable Speedhacks": (--nohacks) Disable speedhacks

For further information, see PCSX2-CLI reference: https://wiki.pcsx2.net/index.php/Command-line_support


You can use a configuration for as many Games as you want, they'll share all their settings while using the same config.

If you delete a config, all Game shortcuts refering to it will not work anymore.
