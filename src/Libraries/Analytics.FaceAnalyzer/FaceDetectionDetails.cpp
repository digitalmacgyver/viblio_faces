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

// end of namespaces
	}
}