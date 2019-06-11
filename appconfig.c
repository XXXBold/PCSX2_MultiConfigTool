#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "appconfig.h"
#include "inifile.h"
#include "datatypes.h"

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h> /* Used for File System access */
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

#define OPT_CHECK(appcfg,opt) (appcfg->uiOptions & opt)

#define DEFAULT_FILENAME "config.ini"

#ifdef APPCONFIG_PRINT_ERRORS
  #define ERR_PRINT(str)           fputs("AppConfig Error: " str "\n",stderr)
  #define ERR_PRINT_VARGS(str,...) fprintf(stderr,"AppConfig Error: " str "\n",__VA_ARGS__)
#else
  #define ERR_PRINT(str)
  #define ERR_PRINT_VARGS(str,...)
#endif /* APPCONFIG_PRINT_ERRORS */

#define INI_PRINT_ERROR(strfunc,errcode) fprintf(stderr,strfunc "() failed (%d): %s\n",errcode,IniFile_GetErrorText(errcode))
#define APPCFG_PRINT_ERROR(strfunc,errcode) fprintf(stderr,strfunc "() failed (%d): %s\n",errcode,appConfig_GetErrorString(errcode))

enum
{
  MAX_PATHLEN = 260,
};

struct TagAppConfig_T
{
  char caPath[MAX_PATHLEN+1];
  unsigned int uiPathLen;
  unsigned int uiOptions;
  AppConfigEntry *pEntries;
  char *pcEntryData;
  size_t szEntriesCount;
  union StoreMethod_T /* Here add probably more methods for storage, e.g. registry for windows... */
  {
    Inifile iniFile;
  }store;
};

/**
 * Functions related to storage via Inifile (inifile.c/.h)
 */
INLINE_PROT int iAppConfig_Ini_Open_m(AppConfig pCfg,
                                      const char *pcAppName,
                                      const char *pcFileName,
                                      const char *pcLocation);
INLINE_PROT int iAppConfig_Ini_DataLoad_m(AppConfig pCfg);
INLINE_PROT int iAppConfig_Ini_DataSave_m(AppConfig pCfg);
INLINE_PROT int iAppConfig_Ini_DataDelete_m(AppConfig pCfg);
INLINE_PROT void vAppConfig_Ini_Close_m(AppConfig pCfg);

INLINE_PROT int iAppConfig_CheckOptionsValid_m(unsigned int uiOptions);
INLINE_PROT int iAppConfig_AssembleDestFolderPath_m(char *pcPathOut,
                                                    unsigned int *puiBufSize,
                                                    const char *pcAppName,
                                                    const char *pcFileName,
                                                    const char *pcLocation);
INLINE_PROT int iAppConfig_GetDefaultConfigPath_m(char *pcPath,
                                                  unsigned int *uiBufsize);
INLINE_PROT int iAppConfig_FileExists_m(const char *pcPath);
INLINE_PROT int iAppConfig_CreateDirIfNotExist_m(const char *pcDir);

int appConfig_Open(AppConfig *config,
                   AppConfigEntry *entries,
                   size_t entriesCount,
                   unsigned int options,
                   const char *appName,
                   const char *fileName,
                   const char *location)
{
  int iRc;

  if(!iAppConfig_CheckOptionsValid_m(options))
  {
    ERR_PRINT_VARGS("iAppConfig_CheckOptionsValid_m() failed, options not valid (0x%X)",options);
    return(APPCFG_ERR_PARAM);
  }


  if(!((*config)=malloc(sizeof(struct TagAppConfig_T)+entriesCount)))
  {
    ERR_PRINT("malloc() failed");
    return(APPCFG_ERR_INTERNAL);
  }

  if(!fileName)
    fileName=DEFAULT_FILENAME;

  (*config)->uiOptions=options;
  if((iRc=iAppConfig_Ini_Open_m(*config,
                                appName,
                                fileName,
                                location)) != APPCFG_ERR_NONE)
  {
    APPCFG_PRINT_ERROR("iAppConfig_Ini_Open_m",iRc);
    free(*config);
    return(iRc);
  }
  (*config)->pEntries=entries;
  (*config)->szEntriesCount=entriesCount;
  (*config)->pcEntryData=((char*)(*config))+sizeof(struct TagAppConfig_T);

  return(APPCFG_ERR_NONE);
}

int appConfig_DataLoad(AppConfig config)
{
  if(!config)
    return(APPCFG_ERR_PARAM);

  return(iAppConfig_Ini_DataLoad_m(config));
}

int appConfig_DataSave(AppConfig config)
{
  return(iAppConfig_Ini_DataSave_m(config));
}

