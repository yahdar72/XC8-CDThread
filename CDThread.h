////////////////////////////////////////////////////////////////////////////
////        (C) Copyright Dario Cortese                                 ////
//// This source code may only be used by Dario Cortese licensed users  ////
//// This source code may only be distributed to Dario Cortese licensed ////
//// users.  No other use, reproduction  or distribution is permitted   ////
//// without written permission.                                        ////
////                                                                    ////
//// Author: Dario Cortese                                              ////
//// Client: Mariano Cerbone (only for Project Mangoose)                ////
//// Created on 02/08/2012                                              ////
//// Modify on 12/05/2018 to be adapted at XC8 compiler                 ////
//// File: cdthread.h                                                   ////
//// Description:                                                       ////
////    This file has the variable, structure and function definition   ////
////     to manage a generic cdthread ().                               ////
////                                                                    ////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//////////////////////////////THREADS///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//// THIS TYPE OF THREAD IS A STRUCTURE THAT MANAGES COOPERATIVE THREADS.///
//// EVERY THREAD COULD BE ENABLED OR DISABLED AND, TODAY, DOESN'T HAVE ////
////   PRIORITY.                                                        ////
//// NO PRIORITY MEANS THAT EVERY THREAD ENABLED, IS EXECUTED BY THREAD ////
////   ENGINE IN SEQUENCE.                                              ////
//// THE RULE IS THAT THREAD ENGINE,WHEN CALLED, EXECUTE IN SEQUENCE    ////
////   ONLY ACTIVE, AND NON USER, THREADS.                              ////
//// Remember that thread engine manager function, cdthread_Engine(),   ////
//// must be called continuosly, so place it in main infinite loop....  ////
//// REMEMBER TO INITIALIZE THREAD SYSTEM ARRAY BEFORE TO CALL FIRST    ////
////   TIME A THREAD OR THE ENGINE MANAGER FUNCTION                     ////
////                                                                    ////
////   void main(void){                                                 ////
////     .......                                                        ////
////     cdthread_initAll();                                            ////
////     .......                                                        ////
////     while(true){                                                   ////
////       .......                                                      ////
////       cdthread_Engine();                                           ////
////       .......                                                      ////
////     }                                                              ////
////   }                                                                ////
////                                                                    ////
//// THE THREAD ENGINE WHEN MEETS A ACTIVE, AND ENGINE MANAGED, THREAD  ////
////   CALL THE FUNCTION INDICATED BY THREAD_FUNCTION POINTER, AFTER    ////
////   WAIT THE ENDS OF THAT FUNCTION (COLLABORATIVE SYSTEM) AND STORE  ////
////   RETURN VALUE.                                                    ////
//// AFTER THAT ANALYZE ITS THREAD LIST AND EXECUTES THE NEXT ACTIVE    ////
////   THREAD....AND SO ON INFINITELY.                                  ////
//// The thread structure has a Priority field but today is used only   ////
////   as boolean to enable/disable thread, but in the CDThread.h       ////
////   there is a function  cdthread_ThreadToRun(....) that can be      ////
////   rewrited to manage priority or change the rule that run threads  ////
////                                                                    ////
//// TO INDICATES THE NUMBER OF MAXIMUN THREAD ALLOWED BY SYSTEM, and   ////
////   so the memory usage allocated for thread (approx. 32bytes on     ////
////   32bits system for every thread), there is a define               ////
////    CDTHREAD_MAX_NUM_THREADS that must be changed by necessity      ////
////                                                                    ////
//// THE THREADS MUST BE ASSIGNED TO THREAD ENGINE OR COULD BE USER     ////
////   THREAD, AND IN THIS CASE THE THREAD ENGINE SKIPS THEIR ANALYSYS  ////
////   AND EXECUTION.                                                   ////
//// To create and register a new thread for engine use...              ////
////   cdThreadID_t ThreadID= CDTHREADID_NOTHREAD;                      ////
////   int allOk; //to know if callfunction was called                  ////
////   allOk = cdthread_new(&ThreadID, functionForThread);              ////
//// To create and register a new thread for user (not used in engine)  ////
////   allOk = cdthread_newUserManaged(&ThreadID, FunctionForThread);   ////
////                                                                    ////
//// This functions could return a invalid cdthreadID to indicates error////
////   use the cdthread_isErrorID(ThreadID) macro to check this invalid ////
////   id, it returns true if id is invalid                             ////
////                                                                    ////
//// To changes only called function by indicated thread without change ////
////   any other thread fields use...                                   ////
////   int allOk; //to know if callfunction was called                  ////
////   allOk = cdthread_changeFunction(ThreadID, FunctionForThread);    ////
//// To call by user code a user managed thread (or a engined thread)   ////
////   use...                                                           ////
////   int fctExitCode;   //used to know exit code of callback function ////
////   int allOk; //to know if callfunction was called                  ////
////   allOk = cdthread_UserCallThreadFunction(ThreadID, &fctExitCode); ////
////                                                                    ////
//// To know if a thread is user managed or engined use...              ////
////   cdthread_isEngineManagedID(ThreadID) ,returns true if engined or ////
////   cdthread_isUserManagedID(ThreadID) ,returns true if user managed ////
////                                                                    ////
//// To destroy/unregister thread, and release memory for another one.. ////
////   cdthread_DestroyThread( &ThreadID);                              ////
////                                                                    ////
//// To check if ThreadID refers to a destroyed thread use..            ////
////   int isKo; //to know if destroyed, true=destroyed, false=exist    ////
////   isKo = cdthread_isDestroyed(ThreadID);                           ////
////                                                                    ////
//// To check if ThreadID exist (valid and thread not destroyed)        ////
////   int allOk; //to know if function exist; true=exist               ////
////   allOk = cdthread_Exist(ThreadID);                                ////
////                                                                    ////
//// HOW SAY BEFORE THERE IS THE POSSIBILITY TO ENABLE, OR DISABLE, THE ////
////   THREAD. WHEN THREAD IS DISABLED CAN'T BE EXECUTED BY ENGINE OR   ////
////   CALLED BY cdthread_UserCallThreadFunction(...) FUNCTION.         ////
//// To enable thread, or change its priority, use ...                  ////
////   int Priority = CDTHREADPRIORITY_ENABLED; //lower enabled priority////
////   allOk = cdthread_Enable(ThreadID, Priority);                     ////
//// To disable thread use ...                                          ////
////   allOk = cdthread_Disable(cdThreadID_t pThId);                    ////
////                                                                    ////
//// To know if thread is enabled use the function....                  ////
////  cdthread_isEnabled(ThreadID) that return a true if enabled        ////
//// To read the thread priority use...                                 ////
////   Priority = cdthread_getPriority(ThreadID);                       ////
////                                                                    ////
////                                                                    ////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//////////////////////////THREAD MESSAGES///////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//// EVERY THREAD COULD RECEIVE MESSAGES, EVERY MESSAGE HAVE A UNIQUE ID////
////   AN INFO VALUE AND A MEMORY POINTER TO OPTIONAL DATA.             ////
//// THE REAL SIZE (in byte, word, etc) IS UNKNOWED, THE ONLY           ////
////   INFORMATION ABOUT SIZE IS THE NUMBER OF LOGICAL DATA STORED; THIS////
////   MEANS THAT REAL TYPE, SIZE, ORDER, ETC, OF DATA MUST BE KNOWED   ////
////   BY RECEIVER.                                                     ////
//// there is a define that indicates the maximun system message number ////
////    CDTHREAD_MAX_NUM_MESSAGES that must be changed by necessity     ////
////                                                                    ////
//// REMEMBER TO INITIALIZE MESSAGES SYSTEM ARRAY BEFORE TO CALL FIRST  ////
////   TIME A THREAD OR THE ENGINE MANAGER FUNCTION                     ////
////                                                                    ////
////   void main(void){                                                 ////
////     .......                                                        ////
////     cdmessage_initAll();                                           ////
////     cdthread_initAll();                                            ////
////     .......                                                        ////
////     while(true){                                                   ////
////       .......                                                      ////
////       cdthread_Engine();                                           ////
////       .......                                                      ////
////     }                                                              ////
////   }                                                                ////
////                                                                    ////
//// TO CREATE A NEW MESSAGE FOR A SPECIFIC THREAD USE...               ////
////   cdMessageID msgID;                                               ////
////   int info= 1;   //this is the code for message used inside        ////
////                    //the thread to determine which message is      ////
////   int NumData=10;   //this is the number (logical, not the size in ////
////                    //byte) of data                                 ////
//// IS VERY IMPORTANT THAT DATA DON'T STAY IN STACK OR VOLATILE ZONE   ////
//// BECAUSE DATA AREN'T COPIED IN THE MESSAGE, BUT ONLY A POINTER IS   ////
//// STORED IN THE MESSAGE                                              ////
////   static int msgData[10]={1,2,3,4,5,6,7,8,9,10}; //the data must be////
////                    //primitive, structure or other, the important  ////
////                    //is that receiver know what is it              ////
////   msgID = cdmessage_new(Threadid,Info,NumData,(void*) msgData );   ////
//// THE MESSAGE ID COULD BE INVALID, THIS IS USED TO ADVISE THAT AN    ////
//// ERRROR WAS HAPPENED.                                               ////
//// To detect invalid msgID use macro cdmessage_isErrorID(msgID) that  ////
////   returns true if id is invalid/error                              ////
////                                                                    ////
//// TO DETECT IF THERE IS A MESSAGE INSIDE THE TREAD FUNCTION USE...   ////
////   cdthread_isThrereMessages(ThisThread) that returns true if there ////
////                                         is one or more messages    ////
//// TO READ THE MESSAGE ID, AND SO READS ITS DATA, INSIDE THE THREAD   ////
////   FUNCTION use...                                                  ////
////   cdMessageID msgID;                                               ////
////   msgID = cdthread_getMessage(ThisThread);                         ////
//// IT SIGN ALSO THE MESSAGE AS READED, SO EXTERNALLY COULD BE DETECTED////
//// To detect externally to thread function the happened read message..////
////   cdmessage_isReaded(msgID); //msgID returned when created the msg,////
////                              //this function return true if message////
////                              //was readed or already deleted       ////
////                                                                    ////
//// TO DELETE A READED MESSAGE INSIDE THE THREAD FUNCTION USE....      ////
//// allOk = cdthread_removeMessage(ThisThread);                        ////
//// To detect externally to thread function the message deletion use   ////
//// cdmessage_isDeleted(msgID); //return true if message was deleted   ////
////                                                                    ////
//// IS POSSIBLE DELETE A MESSAGE ALSO OUTSIDE THE THREAD FUNCTION USING////
//// allOk = cdmessage_deleteMsg(msgID);  //this delete message from    ////
////                                      // message system array and   ////
////                                      // also remove message from   ////
////                                      // connected thread queue     ////
//// Following function delete all messages in the queue of a thread    ////
////   allOk = cdmessage_deleteAllMsgWithThreadID(ThreadID);            ////
//// or inside thread function....                                      ////
////   allOk = cdmessage_deleteAllMsgWithThreadID(ThisThread);          ////
////                                                                    ////
//// TO READ AND USE MESSAGE FIELD, INSIDE THREAD FUNCTION USE FOLLOWING////
//// FUNCTIONS.                                                         ////
//// Remember that msgID is get by cdthread_getMessage(ThisThread)      ////
//// To read info field, that is used to identify message, use....      ////
////   int info;                                                        ////
////   info = cdmessage_getInfo(msgID);                                 ////
//// To read number of logical data use....                             ////
////   int NumData;                                                     ////
////   NumData = cdmessage_getNumData(msgID);                           ////
//// To read the memory pointer to message data (remeber that receiver  ////
////   must know the type or structure of data, for this is used info   ////
////   and number of data fields) use....                               ////
////   int* array;   //suppose that message data is an int array        ////
////   array = (int*)cdmessage_getDataPointer(msgID);                   ////
////                                                                    ////
////                                                                    ////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
///////////////////THREAD CALLBACK FUNCTION/////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//// THE THREAD_FUNCTION (CALLBACK FUNCTION CALLED BY THREAD) HAS TWO   ////
////   PARAMETERS, THE FIRST IS A BOOLEAN (INT), AND IF IT IS TRUE THAN ////
////   INDICATES THAT IS THE FIRST TIME THAT FUNCTION IS CALLED.        ////
//// IN THIS MODE THE THREAD FUNCTION COULD INITIALIZE THEIR VARIABLES, ////
////   MEMORY AND OBJECT.                                               ////
//// To force this variable to be true next time thread function will be////
////   called, so it reinitialize itself, use...                        ////
////   allok = cdthread_signAsFirtsTime(ThreadID);                      ////
////                                                                    ////
//// THE SECOND PARAMETER IS A cdThreadID THAT IS THE ID OF THE THREAD  ////
////   CALLER, THIS IS USEFULL WHEN USE THREAD FUNCTIONS INSIDE CODE.   ////
////   is good practice call this parameter "ThisThread" as in example  ////
////                                                                    ////
//// int ThreadFunctionSample(int firstTime, cdThreadID_t ThisThread, ..////
////                          int prevExitCode){                        ////
//// ....}                                                             ////
////                                                                    ////
//// THE THREAD FUNCTION MUST RETURN A VALUE THAT IS EXIT STATUS; THIS  ////
////   PREVIOUS STATUS MUST BE READED BY THREAD FUNCTION, SO IT CAN     ////
////   ITS PREVIOUS EXIT STATUS.                                        ////
//// To read previous exit status, or last exit status of a thread, use ////
////   int lastExitValue;                                               ////
////   lastExitValue = cdthread_getPrevExitStatus(ThreadID);            ////
//// or if is in the thread callback function is more right write...    ////
////   lastExitValue = cdthread_getPrevExitStatus(ThisThread);          ////
////                                                                    ////
//// PLEASE SEE THE CALLBACK FUNCTION EXAMPLE AT THE END OF THIS        ////
////  INTRODUCTION TO SEE HOW WRITE IT                                  ////
////////////////////////////////////////////////////////////////////////////
///////////////////TIMER FUNCTIONS AND MACRO////////////////////////////////
////////////////////////////////////////////////////////////////////////////
//// TO USE TIMER IS ENOUGH CREATE A cdTimerID VARIABLE AND ASK IN THE  ////
////   CODE WHEN THAT VARIABLE IS EXPIRED; SEE FOLLOWING EXAMPLE        ////
//// TO SET THAT VARIABLE WITH EXPIRATION VALUE YOU CAN USE             ////
////   cdtimers_setupTicks( num of ticks)                               ////
//// THAT CALCULATES NUM OF TICKS THAT MUST SCORE FROM CALL AT cdtimers_////
////   setupTicks(ticks) TO RETAIN TIMER ELAPSED.                       ////
////   cdtimers_setupus(us), cdtimers_setupms(ms), cdtimers_setup_s(s)  ////  
//// THAT INDICATES TIME IN MICROSECONDS, MILLISECONDS AND SECONDS      ////
//// TO KNOW HOW MANY TICKS THERE ARE IN A 1ms THERE IS A DEFINE:       ////
////   CDTIMER_1ms_TICKS(num of ms).                                    ////
//// ANOTHER DEFINE INDICATES HOW MANY MICROSECONDS THERE ARE IN A TICK ////
////   AND THIS DEFINE IS:                                              ////
////   CDTIMER_us_FOR_TICK(num of ticks)                                ////
//// TO CHECK IF TIME IS ELAPSED AND TIMER EXPIRED USE:                 ////
////   cdtimer_isExpired(cdTimerID) and cdtimer_isNotExpired(cdTimerID) ////
////                                                                    ////
//// GENERIC CDTIMER EXAMPLE:                                           ////
//// cdTimerID myTimer1, myTimer2, myTimer3, myTimer4;                  ////
//// myTimer1 = cdtimer_setupTicks(20);  //timer value for wait 20ticks ////
//// myTimer2 = cdtimer_setup_s(1);     //timer value for wait 1second  ////
//// myTimer3 = cdtimer_setupms(10);     //timer value for wait 10ms    ////
//// myTimer4 = cdtimer_setupus(50);     //timer value for wait 50us    ////
//// while( cdtimer_isNotExpired(myTimer1) ){                           ////
////   ....                                                             ////
////   if( cdtimer_isExpired(myTimer2){                                 ////
////     ....                                                           //// 
////   }                                                                ////
////   ....                                                             ////
//// }                                                                  ////
////                                                                    ////
//// WHEN CDTIMER IS INSIDE A THREAD REMEBER TO INDICATES CDTIMERID     ////
////   STATIC.                                                          ////
//// THREAD CDTIMER EXAMPLE:                                            ////
////   static cdTimerID myTimer1=0;                                     ////
////   if( myTimer1!=0 ) goto LABEL_timer_here;                         //// 
////   //place here code to execute before start timer                  ////
////   myTimer1 = cdtimer_setupms(1); //timer value for wait 1ms        ////
//// LABEL_timer_here:                                                  //// 
////   if( cdtimer_isNotExpired(myTimer1) ) return 0;                   ////
////   //place here code to execute when timer expired                  ////
////     ....                                                           //// 
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////


