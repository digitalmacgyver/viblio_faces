#include "Face.h"
#include "Tracker_OpenTLD.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

using namespace cv;
using namespace std;

namespace Analytics
{
	namespace FaceAnalyzer
	{


Face::Face(Tracker_OpenTLD *m_trackerToInitializeFrom, const Mat frame, uint64_t frameTimestamp, Rect initialFaceRegion)
	// generate a random UUID for the unique face identifier
	: m_faceId(boost::uuids::random_generator()()),
	  m_isLost(false),
	  m_factLocationHistorySize(60) // currently hardcoded, should be passed as an input parameter in the future
{
	// in a real system we will probably take a copy of the tracker to initialize the face from as it has learned the background,
	// however this is yet TBD
	m_faceTracker = m_trackerToInitializeFrom;

	m_faceTracker->InitialiseTrack(frame, initialFaceRegion);

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
	Rect estimatedPosition = m_faceTracker->Process(frame);

	if( m_isLost && m_faceTracker->GetConfidence() > 0.0f )
	{
		// the face was lost but not anymore
		m_isLost = false;

		// this is the start of a new time measurement pair
		m_currentFaceVisiblePair.first = frameTimestamp;
	}
	else if( !m_isLost && m_faceTracker->GetConfidence() == 0.0f )
	{
		// the face wasn't lost but it is now

		m_isLost = true;

		// this is the end of the time measurement pair
		m_currentFaceVisiblePair.second = frameTimestamp;

		// add it to the list
		m_timesWhenFaceVisible.push_back(m_currentFaceVisiblePair);
	}
	else if( m_isLost && m_faceTracker->GetConfidence() == 0.0f )
	{
		// face was lost and still is, nothing to do here
	}
	else // !m_isLost && m_faceTracker->GetConfidence() > 0.0f
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

		if( m_faceLocationHistory.size() >= m_factLocationHistorySize )
			// make some space in the history map by removing the oldest item
			m_faceLocationHistory.erase( m_faceLocationHistory.begin() );
		m_faceLocationHistory.insert (m_faceLocationHistory.end(), pair<uint64_t,Rect>(frameTimestamp,estimatedPosition));
	}

	m_mostRecentFrameTimestamp = frameTimestamp;

	return true;
}

// end of namespaces
	}
}