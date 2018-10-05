//******************************************************************************
//  @file AppMain.cpp
//  @author Nicolai Shlapunov
//
//  @details Application: Main file, header
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
#include "DevCfg.h"
#include "DisplayDrv.h"
#include "InputDrv.h"

#include "Application.h"

// *****************************************************************************
// ***   Main function   *******************************************************
// *****************************************************************************
extern "C" void AppMain(void)
{
  // Init Display Driver Task
  DisplayDrv::GetInstance().InitTask();
  // Init Input Driver Task
  InputDrv::GetInstance().InitTask(nullptr, &hadc2);
  // Init Sound Driver Task
  SoundDrv::GetInstance().InitTask(&htim4);

  // Init Application Task
  Application::GetInstance().InitTask();
}

// *****************************************************************************
// ***   Stack overflow hook function   ****************************************
// *****************************************************************************
extern "C" void vApplicationStackOverflowHook(TaskHandle_t* px_task, signed portCHAR* pc_task_name)
{
  while(1);
}

// *****************************************************************************
// ***   Malloc failed hook function   *****************************************
// *****************************************************************************
extern "C" void vApplicationMallocFailedHook(void)
{
  while(1);
}