/* FOLLOWIN THREAD FUNCTION ONLY FOR EXAMPLE 
int CDThreadFunction_Sample(int firstTime, cdThreadID_t ThisThread, int prevExitCode){
   cdMessageID_t actMsg;
   int msgInfo;
   int thLastStatus;
   
   //-**********************************************
   //from here setup code 
   if(firstTime){
      //if this function is called first time then needs to set its variables;
      //setup code
   
      //HERE INSERT CODE FOR SETUP AND INITIALIZATION
      
   }//end else if(firstTime)
   
   //-**********************************************
   //from here messages manager 
   //first think check if there is messages
   if(cdthread_isThrereMessages(ThisThread)){
      //read message
      actMsg= cdthread_getMessage(ThisThread); 
      
      msgInfo = cdmessage_getInfo(actMsg);
      switch(msgInfo){
         case 1:
            //HERE INSERT CODE FOR MESSAGE 1
            break;
         case 2:
            //HERE INSERT CODE FOR MESSAGE 2
            break;
            .
            .
         default:
            //HERE INSERT CODE FOR UNKNOW MESSAGES
      . }
      
      
      //when finisched to manage message destroy it
      cdthread_removeMessage(ThisThread);      
   }//end if(cdthread_isThrereMessages(ThisThread))
   
   //-**********************************************
   //from here jump table...
   switch(prevExitCode){
      case 0: goto LABEL_START;  break;
      case 1: goto LABEL_STATE1; break;
      case 2: goto LABEL_STATE2; break;
      case 3: goto LABEL_STATE3; break;
      .
      .
      .
      .
   }

   //-**********************************************
   //from here run time code (no setup code, no reads messages) 
LABEL_START:
   .... //code for prev exit state 0 (default)
   return 1;   
LABEL_STATE1:
   .... //code for prev exit state 1 (default)
   return 2;
LABEL_STATE2:
   .... //code for prev exit state 2
   return 3;   
LABEL_STATE3:
   .... //code for prev exit state 3
   ....
   return n;
   .... //code for prev exit state n
       
   //HERE INSERT CODE FOR RUNTIME; TYPICALLY USES LAST STATUS TO DETERMINE OPERATION TO DO (PROVATE STATE MACHINE)
   return 0;
}
*/
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

