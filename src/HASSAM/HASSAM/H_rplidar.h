#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <cmath>

#include "rplidar.h" //RPLIDAR standard sdk, all-in-one header

#ifndef _countof
#define _countof(_Array) (int)(sizeof(_Array) / sizeof(_Array[0]))
#endif

#ifdef _WIN32
#include <Windows.h>
#define delay(x)   ::Sleep(x)
#else
#include <unistd.h>
static inline void delay(_word_size_t ms)
{
	while (ms >= 1000)
	{
		usleep(1000 * 1000);
		ms -= 1000;
	};

	if (ms != 0)
		usleep(ms * 1000);
}
#endif


using namespace rp::standalone::rplidar;

bool checkRPLIDARHealth(RPlidarDriver* drv);

float polar_to_cartesian_x(float pi, float theta, float dist);
float polar_to_cartesian_y(float pi, float theta, float dist);
float polar_to_cartesian_z(float pi, float theta, float dist);

int H_rplidar(int argc, const char* argv[], int i, FILE* fp);
