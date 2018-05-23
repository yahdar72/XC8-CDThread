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
//// Created on 08/05/2013 modified 04-11-2015                          ////
//// Modify on 19/05/2018 to be adapted at XC8 compiler                 ////
//// File: cdRoundstream.c                                              ////
//// Description:                                                       ////
////    This file has the functions to manage a generic byte stream.    ////
////    The stream have .....                                           ////  
////                                                                    ////
//// cdRStreamInit(&stream, Buffer,BufSize) intialize stream             ////
////     The stream is a allocated stream_struct and the buffer is a     ////
////     allocated array pointer; BufSize is the number of elements of   ////
////                                allocated array                      ////
//// cdRStreamReset(&Stream) reset counter and pointer of stream to      ////
////                            initial value (after cdRStreamInit)      ////
//// cdRStreamPutVal(&stream, Val)   write a val (int16) in the stream   ////
//// cdRStreamPutArrayVal(&stream, &pData[0], iNumData) write a series   ////
////                              of vals inside the indicated streamer  ////
//// cdRStreamPregetVal(&stream)  returns the actual readable val in the ////
////                                stream, but doesn't update counters  ////
////                              Next time you read re-read same val    ////
//// cdRStreamGetVal(&stream)	  returns the actual readable val in the ////
////                                stream, and update internal counters //// 
//// cdRStreamDiscardVal(&stream) Discard actual val, usefull with       ////
////                                 cdRStreamPregetVal                  ////
//// cdRStreamRestartGetVal(&stream)  reset the logical pointer for      ////
////        cdRStreamGetVal so return at the start of stream buffer to   ////
////        reread all data in                                           ////
//// cdRStreamGetNumVals(&stream)      returns the number of vals (int16)////
////                                    wainting to be read in stream    ////
//// cdRStreamPutAvilableVals(&stream) returns the number of vals (int16)////
////                                    that could be written in stream  ////
//// cdRStreamGetSize(&stream)         returns the total number of vals  ////
////                                    (int16) storable in the stream   ////
////                                    (buffer)                         ////
//// cdRStreamBlockRead(&stream, true)  block the reading                ////
//// cdRStreamBlockRead(&stream, false) re-enable the reading            ////
//// cdRStreamIsBlockedRead(&stream)    return true if read is blocked   ////
//// cdRStreamTocdRStream(&stream_org, &stream_dest, numOfVals) copy     ////
////                                      numOfVals Vals from stream_org ////
////                                      stream to stream_dest stream   ////
//// cdRStreamGetError(&Stream) return the error value (is a bit map     ////
////                             value) and reset errors in the stream   ////
//// UINT8tocdRStream(&stream,Val8bit) add a unsigned byte (8bits) to    ////
////                                     stream                          ////
//// UINT16tocdRStream(&stream,Val16bit) add unsigned word (16bits) to   ////
////                                       stream                        ////
//// UINT24tocdRStream(&stream,Val24bit) add unsigned word24b (32bits    ////
////                                       but storing only 24bits) to   ////
////                                       stream                        ////
//// UINT32tocdRStream(&stream,Val32bit) add unsigned DWord (32bits) to  ////
////                                        stream                       ////
//// cdRStreamToUINT8(&stream)    extract from stream a unsigned 8bits   ////
//// cdRStreamToUINT16(&stream)   extract from stream a unsigned 16bits  ////
//// cdRStreamToUINT24(&stream)   extract from stream a unsigned 24bits  ////
//// cdRStreamToUINT32(&stream,&Val32bit) extract from stream a unsigned ////
////                                        32bits value                 ////
/////////////////////////////////////////////////////////////////////////////



#ifndef _CDROUNDINGSTREAM_C_
#define _CDROUNDINGSTREAM_C_

#include "CDRoundstream.h"

#ifdef DOXYGEN
    #define section(YY)
#endif