#ifndef _CDTHREAD_H_
#define _CDTHREAD_H_

#include "CD_types.h"

/*! \def CDTHREAD_MAX_NUM_THREADS
   used to indicates the absolute maximun number of thread that the system could have (thread array size)
*/
#define  CDTHREAD_MAX_NUM_THREADS   5

/*! \def CDTHREAD_MAX_NUM_MESSAGES
   used to indicates the absolute maximun number of messages that the system could have (thread array size)
*/
#define  CDTHREAD_MAX_NUM_MESSAGES   8

//cdMessageID_t, always signed, but on 8bit platform means 127 messages maximun, on 16bit platform means 32767 messages maximun and so on
typedef sint_t cdMessageID_t;   //!< cdmessage id type, used to store id message that is used by every function
typedef sint_t cdMsgInfo_t;     //!< message info type, used to store info value associated to the message
typedef sint16_t cdMsgData_t;   //!< message data, used to store the a data (because this could be a memory pointer (casting to void*) this must be almost the size of a pointer [16bit for less than 65535 address space or a 32bit])

//on 8 bit platform means 127 threads
//on 16bit platororm means 32767 threads
typedef sint_t cdThreadID_t ;   //!< cdthread id type, used to store id message that is used by every function


//*********************************************************************
//*************messages************************************************
//*********************************************************************




