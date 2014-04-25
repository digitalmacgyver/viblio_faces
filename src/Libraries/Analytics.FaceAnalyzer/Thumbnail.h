
#include <opencv2/opencv.hpp>
#include "FaceAnalyzerConfiguration.h"
#include <Ntfi.h>
#include "ThumbnailDetails.h"
#include "Frame.h"
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
	friend class ThumbnailTests;

private:
	std::unique_ptr<FaceDetector_Neurotech> face_detector_check;
	//std::unique_ptr<FaceDetector_Neurotech> face_detector_neuro;
	std::unique_ptr<EyeDetector_OpenCV> eye_detector_check;
	//std::unique_ptr<ThumbnailDetails> thumbnail_details;
    
	HNtfi tokenFaceExtractor;

	const NChar * m_licenseComponents;

	float Thumbnail_enlarge_percentage;
	bool has_eyecascade;

	// properties related to the region we choose for the thumbnail
	float m_upperThumbnailRegion;
	float m_lowerThumbnailRegion;
	float m_leftRightThumbnailRegion;

	// The various confidence parameters that make up our final confidence result
	float m_confidenceWeightFaceDetected; // If the face is simply redetected in the thumbnail it gives us more confidence
	float m_confidenceWeightEyesDetected; // detecting the eyes in the thumbnail gives us confidence
	float m_confidenceWeightIntereyeDistance; // the larger the intereye distance the better... < lower intereye bound gives 0 additional confidence, > upper bound gives maximum bonus
	const float m_intereyeDistanceLowerBound;
	const float m_intereyeDistanceUpperBound;
	float m_confidenceWeightNoseDetected; // If nose is detected we get a small boost
	float m_confidenceWeightMouthDetected;// If mouth is detected we get a small boost
	float m_confidenceWeightGenderDetected;// If gender is detected we get a small boost
	float m_confidenceWeightQuality;	  // Neurotech quality metric which is a combination of grayscale density, background uniformity & sharpness
	float m_confidenceWeightBlinkDetected; // If we confidently identify that the person is blinking then we subtract a reasonable amount
	float m_blinkConfidenceThreshold; // the minimum required blink confidence level before the weight adjustment (either positive or negative) is allied
	float m_confidenceWeightSharpness; // the sharpness of the image provides a lot of information as to the amount of blur in the image


	bool MatToHNImage(const cv::Mat &matImage, HNImage *hnImage);
	cv::Mat HNImageToMat(HNImage *hnImage);
	cv::Rect ConstrainRect(const cv::Rect &rectToConstrain, const cv::Size &imageSize);
	std::vector<cv::Point> GetFaceRegion(const cv::Mat &frame, cv::Point leftEye, cv::Point rightEye,
										 float intereyeDistance, float aboveEyeBuffer, float belowEyeBuffer, float besideEyeBuffer);

public:
	Thumbnail(FaceAnalyzerConfiguration *faceAnalyzerConfig);
	~Thumbnail(void);
	
	bool ExtractThumbnail(const  cv::Rect &ThumbnailLocation, float &confidence, ThumbnailDetails &thumbnail_details,Frame &origFrame);
	
 
	

};

	}
}