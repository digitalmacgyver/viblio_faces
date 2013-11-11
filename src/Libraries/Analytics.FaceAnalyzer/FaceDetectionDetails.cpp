#include "FaceDetectionDetails.h"

using namespace cv;

namespace Analytics
{
	namespace FaceAnalyzer
	{

FaceDetectionDetails::FaceDetectionDetails()
{
	faceRect = Rect(0,0,0,0);
	faceDetectionConfidence = 0.0f;

	roll = pitch = yaw = 0;

	// eye information
	leftEye = Point(0,0);
	leftEyeConfidence = 0.0f;
	rightEye = Point(0,0);
	rightEyeConfidence = 0.0f;

	intereyeDistance = 0.0f;

	// nose information
	noseLocation = Point(0,0);
	noseLocationConfidence = 0.0f;

	// mouth information
	mouthLocation = Point(0,0);
	mouthLocationConfidence = 0.0f;
	mouthOpen = false;
	mouthOpenConfidence = 0.0f;

	// gender information
	isMale = true;
	genderConfidence = 0.0f;

	// glasses information
	wearingGlasses = false;
	glassesConfidence = 0.0f;
	wearingDarkGlasses = false;
	wearingDarkGlassesConfidence = 0.0f;

	hasAdditionalFaceInformation = false;
	hasImageQualityInformation = false;
}


FaceDetectionDetails::~FaceDetectionDetails()
{
}

FaceDetectionDetails & FaceDetectionDetails::operator= (FaceDetectionDetails other)
{
	if (this != &other) // protect against invalid self-assignment
	{
		faceRect = other.faceRect;
		faceDetectionConfidence=other.faceDetectionConfidence;
		roll = other.roll;
		pitch = other.pitch;
		yaw = other.yaw;

		leftEye = other.leftEye;
		leftEyeConfidence = other.leftEyeConfidence;
		rightEye = other.rightEye;
		rightEyeConfidence = other.rightEyeConfidence;

		intereyeDistance = other.intereyeDistance;

		hasAdditionalFaceInformation = other.hasAdditionalFaceInformation;

		noseLocation = other.noseLocation;
		noseLocationConfidence = other.noseLocationConfidence;

		isMale = other.isMale;
		genderConfidence = other.genderConfidence;

		isHappy = other.isHappy;
		expressionConfidence = other.expressionConfidence;

		wearingDarkGlasses = other.wearingDarkGlasses;
		wearingDarkGlassesConfidence = other.wearingDarkGlassesConfidence;

		wearingGlasses = other.wearingGlasses;
		glassesConfidence = other.glassesConfidence;

		mouthLocation = other.mouthLocation;
		mouthLocationConfidence = other.mouthLocationConfidence;

		mouthOpen = other.mouthOpen;
		mouthOpenConfidence = other.mouthOpenConfidence;
	}

	return *this;
}

//FaceDetectionDetails::FaceDetectionDetails(const FaceDetectionDetails &otherFaceDetails)
//{
//	this.faceRect = otherFaceDetails.faceRect;
//}

// end of namespaces
	}
}