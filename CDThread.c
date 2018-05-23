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
//// File: cdThread.c                                                   ////
//// Description:                                                       ////
//// THIS FILE HAVE ALL IMPLEMENTATION OF FUNCTION USED FOR CDTHREAD    ////
////   MANAGING; THREADS ARE USED ALSO IN CDMESSAGE, AND VICEVERSA      ////        
////   updated 2015-11-04 to implements thread wakeup by messages       ////
////////////////////////////////////////////////////////////////////////////

#ifndef _CDTHREAD_C_
#define _CDTHREAD_C_

#include "CDThread.h"

#ifdef DOXYGEN
    #define section( YY )
#endif

//used to inform extern code which is the last executed thread function called by thread_engine
cdThreadID_t LASTCALLEDTHREADIDBYENGINE; 


//advise external code which is the last called thread by user
cdThreadID_t LASTCALLEDTHREADIDBYUSER;   



//is the system array that stores the cdthread structure used to manage threads
#ifdef PLATFORM_BLACKFIN
section("L1_data_b")  cdThreadStruct_t  cdthreadsSystemArray[CDTHREAD_MAX_NUM_THREADS];
#else
cdThreadStruct_t  cdthreadsSystemArray[CDTHREAD_MAX_NUM_THREADS];
#endif
//COMMENTED BECAUSE ALLOCATED IN memory_alloc.c

//if PIC
//to avoid array splitting located at the start of BANK1 gen Pourpose area (80 bytes), avery message struct is 10 bytes, so 8 thread max
//#locate cdthreadsSystemArray = 0xA0






//********************************************************************************************************************************
//****THREAD ENGINE FUNCTIONS*****************************************************************************************************
//********************************************************************************************************************************
 

/*! \fn cdThreadID_t cdthread_ThreadToRun(cdThreadID_t LastRunnedThread)
   \author Dario Cortese
   \date 09-08-2012 updated 2015-11-04
   \brief this function is called by cdthread_Engine() to determine which is next thread to run
   Actualy this function check only next thread enabled and signed as engine managed, and return it.
   If user want use priority or other sistem to manage thread is this function that must change.
   \param LastRunnedIndex is the index of cdthreadsSystemArray indicating previous runned thread; -1 means no previous thread executed
   \return the next index of cdthreadsSystemArray indicating which thread run; if return -1 means no thread to execute
   \warning LastRunnedIndex IS NOT a cdThreadID, so don't use it with cdThreadFunction
   \version 1.00
*/
cdThreadID_t cdthread_ThreadToRun(cdThreadID_t LastRunnedIndex){

   cdThreadID_t thidx;
   cdThreadStruct_t* p;
   
   //if last values is
   if(LastRunnedIndex < 0)
       LastRunnedIndex=-1; //only for ensurance
   else if(LastRunnedIndex >= CDTHREAD_MAX_NUM_THREADS )
       LastRunnedIndex=-1; //only for ensurance
   
   //check if lastRunnedThread is the last of available thread
   if( LastRunnedIndex < (CDTHREAD_MAX_NUM_THREADS - 1) ){
      for(thidx= LastRunnedIndex+1; thidx < CDTHREAD_MAX_NUM_THREADS; thidx++){
          p = &cdthreadsSystemArray[thidx];
         //checks if actual analyzed thread is thread_engine managed
         if(cdthread_isEngineManagedID( p->ID)){
             //checks if actual analyzed thread is enabled, also disabled if destroyed so useless checks also if destroyed
             if( p->Priority != CDTHREADPRIORITY_DISABLED ){
                   return thidx;
             }else if (p->Wakeup == CDTHREADWAKEUP_BYMESSAGE){
             //checks if there is a message and wakeup by message is sets
                if (p->MessagesCounter > 0 ){
                    p->Priority = CDTHREADPRIORITY_ENABLED;   //enable the thread
                    p->Wakeup = CDTHREADWAKEUP_DISABLED; //disable the wait message
                    return thidx;
                }
             }
         }// end if(cdthread_isEngineManagedID( cdthreadsSystemArray[thidx].ID))
      } //end for(thidx= LastRunnedIndex +1; thidx < CDTHREAD_MAX_NUM_THREADS; thidx++)
   } //end if( LastRunnedIndex < (CDTHREAD_MAX_NUM_THREADS - 1))
   
   //if arrived here means that previous search (from LastRunnedIndex to end of cdthread array) has no success so...
   //if the last runnedindex isn't the first elements of cdthread array then execute a search from start of cdthread array to the LastRunnedIndex  
   
      //restart from first (first idx is 0 that is eaqul to threadid 1) to last runnedIndex
      for(thidx=0 ; thidx <= LastRunnedIndex ; thidx++){
          p = &cdthreadsSystemArray[thidx];
         //checks if actual analyzed thread is thread_engine managed
         if(cdthread_isEngineManagedID( p->ID)){
             //checks if actual analyzed thread is enabled
             if( p->Priority != CDTHREADPRIORITY_DISABLED ){
               return thidx; 
             }else if (p->Wakeup == CDTHREADWAKEUP_BYMESSAGE){
             //checks if there is a message and wakeup by message is sets
                if (p->MessagesCounter > 0 ){
                    p->Priority = CDTHREADPRIORITY_ENABLED;   //enable the thread
                    p->Wakeup = CDTHREADWAKEUP_DISABLED; //disable the wait message
                    return thidx;
                }
            }
         }
      } //end for(thidx= LastRunnedIndex +1; thidx < CDTHREAD_MAX_NUM_THREADS; thidx++)
   

   //if arrives here then means that no active, and engine managed, thread is finded so returns an error index
   return -1;
} 