/*! \fn sint_t cdRStreamInit(CDRStream_t* pStream, CDRStream_data_t* pPtrBuff,CDRStream_counters_t pBuffSize)
	\author Dario Cortese
	\date 08-05-2013
	\version 1.0
	\brief initialize the streamer data (and structure) 
	\param pStream is the pointer to CDRstream structure to initialize
	\param pPtrBuff is the pointer to external allocated buffer
	\param pBuffSize is the dimension, in bytes, of external allocated buffer
	\return 0 or positive number if all ok, otherwise return -1 or negative number
	\todo nothing
*/
sint_t cdRStreamInit(CDRStream_t* pStream, CDRStream_data_t* pPtrBuff, CDRStream_counters_t pBuffSize){
	if( pStream== NULL ) return -1;
	if( pBuffSize==0 ) return -2;
	if( pPtrBuff== NULL ) return -3;
	if( pBuffSize > CDRSTREAMER_MAX_BUFFER_SIZE ) return -4;

    pStream->Writings =0;
	pStream->Readings =0;
	pStream->errors= CDRSTREAMERR_UNITIALIZED;
	pStream->buffPtr = pPtrBuff;
	pStream->buffEndPtr = pPtrBuff + (pBuffSize -1);  
	pStream->buffWRptr = pPtrBuff;
	pStream->buffRDptr = pPtrBuff;
	pStream->buffSize = pBuffSize;	
	pStream->errors= CDRSTREAMERR_NO_ERROR;
	//pStream->autoFlush=true;
	pStream->blockRead=FALSE;
	return 0;
} 

/*! \fn sint_t cdRStreamReset(CDRStream_t* pStream)
	\author Dario Cortese
	\date 08-05-2013
	\version 1.0
	\brief Restore initial streamer pointer and counter (0 readings, 0 writings and pointers to first location of buffer), but not changes buffer size and pointers 
	\param pStream is the pointer to CDRstream structure to initialize
	\return 0 or positive number if all ok, otherwise return -1 or negative number
	\todo nothing
*/
sint_t cdRStreamReset(CDRStream_t* pStream){
	if( pStream== NULL ) return -1;
	if( pStream->buffPtr== NULL ) return -3;

	pStream->Writings =0;
	pStream->Readings =0;
	//pStream->buffPtr = pPtrBuff;
	//pStream->buffEndPtr = pPtrBuff + (pBuffSize -1);  
	pStream->buffWRptr = pStream->buffPtr;
	pStream->buffRDptr = pStream->buffPtr;
	//pStream->buffSize = pBuffSize;	
	pStream->errors= CDRSTREAMERR_NO_ERROR;
	//pStream->autoFlush=true;
	pStream->blockRead=FALSE;
	return 0;
} 




//private/internal function
CDRStream_data_t * cdRStream_CalcPrevPosRead(CDRStream_t* pStream)
{
	//if there is more data to read then reamain on same address
	//if(pStream->buffRDptr == pStream->buffWRptr) return pStream->buffWRptr;
	//if there is a rounding then return the start of buffer
	if(pStream->buffRDptr == pStream->buffPtr) return pStream->buffEndPtr;
	//also calculate new position
	return pStream->buffRDptr -1;
}


//private/internal function
CDRStream_counters_t cdRStream_CalcAvailableWritings(CDRStream_t* pStream)
{
	CDRStream_counters_t a;
	CDRStream_counters_t b;
	b= pStream->buffSize;
	a= (CDRStream_counters_t) (pStream->Writings - pStream->Readings);
	if (a >= b) return 0;   
	return (CDRStream_counters_t)(b - a); 
}

//private/internal function
CDRStream_counters_t cdRStream_CalcAvailableReadings(CDRStream_t* pStream)
{
	CDRStream_counters_t a;
	CDRStream_counters_t b;
	b= pStream->buffSize;
	a= (CDRStream_counters_t)(pStream->Writings - pStream->Readings);
	if (a >= b) return b;   
	return a; 
}



/*! \fn sint_t cdRStreamPutVal(CDRStream_t* pStream, CDRStream_data_t pData)
	\author Dario Cortese
	\date 08-05-2013
	\version 1.0
	\brief write a val inside the indicated streamer
	\param pStream is the pointer to cdRStream (data structure)
	\param pData is the data (16bits unsigned) to store in the stream
	\return 0 or positive number if all ok, otherwise return -1 or negative numbers to indicates errors
	\note for error code see CDRSTREAMERR_OVERWRITE or STREAMERR_UNITIALIZED
	\see cdRStreamGetError  
	\todo test it
*/
sint_t cdRStreamPutVal(CDRStream_t* pStream, CDRStream_data_t pData){
	if( pStream== NULL ) return -1;
	if(( pStream->errors & CDRSTREAMERR_UNITIALIZED )!=0) return -2;
	
	if(	cdRStream_CalcAvailableWritings(pStream) == 0){
		//error, write will be over the readable value
		pStream->errors |= CDRSTREAMERR_OVERWRITE ;
		//return -3;
		//move read pointer to next elements
		pStream->buffRDptr++;
		if(pStream->buffRDptr == pStream->buffEndPtr)
		{
			pStream->buffRDptr = pStream->buffPtr;
		}

	}
	*(pStream->buffWRptr) = pData;
	pStream->Writings++;
	//rounding
	if(pStream->buffWRptr == pStream->buffEndPtr)
	{
		pStream->buffWRptr = pStream->buffPtr;
	}
	else
	{
		pStream->buffWRptr ++;
	}
	return 0;
}


