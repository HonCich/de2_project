/**************************************************************************/
/*!
@file     MQ135.h
@author   G.Krocker (Mad Frog Labs)
@license  GNU GPLv3

First version of an Arduino Library for the MQ135 gas sensor
TODO: Review the correction factor calculation. This currently relies on
the datasheet but the information there seems to be wrong.

@section  HISTORY

v1.0 - First release
*/
/**************************************************************************/
#ifndef MQ135_H
#define MQ135_H

#include <stdint.h>

/// The load resistance on the board
#define RLOAD 20000
/// Calibration resistance at atmospheric CO2 level
extern float RZERO;
/// Parameters for calculating ppm of CO2 from sensor resistance
#define PARA 400
#define PARB 2.769034857

/// Parameters to model temperature and humidity dependence
#define CORA 0.00035
#define CORB 0.02718
#define CORC 1.39538
#define CORD 0.0018

/// Atmospheric CO2 level for calibration purposes
#define ATMOCO2 397.13



float getCorrectionFactor(float t, float h);
float getResistance(float vcc, float vout);
float getCorrectedResistance(float t, float h, float rs);
float getPPM(float rs);
float getCorrectedPPM(float t, float h, float rs);
float getRZero(float rs);
float getCorrectedRZero(float t, float h, float rs);

#endif