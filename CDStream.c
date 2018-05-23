////////////////////////////////////////////////////////////////////////////
////        (C) Copyright Dario Cortese                                 ////
//// This source code may only be used by Dario Cortese licensed users  ////
//// This source code may only be distributed to Dario Cortese licensed ////
//// users.  No other use, reproduction  or distribution is permitted   ////
//// without written permission.                                        ////
////                                                                    ////
//// Author: Dario Cortese                                              ////
//// Client: Mariano Cerbone (only for Project Mangoose)                ////
//// Created on 26/07/2012                                              ////
//// Modify on 12/05/2018 to be adapted at XC8 compiler                 ////
//// File: cdstream.c                                                   ////
//// Description:                                                       ////
////    This file has the functions to manage a generic byte stream.    ////
////    The stream have .....                                           ////  
////                                                                    ////
////  cdinitStream(&pStream, &pPtrBuff, pBuffSize) init the cdstream    ////
////      structure with the pointer to data buffer and the size of     ////
////      stream buffer                                                 ////
////                                                                    ////
////  cdstreamPutAvilableVals(&pStream) return the number of available  ////
////        bytes to be written inside the stream; 0 mean buffer full   ////
////                                                                    ////
////  cdstreamPutVal(&pStream, pData) write the pData in the stream     ////
////  cdstreamPutArrayVal(&stream, &pData[0], iNumData) write a series  ////
////                              of vals (ptr uitn16 array) inside the ////
////                              indicated streamer                    ////
////                                                                    ////
////  cdstreamRemoveLastVal(&pStream) remove last putted byte           ////
////                                                                    ////
////  cdstreamGetNumVals(&pStream) return the number of byte in stream  ////
////        that could be readed; 0 mean nothing to be readed or reading////
////        blocked.                                                    ////
////                                                                    ////
////  cdstreamPregetVal(&pStream) return byte from stream but doesn't   ////
////        remove it from stream                                       ////
////                                                                    ////
////  cdstreamGetVal(&pStream) return byte from stream and remove it    ////
////                                                                    ////
////  cdstreamFlush(&pStream) revove all bytes from the stream          ////
////                                                                    ////
////  cdstreamGetError(&pStream) return error code for indicate stream  ////
////                                                                    ////
////////////////////////////////////////////////////////////////////////////
//// cdstream8ToUINT8(&stream)    extract from stream a unsigned 8bits  ////
//// cdstream8ToUINT16(&stream)   extract from stream a unsigned 16bits ////
//// cdstream8ToUINT32(&stream,&Val32bit) extract from stream a unsigned////
////                                        32bits value                ////
//// UINT8toCDStream8(&stream,Val32bit)  add one val (formatted as 8    ////
////                                        bits) to stream             ////
//// UINT16toCDStream8(&stream,Val16bit) add two val (formatted as 8    ////
////                                        bits MSBfirst) to stream    ////
//// UINT32toCDStream8(&stream,Val32bit) add four val (formatted as 8   ////
////                                        bits MSBfirst) to stream    ////
////                                                                    ////
////////////////////////////////////////////////////////////////////////////
////                                                                    ////
//// cdstreamToCDStream(&stream_org, &stream_dest, numOfVals) copy      ////
////                                      numOfVals Vals from stream_org////
////                                      stream to stream_dest stream  ////
//// fastCDStreamToCDStream(&stream_org, &stream_dest, numOfVals) copy  ////
////                                      numOfVals Vals from stream_org////
////                                      stream to stream_dest stream  ////
////                                      in fast mode (unsafe, some    ////
////                                      errors is not detected)       ////
////                                                                    ////
////////////////////////////////////////////////////////////////////////////



#ifndef _CDSTREAM_C_
#define _CDSTREAM_C_

//#include <stdlib.h>
#include "CDStream.h"

#ifdef DOXYGEN
    #define section( YY )
#endif

sint_t CDstreamFunctLastERROR; //!< every function that can't return a value to advice for an error, uses this global variable to advise for a error that cause a un unaxpected exit