/*! \fn sint_t cdthread_Engine(void)
   \author Dario Cortese
   \date 09-08-2012 modified 2015-11-04
   \brief this function is the engine for thread and must be called continuosly
   every time this function is called, it call a single different active thread function, and after exit.
   \n If there isn't active thread return 0.
   \n If there is almost one thread active with a function callback then return 1 if she exit with a positive exit code, otherwise return -1
   \n To determine which thread run when it is called, it call a cdthread_ThreadToRun function; if there is to implement a new sistem to manage
    thread then that function could be changed. 
   \return 0=not called function, 1 called function with exitcode positive (ok), -1 called function with exit code negative (error) 
   \warning this function must be placed in main infinite loop
*/
sint_t cdthread_Engine(void){
   static cdThreadID_t LastThIndex;
   cdThreadID_t nextThIndex;
   sint_t retVal;   //exit value: 0=not called function, 1 called function with exitcode positive (ok), -1 called function with exit code negative (error)

   cdThreadStruct_t* thptr;
   cdThreadID_t cdthid;
   cdtreadFunctionType ptrFunc;
   int isFirstTime;
   int exitState;
   
   nextThIndex = cdthread_ThreadToRun(LastThIndex);
   retVal = 0; //used to indicate if nothing run
   if((nextThIndex >=0 )&&(nextThIndex < CDTHREAD_MAX_NUM_THREADS)){
      thptr = &cdthreadsSystemArray[(unsigned)nextThIndex];
      //Following test isn't implementable in CCS compiler, so removed
      //check if call back function exist;
      //if(thptr->cdtreadFunction != CDTHREADFUNCTION_NOFUNCTION){
         //call the function with 
         cdthid = thptr->ID;
         ptrFunc = thptr->cdtreadFunction;
         isFirstTime = thptr->isTheFirstTime;
         //call the callback function
         exitState = thptr->LastExitState;
         exitState = (*ptrFunc)( isFirstTime, cdthid, exitState);
         thptr->isTheFirstTime = FALSE;   //after now sign this thread as "not the first time" that is called
         //store actual exitcode for this thread
         thptr->LastExitState = exitState;
         LASTCALLEDTHREADIDBYENGINE = cdthid;   //advise external code which is the last called thread by engine
         //check if exit state is positive (ok exit code) or negative (error exit code) and change properly the engine exit code
         if( exitState < 0 ){
            retVal = -1;   //indicates that function called by engine returned a negative value (normally an error exit code)
         }else{
            retVal = 1;
         } 
      //}
   }
   //store index for next call
   LastThIndex = nextThIndex;
   return retVal;
}




/*! \fn sint_t cdthread_UserCallThreadFunction(cdThreadID_t pThId, int* exitCode)
   \author Dario Cortese
   \date 10-08-2012 modified 2015-11-04
   \brief call function of thread, if thread is enabled and id is good (no errors); return true if called, otherwise false
   checks if threadID is a valid ID, if thread is enables, if function pointer isn't CDTHREADFUNCTION_NOFUNCTION (no associated function to thread),
    after call the pointed thread function.
   \n When call function read threadId, if is the first time that run, and pass this information to called function.
   \n When called function returns, the returned value is stored in thread LastExitState and thread id pubblicated in LASTCALLEDTHREADIDBYUSER; the returned
   value is also returned outside this function by exitCode parameter.
   \n If thread function is called then this function return true otherwise return false 
   \param pThId is cdthread (id) 
   \param exitCode is returned value by called thread_function
   \return true if function was called, false if for error, or simply because thread is disabled, the function was not called
*/
#ifdef PLATFORM_BLACKFIN
section ("L1_code")
#endif
sint_t cdthread_UserCallThreadFunction(cdThreadID_t pThId, int* exitCode){
   cdThreadID_t realidx;
   cdThreadStruct_t* thptr;
   cdThreadID_t cdthid;
   cdtreadFunctionType ptrFunc;
   int isFirstTime;
   int exitState;
      
   //convert pThId in real index for cdthreadsSystemArray
   realidx = cdthread_getArrayIdxFromID(pThId);
   //if an error happened then return false
   if( realidx<0 ) return FALSE; 
   thptr = &cdthreadsSystemArray[(unsigned)realidx];
   //check if thread was destroyed
   if( thptr->ID == CDTHREADID_ERROR) return FALSE;
   //check if disabled
   if( thptr->Priority == CDTHREADPRIORITY_DISABLED){
       if (thptr->Wakeup == CDTHREADWAKEUP_BYMESSAGE){
       //checks if there is a message and wakeup by message is sets
          if (thptr->MessagesCounter > 0 ){
             thptr->Priority = CDTHREADPRIORITY_ENABLED;   //enable the thread
          }else{
             //thread disabled so.... 
             return FALSE;
          }
       }
   }
   //following test isn't possible to implement in CCS compiler
   //check if call back function exist
   //if(thptr->cdtreadFunction != CDTHREADFUNCTION_NOFUNCTION){
      //call the function with 
      cdthid = thptr->ID;
      ptrFunc = thptr->cdtreadFunction;
      isFirstTime = thptr->isTheFirstTime;
      //call the callback function
      exitState = thptr->LastExitState;
      exitState = ptrFunc( isFirstTime, cdthid, exitState);
      thptr->isTheFirstTime = FALSE;   //after now sign this thread as "not the first time" that is called
      //store actual exitcode for this thread
      thptr->LastExitState = exitState;
      LASTCALLEDTHREADIDBYUSER = cdthid;   //advise external code which is the last called thread by engine
      *exitCode = exitState;
      return TRUE;
   //}
   //if arrives here means no called function so singnaling an error
   //return FALSE;
}




