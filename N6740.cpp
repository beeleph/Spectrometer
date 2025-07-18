/******************************************************************************
*
*
***************************************************************************//**
* ote TERMS OF USE:
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation. This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the
* software, documentation and results solely at his own risk.
*
*  Description:
*  -----------------------------------------------------------------------------
*
*  -----------------------------------------------------------------------------
*  Syntax: WaveDump [ConfigFile]
*  Default config file is "WaveDumpConfig.txt"
******************************************************************************/

#define Spectrometer_Release        "0.0"
#define Spectrometer_Release_Date   "month 2020"
#define DBG_TIME

#include <CAENDigitizer.h>
#include "N6740.h"

int cal_ok[MAX_CH] = { 0 };
int dc_file[MAX_CH];
float dc_8file[8];
int thr_file[MAX_CH] = { 0 };

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

/* ###########################################################################
*  Functions
*  ########################################################################### */
/*! \fn      static long get_time()
*   \brief   Get time in milliseconds
*
*   \return  time in msec
*/
static long get_time()
{
    long time_ms;
#ifdef WIN32
    struct _timeb timebuffer;
    _ftime( &timebuffer );
    time_ms = (long)timebuffer.time * 1000 + (long)timebuffer.millitm;
#else
    struct timeval t1;
    struct timezone tz;
    gettimeofday(&t1, &tz);
    time_ms = (t1.tv_sec) * 1000 + t1.tv_usec / 1000;
#endif
    return time_ms;
}

/*! \fn      int WriteRegisterBitmask(int32_t handle, uint32_t address, uint32_t data, uint32_t mask)
*   \brief   writes 'data' on register at 'address' using 'mask' as bitmask
*
*   \param   handle :   Digitizer handle
*   \param   address:   Address of the Register to write
*   \param   data   :   Data to Write on the Register
*   \param   mask   :   Bitmask to use for data masking
*   \return  0 = Success; negative numbers are error codes
*/
int N6740::WriteRegisterBitmask(uint32_t address, uint32_t data, uint32_t mask) {
    int32_t ret = CAEN_DGTZ_Success;
    uint32_t d32 = 0xFFFFFFFF;

    ret = CAEN_DGTZ_ReadRegister(handle, address, &d32);
    if(ret != CAEN_DGTZ_Success)
        return ret;

    data &= mask;
    d32 &= ~mask;
    d32 |= data;
    ret = CAEN_DGTZ_WriteRegister(handle, address, d32);
    return ret;
}

/*! \fn      int ProgramDigitizer(int handle, WaveDumpConfig_t WDcfg)
*   \brief   configure the digitizer according to the parameters read from
*            the cofiguration file and saved in the WDcfg data structure
*
*   \param   handle   Digitizer handle
*   \param   WDcfg:   WaveDumpConfig data structure
*   \return  0 = Success; negative numbers are error codes
*/
int N6740::ProgramDigitizer()
{
    int i, ret = 0;

    /* reset the digitizer */
    ret |= CAEN_DGTZ_Reset(handle);
    if (ret != 0) {
        emit N6740Say("Error: Unable to reset digitizer.\nPlease reset digitizer manually then restart the program");
        return -1;
    }

    // Set the waveform test bit for debugging
    if (TestPattern)
        ret |= CAEN_DGTZ_WriteRegister(handle, CAEN_DGTZ_BROAD_CH_CONFIGBIT_SET_ADD, 1<<3);

    ret |= CAEN_DGTZ_SetRecordLength(handle, RecordLength);
    ret |= CAEN_DGTZ_GetRecordLength(handle, &RecordLength);

    ret |= CAEN_DGTZ_SetDecimationFactor(handle, DecimationFactor);

    ret |= CAEN_DGTZ_SetPostTriggerSize(handle, PostTrigger);
    uint32_t pt;
    ret |= CAEN_DGTZ_GetPostTriggerSize(handle, &pt);
    PostTrigger = pt;
    ret |= CAEN_DGTZ_SetIOLevel(handle, FPIOtype);
//    if( InterruptNumEvents > 0) {
//        // Interrupt handling
//        if( ret |= CAEN_DGTZ_SetInterruptConfig( handle, CAEN_DGTZ_ENABLE,
//            VME_INTERRUPT_LEVEL, VME_INTERRUPT_STATUS_ID,
//            (uint16_t)InterruptNumEvents, INTERRUPT_MODE)!= CAEN_DGTZ_Success) {
//                qDebug() << "\nError configuring interrupts. Interrupts disabled\n\n";
//                InterruptNumEvents = 0;
//        }
//    }
	
    ret |= CAEN_DGTZ_SetMaxNumEventsBLT(handle, NumEvents);
    ret |= CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);
    ret |= CAEN_DGTZ_SetExtTriggerInputMode(handle, ExtTriggerMode);

    ret |= CAEN_DGTZ_SetGroupEnableMask(handle, EnableMask);
    for(i=0; i<(Nch/8); i++) {
        if (EnableMask & (1<<i)) {
            if(Version_used[i] == 1)
                ret |= Set_calibrated_DCO(i);
            else
                ret |= CAEN_DGTZ_SetGroupDCOffset(handle, i, DCoffset[i]);
            ret |= CAEN_DGTZ_SetGroupSelfTrigger(handle, ChannelTriggerMode[i], (1<<i));
            ret |= CAEN_DGTZ_SetGroupTriggerThreshold(handle, i, Threshold[i]);
            ret |= CAEN_DGTZ_SetChannelGroupMask(handle, i, GroupTrgEnableMask[i]);
        }
        ret |= CAEN_DGTZ_SetTriggerPolarity(handle, i, PulsePolarity_to_TriggerPolarity(PulsePolarity[i])); //.TriggerEdge
    }

    /* execute generic write commands */
    for(i=0; i<GWn; i++)
        ret |= WriteRegisterBitmask(GWaddr[i], GWdata[i], GWmask[i]);

    if (ret)
        emit N6740Say("Warning: errors found during the programming of the digitizer.\nSome settings may not be executed");

    return 0;
}


