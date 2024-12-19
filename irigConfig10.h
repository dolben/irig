/***************************************************************************
 *  IRIG Decoder Configuration - 10 samples per pulse
 *
 *  Copyright (c) 2007 by Hank Dolben
 *  Licensed under the Open Software License version 3.0
 *  http://www.opensource.org/osi3.0/licenses/osl-3.0.php
 ***************************************************************************/
#ifndef _IRIG_CONFIG_H
#define _IRIG_CONFIG_H

/*
 * sample rate is 10 times the pulse rate
 * the nominal pulse width+/-1 (in samples) is acceptable
 *
 *      1     2     3     4     5     6     7     8     9    10
 *   |       two       |       five      |      eight      |---
 */
#define TWO_MAX   3
#define FIVE_MAX  6
#define EIGHT_MAX 9

#endif /* _IRIG_CONFIG_H */

