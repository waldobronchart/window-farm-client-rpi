#include "Logging.h"

#include <stdio.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/loglog.h>
#include <log4cplus/helpers/stringhelper.h>

using namespace log4cplus::helpers;

Logger _root_log;

void LogUtil::Setup()
{
	log4cplus::initialize();
	//LogLog::getLogLog()->setInternalDebugging(true);
	PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("logging.properties"));
	_root_log = Logger::getRoot();
}

Logger LogUtil::Get()
{
	return _root_log;
}
