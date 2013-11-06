
#include <opencv2/opencv.hpp>
#include "FaceAnalyzerConfiguration.h"
#include <Ntfi.h>

using namespace cv;
using namespace std;

namespace Analytics
{
	namespace FaceAnalyzer
	{
class FaceDetector_OpenCV;
class FaceDetector_Neurotech;
class EyeDetector_OpenCV;
class Thumbnail
{
private:
	std::unique_ptr<FaceDetector_Neurotech> face_detector_check;
	//std::unique_ptr<FaceDetector_Neurotech> face_detector_neuro;
	std::unique_ptr<EyeDetector_OpenCV> eye_detector_check;

	HNtfi tokenFaceExtractor;

	float Thumbnail_enlarge_percentage;
	bool has_eyecascade;

	bool MatToHNImage(const cv::Mat &matImage, HNImage *hnImage);
	cv::Mat HNImageToMat(HNImage *hnImage);
	cv::Rect ConstrainRect(const cv::Rect &rectToConstrain, const cv::Size &imageSize);

public:
	Thumbnail(FaceAnalyzerConfiguration *faceAnalyzerConfig);
	~Thumbnail(void);

	cv::Mat ExtractThumbnail( const cv::Mat &frame,const  cv::Rect &ThumbnailLocation, float &confidence);
	
    float GetConfidencevalue(const cv::Mat &Thumbnail,bool &has_thumbnails,const float &tracker_confidence);
	

};

	}
}