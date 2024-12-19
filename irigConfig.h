/***************************************************************************
 *  IRIG Decoder Configuration - 20 samples per pulse
 *
 *  Copyright (c) 2007 by Hank Dolben
 *  Licensed under the Open Software License version 3.0
 *  http://www.opensource.org/osi3.0/licenses/osl-3.0.php
 ***************************************************************************/
#ifndef _IRIG_CONFIG_H
#define _IRIG_CONFIG_H

/*
 * sample rate is 20 times the pulse rate
 * the nominal pulse width+/-1 (in samples) is acceptable
 *
 *   1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20
 *  -----|   two  |--------|   five |--------|  eight |--------
 */
#define TWO_MIN    3
#define TWO_MAX    5
#define FIVE_MIN   9
#define FIVE_MAX  11
#define EIGHT_MIN 15
#define EIGHT_MAX 17

#endif /* _IRIG_CONFIG_H */