/*! \fn      void Calibrate_XX740_DC_Offset(int handle, WaveDumpConfig_t WDcfg, CAEN_DGTZ_BoardInfo_t BoardInfo)
*   \brief   calibrates DAC of enabled channel groups (only if BASELINE_SHIFT is in use)
*
*   \param   handle   Digitizer handle
*   \param   WDcfg:   Pointer to the WaveDumpConfig_t data structure
*   \param   BoardInfo: structure with the board info
*/
void N6740::Calibrate_XX740_DC_Offset(){
	float cal[MAX_CH];
	float offset[MAX_CH] = { 0 };
	int i = 0, acq = 0, k = 0, p=0, g = 0;
	for (i = 0; i < MAX_CH; i++)
		cal[i] = 1;
	CAEN_DGTZ_ErrorCode ret;
	CAEN_DGTZ_AcqMode_t mem_mode;
	uint32_t  AllocatedSize;

	ERROR_CODES ErrCode = ERR_NONE;
	uint32_t BufferSize;
	CAEN_DGTZ_EventInfo_t       EventInfo;
	char *buffer = NULL;
	char *EventPtr = NULL;
	CAEN_DGTZ_UINT16_EVENT_t    *Event16 = NULL;

    float avg_value[NPOINTS][MAX_CH] = {{ 0 }};
	uint32_t dc[NPOINTS] = { 25,75 }; //test values (%)
	uint32_t groupmask = 0;

	ret = CAEN_DGTZ_GetAcquisitionMode(handle, &mem_mode);//chosen value stored
	if (ret)
        emit N6740Say("Error trying to read acq mode!!");
	ret = CAEN_DGTZ_SetAcquisitionMode(handle, CAEN_DGTZ_SW_CONTROLLED);
	if (ret)
        emit N6740Say("Error trying to set acq mode!!");
	ret = CAEN_DGTZ_SetExtTriggerInputMode(handle, CAEN_DGTZ_TRGMODE_DISABLED);
	if (ret)
        emit N6740Say("Error trying to set ext trigger!!");
	ret = CAEN_DGTZ_SetMaxNumEventsBLT(handle, 1);
	if (ret)
        emit N6740Say("Warning: error setting max BLT number");
	ret = CAEN_DGTZ_SetDecimationFactor(handle, 1);
	if (ret)
        emit N6740Say("Error trying to set decimation factor!!");
	for (g = 0; g< (int32_t)BoardInfo.Channels; g++) //BoardInfo.Channels is number of groups for x740 boards
		groupmask |= (1 << g);
	ret = CAEN_DGTZ_SetGroupSelfTrigger(handle, CAEN_DGTZ_TRGMODE_DISABLED, groupmask);
	if (ret)
        emit N6740Say("Error disabling self trigger");
	ret = CAEN_DGTZ_SetGroupEnableMask(handle, groupmask);
	if (ret)
        emit N6740Say("Error enabling channel groups.");
	///malloc
	ret = CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &AllocatedSize);
	if (ret) {
		ErrCode = ERR_MALLOC;
        emit N6740Say(ErrMsg[ErrCode]);
        return;
	}

	ret = CAEN_DGTZ_AllocateEvent(handle, (void**)&Event16);
	if (ret != CAEN_DGTZ_Success) {
		ErrCode = ERR_MALLOC;
        emit N6740Say(ErrMsg[ErrCode]);
        return;
	}

    emit N6740Say("Starting DAC calibration...");

	for (p = 0; p < NPOINTS; p++){
		for (i = 0; i < (int32_t)BoardInfo.Channels; i++) { //BoardInfo.Channels is number of groups for x740 boards
                ret = CAEN_DGTZ_SetGroupDCOffset(handle, (uint32_t)i, (uint32_t)((float)(abs((int)dc[p] - 100))*(655.35)));
				if (ret)
                    emit N6740Say("Error setting group test offset" + QString::number(i));
		}
	#ifdef _WIN32
				Sleep(200);
	#else
				usleep(200000);
	#endif

	CAEN_DGTZ_ClearData(handle);

	ret = CAEN_DGTZ_SWStartAcquisition(handle);
	if (ret) {
        emit N6740Say("Error starting X740 acquisition");
        emit N6740Say(ErrMsg[ErrCode]);
        return;
	}

    int value[NACQS][MAX_CH] = {{ 0 }}; //baseline values of the NACQS
	for (acq = 0; acq < NACQS; acq++) {
		CAEN_DGTZ_SendSWtrigger(handle);

		ret = CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize);
		if (ret) {
			ErrCode = ERR_READOUT;
            emit N6740Say(ErrMsg[ErrCode]);
            return;
		}
		if (BufferSize == 0)
			continue;
		ret = CAEN_DGTZ_GetEventInfo(handle, buffer, BufferSize, 0, &EventInfo, &EventPtr);
		if (ret) {
			ErrCode = ERR_EVENT_BUILD;
            emit N6740Say(ErrMsg[ErrCode]);
            return;
		}
		// decode the event //
		ret = CAEN_DGTZ_DecodeEvent(handle, EventPtr, (void**)&Event16);
		if (ret) {
			ErrCode = ERR_EVENT_BUILD;
            emit N6740Say(ErrMsg[ErrCode]);
            return;
		}
		for (g = 0; g < (int32_t)BoardInfo.Channels; g++) {
				for (k = 1; k < 21; k++) //mean over 20 samples
					value[acq][g] += (int)(Event16->DataChannel[g * 8][k]);

				value[acq][g] = (value[acq][g] / 20);
		}

	}//for acq

	///check for clean baselines
	for (g = 0; g < (int32_t)BoardInfo.Channels; g++) {
			int max = 0;
			int mpp = 0;
			int size = (int)pow(2, (double)BoardInfo.ADC_NBits);
            int *freq = (int*)calloc(size, sizeof(int));
			//find the most probable value mpp
			for (k = 0; k < NACQS; k++) {
				if (value[k][g] > 0 && value[k][g] < size) {
					freq[value[k][g]]++;
					if (freq[value[k][g]] > max) {
						max = freq[value[k][g]];
						mpp = value[k][g];
					}
				}
			}
			free(freq);
			//discard values too far from mpp
			int ok = 0;
			for (k = 0; k < NACQS; k++) {
				if (value[k][g] >= (mpp - 5) && value[k][g] <= (mpp + 5)) {
					avg_value[p][g] = avg_value[p][g] + (float)value[k][g];
					ok++;
				}
			}
			avg_value[p][g] = (avg_value[p][g] / (float)ok)*100. / (float)size;
	}

	CAEN_DGTZ_SWStopAcquisition(handle);
	}//close for p

	for (g = 0; g < (int32_t)BoardInfo.Channels; g++) {
			cal[g] = ((float)(avg_value[1][g] - avg_value[0][g]) / (float)(dc[1] - dc[0]));
			offset[g] = (float)(dc[1] * avg_value[0][g] - dc[0] * avg_value[1][g]) / (float)(dc[1] - dc[0]);
            emit N6740Say("Group DAC calibration ready." + QString::number(g));
            emit N6740Say("Cal  offset " + QString::number(cal[g]) + QString::number(offset[g]));

            DAC_Calib.cal[g] = cal[g];
            DAC_Calib.offset[g] = offset[g];
	}

	CAEN_DGTZ_ClearData(handle);

	///free events e buffer
	CAEN_DGTZ_FreeReadoutBuffer(&buffer);

	CAEN_DGTZ_FreeEvent(handle, (void**)&Event16);
    // ret's was changed by me from |= to =. bcs cpp has problems with |=. it's not crucial
    ret = CAEN_DGTZ_SetMaxNumEventsBLT(handle, NumEvents);
    ret = CAEN_DGTZ_SetDecimationFactor(handle,DecimationFactor);
    ret = CAEN_DGTZ_SetPostTriggerSize(handle, PostTrigger);
    ret = CAEN_DGTZ_SetAcquisitionMode(handle, mem_mode);
    ret = CAEN_DGTZ_SetExtTriggerInputMode(handle, ExtTriggerMode);
    ret = CAEN_DGTZ_SetGroupEnableMask(handle, EnableMask);
	for (i = 0; i < BoardInfo.Channels; i++) {
        if (EnableMask & (1 << i))
            ret = CAEN_DGTZ_SetGroupSelfTrigger(handle, ChannelTriggerMode[i], (1 << i));
	}
	if (ret)
        emit N6740Say("Error setting recorded parameters");

    Save_DAC_Calibration_To_Flash();
}