/*! \fn sint_t cdRStreamPutArrayVal(CDRStream_t* pStream, CDRStream_data_t *pData, CDRStream_counters_t iNumData)
	\author Dario Cortese
	\date 08-05-2013
	\version 1.0
	\brief write a series of vals  inside the indicated stream
	\param pStream is the pointer to cdRStream (data structure)
	\param pData is the data source pointer (8/16/32bits depends by CDRStream_data_t) for data to store in the stream
	\param iNumData is the number of data to transfer in stream
	\return 0 or positive number if all ok, otherwise return -1 or negative numbers to indicates errors
	\note for error code see CDRSTREAMERR_OVERWRITE or STREAMERR_UNITIALIZED
	\see cdRStreamGetError  
	\todo test it
*/
sint_t cdRStreamPutArrayVal(CDRStream_t* pStream, CDRStream_data_t *pData, CDRStream_counters_t iNumData){
	sint_t rval;
	for(;iNumData>0;iNumData--){
		rval = cdRStreamPutVal(pStream, *pData);
		if(rval < 0) return rval;
		
		pData++;
	}	
	return 0;
}





/*! \fn CDRStream_counters_t cdRStreamGetNumVals(CDRStream_t* pStream)
	\author Dario Cortese
	\date 26-07-2012
	\version 1.0
	\brief return the number of bytes wainting to be read from indicated stream
	\param pStream is the pointer to cdRStream (data structure)
	\return 0 or a positive number to indicates bytes in the stream, also return 0 if there is an error
	\note if BlockRead is activated then this function return always 0
	\note for error code see STREAMERR_UNITIALIZED
	\see cdRStreamGetError  
	\todo nothing
*/
CDRStream_counters_t cdRStreamGetNumVals(CDRStream_t* pStream){
	if( pStream== NULL ) return -1;
	if(( pStream->errors & CDRSTREAMERR_UNITIALIZED )!=0) return -2;
	//if stream read is blocked then return always zero byte readable
	if( pStream->blockRead) return 0;	//read blocked so always indicates 0 available val to be read
	
	return cdRStream_CalcAvailableReadings(pStream);
}



/*! \fn sint_t  cdRStreamPregetVal(CDRStream_t* pStream, CDRStream_data_t* ptrVal )
	\author Dario Cortese
	\date 26-07-2012
	\version 1.0
	\brief return the next available byte from the stream (using cdRStreamLastReadVal(pStream)), but doesn't remove it from stream, so next time you read stream, it returns the same actual value
	\param pStream is the pointer to cdRStream (data structure)
	\param ptrVal is the pointer to varaiable than will contain the read value from stream 
	\return 0 or a positive number if all ok, otherwise return -1 or negative numbers to indicates errors
	\note if BlockRead is activated then this function generate STREAMERR_READ_EMPTY error and return -4
	\note for error code see STREAMERR_READ_EMPTY or STREAMERR_UNITIALIZED
	\see cdRStreamGetError , cdRStreamLastReadVal  
	\todo test it
*/
sint_t cdRStreamPregetVal(CDRStream_t* pStream, CDRStream_data_t* ptrVal ){
	//if happened an error into streamGetNumBytes, or no reads then
	
	if(cdRStreamGetNumVals(pStream) == 0){
		//generate in the stream the reading an empty stream error
		pStream->errors |= CDRSTREAMERR_READ_EMPTY;
		return -4; //error for empty stream
	}
	//else return stored value
	*ptrVal = *(pStream->buffRDptr);
	return 0; 	
}



