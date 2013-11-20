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
		// the scaled version of the frame doesn't exist yet, produce the scaled frame here
		// Ramsri: add code here
	}
	else
		// we already have the scaled frame so return it here
		return scaledFrame;
}

	}
}