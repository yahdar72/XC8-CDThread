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
//// Created on 08/05/2013  modified 04-11-2015                         ////
//// Modify on 19/05/2018 to be adapted at XC8 compiler                 ////
//// File: cdRoundstream.h                                              ////
//// Description:                                                       ////
////    This file has the variable, structure and function definition   ////
////     to manage a generic rounding stream 				////
////                                                                    ////
//// THIS TYPE OF STREAM IS AS ARRAY BUT WITH THE POWER TO BE WRITTEN   ////
////   AND READ SERIALLY AND CONTINUOSLY.                               ////
//// THIS STREAM IS MORE SLOW AND CODE CONSUMING RESPECT ARRAY BUT HAVE ////
////   MORE CAPABILITY AND VERATILITY VS ARRAY, FOR THIS REASON REDUCE  ////
////   USER CODE AND SAVE TIME                                          ////

//// TO CREATE AND INTIALIZE THIS STREAM YOU MUST ALLOCATE A             ////
////   stream_struct TYPE AND A data ARRAY AND PASS THEY TO INITIALI-    ////
////   ZATION FUNCTION                                                   ////   
//// cdRStreamInit(&stream, Buffer,BufSize) intialize stream             ////
////     The stream is a allocated stream_struct and the buffer is a     ////
////     allocated array pointer; BufSize is the number of elements of   ////
////                                allocated array                      ////
//// cdRStreamReset(&Stream) reset counter and pointer of stream to      ////
////                            initial value (after cdRStreamInit)      ////

//// TO READ AND WRITE VALUES IN THE STREAM ....
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

//// IS POSSIBLE TO ASK HOW MANY VALS ARE STORED IN BUT NOT READ, HOW    ////
//// BIG IS THE STREAM AND HOW MANY VALS COULD BE STORED YET.            ////
//// cdRStreamGetNumVals(&stream)      returns the number of vals (int16)////
////                                    wainting to be read in stream    ////
//// cdRStreamPutAvilableVals(&stream) returns the number of vals (int16)////
////                                    that could be written in stream  ////
//// cdRStreamGetSize(&stream)         returns the total number of vals  ////
////                                    (int16) storable in the stream   ////
////                                    (buffer)                         ////

//// IS POSSIBLE TO BLOCK READING (APPEAR AS THERE ISN'T VALS TO BE      ////
////   READ)BUT COULD WRITE DATA IN.                                     ////
//// cdRStreamBlockRead(&stream, true)  block the reading                ////
//// cdRStreamBlockRead(&stream, false) re-enable the reading            ////
//// cdRStreamIsBlockedRead(&stream)    return true if read is blocked   ////

//// THERE IS THE POSSIBILITY TO COPY VALS FROM ONE cdRStream TO ANOTHER ////
////   cdRStream; TWO METHOD EXIST, ONE MORE SLOW BUT SAFE AND ONTHER    //// 
////   MORE FAST BUT UNSAFE BECAUSE DOESN'T DETECT SOME ERRORS           ////
//// cdRStreamTocdRStream(&stream_org, &stream_dest, numOfVals) copy     ////
////                                      numOfVals Vals from stream_org ////
////                                      stream to stream_dest stream   ////

//// SOMETIME COULD HAPPEN ERRORS AND THE STREAM FUNCTIONS RETURNS      ////
//// NEGATIVE NUMBERS; TO KNOW THE ERROR CODE HAPPENED THERE ARE        ////
//// FOLLOWING FUNCTIONS:                                               ////
//// cdRStreamGetError(&Stream) return the error value (is a bit map    ////
////                             value) and reset errors in the stream  ////


