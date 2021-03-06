#include "Face.h"
#include "Tracker_OpenTLD.h"
#include "Thumbnail.h"
#include "ThumbnailDetails.h"
#include "FaceAnalyzerConfiguration.h"
#include "Jzon/Jzon.h"
#include "FileSystem/FileSystem.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <boost/log/trivial.hpp>

using namespace cv;
using namespace std;

namespace Analytics
{
	namespace FaceAnalyzer
	{


Face::Face(const Mat frame, uint64_t frameTimestamp, Rect initialFaceRegion,FaceAnalyzerConfiguration *faceAnalyzerConfig, int trackNumber)
	// generate a random UUID for the unique face identifier
	: m_faceId(boost::uuids::random_generator()()),
	  m_isLost(false),
	  m_wasLostIsNowFound(false),
	  m_faceLocationHistorySize(1296000), // we theoretically store the face location history information for every frame we have tracked, in practice we have an upper bounds to prevent the size of this map going silly. Max size for video @30fps for 12 hrs ~ 40MB of RAM
	  m_overlapThresholdForSameFace(0.34f),
	  m_faceTrackerConfidenceThreshold(0.5f),
	  m_thumbnailConfidenceSize(5),
	  m_frameProcessedNumber(0),
	  move_to_discarded(false),
	  m_faceNumber(trackNumber)
{
	// not really using the FaceID attribute of the logs as it needs to be class specific, not a global one
	//boost::log::core::get()->add_global_attribute("FaceID", boost::log::attributes::make_constant(to_string(m_faceId)));
	
	BOOST_LOG_TRIVIAL(info) << "Tracking person with ID " << to_string(m_faceId);
	// in a real system we will probably take a copy of the tracker to initialize the face from as it has learned the background,
	// however this is yet TBD
	m_faceTracker.reset(new Tracker_OpenTLD());//m_trackerToInitializeFrom;
	m_firstFrameTrackingSuccess = false; // we start off with this false, as long as the tracker succeeds in tracking the face at least in the 1st frame then this will be set to true
	last_thumbnail_time = 0;
	m_lostThumbnailTimestamp = 0;
    Thumbnail_frequency = faceAnalyzerConfig->Thumbnail_generation_frequency;
	discard_frequency = faceAnalyzerConfig->discarded_tracker_frequency;
	m_faceTracker->InitialiseTrack(frame, initialFaceRegion);
	
	Thumbnail_path = faceAnalyzerConfig->faceThumbnailOutputPath;
	Filenameprefix = faceAnalyzerConfig->filenameprefix;
	Thumbnail_generator.reset(new Thumbnail(faceAnalyzerConfig));

	m_lostFaceProcessingInterval = faceAnalyzerConfig->lostFaceProcessFrequency;

	// this is the start of a new time measurement pair
	m_currentFaceVisiblePair.first = frameTimestamp;
	m_currentFaceVisiblePair.second = 0;

	m_visualizationColor = Scalar(0.0f, 0.0f, 0.0f, 0.0f);
}


Face::~Face()
{
	if( !m_isLost )
	{
		// before we finished we were tracking a face so close off our last measurement using the last seen frame timestamp
		m_currentFaceVisiblePair.second = m_mostRecentFrameTimestamp;
		m_timesWhenFaceVisible.push_back(m_currentFaceVisiblePair);
	}

	BOOST_LOG_TRIVIAL(info) << "FaceID: " << to_string(m_faceId) << ". Being destroyed";
}

void Face::FreeResources()
{
	m_faceTracker.reset();
	
	Thumbnail_generator.reset();
}

// This face and 'theOtherFace' passed in are actually the same face. Take the useful
// information from theOtherFace and combine it into this face
void Face::Merge(Face *theOtherFace)
{
	BOOST_LOG_TRIVIAL(info) << "FaceID: " << to_string(m_faceId) << ".Merging person with ID " << to_string(theOtherFace->m_faceId) << " into person with ID " << to_string(m_faceId);

	// assumes that *this is the face to keep (the original) and 'theOtherFace' is the duplicate

	// both of these tracks were active so both had a m_currentFaceVisiblePair that was under construction... merge these two here
	m_currentFaceVisiblePair.first = std::min(m_currentFaceVisiblePair.first, theOtherFace->m_currentFaceVisiblePair.first); // we always take the older of the pair

	// copy any face visibility information out of 'theOtherFace' and put it into the correct position in
	// this->m_timesWhenFaceVisible... ensure if theOtherFace had info in m_currentFaceVisiblePair we get that too
	MergeFaceVisibleTimes(theOtherFace->m_timesWhenFaceVisible);

	// get the face location history out of theOtherFace and put it into the this->m_faceLocationHistory
	m_faceLocationHistory.insert(theOtherFace->m_faceLocationHistory.begin(),theOtherFace->m_faceLocationHistory.end());

	// look at the thumbnails that we have in theOtherFace and determine if any of them have higher scores
	// than the lowest scored images from this->thumbnails and if so replace them
	//std::map<float, cv::Mat> m_thumbnailConfidence_temp;
	// Merging both and cutting of at size specified in the constructor
	m_thumbnailConfidence.insert(theOtherFace->m_thumbnailConfidence.begin(),theOtherFace->m_thumbnailConfidence.end());
	if(m_thumbnailConfidence.size() >=m_thumbnailConfidenceSize)
	{
		while(m_thumbnailConfidence.size()>m_thumbnailConfidenceSize)
		{
			m_thumbnailConfidence.erase( m_thumbnailConfidence.begin());
		}

	}
	


	// copy the current estimated position from theOtherFace and update this->m_currentEstimatedPosition
	m_currentEstimatedPosition = theOtherFace->m_currentEstimatedPosition;

	// destroy this->m_faceTracker and take theOtherFace->m_faceTracker and use that instead as it is more up to date...
	// would be ideal here if we could exploit the current tracks slightly outdated knowledge as to the model of the tracked
	// subject, not sure we can shoe horn this in
	m_faceTracker = std::move(theOtherFace->m_faceTracker);
	theOtherFace->m_faceTracker = NULL; // make sure when we destroy the other face we don't kill the tracker as well because now we will be using it
}

bool Face::Process( uint64_t frameTimestamp, Frame &frame)
{
	
	m_wasLostIsNowFound = false;
	
	m_frameProcessedNumber++;
	if( m_isLost && m_frameProcessedNumber % m_lostFaceProcessingInterval != 0)
		return true; // all good, we are just going to skip processing of this frame because this face is lost and we don't process every frame for lost faces otherwise it slows us down

	if( move_to_discarded )
		return true; // this face was moved into the discarded list, this is permanent

	// perform the tracking on the latest frame
	m_currentEstimatedPosition = m_faceTracker->Process(frame.GetScaledMat());
		
	std::ostringstream oss;
	std::string imagepath;

	if(m_isLost && (frameTimestamp - m_lostThumbnailTimestamp) > discard_frequency && (m_lostThumbnailTimestamp != 0)) // 15 seconds
	{
		FreeResources();
		move_to_discarded = true;
		return true;
	}

	if( m_isLost && m_faceTracker->GetConfidence() >= m_faceTrackerConfidenceThreshold )
	{
		// the face was lost but not anymore
		m_isLost = false;

		// indicate to interested bystanders that this face was lost but we have just found it again
		m_wasLostIsNowFound = true;

		// this is the start of a new time measurement pair
		m_currentFaceVisiblePair.first = frameTimestamp;
		m_currentFaceVisiblePair.second = 0; // reset the 2nd element to 0
		//cout << " Face visible at : " << m_currentFaceVisiblePair.first << endl;
	}
	else if( !m_isLost && m_faceTracker->GetConfidence() < m_faceTrackerConfidenceThreshold )
	{
		// the face wasn't lost but it is now

		m_isLost = true;
		m_lostThumbnailTimestamp = frameTimestamp;

		// this is the end of the time measurement pair
		m_currentFaceVisiblePair.second = frameTimestamp;
		//cout << " Last lost at : " << m_currentFaceVisiblePair.second << endl;
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
			// if this is the first frame we have of the face OR we enough time has passed that we need to capture
			// another thumbnail then attempt to extract one
			if(last_thumbnail_time == 0 || (frameTimestamp - last_thumbnail_time) > Thumbnail_frequency )//&& (frameTimestamp - last_thumbnail_time) > 0)
			{
				float confidence = 0.0f;
				ThumbnailDetails thumbnail_detail;
				//thumbnail_detail.reset(new ThumbnailDetails());
				bool extractionSuccess = Thumbnail_generator->ExtractThumbnail( m_currentEstimatedPosition, confidence, thumbnail_detail,frame);
				
				//confidence =Thumbnail_generator->GetConfidencevalue(thumbnail_temp,has_thumbnails,m_faceTracker->GetConfidence());
				if(confidence > 0.0)
				{
					if( m_thumbnailConfidence.size() == m_thumbnailConfidenceSize && (confidence>m_thumbnailConfidence.begin()->first))
					{
						m_thumbnailConfidence.erase( m_thumbnailConfidence.begin() );
						m_thumbnailConfidence.insert (m_thumbnailConfidence.end(), pair<float,ThumbnailDetails>(confidence,thumbnail_detail));
					}
					else if (m_thumbnailConfidence.size() != m_thumbnailConfidenceSize)
					{
						m_thumbnailConfidence.insert (m_thumbnailConfidence.end(), pair<float,ThumbnailDetails>(confidence,thumbnail_detail));
					}
					//DEBUG - keep trying till we find a usable thumbnail.
					last_thumbnail_time=frameTimestamp;
				} else if ( last_thumbnail_time == 0 ) {
				  // Always add a thumbnail, but keep looking frame by frame for one with a higher confidence.
				  thumbnail_detail.confidence = 0.01f;
				  m_thumbnailConfidence.insert(m_thumbnailConfidence.end(), pair<float,ThumbnailDetails>(0.01f,thumbnail_detail));
				}
				
				//if ( last_thumbnail_time > 0 ) {
				  // If we didn't find a thumbnail, keep looking each frame.
				  //
				  // If we already have a thumbnail,
				  // only check in occasionally to
				  // find more for performance
				  // reasons.
				//  last_thumbnail_time=frameTimestamp;
				//}
				
			}
		}
		if( m_faceLocationHistory.size() >= m_faceLocationHistorySize )
			// make some space in the history map by removing the oldest item
			m_faceLocationHistory.erase( m_faceLocationHistory.begin() );
		m_faceLocationHistory.insert (m_faceLocationHistory.end(), pair<uint64_t,Rect>(frameTimestamp,m_currentEstimatedPosition));
	}