//********************************************************************************************************************************
//****THREAD  FUNCTIONS***********************************************************************************************************
//********************************************************************************************************************************
//a is a boolena than indicates if is the first time that thread is called
//b it the thread structure used to manage the thread that call this function (trhead function)
//c is the previous ecit state
sint_t CDTHREADFUNCTION_NOFUNCTION(int a, cdThreadID_t b , int c)
{
    //do nothing
    return -1;
}


/*! \fn void cdthread_initAll(void)
   \author Dario Cortese
   \date 10-08-2012 modified 2015-11-04
   \brief must be called by initialization main to reset and init system threads array
   \version 1.00
*/
void cdthread_initAll(void){
   uint8_t idx;
   for(idx=0; idx < CDTHREAD_MAX_NUM_THREADS; idx++){
      cdthreadsSystemArray[idx].ID=CDTHREADID_ERROR;   //free/available position
      cdthreadsSystemArray[idx].Priority=CDTHREADPRIORITY_DISABLED;
      cdthreadsSystemArray[idx].Wakeup = CDTHREADWAKEUP_DISABLED;
      cdthreadsSystemArray[idx].cdtreadFunction = CDTHREADFUNCTION_NOFUNCTION;   //no associated function
      cdthreadsSystemArray[idx].LastExitState =0;         //default value
      cdthreadsSystemArray[idx].isTheFirstTime = TRUE;   //so when vill be activated that condition will be true
      cdthreadsSystemArray[idx].MessagesCounter=0;      //no messages
      cdthreadsSystemArray[idx].FirstMsgID = CDMESSAGEID_ERROR;      //no first message 
      cdthreadsSystemArray[idx].LastMsgID = CDMESSAGEID_ERROR;      //no last message
   }
}


/*! \fn cdThreadID_t cdthread_getFreeID(void)
   \author Dario Cortese
   \date 08-08-2012 updated 2015-11-04
   \brief searchs free/available position in system thread array
   Searchs inside cdthreadsSystemArray if there is a available/free position (searhs for every position if there is a ID==CDTHREADID_ERROR, 
    this means that that position is free to use)
   \return CDTHREADID_ERROR if doesn't find any available position, or positive number that indicate ID (array position + 1) 
   \note ONLY FOR INTERNAL USE
   \version 1.00
*/
cdThreadID_t cdthread_getFreeID(void){
   //uses static id so next time that a new thread position is requested, start from a new position and avoid to 
   //reuse position until array end is reached  
   cdThreadID_t starting_idx;
   static cdThreadID_t idx=0;
   //because cdThreadID_t now is signed, then this test must be performed to avoid error
   if(idx < 0) idx=0;
   if(idx >= CDTHREAD_MAX_NUM_THREADS) idx=0;
   starting_idx = idx;
   for(; idx < CDTHREAD_MAX_NUM_THREADS; idx++){
      if(cdthreadsSystemArray[(unsigned)idx].ID == CDTHREADID_ERROR){
         idx++;   //so next time that reenter use next new position
         //return (idx +1);   //ID is always the real array index +1, because id=0 means unused/free
         return idx;
      }
   }
   //if first search, searching from last idx has no positive result (otherwire exit with return and doesn't arrives here)
   // then execute a second search starting from 0
   if (starting_idx != 0){ //as is starting_idx>0 but more fast
       for(idx=0; idx < starting_idx; idx++){
          if(cdthreadsSystemArray[(unsigned)idx].ID == CDTHREADID_ERROR){
         idx++;   //so next time that reenter use next new position
         //return (idx +1);   //ID is always the real array index +1, because id=0 means unused/free
         return idx;
      }
   }
   } //end if (starting_idx>0)
   //if also second search has no positive result then this means that there isn't available trread id free, at compiling time
   // increments CDTHREAD_MAX_NUM_THREADS define, or check if some thread is many time allocated without destroy   
   return CDTHREADID_ERROR; //means no available threads
} 


