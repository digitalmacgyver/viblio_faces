/*
	The entry point into the application. Parses command line options (or config files) and sets up the program
	flow based on said options

	Date: 01/07/2013

	Author: Jason Catchpole (jason@viblio.com)

*/


#include "FaceAnalyzer.h"
#include "FaceAnalyzerConfiguration.h"
#include "TrackingController.h"
#include "FaceDetectionDetails.h"
#include "FileSystem/FileSystem.h"
#include <boost/any.hpp>
#include <stdexcept>
#include <opencv2/opencv.hpp>
#include <chrono>
#include <future>
#include <iostream>

using namespace std;
using namespace cv;

namespace Analytics
{
	namespace FaceAnalyzer
	{


// pass in some config information or have a global configuration settings object
FaceAnalysis::FaceAnalysis(FaceAnalyzerConfiguration *faceAnalyzerConfig)
{
	if( faceAnalyzerConfig == NULL )
		throw runtime_error("Face analyzer config was NULL");

	if( !faceAnalyzerConfig->faceThumbnailOutputPath.empty() )
	{
		// if they specified an output path for thumbnails then make sure it exists, if it doesn't
		// its an error so we will throw an exception (perhaps in future we could create the path?)
		if( !FileSystem::DirectoryExists(faceAnalyzerConfig->faceThumbnailOutputPath) )
			FileSystem::CreateDirectory(faceAnalyzerConfig->faceThumbnailOutputPath);
			//throw runtime_error("Specified output thumbnail directory does not exist");
	}

	//m_faceDetector.reset( new FaceDetector_OpenCV(faceAnalyzerConfig->faceDetectorCascadeFile, faceAnalyzerConfig->eyeDetectorCascadeFile) );
	m_faceDetector.reset( new FaceDetector_Neurotech( ));
	m_trackingController.reset( new TrackingController(faceAnalyzerConfig) );

	m_faceDetectionFrequency = faceAnalyzerConfig->faceDetectionFrequency;
	m_currentFrameNumber = 0;

	m_imageRescaleFactor = faceAnalyzerConfig->imageRescaleFactor;

	m_renderVisualization = faceAnalyzerConfig->renderVisualization;
}


FaceAnalysis::~FaceAnalysis()
{
	
}

void FaceAnalysis::Process(const Mat &frame, uint64_t frameTimestamp)
{
	Mat resizedFrame;
	frameInfo.setoriginalFrame(frame);
	if( m_imageRescaleFactor != 1.0f )
	{
		resize(frame, resizedFrame, Size(frame.cols * m_imageRescaleFactor, frame.rows * m_imageRescaleFactor));
		frameInfo.setscale((1.0 /m_imageRescaleFactor));
		
	}
	else
	{// Auto resize to 480p ( 640 x 480)
		const int DETECTION_WIDTH = 640;
		float scale =  frame.cols/(float) DETECTION_WIDTH;
		if(frame.cols > DETECTION_WIDTH)
		{ // Shrink the image while keeping the same aspect ratio
			int scaledHeight = cvRound(frame.rows/scale);
			resize(frame, resizedFrame, Size(DETECTION_WIDTH, scaledHeight));
			frameInfo.setscale(scale);
		
		}
		else
		{ resizedFrame = frame;
		  frameInfo.setscale(1.0);
		  
		}
	}

	auto start = std::chrono::monotonic_clock::now();

	// multithreaded version - do 2 things in parallel here
	// 1. Perform face detection	
	std::future<vector<FaceDetectionDetails>> detectedFacesFuture;

	if( m_currentFrameNumber%m_faceDetectionFrequency == 0 )
	{
		try
		{
			// its a pity but we have to make use of the underlying Face Detector that is managed by the unique_ptr, but
			// std::async doesn't seem to like it any other way
			detectedFacesFuture = std::async(std::launch::async, &Analytics::FaceAnalyzer::FaceDetector_Neurotech::Detect, m_faceDetector.get(), resizedFrame,false);
		}
		catch(Exception e)
		{
			cout << "Exception: " << e.msg << endl;
		}
	}

	// 2. Pass the frame off to the tracking controller to update any active trackers
	std::future<void> trackingFuture = std::async(std::launch::async, &Analytics::FaceAnalyzer::TrackingController::Process, m_trackingController.get(), resizedFrame, frameTimestamp,frameInfo);

	// make sure the detector and the tracking controller have been called
	if( detectedFacesFuture.valid() )
		detectedFacesFuture.wait();
	trackingFuture.wait();

	vector<FaceDetectionDetails> detectedFaces;
	if( detectedFacesFuture.valid() )
	{
		try
		{
			detectedFaces = detectedFacesFuture.get();
		}
		catch(...)
		{
			cout << "Caught exception" << endl;
		}
	}

	/*	// Single threaded version
	vector<FaceDetectionDetails> detectedFaces;
	if( m_currentFrameNumber%m_faceDetectionFrequency == 0 )
	detectedFaces = m_faceDetector->Detect(resizedFrame);

	m_trackingController->Process(resizedFrame, frameTimestamp);
	*/
	auto end = std::chrono::monotonic_clock::now();

	auto diff = end - start;
	//std::cout << "Detection + tracking update took " << std::chrono::duration <double, std::milli> (diff).count() << " ms" << std::endl;

	// Now examine any detected faces to determine if they are new faces or ones that are already being tracked (the tracking controller can tell you if they are new or not)
	if( detectedFaces.size() > 0 )
	{
		//cout <<"Detected Faces: "<<detectedFaces.size()<<endl;

		// iterate over all the new detections for this frame
		for( auto startIter=detectedFaces.begin(); startIter!=detectedFaces.end(); ++startIter)
		{
			// determine if we have detected a new object to track or an existing one we are already tracking
			if( !m_trackingController->IsAlreadyBeingTracked((*startIter).faceRect) )
			{
				// we've found a new object, better start tracking it
				m_trackingController->AddNewTrack(resizedFrame, frameTimestamp, (*startIter).faceRect);
			}
			// else do nothing as we are already tracking this one
		}
	}

	// now see if we need to render any visualizations
	if( m_renderVisualization )
	{
		Mat frameCopy = resizedFrame.clone();

		m_trackingController->RenderVisualization(frameCopy);

		if( detectedFaces.size() > 0 )
			m_faceDetector->RenderVisualization(frameCopy, detectedFaces);

		namedWindow("Visualization");
		imshow("Visualization", frameCopy);
		waitKey(2);
	}

	m_currentFrameNumber++;
}

 void FaceAnalysis::GetOutput(Jzon::Object*& root)
{
    m_trackingController->GetOutput(root);
}

// end of namespaces
	}
}
