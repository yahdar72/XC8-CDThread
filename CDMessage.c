////////////////////////////////////////////////////////////////////////////
////        (C) Copyright Dario Cortese                                 ////
//// This source code may only be used by Dario Cortese licensed users  ////
//// This source code may only be distributed to Dario Cortese licensed ////
//// users.  No other use, reproduction  or distribution is permitted   ////
//// without written permission.                                        ////
////                                                                    ////
//// Author: Dario Cortese                                              ////
//// Client: Mariano Cerbone (only for Project Mangoose)                ////
//// Created on 10/08/2012                                              ////
//// Modify on 12/05/2018 to be adapted at XC8 compiler                 ////
//// File: cdmessage.c                                                  ////
//// Description:                                                       ////
//// THIS FILE HAVE ALL IMPLEMENTATION OF FUNCTION USED FOR CDMESSAGE   ////
////   MANAGING; MESSAGES ARE USED ALSO IN CDTHREAD, AND VICEVERSA      ////        
////////////////////////////////////////////////////////////////////////////

#ifndef _CDMESSAGE_C_
#define _CDMESSAGE_C_

#include "CDThread.h"


//is the system array that stores the cdmessage structure used to manage messages
#ifdef PLATFORM_BLACKFIN
section("L1_data_b")  cdmessageStruct cdmessagesSystemArray[CDTHREAD_MAX_NUM_MESSAGES];
#else
cdmessageStruct cdmessagesSystemArray[CDTHREAD_MAX_NUM_MESSAGES];
#endif
//to avoid array splitting located at the start of BANK2 gen Pourpose area (80 bytes), avery message struct is 8 bytes, so 10 messages max
//#locate cdmessagesSystemArray=0x124


//COMMENTED BECAUSE ALLOCATED IN memory_alloc.c


/*! \fn void cdmessage_initAll(void)
   \author Dario Cortese
   \date 08-08-2012
   \brief must be called by initialization main to reset and init system messages array
   \version 1.00
*/
void cdmessage_initAll(void){
   cdMessageID_t idx;
   for(idx=0; idx < CDTHREAD_MAX_NUM_MESSAGES; idx++){
      cdmessagesSystemArray[(unsigned)idx].cdthID=CDTHREADID_ERROR;      //free/available position, so not associated
      cdmessagesSystemArray[(unsigned)idx].State=CDMESSAGESTATE_DELETED;//free/available position
      //cdmessagesSystemArray[(unsigned)idx].ptrData = CDMESSAGEDATA_NODATA;   //no data associated
      cdmessagesSystemArray[(unsigned)idx].Data = 0;   //no data available
      cdmessagesSystemArray[(unsigned)idx].Info = 0;   //default value
      cdmessagesSystemArray[(unsigned)idx].NextMsgID = CDMESSAGEID_ERROR;   //no next message   
   }
}



/*! \fn cdMessageID_t cdmessage_getFreeID(void)
   \author Dario Cortese
   \date 08-08-2012
   \brief searh free/available position in system message array
   Searchs inside cdmessagesSystemArray if there is a available/free position (searhs for every position if there is a State==CDMESSAGESTATE_DELETED, 
    this means that that position is free to use)
   \return CDMESSAGEID_ERROR if doesn't find any available position, or positive number that indicate ID (array position + 1) 
   \note ONLY FOR INTERNAL USE
   \version 1.00
*/
cdMessageID_t cdmessage_getFreeID(void){
   //uses static id so next time that a new message position is requested, start from a new position and avoid to 
   //reuse position until array end is reached  
    cdThreadID_t starting_idx;
   static cdMessageID_t idx=0;
      //because cdThreadID_t now is signed, then this test must be performed to avoid error
   if(idx < 0) idx=0;
   if(idx >= CDTHREAD_MAX_NUM_MESSAGES) idx=0;
   starting_idx = idx;
   for(; idx < CDTHREAD_MAX_NUM_MESSAGES; idx++){
      if (cdmessagesSystemArray[idx].State==CDMESSAGESTATE_DELETED){
         idx++;   //so next time that reenter use next new position
         //return (idx+1); ////ID is always the real array index +1, because id=0 means unused/deleted
         return idx;
      }
   }
   //if first search, searching from last idx has no positive result (otherwire exit with return and doesn't arrives here)
   // then execute a second search starting from 0
   if (starting_idx!= 0){ //as is starting_idx>0 but more fast
   for(idx=0; idx < CDTHREAD_MAX_NUM_MESSAGES; idx++){
      if (cdmessagesSystemArray[idx].State==CDMESSAGESTATE_DELETED){
         idx++;   //so next time that reenter use next new position
         //return (idx+1); ////ID is always the real array index +1, because id=0 means unused/deleted
         return idx;
      }
   }
   } //end if (starting_idx>0)
   //if also second search has no positive result then this means that there isn't available message id free, at compiling time
   // increments CDTHREAD_MAX_NUM_MESSAGES define, or check if some message is many time allocated without remove   
   return CDMESSAGEID_ERROR; //means no available messages
} 