int appConfig_DataDelete(AppConfig config)
{
  errno=0;
  if(!config)
    return(APPCFG_ERR_PARAM);

  return(iAppConfig_Ini_DataDelete_m(config));
}

const char *appConfig_GetErrorString(int error)
{
  switch(error)
  {
    case APPCFG_ERR_NONE:
      return("No Error");
    case APPCFG_LOAD_EXISTING:
      return("Loaded existing Config");
    case APPCFG_LOAD_NEW:
      return("Created new config");
    case APPCFG_ERR_PARAM:
      return("Invalid Parameter");
    case APPCFG_ERR_DATA_MALFORMED:
      return("Data can't be read, is malformed");
    case APPCFG_ERR_IO:
      return("I/O Error occured while accessing the data storage");
    case APPCFG_ERR_INTERNAL:
      return("Internal Error occured");
    default:
      return("Unknown error code");
  }
}

const char *appConfig_GetPath(const AppConfig config)
{
  return(config->caPath);
}

void appConfig_Close(AppConfig config)
{
  vAppConfig_Ini_Close_m(config);
  free(config);
}

void appConfig_DumpContents(const AppConfig config,
                            FILE *fp)
{
  unsigned int uiIndex;
  unsigned int uiIndexBin;

  for(uiIndex=0;uiIndex < config->szEntriesCount;++uiIndex)
  {
    fprintf(fp,
            "Group: \"%s\", key: \"%s\"",
            config->pEntries[uiIndex].groupName,
            config->pEntries[uiIndex].keyName);
    switch(config->pEntries[uiIndex].tagData.uiType & 0xFF)
    {
      case eDataType_Int:
        fprintf(fp,"(Int)=%d\n",config->pEntries[uiIndex].tagData.data.iVal);
        break;
      case eDataType_Uint:
        fprintf(fp,"(Uint)=%u\n",config->pEntries[uiIndex].tagData.data.uiVal);
        break;
      case eDataType_Double:
        fprintf(fp,"(Double)=%f\n",config->pEntries[uiIndex].tagData.data.dVal);
        break;
      case eDataType_Boolean:
        fprintf(fp,"(Boolean)=%s\n",(config->pEntries[uiIndex].tagData.data.bVal)?"true":"false");
        break;
      case eDataType_String:
        fprintf(fp,"(String)=%s\n",config->pEntries[uiIndex].tagData.data.pcVal);
        break;
      case eDataType_Binary:
        fputs("(Binary)=",fp);
        for(uiIndexBin=0;uiIndexBin < config->pEntries[uiIndex].tagData.uiDataSizeUsed;++uiIndexBin)
          fprintf(fp,"0x%.2X ",config->pEntries[uiIndex].tagData.data.pucVal[uiIndexBin]);
        putc('\n',fp);
        break;
      default:
        fprintf(fp,"Unknown type (0x%X)\n",config->pEntries[uiIndex].tagData.uiType);
        break;
    }
  }
}

INLINE_FCT int iAppConfig_Ini_Open_m(AppConfig pCfg,
                                     const char *pcAppName,
                                     const char *pcFileName,
                                     const char *pcLocation)
{
  unsigned int uiTmp;
  int iRc;

  uiTmp=MAX_PATHLEN;
  if((iRc=iAppConfig_AssembleDestFolderPath_m(pCfg->caPath,
                                              &uiTmp,
                                              pcAppName,
                                              pcFileName,
                                              pcLocation)) != APPCFG_ERR_NONE)
  {
    APPCFG_PRINT_ERROR("iAppConfig_AssembleDestFolderPath_m",iRc);
    return(iRc);
  }
  if(iAppConfig_CreateDirIfNotExist_m(pCfg->caPath) < 0)
  {
    ERR_PRINT("iAppConfig_CreateDirIfNotExist_m() failed");
    return(APPCFG_ERR_IO);
  }
  /* bufferlength is checked in AssembleDestFolderPath, so strcpy is fine */
  strcpy(&pCfg->caPath[uiTmp-1],pcFileName);

  uiTmp=INI_OPT_CASE_SENSITIVE;
  if(OPT_CHECK(pCfg,APPCFG_OPT_IGNORE_INVALID_ENTRIES))
    uiTmp|=INI_OPT_IGNORE_MALFORMED_LINES;

  if((iRc=IniFile_New(&pCfg->store.iniFile,
                      pCfg->caPath,
                      uiTmp)) != INI_ERR_NONE)
  {
    INI_PRINT_ERROR("IniFile_New",iRc);
    return(APPCFG_ERR_INTERNAL);
  }
  return(APPCFG_ERR_NONE);
}

