#include "FaceDetector_Neurotech.h"
#include "FaceDetectionDetails.h"
#include "ThumbnailDetails.h"
#include <opencv2/opencv.hpp>

//#include <opencv2/highgui/highgui.hpp>
//#include <opencv/highgui.h>
#include <stdexcept>
#include <vector>

#include <stdio.h>
#include <stdlib.h>

#include <NCore.h>
#include <NImages.h>
#include <NLExtractor.h>
#include <NLicensing.h>

#include "Jzon/Jzon.h"

//#include <TutorialUtils.h>

using namespace std;
using namespace cv;




namespace Analytics
{
	namespace FaceAnalyzer
	{


FaceDetector_Neurotech::FaceDetector_Neurotech(void)
	:
	 result( N_OK),
	 extractor( NULL),
	 tmpl (NULL),
     image(NULL),
	 grayscale(NULL),
	 faces( NULL),
	 components(N_T("Biometrics.FaceDetection,Biometrics.FaceExtraction")),
	 available(NFalse),
	 detectAllFeatures( NTrue),
	 detectBaseFeatures(NTrue),
	 detectGender( NTrue),
	 detectExpression( NTrue),
     detectMouthOpen(NTrue),
	 detectBlink( NTrue),
	 detectGlasses(NTrue),
	 detectDarkGlasses ( NTrue),
	 templateSize(nletsSmall)
{
	/*
	// check the license first
	result = NLicenseObtainComponents(N_T("/local"), N_T("5000"), components, &available);
	if (NFailed(result))
		cout << "NLicenseObtainComponents() failed, result = " << result<<endl;
		//PrintErrorMsg(N_T("NLicenseObtainComponents() failed, result = %d\n"), result);


	if (!available)
	{
		cout << "Licenses for " << components<< "not available" << endl;
		//printf(N_T("Licenses for %s not available\n"), components);
		result = N_E_FAILED;
	}
	*/

	// Check the Licenses
	
	try
	{
		
	     result = NLicenseObtainComponents(N_T("/local"), N_T("5000"), components, &available);
		 if (NFailed(result))
			 throw 20;
	}
	catch(int e)
	{
		cout << "Exception caught while attempting to obtain Neurotech product license. " << " Cannot continue" << endl;
		if (!available)
			cout << "Neurotech Licenses for " << components<< "  not available" << endl;
		throw e;
	}



	// create an extractor
	try
	{
	     result = NleCreate(&extractor);
		 if (NFailed(result))
			 throw 20;
	}
	catch(int e)
	{
		cout << "NleCreate() failed (result = " << result<< ")!" << endl;
		throw e;
	}
	
	
	// Detect all feature points
	try
	{
	     result = NObjectSetParameterEx(extractor, NLEP_DETECT_ALL_FEATURE_POINTS, -1, &detectAllFeatures, sizeof(NBool));
		 if (NFailed(result))
			 throw 20;
	}
	catch(int e)
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		throw e;
	}
	
	
	// Detect all base feature Points
	try
	{
	     result = NObjectSetParameterEx(extractor, NLEP_DETECT_BASE_FEATURE_POINTS, -1, &detectBaseFeatures, sizeof(NBool));
		 if (NFailed(result))
			 throw 20;
	}
	catch(int e)
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		throw e;
	}
	
