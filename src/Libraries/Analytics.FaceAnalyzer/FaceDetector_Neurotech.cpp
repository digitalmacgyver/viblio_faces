#include "FaceDetector_Neurotech.h"

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

	// read image
	
	

	// create an extractor
	result = NleCreate(&extractor);
	if (NFailed(result))
	{
		cout << "NleCreate() failed (result = " << result<< ")!" << endl;
		//PrintErrorMsg(N_T("NleCreate() failed (result = %d)!"), result);
		
	}

	result = NObjectSetParameterEx(extractor, NLEP_DETECT_ALL_FEATURE_POINTS, -1, &detectAllFeatures, sizeof(NBool));
	if (NFailed(result))
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		//PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
		
	}

	result = NObjectSetParameterEx(extractor, NLEP_DETECT_BASE_FEATURE_POINTS, -1, &detectBaseFeatures, sizeof(NBool));
	if (NFailed(result))
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		//PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
		
	}

	result = NObjectSetParameterEx(extractor, NLEP_DETECT_GENDER, -1, &detectGender, sizeof(NBool));
	if (NFailed(result))
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		//PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
		
	}

	result = NObjectSetParameterEx(extractor, NLEP_DETECT_EXPRESSION, -1, &detectExpression, sizeof(NBool));
	if (NFailed(result))
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		//PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
		
	}

	result = NObjectSetParameterEx(extractor, NLEP_DETECT_BLINK, -1, &detectBlink, sizeof(NBool));
	if (NFailed(result))
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		//PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
		
	}

	result = NObjectSetParameterEx(extractor, NLEP_DETECT_MOUTH_OPEN, -1, &detectMouthOpen, sizeof(NBool));
	if (NFailed(result))
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		//PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
		
	}

	result = NObjectSetParameterEx(extractor, NLEP_DETECT_GLASSES, -1, &detectGlasses, sizeof(NBool));
	if (NFailed(result))
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		//PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
	
	}

	result = NObjectSetParameterEx(extractor, NLEP_DETECT_DARK_GLASSES, -1, &detectDarkGlasses, sizeof(NBool));
	if (NFailed(result))
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		//PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
	
	}

	result = NObjectSetParameterEx(extractor, NLEP_TEMPLATE_SIZE, -1, &templateSize, sizeof(NleTemplateSize));
	if (NFailed(result))
	{
		cout << "NObjectSetParameter() failed (result = " << result<< ")!" << endl;
		//PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
		
	}

	
}
//
//
FaceDetector_Neurotech::~FaceDetector_Neurotech(void)
{

	
		NResult result2 = NLicenseReleaseComponents(components);
		if (NFailed(result2))
		{
			cout << "NLicenseReleaseComponents() failed (result = " << result2<< ")!" << endl;
			
		}

}

