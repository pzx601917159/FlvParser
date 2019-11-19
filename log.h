/*************************************************************************
	> File Name: log.h
	> Author: pzx
	> Created Time: 2019年11月14日 星期四 20时26分31秒
************************************************************************/

#include "spdlog/include/spdlog/spdlog.h"
#include "spdlog/include/spdlog/sinks/basic_file_sink.h"


#define LOG_DEBUG(log, ...) spdlog::debug(log, ##__VA_ARGS__)
#define LOG_INFO(log, ...) spdlog::info(log, ##__VA_ARGS__)
#define LOG_ERROR(log, ...) spdlog::error(log, ##__VA_ARGS__)