typedef struct cdMessageStruct_tag{
   cdThreadID_t cdthID ;    //!< a number that identify unequivocal to which thread is associated this message
   int State;           //!< 2=new message, 1=readed but do not delete, 0=was managed and so is deletable 
   //void* ptrData;        //!< pointer to memory where is stored data for this message
   cdMsgData_t Data;        //!< the data of the message (because could be a memory pointer (casting to void*) this must be almost the size of a memory pointer [16bit for less than 65535 address space or a 32bit])
   cdMsgInfo_t Info;            //!< a information value, is optional and is a user value used to inform receiver
   cdMessageID_t NextMsgID;    //!< Next message ID, if pointer is 0 or less (-1 and so on) means no previous message
   cdThreadID_t cdthSenderID ;    //!< a number that identify unequivocal to which thread is sender of this message
}cdmessageStruct;



/*!   \var cdmessageStruct cdmessagesSystemArray
    is the system array that stores the cdmessage structure used to manage messages
*/
extern cdmessageStruct cdmessagesSystemArray[CDTHREAD_MAX_NUM_MESSAGES];





/*! \def CDMESSAGEDATA_NODATA
   used in cdmessageStruct.ptrData to assign a no available pointer to data
*/
#define CDMESSAGEDATA_NODATA      (void*)0x00000000



