#ifndef WXWIDGETS_FUNCTIONS_H_INCLUDED
  #define WXWIDGETS_FUNCTIONS_H_INCLUDED
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif

#define DEFAULT_CONFIG_STRING "<Default>"
#define FILE_CONFIG_INDICATOR ".pcsx2CFG"

//Define this to enable debug output for gui
#define GUI_DEBUG_TRACE

#ifdef GUI_DEBUG_TRACE
  #define DEBUG_WXPUTS(str) wxPuts(str)
  //Fallback if macro not defined
  #ifndef __GNUG__
    #define __PRETTY_FUNCTION__ "__PRETTY_FUNCTION__ not defined"
  #endif
#else
  #define DEBUG_WXPUTS(str)
#endif /* GUI_DEBUG_TRACE */

bool browseForFolder(const wxString &description,
                     wxString &outPath,
                     bool browseReadOnly,
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

bool renameFolder(const wxString &before,
                  const wxString &after);

bool checkPCSX2CFGPathValid(const wxString &path);

bool checkUserCFGPathValid(const wxString &path);

bool createUserConfigIndicator(const wxString &path);

bool getSubFoldersFromPath(const wxString &path,
                           wxArrayString *folderList);

#endif //WXWIDGETS_FUNCTIONS_H_INCLUDED