/*! \fn sint_t cdstreamInit(CDStream_struct* pStream, CDBuffDataType_t *pPtrBuff,CDBuffWidthType_t pBuffSize)
   \author Dario Cortese
   \date 26-07-2012 mod in 18-Apr-2013 
   \version 1.1
   \brief initialize the streamer data (and structure) 
   \param pStream is the pointer to CDstream structure to initialize
   \param pPtrBuff is the pointer to external allocated buffer
   \param pBuffSize is the dimension, in bytes, of external allocated buffer
   \return 0 or positive number if all ok, otherwise return -1 or negative numbers
   \note for errors the same returned value is also copied in CDstreamFunctLastERROR
   \todo re-test it
*/
sint8_t cdstreamInit(CDStream_struct* pStream, CDBuffDataType_t *pPtrBuff, CDBuffWidthType_t pBuffSize){
   CDstreamFunctLastERROR=0;   
   if( pStream == NULL ){ CDstreamFunctLastERROR=-1; return -1; }
   
   pStream->posWrite = 0;
   pStream->posRead = 0;
   pStream->errors = CDSTREAMERR_UNITIALIZED;
   if( pPtrBuff == NULL ){ CDstreamFunctLastERROR = -2; return -2; }
   
   pStream->buffPtr = pPtrBuff;
   if( pBuffSize==0 ){ CDstreamFunctLastERROR=-3; return -3;}
   if( pBuffSize > CDSTREAMER_MAX_BUFFER_SIZE ) {CDstreamFunctLastERROR=-4; return -4;}
   
   pStream->buffSize = pBuffSize;   
   pStream->errors= CDSTREAMERR_NO_ERROR;
   pStream->autoFlush = TRUE;
   pStream->blockRead = FALSE;
   return 0;
} 


/*! \fn sint_t cdstreamPutVal(CDStream_struct* pStream, CDBuffDataType_t pData)
   \author Dario Cortese
   \date 26-07-2012 mod in 18-Apr-2013 
   \version 1.1
   \brief write a byte inside the indicated streamer
   \param pStream is the pointer to CDstream (data structure)
   \param pData is the data (16bits unsigned) to store in the stream
   \return 0 or positive number if all ok, otherwise return -1 or negative numbers to indicates errors
   \note for errors the same returned value is also copied in CDstreamFunctLastERROR
   \note for error code see STREAMERR_WRITE_FULL or STREAMERR_UNITIALIZED
   \see cdstreamGetError  
   \todo re-test it
*/
#ifdef PLATFORM_BLACKFIN
section ("L1_code")
#endif
sint_t cdstreamPutVal(CDStream_struct* pStream, CDBuffDataType_t pData){
   CDstreamFunctLastERROR=0;   
   if( pStream== NULL ){CDstreamFunctLastERROR=-1; return -1; }
   if(( pStream->errors & CDSTREAMERR_UNITIALIZED )!=0){CDstreamFunctLastERROR=-2; return -2;}
   
   if(   pStream->posWrite >= pStream->buffSize){
      //error, write will be over the buffer
      pStream->errors |= CDSTREAMERR_WRITE_FULL ;
      CDstreamFunctLastERROR=-3;      
      return -3;
   }
   pStream->buffPtr[ pStream->posWrite ]= pData;
   pStream->posWrite++;
   return 0;
}


/*! \fn sint_t cdstreamPutArrayVal(CDStream_struct* pStream, CDBuffDataType_t *pData, CDBuffWidthType_t iNumData)
   \author Dario Cortese
   \date 30-01-2013 mod in 18-Apr-2013 
   \version 1.1
   \brief write a series of vals (uitn16 array) inside the indicated streamer
   \param pStream is the pointer to CDstream (data structure)
   \param pData is the data source pointer (16bits unsigned pointer) for data to store in the stream
   \param iNumData is the number of data to transfer in stream
   \return 0 or positive number if all ok, otherwise return -1 or negative numbers to indicates errors
   \note for errors the same returned value is also copied in CDstreamFunctLastERROR
   \note for error code see STREAMERR_WRITE_FULL or STREAMERR_UNITIALIZED
   \see cdstreamGetError  
   \todo re-test it
*/
#ifdef PLATFORM_BLACKFIN
section ("L1_code")
#endif
sint_t cdstreamPutArrayVal(CDStream_struct* pStream, CDBuffDataType_t *pData, CDBuffWidthType_t iNumData){
   CDBuffDataType_t *u16ptr; 
   
   CDstreamFunctLastERROR =0;   
   if( pStream== NULL ){CDstreamFunctLastERROR=-1; return -1;}
   if(( pStream->errors & CDSTREAMERR_UNITIALIZED )!=0){CDstreamFunctLastERROR=-2; return -2;}
   
   if(   (pStream->posWrite + iNumData) >= pStream->buffSize) {
      //error, write will be over the buffer
      pStream->errors |= CDSTREAMERR_WRITE_FULL;
      CDstreamFunctLastERROR=-3;      
      return -3;
   }
   u16ptr = &pStream->buffPtr[ pStream->posWrite ];
   pStream->posWrite += iNumData;
   for(;iNumData>0;iNumData--){
      *u16ptr = *pData;
      pData++;
      u16ptr++;
   }   
   return 0;
}





