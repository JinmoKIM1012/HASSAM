#include <stdio.h>
#include "H_dynamixel.h"
#include "H_rplidar.h"

#define MAX_CHANGE 100
#define INITIAL 450

int main(int argc, const char* argv[])
{
	int pres_pos;
	FILE* fp = fopen("C:/github/HASSAM/src/HASSAM/HASSAM/HASSAM_output88.txt", "w");

	H_dynamixel(INITIAL);
	for (int i = 1; i <= MAX_CHANGE; i += 4)
	{
		if ((pres_pos = H_dynamixel(INITIAL + i)) == -1)
			exit(0);
		float phi = (((pres_pos) - 500.0) * 150.0) / 512.0;
		H_rplidar(argc, argv, phi, fp);
	}

	H_dynamixel(INITIAL);

	fclose(fp);

	return 0;
}

