#include "FaceAnalyzerConfiguration.h"
#include "Thumbnail.h"

using namespace cv;
using namespace std;

namespace Analytics
{
	namespace FaceAnalyzer
	{


Thumbnail::Thumbnail(void)
{
}


Thumbnail::~Thumbnail(void)
{
}


cv::Mat Thumbnail::ExtractThumbnail( const cv::Mat &frame, const cv::Rect &ThumbnailLocation)

{
	cv::Mat temp;
	temp= frame(ThumbnailLocation);
	return temp;

}



	}
}