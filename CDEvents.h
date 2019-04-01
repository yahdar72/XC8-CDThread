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
//// Created on 27/01/2012                                              ////
//// Modify on 12/05/2018 to be adapted at XC8 compiler                 ////
//// File: cdevents.h                                                   ////
//// Description:                                                       ////
////    This file has the functions prototypes and data definitions     ////
////    to manage software events types                                 ////  
////////////////////////////////////////////////////////////////////////////
#ifndef __CDEVENTS_H_
#define __CDEVENTS_H_

#ifdef DOXYGEN
    #define section( YY )
#endif

#include "CD_types.h"


typedef struct cdevent_t_tag{
   sint_t Enabled;   //!< indicates if true that the event is usable (if false and active=true however the isActive function return false) 
   sint_t Active;      //!< indicates if the event is happened, this flag must be resetted (=false) by the event consumer/manager
   sint_t Id;         //!< optional, indicates a number that identify the event of the same type (used in events array)
   void* ptrParam;   //!< optional, is a pointer to static memory that point to a primitive data or struct (user data) that must be used to pass a parameter to event consumer/manager
}cdevent_t;



/*! \def cdeventInit( XX,YY)
   inits the event, this means that is enabled but not activated and the id is set to pEvtId with param pointer to null (0); y is the id number (int) for this event
   \n param XX is the event structure
   \n param YY is the event id number; 
   \todo test it   
*/
#define cdeventInit(XX,YY)  XX.Enabled= TRUE ; XX.Active= FALSE ; XX.Id=( YY ); XX.ptrParam=0

/*! \def cdeventGenerate(XX)
   activate the event, pratically set Active at true, but doesn't change Enabled, so if enabled is false then the event remains inactive untile Enabled became true 
   \n param XX is the event structure
   \todo test it   
*/
#define cdeventGenerate(XX) XX.Active= TRUE

/*! \def cdeventKill(XX)
   deactivate the event, pratically set Active at false 
   \n param XX is the event structure
   \todo test it   
*/
#define cdeventKill(XX) XX.Active= FALSE

/*! \def cdeventEnable(XX)
   Enable the event, pratically set Enable at true, but doesn't change Active, if Active is true the event became immediately active, otherwise the event remains inactive untile Active became true 
   \n param XX is the event structure
   \todo test it   
*/
#define cdeventEnable(XX) XX.Enabled= TRUE

/*! \def cdeventDisable(XX)
   Disable the event, pratically set Enable at false, but doesn't change Active, if Active is true the event became immediately inactive, until it return enabled
   \n param XX is the event structure
   \todo test it   
*/
#define cdeventDisable(XX) XX.Enabled= FALSE

/*! \def cdeventIsActive(XX)
   checks if event is active (was generated); the event must be Enabled to be  also active. It will be active, if was generated, when will return Enabled
   \n param XX is the event structure
   \todo test it   
*/
#define cdeventIsActive(XX) ((XX.Enabled) && (XX.Active))   

/*! \def cdeventGetId(XX)
   read the id of the event; XX is the event_struct variable 
   \n param XX is the event structure
   \todo test it   
*/
#define cdeventGetId(XX) (XX.Id)

/*! \def cdeventSetParamPointer(XX,YY)
   set the ptrParam of event to YY that must be a pointer; XX is the event_struct variable 
   \n param XX is the event structure
   \n param YY is the memory pointer to param data; the structure of data depends by event and must be knowed by event user 
   \todo test it   
*/
#define cdeventSetParamPointer(XX,YY) XX.ptrParam=(void*)(YY)

/*! \def cdeventGetParamPointer(XX)
	get the ptrParam of event, return a void pointer; xx is the event_struct variable 
   \n param XX is the event structure
   \todo test it   
*/
#define cdeventGetParamPointer(XX) (XX.ptrParam)

#endif //__CDEVENTS_H_
