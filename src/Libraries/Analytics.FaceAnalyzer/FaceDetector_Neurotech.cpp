#include "FaceDetector_Neurotech.h"
#include "FaceDetectionDetails.h"
#include "ThumbnailDetails.h"
#include <opencv2/opencv.hpp>
#include <boost/log/trivial.hpp>

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


FaceDetector_Neurotech::FaceDetector_Neurotech(FaceAnalyzerConfiguration *faceAnalyzerConfiguration)
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
	 templateSize(nletsSmall),
	 FaceDetector(faceAnalyzerConfiguration)
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
	
  /*
	try
	{
		
	     result = NLicenseObtainComponents(N_T("/local"), N_T("5000"), components, &available);
		 if (NFailed(result))
			 throw runtime_error("Failed to obtain Neurotech licenses & Image quality related components");
		 if(!available)
			throw runtime_error("Neurotech licenses for the face detection related components are not available");
	}
	catch(int e)
	{
		BOOST_LOG_TRIVIAL(error) << "Exception caught while attempting to obtain Neurotech product license. Cannot continue";
		if (!available)
			BOOST_LOG_TRIVIAL(error) << "Neurotech Licenses for " << components << "  not available";
		throw e;
	}
  */
	// create an extractor
	try
	{
	     result = NleCreate(&extractor);
		 if (NFailed(result))
			 throw runtime_error("Failed to initialise extractor");
	}
	catch(int e)
	{
		BOOST_LOG_TRIVIAL(error) << "NleCreate() failed (result = " << result << ")";
		throw e;
	}
	
	
	// Detect all feature points
	try
	{
	     result = NObjectSetParameterEx(extractor, NLEP_DETECT_ALL_FEATURE_POINTS, -1, &detectAllFeatures, sizeof(NBool));
		 if (NFailed(result))
			 throw runtime_error("Failed to initialise extractor NLEP_DETECT_ALL_FEATURE_POINTS");
	}
	catch(int e)
	{
		BOOST_LOG_TRIVIAL(error) << "NObjectSetParameter() failed (result = " << result<< ")";
		throw e;
	}
	
	
	// Detect all base feature Points
	try
	{
	     result = NObjectSetParameterEx(extractor, NLEP_DETECT_BASE_FEATURE_POINTS, -1, &detectBaseFeatures, sizeof(NBool));
		 if (NFailed(result))
			 throw runtime_error("Failed to initialise extractor NLEP_DETECT_BASE_FEATURE_POINTS");
	}
	catch(int e)
	{
		BOOST_LOG_TRIVIAL(error) << "NObjectSetParameter() failed (result = " << result<< ")";
		throw e;
	}
	
	// 
	NInt maxYawAngle = 45;
	try
	{
	   result = NObjectSetParameterEx(extractor, NLEP_MAX_YAW_ANGLE_DEVIATION, N_TYPE_INT, &maxYawAngle, sizeof(NInt));
	   if (NFailed(result))
		   throw runtime_error("Failed to initialise extractor NLEP_MAX_YAW_ANGLE_DEVIATION");
	}
	catch(int e)
	{
		BOOST_LOG_TRIVIAL(error) << "NObjectSetParameter() failed (result = " << result<< ")";
		throw e;
	}
	
	// Detect Gender
	try
	{
	    result = NObjectSetParameterEx(extractor, NLEP_DETECT_GENDER, -1, &detectGender, sizeof(NBool));
		 if (NFailed(result))
			 throw runtime_error("Failed to initialise extractor NLEP_DETECT_GENDER");
	}
	catch(int e)
	{
		BOOST_LOG_TRIVIAL(error) << "NObjectSetParameter() failed (result = " << result<< ")";
		throw e;
	}
	
	// Detect Expression
	try
	{
	    result = NObjectSetParameterEx(extractor, NLEP_DETECT_EXPRESSION, -1, &detectExpression, sizeof(NBool));
		 if (NFailed(result))
			 throw runtime_error("Failed to initialise extractor  NLEP_DETECT_EXPRESSION");
	}
	catch(int e)
	{
		BOOST_LOG_TRIVIAL(error) << "NObjectSetParameter() failed (result = " << result<< ")";
		throw e;
	}

   // Detect Blink
	try
	{
	   result = NObjectSetParameterEx(extractor, NLEP_DETECT_BLINK, -1, &detectBlink, sizeof(NBool));
		 if (NFailed(result))
			  throw runtime_error("Failed to initialise extractor  NLEP_DETECT_BLINK");
	}
	catch(int e)
	{
		BOOST_LOG_TRIVIAL(error) << "NObjectSetParameter() failed (result = " << result<< ")";
		throw e;
	}

	// Detect Mouth Open
	try
	{
	   result = NObjectSetParameterEx(extractor, NLEP_DETECT_MOUTH_OPEN, -1, &detectMouthOpen, sizeof(NBool));
		 if (NFailed(result))
			 throw runtime_error("Failed to initialise extractor NLEP_DETECT_MOUTH_OPEN");
	}
	catch(int e)
	{
		BOOST_LOG_TRIVIAL(error) << "NObjectSetParameter() failed (result = " << result << ")";
		throw e;
	}

	// Detect Glasses
	
	try
	{
	   result = NObjectSetParameterEx(extractor, NLEP_DETECT_GLASSES, -1, &detectGlasses, sizeof(NBool));
		 if (NFailed(result))
			 throw runtime_error("Failed to initialise extractor NLEP_DETECT_GLASSES");
	}
	catch(int e)
	{
		BOOST_LOG_TRIVIAL(error) << "NObjectSetParameter() failed (result = " << result<< ")";
		throw e;
	}

	// Detect dark Glasses

	try
	{
	   result = NObjectSetParameterEx(extractor, NLEP_DETECT_DARK_GLASSES, -1, &detectDarkGlasses, sizeof(NBool));
		 if (NFailed(result))
			 throw runtime_error("Failed to initialise extractor NLEP_DETECT_DARK_GLASSES");
	}
	catch(int e)
	{
		BOOST_LOG_TRIVIAL(error) << "NObjectSetParameter() failed (result = " << result << ")";
		throw e;
	}

	
	try
	{
	   result = NObjectSetParameterEx(extractor, NLEP_TEMPLATE_SIZE, -1, &templateSize, sizeof(NleTemplateSize));
		 if (NFailed(result))
			  throw runtime_error("Failed to initialise extractor NLEP_TEMPLATE_SIZE");
	}
	catch(int e)
	{
		BOOST_LOG_TRIVIAL(error) << "NObjectSetParameter() failed (result = " << result << ")";
		throw e;
	}
	
}
//
//
FaceDetector_Neurotech::~FaceDetector_Neurotech(void)
{
	if( extractor )
		NObjectFree(extractor);

	/*
	NResult result2 = NLicenseReleaseComponents(components);
	if (NFailed(result2))
	{
		BOOST_LOG_TRIVIAL(error) << "NLicenseReleaseComponents() failed (result = " << result2<< ")";
	}
	*/

}


