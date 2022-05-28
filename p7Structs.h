//                                                                             /
// 2012-2020 (c) Baical                                                        /
//                                                                             /
// This library is free software; you can redistribute it and/or               /
// modify it under the terms of the GNU Lesser General Public                  /
// License as published by the Free Software Foundation; either                /
// version 3.0 of the License, or (at your option) any later version.          /
//                                                                             /
// This library is distributed in the hope that it will be useful,             /
// but WITHOUT ANY WARRANTY; without even the implied warranty of              /
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU           /
// Lesser General Public License for more details.                             /
//                                                                             /
// You should have received a copy of the GNU Lesser General Public            /
// License along with this library.                                            /
//                                                                             /
////////////////////////////////////////////////////////////////////////////////

#ifndef P7_STRUCTS_H
#define P7_STRUCTS_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include "GTypes.h"

#ifdef _MSC_VER
    #define PRAGMA_PACK_ENTER(x)  __pragma(pack(push, x))
    #define PRAGMA_PACK_EXIT()   __pragma(pack(pop))
    #define ATTR_PACK(x)
    #define ATTR_ALIGN(x)
    #define UNUSED_FUNC
#else
    #define PRAGMA_PACK_ENTER(x)
    #define PRAGMA_PACK_EXIT(x)
    #define ATTR_PACK(x) __attribute__ ((aligned(x), packed))
    #define ATTR_ALIGN(x) __attribute__ ((aligned(x)))
    #define UNUSED_FUNC __attribute__ ((unused))
#endif

#define P7_DAMP_FILE_MARKER_V1                           (0x45D2AC71ECF32CA6ULL)
#define P7_DAMP_FILE_HOST_LENGTH                                           (256)
#define P7_DAMP_FILE_PROCESS_LENGTH                                        (256)

//Define the max channels count, and max packet size for IP7_Client::Sent(...)
#define USER_PACKET_CHANNEL_ID_BITS_COUNT                                    (5)
#define USER_PACKET_SIZE_BITS_COUNT       (32-USER_PACKET_CHANNEL_ID_BITS_COUNT)
#define USER_PACKET_MAX_SIZE                  (1 << USER_PACKET_SIZE_BITS_COUNT)
#define USER_PACKET_CHANNEL_ID_MAX_SIZE (1 << USER_PACKET_CHANNEL_ID_BITS_COUNT)

#define P7_EXTENSION_TYPE_BITS_COUNT                                         (5)
#define P7_EXTENSION_SUB_TYPE_BITS_COUNT                                     (5)
#define P7_EXTENSION_PACKET_SIZE_BITS_COUNT                                 (22)

#define P7_EXTENSION_MAX_TYPES               (1 << P7_EXTENSION_TYPE_BITS_COUNT)

// P7Trace
#define FILE_HEADER_MARKER                               (0x2C783F01FE00FFC0ull)
#define FILE_HEADER_VERSION                              (0x0005)
// P7Trace

#define P7TRACE_NAME_LENGTH                                                 (64)
#define P7TRACE_THREAD_NAME_LENGTH                                          (48)
#define P7TRACE_MODULE_NAME_LENGTH                                          (54)

enum eResult
{
    /// <summary> Success result code </summary>
    eOk = 0,

    /// <summary> general error code </summary>
    eError,
    /// <summary> arguments error </summary>
    eErrorWrongInput,
    /// <summary> no available memory </summary>
    eErrorNoBuffer,
    /// <summary> internal error </summary>
    eErrorInternal,
    /// <summary> function isn't supported </summary>
    eErrorNotSupported,
    /// <summary> function isn't implemented </summary>
    eErrorNotImplemented,
    /// <summary> access to function of file is blocked by the system </summary>
    eErrorBlocked,
    /// <summary> functionality is not active for the time being </summary>
    eErrorNotActive,
    /// <summary> not appropriate plugin for the stream </summary>
    eErrorMissmatch,
    /// <summary> stream is closed </summary>
    eErrorClosed,
    /// <summary> timeout </summary>
    eErrorTimeout,
    /// <summary> can't write to file </summary>
    eErrorFileWrite
};

