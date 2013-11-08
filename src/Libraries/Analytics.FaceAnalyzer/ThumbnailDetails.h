#ifndef __THUMBNAIL_DETAILS_H__
#define __THUMBNAIL_DETAILS_H__
#include "FaceAnalyzerConfiguration.h"
#include "FaceDetectionDetails.h"
#include <opencv2/opencv.hpp>

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
	ThumbnailDetails();
	~ThumbnailDetails();
	float backgroundUniformity;
	float sharpness;
	float grayscaleDensity;
	void FillThumbnailDetails(const cv::Mat &frame,  FaceDetectionDetails ThumbnailLocation);

	void SetThumbnail(cv::Mat newThumbnail){ thumbnail = newThumbnail; }
	cv::Mat GetThumbnail(){ return thumbnail; }
	FaceDetectionDetails GetDetailedInformation(){return thumbnaildeets;}
	void SetDetailedInformation(const FaceDetectionDetails &detailedFaceInfo){ thumbnaildeets = detailedFaceInfo; }
};


	}
}
#endif