/*! \fn CDBuffWidthType_t cdstreamGetNumVals(CDStream_struct* pStream)
   \author Dario Cortese
   \date 26-07-2012 mod in 18-Apr-2013 
   \version 1.1
   \brief return the number of bytes wainting to be read from indicated stream
   \param pStream is the pointer to CDstream (data structure)
   \return a positive number to indicates bytes in the stream, or 0 and in this case if CDstreamFunctLastERROR=0 it all ok, otherwise if CDstreamFunctLastERROR=-1 or negative numbers to indicates errors
   \note if BlockRead is activated then this function return always 0
   \note for error code see CDstreamFunctLastERROR and STREAMERR_UNITIALIZED
   \see cdstreamGetError  
   \todo re-test it
*/
CDBuffWidthType_t cdstreamGetNumVals(CDStream_struct* pStream){
   sint32_t numB;

   CDstreamFunctLastERROR=0;   
   if( pStream== NULL ) {
     CDstreamFunctLastERROR=-1;
     return 0;
   } 
   if(( pStream->errors & CDSTREAMERR_UNITIALIZED )!=0){
     CDstreamFunctLastERROR=-2;
     return 0;
   } 
   //if stream read is blocked then return always zero byte readable
   if( pStream->blockRead){
     CDstreamFunctLastERROR=-3;
     return 0;   //read blocked so always indicates 0 available val to be readed
   }
   numB = pStream->posWrite;
   //if there isn't data in the stream buffer to be more fast then.....
   if(numB == 0) return 0; //no data in the stream
   numB -=  pStream->posRead;
   //if posread is equal or over the poswrite then its mean that there isn't bytes to read so....
   if(numB <= 0){
      if(pStream->autoFlush){
         //reset the logic counter (restart)
          pStream->posWrite =0;
         pStream->posRead =0;
      }
      return 0; //no data in the stream
   }
   return (unsigned)numB;   //return the number of bytes (a positive number more than 0)
}



/*! \fn CDBuffDataType_t  cdstreamPregetVal(CDStream_struct* pStream)
   \author Dario Cortese
   \date 26-07-2012 mod in 18-Apr-2013 
   \version 1.1
   \brief return the next available byte from the stream, but doesn't remove it from stream, so next timer you read stream, it return the same actual value
   \param pStream is the pointer to CDstream (data structure)
   \return value readed in the stream, but if returned value is 0 then check CDstreamFunctLastERROR if <0, this indicates an error  
   \note after return of this function see CDstreamFunctLastERROR il less than zero
   \note if BlockRead is activated then this function generate STREAMERR_READ_EMPTY error and return -4
   \note for error code see STREAMERR_READ_EMPTY or STREAMERR_UNITIALIZED
   \see cdstreamGetError, CDstreamFunctLastERROR  
   \todo re-test it
*/
CDBuffDataType_t  cdstreamPregetVal(CDStream_struct* pStream){
   CDBuffDataType_t val;
   CDBuffWidthType_t numB;
   
   CDstreamFunctLastERROR=0;
   if( pStream== NULL ){
     CDstreamFunctLastERROR=-1;
     return 0;
   }
   if(( pStream->errors & CDSTREAMERR_UNITIALIZED )!=0){
     CDstreamFunctLastERROR=-2;
     return 0;
   }
   numB = cdstreamGetNumVals(pStream);
   //if happened an error into streamGetNumBytes 
   if(numB == 0){
     if(CDstreamFunctLastERROR < 0 ){
       CDstreamFunctLastERROR = -3; //error in a sub call
     }else{
       //generate in the stream the reading an empty stream error
       pStream->errors |= CDSTREAMERR_READ_EMPTY;
       CDstreamFunctLastERROR = -4;
     }
     return 0;
   }   
   //if numB > 0 then
   val= pStream->buffPtr[ pStream->posRead ];
   return val; //compiler convert it   
}