/*! \fn cdMessageID_t cdmessage_getArrayIdxFromID(cdMessageID_t pMsgId)
   \author Dario Cortese
   \date 09-08-2012
   \brief return the real index for cdmessagesSystemArray extracted from indicated cdMessageID (pMsgId)
   \param pMsgId is cdmessage id 
   \return -1 if thre is an error or positive number (0 or more) if all ok
   \warning DANGEROUS, because return a index that will be used to access directly an element of system message array without any checks or value casting; possibility of data corruption
   \note ONLY FOR INTERNAL USE
   \version 1.00
*/
cdMessageID_t cdmessage_getArrayIdxFromID(cdMessageID_t pMsgId){
   cdMessageID_t realidx;
   if(!cdmessage_checkValidID( pMsgId)) return -1;
   //convert pMsgId in real index for cdmessagesSystemArray
   realidx = (int)pMsgId;
   realidx--;
   return realidx;
}




/*! \fn cdMessageID_t cdmessage_new(cdThreadID_t pThreadIDdest, cdMsgInfo_t pInfoVal, cdMsgData_t pData, cdThreadID_t pThreadIDorg )
   \author Dario Cortese
   \date 09-08-2012 revised 21-03-2013  modified 2015-11-13
   \brief creates new message, and adds it to message queue of indicated thread 
   Creates new message and add it to indicated thread, if operation has success then return a valid cdMessageID, otherwise CDMESSAGEID_ERROR.
   This function search a available message in the system message array, after checks if thread id is a valid ID.
   If all ok proced to add created message to thread message queue.
   \n If no data and thread to add at this message please use cdmessage_new( pThreadID , pInfoVal, 0 , CDMESSAGEDATA_NODATA, CDTHREADID_NOTHREAD )
   \param pThreadIDdest is the thread where message will be asent (added to msg thread queue)
   \param pInfoVal is an integer that inform receiver what is the message (is a number and values must be user defined)
   \param pData is the data value associated at the message, is possible to cast at void* pointer and use it to pass more data; remember origin data must be static
   \param pThreadIDorg is the thread that send the message, if you don't have this information, then set it to CDTHREADID_NOTHREAD
   \return CDMESSAGEID_ERROR if doesn't find any available position or happen an error, or positive number that indicates new cdmessageID  
   \see cdmessage_getData
   \version 1.01
*/
cdMessageID_t cdmessage_new(cdThreadID_t pThreadIDdest, cdMsgInfo_t pInfoVal, cdMsgData_t pData, cdThreadID_t pThreadIDorg ){
   cdMessageID_t idx;
   cdMessageID_t othIdx;
   cdmessageStruct* MsgPtr;
   cdThreadID_t thRealIdx;
   cdMessageID_t appoInt;

   //check if cdthreadid is valid and get the real index of system thread array
   thRealIdx = cdthread_getArrayIdxFromID(pThreadIDdest);
   if (thRealIdx < 0) return CDMESSAGEID_ERROR;
   //check if thread is destroyed, so is impossible to sent a message and return an error
   if (cdthreadsSystemArray[(unsigned)thRealIdx].ID == CDTHREADID_ERROR ) return CDMESSAGEID_ERROR;
      
   //get the new message struct   
   idx= cdmessage_getFreeID();
   //check if not finds a available message struct
   if (idx == CDMESSAGEID_ERROR) return CDMESSAGEID_ERROR;
   //get a pointer to nmessage struct
   MsgPtr =  &cdmessagesSystemArray[(unsigned)(idx - 1)];
   // initialize values   
   MsgPtr->cdthID=pThreadIDdest;
   MsgPtr->State=CDMESSAGESTATE_NEWMSG;
   //MsgPtr->ptrData = ptrData;
   MsgPtr->Data = pData;
   MsgPtr->Info = pInfoVal;
   MsgPtr->NextMsgID = CDMESSAGEID_ERROR;   //no other messages after this
   MsgPtr->cdthSenderID = pThreadIDorg;

   //now add to the thread message queue this message
   //check if there isn't other messages on the queue, in this case....
   if(cdthreadsSystemArray[(unsigned)thRealIdx].MessagesCounter <= 0){
      //if there isn't others messages on the queue then...
      cdthreadsSystemArray[(unsigned)thRealIdx].FirstMsgID = idx;
      cdthreadsSystemArray[(unsigned)thRealIdx].LastMsgID = idx;
      cdthreadsSystemArray[(unsigned)thRealIdx].MessagesCounter=1;
   }else{ //if(cdthreadsSystemArray[thRealIdx].MessagesCounter <= 0)
      //if there is others messages on the queue then...
      othIdx = cdthreadsSystemArray[(unsigned)thRealIdx].LastMsgID;
      cdthreadsSystemArray[(unsigned)thRealIdx].LastMsgID = idx;
      //now change the NextMsgID of previouse "lastMsgID" to this message id
      //remember that
      appoInt = cdmessage_getArrayIdxFromID(othIdx); 
      //if conversion happen without errors then...
      if(appoInt >= 0){
          cdmessagesSystemArray[(unsigned)appoInt].NextMsgID = idx;
      }
      cdthreadsSystemArray[(unsigned)thRealIdx].MessagesCounter++;         
   } //end else if(cdthreadsSystemArray[thRealIdx].MessagesCounter <= 0)
   return idx;   //return the new cdMessageID added to thread
}