/*! \fn cdThreadID_t cdthread_getArrayIdxFromID(cdThreadID_t pThId)
   \author Dario Cortese
   \date 09-08-2012 updated 2015-11-04
   \brief return the real index for cdthreadsSystemArray extracted from indicated cdThreadID_t (pThId)
   \param pThId is cdthread id 
   \return -1 if thre is an error or positive number (0 or more) if all ok
   \note please don't use it to access thread struct data, prefere to use all other function with higher abstraction from internal data structure
   \warning DANGEROUS, because return a index that will be used to access directly an element of system thread array without any checks or value casting; possibility of data corruption
   \note ONLY FOR INTERNAL USE
   \version 1.00
*/
cdThreadID_t cdthread_getArrayIdxFromID(cdThreadID_t pThId){
   cdThreadID_t realidx;
   if(!cdthread_checkValidID( pThId)) return -1;

   //convert pThId in real index for cdthreadsSystemArray
   if(cdthread_isUserManagedID( pThId )){
      realidx = (int)pThId * (-1);
      realidx--;
   }else{
      realidx = (int)pThId;
      realidx--;
   }
   return realidx;
}



/*! \fn cdthreadStruct* cdthread_getPointerToStruct(cdThreadID_t pThId){
   \author Dario Cortese
   \date 10-08-2012 updated 2015-11-04
   \brief return a cdthreadStruct type pointer directly to inndicated element of internal thread struct array; if error happen return a pointer to 0x0000000 memory address
   \param pThId is cdthread (id) to destroy
   \return a pointer to internal system array of stream struct; if an error happen return a pointer to 0x0000000 address 
   \note please don't use it, prefere to use all other function with higher abstraction from internal data structure
   \warning VERY DANGEROUS, because point directly to an element of private system thread array without any checks or value casting; possibility of data corruption
   \note ONLY FOR INTERNAL USE
   \version 1.00
*/
cdThreadStruct_t* cdthread_getPointerToStruct(cdThreadID_t pThId){
   cdThreadID_t realidx;
   //convert pThId in real index for cdthreadsSystemArray
   realidx = cdthread_getArrayIdxFromID(pThId);
   //if an error happened then return false
   if( realidx<0 ) return (cdThreadStruct_t*)0x00000000; 
   return &cdthreadsSystemArray[(unsigned)realidx];
} 








/*! \fn int cdthread_new(cdThreadID* pThId, cdtreadFunctionType ptrFunction)
   \author Dario Cortese
   \date 08-08-2012, mod 06-Apr-2013,  updated 2015-11-04
   \brief creates a new thread for Thread_Engine; after creation the thread is disabled, doesn't have messages and the firsttime is set to true
   Checks if passed id (pThId) is unused/ previously destroyed thread id and return an error if id already used.
   \n Searchs inside cdthreadsSystemArray if there is a available/free position (searhs for every position if there is a ID==CDTHREADID_ERROR, 
    this means that that position is free to use), after initialize finded thread with ID = returned id value, priority as disabled thread,
    cdtreadFunction as passed ptrFunction, LastExitState as 0, isTheFirstTime as true and message counter =0
   \n At the end, if all ok, return the new id by pThId parameter and true as responce.
   \param pThId is cdthread (id) to verify if was free (previously destroyed) and also is the returned new thread (id); if error happens this is set to CDTHREADID_ERROR 
   \param ptrFunction is the function pointer to a function that is the called thread_function when thread run; if no function pass CDTHREADFUNCTION_NOFUNCTION
   \return true if new thread created or false if happened a problem as thread id also used (not destroyed) or noone id is free for a new thread
   \version 1.01
*/
int cdthread_new(cdThreadID_t* pThId, cdtreadFunctionType ptrFunction){
//prev define was: cdThreadID_t cdthread_new( cdtreadFunctionType ptrFunction){
   //cdThreadID_t idx;
   cdThreadStruct_t* thptr;
   cdThreadID_t LocID;
   LocID = *pThId;  
   
   if(!cdthread_isDestroyed(LocID)) return FALSE;   //the thread id point to an existing thread (not previously destroyed and also actually used)
   LocID = cdthread_getFreeID();
   if(LocID == CDTHREADID_ERROR) { *pThId = CDTHREADID_ERROR; return FALSE; }  //means error, no available 
   //the system array index is equal to |id|-1
   thptr = &cdthreadsSystemArray[(unsigned)( LocID - 1 )];
   thptr->ID = LocID;
   thptr->Priority=CDTHREADPRIORITY_DISABLED;
   thptr->Wakeup=CDTHREADWAKEUP_DISABLED;
   thptr->cdtreadFunction = ptrFunction;
   thptr->LastExitState =0;
   thptr->isTheFirstTime = TRUE;
   thptr->MessagesCounter=0;
   thptr->FirstMsgID = CDMESSAGEID_ERROR;
   thptr->LastMsgID = CDMESSAGEID_ERROR;      
   return TRUE;
} 


