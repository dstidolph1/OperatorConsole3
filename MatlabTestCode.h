#pragma once

#include <vector>
#include <atltypes.h>
//#include "mclmcrrt.h"
#include "MatlabCppSharedLib.hpp"

namespace mc = matlab::cpplib;
namespace md = matlab::data;

typedef struct
{
	double x;
	double y;
	double mtf50;
} FullChartMTF50Data;

typedef struct
{
	double x;
	double y;
	double meanIntensity;
	double RMSNoise;
	double SignalToNoise;
} FullChartSNRData;

typedef struct
{
	double x;
	double y;
	double mtf50;
} QuickMTF50Data;

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
	bool RunTestFullChartMTF50(std::vector<uint16_t>& image, int width, int height, const std::vector<CPoint>& registrationCoordinates, std::vector<FullChartMTF50Data> &output);
	bool RunTestFullChartSNR(std::vector<uint16_t>& image, int width, int height, const std::vector<CPoint>& registrationCoordinates, std::vector<FullChartSNRData> &output);
	bool RunTestQuickMTF50(std::vector<uint8_t>& image, int width, int height, const std::vector<CPoint> &registrationCoordinates, std::vector<QuickMTF50Data> &output);
	
};