/*! \def CDMESSAGESTATE_NEWMSG
   used in cdmessageStruct.State to indicates that message is a new message (never readed)
*/
#define CDMESSAGESTATE_NEWMSG  2

/*! \def CDMESSAGESTATE_READED
   used in cdmessageStruct.State to indicates that message is readed but not deleted
*/
#define CDMESSAGESTATE_READED  1

/*! \def CDMESSAGESTATE_DELETED
   used in cdmessageStruct.State to indicates that message is readed and destroyed, so is reusable and overvritable
*/
#define CDMESSAGESTATE_DELETED 0



/*! \def CDMESSAGESTATE_DELETED
   used in cdmessageStruct.NextMsgID to indicates that there isn't other messages after this, and also is used...
   \n used in every function that return cdMessageID to indicates an invalid ID and at the same time an error
*/
#define CDMESSAGEID_ERROR    0



/*! \def cdmessage_isErrorID(x)
   macro used to check if cdMessageID 'x' is an error id (==CDMESSAGEID_ERROR)
*/
#define cdmessage_isErrorID( YxY )      ( YxY == CDMESSAGEID_ERROR)


/*! \def cdmessage_checkValidID(x)
   checks if cdMessageID 'x' is a valid ID, this means that isn't a CDMESSAGEID_ERROR and is inside from 0 to CDTHREAD_MAX_NUM_MESSAGES.
   \n return true if is valid otherwisw return false
*/
#define cdmessage_checkValidID( YxY )   (( YxY !=CDMESSAGEID_ERROR)&&( YxY >=0)&&( YxY <=CDTHREAD_MAX_NUM_MESSAGES))



//******************************************************************************************************

void cdmessage_initAll(void); //!< must be called by initialization main to reset and init system messages array

cdMessageID_t cdmessage_getFreeID(); //!< searh free/available position in system message array
cdMessageID_t cdmessage_getArrayIdxFromID(cdMessageID_t pMsgId); //!< returns the real index for cdmessagesSystemArray extracted from indicated cdMessageID (pMsgId)