/*! \fn cdMessageID_t cdmessage_new_DataPtr(cdThreadID_t pThreadIDdest, cdMsgInfo_t pInfoVal, void* ptrData, cdThreadID_t pThreadIDorg )
   \author Dario Cortese
   \date 13-11-2015
   \brief creates new message with a pointer to data, and adds it to message queue of indicated thread
    As the same function of cdmessage_new(..) but convert (casting) ptrData memory pointer in a cdMsgData_t type and store it as a simple data.
   \n If no data and thread to add at this message please use cdmessage_new( pThreadID , pInfoVal, 0 , CDMESSAGEDATA_NODATA, CDTHREADID_NOTHREAD )
   \param pThreadIDdest is the thread where message will be asent (added to msg thread queue)
   \param pInfoVal is an integer that inform receiver what is the message (is a number and values must be user defined)
   \param ptrData is the pointer to the data
   \param pThreadIDorg is the thread that send the message, if you don't have this information, then set it to CDTHREADID_NOTHREAD
   \return CDMESSAGEID_ERROR if doesn't find any available position or happen an error, or positive number that indicates new cdmessageID
   \warning the origin data must be static or global otherwise the memory will be overwriten by other functions, but pointer exist until the message will be read
   \see cdmessage_getDataPointer
   \version 1.01
*/
//#inline
cdMessageID_t cdmessage_new_DataPtr(cdThreadID_t pThreadIDdest, cdMsgInfo_t pInfoVal, void* ptrData, cdThreadID_t pThreadIDorg )
{
    return cdmessage_new(pThreadIDdest, pInfoVal, (cdMsgData_t) ptrData, pThreadIDorg );
}





/*! \fn int cdmessage_deleteAllMsgWithThreadID( cdThreadID_t pThId )
   \author Dario Cortese
   \date 09-08-2012 modified 2015-11-05
   \brief delete  all messages in the message system array that have cdthID equal pThId (as is delete all messages associated at indicated thread)
   \return true if all ok otherwise return false if an error happen
   \version 1.00
*/
int cdmessage_deleteAllMsgWithThreadID( cdThreadID_t pThId ){
   cdMessageID_t idx;
   if(!cdthread_checkValidID( pThId)) return FALSE;
   for(idx=0; idx < CDTHREAD_MAX_NUM_MESSAGES; idx++){
      if(cdmessagesSystemArray[idx].cdthID== pThId){
         cdmessagesSystemArray[idx].State=CDMESSAGESTATE_DELETED;//free/available position
      }
   }
   return TRUE;
}


