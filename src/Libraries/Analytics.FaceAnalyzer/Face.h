/*
	Represents a face being tracked and handles all functionality associated with a face

	Date: 21/07/2013

	Author: Jason Catchpole (jason@viblio.com)
*/

#ifndef __TRACKING_FACE__
#define __TRACKING_FACE__

#include "FaceAnalyzerConfiguration.h"

#include <opencv2/opencv.hpp>
#include <boost/uuid/uuid.hpp>

namespace Analytics
{
	namespace FaceAnalyzer
	{

// fwd declarations
class Tracker_OpenTLD;
class FaceDetector_OpenCV;
class Thumbnail;
class Face
{
private:
	// no copy constructor or assignment operator
	Face(const Face&);
	Face& operator=(const Face&);

	

	// a unique identifier for this face
	boost::uuids::uuid m_faceId;

	// indicates whether the face has been lost or not, likely due to them leaving the scene or because they
	// are simply out of view of the camera (occluded etc)
	bool m_isLost;

	bool has_thumbnails;

	// this bool is set to true when this face track was lost but is then found again, this can
	// happen with a discriminative tracker with the capability to automatically redetect & track
	// an object (face) even after it was lost
	bool m_wasLostIsNowFound;

	int m_frameProcessedNumber; // a count of the number of frames we have processed
	int m_lostFaceProcessingInterval;
	int Thumbnail_frequency;
	// indicates whether the face has had recognition applied to them yet
	bool m_hasBeenRecognized;

	// the most recent estimate for the face's position
	cv::Rect m_currentEstimatedPosition;

	// the amount of overlap between a potential detection of this face and the most recent estimated position of the face required for us to consider them one and then same
	float m_overlapThresholdForSameFace;

	// the minimum confidence level required before we consider the track lost
	float m_faceTrackerConfidenceThreshold;

	// stores a history of the frames location in recent frames (the number of frames in the past it will store is based on the face location history size parameter)
	uint32_t m_faceLocationHistorySize;
	uint32_t m_thumbnailConfidenceSize;
	int no_of_thumbnails;
	std::map<uint64_t, cv::Rect> m_faceLocationHistory;
	std::map<float, cv::Mat> m_thumbnailConfidence;

	// a vector of times when the face was visible in the scene. It is a vector of pairs of timestamps when the face entered and left (or was lost track of)
	std::vector<std::pair<uint64_t, uint64_t>> m_timesWhenFaceVisible;
	std::pair<uint64_t, uint64_t> m_currentFaceVisiblePair;
	uint64_t m_mostRecentFrameTimestamp;

	// the tracker that will be used to track this face
	std::unique_ptr<Tracker_OpenTLD> m_faceTracker;
	Thumbnail *Thumbnail_generator;
	std::string Thumbnail_path;

	cv::Scalar m_visualizationColor;

	// Private functions
	void MergeFaceVisibleTimes(std::vector<std::pair<uint64_t, uint64_t>> otherFaceTimesWhenFaceVisible);

	cv::Scalar RandomColor( cv::RNG& rng );
public:
   	Face(const cv::Mat frame, uint64_t frameTimestamp, cv::Rect initialFaceRegion,FaceAnalyzerConfiguration *faceAnalyzerConfig);
	~Face();

	// This face and 'theOtherFace' passed in are actually the same face. Take the useful
	// information from theOtherFace and combine it into this face
	void Merge(Face *theOtherFace);

	bool Process(const cv::Mat &frame, uint64_t frameTimestamp);

	// Returns the timestamp at which this face was first created
	uint64_t Age();

	// indicates if the face was found again having being lost in previous frames
	bool WasLostNowFound(){ return m_wasLostIsNowFound; }

	// Determines whether the rectangle passed in is in the same location as the estimated location
	// of this face
	bool IsSameFace(const cv::Rect &otherFaceLocation);

	cv::Rect GetMostRecentFacePosition(){ return m_currentEstimatedPosition; }

	std::string GetOutput();

	void RenderVisualization(cv::Mat &frame);

	boost::uuids::uuid GetFaceId(){ return m_faceId; }
};

// end of namespaces
	}
}

#endif