cdMessageID_t cdmessage_new(cdThreadID_t pThreadIDdest, cdMsgInfo_t pInfoVal, cdMsgData_t pData, cdThreadID_t pThreadIDorg ); //!< creates new message, and adds it to message queue of indicated thread
//#inline cdMessageID_t cdmessage_new_DataPtr(cdThreadID_t pThreadIDdest, cdMsgInfo_t pInfoVal, void* ptrData, cdThreadID_t pThreadIDorg ); //!< creates new message with a pointer to data, and adds it to message queue of indicated thread
cdMessageID_t cdmessage_new_DataPtr(cdThreadID_t pThreadIDdest, cdMsgInfo_t pInfoVal, void* ptrData, cdThreadID_t pThreadIDorg ); //!< creates new message with a pointer to data, and adds it to message queue of indicated thread
int cdmessage_deleteMsg( cdMessageID_t pMsgId );           //!< delete  this message from messages system array and remove it from message queue of connected thread (only if is the first msg of queue)


int cdmessage_deleteAllMsgWithThreadID( cdThreadID_t pThId ); //!< delete  all messages in the message system array that have cdthID equal pThId

int cdmessage_sigMsgAsReaded( cdMessageID_t pMsgId );   //!< sign this message as readed, if action has success then return true

int cdmessage_isReaded( cdMessageID_t pMsgId );         //!< checks if this message was readed and returns true, this means that could be only readed or yet deleted; returns false also an error happen
int cdmessage_isDeleted( cdMessageID_t pMsgId );        //!< check if this message was deleted and return true otherwise return false (also an error happen)
cdMsgData_t cdmessage_getData( cdMessageID_t pMsgId );       //!< return the number of logical data attached to message; if there isn't, or an error happens, return 0
void* cdmessage_getDataPointer( cdMessageID_t pMsgId ); //!< return pointer to memory where is the data attached to indicated message; if there isn't, or an error happens, return CDMESSAGEDATA_NODATA 
cdMsgInfo_t cdmessage_getInfo( cdMessageID_t pMsgId );          //!< return the info value for indicated message
cdThreadID_t cdmessage_getSenderThread( cdMessageID_t pMsgId ); //!< return the cdThreadID of Thread that has sent the message; if there isn't, or an error happens, return CDTHREADID_ERROR 


//*********************************************************************
//*************thread *************************************************
//*********************************************************************

//! function pointer to the thread code, return the exit state (int) and has two param, first is 'first time' boolean (int), second is a cdThreadID of calling thread
typedef sint_t (*cdtreadFunctionType)(int, cdThreadID_t, int); //

//this is struct type to manages thread
typedef struct cdthreadStruct_tag{
   cdThreadID_t ID;                //!< a number that identify unequivocal this thread structure; used in all cdthread function. 0=unused/free, positive ID managed Thread Engine, negative not managed by Engine and used only by user
   int Priority;                //!< 0 means disabled, 1 or more indicates that is enabled and the priority level (higher value-> higher priority) 
   int Wakeup;                  //!< 0 means disabled, 1 = enable thread when MessagesCounter become >0, 2 or more to be defined in the future
   cdtreadFunctionType cdtreadFunction;//!< function pointer to the thread code, return the exit state (int) and has two param, first is 'first time' boolean (int), second is a cdThreadID of calling thread
   int LastExitState;            //!< indicates the returned value when exiting from *cdtreadFunction; usefull when reenter to know last state.
   int isTheFirstTime;             //!< boolean, if true indicates that is the first time that the cdtreadFunction is called.
   cdMessageID_t MessagesCounter;   //!< indicates the number of messages available for this thread; 0 means no msg. The queue is LIFO type
   cdMessageID_t FirstMsgID;         //!< msgid of first message; if -1 means no messages 
   cdMessageID_t LastMsgID;         //!< msgid of last available message; if -1 means no messages 
} cdThreadStruct_t;



/*!   \var cdthreadStruct cdthreadsSystemArray
    is the system array that stores the cdthread structure used to manage threads
*/
extern cdThreadStruct_t cdthreadsSystemArray[CDTHREAD_MAX_NUM_THREADS];





/*!   \var cdThreadID_t LASTCALLEDTHREADIDBYENGINE
    used to inform extern code which is the last executed thread function called by thread_engine
*/
extern cdThreadID_t LASTCALLEDTHREADIDBYENGINE; 

/*!   \var cdThreadID_t LASTCALLEDTHREADIDBYUSER
    advise external code which is the last called thread by user
*/
extern cdThreadID_t LASTCALLEDTHREADIDBYUSER;   





/*! \def CDTHREADID_ERROR
   used to indicates that cdThreadID that have this value are unusable ID, so this value is also used an error in functions that return cdThreadID
*/
#define CDTHREADID_ERROR   0
#define CDTHREADID_NOTHREAD   0

/*! \def cdthread_isEngineManagedID(x)
   is a macro that check if cdThreadID 'x' is Engine managed cdthread; return true if is Engine managed
*/
#define cdthread_isEngineManagedID( YxY )   ( YxY > CDTHREADID_ERROR)

/*! \def cdthread_isUserManagedID(x)
   is a macro that check if cdThreadID 'x' is user managed cdthread; return true if is user managed
*/
#define cdthread_isUserManagedID( YxY )      ( YxY < CDTHREADID_ERROR)

