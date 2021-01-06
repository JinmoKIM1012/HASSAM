#include <stdio.h>
#include "H_dynamixel.h"
#include "H_rplidar.h"

#define MAX_CHANGE 100
#define INITIAL 450

int main(int argc, const char* argv[])
{
	FILE* fp = fopen("HASSAM_output.txt", "w");

	H_dynamixel(INITIAL);
	for (int i = 1; i <= MAX_CHANGE; i += 20)
	{
		if (H_dynamixel(INITIAL + i))
			exit(0);
		float phi = (((INITIAL + i) - 512.0) * 150.0) / 512.0;
		H_rplidar(argc, argv, phi, fp);
	}

	H_dynamixel(INITIAL);

	fclose(fp);

	return 0;
}