enum eTrace_Arg_Type
{
    P7TRACE_ARG_TYPE_UNK    = 0x00,
    P7TRACE_ARG_TYPE_CHAR   = 0x01,
    P7TRACE_ARG_TYPE_INT8   = 0x01,
    P7TRACE_ARG_TYPE_CHAR16  ,//(0x02)
    P7TRACE_ARG_TYPE_INT16   ,//(0x03)
    P7TRACE_ARG_TYPE_INT32   ,//(0x04)
    P7TRACE_ARG_TYPE_INT64   ,//(0x05)
    P7TRACE_ARG_TYPE_DOUBLE  ,//(0x06)
    P7TRACE_ARG_TYPE_PVOID   ,//(0x07)
    P7TRACE_ARG_TYPE_USTR16  ,//(0x08) //unicode - UTF16 string
    P7TRACE_ARG_TYPE_STRA    ,//(0x09) //ASCII string
    P7TRACE_ARG_TYPE_USTR8   ,//(0x0A) //unicode - UTF8 string
    P7TRACE_ARG_TYPE_USTR32  ,//(0x0B) //unicode - UTF32 string
    P7TRACE_ARG_TYPE_CHAR32  ,//(0x0C)
    P7TRACE_ARG_TYPE_INTMAX  ,//(0x0D)

    P7TRACE_ARGS_COUNT
};


struct sP7Ext_Header
{
   uint32_t dwType   :P7_EXTENSION_TYPE_BITS_COUNT;     //eP7User_Type
   uint32_t dwSubType:P7_EXTENSION_SUB_TYPE_BITS_COUNT;
   //max 4 mb, value should include size of this header
   uint32_t dwSize   :P7_EXTENSION_PACKET_SIZE_BITS_COUNT;
   //At the end of structure we put serialized data
} ATTR_PACK(4);

struct sP7Ext_Raw //mapping of sP7Ext_Header
{
   uint32_t dwBits;
} ATTR_PACK(4);

struct sP7Trace_Data
{
    union
    {
        sP7Ext_Header sCommon;
        sP7Ext_Raw    sCommonRaw;
    };
    uint16_t       wID;          //trace ID
    uint8_t        bLevel;       //eP7Trace_Level
    uint8_t        bProcessor;   //Processor number
    uint32_t       dwThreadID;   //Thread ID
    uint32_t       dwSequence;   //sequence number
    uint64_t       qwTimer;      //High resolution timer value
    //At the end of structure we put serialized data:
    // - trace variable arguments values
    // - extensions [data X bits][type 8 bits], [data X bits][type 8 bits], ... [count 8 bits]
} ATTR_PACK(2);

//P7 damp file header
struct sP7File_Header
{
    uint64_t qwMarker;
    uint32_t dwProcess_ID;
    uint32_t dwProcess_Start_Time_Hi;
    uint32_t dwProcess_Start_Time_Lo;
    char16_t  pProcess_Name[P7_DAMP_FILE_PROCESS_LENGTH];
    char16_t  pHost_Name[P7_DAMP_FILE_HOST_LENGTH];
} ATTR_PACK(2);

enum eP7User_Type
{
    EP7USER_TYPE_TRACE          =  0,
    EP7USER_TYPE_TELEMETRY_V1       ,
    EP7USER_TYPE_TELEMETRY_V2       ,

    EP7USER_TYPE_MAX            = P7_EXTENSION_MAX_TYPES
};

enum eP7Trace_Type
{
    EP7TRACE_TYPE_INFO          =  0, //Client->Server
    EP7TRACE_TYPE_DESC              , //Client->Server
    EP7TRACE_TYPE_DATA              , //Client->Server
    EP7TRACE_TYPE_VERB              , //Client->Server->Client
    EP7TRACE_TYPE_CLOSE             , //Client->Server
    EP7TRACE_TYPE_THREAD_START      , //Client->Server
    EP7TRACE_TYPE_THREAD_STOP       , //Client->Server
    EP7TRACE_TYPE_MODULE            , //Client->Server
    EP7TRACE_TYPE_DELETE            , //Server->Client
    EP7TRACE_TYPE_UTC_OFFS          , //Client->Server

    EP7TRACE_TYPE_MAX           = 32
};

struct sH_User_Data //user data header, map for sH_User_Raw
{
    uint32_t dwSize       :USER_PACKET_SIZE_BITS_COUNT;       //<< 28 bits for Size
    uint32_t dwChannel_ID :USER_PACKET_CHANNEL_ID_BITS_COUNT; //<< 5  bits for ChannelId
} ATTR_PACK(2);

