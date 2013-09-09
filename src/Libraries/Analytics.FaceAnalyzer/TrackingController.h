/*
	Manages all the trackers that are currently active. When the process function is called
	it updates all the trackers so they can update their position. Also allows the caller to
	determine whether a recently detected face (or object) is already being tracked or not

	Date: 07/07/2013

	Author: Jason Catchpole (jason@viblio.com)
*/

#ifndef __TRACKING_CONTROLLER__
#define __TRACKING_CONTROLLER__

#include <opencv2/opencv.hpp>
#include "FaceAnalyzerConfiguration.h"
#include <stdint.h>

namespace Analytics
{
	namespace FaceAnalyzer
	{

// fwd declarations
class Tracker_OpenTLD;
class Face;

class TrackingController
{
private:
	std::vector<Face*> m_trackedFaces;

	// no copy constructor or assignment operator
	TrackingController(const TrackingController&);
	TrackingController& operator=(const TrackingController&);

	FaceAnalyzerConfiguration *faceAnalyzerConfig;

	// check to see if there are any face tracks that are actually of the same person
	bool DuplicateFacesDetected(int &redetectedFaceTrackerIndex, int &duplicateFaceIndex);
	void ResolveDuplicates(int duplicateFaceIndex1, int duplicateFaceIndex2);
public:
	TrackingController(FaceAnalyzerConfiguration *faceAnalyzerConfiguration);
	~TrackingController();

	bool IsAlreadyBeingTracked(const cv::Rect &newObjectLocation);

	void AddNewTrack(const cv::Mat &frame, uint64_t frameTimestamp, cv::Rect &objectLocation);

	void Process(const cv::Mat &frame, uint64_t frameTimestamp);

	std::string GetOutput();

	void RenderVisualization(cv::Mat &frame);
};

// end of namespaces
	}
}

#endif