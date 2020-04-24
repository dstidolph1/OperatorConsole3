#pragma once

#include <vector>
#include <atltypes.h>

class MatlabTestCode
{
public:
	MatlabTestCode();
	~MatlabTestCode();

	bool Initialize();
	bool RunTestFullChartMTF50(std::vector<uint8_t>& image, int width, int height, std::vector<CPoint> &coordinates, std::vector<int> &outputMTF50);
	bool RunTestFullChartSNR(std::vector<uint8_t>& image, int width, int height, std::vector<float> &outputMeanIntensity, std::vector<float> &outputRMSNoise, std::vector<float> &outputSignalToNoise);
	bool RunTestQuickMTF50(std::vector<uint8_t>& image, int width, int height, std::vector<CPoint> &outputCoordinates, std::vector<int> &outputMTF50);
	
};