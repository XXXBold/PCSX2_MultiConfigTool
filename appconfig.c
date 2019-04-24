#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "appconfig.h"
#include "inifile.h"
#include "datatypes.h"

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #define PATH_SEPARATOR '\\'
  #define ENV_VARNAME_1 "APPDATA"
#elif __linux__
  #include <unistd.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #define PATH_SEPARATOR '/'
  #define ENV_VARNAME_1 "XDG_CONFIG_HOME"
  #define ENV_VARNAME_2 "HOME"
  #define LINUX_CFG_DIR ".config"
#endif

#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* >= C99 */
  #define INLINE_FCT inline
  #define INLINE_PROT static inline
#else /* No inline available from C Standard */
  #define INLINE_FCT static
  #define INLINE_PROT static
#endif /* __STDC_VERSION__ >= C99 */

#define DEFAULT_FILENAME "config.ini"

#ifdef APPCONFIG_PRINT_ERRORS
  #define ERR_PRINT(str)           fputs("AppConfig Error: " str "\n",stderr)
  #define ERR_PRINT_VARGS(str,...) fprintf(stderr,"AppConfig Error: " str "\n",__VA_ARGS__)
#else
  #define ERR_PRINT(str)
  #define ERR_PRINT_VARGS(str,...)
#endif /* APPCONFIG_PRINT_ERRORS */

enum
{
  MAX_PATHLEN = 260,
};

struct TagAppConfig_T
{
  char caPath[MAX_PATHLEN+1];
  AppConfigEntry *pEntries;
  size_t szEntriesCount;
  union StoreMethod_T /* Here add probably more methods for storage, e.g. registry for windows... */
  {
    Inifile iniFile;
  }store;
};

INLINE_PROT int iAppConfig_AssembleDestFolderPath_m(AppConfig pCfg,
                                                    unsigned int *puiBufSize,
                                                    const char *pcAppName,
                                                    const char *pcFileName,
                                                    const char *pcLocation);
INLINE_PROT int iAppConfig_Create_m(AppConfig pCfg);
INLINE_PROT int iAppConfig_GetDefaultConfigPath_m(char *pcPath,
                                                  unsigned int *uiBufsize);
INLINE_PROT int iAppConfig_FileExists_m(const char *pcPath);
INLINE_PROT int iAppConfig_CreateDirIfNotExist_m(const char *pcDir);
INLINE_PROT int iAppConfig_Inifile_Load_m(AppConfig pCfg);
INLINE_PROT int iAppConfig_Inifile_Save_m(const AppConfig pCfg);

int appConfig_Load(AppConfig *config,
                   const char *appName,
                   AppConfigEntry *entries,
                   size_t entriesCount,
                   const char *fileName,
                   const char *location)
{
  unsigned int uiTmp;

  if((!entries) || (!entriesCount))
    return(APPCONFIG_LOAD_ERROR);

  if(!((*config)=malloc(sizeof(struct TagAppConfig_T))))
  {
    ERR_PRINT("malloc() failed");
    return(APPCONFIG_LOAD_ERROR);
  }

  if(!fileName)
    fileName=DEFAULT_FILENAME;

  uiTmp=MAX_PATHLEN;
  if(iAppConfig_AssembleDestFolderPath_m((*config),
                                         &uiTmp,
                                         appName,
                                         fileName,
                                         location))
  {
    ERR_PRINT("iAppConfig_AssembleDestFolderPath_m() failed");
    free(*config);
    return(APPCONFIG_LOAD_ERROR);
  }
  if(iAppConfig_CreateDirIfNotExist_m((*config)->caPath) < 0)
  {
    ERR_PRINT("iAppConfig_CreateDirIfNotExist_m() failed");
    free(*config);
    return(APPCONFIG_LOAD_ERROR);
  }
  /* bufferlength is checked in AssembleDestFolderPath, so strcpy is fine */
  strcpy(&(*config)->caPath[uiTmp],fileName);
  (*config)->pEntries=entries;
  (*config)->szEntriesCount=entriesCount;

  if(iAppConfig_Create_m(*config))
  {
    ERR_PRINT("iAppConfig_Create_m() failed");
    free(*config);
    return(APPCONFIG_LOAD_ERROR);
  }
  if(iAppConfig_FileExists_m((*config)->caPath))
  {
    /* Load File if it exists */
    if(iAppConfig_Inifile_Load_m((*config)))
    {
      ERR_PRINT("iAppConfig_Inifile_Load_m() failed");
      free(*config);
      return(APPCONFIG_LOAD_ERROR);
    }
    return(APPCONFIG_LOAD_EXIST);
  }
  return(APPCONFIG_LOAD_NEW);
}

