#include "Frame.h"
#include <opencv2/opencv.hpp>

namespace Analytics
{
	namespace FaceAnalyzer
	{

Frame::Frame(const cv::Mat &original)
{
	originalFrame = original.clone();
}


Frame::~Frame(void)
{
}

cv::Mat Frame::GetScaledMat()
{
	if( scaledFrame.empty() )
	{
		if(Scale == 1.0)
			return originalFrame;
		else
		{
		   const int DETECTION_WIDTH = 960;
		// Shrink the image while keeping the same aspect ratio
			int scaledHeight = cvRound(originalFrame.rows/Scale);
			resize(originalFrame,scaledFrame, Size(DETECTION_WIDTH, scaledHeight));
			return scaledFrame;
		}
		
	}
	else
		// we already have the scaled frame so return it here
		return scaledFrame;
}

	}
}