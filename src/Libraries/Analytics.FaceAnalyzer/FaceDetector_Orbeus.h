/*
	OpenCV based face detector.

	Date: 25/07/2013

	Author: Jason Catchpole (jason@viblio.com)
*/

#ifndef __FACEDETECTOR_ORBEUS__
#define __FACEDETECTOR_ORBEUS__

#include "FaceDetector.h"

#include <memory>
#include <opencv2/objdetect/objdetect.hpp>
#include <string>
//#include "EyeDetector_OpenCV.h"

namespace Analytics
{
	namespace FaceAnalyzer
	{

// forward prototypes of used classes
class FaceAnalyzerConfiguration;

class FaceDetector_Orbeus : public FaceDetector
{
private:
	
	struct membuf : std::streambuf
	{
		membuf(char* begin, char* end) {
			this->setg(begin, begin, end);
		}
	};

	std::string m_apiKey;
	std::string m_apiSecret;
	std::string m_orbeus_namespace;

	// the temporary buffer used to store the jpg buffer when we compress the incoming frame
	char *m_jpgBuffer;
	int m_jpgBufferSize;

	// no copy constructor or assignment operator
	FaceDetector_Orbeus(const FaceDetector_Orbeus&);
	FaceDetector_Orbeus& operator=(const FaceDetector_Orbeus&);

public:
	FaceDetector_Orbeus(FaceAnalyzerConfiguration *faceAnalyzerConfiguration);
	~FaceDetector_Orbeus();

	std::vector<FaceDetectionDetails> Detect(const cv::Mat &frame, bool getDetailedInformation=false);
};

// end of namespaces
	}
}
#endif