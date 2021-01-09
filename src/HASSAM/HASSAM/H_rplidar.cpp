/*
 *  RPLIDAR
 *  Ultra Simple Data Grabber Demo App
 *
 *  Copyright (c) 2009 - 2014 RoboPeak Team
 *  http://www.robopeak.com
 *  Copyright (c) 2014 - 2019 Shanghai Slamtec Co., Ltd.
 *  http://www.slamtec.com
 *
 */
 /*
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  *
  */

#include "H_rplidar.h"

bool checkRPLIDARHealth(RPlidarDriver* drv)
{
	u_result     op_result;
	rplidar_response_device_health_t healthinfo;

	op_result = drv->getHealth(healthinfo);

	if (IS_OK(op_result))
	{ // the macro IS_OK is the preperred way to judge whether the operation is succeed.
		printf("RPLidar health status : %d\n", healthinfo.status);

		if (healthinfo.status == RPLIDAR_STATUS_ERROR)
		{
			fprintf(stderr, "Error, rplidar internal error detected. Please reboot the device to retry.\n");
			// enable the following code if you want rplidar to be reboot by software
			// drv->reset();
			return false;
		}

		else
			return true;
	}

	else
	{
		fprintf(stderr, "Error, cannot retrieve the lidar health code: %x\n", op_result);
		return false;
	}
}

#include <signal.h>
bool ctrl_c_pressed;

void ctrlc(int)
{
	ctrl_c_pressed = true;
}

int H_rplidar(int argc, const char* argv[], int phi, FILE* fp)
{
	const char* opt_com_path = NULL;
	_u32         baudrateArray[2] = { 115200, 256000 };
	_u32         opt_com_baudrate = 0;
	u_result     op_result;

	bool useArgcBaudrate = false;

	int flag = 0;

	float theta_prev = 0;

	printf("Ultra simple LIDAR data grabber for RPLIDAR.\n"
		"Version: " RPLIDAR_SDK_VERSION "\n");

	// read serial port from the command line...
	if (argc > 1)
		opt_com_path = argv[1]; // or set to a fixed value: e.g. "com3" 

	// read baud rate from the command line if specified...
	if (argc > 2)
	{
		opt_com_baudrate = strtoul(argv[2], NULL, 10);
		useArgcBaudrate = true;
	}

	if (!opt_com_path)
	{
#ifdef _WIN32
		// use default com port
		opt_com_path = "\\\\.\\com7";
#elif __APPLE__
		opt_com_path = "/dev/tty.SLAB_USBtoUART";
#else
		opt_com_path = "/dev/ttyUSB0";
#endif
	}

	// create the driver instance
	RPlidarDriver* drv = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);

	if (!drv)
	{
		fprintf(stderr, "insufficent memory, exit\n");
		exit(-2);
	}

	rplidar_response_device_info_t devinfo;
	bool connectSuccess = false;
	// make connection...

	if (useArgcBaudrate)
	{
		if (!drv)
			drv = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);

		if (IS_OK(drv->connect(opt_com_path, opt_com_baudrate)))
		{
			op_result = drv->getDeviceInfo(devinfo);

			if (IS_OK(op_result))
				connectSuccess = true;

			else
			{
				delete drv;
				drv = NULL;
			}
		}
	}
	else
	{
		size_t baudRateArraySize = (sizeof(baudrateArray)) / (sizeof(baudrateArray[0]));
		for (size_t i = 0; i < baudRateArraySize; ++i)
		{
			if (!drv)
				drv = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);

			if (IS_OK(drv->connect(opt_com_path, baudrateArray[i])))
			{
				op_result = drv->getDeviceInfo(devinfo);

				if (IS_OK(op_result))
				{
					connectSuccess = true;
					break;
				}
				else
				{
					delete drv;
					drv = NULL;
				}
			}
		}
	}

	if (!connectSuccess)
	{
		fprintf(stderr, "Error, cannot bind to the specified serial port %s.\n", opt_com_path);
		goto on_finished;
	}

	// print out the device serial number, firmware and hardware version number..
	printf("RPLIDAR S/N: ");

	for (int pos = 0; pos < 16; ++pos)
		printf("%02X", devinfo.serialnum[pos]);

	printf("\n"
		"Firmware Ver: %d.%02d\n"
		"Hardware Rev: %d\n"
		, devinfo.firmware_version >> 8
		, devinfo.firmware_version & 0xFF
		, (int)devinfo.hardware_version);



	// check health...
	if (!checkRPLIDARHealth(drv))
		goto on_finished;

	signal(SIGINT, ctrlc);

	drv->startMotor();
	// start scan...
	drv->startScan(0, 1);


	//���� data ���
	// fetech result and print it out...
	while (flag < 35)
	{
		rplidar_response_measurement_node_hq_t nodes[8192];
		size_t   count = _countof(nodes);

		op_result = drv->grabScanDataHq(nodes, count);

		if (IS_OK(op_result))
		{
			drv->ascendScanData(nodes, count);

			for (int pos = 0; pos < (int)count; ++pos)
			{
				float theta = (nodes[pos].angle_z_q14 * 90.f / (1 << 14)), dist = nodes[pos].dist_mm_q2 / 4.0f;
				printf("%s theta: %03.2f Dist: %08.2f Q: %d \n",
					(nodes[pos].flag & RPLIDAR_RESP_MEASUREMENT_SYNCBIT) ? "S " : "  ",
					theta, dist,
					nodes[pos].quality);

				if (theta - theta_prev < 0)
					flag++;

				if (flag && nodes[pos].quality && dist < 600 && ((theta < 60) || (theta > 13)))
				{
					float theta_rad = theta * PI / 180;
					float phi_rad = phi * PI / 180;
					// r = 7.3cm = 73mm
					float new_dist = sqrt(dist * dist + 4 * 73 * 73 * std::sin(phi_rad / 2) * std::sin(phi_rad / 2) - 2 * dist * 73 * std::sin(phi_rad));
					float y = polar_to_cartesian_y(phi_rad, theta_rad, new_dist / 10);
					float x = polar_to_cartesian_x(phi_rad, theta_rad, new_dist / 10);
					if (y < 20 && 1 < y && x > 0)
						//fprintf(fp, "%03.2f %03.2f\n", polar_to_cartesian_x(phi_rad, theta_rad, new_dist / 10), polar_to_cartesian_z(phi_rad, theta_rad, new_dist / 10));
						fprintf(fp, "%03.2f %03.2f %03.2f\n", x, y, polar_to_cartesian_z(phi_rad, theta_rad, new_dist / 10));
				}
					theta_prev = theta;
			}

			if (ctrl_c_pressed)
				break;
		}
	}
	flag = 0;
	drv->stop();
	drv->stopMotor();


	// done!
on_finished:
	RPlidarDriver::DisposeDriver(drv);
	drv = NULL;

	return 0;
}

float polar_to_cartesian_x(float phi, float theta, float dist)
{
	return (dist * std::sin(theta));
}

float polar_to_cartesian_y(float phi, float theta, float dist)
{
	return (dist * std::cos(theta) * std::cos(phi));
}

float polar_to_cartesian_z(float phi, float theta, float dist)
{
	return (dist * std::cos(theta) * std::sin(phi));
}