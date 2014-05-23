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

	lostFaceProcessFrequency = 5;

	maximumNumberActiveTrackers = 15;

	renderVisualization = false;

	faceDetectorType = FaceDetector::FaceDetector_Neurotech; // default is the Neurotech detector
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