	// Detect Gender
	try
	{
	    result = NObjectSetParameterEx(extractor, NLEP_DETECT_GENDER, -1, &detectGender, sizeof(NBool));
		 if (NFailed(result))
			 throw 20;
	}
	catch(int e)
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		throw e;
	}
	
	// Detect Expression
	try
	{
	    result = NObjectSetParameterEx(extractor, NLEP_DETECT_EXPRESSION, -1, &detectExpression, sizeof(NBool));
		 if (NFailed(result))
			 throw 20;
	}
	catch(int e)
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		throw e;
	}

   // Detect Blink
	try
	{
	   result = NObjectSetParameterEx(extractor, NLEP_DETECT_BLINK, -1, &detectBlink, sizeof(NBool));
		 if (NFailed(result))
			 throw 20;
	}
	catch(int e)
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		throw e;
	}

	// Detect Mouth Open
	try
	{
	   result = NObjectSetParameterEx(extractor, NLEP_DETECT_MOUTH_OPEN, -1, &detectMouthOpen, sizeof(NBool));
		 if (NFailed(result))
			 throw 20;
	}
	catch(int e)
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		throw e;
	}

	// Detect Glasses
	
	try
	{
	   result = NObjectSetParameterEx(extractor, NLEP_DETECT_GLASSES, -1, &detectGlasses, sizeof(NBool));
		 if (NFailed(result))
			 throw 20;
	}
	catch(int e)
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		throw e;
	}

	// Detect dark Glasses

	try
	{
	   result = NObjectSetParameterEx(extractor, NLEP_DETECT_DARK_GLASSES, -1, &detectDarkGlasses, sizeof(NBool));
		 if (NFailed(result))
			 throw 20;
	}
	catch(int e)
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		throw e;
	}

	
	try
	{
	   result = NObjectSetParameterEx(extractor, NLEP_TEMPLATE_SIZE, -1, &templateSize, sizeof(NleTemplateSize));
		 if (NFailed(result))
			 throw 20;
	}
	catch(int e)
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		throw e;
	}
	
}
//
//
FaceDetector_Neurotech::~FaceDetector_Neurotech(void)
{

	   NObjectFree(extractor);
	   NObjectFree(extractor1);
		NResult result2 = NLicenseReleaseComponents(components);
		if (NFailed(result2))
		{
			cout << "NLicenseReleaseComponents() failed (result = " << result2<< ")!" << endl;
			
		}

}


