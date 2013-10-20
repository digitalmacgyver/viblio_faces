

#ifndef __FACEDETECTOR_NEUROTECH__
#define __FACEDETECTOR_NEUROTECH__
#include <memory>
#include <opencv2/objdetect/objdetect.hpp>
#include <string>

#include <NCore.h>
#include <NImages.h>
#include <NLExtractor.h>
#include <NLicensing.h>

//#include <TutorialUtils.h>

namespace Analytics
{
	namespace FaceAnalyzer
	{
class FaceDetector_Neurotech
{

private:
	
	NResult result ;
	HNLExtractor extractor ;
	HNLTemplate tmpl ;
	NleExtractionStatus status;

	HNImage image ;
	HNGrayscaleImage grayscale ;

	int faceCount;
	NleFace *faces ;
	NleDetectionDetails details;
	int i, j;
	const NChar * components ;
	NBool available ;
	NBool detectAllFeatures ;
	NBool detectBaseFeatures;
	NBool detectGender;
	NBool detectExpression;
	NBool detectMouthOpen;
	NBool detectBlink ;
	NBool detectGlasses ;
	NBool detectDarkGlasses ;
	NleTemplateSize templateSize ;
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
	
	FaceDetector_Neurotech(void);
	~FaceDetector_Neurotech(void);
	std::vector<cv::Rect> Detect(const cv::Mat &frame);
	void RenderVisualization(cv::Mat &frame, const std::vector<cv::Rect> &detectedFaces);
};

	}
}
#endif
