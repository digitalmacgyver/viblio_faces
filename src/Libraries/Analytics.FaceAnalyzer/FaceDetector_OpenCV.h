/*
	OpenCV based face detector.

	Date: 25/07/2013

	Author: Jason Catchpole (jason@viblio.com)
*/

#ifndef __FACEDETECTOR_OPENCV__
#define __FACEDETECTOR_OPENCV__

#include <memory>
#include <opencv2/objdetect/objdetect.hpp>
#include <string>
#include "FaceDetector.h"

namespace Analytics
{
	namespace FaceAnalyzer
	{

// forward prototypes of used classes
class EyeDetector_OpenCV;
class FaceAnalyzerConfiguration;

class FaceDetector_OpenCV : public FaceDetector
{
private:
	cv::CascadeClassifier m_faceCascade;

	bool m_filterFacesByEyeDetections;
	std::unique_ptr<EyeDetector_OpenCV> m_eyeDetector;

	// no copy constructor or assignment operator
	FaceDetector_OpenCV(const FaceDetector_OpenCV&);
	FaceDetector_OpenCV& operator=(const FaceDetector_OpenCV&);

public:
	FaceDetector_OpenCV(FaceAnalyzerConfiguration *faceAnalyzerConfiguration);
	~FaceDetector_OpenCV();

	/*
		Performs detection on the opencv frame passed in. Returns a vector of face detection detail structures with the locations of any faces detected (empty if no faces detected)
	*/
	std::vector<FaceDetectionDetails> Detect(const cv::Mat &frame, bool getDetailedInformation=false);
};

// end of namespaces
	}
}
#endif