/*! \fn CDBuffDataType_t cdstreamGetVal(CDStream_struct* pStream)
   \author Dario Cortese
   \date 26-07-2012 mod in 18-Apr-2013 
   \version 1.1
   \brief return the next available byte from the stream, and remove it from the stream
   \param pStream is the pointer to CDstream (data structure)
   \return value readed in the stream, but if returned value is 0 then check CDstreamFunctLastERROR if <0, this indicates an error
   \note after return of this function see CDstreamFunctLastERROR il less than zero
   \note if BlockRead is activated then this function generate STREAMERR_READ_EMPTY error and return -4
   \note for error code see STREAMERR_READ_EMPTY or STREAMERR_UNITIALIZED
   \see cdstreamGetError  
   \todo re-test it
*/
CDBuffDataType_t  cdstreamGetVal(CDStream_struct* pStream){
   CDBuffDataType_t val;

   //useless, zeroed in the called function CDstreamFunctLastERROR=0;
   val = cdstreamPregetVal(pStream);
   //if no error happen then increment read logical pointer for the stream
   
   if(( val == 0) && (CDstreamFunctLastERROR < 0)){
      //useless CDstreamFunctLastERROR = CDstreamFunctLastERROR;
      return 0;   //return without discard char but called function error code
   }
   //pStream->posRead++;
   cdstreamDiscardGetVal(pStream);
   return val;
}      



/*! \fn uint8_t cdstreamGetError(CDStream_struct* pStream)
   \author Dario Cortese
   \date 26-07-2012 mod in 18-Apr-2013
   \version 1.0
   \brief return the error value (is a bit map value) and reset it in the stream
   \param pStream is the pointer to CDstream (data structure)
   \return the error value (uint32) but is a bit map
   \note see STREAMERR_....
   \todo re-test it
*/
uint8_t cdstreamGetError(CDStream_struct* pStream){
   uint32_t errval;

   CDstreamFunctLastERROR=0;
   //if the pointer is null then return error for uninitialized stream
   if( pStream== NULL ) return CDSTREAMERR_UNITIALIZED;
   
   errval= pStream->errors; 
   //never must be reset the unitialized stream signaling/error
   if((errval & CDSTREAMERR_UNITIALIZED) !=0){
      pStream->errors= CDSTREAMERR_UNITIALIZED;
   }else{
      pStream->errors= CDSTREAMERR_NO_ERROR;
   }
   return errval;
}



/*! \fn sint_t cdstreamFlush(CDStream_struct* pStream)
   \author Dario Cortese
   \date 26-07-2012 mod in 18-Apr-2013 
   \version 1.1
   \brief empty the stream buffer and reset errors
   \param pStream is the pointer to CDstream (data structure)
   \return 0 or positive number if all ok, otherwise return -1 or negative numbers to indicates errors
   \note for error code see STREAMERR_UNITIALIZED
   \see cdstreamGetError  
   \todo re-test it
*/
sint_t cdstreamFlush(CDStream_struct* pStream){
   //sint32_t numB;
   
   CDstreamFunctLastERROR=0;
   if( pStream== NULL ){ CDstreamFunctLastERROR=-1; return -1; }
   //numB = cdstreamGetNumVals(pStream);
   //if( numB < 0) return numB;
   //else numB if 0 or more then mean that pStream is initializated and not null 
   pStream->posWrite =0;
   pStream->posRead =0;
   pStream->errors= CDSTREAMERR_NO_ERROR;   //reset errors
   return 0;
}



/*! \fn CDBuffDataType_t cdstreamRemoveLastVal(CDStream_struct* pStream)
   \author Dario Cortese
   \date 31-07-2012 mod in 18-Apr-2013
   \version 1.1
   \brief remove last putted byte in stream buffer, and return its value
   \param pStream is the pointer to CDstream (data structure)
   \return value readed in the stream, but if returned value is 0 then check CDstreamFunctLastERROR if <0, this indicates an error
   \note after return of this function see CDstreamFunctLastERROR il less than zero
   \todo re-test it
*/
CDBuffDataType_t cdstreamRemoveLastVal(CDStream_struct* pStream){
   CDBuffDataType_t val;

   CDstreamFunctLastERROR=0;
   if( pStream== NULL ){CDstreamFunctLastERROR=-1; return 0;}
   if(( pStream->errors & CDSTREAMERR_UNITIALIZED )!=0){CDstreamFunctLastERROR=-2; return 0;}
   if(pStream->posWrite < 1){CDstreamFunctLastERROR=-3; return 0;}
   //if(pStream->posWrite >= 1){
      pStream->posWrite--;
      val = pStream->buffPtr[ pStream->posWrite ];
      return val;   
   //}
}


