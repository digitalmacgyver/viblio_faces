#include "ThumbnailDetails.h"

namespace Analytics
{
	namespace FaceAnalyzer
	{


ThumbnailDetails::ThumbnailDetails(void)
{

	//thumbnaildeets = new FaceDetectionDetails;
}


ThumbnailDetails::~ThumbnailDetails(void)
{
}


void ThumbnailDetails::FillThumbnailDetails(const cv::Mat &frame,  FaceDetectionDetails ThumbnailLocation)
{
	thumbnail = frame.clone();
	thumbnaildeets = ThumbnailLocation;

}

cv::Mat ThumbnailDetails::GetThumbnail()
{
	return thumbnail;
}
	}
}