/*! \fn      void Set_relative_Threshold(int handle, WaveDumpConfig_t *WDcfg, CAEN_DGTZ_BoardInfo_t BoardInfo)
*   \brief   sets the threshold relative to the baseline (only if BASELINE_SHIFT is in use)
*
*   \param   handle   Digitizer handle
*   \param   WDcfg:   Pointer to the WaveDumpConfig_t data structure
*   \param   BoardInfo: structure with the board info
*/
void N6740::Set_relative_Threshold(){
	int ch = 0, i = 0;

	//preliminary check: if baseline shift is not enabled for any channel quit
	int should_start = 0;
	for (ch = 0; ch < (int32_t)BoardInfo.Channels; ch++) {
        if (EnableMask & (1 << ch) && Version_used[ch] == 1) {
			should_start = 1;
			break;
		}
	}
    if (!should_start){
        return;
    }

	CAEN_DGTZ_ErrorCode ret;
	uint32_t  AllocatedSize;
	ERROR_CODES ErrCode = ERR_NONE;
	uint32_t BufferSize;
	CAEN_DGTZ_EventInfo_t       EventInfo;
	char *buffer = NULL;
	char *EventPtr = NULL;
	CAEN_DGTZ_UINT16_EVENT_t    *Event16 = NULL;
	CAEN_DGTZ_UINT8_EVENT_t     *Event8 = NULL;
	uint32_t custom_posttrg = 50, dco, custom_thr;
	float expected_baseline;
	float dco_percent;
	int baseline[MAX_CH] = { 0 }, size = 0, samples = 0;
	int no_self_triggered_event[MAX_CH] = {0};
	int sw_trigger_needed = 0;
	int event_ch;

	///malloc
	ret = CAEN_DGTZ_MallocReadoutBuffer(handle, &buffer, &AllocatedSize);
	if (ret) {
		ErrCode = ERR_MALLOC;
        emit N6740Say(ErrMsg[ErrCode]);
        return;
	}
    if (Nbit == 8)
		ret = CAEN_DGTZ_AllocateEvent(handle, (void**)&Event8);
	else {
		ret = CAEN_DGTZ_AllocateEvent(handle, (void**)&Event16);
	}
	if (ret != CAEN_DGTZ_Success) {
		ErrCode = ERR_MALLOC;
        emit N6740Say(ErrMsg[ErrCode]);
        return;
	}

	//some custom settings
	ret = CAEN_DGTZ_SetPostTriggerSize(handle, custom_posttrg);
	if (ret) {
        emit N6740Say("Threshold calc failed. Error trying to set post trigger!!");
		return;
	}
	//try to set a small threshold to get a self triggered event
	for (ch = 0; ch < (int32_t)BoardInfo.Channels; ch++) {
        if (EnableMask & (1 << ch) && Version_used[ch] == 1) {
			if (BoardInfo.FamilyCode == CAEN_DGTZ_XX740_FAMILY_CODE)
				ret = CAEN_DGTZ_GetGroupDCOffset(handle, ch, &dco);
			else
				ret = CAEN_DGTZ_GetChannelDCOffset(handle, ch, &dco);
			if (ret) {
                emit N6740Say("Threshold calc failed. Error trying to get DCoffset values!!");
				return;
			}
			dco_percent = (float)dco / 65535.;
			expected_baseline = pow(2, (double)BoardInfo.ADC_NBits) * (1.0 - dco_percent);

            custom_thr = (PulsePolarity[ch] == CAEN_DGTZ_PulsePolarityPositive) ? ((uint32_t)expected_baseline + 100) : ((uint32_t)expected_baseline - 100);
			
			if (BoardInfo.FamilyCode == CAEN_DGTZ_XX740_FAMILY_CODE)
				ret = CAEN_DGTZ_SetGroupTriggerThreshold(handle, ch, custom_thr);
			else
				ret = CAEN_DGTZ_SetChannelTriggerThreshold(handle, ch, custom_thr);
			if (ret) {
                emit N6740Say("Threshold calc failed. Error trying to set custom threshold value!!");
				return;
			}
		}
	}

	CAEN_DGTZ_SWStartAcquisition(handle);
	Sleep(300);
	ret = CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize);
	if (ret) {
		ErrCode = ERR_READOUT;
        emit N6740Say(ErrMsg[ErrCode]);
        return;
	}
	//we have some self-triggered event 
	if (BufferSize > 0) {
		ret = CAEN_DGTZ_GetEventInfo(handle, buffer, BufferSize, 0, &EventInfo, &EventPtr);
		if (ret) {
			ErrCode = ERR_EVENT_BUILD;
            emit N6740Say(ErrMsg[ErrCode]);
            return;
		}
		// decode the event //
        if (Nbit == 8)
			ret = CAEN_DGTZ_DecodeEvent(handle, EventPtr, (void**)&Event8);
		else
			ret = CAEN_DGTZ_DecodeEvent(handle, EventPtr, (void**)&Event16);

		if (ret) {
			ErrCode = ERR_EVENT_BUILD;
            emit N6740Say(ErrMsg[ErrCode]);
            return;
		}

		for (ch = 0; ch < (int32_t)BoardInfo.Channels; ch++) {
            if (EnableMask & (1 << ch) && Version_used[ch] == 1) {
				event_ch = (BoardInfo.FamilyCode == CAEN_DGTZ_XX740_FAMILY_CODE) ? (ch * 8) : ch;//for x740 boards shift to channel 0 of next group
                size = (Nbit == 8) ? Event8->ChSize[event_ch] : Event16->ChSize[event_ch];
				if (size == 0) {//no data from channel ch
					no_self_triggered_event[ch] = 1;
					sw_trigger_needed = 1;
					continue;
				}
				else {//use only one tenth of the pre-trigger samples to calculate the baseline
					samples = (int)(size * ((100 - custom_posttrg) / 2) / 100);

					for (i = 0; i < samples; i++) //mean over some pre trigger samples
					{
                        if (Nbit == 8)
							baseline[ch] += (int)(Event8->DataChannel[event_ch][i]);
						else
							baseline[ch] += (int)(Event16->DataChannel[event_ch][i]);
					}
					baseline[ch] = (baseline[ch] / samples);
				}

                if (PulsePolarity[ch] == CAEN_DGTZ_PulsePolarityPositive)
                    Threshold[ch] = (uint32_t)baseline[ch] + thr_file[ch];
                else 	if (PulsePolarity[ch] == CAEN_DGTZ_PulsePolarityNegative)
                    Threshold[ch] = (uint32_t)baseline[ch] - thr_file[ch];

                if (Threshold[ch] < 0) Threshold[ch] = 0;
				size = (int)pow(2, (double)BoardInfo.ADC_NBits);
                if (Threshold[ch] > (uint32_t)size) Threshold[ch] = size;

				if (BoardInfo.FamilyCode == CAEN_DGTZ_XX740_FAMILY_CODE)
                    ret = CAEN_DGTZ_SetGroupTriggerThreshold(handle, ch, Threshold[ch]);
				else
                    ret = CAEN_DGTZ_SetChannelTriggerThreshold(handle, ch, Threshold[ch]);
				if (ret)
                    emit N6740Say("Warning: error setting ch " + QString::number(ch) + " corrected threshold");
			}
		}
	}
	else {
		sw_trigger_needed = 1;
		for(ch = 0; ch < (int32_t)BoardInfo.Channels; ch++)
			no_self_triggered_event[ch] = 1;
	}

	CAEN_DGTZ_ClearData(handle);

	//if from some channels we had no self triggered event, we send a software trigger
	if (sw_trigger_needed) {
		CAEN_DGTZ_SendSWtrigger(handle);

		ret = CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, buffer, &BufferSize);
		if (ret) {
			ErrCode = ERR_READOUT;
            emit N6740Say(ErrMsg[ErrCode]);
            return;
		}
		if (BufferSize == 0)
			return;

		ret = CAEN_DGTZ_GetEventInfo(handle, buffer, BufferSize, 0, &EventInfo, &EventPtr);
		if (ret) {
			ErrCode = ERR_EVENT_BUILD;
            emit N6740Say(ErrMsg[ErrCode]);
            return;
		}
		// decode the event //
        if (Nbit == 8)
			ret = CAEN_DGTZ_DecodeEvent(handle, EventPtr, (void**)&Event8);
		else
			ret = CAEN_DGTZ_DecodeEvent(handle, EventPtr, (void**)&Event16);

		if (ret) {
			ErrCode = ERR_EVENT_BUILD;
            emit N6740Say(ErrMsg[ErrCode]);
            return;
		}

		for (ch = 0; ch < (int32_t)BoardInfo.Channels; ch++) {
            if (EnableMask & (1 << ch) && Version_used[ch] == 1) {
				event_ch = (BoardInfo.FamilyCode == CAEN_DGTZ_XX740_FAMILY_CODE) ? (ch * 8) : ch;//for x740 boards shift to channel 0 of next group
                size = (Nbit == 8) ? Event8->ChSize[event_ch] : Event16->ChSize[event_ch];
				if (!no_self_triggered_event[ch])//we already have a good baseline for channel ch
					continue;

				//use some samples to calculate the baseline
				for (i = 1; i < 11; i++){ //mean over 10 samples
                    if (Nbit == 8)
						baseline[ch] += (int)(Event8->DataChannel[event_ch][i]);
					else
						baseline[ch] += (int)(Event16->DataChannel[event_ch][i]);
				}
					baseline[ch] = (baseline[ch] / 10);
			}

            if (PulsePolarity[ch] == CAEN_DGTZ_PulsePolarityPositive)
                Threshold[ch] = (uint32_t)baseline[ch] + thr_file[ch];
            else 	if (PulsePolarity[ch] == CAEN_DGTZ_PulsePolarityNegative)
                Threshold[ch] = (uint32_t)baseline[ch] - thr_file[ch];

            if (Threshold[ch] < 0) Threshold[ch] = 0;
				size = (int)pow(2, (double)BoardInfo.ADC_NBits);
            if (Threshold[ch] > (uint32_t)size) Threshold[ch] = size;

			if (BoardInfo.FamilyCode == CAEN_DGTZ_XX740_FAMILY_CODE)
                ret = CAEN_DGTZ_SetGroupTriggerThreshold(handle, ch, Threshold[ch]);
			else
                ret = CAEN_DGTZ_SetChannelTriggerThreshold(handle, ch, Threshold[ch]);
			if (ret)
                    emit N6740Say("Warning: error setting ch " + QString::number(ch) + " corrected threshold");
		}
	}//end sw trigger event analysis

	CAEN_DGTZ_SWStopAcquisition(handle);

	//reset posttrigger
    ret = CAEN_DGTZ_SetPostTriggerSize(handle, PostTrigger);
	if (ret)
        emit N6740Say("Error resetting post trigger.");

	CAEN_DGTZ_ClearData(handle);

	CAEN_DGTZ_FreeReadoutBuffer(&buffer);
    if (Nbit == 8)
		CAEN_DGTZ_FreeEvent(handle, (void**)&Event8);
	else
		CAEN_DGTZ_FreeEvent(handle, (void**)&Event16);
}

