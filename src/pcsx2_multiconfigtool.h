#ifndef PCSX2_TOOL_H_INCLUDED
#define PCSX2_TOOL_H_INCLUDED

#include <string>
#include "appconfig.h"

#define MAX_PATHLEN 260

#ifdef _WIN32
  #define PCSX2_TOOL_PATH_DEFAULT_EXE     "C:\\Program Files (x86)\\"
  #define PCSX2_TOOL_PATH_DEFAULT_CFG     "C:\\Users\\"
  #define PCSX2_TOOL_PATH_DEFAULT_USR_CFG "C:\\Users\\"
  #define PCSX2_TOOL_PATH_DEFAULT_GAMES   "C:\\Users\\"
#elif __linux__
  #define PCSX2_TOOL_PATH_DEFAULT_EXE     "/usr/games/PCSX2-linux.sh"
  #define PCSX2_TOOL_PATH_DEFAULT_CFG     "/home/"
  #define PCSX2_TOOL_PATH_DEFAULT_USR_CFG "/home/"
  #define PCSX2_TOOL_PATH_DEFAULT_GAMES   "/home/"
#else
  #error not implemented
#endif /* _WIN32 */

#define APP_DISPLAY_NAME       "PCSX2_MultiConfigTool"
#define APP_VERSION_MAJOR      0
#define APP_VERSION_MINOR      6
#define APP_VERSION_PATCH      0
#define APP_VERSION_DEVSTATE   "Beta"
#define APP_VERSION_BUILDTIME  "Build: " __DATE__ ", " __TIME__
#define APP_PROJECT_HOMEPAGE   "https://github.com/XXXBold/PCSX2_MultiConfigTool"
#define APP_PROJECT_DEVELOPER  "XXXBold"

#ifdef __i386__
  #define APP_VERSION_ARCH "32-bit"
#elif __x86_64__
  #define APP_VERSION_ARCH "64-bit"
#else
  #define APP_VERSION_ARCH "\?\?-bit"
#endif

enum
{
  PCSX2_TOOL_CFG_NEW,
  PCSX2_TOOL_CFG_EXISTING,
  PCSX2_TOOL_CFG_ERR_INTERNAL,
  PCSX2_TOOL_CFG_ERR_DATA_CORRUPT,

  ENTRY_INDEX_PATH_EXE = 0,
  ENTRY_INDEX_PATH_PCSX2CFG,
  ENTRY_INDEX_PATH_USRCFG,
  ENTRY_INDEX_PATH_GAMES,
  ENTRY_INDEX_OPT_START_NOGUI,
  ENTRY_INDEX_OPT_START_FULLSCREEN,
  ENTRY_INDEX_OPT_START_FULLBOOT,
  ENTRY_INDEX_OPT_START_FORCE_CONSOLE,
  ENTRY_INDEX_OPT_START_NO_HACKS,
};

using std::string;

class PCSX2Tool
{
  public:
    PCSX2Tool();
    ~PCSX2Tool();

    int iLoadConfig();
    bool bSaveToolConfig();
    bool bDeleteConfig();

    string GetPCSX2GamesPath();
    string GetPCSX2ExePath();
    string GetPCSX2CFGPath();
    string GetUserCFGPath();
    string GetToolCFGPath();

    bool GetOptionStartNoGUI();
    bool GetOptionStartFullScreen();
    bool GetOptionStartFullBoot();
    bool GetOptionStartForceConsole();
    bool GetOptionStartNoHacks();

    bool SetPCSX2GamesPath(const string &path);
    bool SetPCSX2ExePath(const string &path);
    bool SetPCSX2CFGPath(const string &path);
    bool SetUserCFGPath(const string &path);

    void SetOptionStartNoGUI(bool noGUI);
    void SetOptionStartFullScreen(bool fullscreen);
    void SetOptionStartFullBoot(bool fullboot);
    void SetOptionStartForceConsole(bool forceConsole);
    void SetOptionStartNoHacks(bool noHacks);

    void CreateCommandLine_PCSX2StartWithCFG(const string &configName,
                                             const string &gamePath,
                                             string &cmdLine);

    void CreateCommandLine_PCSX2StartWithCFG(const string &configName,
                                           string &cmdLine);
  private:
    /* Paths */
    char caBufPathGames[MAX_PATHLEN];
    char caBufPathExe[MAX_PATHLEN];
    char caBufPathPCSX2Cfg[MAX_PATHLEN];
    char caBufPathUserCfg[MAX_PATHLEN];
    /* Appconfig and entries */
    AppConfig appCfg;
    AppConfigEntry taEntries[9];
};

extern PCSX2Tool *pcsx2Tool_m;

#endif // PCSX2_TOOL_H_INCLUDED
