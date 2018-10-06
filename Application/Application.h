//******************************************************************************
//  @file Application.h
//  @author Nicolai Shlapunov
//
//  @details Application: User Application Class, header
//
//  @copyright Copyright (c) 2016, Devtronic & Nicolai Shlapunov
//             All rights reserved.
//
//  @section SUPPORT
//
//   Devtronic invests time and resources providing this open source code,
//   please support Devtronic and open-source hardware/software by
//   donations and/or purchasing products from Devtronic.
//
//******************************************************************************

#ifndef Application_h
#define Application_h

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "DevCfg.h"
#include "AppTask.h"
#include "DisplayDrv.h"
#include "InputDrv.h"
#include "SoundDrv.h"
#include "UiEngine.h"

#include "IIic.h"

// *****************************************************************************
// ***   Local const variables   ***********************************************
// *****************************************************************************

// *****************************************************************************
// ***   Defines   *************************************************************
// *****************************************************************************
#define BG_Z (100)

// *****************************************************************************
// ***   Application Class   ***************************************************
// *****************************************************************************
class Application : public AppTask
{
  public:
    // *************************************************************************
    // ***   Get Instance   ****************************************************
    // *************************************************************************
    static Application& GetInstance(void);

    // *************************************************************************
    // ***   Application Loop   ************************************************
    // *************************************************************************
    virtual Result Loop();

  private:

    // *************************************************************************
    // ***   Enum with all channels   ******************************************
    // *************************************************************************
    typedef enum
    {
      CHANNEL_1 = 0U,
      CHANNEL_2,
      CHANNEL_3,
      CHANNEL_4,
      CHANNEL_CNT
    } ChannelType;

    // *************************************************************************
    // ***   Enum with all waveforms   *****************************************
    // *************************************************************************
    typedef enum
    {
      WAVEFORM_SINE = 0U,
      WAVEFORM_TRIANGLE,
      WAVEFORM_SAWTOOTH,
      WAVEFORM_SQUARE,
      WAVEFORM_CNT
    } WaveformType;

    // *************************************************************************
    // ***   Structure for describes all visual elements for the channel   *****
    // *************************************************************************
    struct ChannelDescriptionType
    {
      // UI data
      UiButton box;
      Image img;
      String freq_str;
      String duty_str;
      char freq_str_data[64] = {0};
      char duty_str_data[64] = {0};
      // Generator data
      int32_t frequency;
      int8_t duty;
      WaveformType waveform;
    };
    // Visual channel descriptions
    ChannelDescriptionType ch_dsc[CHANNEL_CNT];

    // Pi
    static constexpr double PI = 3.1415926535897932384626433832795F;

    static const uint32_t DAC_MAX_VAL = 0x00000FFFU;

    // Display driver instance
    DisplayDrv& display_drv = DisplayDrv::GetInstance();
    // Input driver instance
    InputDrv& input_drv = InputDrv::GetInstance();
    // Sound driver instance
    SoundDrv& sound_drv = SoundDrv::GetInstance();

    // DAC arrays
    uint16_t dac1_data[1024U] = {0};
    uint16_t dac2_data[1024U] = {0};

    // Current selected channel
    ChannelType channel = CHANNEL_1;
    // Encoder last buttons values
    bool enc_btn_val[InputDrv::EXT_MAX][InputDrv::ENC_BTN_MAX] = {0};
    // Need update display and generator params
    bool update = true;

    // *************************************************************************
    // ***   Callback   ********************************************************
    // *************************************************************************
    static void Callback(void* ptr, void* param_ptr, uint32_t param);

    // *************************************************************************
    // ***   ProcessFrequencyChange   ******************************************
    // *************************************************************************
    bool ProcessFrequencyChange(int32_t steps);

    // *************************************************************************
    // ***   ProcessDutyChange   ***********************************************
    // *************************************************************************
    bool ProcessDutyChange(int32_t steps);

    // *************************************************************************
    // ***   GenerateWave   ****************************************************
    // *************************************************************************
    Result GenerateWave(uint16_t* dac_data, uint32_t dac_data_cnt, uint8_t duty, WaveformType waveform);

    // *************************************************************************
    // ***   Setup DAC   *******************************************************
    // *************************************************************************
    Result SetupDac(DAC_HandleTypeDef& hdac, uint32_t channel, TIM_HandleTypeDef& htim, uint32_t freq, uint8_t duty, WaveformType waveform);

    // *************************************************************************
    // ***   Setup PWM   *******************************************************
    // *************************************************************************
    Result SetupPwm(TIM_HandleTypeDef& htim, uint32_t channel, uint32_t freq, uint8_t duty);

    // *************************************************************************
    // ***   IsAnalogChannel   *************************************************
    // *************************************************************************
    static bool IsAnalogChannel(uint8_t ch) {return ((ch < 2U) ? true : false);}

    // *************************************************************************
    // ***   Private constructor   *********************************************
    // *************************************************************************
    Application() : AppTask(APPLICATION_TASK_STACK_SIZE, APPLICATION_TASK_PRIORITY,
                            "Application") {};
};

#endif