/*! \fn      void Set_calibrated_DCO(int handle, WaveDumpConfig_t *WDcfg, CAEN_DGTZ_BoardInfo_t BoardInfo)
*   \brief   sets the calibrated DAC value using calibration data (only if BASELINE_SHIFT is in use)
*
*   \param   handle   Digitizer handle
*   \param   WDcfg:   Pointer to the WaveDumpConfig_t data structure
*   \param   BoardInfo: structure with the board info
*/
int N6740::Set_calibrated_DCO(int ch) {
	int ret = CAEN_DGTZ_Success;
    if (Version_used[ch] == 0) //old DC_OFFSET config, skip calibration
		return ret;
    if (PulsePolarity[ch] == CAEN_DGTZ_PulsePolarityPositive) {
        DCoffset[ch] = (uint32_t)((float)(fabs((((float)dc_file[ch] - DAC_Calib.offset[ch]) / DAC_Calib.cal[ch]) - 100.))*(655.35));
        if (DCoffset[ch] > 65535) DCoffset[ch] = 65535;
        if (DCoffset[ch] < 0) DCoffset[ch] = 0;
	}
    else if (PulsePolarity[ch] == CAEN_DGTZ_PulsePolarityNegative) {
        DCoffset[ch] = (uint32_t)((float)(fabs(((fabs(dc_file[ch] - 100.) - DAC_Calib.offset[ch]) / DAC_Calib.cal[ch]) - 100.))*(655.35));
        if (DCoffset[ch] < 0) DCoffset[ch] = 0;
        if (DCoffset[ch] > 65535) DCoffset[ch] = 65535;
	}

    ret = CAEN_DGTZ_SetGroupDCOffset(handle, (uint32_t)ch, DCoffset[ch]);
    if (ret)
        emit N6740Say("Error setting group " + QString::number(ch) + " offset");

	return ret;
}


/*! \brief   Write the event data into the output files
*
*   \param   WDrun Pointer to the WaveDumpRun data structure
*   \param   WDcfg Pointer to the WaveDumpConfig data structure
*   \param   EventInfo Pointer to the EventInfo data structure
*   \param   Event Pointer to the Event to write
*/
void N6740::WriteOutputFiles(double current)
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QFile report("./reports/" + dateTime.date().toString(Qt::ISODate) + "_" + dateTime.time().toString("hh-mm-ss") + ".txt");
    if (report.open(QIODevice::WriteOnly | QIODevice::Text)){
        QTextStream stream(&report);
        stream << dateTime.toString() << endl;
        stream << "Ibruker = " << current << endl;
        stream << "Ch#   MeV   Percent   ExtremumCalibrated   NoiseLevel" << endl;
        for (int i = 0; i < 32; ++i){
            stream << i << "   " << energies[i] << "   " << percents[i] << "              " << extremumCalibrated[i] << "               " << extremumOffset[i] << endl;
        }
        report.close();
        emit N6740Say("Report successfully writed");
    }
    else{
        emit N6740Say("Failed to write report");
    }
}

void N6740::PrepareHistogramUpdate() {
    int bufSums[32];
    for (int i = 0; i < 32; ++i){
        extremum[i] = 0;
        bufSums[i] = 0;
    }
    double extremumSum = 0;        // for propriete divide "/" function (int/double) and it's cheaper than double extremum
    double extremumMax = 0;
    for (int ch = 0; ch < Nch; ch++) {
        uint32_t Size = Event16->ChSize[ch];
        if (Size <= 0) {
            continue;
        }
        if ( PulsePolarity[0] == CAEN_DGTZ_PulsePolarityPositive ){
            extremum[ch] = *std::max_element(Event16->DataChannel[ch], Event16->DataChannel[ch] + Size);
        }
        else{
            extremum[ch] = *std::min_element(Event16->DataChannel[ch], Event16->DataChannel[ch] + Size);
            extremum[ch] = abs(extremum[ch] - 4096);
        }
    }

    for (int ch = 0; ch < Nch; ch++) {
        extremumCalibrated[ch] = extremum[ch] - extremumOffset[ch];
        extremumBuffer[bufCounter][ch] = extremumCalibrated[ch];
        if (extremumCalibrated[ch] < 0)
            extremumCalibrated[ch] = 0;
    }
    for (int buf = 0; buf < AVERAGE_BUFFER_SIZE; ++buf){
        for (int ch = 0; ch < Nch; ch++){
            bufSums[ch] += extremumBuffer[buf][ch];
        }
    }

    extremumSum = std::accumulate(extremumCalibrated, extremumCalibrated + 32, extremumSum);
    extremumMax = *std::max_element(extremumCalibrated, extremumCalibrated + 32);
    if (extremumSum != 0)
        for (int ch = 0; ch < Nch; ch++) {
            percents[ch] = fabs((extremumCalibrated[ch] / extremumSum) * 100);
        }
    else
        percents.fill(0);
    for (int ch = 0; ch < Nch; ch++) {
        if (viewAverage){
            if (viewInPercents)
                dataToHisto[ch] = ((bufSums[ch] / AVERAGE_BUFFER_SIZE) / extremumMax) * 100;    // average instead of extremumcalibrated
            else
                dataToHisto[ch] = ((bufSums[ch] / AVERAGE_BUFFER_SIZE) / 4096.0) * 100;
        } else {
            if (viewInPercents)
                dataToHisto[ch] = (extremumCalibrated[ch] / extremumMax) * 100;
            else
                dataToHisto[ch] = (extremumCalibrated[ch] / 4096.0) * 100;
        }
    }
    bufCounter++;
    if (bufCounter > AVERAGE_BUFFER_SIZE - 1)
        bufCounter = 0;
    emit DataChanged(dataToHisto);
}

