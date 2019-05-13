//******************************************************************************
//  @file Application.cpp
//  @author Nicolai Shlapunov
//
//  @details Application: User Application Class, implementation
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

// *****************************************************************************
// ***   Includes   ************************************************************
// *****************************************************************************
#include "Application.h"

#include <cmath>

#include "Images.h"

// *****************************************************************************
// ***   Get Instance   ********************************************************
// *****************************************************************************
Application& Application::GetInstance(void)
{
   static Application application;
   return application;
}

// *****************************************************************************
// ***   Application Loop   ****************************************************
// *****************************************************************************
Result Application::Loop()
{
  Result result;

  // Create and show UI
  int32_t half_scr_w = display_drv.GetScreenW() / 2;
  int32_t half_scr_h = display_drv.GetScreenH() / 2;
  for(uint32_t i = 0U; i < CHANNEL_CNT; i++)
  {
    // Generator data
    ch_dsc[i].frequency = 1000U * (i + 1U);
    if(IsAnalogChannel(i))
    {
      ch_dsc[i].duty = 100U;
      ch_dsc[i].waveform = WAVEFORM_SINE;
    }
    else
    {
      ch_dsc[i].duty = 50U;
      ch_dsc[i].waveform = WAVEFORM_SQUARE;
    }
    // UI data
    int32_t start_pos_x = half_scr_w * (i%2);
    int32_t start_pos_y = half_scr_h * (i/2);
    ch_dsc[i].box.SetParams(nullptr, start_pos_x, start_pos_y, half_scr_w, half_scr_h, true);
    ch_dsc[i].box.SetCallback(&Callback, this, nullptr, i);
    ch_dsc[i].freq_str.SetParams(ch_dsc[i].freq_str_data, start_pos_x + 4, start_pos_y + 64, COLOR_LIGHTGREY, Font_8x12::GetInstance());
    ch_dsc[i].duty_str.SetParams(ch_dsc[i].duty_str_data, start_pos_x + 4, start_pos_y + 64 + 12, COLOR_LIGHTGREY, Font_8x12::GetInstance());
    ch_dsc[i].img.SetImage(waveforms[ch_dsc[i].waveform]);
    ch_dsc[i].img.Move(start_pos_x + 4, start_pos_y + 4);
    ch_dsc[i].box.Show(1);
    ch_dsc[i].img.Show(2);
    ch_dsc[i].freq_str.Show(3);
    ch_dsc[i].duty_str.Show(3);
  }

  // ***************************************************************************
  // ***   CHANNEL 1 (DAC)   ***************************************************
  // ***************************************************************************
  SetupDac(hdac, DAC_CHANNEL_2, htim7, ch_dsc[0U].frequency, ch_dsc[0U].duty, ch_dsc[0U].waveform);

  // ***************************************************************************
  // ***   CHANNEL 2 (DAC)   ***************************************************
  // ***************************************************************************
  SetupDac(hdac, DAC_CHANNEL_1, htim6, ch_dsc[1U].frequency, ch_dsc[1U].duty, ch_dsc[1U].waveform);

  // ***************************************************************************
  // ***   CHANNEL 3 (PWM)   ***************************************************
  // ***************************************************************************
  SetupPwm(htim5, TIM_CHANNEL_4, ch_dsc[2U].frequency, ch_dsc[2U].duty);

  // ***************************************************************************
  // ***   CHANNEL 4 (PWM)   ***************************************************
  // ***************************************************************************
  SetupPwm(htim2, TIM_CHANNEL_3, ch_dsc[3U].frequency, ch_dsc[3U].duty);

  // Main cycle
  while(1)
  {
    // ***************************************************************************
    // ***   Process user input   ************************************************
    // ***************************************************************************

    // Change channel
    if(input_drv.GetEncoderButtonState(InputDrv::EXT_LEFT, InputDrv::ENC_BTN_ENT, enc_btn_val[InputDrv::EXT_LEFT][InputDrv::ENC_BTN_ENT]) && enc_btn_val[InputDrv::EXT_LEFT][InputDrv::ENC_BTN_ENT])
    {
      // Process selected channel change
      channel = (ChannelType)(channel + 1U);
      if(channel >= CHANNEL_CNT) channel = CHANNEL_1;
      // Set flag for update
      update = true;
    }
    // Change waveform
    if(input_drv.GetEncoderButtonState(InputDrv::EXT_RIGHT, InputDrv::ENC_BTN_ENT, enc_btn_val[InputDrv::EXT_RIGHT][InputDrv::ENC_BTN_ENT]) && enc_btn_val[InputDrv::EXT_RIGHT][InputDrv::ENC_BTN_ENT])
    {
      if(IsAnalogChannel(channel))
      {
        ch_dsc[channel].waveform = (WaveformType)(ch_dsc[channel].waveform + 1U);
        if(ch_dsc[channel].waveform >= WAVEFORM_CNT) ch_dsc[channel].waveform = WAVEFORM_SINE;
      }
      else
      {
        ch_dsc[channel].waveform = WAVEFORM_SQUARE;
      }
      // Set flag for update
      update = true;
    }

    // Get encoder 1 count since last call and pass it to the function
    update |= ProcessFrequencyChange(input_drv.GetEncoderState(InputDrv::EXT_LEFT));

    // Get encoder 2 count since last call and pass it to the function
    update |= ProcessDutyChange(input_drv.GetEncoderState(InputDrv::EXT_RIGHT));

    // ***************************************************************************
    // ***   Update UI and generator if needed   *********************************
    // ***************************************************************************
    if(update == true)
    {
      for(uint32_t i = 0U; i < CHANNEL_CNT; i++)
      {
        ch_dsc[i].img.SetImage(waveforms[ch_dsc[i].waveform]);
        snprintf(ch_dsc[i].freq_str_data, NumberOf(ch_dsc[i].freq_str_data), "Freq: %7lu Hz", ch_dsc[i].frequency);
        if(IsAnalogChannel(i)) snprintf(ch_dsc[i].duty_str_data, NumberOf(ch_dsc[i].duty_str_data), "Ampl: %7d %%", ch_dsc[i].duty);
        else                   snprintf(ch_dsc[i].duty_str_data, NumberOf(ch_dsc[i].duty_str_data), "Duty: %7d %%", ch_dsc[i].duty);
        // Set gray color to all channels
        ch_dsc[i].freq_str.SetColor(COLOR_LIGHTGREY);
        ch_dsc[i].duty_str.SetColor(COLOR_LIGHTGREY);
      }
      // Set white color to selected channel
      ch_dsc[channel].freq_str.SetColor(COLOR_WHITE);
      ch_dsc[channel].duty_str.SetColor(COLOR_WHITE);
      // Update display
      display_drv.UpdateDisplay();

      // Set duty cycle
      switch(channel)
      {
        // *************************************************************************
        // ***   CHANNEL 1 (DAC)   *************************************************
        // *************************************************************************
        case CHANNEL_1:
          SetupDac(hdac, DAC_CHANNEL_2, htim7, ch_dsc[channel].frequency, ch_dsc[channel].duty, ch_dsc[channel].waveform);
          break;

        // *************************************************************************
        // ***   CHANNEL 2 (DAC)   *************************************************
        // *************************************************************************
        case CHANNEL_2:
          SetupDac(hdac, DAC_CHANNEL_1, htim6, ch_dsc[channel].frequency, ch_dsc[channel].duty, ch_dsc[channel].waveform);
          break;

        // *************************************************************************
        // ***   CHANNEL 3 (PWM)   *************************************************
        // *************************************************************************
        case CHANNEL_3:
          SetupPwm(htim5, TIM_CHANNEL_4, ch_dsc[channel].frequency, ch_dsc[channel].duty);
          break;

        // *************************************************************************
        // ***   CHANNEL 4 (PWM)   *************************************************
        // *************************************************************************
        case CHANNEL_4:
          SetupPwm(htim2, TIM_CHANNEL_3, ch_dsc[channel].frequency, ch_dsc[channel].duty);
          break;

        default:
          result = Result::ERR_BAD_PARAMETER;
          break;
      }
      update = false;
    }

    // Delay
    RtosTick::DelayMs(100U);
  }

  // Always run
  return Result::RESULT_OK;
}

