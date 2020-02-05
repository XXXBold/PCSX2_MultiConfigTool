#ifndef DATATYPES_H_INCLUDED
  #define DATATYPES_H_INCLUDED

#if defined (__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) /* >= C99 */
  #define INLINE static inline
#elif defined (__GNUC_GNU_INLINE__)
  #define INLINE static inline
#else /* Fallback, No inline available from C Standard */
  #define INLINE  static
#endif /* __STDC_VERSION__ >= C99 */

typedef enum
{
  eDataType_Int      =0x1,
  eDataType_Uint     =0x2,
  eDataType_Double   =0x4,
  eDataType_Boolean  =0x8,
  eDataType_String   =0x10,
  eDataType_Binary   =0x20,

  /**
   * Data Representations: How the data should be represented
   */
  eRepr_Int_Decimal     =0x100,
  eRepr_Int_Binary      =0x200,
  eRepr_Int_Octal       =0x400,
  eRepr_Int_Hexadecimal =0x800,
  eRepr_Int_Default     =eRepr_Int_Decimal,

  /* Other representations reserved for future use */
  eRepr_Double_Default  = 0x0,

  eRepr_Boolean_Default = 0x0,

  eRepr_String_Default  = 0x0,

  eRepr_Binary_Default  = 0x0,
}EDataType;

typedef struct TagData_T
{
  /**
   * @see enum EDataType, defines what datatype union data contains.
   * eDataType_ Must be selected, for int/unsigned int eDataRepresentation_ should be defined also (OR-ed together).
   */
  unsigned int uiType;
  /**
   * Size of the current data, <br>
   * If dataType is set to eDataType_String, this is the usable size of the buffer pointed by data.pcVal. <br>
   * For other types use sizeof(type) to initialize.
   */
  unsigned int uiDataSizeMax;
  unsigned int uiDataSizeUsed;
  /**
   * Union to store the diffrent datatypes. <br>
   * If String (pcVal) or Binary (pucVal) is used, make sure it points to a valid location, <br>
   * that will stay valid while this struct is used.
   */
  union
  {
    int iVal;
    unsigned int uiVal;
    double dVal;
    int bVal;
    char *pcVal;
    unsigned char *pucVal;
  }data;
}TagData;

INLINE void dataType_Set_Int(TagData *ptagData,
                             int iVal,
                             EDataType eDataRepresentation)
{
  ptagData->data.iVal=iVal;
  if(!eDataRepresentation) /* If Data representation was not set, set to default */
    eDataRepresentation=eRepr_Int_Default;
  ptagData->uiType=(eDataType_Int | eDataRepresentation);
  ptagData->uiDataSizeMax=sizeof(int);
  ptagData->uiDataSizeUsed=sizeof(int);
}

INLINE void dataType_Set_Uint(TagData *ptagData,
                              unsigned int uiVal,
                              EDataType eDataRepresentation)
{
  ptagData->data.uiVal=uiVal;
  if(!eDataRepresentation) /* If Data representation was not set, set to default */
    eDataRepresentation=eRepr_Int_Default;
  ptagData->uiType=(eDataType_Uint | eDataRepresentation);
  ptagData->uiDataSizeMax=sizeof(unsigned int);
  ptagData->uiDataSizeUsed=sizeof(unsigned int);
}

INLINE void dataType_Set_Double(TagData *ptagData,
                                double dVal,
                                EDataType eDataRepresentation)
{
  ptagData->data.dVal=dVal;
  ptagData->uiType=(eDataType_Double | eDataRepresentation);
  ptagData->uiDataSizeMax=sizeof(double);
  ptagData->uiDataSizeUsed=sizeof(double);
}

INLINE void dataType_Set_Boolean(TagData *ptagData,
                                 int bVal,
                                 EDataType eDataRepresentation)
{
  ptagData->data.bVal=(bVal)?1:0;
  ptagData->uiType=(eDataType_Boolean | eDataRepresentation);
  ptagData->uiDataSizeMax=sizeof(int);
  ptagData->uiDataSizeUsed=sizeof(int);
}

INLINE int dataType_Set_String(TagData *ptagData,
                               char *pcBuffer,
                               unsigned int uiBufSize,
                               const char *pcVal,
                               unsigned int uiDataLength,
                               EDataType eDataRepresentation)
{
  if(uiBufSize < uiDataLength)
    return(-1);
  ptagData->uiDataSizeMax=uiBufSize;
  ptagData->data.pcVal=pcBuffer;
  ptagData->uiType=(eDataType_String | eDataRepresentation);
  if(pcVal)
  {
    memcpy(ptagData->data.pcVal,
           pcVal,
           uiDataLength);
    ptagData->uiDataSizeUsed=uiDataLength;
  }
  else
    ptagData->uiDataSizeUsed=0;
  return(0);
}

INLINE int dataType_Set_Binary(TagData *ptagData,
                               unsigned char *pucBuffer,
                               unsigned int uiBufSize,
                               const unsigned char *pucVal,
                               unsigned int uiDataLength,
                               EDataType eDataRepresentation)
{
  if(uiBufSize < uiDataLength)
    return(-1);
  ptagData->uiDataSizeMax=uiBufSize;
  ptagData->data.pucVal=pucBuffer;
  ptagData->uiType=(eDataType_Binary | eDataRepresentation);
  if(pucVal)
  {
    memcpy(ptagData->data.pucVal,
           pucVal,
           uiDataLength);
    ptagData->uiDataSizeUsed=uiDataLength;
  }
  else
    ptagData->uiDataSizeUsed=0;
  return(0);
}

INLINE void dataType_Update_Int(TagData *ptagData,
                                int iVal)
{
  ptagData->data.iVal=iVal;
}

INLINE void dataType_Update_Uint(TagData *ptagData,
                                 unsigned int uiVal)
{
  ptagData->data.uiVal=uiVal;
}

INLINE void dataType_Update_Double(TagData *ptagData,
                                   double dVal)
{
  ptagData->data.dVal=dVal;
}

INLINE void dataType_Update_Boolean(TagData *ptagData,
                                    int bVal)
{
  ptagData->data.bVal=bVal;
}

INLINE int dataType_Update_String(TagData *ptagData,
                                  const char *pcString,
                                  unsigned int uiSize)
{
  if(uiSize > ptagData->uiDataSizeMax)
    return(-1);

  memcpy(ptagData->data.pcVal,pcString,uiSize);
  ptagData->uiDataSizeUsed=uiSize;
  return(0);
}

INLINE int dataType_Update_Binary(TagData *ptagData,
                                  const unsigned char *pucData,
                                  unsigned int uiSize)
{
  if(uiSize > ptagData->uiDataSizeMax)
    return(-1);

  memcpy(ptagData->data.pucVal,pucData,uiSize);
  ptagData->uiDataSizeUsed=uiSize;
  return(0);
}

#define DATA_GET_INT(tagdata)     ((tagdata).data.iVal)
#define DATA_GET_UINT(tagdata)    ((tagdata).data.uiVal)
#define DATA_GET_DOUBLE(tagdata)  ((tagdata).data.dVal)
#define DATA_GET_BOOL(tagdata)    ((tagdata).data.bVal)
#define DATA_GET_STRING(tagdata)  ((tagdata).data.pcVal)
#define DATA_GET_BINARY(tagdata)  ((tagdata).data.pucVal)

#endif /* DATATYPES_H_INCLUDED */