/*! \fn sint_t cdRStreamGetVal(CDRStream_t* pStream, CDRStream_data_t* ptrVal )
	\author Dario Cortese
	\date 08-05-2013
	\version 1.0
	\brief return the next available byte from the stream, and remove it from the stream
	\param pStream is the pointer to cdRStream (data structure)
	\param ptrVal is the pointer to varaiable than will contain the read value from stream 
	\return 0 or a positive number if all ok, otherwise return -1 or negative numbers to indicates errors
	\note if BlockRead is activated then this function generate STREAMERR_READ_EMPTY error
	\note for error code see STREAMERR_READ_EMPTY or STREAMERR_UNITIALIZED
	\see cdRStreamGetError  
	\todo test it
*/
sint_t  cdRStreamGetVal(CDRStream_t* pStream, CDRStream_data_t* ptrVal){
	sint_t val;
	val = cdRStreamPregetVal(pStream, ptrVal);
	if (val < 0) return val;
	//if no error happen then increment read logical pointer for the stream
	pStream->Readings++;
	
	//if there is a rounding then return at the start of buffer
	if(pStream->buffRDptr == pStream->buffEndPtr)
	{
		pStream->buffRDptr = pStream->buffPtr;
	}
	else
	{
		pStream->buffRDptr +=1;
	}
	return 0;
}		


/*! \fn sint_t cdRStreamDiscardVal(CDRStream_t* pStream)
	\author Dario Cortese
	\date 08-05-2013
	\version 1.0
	\brief simply discard actual readable val from stream so next cdRStreamGetVal reads following value
	\param pStream is the pointer to cdRStream (data structure)
	\return 0 or a positive number if all ok, otherwise return -1 or negative numbers to indicates errors
	\note if BlockRead is activated then this function generate STREAMERR_READ_EMPTY error
	\note for error code see STREAMERR_READ_EMPTY or STREAMERR_UNITIALIZED
	\see cdRStreamGetError  
	\todo test it
*/
sint_t cdRStreamDiscardVal(CDRStream_t* pStream){
	if( pStream== NULL ) return -1;
	if(( pStream->errors & CDRSTREAMERR_UNITIALIZED )!=0) return -2;
	//if stream read is blocked then return always zero byte readable
	if( pStream->blockRead) return 0;	//read blocked so always indicates 0 available val to be read
	if(pStream->Readings >= pStream->Writings) return 0;	//if nothing to read there isn't nothing to discard
	pStream->Readings++;
	//if there is a rounding then return at the start of buffer
	if(pStream->buffRDptr == pStream->buffEndPtr)
	{
		pStream->buffRDptr = pStream->buffPtr;
	}
	else
	{
		pStream->buffRDptr +=1;
	}
	return 0;
}
 

/*! \fn int cdRStreamGetError(CDRStream_t* pStream)
	\author Dario Cortese
	\date 26-07-2012
	\version 1.0
	\brief return the error value (is a bit map value) and reset it in the stream
	\param pStream is the pointer to cdRStream (data structure)
	\return the error value but is a bit map
	\note see STREAMERR_....
	\todo test it
*/
int cdRStreamGetError(CDRStream_t* pStream){
	int errval;
	//if the pointer is null then return error for uninitialized stream
	if( pStream== NULL ) return CDRSTREAMERR_UNITIALIZED;
	
	errval= pStream->errors; 
	//never must be reset the unitialized stream signaling/error
	if((errval & CDRSTREAMERR_UNITIALIZED) !=0){
		pStream->errors= CDRSTREAMERR_UNITIALIZED;
	}else{
		pStream->errors= CDRSTREAMERR_NO_ERROR;
	}
	return errval;
}



/*! \fn sint_t cdRStreamFlush(CDRStream_t* pStream)
	\author Dario Cortese
	\date 08-05-2012, modified 04-11-2015
	\version 1.0
	\brief trash unread value int the stream and reset errors
	\param pStream is the pointer to cdRStream (data structure)
	\return 0 if nothing to flush, 1 to indicates one or more data flushed, otherwise return -1 or negative numbers to indicates errors
	\note for error code see STREAMERR_UNITIALIZED
	\see cdRStreamGetError  
	\todo test it
*/
sint_t cdRStreamFlush(CDRStream_t* pStream){
	CDRStream_counters_t numB;
	
	if( pStream== NULL ) return -1;
	numB = cdRStreamGetNumVals(pStream);
	//else numB if 0 or more then mean that pStream is initializated and not null 
	pStream->buffRDptr = pStream->buffWRptr;
	pStream->Readings = pStream->Writings;	
	pStream->errors= CDRSTREAMERR_NO_ERROR;	//reset errors
	if( numB > 0) return 1;
	return 0;
}



