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
	enum ExpressionType
	{
		Expression_Unspecified,
		Expression_Neutral,
		Expression_Smile,
		Expression_SmileOpenedJaw,
		Expression_RaisedBrows,
		Expression_EyesAway,
		Expression_Squinting,
		Expression_Frowning,
		Expression_Unknown
	};

	enum FacialPropertiesType
	{
		FacialProperties_NotSpecified,
		FacialProperties_Specified,
		FacialProperties_Glasses,
		FacialProperties_Moustache,
		FacialProperties_Beard,
		FacialProperties_TeethVisible,
		FacialProperties_Blink,
		FacialProperties_MouthOpen,
		FacialProperties_LeftEyePatch,
		FacialProperties_RightEyePatch,
		FacialProperties_BothEyePatch,
		FacialProperties_DarkGlasses,
		FacialProperties_DistortingCondition,
		FacialProperties_Hat,
		FacialProperties_Scarf,
		FacialProperties_NoEar
	};

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

	// measures the distance between the eyes in pixels
	float intereyeDistance;

	// Nose location information
	cv::Point noseLocation;
	float noseLocationConfidence;

	// Gender information
	bool isMale;
	float genderConfidence;

	// expression information
	ExpressionType expression;
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

	// copy constructor
	//FaceDetectionDetails(const FaceDetectionDetails &otherFaceDetails);
	FaceDetectionDetails & operator=(FaceDetectionDetails other);

	std::string ToJson(){};

	std::string GetExpression();
};

	}
}

#endif