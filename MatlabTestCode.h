#pragma once

#include <vector>
#include <atltypes.h>
//#include "mclmcrrt.h"
#include "MatlabCppSharedLib.hpp"

namespace mc = matlab::cpplib;
namespace md = matlab::data;

class MatlabTestCode
{
protected:
	std::shared_ptr<mc::MATLABApplication> m_matlabApplication;
	std::unique_ptr<mc::MATLABLibrary> m_matlabLibrary;

public:
	MatlabTestCode();
	~MatlabTestCode();

	bool Initialize();
	void Shutdown();
	bool RunTestFullChartMTF50(std::vector<uint8_t>& image, int width, int height, std::vector<CPoint> &coordinates, std::vector<int> &outputMTF50);
	bool RunTestFullChartSNR(std::vector<uint8_t>& image, int width, int height, std::vector<float> &outputMeanIntensity, std::vector<float> &outputRMSNoise, std::vector<float> &outputSignalToNoise);
	bool RunTestQuickMTF50(std::vector<uint8_t>& image, int width, int height, std::vector<CPoint> &outputCoordinates, std::vector<int> &outputMTF50);
	
};