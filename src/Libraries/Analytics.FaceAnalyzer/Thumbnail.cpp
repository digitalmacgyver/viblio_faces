#include "FaceAnalyzerConfiguration.h"
#include "Thumbnail.h"
#include "EyeDetector_OpenCV.h"
#include "Jzon/Jzon.h"
#include <numeric>

// Neurotech related libraries for token image generation
#include <NImages.h>
#include <Ntfi.h>
#include <NLExtractor.h>
#include <NLicensing.h>

using namespace cv;
using namespace std;

namespace Analytics
{
	namespace FaceAnalyzer
	{


Thumbnail::Thumbnail(FaceAnalyzerConfiguration *faceAnalyzerConfig)
{  
	tokenFaceExtractor = NULL;

	has_eyecascade= false;
	face_detector_check.reset( new FaceDetector_Neurotech() );
	//face_detector_check.reset( new FaceDetector_Neurotech(faceAnalyzerConfig->faceDetectorCascadeFile));
	//face_detector_neuro.reset(new FaceDetector_Neurotech());
	//if(!faceAnalyzerConfig->eyeDetectorCascadeFile.empty())
	//{   has_eyecascade = true;
	//	eye_detector_check.reset(new EyeDetector_OpenCV(faceAnalyzerConfig->eyeDetectorCascadeFile));
	//}
	
	Thumbnail_enlarge_percentage = 20;
	NBool available = false;

	NResult result = N_OK;

	const NChar * components = N_T("Biometrics.FaceDetection,Biometrics.FaceSegmentation,Biometrics.FaceQualityAssessment");
	try
	{	
		result = NLicenseObtainComponents(N_T("/local"), N_T("5000"), components, &available);
		if( NFailed(result) )
			throw runtime_error("Failed to obtain Neurotech licenses for the Token & Image quality related components");
	}
	catch(Exception e)
	{
		cout << "Exception caught while attempting to obtain Neurotech product license. Cannot continue" << endl;
		if (!available)
			cout << "Neurotech Licenses for " << components << "  not available" << endl;
		throw e;
	}
	
	try
	{	
		result = NtfiCreate(&tokenFaceExtractor);
		if( NFailed(result) )
			throw runtime_error("Failed to Token image extractor");
	}
	catch(Exception e)
	{
		cout << "Exception caught while attempting to create a Neurotech token image extractor. Cannot continue" << endl;
		
		throw e;
	}

}


Thumbnail::~Thumbnail()
{
	if( tokenFaceExtractor != NULL )
		NObjectFree(tokenFaceExtractor);
}


cv::Mat Thumbnail::ExtractThumbnail( const cv::Mat &frame, const cv::Rect &ThumbnailLocation, float &confidence)
{
	cv::Mat temp;
	
	Rect enlarged_thumbnail(ThumbnailLocation.x-(ThumbnailLocation.width*Thumbnail_enlarge_percentage/100),ThumbnailLocation.y-(ThumbnailLocation.height*Thumbnail_enlarge_percentage/100),ThumbnailLocation.width+(ThumbnailLocation.width*(Thumbnail_enlarge_percentage*2)/100),ThumbnailLocation.height+(ThumbnailLocation.height*Thumbnail_enlarge_percentage*2/100));
	Rect constrainedRect = ConstrainRect(enlarged_thumbnail, Size(frame.cols, frame.rows));
	
	temp = frame(constrainedRect);

	// perform a detailed face extraction to get some detailed information
	vector<FaceDetectionDetails> detectedFaces = face_detector_check->Detect(temp, true);

	if( detectedFaces.size() > 1 || detectedFaces.size() == 0 )
	{
		// if we find either no faces or more than 1 face in this 'little' region then we have 0 confidence in
		// this thumbnail
		confidence = 0.0f;
		return temp;
	}

	// now use the detailed information to create a token image

	// and then extract the quality information for the token image

	return temp;

}


float Thumbnail::GetConfidencevalue(const cv::Mat &Thumbnail,bool &has_thumbnails,const float &tracker_confidence )
{
	std::vector<float> v;
	float confidence = 1;

	string faces_detected =face_detector_check->Detect_return_json(Thumbnail,"temp",1);
	Jzon::Object rootNode;
	Jzon::Parser parser(rootNode, faces_detected);
	if(faces_detected.empty())
	{
		confidence = 0;
		return confidence;
	}
	has_thumbnails = true;
	if (!parser.Parse())
	{
		std::cout << "Error: " << parser.GetError() << std::endl;
	}
	else
	{

		v.push_back(rootNode.Get("face_confidence").ToFloat());
		if(rootNode.Get("Genderconfidence").ToFloat()<255)
			v.push_back(rootNode.Get("Genderconfidence").ToFloat());
		if(rootNode.Get("Blinkconfidence").ToFloat()<255)
			v.push_back(rootNode.Get("Blinkconfidence").ToFloat());
		if(rootNode.Get("MouthOpenConfidence").ToFloat()<255)
			v.push_back(rootNode.Get("MouthOpenConfidence").ToFloat());

	}

	// Take Geometric mean 
	/*
	for(std::vector<float>::iterator it = v.begin(); it != v.end(); ++it) {
    confidence = confidence * *it;
			}
	float temp = 1.0/ v.size();
	confidence =  pow(confidence ,temp);
	*/

	// Arithnmetic mean
	confidence = std::accumulate(v.begin(), v.end(), 0.0) / v.size();
	return confidence;

}

// Ensures that the rect passed in is valid based on the image size it is supposedly from. Returns
// a rect that is sure to be inside the bounds of the image
Rect Thumbnail::ConstrainRect(const Rect &rectToConstrain, const Size &imageSize)
{
	Rect constrainedRect = rectToConstrain;

	if( rectToConstrain.x < 0 )
	{
		// the left edge of the rect is beyond the left edge of the image
		constrainedRect.width += rectToConstrain.x; // will basically subtract this amount from the width to ensure the right edge of the rect stays in the same place
		constrainedRect.x = 0; // because we are going to add it here
	}

	if( rectToConstrain.y < 0 )
	{
		// the top edge of the rect is beyond the top edge of the image
		constrainedRect.height += rectToConstrain.y; // will basically subtract this amount from the height to ensure the bottom edge of the rect stays in the same place
		constrainedRect.y = 0; // because we are going to add it here
	}

	if( (rectToConstrain.x + rectToConstrain.width) > imageSize.width )
	{
		// the right hand edge of the rect goes beyond the edge of the image... crop the right edge so it falls on the edge of the image instead
		int rightEdgeDifference = (rectToConstrain.x + rectToConstrain.width) - imageSize.width;
		constrainedRect.width -= rightEdgeDifference;
	}

	if( (rectToConstrain.y + rectToConstrain.height) > imageSize.height )
	{
		// the bottom hand edge of the rect goes beyond the bottom edge of the image... crop the bottom edge so it falls on the edge of the image instead
		int bottomEdgeDifference = (rectToConstrain.y + rectToConstrain.height) - imageSize.height;
		constrainedRect.height -= bottomEdgeDifference;
	}

	return constrainedRect;
}


	}
}