/*! \fn int cdthread_newUserManaged(cdThreadID_t* pThId, cdtreadFunctionType ptrFunction) 
   \author Dario Cortese
   \date 08-08-2012, mod 06-Apr-2013, updated 2015-11-04
   \brief creates a new USER managed thread (not managed by Thread Engine); after creation the thread is disabled, doesn't have messages and the first time is set to true
   Checks if passed id (pThId) is unused/ previously destroyed thread id and return an error if id already used.
   \n Searchs inside cdthreadsSystemArray if there is a available/free position (searhs for every position if there is a ID==CDTHREADID_ERROR, 
    this means that that position is free to use).
   \n If find a free position initialize that thread with ID = returned id value, priority as disabled thread, cdtreadFunction as passed ptrFunction,
    LastExitState as 0, isTheFirstTime as true and message counter =0
   \n At the end, if all ok, return the new id by pThId parameter and true as responce.  
   \param pThId is cdthread (id) to verify if was free (previously destroyed) and also is the returned new thread (id); if error happens this is set to CDTHREADID_ERROR 
   \param ptrFunction is the function pointer to a function that is the called thread_function when thread run; if no function pass CDTHREADFUNCTION_NOFUNCTION
   \return true if new thread created or false if happened a problem as thread id also used (not destroyed) or noone id is free for a new thread
   \version 1.01
*/
int cdthread_newUserManaged(cdThreadID_t* pThId, cdtreadFunctionType ptrFunction){
//prev was: cdThreadID_t cdthread_newUserManaged(cdtreadFunctionType ptrFunction)
   int ires;
   int previdx;
   ires = cdthread_new( pThId, ptrFunction);
   if(!ires) return FALSE ; //return an error, cause must be passed id already used (not previously destroied) or unavailable thread
   if(*pThId == CDTHREADID_ERROR) return FALSE;   //means error, no available 
   //the system array real id is equal to |id|-1
   previdx =  *pThId - 1;
   //the user managed threads have negative id to distinguish the thread engine managed that have positive id;
   // remeber that the system array real id is equal to |id|-1 (|id| is the id without sign)
   *pThId *= - 1;
   cdthreadsSystemArray[(unsigned)previdx].ID= *pThId;
   return TRUE;
} 


/*! \fn int cdthread_changeFunction(cdThreadID_t pThId, cdtreadFunctionType ptrFunction)
   \author Dario Cortese
   \date 18-09-2012 updated 2015-11-04
   \brief it changes only called function by indicated thread without change any other thread fields
   This method changes the called thread_function by thread, without change messages, last exit code, firsttime and the other fields.
   \n This is very usefull, and power, system to manage differents object that will be controlled by only one thread; a very simple examples could be a DAC, of 
    three different available, that must be selected and drived while other two are stopped.
   \n Changing called function is possible to drive and manage any of three DAC without performance lack generated typically by series of if-else or switch instruction  
   \param pThId is cdthread (id) to change called thread_function
   \param ptrFunction is the function pointer to a function that is the called thread_function when thread run; if no function pass CDTHREADFUNCTION_NOFUNCTION
   \return true if action has succes
   \version 1.00
*/
int cdthread_changeFunction(cdThreadID_t pThId, cdtreadFunctionType ptrFunction){
   cdThreadID_t realidx;
   realidx = cdthread_getArrayIdxFromID(pThId);
   //if an error happened then return false
   if( realidx<0 ) return FALSE; 
   //checks id thread is destroyed and if destroyed return false
   if(cdthreadsSystemArray[(unsigned)realidx].ID == CDTHREADID_ERROR) return FALSE;
   //sets the new callback thread function to thread
   cdthreadsSystemArray[(unsigned)realidx].cdtreadFunction = ptrFunction;
   return TRUE;
} 



/*! \fn int cdthread_isFunction(cdThreadID_t pThId, cdtreadFunctionType ptrFunction)
   \author Dario Cortese
   \date 06-04-2013 updated 2015-11-04
   \brief checks if called function by indicated thread (pThId) is the same passed (ptrFunction)
   \param pThId is cdthread (id) to checks called thread_function
   \param ptrFunction is the function pointer to a function that theorically will be called by thread; if no function pass CDTHREADFUNCTION_NOFUNCTION
   \return true actually thread function is the same indcated (ptrFunction), return false if also id invalid or thread destroyed 
   \version 1.00
*/
int cdthread_isFunction(cdThreadID_t pThId, cdtreadFunctionType ptrFunction){
   cdThreadID_t realidx;
   realidx = cdthread_getArrayIdxFromID(pThId);
   //if an error happened then return false
   if( realidx<0 ) return FALSE; 
   //checks id thread is destroyed and if destroyed return false
   if(cdthreadsSystemArray[(unsigned)realidx].ID == CDTHREADID_ERROR) return FALSE;
   //checks the callback thread function is different by indicates
   if(cdthreadsSystemArray[(unsigned)realidx].cdtreadFunction != ptrFunction) return FALSE;
   return TRUE;
} 



