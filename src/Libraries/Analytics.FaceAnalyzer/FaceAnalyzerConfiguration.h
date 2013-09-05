/*
	The class defines al the configuration data required for the face analyzer to run 
	(some of the data is optional)

	Date: 06/07/2013

	Author: Jason Catchpole (jason@viblio.com)

*/

#ifndef ___FACEANALYZER_CONFIG_H__
#define ___FACEANALYZER_CONFIG_H__

#include "AnalyzerConfig.h"

namespace Analytics
{
	namespace FaceAnalyzer
	{

class FaceAnalyzerConfiguration : public AnalyzerConfig
{
private:
	// no copy constructor or assignment operator
	FaceAnalyzerConfiguration(const FaceAnalyzerConfiguration&);
	FaceAnalyzerConfiguration& operator=(const FaceAnalyzerConfiguration&);

public:
	FaceAnalyzerConfiguration();
	~FaceAnalyzerConfiguration();

	// so we can log all the parameters used for this job
	std::string ToString();

	// the file path for the face detector cascade file to use
	std::string faceDetectorCascadeFile;

	// the file path for the face detector cascade file to use
	std::string eyeDetectorCascadeFile;

	// a path to the location where to write the thumbnails that are extracted
	std::string faceThumbnailOutputPath;

	// how often we apply face detection. Essentially how many frames are skipped before reapplying recognition.
	// Higher values will mean faster processing of the video but with a higher chance of missing detection of a face
	int faceDetectionFrequency;

	// if enabled visualizations of the detection, tracking and recognition will be displayed during processing
	bool renderVisualization;
};


// end of namespaces
	}
}

#endif