/*! \fn CDBuffDataWidth cdstreamPutAvilableVals(CDStream_struct* pStream)
   \author Dario Cortese
   \date 30-07-2012 mod in 18-Apr-2013
   \version 1.1
   \brief return the number of available bytes that could be written into indicated stream
   \param pStream is the pointer to CDstream (data structure)
   \return the number of free value available in the stream, but if returned value is 0 then check CDstreamFunctLastERROR if <0, this indicates an error
   \note after return of this function see CDstreamFunctLastERROR il less than zero
   \note for error code see STREAMERR_UNITIALIZED
   \see ccstreamGetError  
   \todo re-test it
*/
CDBuffWidthType_t cdstreamPutAvilableVals(CDStream_struct* pStream){
   CDBuffWidthType_t numB;

   CDstreamFunctLastERROR=0;
   if( pStream== NULL ) {CDstreamFunctLastERROR=-1; return 0;}
   if(( pStream->errors & CDSTREAMERR_UNITIALIZED )!=0){CDstreamFunctLastERROR=-2; return 0;}
   numB = pStream->buffSize;
   numB -= pStream->posWrite;
   //if poswrite is equal or over the buffsize then its mean that there isn't bytes to write (buffer full).
   //if(numB <= 0){
   if(numB > pStream->buffSize){
      return 0; //no data in the stream
   }
   return numB;   //return the number of available bytes in the buffer
}


//*********************************************************************************************************************
//*********************************************************************************************************************
//*********************************************************************************************************************
//*********************************************************************************************************************

/*!   \fn sint16_t cdstream8ToUINT8(CDStream_struct* cds){
   \author Dario Cortese
   \date 31-07-2012 mod in 18-Apr-2013
   \version 1.1
   \brief extract from stream a unsigned byte 
   extract val from stream and convert it in byte (range value from 0 to 255), after return it.
   \n if there is an error return a negative number, but if value from stream is positive but over 255, it will be masked with 0xFF and no error return
   \param cds is the cdstream pointer
   \return a signed 16bits that if negative indicates error, if 0 to 255 indicates extracted uint8 value
   \note after return of this function see CDstreamFunctLastERROR il less than zero
   \note if the wanted value is a signed int8 then cast the returned value to it after you have checked the negative value for errors
   \todo re-test it
*/
sint16_t cdstream8ToUINT8(CDStream_struct* cds){
   CDBuffDataType_t val; 
   //useless because following function execute same action:CDstreamFunctLastERROR=0;
   val= cdstreamGetVal(cds);
   if(CDstreamFunctLastERROR<0) return CDstreamFunctLastERROR;
   return val&0xFF;
}

/*!   \fn sint32_t cdstream8ToUINT16(CDStream_struct* cds){
   \author Dario Cortese
   \date 31-07-2012 mod in 18-Apr-2013
   \version 1.1
   \brief extract from stream a unsigned 16bits value 
   extract two vals from stream and convert they in bytes, after use the first like MSB of 16bits and the second like LSB, after return the 16bits value  (range value from 0 to 65535).
   if there is an error return a negative number, but if single val extrated from stream is positive but over 255, it will be masked with 0xFF and no error return
   \param cds is the cdstream pointer
   \return a signed 32bits that if negative indicates error, if 0 to 65535 indicates extracted uint16 value
   \note after return of this function see CDstreamFunctLastERROR il less than zero
   \note if the wanted value is a signed int16 then cast the returned value to it after you have checked the negative value for errors
   \todo re-test it
*/
sint32_t cdstream8ToUINT16(CDStream_struct* cds){
   CDBuffDataType_t val;
   CDBuffDataType_t val2;

   val= cdstreamGetVal(cds);
   if(CDstreamFunctLastERROR<0) return CDstreamFunctLastERROR;
   val2= cdstreamGetVal(cds);
   if(CDstreamFunctLastERROR<0) return CDstreamFunctLastERROR;
   return ((val2 & 0xFF) + ((( (uint16_t)val) <<8 ) & 0xFF00 ) );
}