/* ########################################################################### */
/*  Init                                                                       */
//  Opens configuration file, parse, connects, gets info, load DAC, programs the digitizer, allocate event memory.
/* ########################################################################### */
int N6740::Init()
{
    //CAEN_DGTZ_ErrorCode ret = CAEN_DGTZ_Success;
    int ret = 0;
    ERROR_CODES ErrCode= ERR_NONE;
    uint32_t AllocatedSize;
    char ConfigFileName[100];
    int MajorNumber;
    FILE *f_ini;
    loopTimer = new QTimer(this);
    connect(loopTimer, SIGNAL(timeout()), this, SLOT(Loop()));
    percents.resize(32);
    dataToHisto.resize(32);
    energies.resize(32);
    viewInPercents = 1;
    for (int i = 0; i < Nch; ++i){          // initialization
        extremumOffset[i] = 0;
    }

    for (int j = 0; j < AVERAGE_BUFFER_SIZE; ++j){
        for (int i = 0; i < 32; ++i){
            extremumBuffer[j][i] = 0;
        }
    }

    int ReloadCfgStatus = 0x7FFFFFFF; // Init to the bigger positive number

	/* *************************************************************************************** */
	/* Open and parse default configuration file                                                       */
	/* *************************************************************************************** */

    strcpy(ConfigFileName, DEFAULT_CONFIG_FILE);

    emit N6740Say("Opening Configuration File");
	f_ini = fopen(ConfigFileName, "r");
	if (f_ini == NULL) {
		ErrCode = ERR_CONF_FILE_NOT_FOUND;
        emit N6740Say(ErrMsg[ErrCode]);
        return -1;
	}
    ParseConfigFile(f_ini);
	fclose(f_ini);

    /* *************************************************************************************** */
    /* Open the digitizer and read the board information                                       */
    /* *************************************************************************************** */
    //isVMEDevice = BaseAddress ? 1 : 0;

    ret = CAEN_DGTZ_OpenDigitizer(int_to_ConnectionType(LinkType), LinkNum, ConetNode, BaseAddress, &handle);
    if (ret) {
        ErrCode = ERR_DGZ_OPEN;
        emit N6740Say(ErrMsg[ErrCode]);
        return -1;
    }

    ret = CAEN_DGTZ_GetInfo(handle, &BoardInfo);
    if (ret) {
        ErrCode = ERR_BOARD_INFO_READ;
        emit N6740Say(ErrMsg[ErrCode]);
        return -1;
    }
    emit N6740Say("Connected to CAEN Digitizer Model ");
    emit N6740Say(BoardInfo.ModelName);
    emit N6740Say("ROC FPGA Release is ");
    emit N6740Say(BoardInfo.ROC_FirmwareRel);
    emit N6740Say("AMC FPGA Release is ");
    emit N6740Say(BoardInfo.AMC_FirmwareRel);
    // Check firmware rivision (DPP firmwares cannot be used with WaveDump */
    sscanf(BoardInfo.AMC_FirmwareRel, "%d", &MajorNumber);
    if (MajorNumber >= 128) {
        emit N6740Say("This digitizer has a DPP firmwareemit N6740Say(");
        ErrCode = ERR_INVALID_BOARD_TYPE;
        emit N6740Say(ErrMsg[ErrCode]);
        return -1;
    }

	/* *************************************************************************************** */
	/* Check if the board needs a specific config file and parse it instead of the default one */
	/* *************************************************************************************** */


    strcpy(ConfigFileName, "WaveDumpConfig_X740.txt");
    emit N6740Say("WARNING: using configuration file specific for Board model X740.\nEdit this file if you want to modify the default settings. ");
    emit N6740Say(ConfigFileName);
    f_ini = fopen(ConfigFileName, "r");
    if (f_ini == NULL) {
        ErrCode = ERR_CONF_FILE_NOT_FOUND;
        emit N6740Say(ErrMsg[ErrCode]);
        return -1;
    }
    ParseConfigFile(f_ini);
    fclose(f_ini);

	//set default DAC calibration coefficients
    for (int i = 0; i < MAX_SET; i++) {
        DAC_Calib.cal[i] = 1;
        DAC_Calib.offset[i] = 0;
	}
	//load DAC calibration data (if present in flash)
    Load_DAC_Calibration_From_Flash();

    // Perform calibration (if needed).
    //if (StartupCalibration)
        //calibrate(handle, &WDrun, BoardInfo);

    // mask the channels not available for this model
    EnableMask &= (1<<(Nch/8))-1;
    /* *************************************************************************************** */
    /* program the digitizer                                                                   */
    /* *************************************************************************************** */
    ret = ProgramDigitizer();
    if (ret) {
        ErrCode = ERR_DGZ_PROGRAM;
        emit N6740Say(ErrMsg[ErrCode]);
        return -1;
    }

    // Select the next enabled group for plotting

    // Read again the board infos, just in case some of them were changed by the programming
    // (like, for example, the TSample and the number of channels if DES mode is changed)
    if(ReloadCfgStatus > 0) {
        ret = CAEN_DGTZ_GetInfo(handle, &BoardInfo);
        if (ret) {
            ErrCode = ERR_BOARD_INFO_READ;
            emit N6740Say(ErrMsg[ErrCode]);
            return -1;
        }

    }

    // Allocate memory for the event data and readout buffer
    ret = CAEN_DGTZ_AllocateEvent(handle, (void**)&Event16);
    if (ret != CAEN_DGTZ_Success) {
        ErrCode = ERR_MALLOC;
        emit N6740Say(ErrMsg[ErrCode]);
        return -1;
    }
    ret = CAEN_DGTZ_MallocReadoutBuffer(handle, &readoutBuffer,&AllocatedSize); /* WARNING: This malloc must be done after the digitizer programming */
    if (ret) {
        ErrCode = ERR_MALLOC;
        emit N6740Say(ErrMsg[ErrCode]);
        return -1;
    }
    ErrCode = ERR_NONE;
    return 0;
}

/* new things */
CAEN_DGTZ_TriggerPolarity_t PulsePolarity_to_TriggerPolarity(CAEN_DGTZ_PulsePolarity_t pp){
    if (pp)
        return CAEN_DGTZ_TriggerOnFallingEdge;
    else
        return CAEN_DGTZ_TriggerOnRisingEdge;
}
CAEN_DGTZ_ConnectionType int_to_ConnectionType(int i){
    if (i)
        return CAEN_DGTZ_OpticalLink;
    else
        return CAEN_DGTZ_USB;
}
N6740::N6740(){

}

void N6740::SetDefaultConfiguration() {
    int i, j;

    RecordLength = (1024*16);
    PostTrigger = 50;
    NumEvents = 1023;
    EnableMask = 0xFFFF;
    GWn = 0;
    ExtTriggerMode = CAEN_DGTZ_TRGMODE_ACQ_ONLY;
    InterruptNumEvents = 0;
    TestPattern = 0;
    DecimationFactor = 1;
    DesMode = CAEN_DGTZ_DISABLE;
    FastTriggerMode = CAEN_DGTZ_TRGMODE_DISABLED;
    FastTriggerEnabled = 0;
    FPIOtype = CAEN_DGTZ_IOLevel_NIM;

    for(i=0; i<MAX_SET; i++) {
        PulsePolarity[i] = CAEN_DGTZ_PulsePolarityPositive;
        Version_used[i] = 0;
        DCoffset[i] = 0;
        Threshold[i] = 0;
        ChannelTriggerMode[i] = CAEN_DGTZ_TRGMODE_DISABLED;
        GroupTrgEnableMask[i] = 0;
        for(j=0; j<MAX_SET; j++) DCoffsetGrpCh[i][j] = -1;
        FTThreshold[i] = 0;
        FTDCoffset[i] =0;
    }
    useCorrections = -1;
    UseManualTables = -1;
    for(i=0; i<MAX_X742_GROUP_SIZE; i++)
        sprintf(TablesFilenames[i], "Tables_gr%d", i);
    DRS4Frequency = CAEN_DGTZ_DRS4_5GHz;
    StartupCalibration = 1;
}