/*! \fn int cdmessage_deleteMsg( cdMessageID_t pMsgId )
   \author Dario Cortese
   \date 09-08-2012 modified 2015-11-05
   \brief deletes this message from messages system array and remove it from message queue of connected thread (only if is the first msg of queue)
   \param pMsgId is the cdMessageID of message that shall be deleted and removed from thread msg queue
   \return true if all ok otherwise return false if an error happen
   \version 1.00
*/
int cdmessage_deleteMsg( cdMessageID_t pMsgId ){
   cdMessageID_t idx;
   cdThreadID_t thID;
   cdThreadID_t thIdx;
   idx = cdmessage_getArrayIdxFromID(pMsgId);
   if(idx < 0) return FALSE;   //indicate an error
   cdmessagesSystemArray[(unsigned)idx].State=CDMESSAGESTATE_DELETED;//free/available position
   //remove this message from message thread queue
   //take thread id of this message
   thID = cdmessagesSystemArray[(unsigned)idx].cdthID;
   //check if this thread is valid
   thIdx = cdthread_getArrayIdxFromID(thID);
   if( thIdx >= 0){
      //proceed to remove message from msg thread queue
      //check if this thread is deleted, in this case delete every message is used for it
      if( cdthreadsSystemArray[(unsigned)thIdx].ID == CDTHREADID_ERROR ){
         cdmessage_deleteAllMsgWithThreadID( thID );
      }else{
         //if thread isn' delted then check if there is message on queue
         if(cdthreadsSystemArray[(unsigned)thIdx].MessagesCounter <= 0){
            //this is a abnormal case because this message isnt in queue so is an error, 
            // in this case sign FirstMsgID and LastMsgID as CDMESSAGEID_ERROR and search to delete every message with this threadID
            cdthreadsSystemArray[(unsigned)thIdx].FirstMsgID = CDMESSAGEID_ERROR;
            cdthreadsSystemArray[(unsigned)thIdx].LastMsgID = CDMESSAGEID_ERROR;
            cdmessage_deleteAllMsgWithThreadID( thID );
         }else{ //if(cdthreadsSystemArray[thIdx].MessagesCounter<=0)
            //if there is a message then check if this is the first, in this case procede to remove it from queue;
            //if this message isn't the first of thread msg queue than do nothing because is unmanageable error
            if( cdthreadsSystemArray[(unsigned)thIdx].FirstMsgID == pMsgId){
               //assign this NextMsgID to thread msg queue start point 
               cdthreadsSystemArray[(unsigned)thIdx].FirstMsgID = cdmessagesSystemArray[(unsigned)idx].NextMsgID;
               //decrement msg counter
               cdthreadsSystemArray[(unsigned)thIdx].MessagesCounter--;
            } 
            //if no other messages threre is in the queue then .....
            if(cdthreadsSystemArray[(unsigned)thIdx].MessagesCounter<=0){
               cdthreadsSystemArray[(unsigned)thIdx].FirstMsgID=CDMESSAGEID_ERROR;
               cdthreadsSystemArray[(unsigned)thIdx].LastMsgID=CDMESSAGEID_ERROR;
               //only for safe check if there is other messages with this thread id and delte they
               cdmessage_deleteAllMsgWithThreadID( thID );
            }
         }//end else if(cdthreadsSystemArray[thIdx].MessagesCounter<=0)
      } //end else if( cdmessagesSystemArray[thIdx].ID==CDTHREADID_ERROR )
   } //end if( thIdx >= 0)   
   return TRUE;
}



/*! \fn int cdmessage_sigMsgAsReaded( cdMessageID_t pMsgId )
   \author Dario Cortese
   \date 09-08-2012 modified 2015-11-05
   \brief sign this message as readed, if action has success then return true 
   \param pMsgId is the cdMessageID of message that will be signed as readed
   \return true if all ok otherwise return flase if an error happen
   \version 1.00
*/
int cdmessage_sigMsgAsReaded( cdMessageID_t pMsgId ){
   cdMessageID_t idx;
   idx = cdmessage_getArrayIdxFromID(pMsgId);
   if(idx < 0) return FALSE;   //indicate an error
   cdmessagesSystemArray[(unsigned)idx].State = CDMESSAGESTATE_READED;//free/available position
   return TRUE;
}



/*! \fn int cdmessage_isReaded( cdMessageID_t pMsgId )
   \author Dario Cortese
   \date 09-08-2012 modified 2015-11-05
   \brief checks if this message was readed and returns true, this means that could be only readed or yet deleted; returns false also an error happen
   \param pMsgId is the cdMessageID to check
   \return true if message was readed or deleted (readed and after deleted), otherwise false, also an error happen (ie: id ID is invalid)
   \version 1.00
*/
int cdmessage_isReaded( cdMessageID_t pMsgId ){
   cdMessageID_t idx;
   idx = cdmessage_getArrayIdxFromID(pMsgId);
   if(idx < 0) return FALSE;   //indicate an error
   if(cdmessagesSystemArray[(unsigned)idx].State == CDMESSAGESTATE_READED) return TRUE;
   if(cdmessagesSystemArray[(unsigned)idx].State == CDMESSAGESTATE_DELETED) return TRUE;
   return FALSE;
}



