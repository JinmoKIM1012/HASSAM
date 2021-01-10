#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <fstream>
#include <cmath>

int main()
{
	std::ifstream readFile;
	std::vector<std::vector<int>> x(9), y(9), z(9);
	std::string buffer;
	std::string delimiter = " ";
	int i = 0;

	readFile.open("test.txt");
	if (readFile.is_open())
	{
		while (!readFile.eof())
		{
			std::getline(readFile, buffer);
			if (!buffer.compare("\n"))
			{
				i++;
				continue;
			}
			size_t pos = 0;

			std::vector<int> p;
			while ((pos = buffer.find(delimiter)) != std::string::npos) {
				p.push_back(std::stoi(buffer.substr(0, pos));
				buffer.erase(0, pos + delimiter.length());
			}
			if (p.size() == 3)
			{
				x[i].push_back(p[0]);
				y[i].push_back(p[1]);
				z[i].push_back(p[2]);
			}
		}
	}

	std::vector<int> x_round(3000, 0);
	std::vector<double> z_average(9, 0);
	std::vector<double> x_res;
	int window;

	for (int i = 0; i < 9; i++)
	{
		int tmp = 0;
		for (auto it = z[i].begin(); it != z[i].end(); it++)
			tmp += *it;
		z_average[i] = tmp / z[i].size();

		window = std::round(std::sqrt(800 * 800 - z_average[i] * z_average[i]);
		for (auto it = x[i].begin(); it != x[i].end(); it++)
			x_round[*it]++;
	
		int sum = 0;
	
		for (int j = 0; j < window; j++)
			sum += x_round[i];
	
		int min = sum, min_s = 0, min_f = window;

		for (int j = 1; j <= 3000 - window; j++)
		{
			sum += x_round[j + window - 1] - x_round[j - 1];
			if (min > sum)
			{
				min = sum;
				min_s = j;
				min_f = j + window;
			}
		}

		x_res.push_back((min_s + min_f) / 2.0);
	}

	std::vector<int> sum_y(9, 0);
	double y_res_coor = 0;
	
	for (int i = 0; i < 9; i++)
	{
		for (auto it = y[i].begin(); it != y[i].end(); it++)
			sum_y[i] += *it;
		y_res_coor += sum_y[i] / y[i].size();
	}
	y_res_coor /= 9;
	
	double x_res_coor = 0;
	for (auto it = x_res.begin(); it != x_res.end(); it++)
		x_res_coor += *it;
	x_res_coor /= x_res.size();

	std::cout << "x : " << x_res_coor << " y : " << y_res_coor << " z : 0" << std::endl;
	
	return 0;
}