int appConfig_Save(const AppConfig config)
{
  return(iAppConfig_Inifile_Save_m(config));
}

const char *appConfig_GetPath(const AppConfig config)
{
  return(config->caPath);
}

void appConfig_Close(AppConfig config)
{
  IniFile_Dispose(config->store.iniFile);
  free(config);
}

INLINE_FCT int iAppConfig_Inifile_Load_m(AppConfig pCfg)
{
  unsigned int uiIndex;
  int iRc;

  if((iRc=IniFile_Read(pCfg->store.iniFile,
                       pCfg->caPath) != INI_ERR_NONE))
  {
    ERR_PRINT_VARGS("IniFile_Read() failed with error %d",iRc);
    return(-1);
  }
  /* Read all entries, if they exist in the file */
  for(uiIndex=0;uiIndex < pCfg->szEntriesCount;++uiIndex)
  {
    switch((iRc=IniFile_FindEntry_GetValue(pCfg->store.iniFile,
                                           pCfg->pEntries[uiIndex].groupName,
                                           pCfg->pEntries[uiIndex].keyName,
                                           &pCfg->pEntries[uiIndex].tagData)))
    {
      /* These errors are okay */
      case INI_ERR_FIND_NONE:
      case INI_ERR_FIND_SECTION:
        ERR_PRINT_VARGS("IniFile_FindEntry_GetValue() returned %d: Key \"%s\" in group \"%s\" not found",
                        iRc,
                        pCfg->pEntries[uiIndex].keyName,
                        pCfg->pEntries[uiIndex].groupName);
        break;
      case INI_ERR_NONE: /* Entry found */
        break;
      default:
        ERR_PRINT_VARGS("IniFile_FindEntry_GetValue() failed with error %d",iRc);
        return(-1);
    }
  }
  /* Got all needed entries, clear inifile data */
  IniFile_DumpContent(pCfg->store.iniFile);   // TODO: remove this
  IniFile_Clean(pCfg->store.iniFile);
  return(0);
}

INLINE_FCT int iAppConfig_Inifile_Save_m(const AppConfig pCfg)
{
  unsigned int uiIndex;
  int iRc;
  for(uiIndex=0;uiIndex < pCfg->szEntriesCount;++uiIndex)
  {
    switch((iRc=IniFile_CreateEntry_SetValue(pCfg->store.iniFile,
                                             pCfg->pEntries[uiIndex].groupName,
                                             pCfg->pEntries[uiIndex].keyName,
                                             &pCfg->pEntries[uiIndex].tagData)))
    {
      case INI_ERR_NONE: /* Okay */
        break;
      default: /* some Error occured, indicate error... */
        ERR_PRINT_VARGS("Failed to set val for Key %s in group %s with error %d",
                        pCfg->pEntries[uiIndex].keyName,
                        pCfg->pEntries[uiIndex].groupName,
                        iRc);
        return(-1);
    }
  }
  if(IniFile_Write(pCfg->store.iniFile,pCfg->caPath))
  {
    ERR_PRINT_VARGS("Failed to store inifile \"%s\"",pCfg->caPath);
    return(-1);
  }
  return(0);
}

