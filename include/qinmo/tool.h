/**
 * @file <qinmo/tool.h>
 * @brief A Simple Commoly-used Tool Library
 * @details Example: Logger, Random, println, StringView ...
 * @version 7.3.3
 * @author qinmoM
 * @github https://github.com/qinmoM/qinmo_tool
 */
#pragma once

#include "qinmo/tool/Random.h"
#include "qinmo/tool/StringView.h"
#include "qinmo/tool/Span.h"
#include "qinmo/tool/Optional.h"
#include "qinmo/tool/make_unique.h"
#include "qinmo/tool/ObjectPool.h"

#if defined(_WIN32) || defined(__linux__)
/// @note the ache line interference size may vary between compilers, compiler versions and CPU tuning options
#include "qinmo/tool/MPMCUnbounded.h"
#endif

#include "qinmo/tool/Logger.h"