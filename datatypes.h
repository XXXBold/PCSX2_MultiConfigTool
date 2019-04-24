#ifndef DATATYPES_H_INCLUDED
  #define DATATYPES_H_INCLUDED

#include <stdlib.h>

typedef enum
{
  eDataType_Int,
  eDataType_Uint,
  eDataType_Double,
  eDataType_Boolean,
  eDataType_String
}EDataType;

typedef struct TagData_T
{
  /**
   * @see enum EDataType, defines what datatype union data contains.
   */
  EDataType eType;
  /**
   * Size of the current data, <br>
   * If dataType is set to eDataType_String, this is the usable size of the buffer pointed by data.pcVal. <br>
   * For other types use sizeof(type) to initialize.
   */
  size_t dataSize;
  /**
   * Union to store the diffrent datatypes. <br>
   * If String (pcVal) is used, make sure it points to a valid location, <br>
   * that will stay valid while this struct is used.
   */
  union
  {
    int iVal;
    unsigned int uiVal;
    double dVal;
    int bVal;
    char *pcVal;
  }data;
}TagData;

/**
 * Helper Macros to initialise TagData Structures
 */
#define DATA_SET_INT(tagdata,val)    do{(tagdata).data.iVal=val;  (tagdata).eType=eDataType_Int;     (tagdata).dataSize=sizeof(int);          }while(0)
#define DATA_SET_UINT(tagdata,val)   do{(tagdata).data.uiVal=val; (tagdata).eType=eDataType_Uint;    (tagdata).dataSize=sizeof(unsigned int); }while(0)
#define DATA_SET_DOUBLE(tagdata,val) do{(tagdata).data.dVal=val;  (tagdata).eType=eDataType_Double;  (tagdata).dataSize=sizeof(double);       }while(0)
#define DATA_SET_BOOL(tagdata,val)   do{(tagdata).data.bVal=val;  (tagdata).eType=eDataType_Boolean; (tagdata).dataSize=sizeof(int);          }while(0)
#define DATA_SET_STRING(tagdata,buf,bufsize) do{(tagdata).data.pcVal=buf; (tagdata).eType=eDataType_String; (tagdata).dataSize=bufsize;       }while(0)

#define DATA_GET_INT(tagdata)     ((tagdata).data.iVal)
#define DATA_GET_UINT(tagdata)    ((tagdata).data.uiVal)
#define DATA_GET_DOUBLE(tagdata)  ((tagdata).data.dVal)
#define DATA_GET_BOOL(tagdata)    ((tagdata).data.bVal)
#define DATA_GET_STRING(tagdata)  ((tagdata).data.pcVal)

#endif /* DATATYPES_H_INCLUDED */
