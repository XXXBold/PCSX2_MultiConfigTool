#ifndef PCSX2_TOOL_H_INCLUDED
  #define PCSX2_TOOL_H_INCLUDED

#include <string>
#include "inifile.h"

enum
{
  PCSX2_TOOL_CFG_NEW,
  PCSX2_TOOL_CFG_LOAD,
  PCSX2_TOOL_CFG_ERROR
};

using namespace std;

class PCSX2Tool
{
public:
  PCSX2Tool();
  ~PCSX2Tool();

  string toolGetVersionString(void);
  string toolGetAppName(void);

  int LoadToolConfig(const string &userConfigDir);
  bool SaveToolConfig();

  string GetPCSX2GamesPath();
  string GetPCSX2ExePath();
  string GetPCSX2CFGPath();
  string GetUserCFGPath();
  string GetToolCFGPath();

  bool GetOptionStartNoGUI(void);
  bool GetOptionStartFullScreen(void);
  bool GetOptionStartFullBoot(void);

  void SetPCSX2GamesPath(const string &path);
  void SetPCSX2ExePath(const string &path);
  void SetPCSX2CFGPath(const string &path);
  void SetUserCFGPath(const string &path);

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
  string strToolCFGPath;
  string strPCSX2GamesPath;
  string strPCSX2EXEPath;
  string strPCSX2CFGPath;
  string strUserCFGPath;
  /* Options */
  bool bOptionStartNoGUI;
  bool bOptionStartFullscreen;
  bool bOptionStartFullBoot;
  /* Section/Keys for paths */
  const char *pcSectionPaths;
  const char *pcKeyPCSX2GamesPath;
  const char *pcKeyPCSX2EXE;
  const char *pcKeyPCSX2CFG;
  const char *pcKeyUserCFG;
  /* Section/Keys for Options */
  const char *pcSectionOptions;
  const char *pcKeyStartPCSX2GUI;
  const char *pcKeyStartFullScreen;
  const char *pcKeyStartFullBoot;

  Inifile appCFGFile;
};

#endif // PCSX2_TOOL_H_INCLUDED