/*! \def cdthread_isErrorID(x)
   is a macro that check if cdThreadID 'x' is no usable cdThreadID (errorID) ; return true if is unusable 
*/
#define cdthread_isErrorID( YxY )      ( YxY == CDTHREADID_ERROR)

/*! \def cdthread_checkValidID(x)
   checks if cdThredID 'x' is a valid ID, this means that isn't a CDTHREADID_ERROR and is inside from -CDTHREAD_MAX_NUM_THREADS to CDTHREAD_MAX_NUM_THREADS.
   \n return true if is valid otherwis return false
*/
#define cdthread_checkValidID( YxY)   (( YxY !=CDTHREADID_ERROR)&&( YxY <=CDTHREAD_MAX_NUM_THREADS)&&( YxY >=(CDTHREAD_MAX_NUM_THREADS*(-1))))




/*! \def CDTHREADFUNCTION_NOFUNCTION
   used to indicates that there is associated function to pointer; it's a 0 address 
   used also as parameter in function like cdthread_new() and cdthread_newUserManaged to pass a CDTHREADFUNCTION_NOFUNCTION address for tread function
*/
//#define CDTHREADFUNCTION_NOFUNCTION      (cdtreadFunctionType)0x00000000
sint_t CDTHREADFUNCTION_NOFUNCTION(int a, cdThreadID_t b , int c);


/*! \def CDTHREADPRIORITY_DISABLED
   indicates the value for cdthreadStruct.Priority to indicates at the system that tread is disabled
*/
#define CDTHREADPRIORITY_DISABLED      0

/*! \def CDTHREADPRIORITY_ENABLED
   indicates the default value for cdthreadStruct.Priority to indicates at the system that tread is enabled
*/
#define CDTHREADPRIORITY_ENABLED      1


/*! \def CDTHREADWAKEUP_DISABLED
   indicates the default value for cdthreadStruct.Wakeup to indicates at the system that tread doesn't use wakeup system
*/
#define CDTHREADWAKEUP_DISABLED      0

/*! \def CDTHREADWAKEUP_BYMESSAGE
   indicates the default value for cdthreadStruct.Wakeup to indicates at the system that tread is waiting a message to be enabled and run
*/
#define CDTHREADWAKEUP_BYMESSAGE     1




//******************************************************************************************************

void cdthread_initAll(void); //!< must be called by initialization main to reset and init system threads array

cdThreadID_t cdthread_getFreeID(void);                           //!< searchs free/available position in system thread array
cdThreadStruct_t* cdthread_getPointerToStruct(cdThreadID_t pThId); //!< returns a cdthreadStruct type pointer directly to inndicated element of internal thread struct array; in case of error return a pointer to 0x0000000
cdThreadID_t cdthread_getArrayIdxFromID(cdThreadID_t pThId);      //!< return the index for cdthreadsSystemArray extracted from indicated cdThreadID (pThId)

int cdthread_new(cdThreadID_t* pThId, cdtreadFunctionType ptrFunction);    //!< creates a new thread for Thread_Engine; after creation the thread is disabled, doesn't have messages and the firsttime is set to true
int cdthread_newUserManaged(cdThreadID_t* pThId, cdtreadFunctionType ptrFunction); //!<creates a new USER managed thread (not managed by Thread Engine); after creation the thread is disabled, doesn't have messages and the first time is set to true

int cdthread_DestroyThread(cdThreadID_t* pThId);                        //!< destroy the indicated thread (memory are free for another new thread), and every message associated at that thread
int cdthread_isDestroyed(cdThreadID_t pThId);         //!<checks if passed id is destroyed (invalid pThId or pThId signed as destroyed) and if it is then return true
#define cdthread_Exist( pThId )            (!cdthread_isDestroyed( pThId ))   //!<checks if passed id is valid and indicated thread is not destroyed 

int cdthread_changeFunction(cdThreadID_t pThId, cdtreadFunctionType ptrFunction); //!< changes only called function by indicated thread without change any other thread fields
int cdthread_isFunction(cdThreadID_t pThId, cdtreadFunctionType ptrFunction);   //!<checks if called function by indicated thread (pThId) is the same passed (ptrFunction)
int  cdthread_reassign(cdThreadID_t* pThId, cdtreadFunctionType ptrFunction);    //!<destroys allocated thread and assign a new one

int cdthread_signAsFirtsTime(cdThreadID_t pThId);       //!<set firsttime flag as true for indicated thread
int cdthread_Enable(cdThreadID_t pThId, int pPriority); //!< enable indicated Thread with indicated priority 
int  cdthread_Disable(cdThreadID_t pThId);                 //!< disable indicated Thread and stop to wait a message for wakeup
int  cdthread_WaitMessage(cdThreadID_t pThId);   //!<disable indicated Thread and set to wait a message; the thread will be called when a message arrive (wakeup one time) to manage the message, but not enabled

