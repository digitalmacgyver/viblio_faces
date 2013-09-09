#include "FaceAnalyzerConfiguration.h"

using namespace std;

namespace Analytics
{
	namespace FaceAnalyzer
	{

FaceAnalyzerConfiguration::FaceAnalyzerConfiguration()
{
	// set default values for the configuration options
	faceDetectionFrequency = 3;

	renderVisualization = false;
}


FaceAnalyzerConfiguration::~FaceAnalyzerConfiguration()
{
}

string FaceAnalyzerConfiguration::ToString()
{
	return ""; // not yet implemented
}

// end of namespaces
	}
}