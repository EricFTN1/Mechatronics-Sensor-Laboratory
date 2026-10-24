#ifndef LOOKUP_H
#define LOOKUP_H

#include <stdint.h>
#include <stdbool.h>
//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

//extern int LookupTable[40][12];
float AnalogToTemp(float Analog);
float TempToAnalog(int Tempature);

#endif
