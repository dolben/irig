/***************************************************************************
 *  IRIG Decoder
 *
 *  Copyright (c) 2007 by Hank Dolben
 *  Licensed under the Open Software License version 3.0
 *  http://www.opensource.org/osi3.0/licenses/osl-3.0.php
 ***************************************************************************/
#ifndef _IRIG_H
#define _IRIG_H

#include <time.h>

typedef short* (*IRIG_FRAME)( short* irig );

void irigSetFrame( IRIG_FRAME second, short* irig );
void irigReceive( unsigned char byte );
void irigSample( int level );
void irigPulse( int width );

int     irigaGetTime( const short* irig, struct tm* t, int* hundredths );
int     irigbGetTime( const short* irig, struct tm* t );
void    irigdGetTime( const short* irig, struct tm* t );
#define irigeGetTime(IRIG,T) irigbGetTime(IRIG,T)
void    iriggGetTime( const short* irig, struct tm* t, int* hundredths );
#define irighGetTime(IRIG,T) irigdGetTime(IRIG,T)

#endif