int N6740::ParseConfigFile(FILE *f_ini)
{
    char str[1000], str1[1000], *pread;
    int i, ch=-1, val, Off=0, tr = -1;
    int ret = 0;

    // Save previous values (for compare)
    int PrevDesMode = DesMode;
    int PrevUseCorrections = useCorrections;
    int PrevUseManualTables = UseManualTables;
    size_t TabBuf[sizeof(TablesFilenames)];
    // Copy the filenames to watch for changes
    memcpy(TabBuf, TablesFilenames, sizeof(TablesFilenames));

    /* Default settings */
    SetDefaultConfiguration();

    /* read config file and assign parameters */
    while(!feof(f_ini)) {
        int read;
        char *res;
        // read a word from the file
        read = fscanf(f_ini, "%s", str);
        if( !read || (read == EOF) || !strlen(str))
            continue;
        // skip comments
        if(str[0] == '#') {
            res = fgets(str, 1000, f_ini);
            continue;
        }

        if (strcmp(str, "@ON")==0) {
            Off = 0;
            continue;
        }
        if (strcmp(str, "@OFF")==0)
            Off = 1;
        if (Off)
            continue;


        // Section (COMMON or individual channel)
        if (str[0] == '[') {
            if (strstr(str, "COMMON")) {
                ch = -1;
               continue;
            }
            if (strstr(str, "TR")) {
                sscanf(str+1, "TR%d", &val);
                 if (val < 0 || val >= MAX_SET) {
                    emit N6740Say(str);
                    emit N6740Say(": Invalid channel number");
                } else {
                    tr = val;
                }
            } else {
                sscanf(str+1, "%d", &val);
                if (val < 0 || val >= MAX_SET) {
                    emit N6740Say(str);
                    emit N6740Say(": Invalid channel number");
                } else {
                    ch = val;
                }
            }
            continue;
        }

        // OPEN: read the details of physical path to the digitizer
        if (strstr(str, "OPEN")!=NULL) {
            read = fscanf(f_ini, "%s", str1);
            if (strcmp(str1, "USB")==0)
                LinkType = CAEN_DGTZ_USB;
            else if (strcmp(str1, "PCI")==0)
                LinkType = CAEN_DGTZ_OpticalLink;
            else {
                emit N6740Say(str);
                emit N6740Say(str1);
                emit N6740Say("Invalid connection type");
                return -1;
            }
            read = fscanf(f_ini, "%d", &LinkNum);
            if (LinkType == CAEN_DGTZ_USB)
                ConetNode = 0;
            else
                read = fscanf(f_ini, "%d", &ConetNode);
            read = fscanf(f_ini, "%x", &BaseAddress);
            continue;
        }

        // Generic VME Write (address offset + data + mask, each exadecimal)
        if ((strstr(str, "WRITE_REGISTER")!=NULL) && (GWn < MAX_GW)) {
            read = fscanf(f_ini, "%x", (int *)&GWaddr[GWn]);
            read = fscanf(f_ini, "%x", (int *)&GWdata[GWn]);
            read = fscanf(f_ini, "%x", (int *)&GWmask[GWn]);
            GWn++;
            continue;
        }

        // Acquisition Record Length (number of samples)
        if (strstr(str, "RECORD_LENGTH")!=NULL) {
            read = fscanf(f_ini, "%d", &RecordLength);
            continue;
        }

        // Acquisition Frequency (X742 only)
        if (strstr(str, "DRS4_FREQUENCY")!=NULL) {
            int PrevDRS4Freq = DRS4Frequency;
            int freq;
            read = fscanf(f_ini, "%d", &freq);
            DRS4Frequency = (CAEN_DGTZ_DRS4Frequency_t)freq;
            if(PrevDRS4Freq != DRS4Frequency)
                ret |= (0x1 << CFGRELOAD_CORRTABLES_BIT);
            continue;
        }

        // Correction Level (mask)
        if (strstr(str, "CORRECTION_LEVEL")!=NULL) {
            int changed = 0;

            read = fscanf(f_ini, "%s", str1);
            if( strcmp(str1, "AUTO") == 0 )
                useCorrections = -1;
            else {
                int gr = 0;
                char Buf[1000];
                char *ptr = Buf;

                useCorrections = atoi(str1);
                pread = fgets(Buf, 1000, f_ini); // Get the remaining line
                UseManualTables = -1;
                if(sscanf(ptr, "%s", str1) == 0) {
                    emit N6740Say(str);
                    emit N6740Say("Invalid syntax for parameter");
                    continue;
                }
                if(strcmp(str1, "AUTO") != 0) { // The user wants to use custom correction tables
                    UseManualTables = atoi(ptr); // Save the group mask
                    ptr = strstr(ptr, str1);
                    ptr += strlen(str1);
                    while(sscanf(ptr, "%s", str1) == 1 && gr < MAX_X742_GROUP_SIZE) {
                        while( ((UseManualTables) & (0x1 << gr)) == 0 && gr < MAX_X742_GROUP_SIZE)
                            gr++;
                        if(gr >= MAX_X742_GROUP_SIZE) {
                            emit N6740Say(str);
                            emit N6740Say("Error parsing values for parameter");
                            continue;
                        }
                        ptr = strstr(ptr, str1);
                        ptr += strlen(str1);
                        strcpy(TablesFilenames[gr], str1);
                        gr++;
                    }
                }
            }

            // Check for changes
            if (PrevUseCorrections != useCorrections)
                changed = 1;
            else if (PrevUseManualTables != UseManualTables)
                changed = 1;
            else if (memcmp(TabBuf, TablesFilenames, sizeof(TablesFilenames)))
                changed = 1;
            if (changed == 1)
                ret |= (0x1 << CFGRELOAD_CORRTABLES_BIT);
            continue;
        }

        // Test Pattern
        if (strstr(str, "TEST_PATTERN")!=NULL) {
            read = fscanf(f_ini, "%s", str1);
            if (strcmp(str1, "YES")==0)
                TestPattern = 1;
            else if (strcmp(str1, "NO")!=0){
                emit N6740Say(str);
                emit N6740Say("Invalid option");
            }
            continue;
        }

        // Acquisition Record Length (number of samples)
        if (strstr(str, "DECIMATION_FACTOR")!=NULL) {
            read = fscanf(f_ini, "%d", (int*)&DecimationFactor);
            continue;
        }

        // Trigger Edge
        /*if (strstr(str, "TRIGGER_EDGE")!=NULL) {
            read = fscanf(f_ini, "%s", str1);
            if (strcmp(str1, "FALLING")==0)
                TriggerEdge = 1;
            else if (strcmp(str1, "RISING")!=0)
                printf("%s: invalid option\n", str);
            continue;
        }*/

        // External Trigger (DISABLED, ACQUISITION_ONLY, ACQUISITION_AND_TRGOUT)
        if (strstr(str, "EXTERNAL_TRIGGER")!=NULL) {
            read = fscanf(f_ini, "%s", str1);
            if (strcmp(str1, "DISABLED")==0)
                ExtTriggerMode = CAEN_DGTZ_TRGMODE_DISABLED;
            else if (strcmp(str1, "ACQUISITION_ONLY")==0)
                ExtTriggerMode = CAEN_DGTZ_TRGMODE_ACQ_ONLY;
            else if (strcmp(str1, "ACQUISITION_AND_TRGOUT")==0)
                ExtTriggerMode = CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT;
            else {
                emit N6740Say(str);
                emit N6740Say("Invalid parameter");
            }
            continue;
        }

        // Max. number of events for a block transfer (0 to 1023)
        if (strstr(str, "MAX_NUM_EVENTS_BLT")!=NULL) {
            read = fscanf(f_ini, "%d", &NumEvents);
            continue;
        }

        // GNUplot path
        if (strstr(str, "GNUPLOT_PATH")!=NULL) {
            read = fscanf(f_ini, "%s", GnuPlotPath);
            continue;
        }

        // Post Trigger (percent of the acquisition window)
        if (strstr(str, "POST_TRIGGER")!=NULL) {
            read = fscanf(f_ini, "%d", &PostTrigger);
            continue;
        }

        // DesMode (Double sampling frequency for the Mod 731 and 751)
        if (strstr(str, "ENABLE_DES_MODE")!=NULL) {
            read = fscanf(f_ini, "%s", str1);
            if (strcmp(str1, "YES")==0)
                DesMode = CAEN_DGTZ_ENABLE;
            else if (strcmp(str1, "NO")!=0){
                emit N6740Say(str);
                emit N6740Say("Invalid option");
            }
            if(PrevDesMode != DesMode)
                ret |= (0x1 << CFGRELOAD_DESMODE_BIT);
            continue;
        }

        // Output file format (BINARY or ASCII)
        if (strstr(str, "OUTPUT_FILE_FORMAT")!=NULL) {
            read = fscanf(f_ini, "%s", str1);
            if (strcmp(str1, "BINARY")==0)
                OutFileFlags = OFF_BINARY;
            else if (strcmp(str1, "ASCII")!=0){
                emit N6740Say(str1);
                emit N6740Say("Invalid output file format");
            }
            continue;
        }

        // Header into output file (YES or NO)
        if (strstr(str, "OUTPUT_FILE_HEADER")!=NULL) {
            read = fscanf(f_ini, "%s", str1);
            if (strcmp(str1, "YES")==0)
                OutFileFlags = OFF_HEADER;
            else if (strcmp(str1, "NO")!=0){
                emit N6740Say(str);
                emit N6740Say("Invalid option");
            }
            continue;
        }

        // Interrupt settings (request interrupt when there are at least N events to read; 0=disable interrupts (polling mode))
        if (strstr(str, "USE_INTERRUPT")!=NULL) {
            read = fscanf(f_ini, "%d", &InterruptNumEvents);
            continue;
        }

        if (!strcmp(str, "FAST_TRIGGER")) {
            read = fscanf(f_ini, "%s", str1);
            if (strcmp(str1, "DISABLED")==0)
                FastTriggerMode = CAEN_DGTZ_TRGMODE_DISABLED;
            else if (strcmp(str1, "ACQUISITION_ONLY")==0)
                FastTriggerMode = CAEN_DGTZ_TRGMODE_ACQ_ONLY;
            else {
                emit N6740Say(str);
                emit N6740Say("Invalid Parameter");
            }
            continue;
        }

        if (strstr(str, "ENABLED_FAST_TRIGGER_DIGITIZING")!=NULL) {
            read = fscanf(f_ini, "%s", str1);
            if (strcmp(str1, "YES")==0)
                FastTriggerEnabled= 1;
            else if (strcmp(str1, "NO")!=0){
                emit N6740Say(str);
                emit N6740Say("Invalid option");
            }
            continue;
        }
     ///Input polarity
        if (strstr(str, "PULSE_POLARITY")!=NULL) {
            CAEN_DGTZ_PulsePolarity_t pp= CAEN_DGTZ_PulsePolarityPositive;
            read = fscanf(f_ini, "%s", str1);
            if (strcmp(str1, "POSITIVE") == 0)
                pp = CAEN_DGTZ_PulsePolarityPositive;
            else if (strcmp(str1, "NEGATIVE") == 0)
                pp = CAEN_DGTZ_PulsePolarityNegative;
            else {
                emit N6740Say(str);
                emit N6740Say("Invalid parameter");
            }

                for (i = 0; i<MAX_SET; i++)///polarity setting (old trigger edge)is the same for all channels
                    PulsePolarity[i] = pp;

            continue;
        }

        //DC offset (percent of the dynamic range, -50 to 50)
        if (!strcmp(str, "DC_OFFSET"))
        {

            int dc;
            read = fscanf(f_ini, "%d", &dc);
            if (tr != -1) {
                // 				FTDCoffset[tr] = dc;
                FTDCoffset[tr * 2] = (uint32_t)dc;
                FTDCoffset[tr * 2 + 1] = (uint32_t)dc;
                continue;
            }

            val = (int)((dc + 50) * 65535 / 100);
            if (ch == -1)
                for (i = 0; i < MAX_SET; i++)
                {
                    DCoffset[i] = val;
                    Version_used[i] = 0;
                    dc_file[i] = dc;
                }
            else
            {
                DCoffset[ch] = val;
                Version_used[ch] = 0;
                dc_file[ch] = dc;
            }

            continue;
        }


        if (!strcmp(str, "BASELINE_LEVEL"))
        {

            int dc;
            read = fscanf(f_ini, "%d", &dc);
            if (tr != -1) {
                // 				FTDCoffset[tr] = dc;
                FTDCoffset[tr * 2] = (uint32_t)dc;
                FTDCoffset[tr * 2 + 1] = (uint32_t)dc;
                continue;
            }

            if (ch == -1)
            {
                for (i = 0; i < MAX_SET; i++)
                {
                    Version_used[i] = 1;
                    dc_file[i] = dc;
                    if (PulsePolarity[i] == CAEN_DGTZ_PulsePolarityPositive)
                        DCoffset[i] = (uint32_t)((float)(abs(dc - 100))*(655.35));

                    else if (PulsePolarity[i] == CAEN_DGTZ_PulsePolarityNegative)
                        DCoffset[i] = (uint32_t)((float)(dc)*(655.35));
                }
            }
            else
            {
                Version_used[ch] = 1;
                dc_file[ch] = dc;
                if (PulsePolarity[ch] == CAEN_DGTZ_PulsePolarityPositive)
                {
                    DCoffset[ch] = (uint32_t)((float)(abs(dc - 100))*(655.35));
                    //emit N6740Say("ch");
                    //emit N6740Say(QString::number(ch));
                    //emit N6740Say("positive, offset:");
                    //emit N6740Say(QString::number(DCoffset[ch]));
                    ////printf("ch %d positive, offset %d\n",ch, DCoffset[ch]);
                }


                else if (PulsePolarity[ch] == CAEN_DGTZ_PulsePolarityNegative)
                {
                    DCoffset[ch] = (uint32_t)((float)(dc)*(655.35));
                    ////printf("ch %d negative, offset %d\n",ch, DCoffset[ch]);
                }


               }

                continue;
            }

        if (strstr(str, "GRP_CH_DC_OFFSET")!=NULL) ///xx742
        {
            float dc[8];
            read = fscanf(f_ini, "%f,%f,%f,%f,%f,%f,%f,%f", &dc[0], &dc[1], &dc[2], &dc[3], &dc[4], &dc[5], &dc[6], &dc[7]);
            int j = 0;
            for( j=0;j<8;j++) dc_8file[j] = dc[j];

            for(i=0; i<8; i++) //MAX_SET
            {
                val = (int)((dc[i]+50) * 65535 / 100); ///DC offset (percent of the dynamic range, -50 to 50)
                DCoffsetGrpCh[ch][i]=val;
            }
            continue;
        }

        // Threshold
        if (strstr(str, "TRIGGER_THRESHOLD")!=NULL) {
            read = fscanf(f_ini, "%d", &val);
            if (tr != -1) {
//				FTThreshold[tr] = val;
                FTThreshold[tr*2] = val;
                FTThreshold[tr*2+1] = val;

                continue;
            }
            if (ch == -1)
                for (i = 0; i < MAX_SET; i++)
                {
                    Threshold[i] = val;
                    thr_file[i] = val;
                }
            else
            {
                Threshold[ch] = val;
                thr_file[ch] = val;
            }
            continue;
        }

        // Group Trigger Enable Mask (hex 8 bit)
        if (strstr(str, "GROUP_TRG_ENABLE_MASK")!=NULL) {
            read = fscanf(f_ini, "%x", &val);
            if (ch == -1)
                for(i=0; i<MAX_SET; i++)
                    GroupTrgEnableMask[i] = val & 0xFF;
            else
                 GroupTrgEnableMask[ch] = val & 0xFF;
            continue;
        }

        // Channel Auto trigger (DISABLED, ACQUISITION_ONLY, ACQUISITION_AND_TRGOUT)
        if (strstr(str, "CHANNEL_TRIGGER")!=NULL) {
            CAEN_DGTZ_TriggerMode_t tm;
            read = fscanf(f_ini, "%s", str1);
            if (strcmp(str1, "DISABLED") == 0)
                tm = CAEN_DGTZ_TRGMODE_DISABLED;
            else if (strcmp(str1, "ACQUISITION_ONLY") == 0)
                tm = CAEN_DGTZ_TRGMODE_ACQ_ONLY;
            else if (strcmp(str1, "ACQUISITION_AND_TRGOUT") == 0)
                tm = CAEN_DGTZ_TRGMODE_ACQ_AND_EXTOUT;
            else if (strcmp(str1, "TRGOUT_ONLY") == 0)
                tm = CAEN_DGTZ_TRGMODE_EXTOUT_ONLY;
            else {
                emit N6740Say(str);
                emit N6740Say("Invalid parameter");
                continue;
            }
            if (ch == -1)
                for(i=0; i<MAX_SET; i++)
                    ChannelTriggerMode[i] = tm;
            else
                ChannelTriggerMode[ch] = tm;

            continue;
        }

        // Front Panel LEMO I/O level (NIM, TTL)
        if (strstr(str, "FPIO_LEVEL")!=NULL) {
            read = fscanf(f_ini, "%s", str1);
            if (strcmp(str1, "TTL")==0)
                FPIOtype = CAEN_DGTZ_IOLevel_TTL;
            else if (strcmp(str1, "NIM")!=0){
                emit N6740Say(str);
                emit N6740Say("Invalid option");
            }
            continue;
        }

        // Channel Enable (or Group enable for the V1740) (YES/NO)
        if (strstr(str, "ENABLE_INPUT")!=NULL) {
            read = fscanf(f_ini, "%s", str1);
            if (strcmp(str1, "YES")==0) {
                if (ch == -1)
                    EnableMask = 0xFF;
                else
                {
                    EnableMask |= (1 << ch);
                }
                continue;
            } else if (strcmp(str1, "NO")==0) {
                if (ch == -1)
                    EnableMask = 0x00;
                else
                    EnableMask &= ~(1 << ch);
                continue;
            } else {
                emit N6740Say(str);
                emit N6740Say("Invalid option");
            }
            continue;
        }

        // Skip startup calibration
        if (strstr(str, "SKIP_STARTUP_CALIBRATION") != NULL) {
            read = fscanf(f_ini, "%s", str1);
            if (strcmp(str1, "YES") == 0)
                StartupCalibration = 0;
            else
                StartupCalibration = 1;
            continue;
        }
        emit N6740Say(str);
        emit N6740Say("Invalid settings");
    }
    return ret;
}

