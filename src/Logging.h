#pragma once

#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>

using namespace log4cplus;

class LogUtil
{
public:
	static void Setup();
	static Logger Get();
};

#define LOG_TRACE(text) LOG4CPLUS_TRACE(LogUtil::Get(), text)
#define LOG_DEBUG(text) LOG4CPLUS_DEBUG(LogUtil::Get(), text)
#define LOG_INFO(text) LOG4CPLUS_INFO(LogUtil::Get(), text)
#define LOG_WARN(text) LOG4CPLUS_WARN(LogUtil::Get(), text)
#define LOG_ERROR(text) LOG4CPLUS_ERROR(LogUtil::Get(), text)
#define LOG_FATAL(text) LOG4CPLUS_FATAL(LogUtil::Get(), text)

#define LOG_TRACEF(logFmt, ...) LOG4CPLUS_TRACE_FMT(LogUtil::Get(), logFmt, __VA_ARGS__)
#define LOG_DEBUGF(logFmt, ...) LOG4CPLUS_DEBUG_FMT(LogUtil::Get(), logFmt, __VA_ARGS__)
#define LOG_INFOF(logFmt, ...) LOG4CPLUS_INFO_FMT(LogUtil::Get(), logFmt, __VA_ARGS__)
#define LOG_WARNF(logFmt, ...) LOG4CPLUS_WARN_FMT(LogUtil::Get(), logFmt, __VA_ARGS__)
#define LOG_ERRORF(logFmt, ...) LOG4CPLUS_ERROR_FMT(LogUtil::Get(), logFmt, __VA_ARGS__)
#define LOG_FATALF(logFmt, ...) LOG4CPLUS_FATAL_FMT(LogUtil::Get(), logFmt, __VA_ARGS__)
