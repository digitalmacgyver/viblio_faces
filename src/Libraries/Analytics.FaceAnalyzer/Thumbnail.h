
#include <opencv2/opencv.hpp>
#include "FaceAnalyzerConfiguration.h"

using namespace cv;
using namespace std;

namespace Analytics
{
	namespace FaceAnalyzer
	{
class FaceDetector_OpenCV;
class EyeDetector_OpenCV;
class Thumbnail
{
public:
	Thumbnail(FaceAnalyzerConfiguration *faceAnalyzerConfig);
	cv::Mat ExtractThumbnail( const cv::Mat &frame,const  cv::Rect &ThumbnailLocation);
	~Thumbnail(void);
	cv::Rect ConstrainRect(const cv::Rect &rectToConstrain, const cv::Size &imageSize);
    float GetConfidencevalue(const cv::Mat &Thumbnail,bool &has_thumbnails,const float &tracker_confidence);
	std::unique_ptr<FaceDetector_OpenCV> face_detector_check;
	std::unique_ptr<EyeDetector_OpenCV> eye_detector_check;
	int Thumbnail_enlarge_percentage;
	bool has_eyecascade;

};

	}
}