std::vector<cv::Rect> FaceDetector_Neurotech::Detect(const cv::Mat &frame)
{
	Mat temp;
	cvtColor( frame, temp, CV_BGR2GRAY );
    
	//Mat temp2= temp.clone();
	//Mat temp = frame.clone();
	//unsigned char *input = (unsigned char*)(temp.data);
	/*
	NSizeType size, strval;
	NUInt pixformat, width, height; 
	*/
	HNImage oimage;

	/* color image
	pixformat = NPF_RGB_8U;
	width = temp.cols;
	height = temp.rows;
	strval = width*3;
	size = width*height*3;
	*/
	
	//pixformat = NPF_GRAYSCALE_8U;
	/*
	width = temp.cols;
	height = temp.rows;
	strval = temp.cols;
	size = width*height;
	*/
	//imwrite( "image2_opencv.jpg", temp );
	//const NChar * img1 = N_T("image2_neuro.jpg");
	vector<Rect> faces_returned;

result = NImageCreateFromDataEx(npfGrayscale, temp.cols, temp.rows, 0, temp.cols , temp.data ,temp.cols*temp.rows ,0,&oimage);
if (NFailed(result))
	{
		cout << "NImageCreateFromDataEx failed (result = " << result<< ")!" << endl;
		//PrintErrorMsg(N_T("NImageToGrayscale() failed (result = %d)!"), result);
	
	}
//NImageSaveToFileEx(oimage,img1,NULL,NULL,NULL);
	// convert image to grayscale
	
	//const NChar * img = N_T("image1.jpg");

	//result = NImageCreateFromFileEx(img, NULL, 0, NULL, &image);

	result = NImageToGrayscale(oimage, &grayscale);
	if (NFailed(result))
	{
		cout << "NImageToGrayscale() failed (result = " << result<< ")!" << endl;
		//PrintErrorMsg(N_T("NImageToGrayscale() failed (result = %d)!"), result);
	
	}

	NObjectFree(oimage);
	oimage = NULL;

	result = NleDetectFaces(extractor, grayscale, &faceCount, &faces);
	if (NFailed(result))
	{
		cout << "NleDetectFaces() failed (result = " << result<< ")!" << endl;
		//PrintErrorMsg(N_T("NleDetectFaces() failed (result = %d)!"), result);
		//goto FINALLY;
	}

	//printf(N_T("found faces:\n"));
	for (i = 0; i < faceCount; ++i)
	{
		//cout << i << endl;
		Rect temp;
		temp.x= faces[i].Rectangle.X ;
		temp.y = faces[i].Rectangle.Y ; 
		temp.width = faces[i].Rectangle.Width;
		temp.height = faces[i].Rectangle.Height;
		Rect constrainedRect = ConstrainRect(temp, Size(frame.cols, frame.rows));
		faces_returned.push_back(constrainedRect);
		
		// detect features for current face
		cout << " \t location =  (" << constrainedRect.x <<","<< constrainedRect.y<<"), width = "<< constrainedRect.width <<
			", height = " << constrainedRect.height << endl;
		/*
		result = NleDetectFacialFeatures(extractor, grayscale, &faces[i], &details);
		if (NFailed(result))
		{
			cout << "NleDetectFacialFeatures() failed (result = " << result<< "), maybe feature points were not found!" << endl;
			//PrintErrorMsg(N_T("NleDetectFacialFeatures() failed (result = %d), maybe feature points were not found!"), result);
			continue;
		}

		*/
		/*
		// gender, expression and other proeprties only detected during extraction
		result = NleExtractUsingDetails(extractor, grayscale, &details, &status, &tmpl);
		if(NFailed(result))
		{
			cout << "NleExtractUsingDetails() failed (result = " << result<< ")!" << endl;
			//PrintErrorMsg(N_T("NleExtractUsingDetails() failed (result = %d)"), result);
			continue;
		}
		NObjectFree(tmpl); tmpl = NULL;
		*/
		//printf(N_T("\n"));
		//printf(N_T("\t\textraction status: %d\n"), status);
		
		/*if(details.GenderConfidence == 255) 
			printf(N_T("\t\tgender not detected\n"));
		printf(N_T("\t\tgender: %s, confidence: %d\n"), (details.Gender == ngMale ? N_T("Male") : N_T("Female")), details.GenderConfidence);
		if (details.ExpressionConfidence == 255) printf(N_T("\t\texpression not detected\n"));
		else if (details.Expression == nleSmile) printf(N_T("\t\texpression: smile, confidence: %d\n"), details.GenderConfidence);
		if (details.BlinkConfidence == 255) printf(N_T("\t\tblink not detected\n"));
		else printf(N_T("\t\tblink: %s, confidence: %d\n"), (details.Properties & nlpBlink) == nlpBlink ? N_T("Yes") : N_T("No"), details.BlinkConfidence);
		if (details.MouthOpenConfidence == 255) printf(N_T("\t\tmouth open not detected\n"));
		else printf(N_T("\t\tmouth open: %s, confidence: %d\n"), (details.Properties & nlpMouthOpen) == nlpMouthOpen ? N_T("Yes") : N_T("No"), details.MouthOpenConfidence);
		if (details.GlassesConfidence == 255) printf(N_T("\t\tglasses not detected\n"));
		else printf(N_T("\t\tglasses: %s, confidence: %d\n"), (details.Properties & nlpGlasses) == nlpGlasses ? N_T("Yes") : N_T("No"), details.GlassesConfidence);
		if (details.DarkGlassesConfidence == 255) printf(N_T("\t\tdark glasses not detected\n"));
		else printf(N_T("\t\tdark glasses: %s, confidence: %d\n"), (details.Properties & nlpDarkGlasses) == nlpDarkGlasses ? N_T("Yes") : N_T("No"), details.DarkGlassesConfidence);*/
	}

	result = N_OK;

	NObjectFree(oimage);
	NObjectFree(grayscale);
	NObjectFree(extractor);
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

void FaceDetector_Neurotech::RenderVisualization(cv::Mat &frame, const std::vector<cv::Rect> &detectedFaces)
{

	// iterate over each of the faces
	auto faceIter = detectedFaces.begin();
	auto faceIterEnd = detectedFaces.end();

	for(; faceIter != faceIterEnd; faceIter++)
	{
		// Draw the face
		Point center( (*faceIter).x + int((*faceIter).width*0.5f), (*faceIter).y + int((*faceIter).height*0.5f) );
		ellipse( frame, center, Size( int((*faceIter).width*0.5f), int((*faceIter).height*0.5f)), 0, 0, 360, Scalar( 255, 0, 0 ), 2, 8, 0 );
	}

}


	}
}