INLINE_FCT int iAppConfig_Ini_DataLoad_m(AppConfig pCfg)
{
  const char *pcSection;
  const char *pcKey;
  unsigned int uiIndex;
  unsigned int uiEntriesReadCount;
  int iRc;

  if(!iAppConfig_FileExists_m(pCfg->caPath))
    return(APPCFG_LOAD_NEW);

  memset(pCfg->pcEntryData,0,pCfg->szEntriesCount);
  if((iRc=IniFile_Read(pCfg->store.iniFile)) != INI_ERR_NONE)
  {
    INI_PRINT_ERROR("IniFile_Read",iRc);
    switch(iRc)
    {
      case INI_ERR_NONE: /* No error */
        iRc=APPCFG_LOAD_EXISTING;
        break;
      case INI_ERR_MALFORMED:
        iRc=APPCFG_ERR_DATA_MALFORMED;
        break;
      case INI_ERR_IO:
        iRc=APPCFG_ERR_IO;
        break;
      case INI_ERR_INTERNAL:
      default:
        iRc=APPCFG_ERR_INTERNAL;
        break;
    }
    return(iRc);
  }
  uiEntriesReadCount=0;
  for(pcSection=IniFile_Iterator_SetSectionIndex(pCfg->store.iniFile,ITERATOR_FIRST);
      pcSection;
      pcSection=IniFile_Iterator_NextSection(pCfg->store.iniFile))
  {
    for(uiIndex=0;uiIndex < pCfg->szEntriesCount;++uiIndex)
    {
      if(pCfg->pcEntryData[uiIndex]) /* Skip already read entries */
        continue;

      if((pCfg->pEntries[uiIndex].groupName) &&
         (IniFile_StringCompare(pCfg->store.iniFile,pcSection,pCfg->pEntries[uiIndex].groupName) != 0))
         continue;

      ++uiEntriesReadCount;
      pCfg->pcEntryData[uiIndex]=1;
      for(pcKey=IniFile_Iterator_SetKeyIndex(pCfg->store.iniFile,ITERATOR_FIRST);
          pcKey;
          pcKey=IniFile_Iterator_NextKey(pCfg->store.iniFile))
      {
        if(IniFile_StringCompare(pCfg->store.iniFile,pcKey,pCfg->pEntries[uiIndex].keyName) == 0)
        {
          if(((iRc=IniFile_Iterator_KeyGetValue(pCfg->store.iniFile,&pCfg->pEntries[uiIndex].tagData)) != INI_ERR_NONE) &&
             (iRc != INI_ERR_DATA_EMPTY))
          {
            ERR_PRINT_VARGS("IniFile_Iterator_KeyGetValue() failed for Group \"%s\" Key \"%s\" (%d): %s%s",
                            pcSection,
                            pcKey,
                            iRc,
                            IniFile_GetErrorText(iRc),
                            (OPT_CHECK(pCfg,APPCFG_OPT_IGNORE_INVALID_ENTRIES))?"(Ignore error, keep default value)":"");

            if(OPT_CHECK(pCfg,APPCFG_OPT_IGNORE_INVALID_ENTRIES)) /* Ignore error */
              break;
            switch(iRc)
            {
              case INI_ERR_PARAM:
                return(APPCFG_ERR_PARAM);
              case INI_ERR_CONVERSION: /* Conversion can't fail, otherways data have been corrupted */
                return(APPCFG_ERR_DATA_MALFORMED);
              default:
                return(APPCFG_ERR_INTERNAL);
            }
          }
          break;
        }
      }
    }
    if(uiEntriesReadCount == pCfg->szEntriesCount) /* Check if all entries have been read already */
      break;
  }
  /* Got all needed entries, clear inifile data */
  IniFile_Clean(pCfg->store.iniFile);
  return(APPCFG_LOAD_EXISTING);
}

