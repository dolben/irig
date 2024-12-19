/***************************************************************************
 *  IRIG Decoder Test
 *
 *  Copyright (c) 2007 by Hank Dolben
 *  Licensed under the Open Software License version 3.0
 *  http://www.opensource.org/osi3.0/licenses/osl-3.0.php
 ***************************************************************************/
#include "irig.h"
#include <stdio.h>

#ifndef IRIG_SAMPLES
#define IRIG_SAMPLES 20
#endif /* IRIG_SAMPLES */

#define IRIG_A 0
#define IRIG_B 1
#define IRIG_D 2
#define IRIG_E 3
#define IRIG_G 4
#define IRIG_H 5

#ifndef IRIG_FORMAT
#define IRIG_FORMAT IRIG_B
#endif /* IRIG_FORMAT */

#if   IRIG_FORMAT == IRIG_A
#define POSITIONS 10
#define SET_TIME(T,H,IRIG) setIrigaTime(T,H,IRIG)
#define GET_TIME(IRIG,T,H) irigaGetTime(IRIG,T,H)
#elif IRIG_FORMAT == IRIG_B
#define POSITIONS 10
#define SET_TIME(T,H,IRIG) setIrigbTime(T,IRIG)
#define GET_TIME(IRIG,T,H) irigbGetTime(IRIG,T)
#elif IRIG_FORMAT == IRIG_D
#define POSITIONS  6
#define SET_TIME(T,H,IRIG) setIrigdTime(T,IRIG)
#define GET_TIME(IRIG,T,H) irigdGetTime(IRIG,T)
#elif IRIG_FORMAT == IRIG_E
#define POSITIONS 10
#define SET_TIME(T,H,IRIG) setIrigbTime(T,IRIG)
#define GET_TIME(IRIG,T,H) irigeGetTime(IRIG,T)
#elif IRIG_FORMAT == IRIG_G
#define POSITIONS 10
#define SET_TIME(T,H,IRIG) setIriggTime(T,H,IRIG)
#define GET_TIME(IRIG,T,H) iriggGetTime(IRIG,T,H)
#elif IRIG_FORMAT == IRIG_H
#define POSITIONS  6
#define SET_TIME(T,H,IRIG) setIrigdTime(T,IRIG)
#define GET_TIME(IRIG,T,H) irighGetTime(IRIG,T)
#else
#error undefined IRIG format
#endif

#ifndef IRIG_BASE_YEAR
#define IRIG_BASE_YEAR 2000
#endif /* IRIG_BASE_YEAR */

/* private function prototypes */
static short* frame( short* irig );
static void setIrigaTime( struct tm* t, int hundredths, short* irig );
static void setIrigbTime( struct tm* t, short* irig );
static void setIrigdTime( struct tm* t, short* irig );
static void setIriggTime( struct tm* t, int hundredths, short* irig );
static void send( short* irig );
static void sendPulse( int pulsewidth );
static void sendSamples( int n, int b );

/***************************************************************************
 *  Reads times from stdin and
 *  sends generated IRIG data to the decoder.
 */
int main( int argc, char* argv[] ) {
    int   n = 0;
    char  timestr[24];
    
    irigSetFrame(frame,NULL);
    sendPulse(8);
    while ( fgets(timestr,sizeof(timestr),stdin) != NULL ) {
        int       year;
        struct tm t;
        int       hundredths = 0;
        short     irig[POSITIONS];
        
        if ( ++n == 3 ) {
            /* throw in some junk */
            sendPulse(8);
            sendPulse(2);
            sendPulse(3);
            /* give it a position ID to get it to FIRST */
            sendPulse(8);
        }
        sscanf(timestr,"%d/%d/%d:%d:%d.%d",
            &year,&t.tm_yday,&t.tm_hour,&t.tm_min,&t.tm_sec,&hundredths);
        t.tm_year = year-1900;
        SET_TIME(&t,hundredths,irig);
        send(irig);
    }
    sendPulse(8);
    return 0;
}

/***************************************************************************
 *  Handles the IRIG-B decoder's output of a second's data.
 *
 *  Prints the time to stdout in the same format as test input.
 */