// *****************************************************************************
// ***  Callback for the buttons   *********************************************
// *****************************************************************************
void Application::Callback(void* ptr, void* param_ptr, uint32_t param)
{
  Application& app = *((Application*)ptr);
  ChannelType channel = app.channel;
  if(channel == param)
  {
    // Second click - change wave type
    if(IsAnalogChannel(channel))
    {
      app.ch_dsc[channel].waveform = (WaveformType)(app.ch_dsc[channel].waveform + 1U);
      if(app.ch_dsc[channel].waveform >= WAVEFORM_CNT) app.ch_dsc[channel].waveform = WAVEFORM_SINE;
    }
    else
    {
      app.ch_dsc[channel].waveform = WAVEFORM_SQUARE;
    }
  }
  else
  {
    app.channel = (ChannelType)param;
  }
  app.update = true;
}

// *****************************************************************************
// ***   ProcessFrequencyChange   **********************************************
// *****************************************************************************
bool Application::ProcessFrequencyChange(int32_t steps)
{
  bool result = false;

  // Change Frequency
  if(steps != 0)
  {
    // Change frequency
    if(ch_dsc[channel].frequency >= 1000000)
    {
      ch_dsc[channel].frequency += steps * 100000;
    }
    else if(ch_dsc[channel].frequency >= 100000)
    {
      ch_dsc[channel].frequency += steps * 10000;
      if(ch_dsc[channel].frequency > 1000000) ch_dsc[channel].frequency = 1000000;
    }
    else if(ch_dsc[channel].frequency >= 10000)
    {
      ch_dsc[channel].frequency += steps * 1000;
      if(ch_dsc[channel].frequency > 100000) ch_dsc[channel].frequency = 100000;
    }
    else
    {
      ch_dsc[channel].frequency += steps * 100;
      if(ch_dsc[channel].frequency > 10000) ch_dsc[channel].frequency = 10000;
    }
    // Check absolute minimum
    if(ch_dsc[channel].frequency < 100) ch_dsc[channel].frequency = 100;
    // Check absolute maximum
    if(IsAnalogChannel(channel))
    {
      if(ch_dsc[channel].frequency > 200000) ch_dsc[channel].frequency = 200000;
    }
    else
    {
      if(ch_dsc[channel].frequency > 10000000) ch_dsc[channel].frequency = 10000000;
    }
    // Set flag for update
    result = true;
  }

  return result;
}

