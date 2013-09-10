
#include <opencv2/opencv.hpp>
#include "FaceAnalyzerConfiguration.h"

using namespace cv;
using namespace std;

namespace Analytics
{
	namespace FaceAnalyzer
	{
class Thumbnail
{
public:
	Thumbnail(void);
	cv::Mat ExtractThumbnail( const cv::Mat &frame,const  cv::Rect &ThumbnailLocation);
	~Thumbnail(void);
	cv::Rect ConstrainRect(const cv::Rect &rectToConstrain, const cv::Size &imageSize);
};

	}
}