/*! \fn int cdthread_DestroyThread(cdThreadID_t pThId)
   \author Dario Cortese updated 2015-11-04
   \date 09-08-2012, last mod 06-Apr-2013
   \brief destroy the indicated thread (memory are free for another new thread), and every message associated
   sign inside cdthreadsSystemArray that indicated position is unusable/deleted/free (ID==CDTHREADID_ERROR), after
   checks if there are messages for indicated thread and destroy all they.
   Lastest action is to set passed pThId to CDTHREADID_ERROR, so is unusable
   \param pThId is cdthread (id) to destroy
   \return false if a error accurred, or true if indicated thread has been deleted 
   \version 1.01 
*/
int cdthread_DestroyThread(cdThreadID_t* pThId){
   cdThreadID_t realidx;
   //check if isn't a valid id
   //next line unusefull because cdthread_getArrayIdxFromID check it and return -1 in case of error 
   //if( !cdthread_checkValidID( *pThId ) return false;         //error for unusable id

   //convert pThId in real index for cdthreadsSystemArray
   realidx = cdthread_getArrayIdxFromID( *pThId);
   //if an error happened then return false
   if( realidx<0 ) return FALSE; 
   //sign this thread as unusable/overvwritable   
   cdthreadsSystemArray[(unsigned)realidx].ID= CDTHREADID_ERROR; //delete this thread and sign as unusable/overwritable
   cdthreadsSystemArray[(unsigned)realidx].Priority=CDTHREADPRIORITY_DISABLED;
   cdthreadsSystemArray[(unsigned)realidx].Wakeup = CDTHREADWAKEUP_DISABLED;
   //destroy all messages associated to this thread
   cdmessage_deleteAllMsgWithThreadID( *pThId );
   *pThId = CDTHREADID_ERROR;   //force passed thread id to an unexisting thread id
   return TRUE;
} 






/*! \fn int cdthread_IsDestroyed(cdThreadID_t pThId)
   \author Dario Cortese
   \date 06-03-2013 updated 2015-11-04
   \brief checks if passed id is destroyed (invalid pThId or pThId signed as destroyed) and if it is then return true
   \param pThId is cdthread (id) to check
   \return true if pThId is invalid or thread indicated by pThId is signed as destroyed
   \version 1.00
*/
int cdthread_isDestroyed(cdThreadID_t pThId){
   cdThreadID_t realidx;
   //convert pThId in real index for cdthreadsSystemArray
   realidx = cdthread_getArrayIdxFromID(pThId);
   //if an error happened then return true to indicate that thread doesn't exist
   if( realidx<0 ) return TRUE; 
   //checks if thread was destroyed
   if( cdthreadsSystemArray[(unsigned)realidx].ID == CDTHREADID_ERROR) return TRUE;
   //checks if pointed thread by pThId is different by pThId; this operation have a very limited utility because happens only if allocated thread is a different type thread, so removed
   //if( cdthreadsSystemArray[realidx].ID != pThId) return true;
   return FALSE;   //to indicates that thread exist
}



/*! \fn int cdthread_reassign(cdThreadID_t* pThId, cdtreadFunctionType ptrFunction)
 *  \author Dario Cortese
 *  \date 2016-05-22
 *  \brief destroy allocated thread and assign a new one
 *  this function test if the actual id is free, if not then destroy that id and thread, 
 *  after creates a new id and assign indicated thread function 
 *  \param pThId is pointer to the cdthread variable that contain the (id)
 *  \param ptrFunction is the function pointer to a function that is the called thread_function when thread run; if no function pass CDTHREADFUNCTION_NOFUNCTION
 *  \return true if new thread created or false if happened a problem as thread id also used (not destroyed) or noone id is free for a new thread
 *  \version 1.00
*/
int cdthread_reassign(cdThreadID_t* pThId, cdtreadFunctionType ptrFunction){
      if ( ! cdthread_isDestroyed( *pThId))
          cdthread_DestroyThread( pThId );
                  
    return cdthread_new(pThId, ptrFunction);
}







/*! \fn int cdthread_signAsFirtsTime(cdThreadID_t pThId)
   \author Dario Cortese
   \date 09-08-2012 updated 2015-11-04
   \brief set firsttime flag as true for indicated thread
   sign inside cdthreadsSystemArray at indicated position that isTheFirstTime = true, so the called function will reinitialize itself .
   \param pThId is cdthread (id) to sign as firsttime
   \return false if a error accurred, or true if indicated thread has been signed as firsttime 
   \version 1.00
*/
int cdthread_signAsFirtsTime(cdThreadID_t pThId){
   cdThreadID_t realidx;
   //convert pThId in real index for cdthreadsSystemArray
   realidx = cdthread_getArrayIdxFromID(pThId);
   //if an error happened then return false
   if( realidx<0 ) return FALSE; 
   //checks id thread is destroyed and if destroyed return false
   if(cdthreadsSystemArray[(unsigned)realidx].ID == CDTHREADID_ERROR) return FALSE;

   cdthreadsSystemArray[(unsigned)realidx].isTheFirstTime= TRUE; 
   return TRUE;
} 



