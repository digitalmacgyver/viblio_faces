#include "Face.h"
#include "Tracker_OpenTLD.h"
#include "FaceAnalyzerConfiguration.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

using namespace cv;
using namespace std;

namespace Analytics
{
	namespace FaceAnalyzer
	{


Face::Face(Tracker_OpenTLD *m_trackerToInitializeFrom, const Mat frame, uint64_t frameTimestamp, Rect initialFaceRegion,FaceAnalyzerConfiguration *faceAnalyzerConfig)
	// generate a random UUID for the unique face identifier
	: m_faceId(boost::uuids::random_generator()()),
	  m_isLost(false),
	  m_faceLocationHistorySize(60), // currently hardcoded, should be passed as an input parameter in the future
	  m_overlapThresholdForSameFace(0.5f),
	  m_faceTrackerConfidenceThreshold(0.5f)
{
	// in a real system we will probably take a copy of the tracker to initialize the face from as it has learned the background,
	// however this is yet TBD
	m_faceTracker = m_trackerToInitializeFrom;

	m_faceTracker->InitialiseTrack(frame, initialFaceRegion);
	Thumbnail_path = faceAnalyzerConfig->faceThumbnailOutputPath;

	// this is the start of a new time measurement pair
	m_currentFaceVisiblePair.first = frameTimestamp;
}


Face::~Face()
{
	if( !m_isLost )
	{
		// before we finished we were tracking a face so close off our last measurement using the last seen frame timestamp
		m_currentFaceVisiblePair.second = m_mostRecentFrameTimestamp;
		m_timesWhenFaceVisible.push_back(m_currentFaceVisiblePair);
	}
}

bool Face::Process(const Mat &frame, uint64_t frameTimestamp)
{
	m_currentEstimatedPosition = m_faceTracker->Process(frame);
	
	std::ostringstream oss;
	std::string imagepath;

	if( m_isLost && m_faceTracker->GetConfidence() >= m_faceTrackerConfidenceThreshold )
	{
		// the face was lost but not anymore
		m_isLost = false;

		// this is the start of a new time measurement pair
		m_currentFaceVisiblePair.first = frameTimestamp;
		cout << " First visible at : " << m_currentFaceVisiblePair.first << endl;
	}
	else if( !m_isLost && m_faceTracker->GetConfidence() < m_faceTrackerConfidenceThreshold )
	{
		// the face wasn't lost but it is now

		m_isLost = true;

		// this is the end of the time measurement pair
		m_currentFaceVisiblePair.second = frameTimestamp;
		cout << " Last visible at : " << m_currentFaceVisiblePair.second << endl;
		// add it to the list
		m_timesWhenFaceVisible.push_back(m_currentFaceVisiblePair);
	}
	else if( m_isLost && m_faceTracker->GetConfidence() < m_faceTrackerConfidenceThreshold )
	{
		// face was lost and still is, nothing to do here
	}
	else // !m_isLost && m_faceTracker->GetConfidence() >= m_faceTrackerConfidenceThreshold
	{
		// hopefully the typical case, the track wasn't lost and still isn't, we are still tracking the person

		// nothing really to do here
	}
	

	if( !m_isLost )
	{
		// if the face isn't lost then we can do several things here
		// 1) Determine if we need to produce a thumbnail
		// 2) Determine if we need to apply recognition to the face
		// 3) Store the face's location in the location history map

		// Saving a frame for every 800 milliseconds for a tracked frame when Thumbnail path is provided
		if( !Thumbnail_path.empty() )
		{
			if((frameTimestamp-m_currentFaceVisiblePair.first)%800 ==0)
				{
				oss << frameTimestamp;
				imagepath =Thumbnail_path+ "/image"+oss.str()+".png";
				imwrite( imagepath,frame(m_currentEstimatedPosition));
				cout << " Tracking for every 800 milli sec. Frame saved at : "<<  (frameTimestamp-m_currentFaceVisiblePair.first)<<  endl;
			    }
		}
		if( m_faceLocationHistory.size() >= m_faceLocationHistorySize )
			// make some space in the history map by removing the oldest item
			m_faceLocationHistory.erase( m_faceLocationHistory.begin() );
		m_faceLocationHistory.insert (m_faceLocationHistory.end(), pair<uint64_t,Rect>(frameTimestamp,m_currentEstimatedPosition));
	}

	m_mostRecentFrameTimestamp = frameTimestamp;

	return true;
}

/*
*	Determines if the rectangle passed in as 'otherFaceLocation' overlaps enough with the current face's estimated position
*	to be considered the same face
*/
bool Face::IsSameFace(const Rect &otherFaceLocation)
{
	Rect intersectionRect = m_currentEstimatedPosition & otherFaceLocation;

	// determine the percentage overlap between this face and the otherface
	float percentageOverlap = intersectionRect.area() / (float)m_currentEstimatedPosition.area();

	return percentageOverlap > m_overlapThresholdForSameFace;
}

string Face::GetOutput()
{
	/*
		- Each face will store the number of thumbnails generated
		- Each face will store the UUID
		- Each face will also store the visibility information
		- Each face will store the face rectangle information 
	*/

	return "";
}

// end of namespaces
	}
}