INLINE_FCT int iAppConfig_AssembleDestFolderPath_m(AppConfig pCfg,
                                                   unsigned int *puiBufSize,
                                                   const char *pcAppName,
                                                   const char *pcFileName,
                                                   const char *pcLocation)
{
  unsigned int uiTmp;
  if(!pcAppName) /* Appname must be specified */
    return(-1);

  if(!pcLocation)
  {
    uiTmp=*puiBufSize;
    if(iAppConfig_GetDefaultConfigPath_m(pCfg->caPath,
                                         &uiTmp))
    {
      ERR_PRINT("iAppConfig_GetDefaultPath_m() failed");
      return(-1);
    }
  }
  else
  {
    if((uiTmp=strlen(pcLocation)+1) > *puiBufSize-2)
      return(-1);
    memcpy(pCfg->caPath,pcLocation,uiTmp);
  }
  /* Check if buffer is big enough to hold the whole path */
  if(uiTmp+strlen(pcAppName)+strlen(pcFileName) > (*puiBufSize)-2)
  {
    ERR_PRINT("Buffer for path too small, would exceed");
    return(-1);
  }
  if(pCfg->caPath[uiTmp-2] != PATH_SEPARATOR) /* Add path separator if needed */
    pCfg->caPath[uiTmp-1]=PATH_SEPARATOR;

  strcpy(&pCfg->caPath[uiTmp],pcAppName);
  uiTmp=strlen(pCfg->caPath);
  if(pCfg->caPath[uiTmp-1] != PATH_SEPARATOR) /* Add path separator if needed */
  {
    pCfg->caPath[uiTmp++]=PATH_SEPARATOR;
    pCfg->caPath[uiTmp]='\0';
  }
  *puiBufSize=uiTmp;
  return(0);
}

INLINE_FCT int iAppConfig_Create_m(AppConfig pCfg)
{
  if(IniFile_New(&pCfg->store.iniFile,INI_OPT_CASE_SENSITIVE))
    return(-1);

  return(0);
}

INLINE_FCT int iAppConfig_GetDefaultConfigPath_m(char *pcPath,
                                                 unsigned int *puiBufSize)
{
  char *pcTmp;
  unsigned int uiTmp=0;
#ifdef _WIN32
  if((!(pcTmp=getenv(ENV_VARNAME_1))) || (pcTmp[0] == '\0'))
    return(-1);

  uiTmp=strlen(pcTmp)+1;
  if(uiTmp+2 > *puiBufSize)
    return(-1);
  memcpy(pcPath,pcTmp,uiTmp);
  *puiBufSize=uiTmp;
#elif __linux__

#ifdef _GNU_SOURCE
  #define GETENV(env) secure_getenv(env)
#else /* Fallback to default getenv function... */
  #define GETENV(env) getenv(env)
#endif

  /* Look for XDG_CONFIG_HOME Environment var first */
  if((pcTmp=GETENV(ENV_VARNAME_1)) && (pcTmp[0] != '\0'))
  {
    uiTmp=strlen(pcTmp)+1;
    if(uiTmp+2 > *puiBufSize)
      return(-1);

    memcpy(pcPath,pcTmp,uiTmp);
    *puiBufSize=uiTmp;
  } /* As 2nd option, look for home, and add .config folder manually */
  else if((pcTmp=GETENV(ENV_VARNAME_2)) && (pcTmp[0] != '\0'))
  {
    uiTmp=strlen(pcTmp)+1;
    if(uiTmp+sizeof(LINUX_CFG_DIR)+2 > *puiBufSize)
      return(-1);
    memcpy(pcPath,pcTmp,uiTmp);
    pcPath[uiTmp-1]=PATH_SEPARATOR;
    memcpy(&pcPath[uiTmp],LINUX_CFG_DIR,sizeof(LINUX_CFG_DIR));
    *puiBufSize=uiTmp+sizeof(LINUX_CFG_DIR);
  }
  else
    return(-1); /* Env not found or empty */
#else
  #error "Unknown platform, can't determine config path"
#endif
  return(0);
}

INLINE_FCT int iAppConfig_FileExists_m(const char *pcPath)
{
  FILE *fp;
  if(!(fp=fopen(pcPath,"r")))
    return(0);
  fclose(fp);
  return(1);
}

INLINE_FCT int iAppConfig_CreateDirIfNotExist_m(const char *pcDir)
{
#ifdef _WIN32
  if(CreateDirectory(pcDir,NULL))
    return(1);
  if(GetLastError() == ERROR_ALREADY_EXISTS)
    return(0);
  return(-1);
#elif __linux__
  errno=0;
  if(mkdir(pcDir,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0)
    return(1);
  if(errno == EEXIST)
    return(0);
  return(-1);
#endif
}