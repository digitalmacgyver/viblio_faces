#include "FaceDetector_Neurotech.h"
#include "FaceDetectionDetails.h"

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
	 grayscale1(NULL),
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

std::string FaceDetector_Neurotech::Detect_return_json(const cv::Mat &frame1, const string &path,int id)
{
	Mat temp2;
	cvtColor( frame1, temp2, CV_BGR2GRAY );
    if( !temp2.isContinuous())
		cout << " Non continuous memory " << endl;

	//imwrite("frame1.jpg",frame1);


	HNImage ooimage;
	ooimage=NULL;
   std::string faces_jzon;
	vector<Rect> faces_returned;

result = NImageCreateFromDataEx(npfGrayscale, temp2.cols, temp2.rows, 0, temp2.cols , temp2.data ,temp2.cols*temp2.rows ,0,&ooimage);
if (NFailed(result))
	{
		cout << "NImageCreateFromDataEx failed (result = " << result<< ")!" << endl;
		NObjectFree(ooimage);
		return faces_jzon;
		//PrintErrorMsg(N_T("NImageToGrayscale() failed (result = %d)!"), result);
	
	}

	result = NImageToGrayscale(ooimage, &grayscale1);
	if (NFailed(result))
	{
		cout << "NImageToGrayscale() failed (result = " << result<< ")!" << endl;
		NObjectFree(ooimage);
		return faces_jzon;
		
	}


	result = NleDetectFaces(extractor, grayscale1, &faceCount, &faces);
	if (NFailed(result))
	{
		cout << "NleDetectFaces() failed (result = " << result<< ")!" << endl;
		NObjectFree(ooimage);
		return faces_jzon;
		//PrintErrorMsg(N_T("NleDetectFaces() failed (result = %d)!"), result);
		//goto FINALLY;
	}

	// Output in Jzon string Value
	 



	//printf(N_T("found faces:\n"));
	for (i = 0; i < faceCount; ++i)
	{
		Jzon::Object root;
		//cout << i << endl;
		Rect temp;
		temp.x= faces[i].Rectangle.X ;
		temp.y = faces[i].Rectangle.Y ; 
		temp.width = faces[i].Rectangle.Width;
		temp.height = faces[i].Rectangle.Height;
		Rect constrainedRect = ConstrainRect(temp, Size(frame1.cols, frame1.rows));
		faces_returned.push_back(constrainedRect);
		
		// Jzon addition
		  root.Add("face_id",id);
		  root.Add("s3_bucket","s3_bucket");
		  root.Add("s3_key",path);
		  root.Add("md5sum","md5sum");
		  root.Add("face_confidence",faces[i].Confidence);
		  root.Add("face_rotation_pitch",faces[i].Rotation.Pitch);
		  root.Add("face_rotation_roll",faces[i].Rotation.Roll);
		  root.Add("face_rotation_yaw",faces[i].Rotation.Yaw);
		  root.Add("width",faces[i].Rectangle.Width);
		  root.Add("height",faces[i].Rectangle.Height);

		 
		// detect features for current face
		cout << " \t location =  (" << constrainedRect.x <<","<< constrainedRect.y<<"), width = "<< constrainedRect.width <<
			", height = " << constrainedRect.height << endl;

		// Extra features
		
		
		result = NleDetectFacialFeatures(extractor, grayscale1, &faces[i], &details);
		if (NFailed(result))
		{
			cout << "NleDetectFacialFeatures() failed (result = " << result<< "), maybe feature points were not found!" << endl;
			Jzon::Writer writer(root, Jzon::StandardFormat);
			writer.Write();
		// Writing everything ot a string to export
		    std::string result = writer.GetResult();
			faces_jzon = faces_jzon + result;
			continue;
		}
	
		/*
	
		//Jzon Addition
		Jzon::Array left_eye;
		left_eye.Add(int(&details.LeftEyeCenter.X));left_eye.Add(int(&details.LeftEyeCenter.Y));
		root.Add("LeftEyeCenter",left_eye);
		Jzon::Array right_eye;
		right_eye.Add(int(&details.RightEyeCenter.X));right_eye.Add(int(&details.RightEyeCenter.Y));
		root.Add("RightEyeCenter",right_eye);
		Jzon::Array mouth_center;
		mouth_center.Add(int(&details.MouthCenter.X));mouth_center.Add(int(&details.MouthCenter.Y));
		root.Add("MouthCenter",mouth_center);
		Jzon::Array nose_tip;
		nose_tip.Add(int(&details.NoseTip.X));nose_tip.Add(int(&details.NoseTip.Y));
		root.Add("NoseTip",nose_tip);
	  */
		// gender, expression and other proeprties only detected during extraction
		result = NleExtractUsingDetails(extractor, grayscale1, &details, &status, &tmpl);
		if(NFailed(result))
		{
			cout << "NleExtractUsingDetails() failed (result = " << result<< ")!" << endl;
			Jzon::Writer writer(root, Jzon::StandardFormat);
			writer.Write();
		// Writing everything ot a string to export
		    std::string result = writer.GetResult();
			faces_jzon = faces_jzon + result;
			continue;
		}
		
		NObjectFree(tmpl); tmpl = NULL;
		
		
		
	
	//Jzon Addition
		
		details.Gender == ngMale ? root.Add("Gender","male"):root.Add("Gender","female"); 
		root.Add("Genderconfidence",details.GenderConfidence);
		

		(details.Properties & nlpBlink) == nlpBlink ? root.Add("Blink","yes") : root.Add("Blink","no");
		root.Add("Blinkconfidence",details.BlinkConfidence);

	    (details.Properties & nlpMouthOpen) == nlpMouthOpen ?root.Add("MouthOpen" ,"Yes") : root.Add("MouthOpen" ,"No");
	    root.Add("MouthOpenConfidence",details.MouthOpenConfidence);

		(details.Properties & nlpGlasses) == nlpGlasses ?root.Add("Glasses" ,"Yes") : root.Add("Glasses" ,"No");
	    root.Add("GlassesConfidence",details.GlassesConfidence);

		(details.Properties & nlpDarkGlasses) == nlpDarkGlasses ?root.Add("DarkGlasses" ,"Yes") : root.Add("DarkGlasses" ,"No");
	    root.Add("DarkGlassesConfidence",details.DarkGlassesConfidence);

		Jzon::Writer writer(root, Jzon::StandardFormat);
        writer.Write();
		// Writing everything ot a string to export
        std::string result = writer.GetResult();
		faces_jzon = faces_jzon + result;

		
	
	}
	return faces_jzon;
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
				currentFaceDeets.leftEyeConfidence = details.LeftEyeCenter.Confidence;

				currentFaceDeets.rightEye.x = details.RightEyeCenter.X;
				currentFaceDeets.rightEye.y = details.RightEyeCenter.Y;
				currentFaceDeets.rightEyeConfidence = details.RightEyeCenter.Confidence;
			}

			// grab the mouth location information
			currentFaceDeets.mouthLocation.x = details.MouthCenter.X;
			currentFaceDeets.mouthLocation.y = details.MouthCenter.Y;
			currentFaceDeets.mouthLocationConfidence = details.MouthCenter.Confidence;

			// grab the nose location information
			currentFaceDeets.noseLocation.x = details.NoseTip.X;
			currentFaceDeets.noseLocation.y = details.NoseTip.Y;
			currentFaceDeets.noseLocationConfidence = details.NoseTip.Confidence;

			// see if we have gender information
			if(details.Gender == ngUnspecified || details.Gender == ngUnknown)
			{
				currentFaceDeets.genderConfidence = 0.0f;
			}
			else
			{
				currentFaceDeets.isMale = details.Gender == ngMale;
				currentFaceDeets.genderConfidence = details.GenderConfidence;
			}

			// see if we have expression information
			if(details.Expression == nleUnknown || details.Expression == nleUnspecified)
				currentFaceDeets.expressionConfidence = 0.0f;
			else
			{
				// we need to convert the expression into one of our own enums for the various expressions
				// it could detect
				currentFaceDeets.expressionConfidence = details.ExpressionConfidence;
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