void N6740::Load_DAC_Calibration_From_Flash() {
    FLASH_API_ERROR_CODES err = FLASH_API_SUCCESS;
    uint8_t *buffer;
    int ch = 0;
    float calibration_data[2 * MAX_SET];

    err = SPIFlash_init(handle);// init flash
    if (err != FLASH_API_SUCCESS) {
        emit N6740Say("Error in flash init");
        return;
    }

    buffer = (uint8_t*)malloc(1 + VIRTUAL_PAGE_SIZE * sizeof(uint8_t));
    memset(buffer, 0, 1 + VIRTUAL_PAGE_SIZE * sizeof(uint8_t));

    err = SPIFlash_read_virtual_page(handle, OFFSET_CALIBRATION_VIRTUAL_PAGE, buffer);
    if (err != FLASH_API_SUCCESS) {
        emit N6740Say("Error reading flash page size");
        return;
    }
    if (buffer[0] != 0xD) {
        emit N6740Say("No DAC Calibration data found in board flash. Press calibrate button to calibrate DAC.");
        free(buffer);
        return;
    }
    else {
        memcpy(calibration_data, buffer + 1, 2 * MAX_SET * sizeof(float));
        for (ch = 0; ch < (int)BoardInfo.Channels; ch++) {
            DAC_Calib.cal[ch] = calibration_data[2 * ch];
            DAC_Calib.offset[ch] = calibration_data[1 + 2 * ch];
        }
    }

    free(buffer);
    emit N6740Say("DAC calibration correctly loaded from board flash.");
}