	m_mostRecentFrameTimestamp = frameTimestamp;

	if( !m_isLost )
		// assuming the face isn't lost on the very first frame we try and track the face this will be set to true. Its important
		// to know situations when we detect a face, attempt to track it and then the tracker fails to even track it in the very first
		// frame (maybe slightly off the edge of the image), it means it won't be a good frame to track
		m_firstFrameTrackingSuccess = true; // if we reach here then obviously

	return true;
}

// Returns the timestamp at which this face was first created
uint64_t Face::Age()
{
	if( m_timesWhenFaceVisible.size() == 0 )
		return m_currentFaceVisiblePair.first; // this means the current face has been tracked and has never been lost (so not in the times visible map)

	return (*m_timesWhenFaceVisible.begin()).first;
}

/*
*	Determines if the rectangle passed in as 'otherFaceLocation' overlaps enough with the current face's estimated position
*	to be considered the same face
*/
bool Face::IsSameFace(const Rect &otherFaceLocation)
{
	Rect intersectionRect = m_currentEstimatedPosition & otherFaceLocation;

	// determine the percentage overlap between this face and the otherface
	float percentageOverlap1 = intersectionRect.area() / (float)m_currentEstimatedPosition.area();
	float percentageOverlap2 = intersectionRect.area() / (float)otherFaceLocation.area();
	float percentageOverlap = percentageOverlap1>percentageOverlap2?percentageOverlap1:percentageOverlap2;

	return percentageOverlap > m_overlapThresholdForSameFace;
}

// Merges the pairs of times from the other face into our own visibility pair list.
// This process can be fairly complicated as it is possible for many situations to come about:
// 1) Easy (and perhaps most common scenario)
//			- We add a track with ID 1 for Person A at time 5 seconds
//			- Track 1 loses track of Person A at time 10 seconds. We add a pair with [5,10] into the visibility list
//			- We detect Person A again and add a new track with ID 2 at time 15 seconds
//			- At 20 seconds the tracker with ID 1 regains track of Person A, now trackers 1 & 2 are tracking Person A
//			- We detect the duplicate tracks and merge the information from track 2 into track 1. 
//			- At 20 seconds the video finishes
//			- The final visibility information includes [5,10], and [15,20]
// 2) More complicated - interleaved times
//			- Tracker 1 tracks Person A from times [5,10] then loses it
//			- Tracker 2 tracks Person A from times [15,20] then loses it
//			- Tracker 1 tracks Person A from times [25,-] 
//			- Tracker 2 tracks Person A from times [27,35], here we detect the duplicate with Tracker 1
//			- Final visibility information is [5,10], [15,20], [25,35]
//			- Important to note we must combine the visiblity info from the last pair and use the earliest start time of the two, and the latest end time
// 3) Overlapping times (example above shows a small example of this)
// 4) Another complicated case
//			- Tracker 1 & 2 are tracking Person A
//			- Tracker 1 tracks between times [5,15]
//			- Tracker 2 tracks between times [20,25]
//			- Tracker 1 tracks between times [30,40]
//			- Tracker 2 tracks between times [35,-]
//			- In this situation we must note that the older tracker has a m_currentFaceVisiblePair that must be continued to use as its start is before the start of the newer one
//			- There are other permutations like this where we must reuse the current face visible pair (the start value) and continue to use this pair
void Face::MergeFaceVisibleTimes(vector<pair<uint64_t, uint64_t>> otherFaceTimesWhenFaceVisible)
{
	// iterate over the other face visible times and determine where in our face visible vector we
	// should put each entry
	auto otherFaceVisibleStartIter = otherFaceTimesWhenFaceVisible.begin();
	auto otherFaceVisibleEndIter = otherFaceTimesWhenFaceVisible.end();

	for(; otherFaceVisibleStartIter!=otherFaceVisibleEndIter; otherFaceVisibleStartIter++)
	{
		// compare the current visibility pair to our list and try and find where we should insert this pair

		// we will iterate backwards over our visibility info under the knowledge that the other face is newer in terms
		// of when it was first constructed (as we always merge the newer face into the older one)
		int fromEndCounter = 0;
		auto ourFaceVisibleReverseStartIter = m_timesWhenFaceVisible.rbegin();
		auto ourFaceVisibleReverseEndIter = m_timesWhenFaceVisible.rend();
		for(; ourFaceVisibleReverseStartIter != ourFaceVisibleReverseEndIter; ourFaceVisibleReverseStartIter++)
		{
			// check to see if this pair from the other face started after the end of the first (in reverse) from our face
			if( (*otherFaceVisibleStartIter).first >= (*ourFaceVisibleReverseStartIter).second )
			{
				// if it did then we can insert this pair from the other face into our list after our current iterator
				m_timesWhenFaceVisible.insert(m_timesWhenFaceVisible.end()-fromEndCounter, (*otherFaceVisibleStartIter));
				break; // break out of this inner loop and look at the next entry from the other face
			}

			fromEndCounter++;
		}
	}
}

void Face::GetOutput(Jzon::Object*& root)
{
	std::string result;


	// before we dump the data make sure we don't have another visibility pair to dump
	if( !m_isLost )
	{
		// before we finished we were tracking a face so close off our last measurement using the last seen frame timestamp
		m_currentFaceVisiblePair.second = m_mostRecentFrameTimestamp;
		m_timesWhenFaceVisible.push_back(m_currentFaceVisiblePair);
	}
	/*
	- Each face will store the number of thumbnails generated
	- Each face will store the UUID
	- Each face will also store the visibility information
	- Each face will store the face rectangle information 
	*/
	// Getting UUid to string
	//Jzon::Object root;
	std::stringstream ss;
	ss << m_faceId;
	// Store the top five thumbnails
	string imagepath;
	int count=0;
	//cout << " No of faces : " << m_thumbnailConfidence.size();
	if(m_thumbnailConfidence.size()>0)
	{
		std::string path =Thumbnail_path+"/"+ss.str();
		//	FileSystem::CreateDirectory(path);
	}
	else
		return ;
	//	std::string all_thumbnails;
	Jzon::Array listOfStuff;
	for(std::map<float,ThumbnailDetails>::iterator iter = m_thumbnailConfidence.begin(); iter != m_thumbnailConfidence.end(); ++iter)
	{
		count++;
		Mat k =  iter->second.GetThumbnail();
		std::stringstream oss;
		std::stringstream tracknumber;
		//oss << frameTimestamp;
		oss << count-1;
		tracknumber << m_faceNumber;

		imagepath =Thumbnail_path+"/"+Filenameprefix+"_face_"+tracknumber.str()+"_"+oss.str()+".jpg";
		string pass = Filenameprefix +"/"+ Filenameprefix+"_face_"+tracknumber.str()+"_"+oss.str()+".jpg";

		// Try PNGs for better quality.
		//imagepath =Thumbnail_path+"/"+Filenameprefix+"_face_"+tracknumber.str()+"_"+oss.str()+".png";
		//string pass = Filenameprefix +"/"+ Filenameprefix+"_face_"+tracknumber.str()+"_"+oss.str()+".png";
		string temp;

		// Adding data and details .....
		Jzon::Object root1;
		root1.Add("face_id",count-1);
		root1.Add("s3_bucket","s3_bucket");
		root1.Add("s3_key",pass);
		root1.Add("md5sum","md5sum");
		root1.Add("face_confidence",iter->second.GetDetailedInformation().faceDetectionConfidence);
		root1.Add("face_rotation_pitch",iter->second.GetDetailedInformation().pitch);
		root1.Add("face_rotation_roll",iter->second.GetDetailedInformation().roll);
		root1.Add("face_rotation_yaw",iter->second.GetDetailedInformation().yaw);
		root1.Add("width",iter->second.GetDetailedInformation().faceRect.width);
		root1.Add("height",iter->second.GetDetailedInformation().faceRect.height);
		root1.Add("backgroundUniformity",iter->second.backgroundUniformity);
		root1.Add("sharpness",iter->second.sharpness);
		root1.Add("totalConfidence",iter->second.confidence);
		root1.Add("grayscaleDensity",iter->second.grayscaleDensity);
		root1.Add("hasAdditionalFaceInformation",iter->second.GetDetailedInformation().hasAdditionalFaceInformation);
		Jzon::Array left_eye;
		left_eye.Add(int(iter->second.GetDetailedInformation().leftEye.x));left_eye.Add(int(iter->second.GetDetailedInformation().leftEye.x));
		root1.Add("leftEyeCenter",left_eye);
		root1.Add("leftEyeConfidence",iter->second.GetDetailedInformation().leftEyeConfidence);
		Jzon::Array right_eye;
		right_eye.Add(int(iter->second.GetDetailedInformation().rightEye.x));right_eye.Add(int(iter->second.GetDetailedInformation().rightEye.y));
		root1.Add("rightEyeCenter",right_eye);
		root1.Add("rightEyeConfidence",iter->second.GetDetailedInformation().rightEyeConfidence);
		root1.Add("interEyeDistance",iter->second.GetDetailedInformation().intereyeDistance);
		root1.Add("blinking",iter->second.GetDetailedInformation().blinking);
		root1.Add("blinkingConfidence",iter->second.GetDetailedInformation().blinkingConfidence);
		Jzon::Array NoseLocation;
		NoseLocation.Add(int(iter->second.GetDetailedInformation().noseLocation.x));NoseLocation.Add(int(iter->second.GetDetailedInformation().noseLocation.y));
		root1.Add("noseLocation",NoseLocation);
		root1.Add("noseLocationConfidence",iter->second.GetDetailedInformation().noseLocationConfidence);
		Jzon::Array MouthLocation;
		MouthLocation.Add(int(iter->second.GetDetailedInformation().mouthLocation.x));MouthLocation.Add(int(iter->second.GetDetailedInformation().mouthLocation.y));
		root1.Add("mouthLocation",MouthLocation);
		root1.Add("mouthLocationConfidence",iter->second.GetDetailedInformation().mouthLocationConfidence);
		root1.Add("mouthOpen",iter->second.GetDetailedInformation().mouthOpen);
		root1.Add("mouthOpenConfidence",iter->second.GetDetailedInformation().mouthOpenConfidence);
		root1.Add("isMale",iter->second.GetDetailedInformation().isMale);
		root1.Add("genderConfidence",iter->second.GetDetailedInformation().genderConfidence);
		root1.Add("expression",iter->second.GetDetailedInformation().GetExpression());
		root1.Add("expressionConfidence",iter->second.GetDetailedInformation().expressionConfidence);
		root1.Add("wearingGlasses",iter->second.GetDetailedInformation().wearingGlasses);
		root1.Add("glassesConfidence",iter->second.GetDetailedInformation().glassesConfidence);
		root1.Add("wearingDarkGlasses",iter->second.GetDetailedInformation().wearingDarkGlasses);
		root1.Add("wearingDarkGlassesConfidence",iter->second.GetDetailedInformation().wearingDarkGlassesConfidence);

		listOfStuff.Add(root1);
		
		// Try various quality parameters.
		//imwrite( imagepath, k, { CV_IMWRITE_PNG_COMPRESSION, 9 } );
		imwrite( imagepath, k );

	}


	// Defining a jason object and adding some attributes and their values


	// root.Add("UUID", ss.str());
	root->Add("track_id", m_faceNumber);
	root->Add("faces",listOfStuff);

	// Adding visibility information as an array under visibilty info attribute
	Jzon::Array visibilty_info;
	std::vector<std::pair<uint64_t, uint64_t>>::iterator iter = m_timesWhenFaceVisible.begin();
	for (iter=m_timesWhenFaceVisible.begin();iter!=m_timesWhenFaceVisible.end();++iter)
	{
		Jzon::Object each_visibility;
		each_visibility.Add("start_frame",int(iter->first));
		each_visibility.Add("end_frame",int(iter->second));
		visibilty_info.Add(each_visibility);

	}
	root->Add("visiblity_info", visibilty_info);

	return ;
}

void Face::RenderVisualization(Mat &frame)
{
	if( !m_isLost )
	{
		if( m_visualizationColor == Scalar(0.0f, 0.0f, 0.0f, 0.0f) )
		{
			// generate a random color for this track
			RNG rng( getTickCount() );
			rng.next();
			m_visualizationColor = RandomColor( rng );
		}

		// if we are actually tracking the face at present then render its location
		rectangle(frame, m_currentEstimatedPosition, m_visualizationColor, 8, 8, 0);
	}
}

// Produces a random color given a random object
Scalar Face::RandomColor( RNG& rng )
{
  int icolor = (unsigned) rng;
  return Scalar( icolor&255, (icolor>>8)&255, (icolor>>16)&255 );
}

// end of namespaces
	}
}