/*! \fn int cdthread_Enable(cdThreadID_t pThId, int pPriority)
   \author Dario Cortese
   \date 09-08-2012 updated 2015-11-04
   \brief enable indicated Thread with indicated priority 
   sign inside cdthreadsSystemArray at indicated position that Priority = pPriority.
   \n If pPriority is under 1 or is CDTHREADPRIORITY_DISABLED will be changed into equal to 1
   \param pThId is cdthread (id) to enable
   \param pPriority is the priority level that must be higher than 1 (more higher = more priority)
   \return false if a error accurred, or true if indicated thread has been disabled 
   \version 1.00
*/
int cdthread_Enable(cdThreadID_t pThId, int pPriority){
   cdThreadID_t realidx;
   //convert pThId in real index for cdthreadsSystemArray
   realidx = cdthread_getArrayIdxFromID(pThId);
   //if an error happened then return false
   if( realidx<0 ) return FALSE; 
   //ensure that pRiority is a valid priority to indicates enabled thread
   if((pPriority == CDTHREADPRIORITY_DISABLED)||(pPriority < 1)) pPriority=CDTHREADPRIORITY_ENABLED;
   //cdthreadsSystemArray[(unsigned)realidx].Wakeup = CDTHREADWAKEUP_DISABLED;
   cdthreadsSystemArray[(unsigned)realidx].Priority= pPriority; 
   cdthreadsSystemArray[(unsigned)realidx].Wakeup = CDTHREADWAKEUP_DISABLED;
   return TRUE;
} 



/*! \fn int cdthread_Disable(cdThreadID_t pThId)
   \author Dario Cortese
   \date 09-08-2012 updated 2015-11-04
   \brief disable indicated Thread  and stop to wait a message for wakeup
   sign inside cdthreadsSystemArray at indicated position that Priority = CDTHREADPRIORITY_DISABLED
   \param pThId is cdthread (id) to disable
   \return false if a error accurred, or true if indicated thread has been disabled 
   \version 1.00
*/
int cdthread_Disable(cdThreadID_t pThId){
   cdThreadID_t realidx;
   //convert pThId in real index for cdthreadsSystemArray
   realidx = cdthread_getArrayIdxFromID(pThId);
   //if an error happened then return false
   if( realidx<0 ) return FALSE; 
   cdthreadsSystemArray[(unsigned)realidx].Priority= CDTHREADPRIORITY_DISABLED; //disable this thread
   cdthreadsSystemArray[(unsigned)realidx].Wakeup = CDTHREADWAKEUP_DISABLED;
   return TRUE;
}


/*! \fn int cdthread_WaitMessage(cdThreadID_t pThId)
   \author Dario Cortese
   \date 05-11-2015 updated 2015-11-04
   \brief disable indicated Thread and set to wait a message; the thread will be called when a message arrive (wakeup one time) to manage the message, but not enabled
   sign inside cdthreadsSystemArray at indicated position that Priority = CDTHREADPRIORITY_DISABLED and Wakeup = CDTHREADWAKEUP_BYMESSAGE
   \param pThId is cdthread (id) to disable
   \return false if a error accurred, or true if indicated thread has been disabled
   \version 1.00
*/
int cdthread_WaitMessage(cdThreadID_t pThId){
   cdThreadID_t realidx;
   //convert pThId in real index for cdthreadsSystemArray
   realidx = cdthread_getArrayIdxFromID(pThId);
   //if an error happened then return false
   if( realidx<0 ) return FALSE;
   cdthreadsSystemArray[(unsigned)realidx].Priority= CDTHREADPRIORITY_DISABLED; //disable this thread
   cdthreadsSystemArray[(unsigned)realidx].Wakeup = CDTHREADWAKEUP_BYMESSAGE;
   return TRUE;
}





/*! \fn int cdthread_isEnabled(cdThreadID_t pThId)
   \author Dario Cortese
   \date 10-08-2012 updated 2015-11-04
   \brief checks if indicated Thread is enabled (priority over 0) and return true if enabled 
   checks inside cdthreadsSystemArray at indicated position that Priority != CDTHREADPRIORITY_DISABLED
   \param pThId is cdthread (id) to disable
   \return true if enabled or false if disabled or an error happened
   \version 1.00
*/
int cdthread_isEnabled(cdThreadID_t pThId){
   cdThreadID_t realidx;
   //convert pThId in real index for cdthreadsSystemArray
   realidx = cdthread_getArrayIdxFromID(pThId);
   //if an error happened then return false
   if( realidx<0 ) return FALSE; 
   if( cdthreadsSystemArray[(unsigned)realidx].Priority == CDTHREADPRIORITY_DISABLED){
      return FALSE;
   }
   return TRUE;
} 


