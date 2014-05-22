

#ifndef __FACEDETECTOR_NEUROTECH__
#define __FACEDETECTOR_NEUROTECH__
#include "FaceDetectionDetails.h"

#include "FaceDetector.h"

#include <memory>
#include <opencv2/objdetect/objdetect.hpp>
#include <string>
#include <NCore.h>
#include <NImages.h>
#include <NLExtractor.h>
#include <NLicensing.h>

#include "Frame.h"

//#include <TutorialUtils.h>

namespace Analytics
{
	namespace FaceAnalyzer
	{
class FaceDetector_Neurotech : public FaceDetector
{

private:
	
	NResult result ;
	HNLExtractor extractor ;
	HNLExtractor extractor1 ;
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
//
//
//
//
//	// no copy constructor or assignment operator
//	FaceDetector_Neurotech(const FaceDetector_Neurotech&);
//	FaceDetector_Neurotech& operator=(const FaceDetector_Neurotech&);
//

	FaceDetectionDetails::ExpressionType ConvertNlpExpression(NLExpression nlpExpression);
public:
	
	FaceDetector_Neurotech(FaceAnalyzerConfiguration *faceAnalyzerConfiguration);
	~FaceDetector_Neurotech();
	std::vector<FaceDetectionDetails> Detect(const cv::Mat &frame, bool getDetailedInformation=false);
};

	}
}
#endif