//// IS POSSIBLE STORE DIRECTLY IN THE STREAM A 8,16,32 BITS VALUES     //// 
//// ABSTRACTING BY REAL STREAM DATA SIZE USING FOLLOWING FUNCTIONS:    //// 
//// REMEMBER THAT DATA ARE ALWAYS UNSIGNED.                            ////
//// THE INSERTED VALUES ARE BUILDED AS BIGENDIAN NUMBER (MSB FIRST)    ////
//// UINT8tocdRStream(&stream,Val8bit) add a unsigned byte (8bits) to   ////
////                                     stream                         ////
//// UINT16tocdRStream(&stream,Val16bit) add unsigned word (16bits) to  ////
////                                       stream                       ////
//// UINT24tocdRStream(&stream,Val24bit) add unsigned word24b (32bits   ////
////                                       but storing only 24bits) to  ////
////                                       stream                       ////
//// UINT32tocdRStream(&stream,Val32bit) add unsigned DWord (32bits) to ////
////                                        stream                      ////

//// IS POSSIBLE EXTRACT DIRECTLY FROM STREAM A 8,16,24,32BIT VALUE     ////
//// ABSTRACTING BY REAL STREAM DATA AND STORED WITH PREVIOUS FUNCTIONS.//// 
//// THE EXTRACTED VALUE ARE BUILDED AS BIGENDIAN NUMBER (MSB FIRST)    ////
//// cdRStreamToUINT8(&stream)    extract from stream a unsigned 8bits  ////
//// cdRStreamToUINT16(&stream)   extract from stream a unsigned 16bits ////
//// cdRStreamToUINT24(&stream)   extract from stream a unsigned 24bits ////
//// cdRStreamToUINT32(&stream,&Val32bit) extract from stream a unsigned////
////                                        32bits value                ////


////////////////////////////////////////////////////////////////////////////


#ifndef __CDROUNDINGSTREAM_H_
#define __CDROUNDINGSTREAM_H_

#ifdef DOXYGEN
    #define section( YY )
#endif


#include "CD_types.h"


//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------

/*! \def CDRSTREAMER_MAX_BUFFER_SIZE
	used to indicate maximun possible size for streamBuffer; it isn't the default size, but the size beyound which will generate an error
	For this reasono this is the systema maximun allocable memory for single streamer intended like number of vals (int16 or int8) 
*/
//for 32bit uC: #define CDRSTREAMER_MAX_BUFFER_SIZE	0x3fffff
//for 16bit uC: #define CDRSTREAMER_MAX_BUFFER_SIZE	0x3fff
//for 8bit uC:
#define CDRSTREAMER_MAX_BUFFER_SIZE	0x80


/*! \def CDRSTREAM_DATA_SIZE
	used to indicate the data size inside the buffers, it could be a 8, 16, 32 bits size. 
	This is independent from platform but is dependend about the data shall be saved and managed in the buffer
*/
//#define CDRSTREAM_DATA_SIZE8
#define CDRSTREAM_DATA_SIZE16
//#define CDRSTREAM_DATA_SIZE32


/*! \def CDRSTREAM_COUNTER_SIZE
	depends upon the amount of values that you want store inside cdRStream, the better is unsigned 32bits, but for optimization and ram limitation is possible select a less dimesnion
	remember that in this type of stream, that is rounding, the amount of stored value is intended as the maximum data saved before a read operation happen (counter size is the difference
	between write and read pointer)
	for less than 256 data use CDRSTREAM_COUNTER_SIZE8, for less than 65535 data use CDRSTREAM_COUNTER_SIZE16, otherwise use CDRSTREAM_COUNTER_SIZE32
*/

#define CDRSTREAM_COUNTER_SIZE8
//#define CDRSTREAM_COUNTER_SIZE16
//#define CDRSTREAM_COUNTER_SIZE32

//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------




//depends upon the size of value (or value range) that you want store inside cdRStream, the better is the RAM/platform standar size but depends by many necessity

#ifdef CDRSTREAM_DATA_SIZE8
	typedef uint8_t CDRStream_data_t;	
	#define  CDRStream_data_MINVAL	0
	#define  CDRStream_data_MAXVAL	255
#endif //CDRSTREAM_DATA_SIZE8

#ifdef CDRSTREAM_DATA_SIZE16
	typedef uint16_t CDRStream_data_t;	
	#define  CDRStream_data_MINVAL	0
	#define  CDRStream_data_MAXVAL	65535
#endif //CDRSTREAM_DATA_SIZE16