void N6740::Save_DAC_Calibration_To_Flash() {
    FLASH_API_ERROR_CODES err = FLASH_API_SUCCESS;
    uint8_t *buffer;
    int ch = 0;
    float calibration_data[2*MAX_SET];

    for (ch = 0; ch < (int)BoardInfo.Channels; ch++) {
        calibration_data[2*ch] = DAC_Calib.cal[ch];
        calibration_data[1 + 2 * ch] = DAC_Calib.offset[ch];
    }

    err = SPIFlash_init(handle);// init flash
    if (err != FLASH_API_SUCCESS) {
        emit N6740Say("Error in flash init.");
        return;
    }

    buffer = (uint8_t*)malloc(1 + VIRTUAL_PAGE_SIZE * sizeof(uint8_t));
    memset(buffer, 0, 1 + VIRTUAL_PAGE_SIZE * sizeof(uint8_t));

    buffer[0] = 0xD;
    memcpy((buffer +1), calibration_data, VIRTUAL_PAGE_SIZE * sizeof(uint8_t));//copy cal vector to buffer

    err = SPIFlash_write_virtual_page(handle, OFFSET_CALIBRATION_VIRTUAL_PAGE,buffer);
    if (err != FLASH_API_SUCCESS) {
        emit N6740Say("Error writing flash page");
        return;
    }

    free(buffer);

    emit N6740Say("DAC calibration correctly saved on flash.");
}

void N6740::Run() {
    PrevRateTime = get_time();
    Set_relative_Threshold();
    emit N6740Say("Acquisition started");
    CAEN_DGTZ_SWStartAcquisition(handle);
    loopTimer->start(500);
}

void N6740::Stop() {
    emit N6740Say("Acquisition stopped");
    loopTimer->stop();
    CAEN_DGTZ_SWStopAcquisition(handle);
}

void N6740::Loop() {
    int ret = 0;
    /* Send a software trigger .cutted.*/

    /* Wait for interrupt (if enabled) .cutted.*/

    /* Read data from the board */
    ret = CAEN_DGTZ_ReadData(handle, CAEN_DGTZ_SLAVE_TERMINATED_READOUT_MBLT, readoutBuffer, &BufferSize);
    if (ret) {
        ErrCode = ERR_READOUT;
        emit N6740Say(ErrMsg[ErrCode]);
        return;
    }
    NumEvents_t = 0;
    if (BufferSize != 0) {
        ret = CAEN_DGTZ_GetNumEvents(handle, readoutBuffer, BufferSize, &NumEvents_t);
        if (ret) {
            ErrCode = ERR_READOUT;
            emit N6740Say(ErrMsg[ErrCode]);
            return;
        }
    }
    else {
        uint32_t lstatus;
        ret = CAEN_DGTZ_ReadRegister(handle, CAEN_DGTZ_ACQ_STATUS_ADD, &lstatus);
        if (ret) {
            emit N6740Say("Warning: Failure reading reg:" + QString::number(CAEN_DGTZ_ACQ_STATUS_ADD) + " " + QString::number(ret));
        }
        else {
            if (lstatus & (0x1 << 19)) {
                    ErrCode = ERR_OVERTEMP;
                    emit N6740Say(ErrMsg[ErrCode]);
                    return;
            }
        }
    }
//InterruptTimeout:
    /* Calculate throughput and trigger rate (every second) */
    CurrentTime = get_time();
    ElapsedTime = CurrentTime - PrevRateTime;
    if (BufferSize == 0)
        if (ret == CAEN_DGTZ_Timeout)
            emit N6740Say("Timeout...");
        else
            emit N6740Say("No data...");
    else
        emit N6740Say("Reading at " + QString::number((float)BufferSize/((float)ElapsedTime*1048.576f)) + " MB/s (Trg Rate):" +  QString::number((float)NumEvents_t*1000.0f/(float)ElapsedTime) + " Hz)");
    PrevRateTime = CurrentTime;

    /* Analyze data */
    for(int i = 0; i < (int)NumEvents_t; i++) {
        /* Get one event from the readout buffer */
        ret = CAEN_DGTZ_GetEventInfo(handle, readoutBuffer, BufferSize, i, &EventInfo, &EventPtr);
        if (ret) {
            ErrCode = ERR_EVENT_BUILD;
            emit N6740Say(ErrMsg[ErrCode]);
            return;
        }
        /* decode the event */
        ret = CAEN_DGTZ_DecodeEvent(handle, EventPtr, (void**)&Event16);
        if (ret) {
            ErrCode = ERR_EVENT_BUILD;
            emit N6740Say(ErrMsg[ErrCode]);
            return;
        }

        /* Update Histograms .cutted.*/
        PrepareHistogramUpdate();
    }
}

void N6740::PerformCalibrate() {
    emit N6740Say("Remember to leave only noise signal before calibration");
    for (int ch = 0; ch < Nch; ++ch){
        extremumOffset[ch] = extremum[ch];
    }
    emit N6740Say("Calibration finished");
}

void N6740::Exit() {
    /* stop the acquisition */
    CAEN_DGTZ_SWStopAcquisition(handle);
    /* close the device and free the buffers */
    CAEN_DGTZ_FreeEvent(handle, (void**)&Event16);
    CAEN_DGTZ_FreeReadoutBuffer(&readoutBuffer);
    CAEN_DGTZ_CloseDigitizer(handle);
}

void N6740::UpdateEnergies(QVector<double> energies){
    this->energies = energies;
}
void N6740::ViewChanged(bool percent){
    viewInPercents = percent;
}
void N6740::AverageChanged(bool average){
    viewAverage = average;
}