std::vector<FaceDetectionDetails> FaceDetector_Neurotech::Detect(const cv::Mat &frame, bool getDetailedInformation)
{
	Mat temp;
	cvtColor( frame, temp, CV_BGR2GRAY );
    if( !temp.isContinuous())
		cout << " Non continuous memory " << endl;
	
	HNImage oimage;

	vector<FaceDetectionDetails> faces_returned;

	result = NImageCreateFromDataEx(npfGrayscale, temp.cols, temp.rows, 0, temp.cols , temp.data ,temp.cols*temp.rows ,0,&oimage);
	if(NFailed(result))
	{
		cout << "NImageCreateFromDataEx failed (result = " << result<< ")!" << endl;
		NObjectFree(oimage);
		return faces_returned;
		//PrintErrorMsg(N_T("NImageToGrayscale() failed (result = %d)!"), result);
	
	}

	result = NImageToGrayscale(oimage, &grayscale);
	if (NFailed(result))
	{
		cout << "NImageToGrayscale() failed (result = " << result<< ")!" << endl;
		NObjectFree(oimage);
		return faces_returned;
		
	}


	result = NleDetectFaces(extractor, grayscale, &faceCount, &faces);
	if (NFailed(result))
	{
		cout << "NleDetectFaces() failed (result = " << result<< ")!" << endl;
		NObjectFree(oimage);
		return faces_returned;
		
	}

	
	for (i = 0; i < faceCount; ++i)
	{
		FaceDetectionDetails currentFaceDeets;
		//cout << i << endl;
		Rect currentFaceRect;
		currentFaceRect.x= faces[i].Rectangle.X ;
		currentFaceRect.y = faces[i].Rectangle.Y ; 
		currentFaceRect.width = faces[i].Rectangle.Width;
		currentFaceRect.height = faces[i].Rectangle.Height;
		Rect constrainedRect = ConstrainRect(currentFaceRect, Size(frame.cols, frame.rows));

		currentFaceDeets.faceRect = constrainedRect;
		currentFaceDeets.faceDetectionConfidence = faces[i].Confidence;

		if( getDetailedInformation )
		{
			// they have requested that we get more detailed information for this face
			result = NleDetectFacialFeatures(extractor, grayscale, &faces[i], &details);
			if (NFailed(result))
			{
				cout << "NleDetectFacialFeatures() failed (result = " << result<< "), maybe feature points were not found!" << endl;
				
				continue;
			}
			 // gender, expression and other proeprties only detected during extraction
			result = NleExtractUsingDetails(extractor, grayscale, &details, &status, &tmpl);
			if(NFailed(result))
			{
				cout << "NleExtractUsingDetails() failed (result = " << result<< "), maybe feature points were not found!" << endl;
				continue;
			}

			currentFaceDeets.hasAdditionalFaceInformation = true;

			// success
			currentFaceDeets.roll = details.Face.Rotation.Roll;
			currentFaceDeets.yaw = details.Face.Rotation.Yaw;
			currentFaceDeets.pitch = details.Face.Rotation.Pitch;

			// pull out the eye information
			if( details.EyesAvailable )
			{
				currentFaceDeets.leftEye.x = details.LeftEyeCenter.X;
				currentFaceDeets.leftEye.y = details.LeftEyeCenter.Y;
				if( details.LeftEyeCenter.Confidence == 254 || details.LeftEyeCenter.Confidence == 255 )
					currentFaceDeets.leftEyeConfidence = 0.0f;
				else
					currentFaceDeets.leftEyeConfidence = (float)(details.LeftEyeCenter.Confidence / 100.0f);

				currentFaceDeets.rightEye.x = details.RightEyeCenter.X;
				currentFaceDeets.rightEye.y = details.RightEyeCenter.Y;
				if( details.RightEyeCenter.Confidence == 254 || details.RightEyeCenter.Confidence == 255 )
					currentFaceDeets.rightEyeConfidence = 0.0f;
				else
					currentFaceDeets.rightEyeConfidence = (float)(details.RightEyeCenter.Confidence / 100.0f);

				if( currentFaceDeets.rightEyeConfidence > 0.0f && currentFaceDeets.leftEyeConfidence > 0.0f )
					// we can only calculate the intereye distance if we have both eye locations
					currentFaceDeets.intereyeDistance = sqrt( pow( currentFaceDeets.leftEye.x - currentFaceDeets.rightEye.x, 2 )
															 +pow( currentFaceDeets.leftEye.y - currentFaceDeets.rightEye.y, 2 ) );
				else
					currentFaceDeets.intereyeDistance = 0.0f;
			}

			// grab the mouth location information
			currentFaceDeets.mouthLocation.x = details.MouthCenter.X;
			currentFaceDeets.mouthLocation.y = details.MouthCenter.Y;
			if( details.MouthCenter.Confidence == 254 || details.MouthCenter.Confidence == 255 )
				currentFaceDeets.mouthLocationConfidence = 0.0f;
			else
				currentFaceDeets.mouthLocationConfidence = (float)(details.MouthCenter.Confidence / 100.0f);

			// grab the nose location information
			currentFaceDeets.noseLocation.x = details.NoseTip.X;
			currentFaceDeets.noseLocation.y = details.NoseTip.Y;
			if( details.NoseTip.Confidence == 254 || details.NoseTip.Confidence == 255 )
				currentFaceDeets.noseLocationConfidence = 0.0f;
			else
				currentFaceDeets.noseLocationConfidence = (float)(details.NoseTip.Confidence / 100.0f);

			// see if we have gender information
			if(details.Gender == ngUnspecified || details.Gender == ngUnknown)
			{
				currentFaceDeets.genderConfidence = 0.0f;
			}
			else
			{
				currentFaceDeets.isMale = details.Gender == ngMale;
				currentFaceDeets.genderConfidence = (float)(details.GenderConfidence / 100.0f);
			}

			// see if we have expression information
			if(details.Expression == nleUnknown || details.Expression == nleUnspecified)
				currentFaceDeets.expressionConfidence = 0.0f;
			else
			{
				// we need to convert the expression into one of our own enums for the various expressions
				// it could detect
				currentFaceDeets.expressionConfidence = (float)(details.ExpressionConfidence / 100.0f);
			}
			
		}

		faces_returned.push_back(currentFaceDeets);
	}

	NObjectFree(oimage);
	NObjectFree(grayscale);
	
	NObjectFree(tmpl);
	NFree(faces);

	return faces_returned;
}


Rect FaceDetector_Neurotech::ConstrainRect(const Rect &rectToConstrain, const Size &imageSize)
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

void FaceDetector_Neurotech::RenderVisualization(cv::Mat &frame, const std::vector<FaceDetectionDetails> &detectedFaces)
{

	// iterate over each of the faces
	auto faceIter = detectedFaces.begin();
	auto faceIterEnd = detectedFaces.end();

	for(; faceIter != faceIterEnd; faceIter++)
	{
		// Draw the face
		Point center( (*faceIter).faceRect.x + int((*faceIter).faceRect.width*0.5f), (*faceIter).faceRect.y + int((*faceIter).faceRect.height*0.5f) );
		ellipse( frame, center, Size( int((*faceIter).faceRect.width*0.5f), int((*faceIter).faceRect.height*0.5f)), 0, 0, 360, Scalar( 255, 0, 0 ), 2, 8, 0 );
	}

}


	}
}