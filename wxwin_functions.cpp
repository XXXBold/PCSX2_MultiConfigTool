#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
  #include <wx/dir.h>
  #include <wx/filename.h>
  #include <wx/filesys.h>
#endif

#include "wxwin_functions.h"

bool browseForFolder(const wxString &description,
                     wxString &outPath,
                     bool browseReadOnly,
                     wxWindow *parent,
                     const wxString &defaultPath)
{
  wxDirDialog pathSelector(parent,
                           description,
                           defaultPath,
                           ((browseReadOnly)?wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST:wxDD_DEFAULT_STYLE));
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(pathSelector.ShowModal() == wxID_CANCEL)
    return(false);     // the user changed idea...
  outPath=pathSelector.GetPath();
  return(true);
}

bool browseForFile(bool openFile,
                   const wxString &description,
                   wxString &outPath,
                   const wxString &fileSelectionMask,
                   wxWindow *parent,
                   const wxString &defaultPath)
{
  wxFileDialog fileSelector(parent,
                            _(description),
                            defaultPath,
                            "",
                            fileSelectionMask,
                            (openFile)?wxFD_OPEN|wxFD_FILE_MUST_EXIST:wxFD_SAVE);
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(fileSelector.ShowModal() == wxID_CANCEL)
    return(false);
  outPath=fileSelector.GetPath();
  return(true);
}

bool copyFolderContents(const wxString &srcPath,
                        const wxString &destPath)
{
  /* Beg for mercy for this ugly function... */
  wxArrayString arrFiles;
  wxString strSrcPath, strDestPath, strName, strExt;
  strSrcPath=srcPath;
  strDestPath=destPath;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);

  if(!strSrcPath.EndsWith(wxFILE_SEP_PATH))
    strSrcPath.append(wxFILE_SEP_PATH);
  if(!strDestPath.EndsWith(wxFILE_SEP_PATH))
    strDestPath.append(wxFILE_SEP_PATH);

  if(!wxFileName::DirExists(strSrcPath))
    return(false);
  if(wxFileName::DirExists(strDestPath))
  {
    if(wxMessageBox("Folder " + strDestPath + " already exists, continue with copy? All files will be overwritten.",wxMessageBoxCaptionStr,wxYES_NO)!=wxYES)
      return(false);
  }
  else if(!wxDir::Make(strDestPath)) /* Initially make dest dir, if needed */
    return(false);

  wxDir::GetAllFiles(strSrcPath,&arrFiles);

//strDestPath=fnDestObj.GetPathWithSep();
  for(unsigned int iIndex=0;iIndex<arrFiles.GetCount();++iIndex)
  {
//  strDestPath=fnDestObj.GetPathWithSep();
    wxFileName::SplitPath(arrFiles[iIndex],NULL,&strName,&strExt);
    if(!strExt.IsEmpty())
      strName+="." + strExt;
    DEBUG_WXPUTS(wxString::Format("Copy %s -> %s",arrFiles[iIndex],strDestPath+strName));

    if(!wxCopyFile(arrFiles[iIndex],strDestPath + strName))
      return(false);
  }
  return(true);
}

bool deleteFolderRecursive(const wxString &path)
{
  wxString strPath=path;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(path.Last()!=wxFILE_SEP_PATH)
    strPath.append(wxFILE_SEP_PATH);
  return(wxDir::Remove(strPath,wxPATH_RMDIR_RECURSIVE));
}

bool renameFolder(const wxString &before,
                  const wxString &after)
{
  if(!wxDir::Exists(before))
    return(false);

  return(wxRenameFile(before,after,false));
}

bool checkPCSX2CFGPathValid(const wxString &path)
{
  wxFileSystem fileIterator;
  wxString strPath=path;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(path.Last()!=wxFILE_SEP_PATH)
    strPath.append(wxFILE_SEP_PATH);

  if(!wxDir::Exists(strPath))
    return(false);

  fileIterator.ChangePathTo(strPath);
  strPath=fileIterator.FindFirst("*");
  if(strPath.IsEmpty()) /* No file found */
    return(false);
  DEBUG_WXPUTS("First file found: " + strPath);
  /* Found some file, now test if it's an .ini file */
  return(strPath.EndsWith(".ini"));
}

bool checkUserCFGPathValid(const wxString &path)
{
  wxFileSystem fileIterator;
  wxString strPath=path;
  wxString strCurrFolder;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(path.Last()!=wxFILE_SEP_PATH)
    strPath.append(wxFILE_SEP_PATH);
  if(!wxDir::Exists(strPath))
    return(false);

  fileIterator.ChangePathTo(strPath);
  if(fileIterator.FindFirst("*").IsEmpty())
    return(true);
  DEBUG_WXPUTS("First file: " + fileIterator.FindFirst("*"));

  if(fileIterator.FindFileInPath(&strCurrFolder,strPath,FILE_CONFIG_INDICATOR))
    return(true);
  return(false);
}

bool createUserConfigIndicator(const wxString &path)
{
  wxFileSystem fileIterator;
  wxString strPath=path;
  wxFile newFile;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(path.Last()!=wxFILE_SEP_PATH)
    strPath.append(wxFILE_SEP_PATH);
  strPath.append(FILE_CONFIG_INDICATOR);

  if(wxFile::Exists(strPath))
    return(true);

  fileIterator.ChangePathTo(strPath);
  if(fileIterator.FindFirst("*").IsEmpty())
  {
    if(!newFile.Create(strPath))
      return(false);

    return(newFile.Close());
  }
  return(false);
}

bool getSubFoldersFromPath(const wxString &path,
                           wxArrayString *folderList)
{
  wxFileSystem fileIterator;
  wxString strCurrFolder;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(!wxDir::Exists(path))
    return(false);
  fileIterator.ChangePathTo(path);
  if((strCurrFolder=fileIterator.FindFirst("*",wxDIR).AfterLast('/')).IsEmpty())
    return(true);
  folderList->Add(strCurrFolder);
  while(!(strCurrFolder=fileIterator.FindNext().AfterLast('/')).IsEmpty())
  {
    folderList->Add(strCurrFolder);
  }
  return(true);
}