/*!   \fn sint_t cdstream8ToUINT32(CDStream_struct* cds,uint32_t* ptrVal){
   \author Dario Cortese
   \date 31-07-2012 mod in 18-Apr-2013
   \version 1.1
   \brief extract from stream a unsigned 32bits value 
   extract four vals from stream and convert they in bytes, after use the first like MSB of 32bits and the fourth like LSB, after return the 32bits value
   if there is an error return a negative number, but if single val extrated from stream is positive but over 255, it will be masked with 0xFF and no error return
   \param cds is the cdstream pointer
   \param ptrVal is a pointer to external uint32 that is used to store extracted uint32 from stream values 
   \return if negative indicates error, if 0 or more that all is ok
   \note after return of this function see CDstreamFunctLastERROR il less than zero
   \note if the wanted value is a signed int32 then cast the returned value to it after you have checked if the returned value is negative (to check errors)
   \todo re-test it
*/
sint_t cdstream8ToUINT32(CDStream_struct* cds, uint32_t* ptrVal){
   CDBuffDataType_t val;
   CDBuffDataType_t val2;
   CDBuffDataType_t val3;
   CDBuffDataType_t val4;

   val= cdstreamGetVal(cds);
   if(CDstreamFunctLastERROR<0) return CDstreamFunctLastERROR;
   val2= cdstreamGetVal(cds);
   if(CDstreamFunctLastERROR<0) return CDstreamFunctLastERROR;
   val3= cdstreamGetVal(cds);
   if(CDstreamFunctLastERROR<0) return CDstreamFunctLastERROR;
   val4= cdstreamGetVal(cds);
   if(CDstreamFunctLastERROR<0) return CDstreamFunctLastERROR;
   //*ptrVal =(((uint32_t)val <<24) & 0xFF000000);
   //*ptrVal +=(        (val2 <<16) & 0x00FF0000);
   //*ptrVal +=(        (val3 <<8)  & 0x0000FF00);
   //*ptrVal +=(         val4       & 0x000000FF);
   *ptrVal= ((( (uint32_t)val <<24 ) & 0xFF000000) + (( (uint32_t)val2 <<16 ) & 0xFF0000 ) + (( (uint16_t)val3 <<8 ) & 0xFF00 ) + ((uint8_t)val4 & 0xFF ) );
   return 0; //all ok
}




/*! \fn sint_t UINT8toCDStream8(CDStream_struct* cds, uint8_t pByte)
   \author Dario Cortese
   \date 02-08-2012
   \version 1.0
   \brief add one val (formatted like 8bits) to stream by a unsigned 8bits
   \param cds is the cdstream pointer
   \param pByte is the 8bits to add as answer; must be signed or unsiged but must be casted to unsigned
   \return 0 or positive number if all ok, otherwise return -1 or negative numbers to indicates errors
   \note after return of this function see CDstreamFunctLastERROR il less than zero
*/
sint_t UINT8toCDStream8(CDStream_struct* cds, uint8_t pByte){
   return cdstreamPutVal(cds, pByte);
}



/*! \fn sint_t UINT16toCDStream8(CDStream_struct* cds, uint16_t pDiByte)
   \author Dario Cortese
   \date 02-08-2012 mod in 18-Apr-2013
   \version 1.1
   \brief add two val (formatted like 8bits) to stream by a unsigned 16bits (first val is MSB last is LSB of 16bits)
   \param cds is the cdstream pointer
   \param pDiByte is the 16bits to add as answer; must be signed or unsiged but must be casted to unsigned
   \return 0 or positive number if all ok, otherwise return -1 or negative numbers to indicates errors
   \note after return of this function see CDstreamFunctLastERROR il less than zero
   \todo re-test it
*/
sint_t UINT16toCDStream8(CDStream_struct* cds, uint16_t pDiByte){
   //check if in streamer there is almost two bytes free
   if(cdstreamPutAvilableVals(cds)<2){CDstreamFunctLastERROR=-20; return -20;}
   cdstreamPutVal(cds, (uint8_t)(pDiByte>>8));
   cdstreamPutVal(cds, (uint8_t)pDiByte);
   return 2;   //added 2 bytes
}


/*! \fn sint_t UINT24toCDStream8(CDStream_struct* cds, uint32_t pQuadByte)
   \author Dario Cortese
   \date 25-10-2012 mod in 18-Apr-2013
   \version 1.1
   \brief add three val (formatted like 8bits) to stream by a unsigned 32bits (first val is MSB last is LSB of 32bits)
   \param cds is the cdstream pointer
   \param pQuadByte is the 32bits to add as answer; must be signed or unsiged but must be casted to unsigned
   \note after return of this function see CDstreamFunctLastERROR il less than zero
   \return 0 or positive number if all ok, otherwise return -1 or negative numbers to indicates errors
   \todo re-test it
*/
sint_t UINT24toCDStream8(CDStream_struct* cds, uint32_t pQuadByte){
   //check if in streamer there is almost four bytes free
   if(cdstreamPutAvilableVals(cds)<3){CDstreamFunctLastERROR=-20; return -20;}
   cdstreamPutVal(cds, (uint8_t)(pQuadByte>>16));
   cdstreamPutVal(cds, (uint8_t)(pQuadByte>>8));
   cdstreamPutVal(cds, (uint8_t)pQuadByte);
   return 4; //added 4 bytes
}


