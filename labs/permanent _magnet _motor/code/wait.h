// Wait functions
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#ifndef WAIT_H_
#define WAIT_H_

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

extern void wait4Seconds(void);
extern void waitMicrosecond(uint32_t us);

#endif
