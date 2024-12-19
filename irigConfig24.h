/***************************************************************************
 *  IRIG Decoder Configuration - 24 samples per pulse
 *
 *  Copyright (c) 2007 by Hank Dolben
 *  Licensed under the Open Software License version 3.0
 *  http://www.opensource.org/osi3.0/licenses/osl-3.0.php
 ***************************************************************************/
#ifndef _IRIG_CONFIG_H
#define _IRIG_CONFIG_H

/*
 * sample rate is 24 times the pulse rate
 * the nominal pulse width+/-2 (in samples) is acceptable
 *
 *   1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24
 *  ------|     two     |-----|     five     |-----|     eight    |--------
 */
#define TWO_MIN    3
#define TWO_MAX    7
#define FIVE_MIN  10
#define FIVE_MAX  14
#define EIGHT_MIN 17
#define EIGHT_MAX 21

#endif /* _IRIG_CONFIG_H */