#ifdef CDRSTREAM_DATA_SIZE32
	typedef uint32_t CDRStream_data_t;	
	#define CDRStream_data_MINVAL	0
	//#define  CDRStream_data_MAXVAL	4294967295
	#define  CDRStream_data_MAXVAL	2147483647
#endif //CDRSTREAM_DATA_SIZE32


//depends upon the amount of values that you want store inside cdRStream, the better is unsigned 32bits, but for optimization and ram limitation is possible select a less dimesnion
//remember that in this type of stream, that is rounding, the amount of stored value is intended the total amount od data stored from stream initialization (or stream reset) and 
// stream dead (or stream reset); for example a 256 byte buffer coulds read over a bilion of value, is sufficient that he lives more than 11 days and receives a value every 1ms
#ifdef CDRSTREAM_COUNTER_SIZE8
typedef uint8_t CDRStream_counters_t;
#endif

#ifdef CDRSTREAM_COUNTER_SIZE16
typedef uint16_t CDRStream_counters_t;
#endif

#ifdef CDRSTREAM_COUNTER_SIZE32
typedef uint32_t CDRStream_counters_t;
#endif


typedef struct CDRStream_struct_tag{
	//uint8_t *buffPtr;	//!< pointer to external allocated buffer
	CDRStream_data_t *buffPtr;	 //!< pointer to start location of external allocated buffer ;will be casted to 8bits
	CDRStream_data_t *buffEndPtr;//!< pointer to end of external allocated buffer ;will be casted to 8bits
	
	CDRStream_counters_t  buffSize;	//!< indicates the maximun size for buffer; is sets at the init phase
	CDRStream_counters_t  Writings;	//!< indicates the number of values written inside stream from stream inception (or its reset) 
	CDRStream_data_t *buffWRptr;	 //!< pointer to next buffer value that will be written next time
	
	CDRStream_counters_t  Readings;	//!< indicates the number of read values inside stream from stream inception (or its reset)
	CDRStream_data_t *buffRDptr;	 //!< pointer to next buffer value that will be read next time
	//CDRStream_data_t lastReadValue;  //!< used with pregetvalue and getvalue function to pass back the read value
	int errors;		//!< the bitmap for errors, every bit set to 1 indicates a different type of error
	int blockRead;		//!< boolean that indicates if read is blocked, always return 0 bytes available to be read
	//autoflush is useless in rounding buffer because not exist buffer overflow but only a logical writing overflow (CDRSTREAMERR_OVERWRITE):
	//int autoFlush;		//!< boolean that indicates if reset counter when there isn't vals to be read (all vals read)
 }CDRStream_t;

#define CDRSTREAMERR_NO_ERROR	 0x0000
#define CDRSTREAMERR_UNITIALIZED 0x0001
#define CDRSTREAMERR_READ_EMPTY	 0x0002
#define CDRSTREAMERR_OVERWRITE	 0x0004

 
 

sint_t cdRStreamInit(CDRStream_t* pStream, CDRStream_data_t* pPtrBuff, CDRStream_counters_t pBuffSize);	//!< initialize the streamer data (and structure)
 
sint_t cdRStreamReset(CDRStream_t* pStream);	//!< Restore initial streamer pointer and counter, but not changes buffer size and pointers 

sint_t cdRStreamPutVal(CDRStream_t* pStream, CDRStream_data_t pData); //!< write a byte inside the indicated streamer
 
sint_t cdRStreamPutArrayVal(CDRStream_t* pStream, CDRStream_data_t *pData, CDRStream_counters_t iNumData);	//!< write a series of vals (uitn16 array) inside the indicated streamer
 
CDRStream_counters_t cdRStreamGetNumVals(CDRStream_t* pStream);			//!< returns the number of vals wainting to be read from indicated stream
sint_t cdRStreamPregetVal(CDRStream_t* pStream, CDRStream_data_t* ptrVal );	//!< returns the next available val from the stream, but doesn't remove it from stream, so next time you read the same value
sint_t  cdRStreamGetVal(CDRStream_t* pStream, CDRStream_data_t* ptrVal);	//!< returns the next available val from the stream, and remove it from the stream
sint_t cdRStreamDiscardVal(CDRStream_t* pStream);	//!<discard actual readable val from stream so next cdRStreamGetVal reads following value