static short* frame( short* irig ) {
    struct tm t;
    int       hundredths = 0;
    
    GET_TIME(irig,&t,&hundredths);
    printf("%4d/%03d/%02d:%02d:%02d.%02d\n",
        t.tm_year+1900,t.tm_yday,t.tm_hour,t.tm_min,t.tm_sec,hundredths);
    return irig;
}

#define BCD_FROM_BINARY(BINARY) ((((BINARY)/10)<<5)+(BINARY)%10)

/***************************************************************************
 *  Sets an IRIG-A data array with the given time.
 */
static void setIrigaTime( struct tm* t, int hundredths, short* irig ) {
    setIrigbTime(t,irig);
    irig[4] |= (hundredths/10)<<5;
}

/***************************************************************************
 *  Sets an IRIG-B data array with the given time.
 */
static void setIrigbTime( struct tm* t, short* irig ) {
    int second = (t->tm_hour*60+t->tm_min)*60+t->tm_sec;
    int year = t->tm_year+1900-IRIG_BASE_YEAR;
    
    setIrigdTime(t,irig);
    irig[5] = BCD_FROM_BINARY(year);
    irig[6] = 0;
    irig[7] = 0;
    irig[8] = second&0x1FF;
    irig[9] = second>>9;
}

/***************************************************************************
 *  Sets an IRIG-D data array with the given time.
 */
static void setIrigdTime( struct tm* t, short* irig ) {
    irig[0] = BCD_FROM_BINARY(t->tm_sec)<<1;
    irig[1] = BCD_FROM_BINARY(t->tm_min);
    irig[2] = BCD_FROM_BINARY(t->tm_hour);
    irig[3] = BCD_FROM_BINARY(t->tm_yday%100);
    irig[4] = t->tm_yday/100;
    irig[5] = 0;
}

/***************************************************************************
 *  Sets an IRIG-G data array with the given time.
 */
static void setIriggTime( struct tm* t, int hundredths, short* irig ) {
    int year = t->tm_year+1900-IRIG_BASE_YEAR;
    
    setIrigdTime(t,irig);
    irig[4] |= (hundredths/10)<<5;
    irig[5] = hundredths%10;
    irig[6] = BCD_FROM_BINARY(year);
    irig[7] = 0;
    irig[8] = 0;
    irig[9] = 0;
}

/***************************************************************************
 *  Sends pulses for all the bits in an array of IRIG-B data.
 */
static void send( short* irig ) {
    int tens;
    int bit = 2;

    sendPulse(8);
    for ( tens = 0; tens < POSITIONS; tens++ ) {
        for ( ; bit <= 0x100; bit <<= 1 ) {
            if ( (irig[tens]&bit) != 0 ) {
                sendPulse(5);
            } else {
                sendPulse(2);
            }
        }
        sendPulse(8);
        bit = 1;
    }
}

/***************************************************************************
 *  Sends samples of a pulse of given width.
 */
static void sendPulse( int pulsewidth ) {
    sendSamples(pulsewidth*IRIG_SAMPLES/10.0+0.5,1);
    sendSamples((10-pulsewidth)*IRIG_SAMPLES/10.0+0.5,0);
}

/* definitions to handle either order of bits wrt to pulse train */
#ifdef IRIG_LSBIT_FIRST
#define FIRST_BIT     1
#define LAST_BIT   0x80
#define SHIFT(BIT) BIT <<= 1
#else
#define FIRST_BIT  0x80
#define LAST_BIT      1
#define SHIFT(BIT) BIT >>= 1
#endif

/***************************************************************************
 *  Sends a number of samples as bits in a byte stream, set as given.
 */
static void sendSamples( int n, int b ) {
    static unsigned char sampleByte = 0;
    static unsigned char sampleBit = FIRST_BIT;
    
    while ( n-- > 0 ) {
        if ( b != 0 ) {
            sampleByte |= sampleBit;
        }
        if ( sampleBit == LAST_BIT ) {
            irigReceive(sampleByte);
            sampleByte = 0;
            sampleBit = FIRST_BIT;
        } else {
            SHIFT(sampleBit);
        }
    }
}

