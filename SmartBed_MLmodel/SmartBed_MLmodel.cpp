// SmartBed_MLmodel.cpp : Defines the entry point for the application.
//
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include "SmartBed_MLmodel.h"

using namespace std;

std::pair<std::string, std::vector<int>> processPressureValues(const std::string& line)
{
	std::string time;
	std::vector<int> pres_decimal_arr;

	size_t time_begin = line.find('[');
	size_t time_end = line.find(']');
	if (time_begin != std::string::npos && time_end != std::string::npos)
	{
		time = line.substr(time_begin + 1, time_end - time_begin - 1);
		std::string values = line.substr(time_end + 1);

		if (values.substr(0, 4) == "AA23" && values.substr(values.length() - 2) == "55")
		{
			std::string pres_hex_values = values.substr(4, values.length() - 6);

			if (pres_hex_values.length() == 64)
			{
				for (size_t i = 0; i < pres_hex_values.length(); i += 4)
				{
					std::string swapped = pres_hex_values.substr(i + 2, 2) + pres_hex_values.substr(i, 2);
					pres_decimal_arr.push_back(4095 - static_cast<int>(std::strtol(swapped.c_str(), nullptr, 16)));
				}
			}
		}
	}

	return { time, pres_decimal_arr };
}

std::pair<std::string, std::vector<int>> processSleepValues(const std::string& line)
{
	std::string time;
	std::vector<int> pres_decimal_arr;

	size_t time_begin = line.find('[');
	size_t time_end = line.find(']');
	if (time_begin != std::string::npos && time_end != std::string::npos)
	{
		time = line.substr(time_begin + 1, time_end - time_begin - 1);
		std::string values = line.substr(time_end + 1);
		if (values.substr(0, 4) == "AB11" && values.substr(values.length() - 2) == "55")
		{
			std::string pres_hex_values = values.substr(4, values.length() - 6);

			if (pres_hex_values.length() == 28)
			{
				std::string processed_hex_values = pres_hex_values.substr(0, 12);

				for (size_t i = 12; i < 20; i += 4)
				{
					processed_hex_values += pres_hex_values.substr(i + 2, 2) + pres_hex_values.substr(i, 2);
				}

				processed_hex_values += pres_hex_values.substr(20, 2)
					+ pres_hex_values.substr(24, 2) + pres_hex_values.substr(22, 2)
					+ pres_hex_values.substr(26, 2);

				for (size_t i = 0; i < 12; i += 2)
				{
					pres_decimal_arr.push_back(static_cast<int>(std::strtol(processed_hex_values.substr(i, 2).c_str(), nullptr, 16)));
				}

				pres_decimal_arr.push_back(static_cast<int>(std::strtol(processed_hex_values.substr(12, 4).c_str(), nullptr, 16)));
				pres_decimal_arr.push_back(static_cast<int>(std::strtol(processed_hex_values.substr(16, 4).c_str(), nullptr, 16)));
				pres_decimal_arr.push_back(static_cast<int>(std::strtol(processed_hex_values.substr(20, 2).c_str(), nullptr, 16)));
				pres_decimal_arr.push_back(static_cast<int>(std::strtol(processed_hex_values.substr(22, 4).c_str(), nullptr, 16)));
				pres_decimal_arr.push_back(static_cast<int>(std::strtol(processed_hex_values.substr(26, 2).c_str(), nullptr, 16)));
			}
		}
	}

	return { time, pres_decimal_arr };
}

void test_for_process_values(const std::map<std::string, std::vector<int>>& test_map)
{
	std::string test_time = "16:07:59.569";

	auto it = test_map.find(test_time);
	if (it != test_map.end())
	{
		std::cout << "Values for " << test_time << ": ";
		for (int val : it->second)
		{
			std::cout << val << " ";
		}
		std::cout << std::endl;
	}
	else
	{
		std::cout << "No values found for " << test_time << std::endl;
	}
}

std::vector<std::vector<std::vector<int>>> changeDimension(const std::vector<int>& input_data, const std::vector<int>& input_sleep_data)
{
	std::vector<std::vector<std::vector<int>>> new_input_data(12, std::vector<std::vector<int>>(32, std::vector<int>(64, 0)));  // [12, 32, 64]

	for (int ch = 0; ch < 11; ++ch)
	{
		for (int i = 0; i < 32; ++i)
		{
			for (int j = 0; j < 64; ++j)
			{
				new_input_data[ch][i][j] = input_sleep_data[ch];
			}
		}
	}

	for (int j = 0; j < 16; ++j)
	{
		for (int i = 0; i < 32; ++i)
		{
			for (int k = 0; k < 4; ++k)
			{
				new_input_data[11][i][j * 4 + k] = input_data[j];
			}
		}
	}

	return new_input_data;
}

void print3DVector(const std::vector<std::vector<std::vector<int>>>& vec)
{
    for (size_t i = 0; i < vec.size(); ++i)
	{
        std::cout << "Channel " << i << ":" << std::endl;
        for (size_t j = 0; j < vec[i].size(); ++j)
		{
            for (size_t k = 0; k < vec[i][j].size(); ++k)
			{
                std::cout << vec[i][j][k] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}


int main()
{
	std::string line0 = "[16:07:59.569]AA23F40FFF0FFF0F3B0FFF0FFF0FFF0F8E0F280D64086B0CBC05140B7F0C4F0AFF0F55";
	auto [time, values] = processPressureValues(line0);
	std::map<std::string, std::vector<int>> pressure_map;
	pressure_map[time] = values;

	std::string line0_sleep = "[16:07:59.789]AB11450C00000001861B060000710A1955";
	auto [time_sleep, values_sleep] = processSleepValues(line0_sleep);
	std::map<std::string, std::vector<int>> sleep_map;
	sleep_map[time_sleep] = values_sleep;

	//test_for_process_values(pressure_map);

	auto new_input_data = changeDimension(values, values_sleep);
	print3DVector(new_input_data);

	return 0;
}