/*! \fn CDRStream_counters_t cdRStreamPutAvilableVals(CDRStream_t* pStream)
	\author Dario Cortese
	\date 08-05-2013
	\version 1.0
	\brief return the number of available bytes that could be written into indicated stream
	\param pStream is the pointer to cdRStream (data structure)
	\return 0 or a positive number to indicates bytes in the stream, return 0 also if error happens
	\note for error code see STREAMERR_UNITIALIZED
	\see ccstreamGetError  
	\todo test it
*/
CDRStream_counters_t cdRStreamPutAvilableVals(CDRStream_t* pStream){
	//sint32_t numB;
	//if( pStream== NULL ) return -1;
	if( pStream== NULL ) return 0;
	//if(( pStream->errors & CDRSTREAMERR_UNITIALIZED )!=0) return -2;
	if(( pStream->errors & CDRSTREAMERR_UNITIALIZED )!=0) return 0;
	return cdRStream_CalcAvailableWritings(pStream);
}


/*! \fn sint_t cdRStreamTocdRStream(CDRStream_t* org, CDRStream_t* dest, CDRStream_counters_t numOfVals){
	\author Dario Cortese
	\date 03-08-2012
	\version 1.0
	\brief copy, nad moves the internal pointers, numOfVals Vals from org stream to dest stream
	
	copy numOfVals value from cdRStream org to cdRStream dest; the copy start from actual read position of org cdRStream and start to copy at actual
	 position of dest cdRStream.
	\n At the end of copy the internal counter of org (get counter) and dest (put counter) was incremented of numOfVals.
	\n Before start the function check if are accessible the two cdRStreams and if they have sufficient vals to complete copy.
	\n If when copying an error happen, while read org or write dest, a negative number was returned 
	\n The possible errors/negative returned values	 
	\n -1 problem to access org stream
	\n -2 number of readable val in org stream are less than numOfVals
	\n -3 problem to access dest stream
	\n -4 number of writable val in org stream are less than numOfVals
	\n -5 problem to read a val from stream org
	\n -6 problem to write a val to stream dest

	\param org is the cdRStream pointer where val will be read
	\param dest is the cdRStream pointer where val will be written
	\param numOfVals is the number of vals that will be copied from org to dest 
	\return 0 if all ok, otherwise a negative number indicating error type
	\note to test
*/
sint_t cdRStreamTocdRStream(CDRStream_t* org, CDRStream_t* dest, CDRStream_counters_t numOfVals){
	//sint32_t sAppo;
	CDRStream_counters_t Appo;
	sint_t si;
	CDRStream_data_t val;

	if( numOfVals==0) return 0;	//nothing to do
	//checks if the two streamer have space to move numOfVals vals
	//sAppo = cdRStreamGetNumVals(org);
	Appo = cdRStreamGetNumVals(org); //return only positive values
	//check errors
	//if(sAppo<0) return -1; //an error when attempt to access at org stream
	if(Appo==0) return -1; //an error because 0 data to transfer
	//if(sAppo < numOfVals) return -2; //few vals in org stream;
	if(Appo < numOfVals) return -2; //few vals in org stream;
	
	//sAppo = cdRStreamPutAvilableVals(dest);
	Appo = cdRStreamPutAvilableVals(dest);
	//check errors
	//if(sAppo<0) return -3; //an error when attempt to access at dest stream
	if(Appo==0) return -3; //an error because 0 data to copy
	//if(sAppo < numOfVals) return -4; //few vals in dest stream;
	if(Appo < numOfVals) return -4; //few vals in dest stream;

	//copy cycle
	for( ; numOfVals != 0; numOfVals--){
		si = cdRStreamGetVal(org, &val);
		//check error in read
		if(si < 0) return -5;
	
		si = cdRStreamPutVal(dest, val);
		//check error in write
		if(si < 0) return -6;
	}//end for( ; numOfVals != 0; numOfVals--)
	return 0;	//all ok
}


//*********************************************************************************************************************
//*********************************************************************************************************************
//*********************************************************************************************************************
//*********************************************************************************************************************

