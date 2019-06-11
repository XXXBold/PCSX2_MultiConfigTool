#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "appconfig.h"

#define TEST_STRING  "This is a test string!"
#define TEST_STRING2 "This is another test string!"

char caTestString_m[100];

enum
{
  CFG_INDEX_INT=0,
  CFG_INDEX_UINT,
  CFG_INDEX_DOUBLE,
  CFG_INDEX_BOOL,
  CFG_INDEX_STRING,
  CFG_INDEX_G1_INT,
  CFG_INDEX_G1_DOUBLE,
  CFG_INDEX_G2_INT,
  CFG_INDEX_G2_BOOL,
  CFG_INDEX_G3_INT,
  CFG_INDEX_G3_EMPTY,
};

AppConfigEntry taEntrys_m[]={{.keyName="TestInt",     .groupName=NULL},
                             {.keyName="TestUint",    .groupName=NULL},
                             {.keyName="TestDouble",  .groupName=NULL},
                             {.keyName="TestBoolean", .groupName=NULL},
                             {.keyName="TestString",  .groupName=NULL},
                             {.keyName="TestInt",     .groupName="Grp1"},
                             {.keyName="TestDouble",  .groupName="Grp1"},
                             {.keyName="TestInt",     .groupName="Grp2"},
                             {.keyName="TestBoolean", .groupName="Grp2"},
                             {.keyName="TestInt",     .groupName="Grp3"},
                             };
AppConfig pConfig;


int main(void)
{
  int iRc;

  dataType_Set_Int(&taEntrys_m[CFG_INDEX_INT].tagData,0x8FFFFFFF,eRepr_Int_Binary);
  dataType_Set_Uint(&taEntrys_m[CFG_INDEX_UINT].tagData,20202020,eRepr_Int_Default);
  dataType_Set_Double(&taEntrys_m[CFG_INDEX_DOUBLE].tagData,123456.78,eRepr_Double_Default);
  dataType_Set_Boolean(&taEntrys_m[CFG_INDEX_BOOL].tagData,1,eRepr_Boolean_Default);
  dataType_Set_String(&taEntrys_m[CFG_INDEX_STRING].tagData,
                      caTestString_m,
                      sizeof(caTestString_m),
                      TEST_STRING,
                      sizeof(TEST_STRING),
                      eRepr_String_Default);

  dataType_Set_Int(&taEntrys_m[CFG_INDEX_G1_INT].tagData,0x12345,eRepr_Int_Hexadecimal);
  dataType_Set_Double(&taEntrys_m[CFG_INDEX_G1_DOUBLE].tagData,54.321,eRepr_Double_Default);

  dataType_Set_Int(&taEntrys_m[CFG_INDEX_G2_INT].tagData,-12345,eRepr_Int_Decimal);
  dataType_Set_Boolean(&taEntrys_m[CFG_INDEX_G2_BOOL].tagData,1,eRepr_Boolean_Default);

  dataType_Set_Int(&taEntrys_m[CFG_INDEX_G3_INT].tagData,1234567,eRepr_Int_Octal);

  switch((iRc=appConfig_Open(&pConfig,
                             taEntrys_m,
                             sizeof(taEntrys_m)/sizeof(AppConfigEntry),
                             APPCFG_OPT_IGNORE_INVALID_ENTRIES,
                             "TestAPP",
                             NULL,
                             NULL)))
  {
    case APPCFG_ERR_NONE:
      puts("Created new config datastructure");
      break;
    default:
      fprintf(stderr,"appConfig_Load() failed (%d): %s\n",iRc,appConfig_GetErrorString(iRc));
      return(EXIT_FAILURE);
  }
  switch((iRc=appConfig_DataLoad(pConfig)))
  {
    case APPCFG_LOAD_NEW:
    case APPCFG_LOAD_EXISTING:
      puts("Successfull loaded configuration");
      break;
    default:
      fprintf(stderr,"appConfig_Load() failed (%d): %s\n",iRc,appConfig_GetErrorString(iRc));
      return(EXIT_FAILURE);
  }

  appConfig_DumpContents(pConfig,stdout);
  puts("appConfig_Load() done, press enter to save changes...");
  getchar();

  dataType_Update_String(&taEntrys_m[CFG_INDEX_STRING].tagData,TEST_STRING2,sizeof(TEST_STRING2));
  taEntrys_m[CFG_INDEX_STRING].tagData.uiDataSizeUsed=sizeof(TEST_STRING2);
  memcpy(taEntrys_m[CFG_INDEX_STRING].tagData.data.pcVal,
         TEST_STRING2,
         sizeof(TEST_STRING2));

  printf("Saving to path: %s\n",appConfig_GetPath(pConfig));
  if(appConfig_DataSave(pConfig))
  {
    fputs("appConfig_Save() failed\n",stderr);
    return(EXIT_FAILURE);
  }
  else
    printf("Deleted file \"%s\"\n",appConfig_GetPath(pConfig));

  puts("press enter to delete the saved file (check before delete if correct)...");
  getchar();
  if((iRc=appConfig_DataDelete(pConfig)) != APPCFG_ERR_NONE)
  {
    fprintf(stderr,"appConfig_DataDelete() failed (%d): %s\n",iRc,appConfig_GetErrorString(iRc));
  }
  appConfig_Close(pConfig);

  return(EXIT_SUCCESS);
}
