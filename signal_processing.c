///////////////////////////////////////////////////////////////////////////////////////
//                                                                                   //
//  Copyright (c) 2016-2020 Leonardo Consoni <leonardojc@protonmail.com>             //
//                                                                                   //
//  This file is part of Simple Signal Processing.                                   //
//                                                                                   //
//  Simple Signal Processing is free software: you can redistribute it and/or modify //
//  it under the terms of the GNU Lesser General Public License as published         //
//  by the Free Software Foundation, either version 3 of the License, or             //
//  (at your option) any later version.                                              //
//                                                                                   //
//  Simple Signal Processing is distributed in the hope that it will be useful,      //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of                   //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                     //
//  GNU Lesser General Public License for more details.                              //
//                                                                                   //
//  You should have received a copy of the GNU Lesser General Public License         //
//  along with Simple Signal Processing. If not, see <http://www.gnu.org/licenses/>. //
//                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////


#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "signal_processing.h"

#define FILTER_LENGTH 3
#define OFFSET_SAMPLES_MAX_NUMBER 100

struct _SignalProcessorData
{
  double inputGain;
  double signalLimitsList[ 2 ];
  double signalOffset;
  size_t offsetSamplesCount;
  enum SigProcState processingPhase;
  bool rectify, normalize;
  double hpInputFilterCoeffs[ FILTER_LENGTH ], hpOutputFilterCoeffs[ FILTER_LENGTH ];
  double hpInputSamplesList[ FILTER_LENGTH ], hpOutputSamplesList[ FILTER_LENGTH ];
  double lpInputFilterCoeffs[ FILTER_LENGTH ], lpOutputFilterCoeffs[ FILTER_LENGTH ];
  double lpInputSamplesList[ FILTER_LENGTH ], lpOutputSamplesList[ FILTER_LENGTH ];
};


SignalProcessor SignalProcessor_Create( uint8_t flags )
{
  SignalProcessor newProcessor = (SignalProcessor) malloc( sizeof(SignalProcessorData) );
  memset( newProcessor, 0, sizeof(SignalProcessorData) );
  
  newProcessor->inputGain = 1.0;
  newProcessor->signalLimitsList[ 0 ] = newProcessor->signalLimitsList[ 1 ] = 0.0;
  newProcessor->signalOffset = 0.0;
  
  newProcessor->processingPhase = SIG_PROC_STATE_MEASUREMENT;
  
  newProcessor->rectify = (bool) ( flags & SIG_PROC_RECTIFY );
  newProcessor->normalize = (bool) ( flags & SIG_PROC_NORMALIZE );
  
  newProcessor->lpInputFilterCoeffs[ 0 ] = 1.0;
  newProcessor->hpInputFilterCoeffs[ 0 ] = 1.0;
  
  return newProcessor;
}

void SignalProcessor_Discard( SignalProcessor processor )
{
  if( processor == NULL ) return;
  
  free( processor );
}

void SignalProcessor_SetInputGain( SignalProcessor processor, double inputGain )
{
  if( processor == NULL ) return;
  
  processor->inputGain = inputGain;
}

void SignalProcessor_SetMaxFrequency( SignalProcessor processor, double relativeFrequency )
{
  if( processor == NULL ) return;
  
  if( relativeFrequency <= 0.0 ) return;
  
  if( relativeFrequency >= 0.5 ) relativeFrequency = 0.49;
  
  relativeFrequency *= 6.28;
  
  double outputGain = 4 + 2 * sqrt( 2.0 ) * relativeFrequency + relativeFrequency * relativeFrequency;
  
  processor->lpOutputFilterCoeffs[ 1 ] = ( -8 + 2 * relativeFrequency * relativeFrequency ) / outputGain;
  processor->lpOutputFilterCoeffs[ 2 ] = ( 4 - 2 * sqrt( 2.0 ) * relativeFrequency + relativeFrequency * relativeFrequency ) / outputGain;
  
  processor->lpInputFilterCoeffs[ 0 ] = relativeFrequency * relativeFrequency / outputGain;
  processor->lpInputFilterCoeffs[ 1 ] = 2 * relativeFrequency * relativeFrequency / outputGain;
  processor->lpInputFilterCoeffs[ 2 ] = relativeFrequency * relativeFrequency / outputGain;
}

void SignalProcessor_SetMinFrequency( SignalProcessor processor, double relativeFrequency )
{
  if( processor == NULL ) return;
  
  if( relativeFrequency <= 0.0 ) return;
  
  if( relativeFrequency >= 0.5 ) relativeFrequency = 0.49;
  
  relativeFrequency *= 6.28;
  
  double outputGain = 4 + 2 * sqrt( 2.0 ) * relativeFrequency + relativeFrequency * relativeFrequency;
  
  processor->hpOutputFilterCoeffs[ 1 ] = ( -8 + 2 * relativeFrequency * relativeFrequency ) / outputGain;
  processor->hpOutputFilterCoeffs[ 2 ] = ( 4 - 2 * sqrt( 2.0 ) * relativeFrequency + relativeFrequency * relativeFrequency ) / outputGain;
  
  processor->hpInputFilterCoeffs[ 0 ] = 4 / outputGain;
  processor->hpInputFilterCoeffs[ 1 ] = -8 / outputGain;
  processor->hpInputFilterCoeffs[ 2 ] = 4 / outputGain;
}

