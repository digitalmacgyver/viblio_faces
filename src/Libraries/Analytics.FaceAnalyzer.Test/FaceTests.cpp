#include "gtest/gtest.h"
#include "Analytics.FaceAnalyzer/Face.h"
#include "Analytics.FaceAnalyzer/FaceAnalyzerConfiguration.h"
#include "opencv2/opencv.hpp"
#include "FaceTests.h"

using namespace Analytics::FaceAnalyzer;
using namespace std;

namespace Analytics
{
	namespace FaceAnalyzer
	{

// a simple case where 1 tracker tracks the face and then loses it, then
// another tracker finds the same face. A short time later the 1st track
// reacquires the track at which point the two trackers are merged
void FaceTests::TestMergeFunctionalitySimple()
{
	FaceAnalyzerConfiguration faceAnalyzerConfig;
	faceAnalyzerConfig.faceDetectorCascadeFile = "";
	faceAnalyzerConfig.eyeDetectorCascadeFile = "";

	cv::Mat frame(480, 640, CV_8UC3);
	// setup each of the two faces

	pair<uint64_t, uint64_t> currentTimes;

	// the first is the older one
	Face tracker1(frame, 0, cv::Rect(20, 20, 40, 40), &faceAnalyzerConfig);

	// tracker 1 tracks Person A for period [5000, 10000]
	currentTimes.first = 5000;
	currentTimes.second = 10000;
	tracker1.m_timesWhenFaceVisible.push_back(currentTimes);
	
	tracker1.m_currentFaceVisiblePair.first = 15000;
	tracker1.m_currentFaceVisiblePair.second = 0;
	tracker1.m_mostRecentFrameTimestamp = 17000;

	Face tracker2(frame, 0, cv::Rect(20, 20, 40, 40), &faceAnalyzerConfig);
	tracker2.m_currentFaceVisiblePair.first = 17000;
	tracker2.m_currentFaceVisiblePair.second = 0;
	tracker2.m_mostRecentFrameTimestamp = 17000; // at time 17 seconds tracker 1 reacquires the face, this is when we detect we have to merge
	
	// we have just detected that tracker2 & tracker1 are tracking the same face! time to merge
	tracker1.Merge(&tracker2);

	// ensure we have 1 pair of times and that the state of the tracker 1 is such
	// that it is tracking the face and would store the correct time pair if it lost
	// the face again
	EXPECT_EQ(1, tracker1.m_timesWhenFaceVisible.size());

	// ensure the times are actually correct
	EXPECT_EQ(5000, tracker1.m_timesWhenFaceVisible[0].first);
	EXPECT_EQ(10000, tracker1.m_timesWhenFaceVisible[0].second);
	EXPECT_EQ(15000, tracker1.m_currentFaceVisiblePair.first);
	EXPECT_EQ(0, tracker1.m_currentFaceVisiblePair.second);
}

void FaceTests::TestMergeFunctionalityLessSimple()
{
	FaceAnalyzerConfiguration faceAnalyzerConfig;
	faceAnalyzerConfig.faceDetectorCascadeFile = "";
	faceAnalyzerConfig.eyeDetectorCascadeFile = "";

	cv::Mat frame(480, 640, CV_8UC3);
	// setup each of the two faces

	pair<uint64_t, uint64_t> currentTimes;

	// the first is the older one
	Face tracker1(frame, 0, cv::Rect(20, 20, 40, 40), &faceAnalyzerConfig);

	// tracker 1 tracks Person A for period [5000, 10000]
	currentTimes.first = 5000;
	currentTimes.second = 10000;
	tracker1.m_timesWhenFaceVisible.push_back(currentTimes);
	
	Face tracker2(frame, 0, cv::Rect(20, 20, 40, 40), &faceAnalyzerConfig);
	tracker2.m_currentFaceVisiblePair.first = 15000;
	tracker2.m_currentFaceVisiblePair.second = 0;
	tracker2.m_mostRecentFrameTimestamp = 17000; // at time 17 seconds tracker 1 reacquires the face, this is when we detect we have to merge

	tracker1.m_currentFaceVisiblePair.first = 17000;
	tracker1.m_currentFaceVisiblePair.second = 0;
	tracker2.m_mostRecentFrameTimestamp = 17000;
	
	// we have just detected that tracker2 & tracker1 are tracking the same face! time to merge
	tracker1.Merge(&tracker2);

	// ensure we have 1 pair of times and that the state of the tracker 1 is such
	// that it is tracking the face and would store the correct time pair if it lost
	// the face again
	EXPECT_EQ(1, tracker1.m_timesWhenFaceVisible.size());

	// ensure the times are actually correct
	EXPECT_EQ(5000, tracker1.m_timesWhenFaceVisible[0].first);
	EXPECT_EQ(10000, tracker1.m_timesWhenFaceVisible[0].second);
	EXPECT_EQ(15000, tracker1.m_currentFaceVisiblePair.first);
	EXPECT_EQ(0, tracker1.m_currentFaceVisiblePair.second);
}

// more complicated than the simple case above this
// time tracker 1 tracks and loses the face 3 times before tracker 2
// tracks and loses it twice, then tracker 1 goes round another 2 times
// before tracker 2 and 1 tracker it at the same time at which point 
// they merge. In this example the times are interleaved
void FaceTests::TestMergeFunctionalityMultipleInterleaved()
{
	FaceAnalyzerConfiguration faceAnalyzerConfig;
	faceAnalyzerConfig.faceDetectorCascadeFile = "";
	faceAnalyzerConfig.eyeDetectorCascadeFile = "";

	cv::Mat frame(480, 640, CV_8UC3);
	// setup each of the two faces

	pair<uint64_t, uint64_t> currentTimes;

	// the first is the older one
	Face tracker1(frame, 0, cv::Rect(20, 20, 40, 40), &faceAnalyzerConfig);

	// tracker 1 tracks Person A for period [5000, 10000]
	currentTimes.first = 5000;
	currentTimes.second = 10000;
	tracker1.m_timesWhenFaceVisible.push_back(currentTimes);

	// then for period [12000, 15000]
	currentTimes.first = 12000;
	currentTimes.second = 15000;
	tracker1.m_timesWhenFaceVisible.push_back(currentTimes);

	// then for period [17000, 20000]
	currentTimes.first = 17000;
	currentTimes.second = 20000;
	tracker1.m_timesWhenFaceVisible.push_back(currentTimes);
	
	// now tracker 2 joins the action
	Face tracker2(frame, 0, cv::Rect(20, 20, 40, 40), &faceAnalyzerConfig);
	currentTimes.first = 22000;
	currentTimes.second = 25000;
	tracker2.m_timesWhenFaceVisible.push_back(currentTimes);

	currentTimes.first = 27000;
	currentTimes.second = 30000;
	tracker2.m_timesWhenFaceVisible.push_back(currentTimes);

	// then for period [32000, 35000]
	currentTimes.first = 32000;
	currentTimes.second = 35000;
	tracker1.m_timesWhenFaceVisible.push_back(currentTimes);

	// now tracker 1 finds it and begins tracking
	tracker1.m_currentFaceVisiblePair.first = 37000;
	tracker1.m_currentFaceVisiblePair.second = 0;

	// and then at 39 seconds tracker 2 also finds it
	tracker1.m_mostRecentFrameTimestamp = 39000;
	tracker2.m_mostRecentFrameTimestamp = 39000;

	tracker2.m_currentFaceVisiblePair.first = 39000;
	tracker2.m_currentFaceVisiblePair.second = 0;
	
	// we have just detected that tracker2 & tracker1 are tracking the same face! time to merge
	tracker1.Merge(&tracker2);

	// ensure we have 2 pairs of times
	EXPECT_EQ(6, tracker1.m_timesWhenFaceVisible.size());

	// ensure the times are actually correct
	EXPECT_EQ(5000, tracker1.m_timesWhenFaceVisible[0].first);
	EXPECT_EQ(10000, tracker1.m_timesWhenFaceVisible[0].second);
	EXPECT_EQ(12000, tracker1.m_timesWhenFaceVisible[1].first);
	EXPECT_EQ(15000, tracker1.m_timesWhenFaceVisible[1].second);
	EXPECT_EQ(17000, tracker1.m_timesWhenFaceVisible[2].first);
	EXPECT_EQ(20000, tracker1.m_timesWhenFaceVisible[2].second);
	EXPECT_EQ(22000, tracker1.m_timesWhenFaceVisible[3].first);
	EXPECT_EQ(25000, tracker1.m_timesWhenFaceVisible[3].second);
	EXPECT_EQ(27000, tracker1.m_timesWhenFaceVisible[4].first);
	EXPECT_EQ(30000, tracker1.m_timesWhenFaceVisible[4].second);
	EXPECT_EQ(32000, tracker1.m_timesWhenFaceVisible[5].first);
	EXPECT_EQ(35000, tracker1.m_timesWhenFaceVisible[5].second);
	EXPECT_EQ(37000, tracker1.m_currentFaceVisiblePair.first);
	EXPECT_EQ(0, tracker1.m_currentFaceVisiblePair.second);
}

void FaceTests::TestMergeFunctionalitySimplethumbnails()
{
	FaceAnalyzerConfiguration faceAnalyzerConfig;
	faceAnalyzerConfig.faceDetectorCascadeFile = "";
	faceAnalyzerConfig.eyeDetectorCascadeFile = "";

	cv::Mat frame(480, 640, CV_8UC3);
	// setup each of the two faces

	pair<uint64_t, uint64_t> currentTimes;

	// the first is the older one
	Face tracker1(frame, 0, cv::Rect(20, 20, 40, 40), &faceAnalyzerConfig);

	// tracker 1 tracks Person A for period [5000, 10000]
	currentTimes.first = 5000;
	currentTimes.second = 10000;
	tracker1.m_timesWhenFaceVisible.push_back(currentTimes);
	
	tracker1.m_currentFaceVisiblePair.first = 15000;
	tracker1.m_currentFaceVisiblePair.second = 0;
	tracker1.m_mostRecentFrameTimestamp = 17000;
	cv::Mat a(10,10,CV_16U);
	tracker1.m_thumbnailConfidence.insert(tracker1.m_thumbnailConfidence.end(), pair<float,cv::Mat>(0.50,a));
	tracker1.m_thumbnailConfidence.insert(tracker1.m_thumbnailConfidence.end(), pair<float,cv::Mat>(0.25,a));
	tracker1.m_thumbnailConfidence.insert(tracker1.m_thumbnailConfidence.end(), pair<float,cv::Mat>(0.70,a));
	tracker1.m_thumbnailConfidence.insert(tracker1.m_thumbnailConfidence.end(), pair<float,cv::Mat>(0.65,a));


	Face tracker2(frame, 0, cv::Rect(20, 20, 40, 40), &faceAnalyzerConfig);
	tracker2.m_currentFaceVisiblePair.first = 17000;
	tracker2.m_currentFaceVisiblePair.second = 0;
	tracker2.m_mostRecentFrameTimestamp = 17000; // at time 17 seconds tracker 1 reacquires the face, this is when we detect we have to merge
	
	cv::Mat b(10,10,CV_16U);
	
	tracker2.m_thumbnailConfidence.insert(tracker2.m_thumbnailConfidence.end(), pair<float,cv::Mat>(0.8,a));
	tracker2.m_thumbnailConfidence.insert(tracker2.m_thumbnailConfidence.end(), pair<float,cv::Mat>(0.9,a));
	tracker2.m_thumbnailConfidence.insert(tracker2.m_thumbnailConfidence.end(), pair<float,cv::Mat>(0.2,a));
	

	// we have just detected that tracker2 & tracker1 are tracking the same face! time to merge
	tracker1.Merge(&tracker2);

	// ensure we have 1 pair of times and that the state of the tracker 1 is such
	// that it is tracking the face and would store the correct time pair if it lost
	// the face again
	EXPECT_EQ(1, tracker1.m_timesWhenFaceVisible.size());

	// ensure the times are actually correct
	EXPECT_EQ(5000, tracker1.m_timesWhenFaceVisible[0].first);
	EXPECT_EQ(10000, tracker1.m_timesWhenFaceVisible[0].second);
	EXPECT_EQ(15000, tracker1.m_currentFaceVisiblePair.first);
	EXPECT_EQ(0, tracker1.m_currentFaceVisiblePair.second);

	EXPECT_EQ(5, tracker1.m_thumbnailConfidence.size());
	EXPECT_EQ(0.5,tracker1.m_thumbnailConfidence.begin()->first);
	std::map<float,cv::Mat>::iterator it = tracker1.m_thumbnailConfidence.begin();
	std::advance(it,tracker1.m_thumbnailConfidence.size()-1);
	EXPECT_FLOAT_EQ(0.9,it->first);
	
	
}


TEST_F(FaceTests, TestMergeFunctionalitySimple) 
{
	TestMergeFunctionalitySimple();
}

TEST_F(FaceTests, TestMergeFunctionalityLessSimple) 
{
	TestMergeFunctionalityLessSimple();
}

TEST_F(FaceTests, TestMergeFunctionalityMultipleInterleaved) 
{
	TestMergeFunctionalityMultipleInterleaved();
}

TEST_F(FaceTests, TestMergeFunctionalitySimplethumbnails) 
{
	TestMergeFunctionalitySimplethumbnails();
}

	}
}