/*! \fn int cdmessage_isDeleted( cdMessageID_t pMsgId )
   \author Dario Cortese
   \date 09-08-2012 modified 2015-11-05
   \brief checks if this message was deleted and returns true otherwise returns false (also an error happen)
   \param pMsgId is the cdMessageID to check
   \return true if message was deleted (readed and after deleted), otherwise false, also if an error happen (ie: id ID is invalid)
   \version 1.00
*/
int cdmessage_isDeleted( cdMessageID_t pMsgId ){
   cdMessageID_t idx;
   idx = cdmessage_getArrayIdxFromID(pMsgId);
   if(idx < 0) return FALSE;   //indicate an error
   if(cdmessagesSystemArray[(unsigned)idx].State==CDMESSAGESTATE_DELETED) return TRUE;
   return FALSE;
}


/*! \fn cdThreadID_t cdmessage_getSenderThread( cdMessageID_t pMsgId ){
   \author Dario Cortese
   \date 21-03-2013 modified 2015-11-05
   \brief return the cdThreadID of Thread that has sent the message; if there isn't, or an error happens, return CDTHREADID_ERROR 
   \param pMsgId is the cdMessageID to check
   \return CDTHREADID_ERROR for error or no data, otherwise return the cdThreadID of sender thread
   \version 1.00
*/
cdThreadID_t cdmessage_getSenderThread( cdMessageID_t pMsgId ){
   cdMessageID_t idx;
   idx = cdmessage_getArrayIdxFromID(pMsgId);
   if(idx < 0) return CDTHREADID_ERROR;   //indicate an error
   return cdmessagesSystemArray[(unsigned)idx].cdthSenderID;
}



/*! \fn cdMsgData_t cdmessage_getNumData( cdMessageID_t pMsgId )
   \author Dario Cortese
   \date 09-08-2012 modified 2015-11-13
   \brief return the data value associated at the message, it could be also a memory pointer to pass more data
   \param pMsgId is the cdMessageID to check
   \return the associated data value at the message, remember that could be a memory pointer, use info to know which type of data is it
 * \see cdmessage_getDataPointer
   \version 1.00
*/
cdMsgData_t cdmessage_getData( cdMessageID_t pMsgId ){
   cdMessageID_t idx;
   idx = cdmessage_getArrayIdxFromID(pMsgId);
   if(idx < 0) return 0;   //indicate an error
   //if(cdmessagesSystemArray[idx].Data <= 0) return 0;
   return cdmessagesSystemArray[(unsigned)idx].Data;
}


/*! \fn void* cdmessage_getDataPointer( cdMessageID_t pMsgId )
   \author Dario Cortese
   \date 09-08-2012 modified 2015-11-13
   \brief return memory pointer, is the data attached to the message converted in a memory pointer; is the same think that (void *)cdmessage_getData( pMsgId )
   \param pMsgId is the cdMessageID to check
   \return the data converted in a void* , if error happen return a pointer to address 0
   \see cdmessage_getData
   \version 1.00
*/
void* cdmessage_getDataPointer( cdMessageID_t pMsgId ){
   cdMessageID_t idx;
   idx = cdmessage_getArrayIdxFromID(pMsgId);
   if(idx < 0) return CDMESSAGEDATA_NODATA;   //indicate an error
   return (void *)cdmessagesSystemArray[(unsigned)idx].Data;
 
}



/*! \fn cdMsgInfo_t cdmessage_getInfo( cdMessageID_t pMsgId )
   \author Dario Cortese
   \date 10-08-2012 modified 2015-11-05
   \brief return the info value for indicated message
   \param pMsgId is the cdMessageID to check
   \return the info value stored int indicated message; if an error happen return 0 (but 0 is a default value, not a error value)
   \version 1.00
*/
cdMsgInfo_t cdmessage_getInfo( cdMessageID_t pMsgId ){
   cdMessageID_t idx;
   idx = cdmessage_getArrayIdxFromID(pMsgId);
   if(idx < 0) return 0;   //indicate an error
   return cdmessagesSystemArray[(unsigned)idx].Info;
}




#endif //_CDMESSAGE_C_

