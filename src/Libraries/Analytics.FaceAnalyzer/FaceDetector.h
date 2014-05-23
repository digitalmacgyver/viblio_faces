#ifndef __FACEDETECTOR__
#define __FACEDETECTOR__
#include "FaceDetectionDetails.h"

#include "Frame.h"

namespace Analytics
{
	namespace FaceAnalyzer
	{

// forward declarations
class FaceAnalyzerConfiguration;

class FaceDetector
{

public:
	enum DetectorType
	{
		FaceDetector_Neurotech,
		FaceDetector_Orbeus,
		FaceDetector_OpenCV,
		FaceDetector_Unknown
	};

	static DetectorType ConvertStringToDetectorType(const std::string &detectorType);

protected:
	cv::Rect ConstrainRect(const cv::Rect &rectToConstrain, const cv::Size &imageSize);
//
//
//
//
//	// no copy constructor or assignment operator
//	FaceDetector_Neurotech(const FaceDetector_Neurotech&);
//	FaceDetector_Neurotech& operator=(const FaceDetector_Neurotech&);
//

public:
	
	FaceDetector(FaceAnalyzerConfiguration *faceAnalyzerConfiguration) {};
	virtual ~FaceDetector() {};
	virtual std::vector<FaceDetectionDetails> Detect(const cv::Mat &frame, bool getDetailedInformation=false) = 0;
	
	void RenderVisualization(cv::Mat &frame, const std::vector<FaceDetectionDetails> &detectedFaces);
};

	}
}
#endif

