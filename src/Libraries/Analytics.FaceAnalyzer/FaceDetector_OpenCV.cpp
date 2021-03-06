/*
	The entry point into the application. Parses command line options (or config files) and sets up the program
	flow based on said options

	Date: 25/06/2013

	Author: Jason Catchpole (jason@viblio.com)

*/

#include "FaceDetector_OpenCV.h"
#include "EyeDetector_OpenCV.h"
#include "FaceAnalyzerConfiguration.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/highgui.h>
#include <stdexcept>
#include <vector>
#include <boost/log/trivial.hpp>

using namespace std;
using namespace cv;

namespace Analytics
{
	namespace FaceAnalyzer
	{

FaceDetector_OpenCV::FaceDetector_OpenCV(FaceAnalyzerConfiguration *faceAnalyzerConfiguration)
	:
	FaceDetector(faceAnalyzerConfiguration)
{
	if( faceAnalyzerConfiguration == NULL )
	{
		BOOST_LOG_TRIVIAL(error) << "Face Analyzer configuration provided is NULL. Cannot construct OpenCV face detector";
		throw runtime_error("Face Analyzer configuration is NULL");
	}

	if( faceAnalyzerConfiguration->faceDetectorCascadeFile.empty() )
		return;

	if( !m_faceCascade.load( faceAnalyzerConfiguration->faceDetectorCascadeFile ) )
	{ 
		BOOST_LOG_TRIVIAL(error) << "Error loading face cascade with filename " << faceAnalyzerConfiguration->faceDetectorCascadeFile;
		throw runtime_error("Error loading face cascade with filename " + faceAnalyzerConfiguration->faceDetectorCascadeFile);
	}
	
	if( !faceAnalyzerConfiguration->eyeDetectorCascadeFile.empty() )
	{
		try
		{
			m_eyeDetector.reset( new EyeDetector_OpenCV(faceAnalyzerConfiguration->eyeDetectorCascadeFile) );
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

vector<FaceDetectionDetails> FaceDetector_OpenCV::Detect(const Mat &frame, bool getDetailedInformation)
{
	vector<FaceDetectionDetails> faceDetails;
	Mat frame_gray;

	// this copy of the frame is only needed for debug purposes, can be deleted
	Mat frameCopy;
	frame.copyTo(frameCopy);

	cvtColor( frame, frame_gray, CV_BGR2GRAY );
	equalizeHist( frame_gray, frame_gray );

	//-- Detect faces
	// If you end up using a HAAR cascade here then you should use face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );
	vector<Rect> faces;
	m_faceCascade.detectMultiScale( frame_gray, faces, 1.1, 3, 0 );//, Size(50, 50) );

	if( m_filterFacesByEyeDetections )
	{
		for( unsigned int i = 0; i < faces.size(); i++ )
		{
			FaceDetectionDetails currentFaceDeets;

			Rect constrainedRect = ConstrainRect(faces[i], Size(frame_gray.cols, frame_gray.rows));
		
			currentFaceDeets.faceRect = constrainedRect;
			currentFaceDeets.faceDetectionConfidence = 75; // we don't have a confidence measure from OpenCV. Arbitrarily set to 75%

			Mat faceROI = frame_gray( constrainedRect );

			// for each face pass it to the eye detector
			vector<Rect> eyes = m_eyeDetector->Detect(faceROI);
			
			if( eyes.size() == 2) // means we found the two eyes
			{
				faceDetails.push_back(currentFaceDeets);
			}
		}

		return faceDetails;
	}
	else
	{
		if(!faces.empty())
		{
			//-- Draw the face
			Point center( faces[0].x + int(faces[0].width*0.5f), faces[0].y + int(faces[0].height*0.5f) );
			ellipse( frameCopy, center, Size( int(faces[0].width*0.5f), int(faces[0].height*0.5f)), 0, 0, 360, Scalar( 255, 0, 0 ), 2, 8, 0 );
		}
		
		// no filtering so just return the found faces
		for( unsigned int i = 0; i < faces.size(); i++ )
		{
			FaceDetectionDetails currentFaceDeets;

			Rect constrainedRect = ConstrainRect(faces[i], Size(frame_gray.cols, frame_gray.rows));
		
			currentFaceDeets.faceRect = constrainedRect;
			currentFaceDeets.faceDetectionConfidence = 75; // we don't have a confidence measure from OpenCV. Arbitrarily set to 75%

			faceDetails.push_back(currentFaceDeets);
		}

		return faceDetails;
	}
}

// end of namespaces
	}
}
