/*
	Manages all the trackers that are currently active. When the process function is called
	it updates all the trackers so they can update their position. Also allows the caller to
	determine whether a recently detected face (or object) is already being tracked or not

	Date: 07/07/2013

	Author: Jason Catchpole (jason@viblio.com)
*/

#include "TrackingController.h"
#include "Tracker_OpenTLD.h"
#include "Face.h"
#include "Jzon/Jzon.h"
#include <boost/log/trivial.hpp>

#include <thread>
#include <future>

using namespace cv;
using namespace std;

namespace Analytics
{
	namespace FaceAnalyzer
	{

TrackingController::TrackingController(FaceAnalyzerConfiguration *faceAnalyzerConfiguration)
{
	faceAnalyzerConfig = faceAnalyzerConfiguration;
	m_trackCountSoFar = 0;
}

TrackingController::~TrackingController()
{
	// go through each of the trackers and destroy them
	for(auto startIter=m_trackedFaces.begin(); startIter != m_trackedFaces.end(); ++startIter)
		delete *startIter;

	m_trackedFaces.clear();

}

/*
	Determines whether the potentially new object/face whose position is specified in newObjectLocation
	is a new object or one that is already being tracked
*/
bool TrackingController::IsAlreadyBeingTracked(const Rect &newObjectLocation)
{
	if( m_trackedFaces.size() == 0 )
		return false;

	//if( m_trackedFaces.size() == 1 )
	//	return true; // for the moment we will force only a single face to be tracked

	// not yet implemented. Currently always assumes there should only be 1 track thus if we are tracking
	// something then the new object is simply this same object that we are tracking already.
	//		Eventually this will go through each of the trackers in m_trackers and determine if the newObjectLocation
	//		is the same as any of the given trackers (by examining the overlap of the trackers estimated location
	//		and the new objects location

	// perform this process in parallel... compare the newObjectLocation with each of the face's estimated position
	bool isAlreadyBeingTracked = false;
	for( auto startIter=m_trackedFaces.begin(); startIter!=m_trackedFaces.end(); ++startIter)
	{
		bool isCurrentFaceTheSame = (*startIter)->IsSameFace( newObjectLocation );
		if( isCurrentFaceTheSame )
		{
			// as soon as we find a face that says it is the same then we have our answer

			isAlreadyBeingTracked = true;
			break;
		}
	}

	return isAlreadyBeingTracked;
}

/*
	Adds a new tracker which is initialised using the provided object/face location that was detected
	in the provided frame
*/
void TrackingController::AddNewTrack(const Mat &frame, uint64_t frameTimestamp, Rect &objectLocation)
{
	// It is possible that the new track is one we had previously, we should examine this
	// possibility here depending on the tracker and how discriminative it is (can it tell
	// that this new object is the one it was tracking previously). Alternatively, we could
	// use the face recognition module to help us determine this (assuming we had enough images
	// of the person we were tracking previously to be able to recognize them again in this

	// 'new' detection). Lastly we could simply consider every person new and try and consolidate
	// the people we are detected and tracked at the end of the processing (the problem with this
	// approach is we might only have a few thumbnails of each person to work with. Probably best
	// to do both, do it now (while tracking) while we have the maximum amount of source material 
	// to try and consolidate them, and again later just to be doubly sure

	// for the moment we'll just add a new tracker for this person
	//Tracker_OpenTLD *newTracker = new Tracker_OpenTLD();

	//m_backgroundLearningTLD->InitialiseTrack(frame, objectLocation);

	if(faceAnalyzerConfig != NULL && m_trackedFaces.size() >= faceAnalyzerConfig->maximumNumberActiveTrackers)
	{
		// we aren't supposed to add any more as we have reached the maximum number of trackers we are supposed to have active.
		// Start by trying to remove any tracks that are already discarded
		RemoveDiscardedTracks();

		if(m_trackedFaces.size() >= faceAnalyzerConfig->maximumNumberActiveTrackers)
		{
			// still no space, lets remove the lost track that has been lost for the longest time

			int longestLostDuration = -1;
			auto longestLostTrackIter = m_trackedFaces.begin();

			auto startIterator=m_trackedFaces.begin();
			for(  ;startIterator!=m_trackedFaces.end(); startIterator++)
			{

				if((*startIterator)->IsLost() && (*startIterator)->TrackLostDuration() > longestLostDuration )
				{   
					longestLostDuration = (*startIterator)->TrackLostDuration();
					longestLostTrackIter = startIterator;
				}
			}

			if(longestLostDuration > -1)
			{
				// yay we found one to delete
				// Before we delete it we need to save the information from this track (the JSON data and the thumbnails etc)
				Jzon::Object *root1 = new Jzon::Object;
				(*longestLostTrackIter)->GetOutput(root1);

				if((root1->GetCount())>0)
					discardedFacesJason.Add(*root1);
				delete root1;
				root1 = NULL;
				delete *longestLostTrackIter;
				m_trackedFaces.erase(longestLostTrackIter);
			}
			else
				// we didn't find a lost track we could delete, we have no choice but to return without
				// adding the new track, we can't exceed our maximum
				return;
		}

		// all good, now we have some space
	}

	m_trackedFaces.push_back( new Face(frame, frameTimestamp, objectLocation, faceAnalyzerConfig, m_trackCountSoFar) );
	m_trackCountSoFar++;

	cout << "Now tracking " << m_trackedFaces.size() << " faces" << endl;
}

/*
	Updates each of the trackers that are active so that they can estimate the new position of
	the object/face they are tracking
*/
void TrackingController::Process( uint64_t frameTimestamp, Frame &frame)
{
	// could call each of these trackers in parallel assuming they are independent (they should 
	// be at least when they are estimating the new position of the object/face)
	//vector<future<bool>> futures; // multithreading
	for( auto startIter=m_trackedFaces.begin(); startIter!=m_trackedFaces.end(); ++startIter)
	{
		// multithreading
		//futures.push_back(async(std::launch::async, &Analytics::FaceAnalyzer::Face::Process, (*startIter), frame, frameTimestamp));

		// single threaded
		(*startIter)->Process( frameTimestamp,frame);
	}

	// Deleting tracks that are initialized but disappear immediately that stay active and use memory
	auto startIterator=m_trackedFaces.begin();
	for(  ;startIterator!=m_trackedFaces.end(); )
	{

		if((*startIterator)->get_last_thumbnail_time()==0)
		{   
			delete *startIterator;
			startIterator = m_trackedFaces.erase(startIterator);
		}
		else
		{
			++startIterator;
		}

	}
	

	// Moving any discarded faces to new vector
	RemoveDiscardedTracks();

	//cout << "Total tracks yet " << m_trackedFaces.size()+m_trackCountSoFar <<endl;

	// multithreaded
	//for(auto &e : futures) 
	//{
	//	e.wait();
	//	//std::cout << e.get() << std::endl;
	//}

	// check to make sure we don't have any face tracks that are actually of the same person. This can
	// happen. If we start tracking person A and then we lose track of them (if they leave the scene) and
	// then they come back in and we redetect using the face detector, setup a new tracker and start tracking
	// only to later find that the original face tracker for A picks their track up again then we will have
	// two trackers tracking the same person... we must detect this and consolidate them
	int duplicateFaceIndex1 = 0;
	int duplicateFaceIndex2 = 0;
	while( DuplicateFacesDetected(duplicateFaceIndex1, duplicateFaceIndex2) )
	{
		// we may have to resolve more than 1 duplicate face in one iteration of the loop, hence the while loop

		ResolveDuplicates(duplicateFaceIndex1, duplicateFaceIndex2);
	}
}

/* 
	Check to see if there are any face tracks that are actually of the same person
*/
bool TrackingController::DuplicateFacesDetected(int &redetectedFaceTrackerIndex, int &duplicateFaceIndex)
{
	redetectedFaceTrackerIndex = 0;
	duplicateFaceIndex = 0;

	if( m_trackedFaces.size() <= 1 )
		return false; // we certainly can't have duplicate faces if we have 0 or 1 trackers

	// now go through and check to see if any of the trackers were once lost and now are found. If they
	// are we need to double check to ensure we don't have duplicate trackers
	//		- if we do find duplicates we need to determine the older of the two and then call Merge on that one and pass it the newer one... we can then destroy the newer one
	bool redetectedFacePresent = false;
	for( auto startIter=m_trackedFaces.begin(); startIter!=m_trackedFaces.end(); ++startIter)
	{
		bool currentFaceWasLostNowFound = (*startIter)->WasLostNowFound();
		if( currentFaceWasLostNowFound )
		{
			redetectedFacePresent = true; // we will have to do extra work to ensure this redetected face isn't now a duplicate
			break;
		}

		redetectedFaceTrackerIndex++;
	}

	// we don't even have any faces which have recently been refound after being lost so we don't have
	// a chance of any duplicates
	if( !redetectedFacePresent )
		return false;

	// we have a track that was lost but is not found, lets make sure its not a duplicate
	bool duplicateFound = false;
	for(duplicateFaceIndex=0; duplicateFaceIndex<m_trackedFaces.size(); duplicateFaceIndex++)
	{
		if( duplicateFaceIndex != redetectedFaceTrackerIndex )
		{
			if( m_trackedFaces[redetectedFaceTrackerIndex]->IsSameFace( m_trackedFaces[duplicateFaceIndex]->GetMostRecentFacePosition() ) )
			{
				duplicateFound = true;
				break;
			}
		}
	}

	return duplicateFound;
}

void TrackingController::ResolveDuplicates(int duplicateFaceIndex1, int duplicateFaceIndex2)
{
	BOOST_LOG_TRIVIAL(info) << "About to resolve duplicates, number faces " << m_trackedFaces.size();
	// we have found a face which is a duplicate, lets determine which of the two faces is the older
	uint64_t age1 = m_trackedFaces[duplicateFaceIndex1]->Age();
	uint64_t age2 = m_trackedFaces[duplicateFaceIndex2]->Age();

	if( age1 < age2 )
	{
		m_trackedFaces[duplicateFaceIndex1]->Merge( m_trackedFaces[duplicateFaceIndex2] );
		m_trackedFaces.erase(m_trackedFaces.begin()+duplicateFaceIndex2); // erase the newer one with index duplicateFaceIndex2
	}
	else
	{
		m_trackedFaces[duplicateFaceIndex2]->Merge( m_trackedFaces[duplicateFaceIndex1] );
		m_trackedFaces.erase(m_trackedFaces.begin()+duplicateFaceIndex1); // erase the newer one with index duplicateFaceIndex1
	}
	BOOST_LOG_TRIVIAL(info) << "Finished resolving duplicates, number faces " << m_trackedFaces.size();
}

void TrackingController::RemoveDiscardedTracks()
{
	auto startIter=m_trackedFaces.begin();
	for(  ;startIter!=m_trackedFaces.end(); )
	{
		if((*startIter)->DiscardStatus())
		{   
			Jzon::Object *root1 = new Jzon::Object;
			(*startIter)->GetOutput(root1);
			
			if((root1->GetCount())>0)
				discardedFacesJason.Add(*root1);
			delete root1;
			root1 = NULL;
			delete *startIter;
			startIter = m_trackedFaces.erase(startIter);
		}
		else
		{
			++startIter;
		}
	}
}

void TrackingController::GetOutput(Jzon::Object*& root)
{

	string facesArrayJson = "";
	//Jzon::Array listOfStuff2;

	//Appending the discardedFaces track
	//m_trackedFaces.insert(m_trackedFaces.end(),discardedFaces.begin(),discardedFaces.end());

	auto startIter=m_trackedFaces.begin();
	while( startIter!=m_trackedFaces.end() )
	{
		Jzon::Object *root = new Jzon::Object;
		(*startIter)->GetOutput(root);
		if((root->GetCount())>0)
			discardedFacesJason.Add(*root);
		
		delete root;
		root = NULL;
		delete *startIter;
		startIter = m_trackedFaces.erase(startIter);
	}

	// clean up

	
	root->Add("tracks",discardedFacesJason);
	return;
}

void TrackingController::RenderVisualization(Mat &frame)
{
	for( auto startIter=m_trackedFaces.begin(); startIter!=m_trackedFaces.end(); ++startIter)
	{
		(*startIter)->RenderVisualization( frame );
	}
}

// end of namespaces
	}
}