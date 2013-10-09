/*
	Handles the task of configuring the pipeline based on the configuration requirements for this job, and then taking each frame from the video source and 
	passing them through each of the analyzers

	Date: 03/07/2013

	Author: Jason Catchpole (jason@viblio.com)
*/

#ifndef __VIDEO_PROCESSOR__
#define __VIDEO_PROCESSOR__

#include <vector>

#include "JobConfiguration.h"
#include "Analytics.FaceAnalyzer/Analyzer.h"
#include "VideoSource/VideoSource.h"
#include <chrono>

class VideoProcessor
{
private:
	// no copy constructor or assignment operator
	VideoProcessor(const VideoProcessor&);
	VideoProcessor& operator=(const VideoProcessor&);

	// the vector of analyzers that will be used for this video processing job
	std::vector<Analytics::Analyzer*> m_analyzers;

	// the video source that will be supplying us with frames for analysis
	VideoSource::VideoSourceBase *m_videoSource;

	// the time at which we started processing the video
	std::chrono::system_clock::time_point m_startProcessingTime;

	// the time at which we finished processing the video
	std::chrono::system_clock::time_point m_endProcessingTime;

	// the timestamp of the last frame in the video
	uint64_t m_lastFrameTimestamp;

	
	bool SetupAnalyzers(const JobConfiguration &jobConfig);

public:
	VideoProcessor(const JobConfiguration &jobConfig);
	~VideoProcessor();
	
	// Do the main work of the job. Takes in each frame, decodes it and then passes it along for analysis and then encoding
	bool PerformProcessing();

	void OutputJobSummaryStatistics();

	// Dump the output data for this job
	bool DumpOutput(const JobConfiguration &jobConfig);
};

#endif