enum eTraceLevel
{
    /// <summary> Trace </summary>
    eTraceLevelTrace = 0,
    /// <summary> Debug </summary>
    eTraceLevelDebug ,
    /// <summary> Info </summary>
    eTraceLevelInfo ,
    /// <summary> Warning </summary>
    eTraceLevelWarning ,
    /// <summary> Error </summary>
    eTraceLevelError ,
    /// <summary> Critical </summary>
    eTraceLevelCritical ,

    /// <summary> Level max count </summary>
    eTraceLevelsCount
};

enum eP7Trace_Level
{
    EP7TRACE_LEVEL_TRACE        = 0,
    EP7TRACE_LEVEL_DEBUG           ,
    EP7TRACE_LEVEL_INFO            ,
    EP7TRACE_LEVEL_WARNING         ,
    EP7TRACE_LEVEL_ERROR           ,
    EP7TRACE_LEVEL_CRITICAL        ,

    EP7TRACE_LEVEL_COUNT
};

PRAGMA_PACK_ENTER(2)

struct sP7Trace_Info
{
    union
    {
        sP7Ext_Header sCommon;
        sP7Ext_Raw    sCommonRaw;
    };
    //Contains a 64-bit value representing the number of 100-nanosecond intervals
    //since January 1, 1601 (UTC). In windows we use FILETIME structure for
    //representing
    uint32_t       dwTime_Hi;
    uint32_t       dwTime_Lo;
    //Hi resolution timer value, we get this value when we retrieve current time.
    //using difference between this value and timer value for every trace we can
    //calculate time of the trace event with hi resolution
    uint64_t       qwTimer_Value;
    //timer's count heartbeats in second
    uint64_t       qwTimer_Frequency;
    uint64_t       qwFlags;
    char16_t        pName[P7TRACE_NAME_LENGTH];
} ATTR_PACK(2);


//this structure describe each argument inside variable arguments list
//all arguments are serialized data block
struct sP7Trace_Arg
{
    //argument's type - one of P7TRACE_ARG_TYPE_XXX
    uint8_t bType;
    //Size - how many bytes is used by argument inside block, this value is not
    //       directly depend on type, usually it depend on processor architecture
    //       for example "char" on WIN32 this is 4 bytes, but for EventTrace
    //       engine this is only 1 byte.
    //       N.B.: All strings has 0 size.
    uint8_t bSize;
} ATTR_PACK(2);

struct sP7Trace_Format
{
    union
    {
        sP7Ext_Header sCommon;
        sP7Ext_Raw    sCommonRaw;
    };
    uint16_t       wID;
    uint16_t       wLine;
    uint16_t       wModuleID;  //Module ID, who send trace
    uint16_t       wArgs_Len;  //arguments count
    //At the end of structure we put serialized data:
    //sP7Trace_Arg [dwArgs_Len]   - array of arguments
    //wchar_t      Format[]       - null terminated string
    //char         FileName[]     - null terminated string
    //char         FunctionName[] - null terminated string
} ATTR_PACK(2);

//Thread start info
struct sP7Trace_Thread_Start
{
    union
    {
        sP7Ext_Header sCommon;
        sP7Ext_Raw    sCommonRaw;
    };
    uint32_t       dwThreadID;                        //Thread ID
    uint64_t       qwTimer;                           //High resolution timer value
    int8_t         pName[P7TRACE_THREAD_NAME_LENGTH]; //Thread name (UTF-8)
} ATTR_PACK(2);

//Module info
struct sP7Trace_Module
{
    union
    {
        sP7Ext_Header sCommon;
        sP7Ext_Raw    sCommonRaw;
    };
    uint16_t        wModuleID;
    eP7Trace_Level eVerbosity;
    int8_t           pName[P7TRACE_MODULE_NAME_LENGTH]; //name (UTF-8)
} ATTR_PACK(2);

PRAGMA_PACK_EXIT()