std::vector<FaceDetectionDetails> FaceDetector_Neurotech::Detect(const cv::Mat &frame, bool getDetailedInformation)
{
	Mat temp;
	cvtColor( frame, temp, CV_BGR2GRAY );
    if( !temp.isContinuous())
		BOOST_LOG_TRIVIAL(info) << "Non continuous memory detected";
	
	HNImage oimage;

	vector<FaceDetectionDetails> faces_returned;

	result = NImageCreateFromDataEx(npfGrayscale, temp.cols, temp.rows, 0, temp.cols , temp.data ,temp.cols*temp.rows ,0,&oimage);
	if(NFailed(result))
	{
		BOOST_LOG_TRIVIAL(error) << "NImageCreateFromDataEx failed (result = " << result<< ")";
		NObjectFree(oimage);
		oimage = NULL;
		return faces_returned;
		//PrintErrorMsg(N_T("NImageToGrayscale() failed (result = %d)!"), result);
	
	}

	result = NImageToGrayscale(oimage, &grayscale);
	if (NFailed(result))
	{
		BOOST_LOG_TRIVIAL(error) << "NImageToGrayscale() failed (result = " << result << ")";
		NObjectFree(oimage);
		oimage = NULL;
		return faces_returned;
		
	}


	result = NleDetectFaces(extractor, grayscale, &faceCount, &faces);
	if (NFailed(result))
	{
		BOOST_LOG_TRIVIAL(error) << "NleDetectFaces() failed (result = " << result<< ")";
		NObjectFree(oimage);
		oimage = NULL;
		return faces_returned;
		
	}

	
	for (i = 0; i < faceCount; ++i)
	{
		FaceDetectionDetails currentFaceDeets;
		
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
				BOOST_LOG_TRIVIAL(error) << "NleDetectFacialFeatures() failed (result = " << result<< "), maybe feature points were not found";
				
				continue;
			}
			 // gender, expression and other proeprties only detected during extraction
			result = NleExtractUsingDetails(extractor, grayscale, &details, &status, &tmpl);
			if(NFailed(result))
			{
				BOOST_LOG_TRIVIAL(error) << "NleExtractUsingDetails() failed (result = " << result<< "), maybe feature points were not found";
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
				{
					// we can only calculate the intereye distance if we have both eye locations
					if( currentFaceDeets.leftEye.y == currentFaceDeets.rightEye.y )
						currentFaceDeets.intereyeDistance = float(currentFaceDeets.leftEye.x - currentFaceDeets.rightEye.x);
					else
						currentFaceDeets.intereyeDistance = sqrt( pow( currentFaceDeets.leftEye.x - currentFaceDeets.rightEye.x, 2 )
																 +pow( currentFaceDeets.leftEye.y - currentFaceDeets.rightEye.y, 2 ) );
				}
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
			currentFaceDeets.expression = ConvertNlpExpression(details.Expression);
			if(details.Expression == FaceDetectionDetails::ExpressionType::Expression_Unknown || 
			   details.Expression == FaceDetectionDetails::ExpressionType::Expression_Unspecified)
				currentFaceDeets.expressionConfidence = 0.0f;
			else
			{
				// we need to convert the expression into one of our own enums for the various expressions
				// it could detect
				currentFaceDeets.expressionConfidence = (float)(details.ExpressionConfidence / 100.0f);
			}

			// glasses information
			if(details.GlassesConfidence ==255)
			{
				currentFaceDeets.wearingGlasses = false;
				currentFaceDeets.glassesConfidence =0.0f;
			}
			else
			{
				if((details.Properties & nlpGlasses) == nlpGlasses)
					currentFaceDeets.wearingGlasses = true;
				else
					currentFaceDeets.wearingGlasses = false;

				currentFaceDeets.glassesConfidence = (float)(details.GlassesConfidence/100.0f);
			}
			// dark glasses information
			if(details.DarkGlassesConfidence == 255)
			{
				currentFaceDeets.wearingDarkGlasses = false;
				currentFaceDeets.wearingDarkGlassesConfidence =0.0f;
			}
			else
			{
				if((details.Properties & nlpDarkGlasses) == nlpDarkGlasses)
					currentFaceDeets.wearingDarkGlasses = true;
				else
					currentFaceDeets.wearingDarkGlasses = false;

				currentFaceDeets.wearingDarkGlassesConfidence = (float)(details.DarkGlassesConfidence/100.0f);
			}
			
			if(details.BlinkConfidence == 255)
			{
				currentFaceDeets.blinking = false;
				currentFaceDeets.blinkingConfidence = 0.0f;
			}
			else
			{
				if((details.Properties & nlpBlink) == nlpBlink)
					currentFaceDeets.blinking = true;
				else
					currentFaceDeets.blinking = false;

				currentFaceDeets.blinkingConfidence = (float)(details.BlinkConfidence/100.0f);
			}

			if(details.MouthOpenConfidence == 255)
			{
				currentFaceDeets.mouthOpen = false;
				currentFaceDeets.mouthOpenConfidence =0.0f;
			}
			else
			{
				if((details.Properties & nlpMouthOpen) == nlpMouthOpen)
					currentFaceDeets.mouthOpen = true;
				else
					currentFaceDeets.mouthOpen = false;

				currentFaceDeets.mouthOpenConfidence = (float)(details.MouthOpenConfidence/100.0f);
			}
		}

		faces_returned.push_back(currentFaceDeets);
	}

	if( oimage != NULL )
	{
		NObjectFree(oimage);
		oimage = NULL;
	}

	if( grayscale != NULL )
	{
		NObjectFree(grayscale);
		grayscale = NULL;
	}
	
	if( tmpl != NULL )
	{
		NObjectFree(tmpl);
		tmpl = NULL;
	}

	if( faces != NULL )
	{
		NFree(faces);
		faces = NULL;
	}

	return faces_returned;
}

FaceDetectionDetails::ExpressionType FaceDetector_Neurotech::ConvertNlpExpression(NLExpression nlpExpression)
{
	switch(nlpExpression)
	{
	case nleUnspecified:
		return FaceDetectionDetails::ExpressionType::Expression_Unspecified;
	case nleNeutral:
		return FaceDetectionDetails::ExpressionType::Expression_Neutral;
	case nleSmile:
		return FaceDetectionDetails::ExpressionType::Expression_Smile;
	case nleSmileOpenedJaw:
		return FaceDetectionDetails::ExpressionType::Expression_SmileOpenedJaw;
	case nleRaisedBrows:
		return FaceDetectionDetails::ExpressionType::Expression_RaisedBrows;
	case nleEyesAway:
		return FaceDetectionDetails::ExpressionType::Expression_EyesAway;
	case nleSquinting:
		return FaceDetectionDetails::ExpressionType::Expression_Squinting;
	case nleFrowning:
		return FaceDetectionDetails::ExpressionType::Expression_Frowning;
	default:
		return FaceDetectionDetails::ExpressionType::Expression_Unknown;
	};
}

	}
}
