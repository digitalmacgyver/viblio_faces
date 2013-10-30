#include "Face.h"
#include "Tracker_OpenTLD.h"
#include "Thumbnail.h"
#include "FaceAnalyzerConfiguration.h"
#include "Jzon/Jzon.h"
#include "FileSystem/FileSystem.h"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

using namespace cv;
using namespace std;

namespace Analytics
{
	namespace FaceAnalyzer
	{


Face::Face(const Mat frame, uint64_t frameTimestamp, Rect initialFaceRegion,FaceAnalyzerConfiguration *faceAnalyzerConfig)
	// generate a random UUID for the unique face identifier
	: m_faceId(boost::uuids::random_generator()()),
	  m_isLost(false),
	  m_wasLostIsNowFound(false),
	  m_faceLocationHistorySize(1296000), // we theoretically store the face location history information for every frame we have tracked, in practice we have an upper bounds to prevent the size of this map going silly. Max size for video @30fps for 12 hrs ~ 40MB of RAM
	  m_overlapThresholdForSameFace(0.5f),
	  m_faceTrackerConfidenceThreshold(0.5f),
	  m_thumbnailConfidenceSize(5),
	  no_of_thumbnails(0),
	  has_thumbnails(false),
	  m_frameProcessedNumber(0)
{
	// in a real system we will probably take a copy of the tracker to initialize the face from as it has learned the background,
	// however this is yet TBD
	m_faceTracker.reset(new Tracker_OpenTLD());//m_trackerToInitializeFrom;
	face_detector_neuro.reset( new FaceDetector_Neurotech());
	m_startTime = chrono::monotonic_clock::now();

    Thumbnail_frequency = faceAnalyzerConfig->Thumbnail_generation_frequency;
	m_faceTracker->InitialiseTrack(frame, initialFaceRegion);
	
	Thumbnail_path = faceAnalyzerConfig->faceThumbnailOutputPath;
	Filenameprefix = faceAnalyzerConfig->filenameprefix;
	Thumbnail_generator = new Thumbnail(faceAnalyzerConfig);

	m_lostFaceProcessingInterval = faceAnalyzerConfig->lostFaceProcessFrequency;

	// this is the start of a new time measurement pair
	m_currentFaceVisiblePair.first = frameTimestamp;
	m_currentFaceVisiblePair.second = 0;

	m_visualizationColor = Scalar(0.0f, 0.0f, 0.0f, 0.0f);

	std::cout << "Tracking person with ID " << to_string(m_faceId) << std::endl;
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

// This face and 'theOtherFace' passed in are actually the same face. Take the useful
// information from theOtherFace and combine it into this face
void Face::Merge(Face *theOtherFace)
{
	std::cout << "Merging person with ID " << to_string(theOtherFace->m_faceId) << " into person with ID " << to_string(m_faceId) << std::endl;

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

bool Face::Process(const Mat &frame, uint64_t frameTimestamp)
{
	m_wasLostIsNowFound = false;

	m_frameProcessedNumber++;
	if( m_isLost && m_frameProcessedNumber % m_lostFaceProcessingInterval != 0)
		return true; // all good, we are just going to skip processing of this frame because this face is lost and we don't process every frame for lost faces otherwise it slows us down

	m_endTime = chrono::monotonic_clock::now();

	std::chrono::duration<double> elapsed_time = m_endTime - m_startTime;

	double temp =(std::chrono::duration <double, std::milli> (elapsed_time).count())/1000;

	//if (temp>20.0)
		//return true;


	m_currentEstimatedPosition = m_faceTracker->Process(frame);
		
	std::ostringstream oss;
	std::string imagepath;

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

			//if((frameTimestamp-m_currentFaceVisiblePair.first)%800 ==0)
				
			if(no_of_thumbnails%Thumbnail_frequency==0)
			{
				float confidence = 0.0f;
		  		Mat thumbnail_temp = Thumbnail_generator->ExtractThumbnail(frame.clone(), m_currentEstimatedPosition, confidence);
				confidence =Thumbnail_generator->GetConfidencevalue(thumbnail_temp,has_thumbnails,m_faceTracker->GetConfidence());
				
			       if(has_thumbnails)
				   {
					if( m_thumbnailConfidence.size() == m_thumbnailConfidenceSize && (confidence>m_thumbnailConfidence.begin()->first))
					{
						m_thumbnailConfidence.erase( m_thumbnailConfidence.begin() );
						m_thumbnailConfidence.insert (m_thumbnailConfidence.end(), pair<float,Mat>(confidence,thumbnail_temp));
					}
					else if (m_thumbnailConfidence.size() != m_thumbnailConfidenceSize)
						m_thumbnailConfidence.insert (m_thumbnailConfidence.end(), pair<float,Mat>(confidence,thumbnail_temp));
				
				   }
			}

				   no_of_thumbnails = no_of_thumbnails+1;


				
		}
		if( m_faceLocationHistory.size() >= m_faceLocationHistorySize )
			// make some space in the history map by removing the oldest item
			m_faceLocationHistory.erase( m_faceLocationHistory.begin() );
		m_faceLocationHistory.insert (m_faceLocationHistory.end(), pair<uint64_t,Rect>(frameTimestamp,m_currentEstimatedPosition));
	}

	m_mostRecentFrameTimestamp = frameTimestamp;

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
	float percentageOverlap = intersectionRect.area() / (float)m_currentEstimatedPosition.area();

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

string Face::GetOutput(int trackno)
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
	std::stringstream ss;
	ss << m_faceId;
	// Store the top five thumbnails
		string imagepath;
		int count=0;
		cout << " No of faces : " << m_thumbnailConfidence.size();
		if(m_thumbnailConfidence.size()>0)
		{
		std::string path =Thumbnail_path+"/"+ss.str();
				//	FileSystem::CreateDirectory(path);
		}
		else
			return result;
	//	std::string all_thumbnails;
		  Jzon::Array listOfStuff;
	for(std::map<float,Mat>::iterator iter = m_thumbnailConfidence.begin(); iter != m_thumbnailConfidence.end(); ++iter)
					{
						count++;
						Mat k =  iter->second;
					std::stringstream oss;
					std::stringstream tracknumber;
				//oss << frameTimestamp;
					oss << count-1;
					tracknumber << trackno;
					
			   imagepath =Thumbnail_path+"/"+Filenameprefix+"_face_"+tracknumber.str()+"_"+oss.str()+".jpg";
				string pass = Filenameprefix +"/"+ Filenameprefix+"_face_"+tracknumber.str()+"_"+oss.str()+".jpg";
				string temp;
				temp = face_detector_neuro->Detect_return_json(k,pass,count-1);
				cout << temp;
				 Jzon::Object tempNode;
                 Jzon::Parser parser(tempNode, temp);
				  if (!parser.Parse())
				{
                std::cout << "Error: " << parser.GetError() << std::endl;
				 }
				//face_detector_neuro->Detect(k);
				  if(!temp.empty())
				  {
					listOfStuff.Add(tempNode);
					imwrite( imagepath,k);
				  }
			//	all_thumbnails = all_thumbnails + temp ;
				//cout << all_thumbnails;
					}


    // Defining a jason object and adding some attributes and their values

	 Jzon::Object root;
       // root.Add("UUID", ss.str());
	    root.Add("track_id", trackno);
		root.Add("faces",listOfStuff);
      //  root.Add("number_of_thumbnails", count);
		
	

   // Adding visibility information as an array under visibilty info attribute
	 Jzon::Array visibilty_info;
	 std::vector<std::pair<uint64_t, uint64_t>>::iterator iter = m_timesWhenFaceVisible.begin();
	 for (iter=m_timesWhenFaceVisible.begin();iter!=m_timesWhenFaceVisible.end();++iter)
	 {
		Jzon::Object each_visibility;
       each_visibility.Add("start_frame",int(iter->first));
	   each_visibility.Add("end_frame",int(iter->second));
	   visibilty_info.Add(each_visibility);
		// visibilty_info.Add("start_frame",iter->first);
	 }
        root.Add("visiblity_info", visibilty_info);

		/*
		// Adding face rectangle information as an array of arrays under "face_rectangles" attribute
		 std::map<uint64_t, cv::Rect>::iterator it = m_faceLocationHistory.begin();

		 Jzon::Array rect_info;

		for ( it=m_faceLocationHistory.begin(); it!=m_faceLocationHistory.end(); ++it)
		{
		Jzon::Array each_rect;
			each_rect.Add(it->second.x);
			each_rect.Add(it->second.y);
			each_rect.Add(it->second.width);
			each_rect.Add(it->second.height);
			each_rect.Add(int(it->first));
			rect_info.Add(each_rect);
		}
		root.Add("face_rectangles",rect_info);

		*/
		Jzon::Writer writer(root, Jzon::StandardFormat);
        writer.Write();
		// Writing everything ot a string to export
        result = writer.GetResult();


	return result;
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