static uint64_t ntohqw(uint64_t i_qwX)
{
#if defined(_WIN32) || defined(_WIN64)
    return _byteswap_uint64(i_qwX);
#else
    i_qwX = (i_qwX & 0x00000000FFFFFFFFull) << 32 | (i_qwX & 0xFFFFFFFF00000000ull) >> 32;
    i_qwX = (i_qwX & 0x0000FFFF0000FFFFull) << 16 | (i_qwX & 0xFFFF0000FFFF0000ull) >> 16;
    return (i_qwX & 0x00FF00FF00FF00FFull) << 8  | (i_qwX & 0xFF00FF00FF00FF00ull) >> 8;
#endif
}

//Get_UTF16_Length
static UNUSED_FUNC size_t Get_UTF16_Length(const uint16_t *i_pText)
{
    size_t  l_dwLength = 0;
    uint16_t  l_wCh      = 0;

    if (NULL == i_pText)
    {
        return 0;
    }

    while ( 0 != (*i_pText))
    {
        l_wCh = *i_pText;

        if (    (l_wCh >= 0xD800ul) //processing surrogate pairs
             && (l_wCh <= 0xDFFFul)
           )
        {
            i_pText += 2;
        }
        else
        {
            i_pText += 1;
        }

        l_dwLength ++;
    }

    return l_dwLength;
}//Get_UTF16_Length

//PSPrint
static __attribute__ ((unused)) tINT32 PSPrint(tXCHAR       *o_pBuffer,
                                               size_t        i_szBuffer,
                                               const tXCHAR *i_pFormat,
                                               ...
                                              )
{
    va_list l_pVA;
    int     l_iReturn = 0;

    va_start(l_pVA, i_pFormat);

    l_iReturn = vsnprintf((char*)o_pBuffer,
                          i_szBuffer,
                          (char*)i_pFormat,
                          l_pVA);

    va_end(l_pVA);

    return (tINT32)l_iReturn;
}//PSPrint

//PStrNCmp
static __attribute__ ((unused)) tINT32 PStrNCmp(const tXCHAR *i_pS1,
                                                const tXCHAR *i_pS2,
                                                size_t        i_szLen
                                               )
{
    return strncmp(i_pS1, i_pS2, i_szLen);
}//PStrNCmp


////////////////////////////////////////////////////////////////////////////////
//PStrNCmp
static __attribute__ ((unused)) tINT32 PStrNiCmp(const tXCHAR *i_pS1,
                                                 const tXCHAR *i_pS2,
                                                 size_t        i_szLen
                                                )
{
    return strncasecmp(i_pS1, i_pS2, i_szLen);
}//PStrNCmp


////////////////////////////////////////////////////////////////////////////////
//PStrNCmp
static __attribute__ ((unused)) tINT32 PStrICmp(const tXCHAR *i_pS1,
                                                const tXCHAR *i_pS2
                                               )
{
    return strcasecmp(i_pS1, i_pS2);
}//PStrNCmp

////////////////////////////////////////////////////////////////////////////////
//Convert_UTF16_To_UTF8 (LE)
//Return - count of the used bytes, except trailing 0, if destination buffer
//size is not enough to store whole source string, result will be truncated
static UNUSED_FUNC tINT32 Convert_UTF16_To_UTF8(const tWCHAR *i_pSrc,
                                                tACHAR       *o_pDst,
                                                tUINT32       i_dwDst_Len
                                               )
{
    tINT32  l_iLength = i_dwDst_Len;
    tUINT32 l_dwCh    = 0;

    if (    (NULL == i_pSrc)
         || (NULL == o_pDst)
         || (0    >= i_dwDst_Len)
       )
    {
        return -1;
    }

    while (    ( 0ul != (*i_pSrc))
            && (2 <= l_iLength)
          )
    {
        l_dwCh = (tUINT16)(*i_pSrc);

        if (    (l_dwCh >= 0xD800ul) //processing surrogate pairs
             && (l_dwCh <= 0xDFFFul)
           )
        {
            tUINT32 l_dwTrailing = (tUINT16)*(++i_pSrc);
            if (    (0xDC00ul <= l_dwTrailing)
                 && (0xDFFFul >= l_dwTrailing)
               )
            {
                l_dwCh = 0x10000ul + (((l_dwCh & 0x3FFul) << 10) | (l_dwTrailing & 0x3FFul));
            }
            else //unexpected
            {
                l_dwCh = '?';
            }
        }

        if (0x80 > l_dwCh)
        {
            *o_pDst = (tACHAR)(l_dwCh & 0x7Ful);
            o_pDst ++;
            l_iLength--;
        }
        else if (0x800ul > l_dwCh)
        {
            if (3 <= l_iLength)
            {
                *o_pDst = (tACHAR)(0xC0ul | ((l_dwCh >> 6) & 0x1Ful));  o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >> 0) & 0x3Ful));  o_pDst ++;
                l_iLength -= 2;
            }
            else
            {
                break;
            }
        }
        else if (0x10000ul > l_dwCh)
        {
            if (4 <= l_iLength)
            {
                *o_pDst = (tACHAR)(0xE0ul | ((l_dwCh >> 12) & 0x0Ful));  o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >>  6) & 0x3Ful));  o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >>  0) & 0x3Ful));  o_pDst ++;
                l_iLength -= 3;
            }
            else
            {
                break;
            }
        }
        else if (0x200000ul  > l_dwCh)
        {
            if (5 <= l_iLength)
            {
                *o_pDst = (tACHAR)(0xF0ul | ((l_dwCh >> 18) & 0x07ul)); o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >> 12) & 0x3Ful)); o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >>  6) & 0x3Ful)); o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >>  0) & 0x3Ful)); o_pDst ++;
                l_iLength -= 4;
            }
            else
            {
                break;
            }
        }
        else
        {
            *o_pDst = '?';
            o_pDst ++;
            l_iLength -= 1;
        }

        if (*i_pSrc)
        {
            ++i_pSrc;
        }
        else
        {
            break;
        }
    }

    *o_pDst = 0;

    return i_dwDst_Len - l_iLength;
}//Convert_UTF8_To_UTF16

