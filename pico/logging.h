#ifndef __PICO_LOGGING_H__
#define __PICO_LOGGING_H__

#include "log/LoggerManager.h"
#include "log/PatternLayout.h"
#include "log/SimpleLayout.h"
#include "log/TTCCLayout.h"
#include "singleton.h"

typedef pico::Singleton<pico::LoggerManager> LoggerMgr;

#endif