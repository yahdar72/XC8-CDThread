////////////////////////////////////////////////////////////////////////////
////        (C) Copyright Dario Cortese                                 ////
//// This source code may only be used by Dario Cortese licensed users  ////
//// This source code may only be distributed to Dario Cortese licensed ////
//// users.  No other use, reproduction  or distribution is permitted   ////
//// without written permission.                                        ////
////                                                                    ////
//// Author: Dario Cortese                                              ////
//// Client: Mariano Cerbone (only for Project Mangoose)                ////
//// Created on 26/08/2012                                              ////
//// Modify on 12/05/2018 to be adapted at XC8 compiler                 ////
//// File: cdtimers.c                                                   ////
//// Description:                                                       ////
//// THIS FILE HAS ALL IMPLEMENTATION OF FUNCTIONS USED FOR CDTIMERS    ////
////   MANAGING; TIMERS ARE USED WITH CDTHREAD, AND VICEVERSA           ////        
////////////////////////////////////////////////////////////////////////////

#ifndef _CDTIMERS_C_
#define _CDTIMERS_C_

#include "CDThread.h"


/*      FOLLOWING CODE IS FOR CCS AND MUST BE RECODED FOR XC8 
************************************ONLY FOR CCS*******************************************************************
#byte SFR_TMR4=0x415
#byte SFR_PR4=0x416
#byte SFR_T4CON=0x417
*/



//enable this define if you want that will be stored 8 time-stamp in cdtimer_GetTicks
//#define _CDTIMERS_DEBUG_

cdTimerID Absolute_timer;


#ifdef _CDTIMERS_DEBUG_
    unsigned int Tim4idx=0;
    unsigned int8 Tim4Vals[8];
#endif


typedef union{
    unsigned int8 u8;
    struct{
    int8 TxCKPS:2;       //bits 0-1//0=1:1, 1=1:4, 2=1:16, 3=1:64
    int8 TMRxON:1;      //bit 2   //0=off, 1=on
    int8 TxOUTPUTS:4;   //bits 3-6//0=1:1, 1=1:2, .... 15=1:16
    int8 N_U:1;         //bit 7   //not used
    } bitfield;
}SFR_TMR246_t;




//it uses TIM4 and a global variable u32_SysTicks to increments my system timer
// every one ticks is equal to 1/125KHz= 8us
//calls almost every 2,048ms

/*! \fn void cdtimer_GetTicks()
   \author Dario Cortese
   \date 01-11-2015
   \brief this function is called by main loop user, or some interrupts, to update Absolute_timer global variable.
   Actualy it uses TMR4 as time counter, with /64 prescaler and PR4=255, so reset happens every ((Fosc/4)/64)/256=2048us wit 32MHz oscillator.
   The
   \return none, but update Absolute_timer global variable
   \warning this function must be redefined on different hardware or implementation
   \version 1.00
   \see cdtimer_INIT
*/
void cdtimer_GetTicks()
{
    static unsigned int8 prev_TMR=0;
    unsigned int8 act_TMR;
    unsigned int8 delta;

    act_TMR = SFR_TMR4;
    delta = act_TMR - prev_TMR; //now is delta ticks
    prev_TMR = act_TMR;
    //add delta to global u32_SysTicks
    Absolute_timer += delta;

#ifdef _CDTIMERS_DEBUG_
    Tim4Vals[Tim4idx]=act_TMR;
    Tim4idx++;
    if (Tim4idx>7)
        Tim4idx=0;
#endif
}



/*! \fn void cdtimer_INIT()
   \author Dario Cortese
   \date 01-11-2015
   \brief this function is called by main setup to initialize the time counter (one of the timers available in the microcontroller) in polling mode (NO ISR)
   Actualy it uses TMR4 with prescaler=/64  and PR4=255, so reset happens every ((Fosc/4)/64)/256=2048us wit 32MHz oscillator.
   \return none
   \warning this function must be redefined on different hardware or implementation
   \version 1.00
   \see cdtimer_GetTicks
*/
void cdtimer_INIT()
{
    //intialize timer 4 as
    //#use timer(timer=4,tick=1000us,bits=32,NOISR)
    SFR_TMR246_t tmr;

/*      FOLLOWING CODE IS FOR CCS AND MUST BE RECODED FOR XC8 

    disable_interrupts(INT_TIMER4);    //no interrupt, only polling

    SFR_PR4 = 0xFF;     //reset timer after 256 clock (clock is output of prescaler)
    tmr.bitfield.TxCKPS = 3;    //prescaler = /64
    tmr.bitfield.TxOUTPUTS = 0; //postacaler =1:1
    tmr.bitfield.TMRxON = 1;    //timer run
    Absolute_timer=0;
    SFR_TMR4=0;
    SFR_T4CON = tmr.u8;  //setup and start timer 4
*/
}


#endif //_CDTIMERS_C_

