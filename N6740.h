/******************************************************************************
* 
* CAEN SpA - Front End Division
* Via Vetraia, 11 - 55049 - Viareggio ITALY
* +390594388398 - www.caen.it
*
***************************************************************************//**
* \note TERMS OF USE:
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation. This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the 
* software, documentation and results solely at his own risk.
******************************************************************************/

#ifndef _N6740_H_
#define _N6740_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cmath>
#include <CAENDigitizer.h>
#include <CAENDigitizerType.h>
#include <QApplication>
#include <QDebug>
#include <QTimer>
#include <QFile>
#include <QDate>
#include <flash.h>


#include <time.h>
#include <sys/timeb.h>
#include <conio.h>
#include <process.h>
#include <algorithm>

#define		_PACKED_            // Какая-то дичь которая сжимает структуры (убирает пропуски необходимые для добора до 32х или 64х бит) чтобы??? чтобы они меньше памяти занимали?
#define		_INLINE_            // директива для прямой вставки функций в месте их вызова вместо использования скомпилированной и оптимизированной версии.


#define DEFAULT_CONFIG_FILE  "WaveDumpConfig.txt"  /* local directory */

#define MAX_CH  64          /* max. number of channels */
#define MAX_SET 16           /* max. number of independent settings */
#define MAX_GROUPS  8          /* max. number of groups */

#define MAX_GW  1000        /* max. number of generic write commads */

#define CFGRELOAD_CORRTABLES_BIT (0)
#define CFGRELOAD_DESMODE_BIT (1)

#define NPOINTS 2
#define NACQS   50

/* ###########################################################################
   Typedefs
   ###########################################################################
*/
typedef enum {
    OFF_BINARY=	0x00000001,			// Bit 0: 1 = BINARY, 0 =ASCII
    OFF_HEADER= 0x00000002,			// Bit 1: 1 = include header, 0 = just samples data
} OUTFILE_FLAGS;

typedef struct{
    float cal[MAX_SET];
    float offset[MAX_SET];
}DAC_Calibration_data;

/* Error messages */
typedef enum  {
    ERR_NONE= 0,
    ERR_CONF_FILE_NOT_FOUND,
    ERR_DGZ_OPEN,
    ERR_BOARD_INFO_READ,
    ERR_INVALID_BOARD_TYPE,
    ERR_DGZ_PROGRAM,
    ERR_MALLOC,
    ERR_RESTART,
    ERR_INTERRUPT,
    ERR_READOUT,
    ERR_EVENT_BUILD,
    ERR_HISTO_MALLOC,
    ERR_UNHANDLED_BOARD,
    ERR_OUTFILE_WRITE,
    ERR_OVERTEMP,

    ERR_DUMMY_LAST,
} ERROR_CODES;
static char ErrMsg[ERR_DUMMY_LAST][100] = {
    "No Error",                                         /* ERR_NONE */
    "Configuration File not found",                     /* ERR_CONF_FILE_NOT_FOUND */
    "Can't open the digitizer",                         /* ERR_DGZ_OPEN */
    "Can't read the Board Info",                        /* ERR_BOARD_INFO_READ */
    "Can't run WaveDump for this digitizer",            /* ERR_INVALID_BOARD_TYPE */
    "Can't program the digitizer",                      /* ERR_DGZ_PROGRAM */
    "Can't allocate the memory for the readout buffer", /* ERR_MALLOC */
    "Restarting Error",                                 /* ERR_RESTART */
    "Interrupt Error",                                  /* ERR_INTERRUPT */
    "Readout Error",                                    /* ERR_READOUT */
    "Event Build Error",                                /* ERR_EVENT_BUILD */
    "Can't allocate the memory fro the histograms",     /* ERR_HISTO_MALLOC */
    "Unhandled board type",                             /* ERR_UNHANDLED_BOARD */
    "Output file write error",                          /* ERR_OUTFILE_WRITE */
    "Over Temperature",									/* ERR_OVERTEMP */

};