// *****************************************************************************
// ***   ProcessDutyChange   ***************************************************
// *****************************************************************************
bool Application::ProcessDutyChange(int32_t steps)
{
  bool result = false;

  // Change Frequency
  if(steps != 0)
  {
    int8_t max_val = 0;
    // Find maximum value
    if(IsAnalogChannel(channel))
    {
      max_val = 100;
    }
    else
    {
      max_val = 99;
    }
    // Process amplitude/duty change
    ch_dsc[channel].duty += steps;
    if(ch_dsc[channel].duty < 1) ch_dsc[channel].duty = max_val;
    if(ch_dsc[channel].duty > max_val) ch_dsc[channel].duty = 1;
    // Set flag for update
    result = true;
  }

  return result;
}

// *****************************************************************************
// ***   GenerateWave   ********************************************************
// *****************************************************************************
Result Application::GenerateWave(uint16_t* dac_data, uint32_t dac_data_cnt, uint8_t duty, WaveformType waveform)
{
  Result result;

  uint32_t max_val = (DAC_MAX_VAL * duty) / 100U;
  uint32_t shift = (DAC_MAX_VAL - max_val) / 2U;

  switch(waveform)
  {
    case WAVEFORM_SINE:
      for(uint32_t i = 0U; i < dac_data_cnt; i++)
      {
        dac_data[i] = (uint16_t)((sin((2.0F * i * PI) / (dac_data_cnt + 1)) + 1.0F) * max_val) >> 1U;
        dac_data[i] += shift;
      }
      break;

    case WAVEFORM_TRIANGLE:
      for(uint32_t i = 0U; i < dac_data_cnt; i++)
      {
        if(i <= dac_data_cnt / 2U)
        {
          dac_data[i] = (max_val * i) / (dac_data_cnt / 2U);
        }
        else
        {
          dac_data[i] = (max_val * (dac_data_cnt - i)) / (dac_data_cnt / 2U);
        }
        dac_data[i] += shift;
      }
      break;

    case WAVEFORM_SAWTOOTH:
      for(uint32_t i = 0U; i < dac_data_cnt; i++)
      {
        dac_data[i] = (max_val * i) / (dac_data_cnt - 1U);
        dac_data[i] += shift;
      }
      break;

    case WAVEFORM_SQUARE:
      for(uint32_t i = 0U; i < dac_data_cnt; i++)
      {
        dac_data[i] = (i < dac_data_cnt / 2U) ? max_val : 0x000;
        dac_data[i] += shift;
      }
      break;

    default:
      result = Result::ERR_BAD_PARAMETER;
      break;
  }

  return result;
}

