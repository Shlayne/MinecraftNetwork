#pragma once

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#if defined(_WIN32) && !defined(_WIN32_WINNT)
	#define _WIN32_WINNT 0x0A00
#endif

//#define ASIO_NO_EXCEPTIONS
#define ASIO_NO_DEPRECATED
#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