int cdRStreamGetError(CDRStream_t* pStream);		//!< return the error value (is a bit map value) and reset it in the stream

sint_t cdRStreamFlush(CDRStream_t* pStream);				//!< empty the stream buffer
CDRStream_counters_t cdRStreamPutAvilableVals(CDRStream_t* pStream);	//!< return the number of available bytes that could be written into indicated stream

sint_t cdRStreamTocdRStream(CDRStream_t* org, CDRStream_t* dest, CDRStream_counters_t numOfVals); //!< copy numOfVals Vals from org stream to dest stream

sint_t cdRStreamToUINT8(CDRStream_t* cds, uint8_t* dat );	//!< extract from stream a unsigned byte 
sint_t cdRStreamToUINT16(CDRStream_t* cds, uint16_t* dat);	//!< extract from stream a unsigned 16bits value 
sint_t cdRStreamToUINT32(CDRStream_t* cds, uint32_t* dat);	//!< extract from stream a unsigned 32bits value 

sint_t UINT32tocdRStream(CDRStream_t* cds, uint32_t pQuadByte); //!< add four val (formatted like 8bits) to stream by a unsigned 32bits (first val is MSB last is LSB of 32bits)
sint_t UINT16tocdRStream(CDRStream_t* cds, uint16_t pDiByte);   //!< add two val (formatted like 8bits) to stream by a unsigned 16bits (first val is MSB last is LSB of 16bits)
sint_t UINT8tocdRStream(CDRStream_t* cds, uint8_t pByte);       //!< add one val (formatted like 8bits) to stream by a unsigned 8bits



/*-! \def cdRStreamLastReadVal(xx)
	reads last read value from the function getval and pregetval
	\n Param xx is pStream pointer to CDstream (data structure)
*/
//#define cdRStreamLastReadVal(xx) (((CDRStream_struct*)(xx))->lastReadValue)


/*! \def cdRStreamAllReadings(xx)
	reads the number of values read from stream initialization
	\n Param xx is pStream pointer to CDstream (data structure)
*/
#define cdRStreamAllReadings(xx) (((CDRStream_struct*)(xx))->Readings)

/*! \def cdRStreamAllWritings(xx)
	reads the number of values read from stream initialization
	\n Param xx is pStream pointer to CDstream (data structure)
*/
#define cdRStreamAllWritings(xx) (((CDRStream_struct*)(xx))->Writings)


/*! \def cdRStreamGetSize(xx)
	return the size of stream buffer in number of storable Vals (int16)
	\n Param xx is pStream pointer to CDstream (data structure)
*/
#define cdRStreamGetSize(xx) (((CDRStream_struct*)(xx))->buffSize)


//void cdstreamBlockRead(CDRStream_struct* pStream, int pBlock);	//!< block or re-enable the read for stream
/*!	\def cdRStreamBlockRead(xx,yy)
	If set block = true then when ask number of readable vals it always return 0 regardless of the reality, and
	 if you attempt to read a value from stream it will generate an error.
	\n When block is removed (yy =false) then the available byte in stream to be read is the reality.
	\n When block is active (yy=true), is possible to write data in the stream without any error, but is impossible to read
	   values or ask number of readable vals in stream.
	\n Param xx is pStream pointer to CDstream (data structure)
	\n Param yy is a boolean (int) with value true or false
*/
#define cdRStreamBlockRead(xx,yy) ((CDRStream_struct*)(xx))->blockRead=(yy)


/*! \def cdRStreamIsBlockedRead(xx)
	indicates if stream reading is blocked (return true) or enabled (return false)
	\n Param xx is pStream pointer to CDstream (data structure)
*/
#define cdRStreamIsBlockedRead(xx) (((CDRStream_struct*)(xx))->blockRead)



#endif //__CDROUNDINGSTREAM_H_
