#include "FaceAnalyzerConfiguration.h"
#include "Thumbnail.h"
#include "ThumbnailDetails.h"
#include "EyeDetector_OpenCV.h"
#include "Jzon/Jzon.h"
#include <numeric>
#include "Frame.h"

// Neurotech related libraries for token image generation
#include <NImages.h>
#include <Ntfi.h>
#include <NLExtractor.h>
#include <NLicensing.h>

using namespace cv;
using namespace std;

namespace Analytics
{
	namespace FaceAnalyzer
	{


Thumbnail::Thumbnail(FaceAnalyzerConfiguration *faceAnalyzerConfig) :
	m_confidenceWeightFaceDetected(0.2f),
	m_confidenceWeightEyesDetected(0.15f),
	m_confidenceWeightIntereyeDistance(0.15f),
	m_confidenceWeightNoseDetected(0.04f),
	m_confidenceWeightMouthDetected(0.03f),
	m_confidenceWeightGenderDetected(0.03f),
	m_confidenceWeightQuality(0.4f), // all these confidence weights must add to 1
	m_intereyeDistanceLowerBound(40.0f),
	m_intereyeDistanceUpperBound(100.0f)
{  
	tokenFaceExtractor = NULL;
	has_eyecascade= false;
	face_detector_check.reset( new FaceDetector_Neurotech() );
//	thumbnail_details.reset(new ThumbnailDetails());
	//face_detector_check.reset( new FaceDetector_Neurotech(faceAnalyzerConfig->faceDetectorCascadeFile));
	//face_detector_neuro.reset(new FaceDetector_Neurotech());
	//if(!faceAnalyzerConfig->eyeDetectorCascadeFile.empty())
	//{   has_eyecascade = true;
	//	eye_detector_check.reset(new EyeDetector_OpenCV(faceAnalyzerConfig->eyeDetectorCascadeFile));
	//}
	
	Thumbnail_enlarge_percentage = 0.20f;
	NBool available = false;

	NResult result = N_OK;

	const NChar * components = N_T("Biometrics.FaceDetection,Biometrics.FaceSegmentation,Biometrics.FaceQualityAssessment");
	try
	{	
		result = NLicenseObtainComponents(N_T("/local"), N_T("5000"), components, &available);
		if( NFailed(result) )
			throw runtime_error("Failed to obtain Neurotech licenses for the Token & Image quality related components");
	}
	catch(Exception e)
	{
		cout << "Exception caught while attempting to obtain Neurotech product license. Cannot continue" << endl;
		if (!available)
			cout << "Neurotech Licenses for " << components << "  not available" << endl;
		throw e;
	}
	
	try
	{	
		result = NtfiCreate(&tokenFaceExtractor);
		if( NFailed(result) )
			throw runtime_error("Failed to Token image extractor");
	}
	catch(Exception e)
	{
		cout << "Exception caught while attempting to create a Neurotech token image extractor. Cannot continue" << endl;
		
		throw e;
	}

}


Thumbnail::~Thumbnail()
{
	if( tokenFaceExtractor != NULL )
		NObjectFree(tokenFaceExtractor);
}



bool Thumbnail::ExtractThumbnail(const cv::Rect &ThumbnailLocation, float &confidence, ThumbnailDetails &thumbnail_details,Frame &origFrame)
{   
	cv::Mat thumbnail;
	confidence = 0.0f;
	
	float scalefactor = origFrame.getscale();
	

	Rect enlarged_thumbnail((int)(ThumbnailLocation.x*scalefactor)-(int)(ThumbnailLocation.width*Thumbnail_enlarge_percentage*scalefactor),(int)(ThumbnailLocation.y*scalefactor)-(int)(ThumbnailLocation.height*scalefactor*Thumbnail_enlarge_percentage),(int)(ThumbnailLocation.width*scalefactor)+(int)(ThumbnailLocation.width*scalefactor*(Thumbnail_enlarge_percentage*2)),(int)(ThumbnailLocation.height*scalefactor)+(int)(ThumbnailLocation.height*scalefactor*Thumbnail_enlarge_percentage*2));
	Rect constrainedRect = ConstrainRect(enlarged_thumbnail, Size(origFrame.getcols(), origFrame.getrows()));

	thumbnail = origFrame.GetThumbnail(constrainedRect);
 
	thumbnail_details.SetThumbnail(thumbnail);

	// perform a detailed face extraction to get some detailed information
	vector<FaceDetectionDetails> detectedFaces = face_detector_check->Detect(origFrame, true);

	if( detectedFaces.size() > 1 || detectedFaces.size() == 0 )
	{
		// if we find either no faces or more than 1 face in this 'little' region then we have 0 confidence in
		// this thumbnail
		return false;
	}
	
	// now use the detailed information to create a token image
	FaceDetectionDetails detailedFaceInfo;
	detailedFaceInfo = detectedFaces.at(0);

	// we have detected the face at least
	confidence += m_confidenceWeightFaceDetected * (detailedFaceInfo.faceDetectionConfidence/100.0f);

	// if we have detected the eyes then this gives us even more confidence
	if( detailedFaceInfo.rightEyeConfidence > 0 && detailedFaceInfo.leftEyeConfidence > 0 )
	{
		confidence += ((m_confidenceWeightEyesDetected/2.0f) * detailedFaceInfo.rightEyeConfidence) + 
					  ((m_confidenceWeightEyesDetected/2.0f) * detailedFaceInfo.leftEyeConfidence);

		// based on the intereye distance calculate how much additional confidence to add
		float adjustedIntereyeDist = max(0.0f, detailedFaceInfo.intereyeDistance - m_intereyeDistanceLowerBound);

		float intereyeWeightAdjusted = min(1.0f, adjustedIntereyeDist / (m_intereyeDistanceUpperBound - m_intereyeDistanceLowerBound));

		confidence += m_confidenceWeightIntereyeDistance * intereyeWeightAdjusted;
	}

	thumbnail_details.SetDetailedInformation(detailedFaceInfo);

	// check to make sure we have the prerequisite information for token extraction before proceeding
	if(detailedFaceInfo.leftEyeConfidence <= 0 || detailedFaceInfo.rightEyeConfidence <= 0)
	{
		return false;
	}

	// Make use of the nose and mouth location information in our confidence estimation. Having one or both
	// of these gives us more confidence
	confidence += m_confidenceWeightNoseDetected * detailedFaceInfo.noseLocationConfidence;
	confidence += m_confidenceWeightMouthDetected * detailedFaceInfo.noseLocationConfidence;

	NResult result ;
	NPoint first;
	NPoint second;
	HNImage token = NULL;
	HNImage image = NULL;
	HNtfiAttributes ntfiAttributes = NULL;
	NDouble quality;

	// first convert out thumbnail into an HNImage
	bool success = MatToHNImage(thumbnail, &image);

	if(!success)
	{
		// Failed to convert the OpenCV Mat into a Neurotech HNImage

		if(image)
			NObjectFree(image);

		return false;
	}

	// now setup the left and right eye positions and use them to create the token image
	first.X = detailedFaceInfo.rightEye.x;
	first.Y = detailedFaceInfo.rightEye.y;
	second.X = detailedFaceInfo.leftEye.x;
	second.Y = detailedFaceInfo.leftEye.y;
	result = NtfiCreateTokenFaceImageEx(tokenFaceExtractor, image, &first, &second, &token);
	if (NFailed(result))
	{
		cout << "NtfiCreateTokenFaceImage()failed (result = " << result<< ")!" << endl;
		if(image)
			NObjectFree(image);
		if( token)
			NObjectFree(token);
		
		return false;
	}

	//const NChar * savePath = N_T("C:\\temp\\tokenTest.jpg");
	//result = NImageSaveToFileEx(token, savePath, NULL, NULL, 0);

	// now that we have the token image lets test it to establish its quality
	result = NtfiTestTokenFaceImage(tokenFaceExtractor, token, &ntfiAttributes, &quality);
	if (NFailed(result))
	{
		cout << "NtfiCreateTokenFaceImage() quality extractor failed (result = " << result<< ")!" << endl;
		if(image)
			NObjectFree(image);

		if(token)
			NObjectFree(token);

		if (ntfiAttributes)
			NObjectFree(ntfiAttributes);

		return false;
	}

	confidence += m_confidenceWeightQuality * quality;

	// Determine the background uniformity - see page 729 in the SDK documentation
	double backgroundUniformity = 0.0f;
	NtfiAttributesGetBackgroundUniformity(ntfiAttributes, &backgroundUniformity);

	// determine the image sharpness
	double sharpness = 0.0f;
	NtfiAttributesGetSharpness(ntfiAttributes, &sharpness);
	
	// lastly, determine the grayscale density
	double grayscaleDensity = 0.0f;
	NtfiAttributesGetGrayscaleDensity(ntfiAttributes, &grayscaleDensity);
	
	thumbnail_details.backgroundUniformity = backgroundUniformity;
	thumbnail_details.grayscaleDensity = grayscaleDensity;
	thumbnail_details.sharpness = sharpness;
	// Now convert the token image back into an OpenCV Mat
	//Mat tokenMat = HNImageToMat(&token);
	
	//imshow("Token image", tokenMat);
	//waitKey(0);

	if(image)
		NObjectFree(image);
	if(token)
		NObjectFree(token);
	if (ntfiAttributes)
		NObjectFree(ntfiAttributes);

//	confidence = quality;

	return true;
}

bool Thumbnail::MatToHNImage(const Mat &matImage, HNImage *hnImage)
{
	unsigned char *pixelData = new unsigned char[matImage.rows*matImage.cols*matImage.channels()];
	
	for (int row=0;row<matImage.rows;row++)
	{
		const unsigned char *data = matImage.ptr(row);
		for (int col=0;col<matImage.cols;col++)
		{
			// then use *data for the pixel value, assuming you know the order, RGB etc           
			// Note 'rgb' is actually stored B,G,R
		   pixelData[(row * matImage.cols + col)*3 + 2]= *data++;
		   pixelData[(row * matImage.cols + col)*3 + 1] = *data++;
		   pixelData[(row * matImage.cols + col)*3 + 0]= *data++;
		}
	}

	NResult result = NImageCreateFromDataEx(npfRgb, matImage.cols, matImage.rows, matImage.cols*matImage.channels(), matImage.cols*matImage.channels(), pixelData, (matImage.cols*matImage.channels())*matImage.rows, 0, hnImage);

	delete [] pixelData;

	if(NFailed(result))
	{
		cout << "NImageCreateFromDataEx failed (result = " << result<< ")!" << endl;
		return false;
	}

	// success
	return true;
}

Mat Thumbnail::HNImageToMat(HNImage *hnImage)
{
	NUInt width = 0, height = 0;

	NImageGetWidth(*hnImage, &width);
	NImageGetHeight(*hnImage, &height);
	NInt channels = 0;
	NImageGetPlaneCount(*hnImage, &channels);
	Mat matImage;

	if( channels == 3 )
	{
		matImage = Mat(height, width, CV_8UC3);
		NImageCopyToData(*hnImage, npfRgb, matImage.cols, matImage.rows, matImage.step, matImage.data, matImage.step * matImage.rows, 0, 0, 0);
		cvtColor(matImage, matImage, CV_BGR2RGB);
	}
	else if( channels == 1 )
	{
		matImage = Mat(height, width, CV_8UC1);
		NImageCopyToData(*hnImage, npfGrayscale, matImage.cols, matImage.rows, matImage.step, matImage.data, matImage.step * matImage.rows, 0, 0, 0);
	}
	else
	{
		cout << "Unsupported number of channels when converting from HNImage to Mat" << endl;
	}

	return matImage;
}


// Ensures that the rect passed in is valid based on the image size it is supposedly from. Returns
// a rect that is sure to be inside the bounds of the image
Rect Thumbnail::ConstrainRect(const Rect &rectToConstrain, const Size &imageSize)
{
	Rect constrainedRect = rectToConstrain;

	if( rectToConstrain.x < 0 )
	{
		// the left edge of the rect is beyond the left edge of the image
		constrainedRect.width += rectToConstrain.x; // will basically subtract this amount from the width to ensure the right edge of the rect stays in the same place
		constrainedRect.x = 0; // because we are going to add it here
	}

	if( rectToConstrain.y < 0 )
	{
		// the top edge of the rect is beyond the top edge of the image
		constrainedRect.height += rectToConstrain.y; // will basically subtract this amount from the height to ensure the bottom edge of the rect stays in the same place
		constrainedRect.y = 0; // because we are going to add it here
	}

	if( (rectToConstrain.x + rectToConstrain.width) > imageSize.width )
	{
		// the right hand edge of the rect goes beyond the edge of the image... crop the right edge so it falls on the edge of the image instead
		int rightEdgeDifference = (rectToConstrain.x + rectToConstrain.width) - imageSize.width;
		constrainedRect.width -= rightEdgeDifference;
	}

	if( (rectToConstrain.y + rectToConstrain.height) > imageSize.height )
	{
		// the bottom hand edge of the rect goes beyond the bottom edge of the image... crop the bottom edge so it falls on the edge of the image instead
		int bottomEdgeDifference = (rectToConstrain.y + rectToConstrain.height) - imageSize.height;
		constrainedRect.height -= bottomEdgeDifference;
	}

	return constrainedRect;
}


	}
}