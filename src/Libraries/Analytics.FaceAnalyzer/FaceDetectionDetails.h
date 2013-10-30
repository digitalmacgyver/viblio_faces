#ifndef __FACE_DETECTION_DETAILS_H__
#define __FACE_DETECTION_DETAILS_H__

#include <opencv2/opencv.hpp>

namespace Analytics
{
	namespace FaceAnalyzer
	{

class FaceDetectionDetails
{
public:
	cv::Rect faceRect;
	float faceDetectionConfidence;

	bool hasAdditionalFaceInformation;

	// Face pose information
	short roll,
		  pitch,
		  yaw;

	// Eye information
	cv::Point leftEye;
	float leftEyeConfidence;
	cv::Point rightEye;
	float rightEyeConfidence;

	// Nose location information
	cv::Point noseLocation;
	float noseLocationConfidence;

	// Gender information
	bool isMale;
	float genderConfidence;

	// expression information
	bool isHappy; // this will change to an enum soon
	float expressionConfidence;

	// glasses information
	bool wearingGlasses;
	float glassesConfidence;
	bool wearingDarkGlasses;
	float wearingDarkGlassesConfidence;

	// mouth information
	cv::Point mouthLocation;
	float mouthLocationConfidence;
	bool mouthOpen;
	float mouthOpenConfidence;

	bool hasImageQualityInformation;

	float sharpness;
	float backgroundUniformity;
	float grayscaleDensity;

	FaceDetectionDetails();
	~FaceDetectionDetails();

	std::string ToJson(){};
};

	}
}

#endif