/*!	\fn sint_t cdRStreamToUINT8(CDRStream_t* cds uint8_t* dat ){
	\author Dario Cortese
	\date 08-05-2013
	\version 1.0
	\brief extract from stream a unsigned byte 
	extract val from stream and convert it in byte (range value from 0 to 255), after return it.
	\n if there is an error return a negative number, but if value from stream is positive but over 255, it will be masked with 0xFF and no error return
	\param cds is the cdRStream pointer
	\param dat is the pointer to extracted and converted data
	\return a negative value indicates error, otherwise a 0 or positive value indicates all ok
	\note if the wanted value is a signed int8 then cast the returned value to it after you have checked the negative value for errors
*/
sint_t cdRStreamToUINT8(CDRStream_t* cds, uint8_t* dat ){
	sint_t rv;
#ifdef CDRSTREAM_DATA_SIZE8
	rv= cdRStreamGetVal(cds, dat);
	if(rv<0) return rv;
#else  
	//if CDRSTREAM_DATA_SIZE16 or CDRSTREAM_DATA_SIZE32
	CDRStream_data_t val;
	rv= cdRStreamGetVal(cds, &val);
	if(rv<0) return rv;
	*dat = (uint8_t)val;
#endif
	return 1; //one data readed
}

/*!	\fn sint_t cdRStreamToUINT16(CDRStream_t* cds, uint16_t* dat){
	\author Dario Cortese
	\date 31-07-2012
	\version 1.0
	\brief extract from stream a unsigned 16bits value 
	extract two vals from stream and convert they in bytes, after use the first like MSB of 16bits and the second like LSB, after return the 16bits value  (range value from 0 to 65535).
	if there is an error return a negative number, but if single val extrated from stream is positive but over 255, it will be masked with 0xFF and no error return
	\param cds is the cdRStream pointer
	\param dat is the pointer to extracted and converted data
	\return a negative value indicates error, if positive indicates all ok ()
	\note if the wanted value is a signed int16 then cast the returned value to it after you have checked the negative value for errors
*/
sint_t cdRStreamToUINT16(CDRStream_t* cds, uint16_t* dat){
	sint_t si;
#ifdef CDRSTREAM_DATA_SIZE8
	CDRStream_data_t val;
	CDRStream_data_t val2;
	si= cdRStreamGetVal(cds, &val);
	if(si<0) return si;
	si= cdRStreamGetVal(cds, &val2);
	if(si<0) return val2;
	*dat = (val2+(((uint16_t)val<<8)));
	return 2;	//two data extracted from buffer
#endif
#ifdef CDRSTREAM_DATA_SIZE16
	si= cdRStreamGetVal(cds, dat);
	if(si<0) return si;
	return 1; // one data extracted from buffer
#endif
#ifdef CDRSTREAM_DATA_SIZE32	
	CDRStream_data_t val;
	si= cdRStreamGetVal(cds, &val);
	if(si<0) return si;
	*dat = (uint16_t)val;
	return 1;	//one data estracted from buffer
#endif 
}


/*!	\fn sint_t cdRStreamToUINT32(CDRStream_t* cds,uint32_t* ptrVal){
	\author Dario Cortese
	\date 08-05-2013
	\version 1.0
	\brief extract from stream a unsigned 32bits value 
	extract vals from stream and convert they in a unsigned 32bits and return it
	if there is an error return a negative number, otherwise return 0 (or a positive value)
	\param cds is the cdRStream pointer
	\param dat is a pointer to external uint32 that is used to store extracted uint32 from stream values 
	\return if negative indicates error, if 0 or more that all is ok
	\note if the wanted value is a signed int32 then cast the returned value to it after you have checked if the returned value is negative (to check errors)
*/
sint_t cdRStreamToUINT32(CDRStream_t* cds, uint32_t* dat){
	sint_t si;
#ifdef CDRSTREAM_DATA_SIZE8
	CDRStream_data_t val;
	CDRStream_data_t val2;
	CDRStream_data_t val3;
	CDRStream_data_t val4;
	si= cdRStreamGetVal(cds, &val);
	if(si<0) return si;
	si= cdRStreamGetVal(cds, &val2);
	if(si<0) return si;
	si= cdRStreamGetVal(cds, &val3);
	if(si<0) return si;
	si= cdRStreamGetVal(cds, &val4);
	if(si<0) return si;
	*dat= (((uint32_t)val<<24)+((uint32_t)val2<<16)+((uint16_t)val3<<8)+val4);
	return 4;	//four data from buffer
#endif //CDRSTREAM_DATA_SIZE8
#ifdef CDRSTREAM_DATA_SIZE16
	CDRStream_data_t val;
	CDRStream_data_t val2;
	si= cdRStreamGetVal(cds, &val);
	if(si<0) return si;
	si= cdRStreamGetVal(cds, &val2);
	if(si<0) return si;
	*dat= (((uint32_t)val<<16)+val2);
	return 2;	//two data from buffer
#endif //CDRSTREAM_DATA_SIZE16
#ifdef CDRSTREAM_DATA_SIZE32
	si= cdRStreamGetVal(cds, &val);
	if(si<0) return si;
	*ptrVal= val;
	return 1;	//one data from buffer
#endif //CDRSTREAM_DATA_SIZE32
}