static double ApplyFilterStep( double* inputSamplesList, double* inputCoeffsList, double* outputSamplesList, double* outputCoeffsList, double newInputValue )
{
  for( int sampleIndex = FILTER_LENGTH - 1; sampleIndex > 0; sampleIndex-- )
  {
    inputSamplesList[ sampleIndex ] = inputSamplesList[ sampleIndex - 1 ];
    outputSamplesList[ sampleIndex ] = outputSamplesList[ sampleIndex - 1 ];
  }
  inputSamplesList[ 0 ] = newInputValue;
  
  outputSamplesList[ 0 ] = 0.0;
  for( size_t sampleIndex = 0; sampleIndex < FILTER_LENGTH; sampleIndex++ )
  {
    outputSamplesList[ 0 ] -= outputCoeffsList[ sampleIndex ] * outputSamplesList[ sampleIndex ];
    outputSamplesList[ 0 ] += inputCoeffsList[ sampleIndex ] * inputSamplesList[ sampleIndex ];
  }
  
  return outputSamplesList[ 0 ];
}

double SignalProcessor_UpdateSignal( SignalProcessor processor, double* newInputValuesList, size_t newValuesNumber )
{
  if( processor == NULL ) return 0.0;
  
  double newInputValue = processor->lpOutputSamplesList[ 0 ];
  
  if( processor->processingPhase == SIG_PROC_STATE_OFFSET )
  {
    if( newValuesNumber > 0 )
    {
      if( processor->offsetSamplesCount >= OFFSET_SAMPLES_MAX_NUMBER ) processor->offsetSamplesCount = ( OFFSET_SAMPLES_MAX_NUMBER - 1 );
      processor->signalOffset *= processor->offsetSamplesCount;
      for( size_t valueIndex = 0; valueIndex < newValuesNumber; valueIndex++ )
      {
        processor->signalOffset += newInputValuesList[ valueIndex ] * processor->inputGain;
        processor->offsetSamplesCount++;
      }
      processor->signalOffset /= processor->offsetSamplesCount;
    }
    newInputValue = processor->signalOffset;
  }
  else
  {
    for( size_t valueIndex = 0; valueIndex < newValuesNumber; valueIndex++ )
    {
      newInputValue = newInputValuesList[ valueIndex ] * processor->inputGain - processor->signalOffset;

      newInputValue = ApplyFilterStep( processor->hpInputSamplesList, processor->hpInputFilterCoeffs, processor->hpOutputSamplesList, processor->hpOutputFilterCoeffs, newInputValue );
      
      if( processor->rectify ) newInputValue = fabs( newInputValue );
      
      newInputValue = ApplyFilterStep( processor->lpInputSamplesList, processor->lpInputFilterCoeffs, processor->lpOutputSamplesList, processor->lpOutputFilterCoeffs, newInputValue );
    
      if( processor->processingPhase == SIG_PROC_STATE_CALIBRATION )
      {
        if( newInputValue > processor->signalLimitsList[ 1 ] ) processor->signalLimitsList[ 1 ] = newInputValue;
        else if( newInputValue < processor->signalLimitsList[ 0 ] ) processor->signalLimitsList[ 0 ] = newInputValue;
      }
      else if( processor->processingPhase == SIG_PROC_STATE_MEASUREMENT )
      {
        if( processor->normalize && ( processor->signalLimitsList[ 0 ] != processor->signalLimitsList[ 1 ] ) )
        {
          if( newInputValue > processor->signalLimitsList[ 1 ] ) newInputValue = processor->signalLimitsList[ 1 ];
          else if( newInputValue < processor->signalLimitsList[ 0 ] ) newInputValue = processor->signalLimitsList[ 0 ];

          newInputValue = newInputValue / ( processor->signalLimitsList[ 1 ] - processor->signalLimitsList[ 0 ] );
        }
      }
    }
  }
  
  return newInputValue;
}

void SignalProcessor_SetState( SignalProcessor processor, enum SigProcState newProcessingPhase )
{
  if( processor == NULL ) return;
  
  if( newProcessingPhase >= SIG_PROC_STATES_NUMBER ) return;
  
  if( newProcessingPhase == SIG_PROC_STATE_CALIBRATION )
  {
    processor->signalLimitsList[ 1 ] = 0.0;
    processor->signalLimitsList[ 0 ] = 0.0;
  }
  else if( newProcessingPhase == SIG_PROC_STATE_OFFSET )
  {
    processor->signalOffset = 0.0;
    processor->offsetSamplesCount = 0;
  }
  
  processor->processingPhase = newProcessingPhase;
}

double SignalProcessor_GetOffset( SignalProcessor processor )
{
  if( processor == NULL ) return 0.0;
  
  if( processor->processingPhase == SIG_PROC_STATE_OFFSET ) return 0.0;
  
  return processor->signalOffset;
}

double SignalProcessor_GetAmplitude( SignalProcessor processor )
{
  if( processor == NULL ) return 1.0;
  
  if( processor->signalLimitsList[ 0 ] == processor->signalLimitsList[ 1 ] ) return 1.0;
  
  return ( processor->signalLimitsList[ 1 ] - processor->signalLimitsList[ 0 ] );
}