/*! \fn sint_t UINT32toCDStream8(CDStream_struct* cds, uint32_t pQuadByte)
   \author Dario Cortese
   \date 02-08-2012 mod in 18-Apr-2013
   \version 1.1
   \brief add four val (formatted like 8bits) to stream by a unsigned 32bits (first val is MSB last is LSB of 32bits)
   \param cds is the cdstream pointer
   \param pQuadByte is the 32bits to add as answer; must be signed or unsiged but must be casted to unsigned
   \return 0 or positive number if all ok, otherwise return -1 or negative numbers to indicates errors
   \note after return of this function see CDstreamFunctLastERROR il less than zero
   \todo re-test it
*/
sint_t UINT32toCDStream8(CDStream_struct* cds, uint32_t pQuadByte){
   //check if in streamer there is almost four bytes free
   if(cdstreamPutAvilableVals(cds)<4){CDstreamFunctLastERROR=-20; return -20;}
   
   cdstreamPutVal(cds, (uint8_t)((pQuadByte>>24) & 0xff));
   cdstreamPutVal(cds, (uint8_t)((pQuadByte>>16) & 0xff));
   cdstreamPutVal(cds, (uint8_t)((pQuadByte>>8) & 0xff));
   cdstreamPutVal(cds, (uint8_t)(pQuadByte & 0xff));
   return 4; //added 4 bytes
}



/*! \fn sint_t cdstreamToCDStream(CDStream_struct* org, CDStream_struct* dest, CDBuffWidthType_t numOfVals){
   \author Dario Cortese
   \date 03-08-2012 mod in 18-Apr-2013
   \version 1.1
   \brief copy, nad moves the internal pointers, numOfVals Vals from org stream to dest stream
   
   copy numOfVals value from cdstream org to cdstream dest; the copy start from actual read position of org cdstream and start to copy at actual
    position of dest cdstream.
   \n At the end of copy the internal counter of org (get counter) and dest (put counter) was incremented of numOfVals.
   \n Before start the function check if are accessible the two cdstreams and if they have sufficient vals to complete copy.
   \n If when copying an error happen, while read org or write dest, a negative number was returned but the streams pointer are restored as entered
    in this function, pratically no modification are made to stream if an error happen.
   \n To be more precise, when an error copy happen, the values in the dest cdstream from initial position (put position/counter when call this
    function) are unpredictable changed but the initial position is restored as if you had never written anything. 
   \n The possible errors/negative returned values    
   \n -1 problem to access org stream
   \n -2 number of readable val in org stream are less than numOfVals
   \n -3 problem to access dest stream
   \n -4 number of writable val in org stream are less than numOfVals
   \n -5 problem to read a val from stream org
   \n -6 problem to write a val to stream dest

   \param org is the cdstream pointer where val will be readed
   \param dest is the cdstream pointer where val will be written
   \param numOfVals is the number of vals that will be copied from org to dest 
   \return 0 if all ok, otherwise a negative number indicating error type
   \note after return of this function see CDstreamFunctLastERROR il less than zero
   \todo re-test it
*/
sint_t cdstreamToCDStream(CDStream_struct* org, CDStream_struct* dest, CDBuffWidthType_t numOfVals){
   CDBuffDataType_t val;
   CDBuffWidthType_t Appo;
   CDBuffWidthType_t posGet;
   CDBuffWidthType_t posPut;

   CDstreamFunctLastERROR=0;   
   if( numOfVals==0) return 0;   //nothing to do
   //checks if the two streamer have space to move numOfVals vals
   Appo = cdstreamGetNumVals(org);
   //check errors
   if(CDstreamFunctLastERROR < 0){CDstreamFunctLastERROR=-1; return -1;} //an error when attempt to access at org stream
   if(Appo < numOfVals){CDstreamFunctLastERROR=-2; return -2;} //few vals in org stream;
   
   Appo = cdstreamPutAvilableVals(dest);
   //check errors
   if(CDstreamFunctLastERROR < 0){CDstreamFunctLastERROR=-3; return -3;} //an error when attempt to access at dest stream
   if(Appo < numOfVals){CDstreamFunctLastERROR=-4; return -4;} //few vals in dest stream;

   //store intial position, for rollback action if errors happen while copying
   posGet=cdstreamGetPosGet(org);
   posPut=cdstreamGetPosPut(dest);
   
   //copy cycle
   for( ; numOfVals != 0; numOfVals--){
      val = cdstreamGetVal(org);
      //check error in read
      if(CDstreamFunctLastERROR < 0){
         //restore the original state of streamer
         cdstreamSetPosGet(org,posGet);
         cdstreamSetPosPut(dest,posPut);
         CDstreamFunctLastERROR=-5;     
         return -5;
      } 
      
      cdstreamPutVal(dest, val);
      //check error in write
      if(CDstreamFunctLastERROR < 0){
         //restore the original state of streamer
         cdstreamSetPosGet(org,posGet);
         cdstreamSetPosPut(dest,posPut);
         CDstreamFunctLastERROR=-6;     
         return -6;
      } 
   }//end for( ; numOfVals != 0; numOfVals--)
   return 0;   //all ok
}


