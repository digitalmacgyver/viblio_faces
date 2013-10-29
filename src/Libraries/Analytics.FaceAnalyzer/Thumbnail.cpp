


#include "FaceAnalyzerConfiguration.h"
#include "Thumbnail.h"
#include "EyeDetector_OpenCV.h"
#include "Jzon/Jzon.h"
#include <numeric>

using namespace cv;
using namespace std;

namespace Analytics
{
	namespace FaceAnalyzer
	{


Thumbnail::Thumbnail(FaceAnalyzerConfiguration *faceAnalyzerConfig)
{  has_eyecascade= false;
  face_detector_check.reset( new FaceDetector_Neurotech());
	//face_detector_check.reset( new FaceDetector_Neurotech(faceAnalyzerConfig->faceDetectorCascadeFile));
	//face_detector_neuro.reset(new FaceDetector_Neurotech());
	if(!faceAnalyzerConfig->eyeDetectorCascadeFile.empty())
	{   has_eyecascade = true;
		eye_detector_check.reset(new EyeDetector_OpenCV(faceAnalyzerConfig->eyeDetectorCascadeFile));
	}
	Thumbnail_enlarge_percentage = 20;
	
}


Thumbnail::~Thumbnail(void)
{
}


cv::Mat Thumbnail::ExtractThumbnail( const cv::Mat &frame, const cv::Rect &ThumbnailLocation)

{
	cv::Mat temp;
	
	Rect enlarged_thumbnail(ThumbnailLocation.x-(ThumbnailLocation.width*Thumbnail_enlarge_percentage/100),ThumbnailLocation.y-(ThumbnailLocation.height*Thumbnail_enlarge_percentage/100),ThumbnailLocation.width+(ThumbnailLocation.width*(Thumbnail_enlarge_percentage*2)/100),ThumbnailLocation.height+(ThumbnailLocation.height*Thumbnail_enlarge_percentage*2/100));
	Rect constrainedRect = ConstrainRect(enlarged_thumbnail, Size(frame.cols, frame.rows));
	
	temp= frame(constrainedRect);



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