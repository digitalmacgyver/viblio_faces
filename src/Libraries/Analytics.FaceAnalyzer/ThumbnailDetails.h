#ifndef __THUMBNAIL_DETAILS_H__
#define __THUMBNAIL_DETAILS_H__
#include "FaceAnalyzerConfiguration.h"
#include "FaceDetectionDetails.h"
#include <opencv2/opencv.hpp>


using namespace cv;
using namespace std;

namespace Analytics
{
	namespace FaceAnalyzer
	{


class ThumbnailDetails
{


private:
	FaceDetectionDetails thumbnaildeets;
	cv::Mat  thumbnail;

public:
	void FillThumbnailDetails(const cv::Mat &frame,  FaceDetectionDetails ThumbnailLocation);
	cv::Mat ThumbnailDetails::GetThumbnail();
	ThumbnailDetails(void);
	~ThumbnailDetails(void);
};


	}
}
#endif
