/***************************************************************************
 *  IRIG Decoder
 *
 *  Copyright (c) 2007 by Hank Dolben
 *  Licensed under the Open Software License version 3.0
 *  http://www.opensource.org/osi3.0/licenses/osl-3.0.php
 ***************************************************************************/
#include "irig.h"
#include <stdio.h>

#ifdef   IRIG_CONFIG_H
#include IRIG_CONFIG_H
#else
#include "irigConfig.h"
#endif /* IRIG_CONFIG_H */

/*
 * if the time code is pre-200-04 there is no year encoded
 * and IRIG_BASE_YEAR should be set to 0
 */
#ifndef IRIG_BASE_YEAR
#define IRIG_BASE_YEAR 2000
#endif /* IRIGB_BASE_YEAR */

/* the number of positions in a frame, e.g., 10 in one second for IRIG-B */
#ifndef IRIG_POSITIONS
#define IRIG_POSITIONS 10
#endif /* IRIG_POSITIONS */

/* the last position */
#define LAST (IRIG_POSITIONS-1)

/* if minima are undefined, there are no "invalid" pulse widths */
#ifndef TWO_MIN
#define TWO_MIN 1
#endif
#ifndef FIVE_MIN
#define FIVE_MIN (TWO_MAX+1)
#endif
#ifndef EIGHT_MIN
#define EIGHT_MIN (FIVE_MAX+1)
#endif

#ifdef DEBUG
#define DBG_MSG(ARGS...) fprintf(stderr,ARGS)
#else
#define DBG_MSG(ARGS...)
#endif /* DEBUG */

/* states of IRIG pulse scanner                 */
typedef enum {
    IRIG_START = 0,
    IRIG_FIRST,
    IRIG_DATA,
    IRIG_SYNCH
} IRIG_STATE;

/* private function prototype for valid frame   */
static short* dummy( short* irig );

/* function called with each valid frame's data */
static IRIG_FRAME frame = dummy;

/* buffer to use if not set by irigSetFrame     */
static short      buffer[IRIG_POSITIONS];

/* data store                                   */
static short*     data = buffer;

/***************************************************************************
 *  Sets the function to call, when there is one frame of valid data,
 *  and the data buffer to use.
 */
void irigSetFrame( IRIG_FRAME irigFrame, short* irig ) {
    if ( irigFrame != NULL ) {
        frame = irigFrame;
    }
    if ( irig != NULL ) {
        data = irig;
    }
}

/***************************************************************************
 *  placeholder for the function called with each valid frame's data
 */
static short* dummy( short* irig ) {
    return irig;
}

/* definitions to handle either order of bits wrt to pulse train */
#ifdef IRIG_LSBIT_FIRST   /* least significant bit first         */
#define FIRST_BIT     1
#define NO_BIT    0x100
#define SHIFT_BIT <<= 1
#else                     /* most significant bit first          */
#define FIRST_BIT  0x80
#define NO_BIT        0
#define SHIFT_BIT >>= 1
#endif

/***************************************************************************
 *  Handles one byte of sample data.
 *
 *  Each bit is one sample.
 */
void irigReceive( unsigned char byte ) {
    int bit;

    for ( bit = FIRST_BIT; bit != NO_BIT; bit SHIFT_BIT ) {
        irigSample(byte&bit);
    }
}

/***************************************************************************
 *  Handles a sample of the waveform, counting pulse width.
 */
void irigSample( int level ) {
    static short pulsewidth;
    
    if ( level == 0 ) {
        if ( pulsewidth > 0 ) {
            if ( pulsewidth <  TWO_MIN   ) irigPulse(1); else
            if ( pulsewidth <= TWO_MAX   ) irigPulse(2); else
            if ( pulsewidth <  FIVE_MIN  ) irigPulse(4); else
            if ( pulsewidth <= FIVE_MAX  ) irigPulse(5); else
            if ( pulsewidth <  EIGHT_MIN ) irigPulse(7); else
            if ( pulsewidth <= EIGHT_MAX ) irigPulse(8); else
                                           irigPulse(10);
        }
        pulsewidth = 0;
    } else {
        pulsewidth += 1;
    }
}