int cdthread_isEnabled(cdThreadID_t pThId);          //!< checks if indicated Thread is enabled (priority over 0) and return true if enabled 
int cdthread_getPriority(cdThreadID_t pThId);          //!< return the actual pripority of thread (0=disable), or -1 for error
int cdthread_getPrevExitStatus(cdThreadID_t pThId);  //!< return the last exit state; if hasn't a previous exit state (first time executed) return 0. if an error happen return -1

int cdthread_isThrereMessages(cdThreadID_t pThId);   //!< check if indicated thread has almost a message on the thread message queue
cdMessageID_t cdthread_getMessage(cdThreadID_t pThId); //!<return the first message (ID) on msg queue of indicated thread (and sign it as readed); if there isn't or an error occours then return CDMESSAGEID_ERROR
int cdthread_removeMessage(cdThreadID_t pThId);      //!<remove first message from thread msg queue and return true if action has success, otherwise return false


//******************************************************************************************************

sint_t cdthread_Engine(void);   //!< is the engine for thread and must be called continuosly

//cdThreadID_t cdthread_ThreadToRun(cdThreadID_t LastRunnedThread);   //!< is called by cdthread_Engine() to determine which is next thread to run

sint_t cdthread_UserCallThreadFunction(cdThreadID_t pThId, int* exitCode); //!< call thread_function, if thread is enabled and id is good (no errors); return true if called, otherwise false



//*********************************************************************
//*************timers *************************************************
//*********************************************************************


//use a 64bit means that if ticks is 1us we have 580000 years before it has a round 

//indicates that 1ms is equal to 100 ticks of Absolute_timer
#define CDTIMER_us_FOR_TICK   8
#ifndef CDTIMER_us_FOR_TICK
    #error CDTIMER_us_FOR_TICK not defined, see CDThread.h
#endif


//indicates that 1ms is equal to 100 ticks of Absolute_timer
//#define CDTIMER_1ms_TICKS   ( 1000 / CDTIMER_us_FOR_TICK )
#define CDTIMER_1ms_TICKS   125

//#define CDTIMER_1s_TICKS   ( 1000000 / CDTIMER_us_FOR_TICK )
#define CDTIMER_1s_TICKS    125000

//the cdTimerID is not a simple ID but contains the value when timer expires (expeted absolute timer value)
//with CDTIMER_us_FOR_TICK = 8 the maximun storable time is 34359,7384 seconds that is a little more than 9,5 hours
#ifdef PLATFORM_BLACKFIN
   typedef uint64_t cdTimerID;
#else
   typedef uint32_t cdTimerID;
#endif
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
//GLOBAL VARIABLE FOR TIME, see CDTIMER_us_FOR_TICK to know how many us for ticks
extern  cdTimerID   Absolute_timer;
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

   
#define cdtimer_setupTicks(ticksVal)    (Absolute_timer + ticksVal)
#define cdtimer_setup_s(msVal)          (Absolute_timer + (msVal * CDTIMER_1s_TICKS))
#define cdtimer_setupms(msVal)          (Absolute_timer + (msVal * CDTIMER_1ms_TICKS))
#define cdtimer_setupus(usVal)          (Absolute_timer + (usVal / CDTIMER_us_FOR_TICK))
#define cdtimer_isExpired(cdTID)      (cdTID <= Absolute_timer)
#define cdtimer_isNotExpired(cdTID)      (cdTID > Absolute_timer)

//use example:
//cdTimerID myTimer1, myTimer2;
//myTimer1 = cdtimers_setupTicks(20);   //assign the timer value for 200us
//myTimer2 = cdtimers_setupms(1);      //assign the timer value for 1ms

//while( cdtimer_isNotExpired(myTimer1) ){
//  ....
//  if( cdtimer_isExpired(myTimer2){
//    ....
//  }
//  ....
//} 


//*********************************************************************
//*************jump table inside cdth_functions************************
//*********************************************************************
//if varaible equl to n then jump to CDTH_EXITandRETURN with same n value
#define CDTH_JUMPTABLE( variable , n )   if( variable == n ) goto LABEL_JUMP_WAIT##n  


//exit with XZX value
#define CDTH_EXITwithCODE(  XZX  )   return XZX


//exit with n value if condition is true, also return here from a CDTH_JUMPTABLE with same n value; return and test condition
#define CDTH_RETURN_HERE(  n  )   LABEL_JUMP_WAIT##n: 

//exit with n value if condition is true, also return here from a CDTH_JUMPTABLE with same n value; return and test condition
#define CDTH_RETURNbutEXITif(  n , condition )   LABEL_JUMP_WAIT##n:  if( condition ) return n

//exit with actual state of internal ThisThread variable
#define CDTH_EXIT      return prevExitCode

//set EXIT CODE code of function bu NOT EXIT NOW
#define CDTH_SET_EXIT_CODE(  XZX  )   prevExitCode = XZX



#endif  //_CDTHREAD_H_

