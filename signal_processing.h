///////////////////////////////////////////////////////////////////////////////////////
//                                                                                   //
//  Copyright (c) 2016-2025 Leonardo Consoni <leonardojc@protonmail.com>             //
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


/// @file signal_processing.h
/// @brief Common signal processing functions
///
/// Interface for applying common operations to a measured signal, such as amplification, offset measuring/removal, rectification, normalization, filtering, etc.

#ifndef SIGNAL_PROCESSING_H
#define SIGNAL_PROCESSING_H

#include <stdint.h>
#include <stddef.h>


/// Selectable signal processing phases/modes
enum SigProcState 
{ 
  SIG_PROC_STATE_MEASUREMENT,    ///< Default mode: signal is processed and result is outputed assuming normal operation (preprocessed offset, gain, filter, and limits values are applied)
  SIG_PROC_STATE_CALIBRATION,    ///< Minimum and maximum values from filtered signal are registered for posterior normalization, if specified
  SIG_PROC_STATE_OFFSET,         ///< Raw values mean is stored for posterior offset removal (no processed result is outputed)
  SIG_PROC_STATES_NUMBER         ///< Number of signal processing phases/modes
};

#define SIG_PROC_RECTIFY 0x0F      ///< Create a signal processor that rectifies input signal
#define SIG_PROC_NORMALIZE 0xF0    ///< Create a signal processor that normalizes input signal after calibration

/// Single signal processor (holds single signal processing data) internal data structure
typedef struct _SignalProcessorData SignalProcessorData;
/// Opaque reference to a signal processor data structure
typedef SignalProcessorData* SignalProcessor;

                                                          
/// @brief Create processor data structure based on given flags                                              
/// @param[in] flags set of signal processing options (SIGNAL_PROCESSING_RECTIFY and SIGNAL_PROCESSING_NORMALIZE), that could be omitted (0) or combined (with | operator) 
/// @return reference/pointer to newly created processor data structure
SignalProcessor SignalProcessor_Create( uint8_t flags );

/// @brief Deallocate internal data of given processor                    
/// @param[in] processor reference to signal processor
void SignalProcessor_Discard( SignalProcessor processor );

/// @brief Set value that multiplies the input signal before processing
/// @param[in] processor reference to signal processor
/// @param[out] inputGain input gain factor
void SignalProcessor_SetInputGain( SignalProcessor processor, double inputGain );
                                                                  
/// @brief Create internal low-pass filter for removing processor input signal higher frequencies
/// @param[in] processor reference to signal processor
/// @param[in] relativeFrequency cut frequency, relative to (factor of) input sampling frequency
void SignalProcessor_SetMaxFrequency( SignalProcessor processor, double relativeFrequency );

/// @brief Create internal high-pass filter for removing processor input signal lower frequencies
/// @param[in] processor reference to signal processor
/// @param[in] relativeFrequency cut frequency, relative to (factor of) input sampling frequency
void SignalProcessor_SetMinFrequency( SignalProcessor processor, double relativeFrequency );

/// @brief Update processor internal signal value based on given new raw input samples             
/// @param[in] processor reference to signal processor
/// @param[in] newInputValuesList pointer to array of new sampled raw values
/// @param[in] newValuesNumber length (in elements) of the input samples array
/// @return processed (amplified, rectified, filtered, normalized, etc.) resulting signal value
double SignalProcessor_UpdateSignal( SignalProcessor processor, double* newInputValuesList, size_t newValuesNumber );

/// @brief Set current processing phase/state/mode of given processor                     
/// @param[in] processor reference to signal processor
/// @param[in] newProcessingPhase desired signal processing phase
void SignalProcessor_SetState( SignalProcessor processor, enum SigProcState newProcessingPhase );

/// @brief Get last measured signal offset from given processor                    
/// @param[in] processor reference to signal processor
/// @return measured signal offset value
double SignalProcessor_GetOffset( SignalProcessor processor );

/// @brief Get last measured signal range (max-min) from given processor                    
/// @param[in] processor reference to signal processor
/// @return measured signal amplitude value
double SignalProcessor_GetAmplitude( SignalProcessor processor );

#endif // SIGNAL_PROCESSING_H