/***************************************************************************
 *  Cranks the state machine for the next pulse of given width
 *  putting the denoted data into the current buffer and calling
 *  the designated function with each valid frame's data.
 *
 *  The state machine accepts a valid sequence of pulses where
 *  the 8 unit wide position identifiers occur as expected.
 *
 *  Only pulse widths of 8, 5, and 2 are accepted as valid.
 *  To allow more latitude, condition the width before calling.
 *
 *  Note that the state machine does not require the signal
 *  to be zero for the correct interval between pulses.
 */
void irigPulse( int width ) {
    static IRIG_STATE state; /* current state                     */
    static short      tens;  /* the tens place of the pulse index */
    static short      ones;  /* the ones place of the pulse index */
    
    DBG_MSG(" %d",width);
    switch ( state ) {
    case IRIG_START:
        DBG_MSG("-START\n");
        if ( width == 8 ) {
            DBG_MSG("--------------------\n");
            state = IRIG_FIRST;
        }
        break;
    case IRIG_FIRST:
        if ( width == 8 ) {
            ones = 1;
            tens = 0;
            data[0] = 0;
            state = IRIG_DATA;
        } else {
            state = IRIG_START;
        }
        break;
    case IRIG_DATA:
        if ( width == 8 ) {
            state = IRIG_FIRST;
        } else {
            if ( width == 5 ) {
                data[tens] |= (1<<ones);
            } else if ( width != 2 ) {
                state = IRIG_START;
            }
            if ( ones++ == 8 ) {
                state = IRIG_SYNCH;
            }
        }
        break;
    case IRIG_SYNCH:
        if ( width != 8 ) {
            state = IRIG_START;
        } else {
            DBG_MSG("-SYNCH\n");
            if ( tens++ == LAST ) {
                data = (*frame)(data);
                DBG_MSG("--------------------\n");
                state = IRIG_FIRST;
            } else {
                ones = 0;
                data[tens] = 0;
                state = IRIG_DATA;
            }
        }
        break;
    default:
        state = IRIG_START; /* just in case */
        break;
    }
}

#define BINARY_FROM_BCD(BCD) (((BCD)>>5)*10+((BCD)&0xF))

/***************************************************************************
 *  Gets the time from an array of IRIG-A data.
 *
 *  returns the second of the day
 */
int irigaGetTime( const short* irig, struct tm* t, int* hundredths ) {
    *hundredths = (irig[4]>>5)*10;
    return irigbGetTime(irig,t);
}

/***************************************************************************
 *  Gets the time from an array of IRIG-B or IRIG-E data.
 *
 *  returns the second of the day
 */
int irigbGetTime( const short* irig, struct tm* t ) {
    irigdGetTime(irig,t);
#if IRIG_BASE_YEAR != 0
    t->tm_year = BINARY_FROM_BCD(irig[5])+IRIG_BASE_YEAR-1900;
#endif
    return (irig[9]<<9)|irig[8];
}

/***************************************************************************
 *  Gets the time from an array of IRIG data;
 *  all that there is in IRIG-D and IRIG-H.
 */
void irigdGetTime( const short* irig, struct tm* t ) {
    t->tm_sec   = BINARY_FROM_BCD(irig[0]>>1);
    t->tm_min   = BINARY_FROM_BCD(irig[1]);
    t->tm_hour  = BINARY_FROM_BCD(irig[2]);
    t->tm_yday  = BINARY_FROM_BCD(irig[3])+(irig[4]&3)*100;
    t->tm_year  = 0;
    t->tm_mday  = 0;
    t->tm_mon   = 0;
    t->tm_wday  = 0;
    t->tm_isdst = 0;
}

/***************************************************************************
 *  Gets the time from an array of IRIG-G data.
 */
void iriggGetTime( const short* irig, struct tm* t, int* hundredths ) {
    irigdGetTime(irig,t);
    *hundredths = (irig[4]>>5)*10+(irig[5]&0xF);
#if IRIG_BASE_YEAR != 0
    t->tm_year = BINARY_FROM_BCD(irig[6])+IRIG_BASE_YEAR-1900;
#endif
}

