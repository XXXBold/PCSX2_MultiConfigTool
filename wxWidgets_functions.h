#ifndef WXWIDGETS_FUNCTIONS_H_INCLUDED
  #define WXWIDGETS_FUNCTIONS_H_INCLUDED
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif

#include "PCSX2_Tool.h"

#define DEFAULT_CONFIG_STRING "<Default>"
#define FILE_CONFIG_INDICATOR ".pcsx2CFG"

extern PCSX2Tool *pcsx2Tool_m;

bool browseForFolder(const wxString &description,
                     wxString &outPath,
                     wxWindow *parent,
                     const wxString &defaultPath);

bool browseForFile(bool openFile,
                   const wxString &description,
                   wxString &outPath,
                   const wxString &fileSelectionMask,
                   wxWindow *parent,
                   const wxString &defaultPath);

bool copyFolderContents(const wxString &srcPath,
                        const wxString &destPath);

bool deleteFolderRecursive(const wxString &path);

bool checkPCSX2CFGPathValid(const wxString &path);

int checkUserCFGPathValid(const wxString &path);

bool createUserConfigIndicator(const wxString &path);

bool getSubFoldersFromPath(const wxString &path,
                           wxArrayString *folderList);

#endif //WXWIDGETS_FUNCTIONS_H_INCLUDED