// *****************************************************************************
// ***   Setup DAC   ***********************************************************
// *****************************************************************************
Result Application::SetupDac(DAC_HandleTypeDef& hdac, uint32_t channel, TIM_HandleTypeDef& htim, uint32_t freq, uint8_t duty, WaveformType waveform)
{
  Result result;

  uint16_t* dac_data = nullptr;
  uint32_t dac_data_cnt = 0U;
  uint32_t freq_sampling = 4000000U;

  if(channel == DAC_CHANNEL_1)
  {
    dac_data = dac1_data;
    dac_data_cnt = NumberOf(dac1_data);
  }
  else
  {
    dac_data = dac2_data;
    dac_data_cnt = NumberOf(dac2_data);
  }

  // Find sampling frequency
  while(freq_sampling/freq > dac_data_cnt) freq_sampling >>= 1U;
  // Find count
  dac_data_cnt = freq_sampling/freq;
  // Generate waveform
  GenerateWave(dac_data, dac_data_cnt, duty, waveform);

  // Stop timer
  (void) HAL_TIM_Base_Stop(&htim);
  // Stop DAC DMA
  (void) HAL_DAC_Stop_DMA(&hdac, channel);
  // Calculate ARR
  uint16_t arr = ((HAL_RCC_GetPCLK1Freq() * 2U) / freq_sampling) - 1U;
  // Prevent set to zero
  if(arr < 20U) arr = 20U;
  // Set period
  htim.Instance->ARR = arr;
  // Generate an update event
  htim.Instance->EGR = TIM_EGR_UG;
  // Start DAC DMA
  (void) HAL_DAC_Start_DMA(&hdac, channel, (uint32_t*)dac_data, dac_data_cnt, DAC_ALIGN_12B_R);
  // Start timer
  (void) HAL_TIM_Base_Start(&htim);

  return result;
}

// *****************************************************************************
// ***   Setup PWM   ***********************************************************
// *****************************************************************************
Result Application::SetupPwm(TIM_HandleTypeDef& htim, uint32_t channel, uint32_t freq, uint8_t duty)
{
  Result result;

  if((duty > 0U) && (duty < 100U))
  {
    // Set period
    htim.Instance->ARR = (HAL_RCC_GetPCLK1Freq() * 2U) / freq;
    // Set duty cycle
    switch(channel)
    {
      case TIM_CHANNEL_1:
        htim.Instance->CCR1 = (htim.Instance->ARR * duty) / 100U;
        break;
      case TIM_CHANNEL_2:
        htim.Instance->CCR2 = (htim.Instance->ARR * duty) / 100U;
        break;
      case TIM_CHANNEL_3:
        htim.Instance->CCR3 = (htim.Instance->ARR * duty) / 100U;
        break;
      case TIM_CHANNEL_4:
        htim.Instance->CCR4 = (htim.Instance->ARR * duty) / 100U;
        break;
      default:
        result = Result::ERR_BAD_PARAMETER;
        break;
    }
    if(result.IsGood())
    {
      // Generate an update event
      htim.Instance->EGR = TIM_EGR_UG;
      // Start timer in PWM mode
      (void) HAL_TIM_PWM_Start(&htim, channel);
    }
  }
  else
  {
    result = Result::ERR_BAD_PARAMETER;
  }

  return result;
}

