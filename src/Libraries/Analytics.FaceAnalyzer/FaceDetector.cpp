#include "FaceDetector.h"

#include "FaceDetectionDetails.h"
#include "ThumbnailDetails.h"
#include <opencv2/opencv.hpp>
#include <boost/log/trivial.hpp>

using namespace std;
using namespace cv;

namespace Analytics
{
	namespace FaceAnalyzer
	{

Rect FaceDetector::ConstrainRect(const Rect &rectToConstrain, const Size &imageSize)
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

void FaceDetector::RenderVisualization(cv::Mat &frame, const std::vector<FaceDetectionDetails> &detectedFaces)
{

	// iterate over each of the faces
	auto faceIter = detectedFaces.begin();
	auto faceIterEnd = detectedFaces.end();

	for(; faceIter != faceIterEnd; faceIter++)
	{
		// Draw the face
		Point center( (*faceIter).faceRect.x + int((*faceIter).faceRect.width*0.5f), (*faceIter).faceRect.y + int((*faceIter).faceRect.height*0.5f) );
		ellipse( frame, center, Size( int((*faceIter).faceRect.width*0.5f), int((*faceIter).faceRect.height*0.5f)), 0, 0, 360, Scalar( 255, 0, 0 ), 2, 8, 0 );
	}
}

FaceDetector::DetectorType FaceDetector::ConvertStringToDetectorType(const string &detectorType)
{
	std::string lowerCaseVersion = detectorType;
	std::transform(lowerCaseVersion.begin(), lowerCaseVersion.end(), lowerCaseVersion.begin(), ::tolower);

	if( lowerCaseVersion.compare("neurotech") == 0 )
		return FaceDetector_Neurotech;
	else if( lowerCaseVersion.compare("orbeus") == 0 )
		return FaceDetector_Orbeus;
	else if( lowerCaseVersion.compare("opencv") == 0 )
		return FaceDetector_OpenCV;
	else
		return FaceDetector_Unknown;
}

	}
}