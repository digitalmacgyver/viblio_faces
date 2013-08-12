/*
	The entry point into the application. Parses command line options (or config files) and sets up the program
	flow based on said options

	Date: 25/06/2013

	Author: Jason Catchpole (jason@viblio.com)

*/

#include "FaceDetector_OpenCV.h"
#include "EyeDetector_OpenCV.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/highgui.h>

#include <vector>

using namespace std;
using namespace cv;

namespace Analytics
{
	namespace FaceAnalyzer
	{

FaceDetector_OpenCV::FaceDetector_OpenCV(const string &face_cascade_name, const string &eyes_cascade_name)
{
	if( face_cascade_name.empty() )
		return;

	if( !m_faceCascade.load( face_cascade_name ) )
	{ 
		cout << "Error loading face cascade with filename " << face_cascade_name << endl;
		throw runtime_error("Error loading face cascade with filename " + face_cascade_name);
	}
	
	if( !eyes_cascade_name.empty() )
	{
		try
		{
			m_eyeDetector.reset( new EyeDetector_OpenCV(eyes_cascade_name) );
		}
		catch(Exception e)
		{
			throw e;
		}

		m_filterFacesByEyeDetections = true;
	}
	else
	{
		m_eyeDetector = NULL;
		m_filterFacesByEyeDetections = false;
	}
}


FaceDetector_OpenCV::~FaceDetector_OpenCV()
{
}

vector<Rect> FaceDetector_OpenCV::Detect(const Mat &frame)
{
	vector<Rect> faces;
	Mat frame_gray;

	// this copy of the frame is only needed for debug purposes, can be deleted
	Mat frameCopy;
	frame.copyTo(frameCopy);

	cvtColor( frame, frame_gray, CV_BGR2GRAY );
	equalizeHist( frame_gray, frame_gray );

	//-- Detect faces
	// If you end up using a HAAR cascade here then you should use face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );
	m_faceCascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0, Size(80, 80) );

	if( m_filterFacesByEyeDetections )
	{
		vector<Rect> filteredFaces;

		for( unsigned int i = 0; i < faces.size(); i++ )
		{
			Rect constrainedRect = ConstrainRect(faces[i], Size(frame_gray.cols, frame_gray.rows));

			Mat faceROI = frame_gray( constrainedRect );

			// for each face pass it to the eye detector
			vector<Rect> eyes = m_eyeDetector->Detect(faceROI);
			
			if( eyes.size() == 2) // means we found the two eyes
			{
				filteredFaces.push_back(constrainedRect);

				//-- Draw the face
				Point center( constrainedRect.x + int(constrainedRect.width*0.5f), constrainedRect.y + int(constrainedRect.height*0.5f) );
				ellipse( frameCopy, center, Size( int(constrainedRect.width*0.5f), int(constrainedRect.height*0.5f)), 0, 0, 360, Scalar( 255, 0, 0 ), 2, 8, 0 );

				for( unsigned int j = 0; j < eyes.size(); j++ )
				{ //-- Draw the eyes
					Point center( constrainedRect.x + eyes[j].x + int(eyes[j].width*0.5f), constrainedRect.y + eyes[j].y + int(eyes[j].height*0.5f) );
					int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25f );
					circle( frameCopy, center, radius, Scalar( 255, 0, 255 ), 3, 8, 0 );
				}
			}
		}

		//-- Show what you got
		imshow( "temp", frameCopy );

		return filteredFaces;
	}
	else
	{
		if(!faces.empty())
		{
		//-- Draw the face
		Point center( faces[0].x + int(faces[0].width*0.5f), faces[0].y + int(faces[0].height*0.5f) );
		ellipse( frameCopy, center, Size( int(faces[0].width*0.5f), int(faces[0].height*0.5f)), 0, 0, 360, Scalar( 255, 0, 0 ), 2, 8, 0 );
		}
		//-- Show what you got
		imshow( "temp", frameCopy );

		// no filtering so just return the found faces
		return faces;
	}
}

// Ensures that the rect passed in is valid based on the image size it is supposedly from. Returns
// a rect that is sure to be inside the bounds of the image
Rect FaceDetector_OpenCV::ConstrainRect(const Rect &rectToConstrain, const Size &imageSize)
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

// end of namespaces
	}
}