/*! \fn sint_t fastCDStreamToCDStream(CDStream_struct* org, CDStream_struct* dest, CDBuffWidthType_t numOfVals)
   \author Dario Cortese
   \date 03-08-2012 mod in 18-Apr-2013
   \version 1.1
   \brief fast copy, but unsafe, numOfVals Vals from org stream to dest stream
   
   is as cdstreamToCDStream function but doesn't check if errors happen while copyng, moreover uses mem pointer that access directly to internal stream
    buffers to avoid to spent time to calculate location and check bound limits.
   \n This is dangerous becaus if some change to counters or data inside buffer happen it can't detect and cause unpredictable problem and data corruption.
   \n But if you sure that nothing change streamers when execute this copy, this function allow to save time.
   \n At the end of copy the internal counter of org (get counter) and dest (put counter) was incremented of numOfVals.
   \n Before start the function check if are accessible the two cdstreams and if they have sufficient vals to complete copy.
   \n When copying doesn't detect errors (while read org or write dest)
   \n Only this errors are possible to return    
   \n -2 problem to access org stream or number of readable val in org stream are less than numOfVals
   \n -4 problem to access dest stream or number of writable val in org stream are less than numOfVals

   \param org is the cdstream pointer where val will be readed
   \param dest is the cdstream pointer where val will be written
   \param numOfVals is the number of vals that will be copied from org to dest 
   \return 0 if all ok, otherwise a negative number indicating error type
   \note after return of this function see CDstreamFunctLastERROR il less than zero
   \see cdsstreamToCDStream
   \todo re-test it
*/
sint_t fastCDStreamToCDStream(CDStream_struct* org, CDStream_struct* dest, CDBuffWidthType_t numOfVals){
   CDBuffWidthType_t Appo;
   CDBuffWidthType_t iCount;
   CDBuffDataType_t *buffOrgPtr;
   CDBuffDataType_t *buffDestPtr;
   
   CDstreamFunctLastERROR=0;
   if( numOfVals==0) return 0;   //nothing to do
   //checks if the two streamer have space to move numOfVals vals
   Appo = cdstreamGetNumVals(org);
   if(Appo < numOfVals){CDstreamFunctLastERROR=-2; return -2;} //few vals in org stream or an error when attempt to access at org stream
   Appo = cdstreamPutAvilableVals(dest);
   if(Appo < numOfVals){CDstreamFunctLastERROR=-4; return -4;} //few vals in dest stream or an error when attempt to access at dest stream

   //copy cycle
   //buffOrgPtr= &org.buffPtr[ org->posRead ]);
   //buffDestPtr= &dest.buffPtr[ dest->posWrite ]);
   
   buffOrgPtr= &(org->buffPtr[ org->posRead ]);
   buffDestPtr= &(dest->buffPtr[ dest->posWrite ]);
   
   for(iCount = numOfVals; iCount != 0; iCount--){
      *buffDestPtr = *buffOrgPtr;
      buffDestPtr++;
      buffOrgPtr++;
   }//end for(iCount = numOfVals; iCount != 0; iCount--)
   org->posRead+=numOfVals;
   dest->posWrite+=numOfVals;
   return 0;   //all ok
}




#endif //_CDSTREAM_C_
