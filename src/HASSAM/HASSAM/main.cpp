#include <stdio.h>
#include "H_dynamixel.h"
#include "H_rplidar.h"

#define MAX_CHANGE 20
#define INITIAL 350 

int main(int argc, const char* argv[])
{
	for (int i = 1; i <= MAX_CHANGE; i++)
	{
		if (!H_dynamixel(INITIAL + i))
			exit(0);
		H_rplidar(argc, argv, INITIAL + i);
	}

	H_dynamixel(INITIAL);

	return 0;
}

