////////////////////////////////////////////////////////////////////////////
////        (C) Copyright Dario Cortese                                 ////
//// This source code may only be used by Dario Cortese licensed users  ////
//// This source code may only be distributed to Dario Cortese licensed ////
//// users.  No other use, reproduction  or distribution is permitted   ////
//// without written permission.                                        ////
////                                                                    ////
//// Author: Dario Cortese                                              ////
//// Client: Mariano Cerbone (only for Project Mangoose)                ////
//// User: Dario Cortese                                                ////
//// Created on 26/01/2012                                              ////
//// Modify on 12/05/2018 to be adapted at XC8 compiler                 ////
//// File: CD_types.h                                                   ////
//// Description:                                                       ////
////    This file has the generic variable type, structure definition   ////
////      used in my developed code.                                    ////
////    I use this types to isolate my code from processor specific     ////
////      data size                                                     ////
////////////////////////////////////////////////////////////////////////////


#ifndef _CD_TYPES_H_
#define _CD_TYPES_H_

#ifdef DOXYGEN
    #define section( YY )
#endif

//PLEASE INDICATE THE COMPILER
//#define  COMPILER_CCS
#define  COMPILER_MICROCHIP
//#define COMPILER_VISUALDSP

//PLEASE INDICATE PLATFORM OR PROCESSOR FAMILY
//#define PLATFORM_PIC32
//#define PLATFORM_PIC24
#define PLATFORM_PICxx
//#define PLATFORM_AVR
//#define PLATFORM_BLACKFIN





#ifdef PLATFORM_PICxx
   //#define TRUE   TRUE
   //#define FALSE  FALSE
   #define NULL   0
   #ifdef COMPILER_CCS
      typedef unsigned int8   uint8_t;
      typedef signed int8   sint8_t;
      typedef unsigned int16   uint16_t;
      typedef signed int16   sint16_t;
      typedef unsigned int32   uint32_t;
      typedef signed int32   sint32_t;
      typedef float32   float32_t;
      typedef signed int8   sint_t;
      typedef unsigned int8   uint_t;
   #endif //COMPILER_CCS
   #ifdef COMPILER_MICROCHIP
    #include <stdint.h>

      #define TRUE  0xffffffff
      #define FALSE 0 

      typedef unsigned int8_t   uint_t;
      typedef unsigned long  uint32_t;
      typedef signed int8_t   sint_t;
      typedef signed int8_t   sint8_t;
      typedef signed int16_t   sint16_t;
      typedef signed long    sint32_t;
   #endif
#endif //PLATFORM_PICxx


#ifdef PLATFORM_PIC24
   #ifdef COMPILER_CCS
   #endif //COMPILER_CCS
#endif //PLATFORM_PIC24

#ifdef PLATFORM_PIC32
#endif //PLATFORM_PIC32


#ifdef PLATFORM_BLACKFIN
   #ifdef COMPILER_VISUALDSP
      #define TRUE   true
      #define FALSE  false
      //#define NULL   NULL
      //for blackfin bf533 hw supported and managed
      typedef unsigned char    uint8_t;
      typedef signed char      sint8_t;
      typedef unsigned short  uint16_t;
      typedef signed short      sint16_t;
      typedef unsigned int    uint32_t;
      typedef signed int        sint32_t;
      //typedef unsigned long   uint32_t;
      //typedef signed long       sint32_t;
      //for blackfin bf533 sw supported and managed
      typedef unsigned long long  uint64_t;
      typedef signed long long    sint64_t;
      typedef float              float32_t;
      typedef signed int   sint_t;
      typedef unsigned int   uint_t;
      //typedef double            float32_t;
      //typedef unsigned fract      ufract16_t;
      //typedef signed fract      sfract16_t;
      //typedef unsigned long fract   ufract32_t;
      //typedef signed long fract   sfract32_t;
   #endif //COMPILER_VISUALDSP
#endif //PLATFORM_BLACKFIN



//little-endian bytes ordering
typedef union uniInt8{
   uint8_t i8;
   struct{
      uint8_t Nibble0:4;
      uint8_t Nibble1:4;
   }i4log;
   struct{
      uint8_t bit0:1;
      uint8_t bit1:1;
      uint8_t bit2:1;
      uint8_t bit3:1;
      uint8_t bit4:1;
      uint8_t bit5:1;
      uint8_t bit6:1;
      uint8_t bit7:1;
   }bits;
}TuniInt8_t; //union uniInt8


//little-endian bytes ordering
typedef union uniInt16{
   uint16_t i16;
   uint8_t i8[2];
   struct{
      uint8_t low;
      uint8_t high;
   }i8log;
   struct{
      uint16_t bit0:1;
      uint16_t bit1:1;
      uint16_t bit2:1;
      uint16_t bit3:1;
      uint16_t bit4:1;
      uint16_t bit5:1;
      uint16_t bit6:1;
      uint16_t bit7:1;
      uint16_t bit8:1;
      uint16_t bit9:1;
      uint16_t bit10:1;
      uint16_t bit11:1;
      uint16_t bit12:1;
      uint16_t bit13:1;
      uint16_t bit14:1;
      uint16_t bit15:1;
   }bits;
}TuniInt16_t; //end union uniInt16


//little-endian bytes ordering
typedef union uniInt32{
   uint32_t i32;
   struct{
      uint16_t i16low;
      uint16_t i16high;   
   }i16log;      //logical
   uint16_t i16[2];

   struct{
      uint8_t lower;
      uint8_t low;
      uint8_t high;
      uint8_t higher;
   }i8log;      //logical 
   uint8_t i8[4];

#ifndef COMPILER_MICROCHIP
   struct{
      uint32_t bit0:1;
      uint32_t bit1:1;
      uint32_t bit2:1;
      uint32_t bit3:1;
      uint32_t bit4:1;
      uint32_t bit5:1;
      uint32_t bit6:1;
      uint32_t bit7:1;
      uint32_t bit8:1;
      uint32_t bit9:1;
      uint32_t bit10:1;
      uint32_t bit11:1;
      uint32_t bit12:1;
      uint32_t bit13:1;
      uint32_t bit14:1;
      uint32_t bit15:1;
      uint32_t bit16:1;
      uint32_t bit17:1;
      uint32_t bit18:1;
      uint32_t bit19:1;
      uint32_t bit20:1;
      uint32_t bit21:1;
      uint32_t bit22:1;
      uint32_t bit23:1;
      uint32_t bit24:1;
      uint32_t bit25:1;
      uint32_t bit26:1;
      uint32_t bit27:1;
      uint32_t bit28:1;
      uint32_t bit29:1;
      uint32_t bit30:1;
      uint32_t bit31:1;
   }bits;
#endif
}TuniInt32_t; //end union uniInt32


#ifdef PLATFORM_BLACKFIN
   //little-endian bytes ordering
   typedef union uniInt64{
      uint64_t i64;
      struct{
         uint32_t i32low;
         uint32_t i32high;   
      }i32log;      //logical
      uint32_t i32[2];
      struct{
         uint16_t i16lower;
         uint16_t i16low;
         uint16_t i16high;   
         uint16_t i16higher;   
      }i16log;      //logical
      uint16_t i16[4];
      uint8_t i8[8];
   }TuniInt64_t; //end union uniInt64
#endif //PLATFORM_BLACKFIN


#endif //_CD_TYPES_H_
