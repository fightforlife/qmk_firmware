// Copyright 2023 ziptyze
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once


#pragma once

#include_next <mcuconf.h>

#undef SN32_PWM_USE_CT16B1
#define SN32_PWM_USE_CT16B1 TRUE

#undef SN32_PWM_NO_RESET
#define SN32_PWM_NO_RESET TRUE
//#define SN32_I2C_USE_I2C0 TRUE