INLINE_FCT int iAppConfig_Ini_DataSave_m(AppConfig pCfg)
{
  unsigned int uiIndex;
  int iRc;

  for(uiIndex=0;uiIndex < pCfg->szEntriesCount;++uiIndex)
  {
    if(pCfg->pEntries[uiIndex].groupName)
    {
      if((iRc=IniFile_Iterator_CreateSection(pCfg->store.iniFile,pCfg->pEntries[uiIndex].groupName)) != INI_ERR_NONE)
      {
        INI_PRINT_ERROR("IniFile_Iterator_CreateSection",iRc);
        switch(iRc)
        {
          case INI_ERR_PARAM:
            return(APPCFG_ERR_PARAM);
          case INI_ERR_INTERNAL:
          default:
            return(APPCFG_ERR_INTERNAL);
        }
      }
    }
    else
    {
      IniFile_Iterator_SetSectionIndex(pCfg->store.iniFile,ITERATOR_FIRST);
    }

    if((iRc=IniFile_Iterator_CreateKey(pCfg->store.iniFile,pCfg->pEntries[uiIndex].keyName)) != INI_ERR_NONE)
    {
      INI_PRINT_ERROR("IniFile_Iterator_CreateKey",iRc);
      switch(iRc)
      {
        case INI_ERR_PARAM:
          return(APPCFG_ERR_PARAM);
        case INI_ERR_INTERNAL:
        default:
          return(APPCFG_ERR_INTERNAL);
      }
    }
    if((iRc=IniFile_Iterator_KeySetValue(pCfg->store.iniFile,&pCfg->pEntries[uiIndex].tagData)) != INI_ERR_NONE)
    {
      INI_PRINT_ERROR("IniFile_Iterator_KeySetValue",iRc);
      switch(iRc)
      {
        case INI_ERR_PARAM:
          return(APPCFG_ERR_PARAM);
        case INI_ERR_INTERNAL:
        default:
          return(APPCFG_ERR_INTERNAL);
      }
    }
  }
  if((iRc=IniFile_Write(pCfg->store.iniFile)) != INI_ERR_NONE)
  {
    INI_PRINT_ERROR("IniFile_Write",iRc);
    switch(iRc)
    {
      case INI_ERR_PARAM:
        return(APPCFG_ERR_PARAM);
      case INI_ERR_IO:
        return(APPCFG_ERR_IO);
      case INI_ERR_INTERNAL:
      default:
        return(APPCFG_ERR_INTERNAL);
    }
  }
  return(APPCFG_ERR_NONE);
}

INLINE_FCT int iAppConfig_Ini_DataDelete_m(AppConfig pCfg)
{
  int iRc;
  if((iRc=IniFile_Remove(pCfg->store.iniFile)) != INI_ERR_NONE)
  {
    INI_PRINT_ERROR("IniFile_Remove",iRc);
    return(APPCFG_ERR_IO);
  }
  return(APPCFG_ERR_NONE);
}

INLINE_FCT void vAppConfig_Ini_Close_m(AppConfig pCfg)
{
  IniFile_Dispose(pCfg->store.iniFile);
}

INLINE_FCT int iAppConfig_CheckOptionsValid_m(unsigned int uiOptions)
{
  /* clear all valid option flags, value should be =0 after that */
  uiOptions&=~(APPCFG_OPT_IGNORE_INVALID_ENTRIES);
  return((uiOptions)?0:1);
}

INLINE_FCT int iAppConfig_AssembleDestFolderPath_m(char *pcOutPath,
                                                   unsigned int *puiBufSize,
                                                   const char *pcAppName,
                                                   const char *pcFileName,
                                                   const char *pcLocation)
{
  unsigned int uiTmp;
  if(!pcAppName) /* Appname must be specified */
    return(APPCFG_ERR_PARAM);

  if(!pcLocation)
  {
    uiTmp=*puiBufSize;
    if(iAppConfig_GetDefaultConfigPath_m(pcOutPath,
                                         &uiTmp))
    {
      ERR_PRINT("iAppConfig_GetDefaultPath_m() failed");
      return(APPCFG_ERR_INTERNAL);
    }
  }
  else
  {
    if((uiTmp=strlen(pcLocation)+1) > *puiBufSize-2)
      return(APPCFG_ERR_PARAM);
    memcpy(pcOutPath,pcLocation,uiTmp);
  }
  /* Check if buffer is big enough to hold the whole path */
  if(uiTmp+strlen(pcAppName)+strlen(pcFileName) > (*puiBufSize)-2)
  {
    ERR_PRINT("Buffer for path too small, would exceed");
    return(APPCFG_ERR_PARAM);
  }
  if(pcOutPath[uiTmp-2] != PATH_SEPARATOR) /* Add path separator if needed */
    pcOutPath[uiTmp-1]=PATH_SEPARATOR;

  strcpy(&pcOutPath[uiTmp],pcAppName);
  uiTmp=strlen(pcOutPath);
  if(pcOutPath[uiTmp-1] != PATH_SEPARATOR) /* Add path separator if needed */
  {
    pcOutPath[uiTmp++]=PATH_SEPARATOR;
    pcOutPath[uiTmp]='\0';
  }
  *puiBufSize=uiTmp+1;
  return(APPCFG_ERR_NONE);
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
