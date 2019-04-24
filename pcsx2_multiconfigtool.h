#ifndef PCSX2_TOOL_H_INCLUDED
#define PCSX2_TOOL_H_INCLUDED
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif

#include <string>
#include "appconfig.h"

#define MAX_PATHLEN 260

#ifdef _WIN32
  #define PCSX2_TOOL_PATH_DEFAULT_EXE     "C:\\Program Files (x86)\\"
  #define PCSX2_TOOL_PATH_DEFAULT_CFG     "C:\\Users\\"
  #define PCSX2_TOOL_PATH_DEFAULT_USR_CFG "C:\\Users\\"
  #define PCSX2_TOOL_PATH_DEFAULT_GAMES   "C:\\Users\\"
#elif __linux__
  #define PCSX2_TOOL_PATH_DEFAULT_EXE     "/usr/games/"
  #define PCSX2_TOOL_PATH_DEFAULT_CFG     "/home/"
  #define PCSX2_TOOL_PATH_DEFAULT_USR_CFG "/home/"
  #define PCSX2_TOOL_PATH_DEFAULT_GAMES   "/home/"
#else
  #error not implemented
#endif /* _WIN32 */

#define APP_DISPLAY_NAME       "PCSX2_MultiConfigTool"
#define APP_VERSION_MAJOR      0
#define APP_VERSION_MINOR      3
#define APP_VERSION_PATCH      0
#define APP_VERSION_DEVSTATE   "Beta"
#define APP_VERSION_BUILDTIME  "Build: " __DATE__ ", " __TIME__
#define APP_PROJECT_HOMEPAGE   "https://github.com/XXXBold/PCSX2_MultiConfigTool"
#define APP_PROJECT_DEVELOPER  "XXXBold"

enum
{
  PCSX2_TOOL_CFG_NEW,
  PCSX2_TOOL_CFG_EXISTING,
  PCSX2_TOOL_CFG_ERROR,

  ENTRY_INDEX_PATH_EXE = 0,
  ENTRY_INDEX_PATH_PCSX2CFG,
  ENTRY_INDEX_PATH_USRCFG,
  ENTRY_INDEX_PATH_GAMES,
  ENTRY_INDEX_OPT_START_NOGUI,
  ENTRY_INDEX_OPT_START_FULLSCREEN,
  ENTRY_INDEX_OPT_START_FULLBOOT
};

using std::string;

class PCSX2Tool
{
public:
  PCSX2Tool(void);
  ~PCSX2Tool(void);

  int LoadToolConfig(void);
  bool SaveToolConfig(void);

  string GetPCSX2GamesPath(void);
  string GetPCSX2ExePath(void);
  string GetPCSX2CFGPath(void);
  string GetUserCFGPath(void);
  string GetToolCFGPath(void);

  bool GetOptionStartNoGUI(void);
  bool GetOptionStartFullScreen(void);
  bool GetOptionStartFullBoot(void);

  bool SetPCSX2GamesPath(const string &path);
  bool SetPCSX2ExePath(const string &path);
  bool SetPCSX2CFGPath(const string &path);
  bool SetUserCFGPath(const string &path);

  void SetOptionStartNoGUI(bool noGUI);
  void SetOptionStartFullScreen(bool fullscreen);
  void SetOptionStartFullBoot(bool fullboot);

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
  AppConfig tagAppConfig;
  AppConfigEntry taEntries[7];
};

extern PCSX2Tool *pcsx2Tool_m;

#endif // PCSX2_TOOL_H_INCLUDED