class N6740 : public QObject {
    Q_OBJECT

public:
    N6740();
    int Init();

signals:
    void N6740Say(QString text);       // for debug info to show in GUI
    void DataChanged(QVector<double>);

public slots:
    void Run();
    void Stop();
    void PerformCalibrate();
    void Exit();
    void Loop();
    void WriteOutputFiles(double current); // change
    void UpdateEnergies(QVector<double> energies);
    void ViewChanged(bool percent);

private:
    void Set_relative_Threshold();
    void Calibrate_XX740_DC_Offset();
    int Set_calibrated_DCO(int ch);
    int ParseConfigFile(FILE *f_ini);
    int ProgramDigitizer();
    int WriteRegisterBitmask(uint32_t address, uint32_t data, uint32_t mask);
    void Load_DAC_Calibration_From_Flash();
    void Save_DAC_Calibration_To_Flash();
    void SetDefaultConfiguration();
    void PrepareHistogramUpdate();

    // wdconfig part
    int LinkType;
    int LinkNum;
    int ConetNode;
    uint32_t BaseAddress;
    int Nch = 32;
    int Nbit = 12;
    float Ts = 16.0;
    int NumEvents;
    uint32_t RecordLength;
    int PostTrigger;
    int InterruptNumEvents;
    int TestPattern;
    CAEN_DGTZ_EnaDis_t DesMode;
    //int TriggerEdge;
    CAEN_DGTZ_IOLevel_t FPIOtype;
    CAEN_DGTZ_TriggerMode_t ExtTriggerMode;
    uint16_t EnableMask;
    char GnuPlotPath[1000];
    CAEN_DGTZ_TriggerMode_t ChannelTriggerMode[MAX_SET];
    CAEN_DGTZ_PulsePolarity_t PulsePolarity[MAX_SET];
    uint32_t DCoffset[MAX_SET];
    int32_t  DCoffsetGrpCh[MAX_SET][MAX_SET];
    uint32_t Threshold[MAX_SET];
    int Version_used[MAX_SET];
    uint8_t GroupTrgEnableMask[MAX_SET];
    uint32_t MaxGroupNumber;

    uint32_t FTDCoffset[MAX_SET];
    uint32_t FTThreshold[MAX_SET];
    CAEN_DGTZ_TriggerMode_t	FastTriggerMode;
    uint32_t	 FastTriggerEnabled;
    int GWn;
    uint32_t GWaddr[MAX_GW];
    uint32_t GWdata[MAX_GW];
    uint32_t GWmask[MAX_GW];
    OUTFILE_FLAGS OutFileFlags;
    uint16_t DecimationFactor;
    int useCorrections;
    int UseManualTables;
    char TablesFilenames[MAX_X742_GROUP_SIZE][1000];
    CAEN_DGTZ_DRS4Frequency_t DRS4Frequency;
    int StartupCalibration;
    DAC_Calibration_data DAC_Calib;

    // wdrun part

    // oldmain part
    int  handle = -1;
    CAEN_DGTZ_BoardInfo_t BoardInfo;
    CAEN_DGTZ_EventInfo_t       EventInfo;
    CAEN_DGTZ_UINT16_EVENT_t    *Event16=NULL; /* generic event struct with 16 bit data (10, 12, 14 and 16 bit digitizers */
    uint64_t CurrentTime, PrevRateTime, ElapsedTime;
    char *readoutBuffer = NULL;
    uint32_t BufferSize, NumEvents_t;
    ERROR_CODES ErrCode= ERR_NONE;
    char *EventPtr = NULL;

    //my own
    QTimer *loopTimer;
    bool viewInPercents;
    int extremum[32];                           // i believe mr.mingw will initialize my sweet array with zero's.
    int extremumOffset[32];                     // offSet?
    int extremumCalibrated[32];     // extremum - offset
    QVector<double> percents;
    QVector<double> dataToHisto;
    QVector<double> energies;

};

/* new things */
CAEN_DGTZ_TriggerPolarity_t PulsePolarity_to_TriggerPolarity(CAEN_DGTZ_PulsePolarity_t pp);
CAEN_DGTZ_ConnectionType int_to_ConnectionType(int i);
#endif /* _WAVEDUMP__H */
