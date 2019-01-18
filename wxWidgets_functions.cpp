#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
  #include <wx/dir.h>
  #include <wx/filename.h>
  #include <wx/filesys.h>
#endif

#include "wxWidgets_functions.h"

bool browseForFolder(const wxString &description,
                     wxString &outPath,
                     wxWindow *parent,
                     const wxString &defaultPath)
{
  wxDirDialog pathSelector(parent,description,defaultPath,wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
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
    wxPuts(wxString::Format("Copy %s -> %s",arrFiles[iIndex],strDestPath+strName));

    if(!wxCopyFile(arrFiles[iIndex],strDestPath + strName))
      return(false);
  }
  return(true);
}

bool deleteFolderRecursive(const wxString &path)
{
  wxString strPath=path;

  if(path.Last()!=wxFILE_SEP_PATH)
    strPath.append(wxFILE_SEP_PATH);
  return(wxDir::Remove(strPath,wxPATH_RMDIR_RECURSIVE));
}

bool checkPCSX2CFGPathValid(const wxString &path)
{
  wxFileSystem fileIterator;
  wxString strPath=path;

  if(path.Last()!=wxFILE_SEP_PATH)
    strPath.append(wxFILE_SEP_PATH);

  fileIterator.ChangePathTo(strPath);
  if(fileIterator.FindFirst("*.ini").IsEmpty())
    return(false);
  return(true);
}

bool createUserConfigIndicator(const wxString &path)
{
  wxFileSystem fileIterator;
  wxString strPath=path;
  wxFile newFile;
  if(path.Last()!=wxFILE_SEP_PATH)
    strPath.append(wxFILE_SEP_PATH);
  strPath.append(FILE_CONFIG_INDICATOR);

  fileIterator.ChangePathTo(strPath);
  if(fileIterator.FindFirst("*").IsEmpty())

  if(!newFile.Create(strPath))
    return(false);
  return(newFile.Close());
}

int checkUserCFGPathValid(const wxString &path)
{
  wxFileSystem fileIterator;
  wxString strPath=path;
  wxString strCurrFolder;
  if(path.Last()!=wxFILE_SEP_PATH)
    strPath.append(wxFILE_SEP_PATH);
  if(!wxDir::Exists(strPath))
    return(-1);
  fileIterator.ChangePathTo(strPath);
  if(fileIterator.FindFirst("*").IsEmpty())
    return(0);
//wxPuts("First file: " + fileIterator.FindFirst("*"));

  if(fileIterator.FindFileInPath(&strCurrFolder,strPath,FILE_CONFIG_INDICATOR))
    return(1);
  return(-1);
}

bool getSubFoldersFromPath(const wxString &path,
                           wxArrayString *folderList)
{
  wxFileSystem fileIterator;
  wxString strCurrFolder;
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