////////////////////////////////////////////////////////////////////////////////
//Convert_UTF32_To_UTF8 (LE)
//Return - count of the used bytes, except trailing 0, if destination buffer
//size is not enough to store whole source string, result will be truncated
static UNUSED_FUNC tINT32 Convert_UTF32_To_UTF8(const tUINT32 *i_pSrc,
                                                tACHAR        *o_pDst,
                                                tUINT32        i_dwDst_Len
                                               )
{
    tINT32  l_iLength = i_dwDst_Len;
    tUINT32 l_dwCh    = 0;

    if (    (NULL == i_pSrc)
         || (NULL == o_pDst)
         || (0    >= i_dwDst_Len)
       )
    {
        return -1;
    }

    while (    ( 0ul != (*i_pSrc))
            && (2 <= l_iLength)
          )
    {
        l_dwCh = (*i_pSrc);

        if (0x80 > l_dwCh)
        {
            *o_pDst = (tACHAR)(l_dwCh & 0x7Ful);
            o_pDst ++;
            l_iLength--;
        }
        else if (0x800ul > l_dwCh)
        {
            if (3 <= l_iLength)
            {
                *o_pDst = (tACHAR)(0xC0ul | ((l_dwCh >> 6) & 0x1Ful));  o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >> 0) & 0x3Ful));  o_pDst ++;
                l_iLength -= 2;
            }
            else
            {
                break;
            }
        }
        else if (0x10000ul > l_dwCh)
        {
            if (4 <= l_iLength)
            {
                *o_pDst = (tACHAR)(0xE0ul | ((l_dwCh >> 12) & 0x0Ful));  o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >>  6) & 0x3Ful));  o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >>  0) & 0x3Ful));  o_pDst ++;
                l_iLength -= 3;
            }
            else
            {
                break;
            }
        }
        else if (0x200000ul  > l_dwCh)
        {
            if (5 <= l_iLength)
            {
                *o_pDst = (tACHAR)(0xF0ul | ((l_dwCh >> 18) & 0x07ul)); o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >> 12) & 0x3Ful)); o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >>  6) & 0x3Ful)); o_pDst ++;
                *o_pDst = (tACHAR)(0x80ul | ((l_dwCh >>  0) & 0x3Ful)); o_pDst ++;
                l_iLength -= 4;
            }
            else
            {
                break;
            }
        }
        else
        {
            *o_pDst = '?';
            o_pDst ++;
            l_iLength -= 1;
        }

        if (*i_pSrc)
        {
            ++i_pSrc;
        }
        else
        {
            break;
        }
    }

    *o_pDst = 0;

    return i_dwDst_Len - l_iLength;
}//Convert_UTF32_To_UTF8


#endif
