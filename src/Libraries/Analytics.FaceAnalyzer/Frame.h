
#ifndef __FRAME_H__
#define __FRAME_H__
#include <opencv2/opencv.hpp>
using namespace cv;


namespace Analytics
{
	namespace FaceAnalyzer
	{

class Frame
{
private:
	cv::Mat originalFrame;
	float Scale;
	
public:
	Frame(void);
	~Frame(void);
	cv::Mat GetThumbnail(const cv::Rect &thumbnail)
	{ 
		return originalFrame(thumbnail);
	}
	void setoriginalFrame(const cv::Mat &original){originalFrame = original.clone();};
	void setscale(const float &i){Scale = i;};
	float getscale(){return Scale;}; 
	int getcols(){return originalFrame.cols;};
	int getrows(){return originalFrame.rows;};

};

	}
}
#endif