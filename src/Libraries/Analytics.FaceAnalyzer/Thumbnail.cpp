


#include "FaceAnalyzerConfiguration.h"
#include "Thumbnail.h"
#include "EyeDetector_OpenCV.h"

using namespace cv;
using namespace std;

namespace Analytics
{
	namespace FaceAnalyzer
	{


Thumbnail::Thumbnail(FaceAnalyzerConfiguration *faceAnalyzerConfig)
{  has_eyecascade= false;
	face_detector_check.reset( new FaceDetector_OpenCV(faceAnalyzerConfig->faceDetectorCascadeFile));
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
	vector<Rect> faces_detected =face_detector_check->Detect(Thumbnail);
	float confidence;
	confidence = tracker_confidence;
	if(faces_detected.size()>0)
				{
					
					has_thumbnails = true;
					confidence = confidence + 0.15 ;
				}

	if(has_eyecascade)
	{

		vector<Rect> eyes_detected =eye_detector_check->Detect(Thumbnail);
		if(eyes_detected.size()>0)
				{
					confidence = confidence + 0.15 ;
				}
	}

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