/*! \fn sint_t UINT8tocdRStream(CDRStream_t* cds, uint8_t pByte)
	\author Dario Cortese
	\date 08-05-2013
	\version 1.0
	\brief add one val (formatted like 8bits) to stream by a unsigned 8bits
	\param cds is the cdRStream pointer
	\param pByte is the 8bits to add to stream; if unsiged must be casted to unsigned
	\return 0 or positive number if all ok, otherwise return -1 or negative numbers to indicates errors
*/
sint_t UINT8tocdRStream(CDRStream_t* cds, uint8_t pByte){
	return cdRStreamPutVal(cds, (CDRStream_data_t)pByte);
}



/*! \fn sint_t UINT16tocdRStream(CDRStream_t* cds, uint16_t pDiByte)
	\author Dario Cortese
	\date 08-05-2013
	\version 1.0
	\brief add two val (formatted like 8bits) to stream by a unsigned 16bits (first val is MSB last is LSB of 16bits)
	\param cds is the cdRStream pointer
	\param pDiByte is the 16bits to add as answer; must be signed or unsiged but must be casted to unsigned
	\return 0 if all ok, otherwise return -1 or negative numbers to indicates errors
*/
sint_t UINT16tocdRStream(CDRStream_t* cds, uint16_t pDiByte){
#ifdef CDRSTREAM_DATA_SIZE8
	signed int si;
	si = cdRStreamPutVal(cds, (CDRStream_data_t)(pDiByte>>8));
	if(si<0) return si;
#endif //CDRSTREAM_DATA_SIZE8
	//lower part of 16bits for CDRSTREAM_DATA_SIZE8 or a single part for CDRSTREAM_DATA_SIZE16 or CDRSTREAM_DATA_SIZE32
	return cdRStreamPutVal(cds, (CDRStream_data_t)(pDiByte));
}



/*! \fn sint_t UINT32tocdRStream(CDRStream_t* cds, uint32_t pQuadByte)
	\author Dario Cortese
	\date 08-05-2013
	\version 1.0
	\brief add four val (formatted like 8bits) to stream by a unsigned 32bits (first val is MSB last is LSB of 32bits)
	\param cds is the cdRStream pointer
	\param pQuadByte is the 32bits to add as answer; must be signed or unsiged but must be casted to unsigned
	\return 0 or positive number if all ok, otherwise return -1 or negative numbers to indicates errors
*/
sint_t UINT32tocdRStream(CDRStream_t* cds, uint32_t pQuadByte){
#ifdef CDRSTREAM_DATA_SIZE8
	signed int si;
	si = cdRStreamPutVal(cds, (CDRStream_data_t)(pQuadByte>>24));
	if(si<0) return si;
	si = cdRStreamPutVal(cds, (CDRStream_data_t)(pQuadByte>>16));
	if(si<0) return si;
	si = cdRStreamPutVal(cds, (CDRStream_data_t)(pQuadByte>>8));
	if(si<0) return si;
#endif //CDRSTREAM_DATA_SIZE8
#ifdef CDRSTREAM_DATA_SIZE16
	signed int si;
	si = cdRStreamPutVal(cds, (CDRStream_data_t)(pQuadByte>>16));
	if(si<0) return si;
#endif //CDRSTREAM_DATA_SIZE16
	return cdRStreamPutVal(cds, (CDRStream_data_t)pQuadByte);
}

#endif //_CDROUNDINGSTREAM_C_