/*! \fn int cdthread_getPriority(cdThreadID_t pThId)
   \author Dario Cortese
   \date 10-08-2012 updated 2015-11-04
   \brief return the actual pripority of thread
   check inside cdthreadsSystemArray at indicated position the Priority 
   \param pThId is cdthread (id) to disable
   \return priority (0=disabled) or -1 if an error happened
   \version 1.00
*/
int cdthread_getPriority(cdThreadID_t pThId){
   cdThreadID_t realidx;
   //convert pThId in real index for cdthreadsSystemArray
   realidx = cdthread_getArrayIdxFromID(pThId);
   //if an error happened then return false
   if( realidx<0 ) return -1; 
   return cdthreadsSystemArray[(unsigned)realidx].Priority;
} 


/*! \fn int cdthread_getPrevExitStatus(cdThreadID_t pThId)
   \author Dario Cortese
   \date 10-08-2012 updated 2015-11-04
   \brief return the last exit state; if hasn't a previous exit state (first time executed) return 0. if an error happen return -1
   \param pThId is cdthread (id) to check
   \return -1 for an error, if is the first time return  0(default val)
   \note is good practice don't use -1 in the possible exit state, use negative exit state for errors, and positive values for normal exit
   \version 1.00
*/
int cdthread_getPrevExitStatus(cdThreadID_t pThId){
   cdThreadID_t realidx;
   //convert pThId in real index for cdthreadsSystemArray
   realidx = cdthread_getArrayIdxFromID(pThId);
   //if an error happened then return false
   if( realidx<0 ) return -1; 
   return cdthreadsSystemArray[(unsigned)realidx].LastExitState; 
} 





/*! \fn int cdthread_isThrereMessages(cdThreadID_t pThId)
   \author Dario Cortese
   \date 09-08-2012 updated 2015-11-04
   \brief check if indicated thread has almost a message
   \param pThId is cdthread (id) to check
   \return true if there is a message, otherwise return false if there isn't messages or an error accurred
   \version 1.00
*/
int cdthread_isThrereMessages(cdThreadID_t pThId){
   cdThreadID_t realidx;
   //convert pThId in real index for cdthreadsSystemArray
   realidx = cdthread_getArrayIdxFromID(pThId);
   //if an error happened then return false
   if( realidx<0 ) return FALSE; 
   if( cdthreadsSystemArray[(unsigned)realidx].MessagesCounter> 0) return TRUE; 
   return FALSE;
} 




/*! \fn cdMessageID_t cdthread_getMessage(cdThreadID_t pThId)
   \author Dario Cortese
   \date 09-08-2012 updated 2015-11-04
   \brief return the first message (id) on msg queue of indicated thread, and sign it as readed; if there isn't or an error occours then return CDMESSAGEID_ERROR
   \param pThId is cdthread (id) to check
   \return the cdmessageid of available message, otherwise return CDMESSAGEID_ERROR if there isn't messages or an error accurred
   \note this function doesn't remove message from queue, and so if you doesn't remove it by cdthread_removeMessage(thid), next time reread this message
   \version 1.00
*/
cdMessageID_t cdthread_getMessage(cdThreadID_t pThId){
   cdThreadID_t realidx;
   cdMessageID_t msgid;
   //convert pThId in real index for cdthreadsSystemArray
   realidx = cdthread_getArrayIdxFromID(pThId);
   //if an error happened then return false
   if( realidx<0 ) return CDMESSAGEID_ERROR; 
   if( cdthreadsSystemArray[(unsigned)realidx].MessagesCounter> 0){
      msgid = cdthreadsSystemArray[(unsigned)realidx].FirstMsgID;
      //sign indicated message as readed, but doesnt remove it from queue
      cdmessage_sigMsgAsReaded( msgid);
      return msgid;
   }
   return CDMESSAGEID_ERROR;
} 


/*! \fn int cdthread_removeMessage(cdThreadID_t pThId)
   \author Dario Cortese
   \date 09-08-2012 updated 2015-11-04
   \brief remove first message from thread msg queue and return true if action has success, otherwise return false
   \param pThId is cdthread (id) to check
   \return true if remove msg from thread queue or false if an error happen
   \version 1.00
*/
int cdthread_removeMessage(cdThreadID_t pThId){
   cdThreadID_t realidx;
   cdMessageID_t msgid;
   //convert pThId in real index for cdthreadsSystemArray
   realidx = cdthread_getArrayIdxFromID(pThId);
   //if an error happened then return false
   if( realidx<0 ) return FALSE; 
   if( cdthreadsSystemArray[(unsigned)realidx].MessagesCounter> 0){
      msgid = cdthreadsSystemArray[(unsigned)realidx].FirstMsgID;
      return cdmessage_deleteMsg( msgid );
   }
   return CDMESSAGEID_ERROR;
} 



#endif //_CDTHREAD_C_

