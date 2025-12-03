/**************************************************************************/
/*!
@file     MQ135.cpp
@author   G.Krocker (Mad Frog Labs)
@license  GNU GPLv3

First version of an Arduino Library for the MQ135 gas sensor
TODO: Review the correction factor calculation. This currently relies on
the datasheet but the information there seems to be wrong.

@section  HISTORY

v1.0 - First release
*/
/**************************************************************************/

#include "MQ135.h"
#include <math.h>

float RZERO = 28000;//28000.0f;
/**************************************************************************/
/*!
@brief  Default constructor

@param[in] pin  The analog input pin for the readout of the sensor
*/
/**************************************************************************/

/**************************************************************************/
/*!
@brief  Get the correction factor to correct for temperature and humidity

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The calculated correction factor
*/
/**************************************************************************/
float getCorrectionFactor(float t, float h)
{
    return CORA*t*t - CORB*t + CORC - (h - 33.0f)*CORD;
}

/**************************************************************************/
/*!
@brief  Get the resistance of the sensor, ie. the measurement value

@return The sensor resistance in kOhm
*/
/**************************************************************************/
float getResistance(float vcc, float vout)
{
    return RLOAD * (vcc / vout - 1.0f);
}

/**************************************************************************/
/*!
@brief  Get the resistance of the sensor, ie. the measurement value corrected
        for temp/hum

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The corrected sensor resistance kOhm
*/
/**************************************************************************/
float getCorrectedResistance(float t, float h, float rs)
{
    float corr = getCorrectionFactor(t, h);
    return rs / corr;
}

/**************************************************************************/
/*!
@brief  Get the ppm of CO2 sensed (assuming only CO2 in the air)

@return The ppm of CO2 in the air
*/
/**************************************************************************/
float getPPM(float rs)
{
    return PARA * powf((rs / RZERO), -PARB);
}

/**************************************************************************/
/*!
@brief  Get the ppm of CO2 sensed (assuming only CO2 in the air), corrected
        for temp/hum

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The ppm of CO2 in the air
*/
/**************************************************************************/
float getCorrectedPPM(float t, float h, float rs)
{
    float rs_corr = getCorrectedResistance(t, h, rs);
    return PARA * powf((rs_corr / RZERO), -PARB);
}

/**************************************************************************/
/*!
@brief  Get the resistance RZero of the sensor for calibration purposes

@return The sensor resistance RZero in kOhm
*/
/**************************************************************************/
float getRZero(float rs)
{
    return rs * powf((ATMOCO2 / PARA), (1.0f / PARB));
}

/**************************************************************************/
/*!
@brief  Get the corrected resistance RZero of the sensor for calibration
        purposes

@param[in] t  The ambient air temperature
@param[in] h  The relative humidity

@return The corrected sensor resistance RZero in kOhm
*/
/**************************************************************************/
float getCorrectedRZero(float t, float h, float rs)
{
    float rs_corr = getCorrectedResistance(t, h, rs);
    return rs_corr * powf((ATMOCO2 / PARA), (1.0f / PARB));
}