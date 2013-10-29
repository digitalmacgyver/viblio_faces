/*
	Handles the task of configuring the pipeline based on the configuration requirements for this job, and then taking each frame from the video source and 
	passing them through each of the analyzers

	Date: 03/07/2013

	Author: Jason Catchpole
*/

#include "VideoProcessor.h"
#include "JobConfiguration.h"
#include "VideoSource/FileVideoSource.h"
#include "Analytics.FaceAnalyzer/FaceAnalyzer.h"
#include "FileSystem/FileSystem.h"
#include "Jzon/Jzon.h"
#include "Analytics.FaceAnalyzer/FaceAnalyzerConfiguration.h"
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

#include <opencv2/opencv.hpp>
#include <string>
#include <iomanip>

using namespace std;
using namespace cv;

VideoProcessor::VideoProcessor(const JobConfiguration &jobConfig)
{
	// go through all the config, ensure sane values, ensure input paths/files exist, ensure output paths exist (and are writable).
	// If there are any problems log it and then throw an exception

	// setup the video source based on the type of the source specified by the job
	try
	{
		m_videoSource = new VideoSource::FileVideoSource(jobConfig.videoSourceFilename);
	}
	catch(Exception e)
	{
		cout << "Exception caught while attempting to create the FileVideoSource. Message = " << e.msg << ". Cannot continue" << endl;
		throw e;
	}

	

	// pull info out of the job config as to all the Analyzers they want to use in this job and then call SetupAnalyzers to actually initialize/configure each of them
	SetupAnalyzers(jobConfig);
}

VideoProcessor::~VideoProcessor()
{
	if( m_videoSource )
	{
		delete m_videoSource;
		m_videoSource = NULL;
	}

	// go through each of the analyzers and deallocate them
}

bool VideoProcessor::SetupAnalyzers(const JobConfiguration &jobConfig)
{
	// for each analyzer that we implement we should log the configuration options used to set the analyzer up (each AnalyzerConfig should
	// implement the ToString() method

	if( jobConfig.isFaceAnalyzerEnabled )
	{
		if( jobConfig.faceAnalyzerConfig == NULL )
			cout << "Error - Face Analyzer is enabled but configuration information is not present" << endl;
		else
			m_analyzers.push_back(new Analytics::FaceAnalyzer::FaceAnalysis(jobConfig.faceAnalyzerConfig));
	}

	return true;
}

bool VideoProcessor::PerformProcessing()
{
	m_startProcessingTime = chrono::monotonic_clock::now();

	// go through each frame from the video source and pass it along to each of the analyzers
	Mat currentFrame;
	uint64_t timestamp;
	int TotalFrames = m_videoSource->NumberFrames();
	int CurrentFrameNumber;
	while(true)
	{
		currentFrame = m_videoSource->GetNextFrame();

		timestamp = m_videoSource->GetTimestamp();
		CurrentFrameNumber=m_videoSource->CurrentFrameNo();
		//cout<<"Processing Frame no : " <<CurrentFrameNumber << " of " << TotalFrames << " Total " <<endl;
		// Printing the current Timestamp
		//cout<< "Time stamp : " << timestamp<<endl;
		if( currentFrame.empty() )
			break;

		// pass to each of the analyzers - could be performed in parallel assuming each analyzer is independent (which it should be)
		for(vector<Analytics::Analyzer*>::iterator startIter=m_analyzers.begin(); startIter != m_analyzers.end(); ++startIter)
			(*startIter)->Process(currentFrame,timestamp);
	 
		cout << std::setprecision(2) << std::fixed << (CurrentFrameNumber / (float) TotalFrames) * 100 << "% done       \r";
	}

	m_lastFrameTimestamp = timestamp;
	m_endProcessingTime = chrono::monotonic_clock::now();

	return true;
}

void VideoProcessor::OutputJobSummaryStatistics()
{
	using namespace chrono;

	std::chrono::duration<double> elapsed_time = m_endProcessingTime - m_startProcessingTime;
	cout << "########### SUMMARY JOB STATISTICS ###########" << endl;

	cout << "Total time to process: " << (duration <double, std::milli> (elapsed_time).count())/1000 << " seconds" << endl;
	//duration <double, std::milli> (elapsed_time).count()
	//hours h = duration_cast<hours>(elapsed_time);
    //elapsed_time -= h;
    //minutes m = duration_cast<minutes>(elapsed_time);
    //elapsed_time -= m;
    //seconds s = duration_cast<seconds>(elapsed_time);
    //elapsed_time -= s;
    //std::cout << h.count() << ':' << m.count() << ':' << s.count() << endl;

	cout << "Video length: " << m_lastFrameTimestamp / 1000 << " seconds" << endl;

	cout << "Percentage of realtime: " << (duration <double, std::milli> (elapsed_time).count() / (double)m_lastFrameTimestamp) * 100 << "%" << endl;
}

bool VideoProcessor::DumpOutput(const JobConfiguration &jobConfig)
{
	/*
	{
		"video_file_processed": "C:\temp\example_video_file.avi",
		.
		.
		.

		- Video file that was processed
		- An array of Analyzer information
- For each analyzer, output the type of analyzer it is
- If its a face analyzer
-     Thumbnail path that the user passed in
-     An array of faces
-        Each face will store the number of thumbnails generated
-        Each face will store the UUID
-        Each face will also store the visibility information
-        Each face will store the face rectangle information 
	*/

	string outputJsonData = "";

		//Jzon::Array listOfStuff3;
	Jzon::Object tempNode;
	for(auto startIter=m_analyzers.begin(); startIter != m_analyzers.end(); ++startIter)
	{
		//outputJsonData += (*startIter)->GetOutput();

		string temp;
		temp = (*startIter)->GetOutput();
		 
         Jzon::Parser parser(tempNode,temp );
		 if (!parser.Parse())
		    {
               std::cout << "Error: " << parser.GetError() << std::endl;
		     }
		// tempNode.Add("Analyzer",startIter-m_analyzers.begin());
				//face_detector_neuro->Detect(k);
				//listOfStuff3.Add(tempNode);

	}


	//Jzon::Object root2;
   // root2.Add("AnalyzerInfo",listOfStuff3);

	Jzon::Writer writer(tempNode, Jzon::StandardFormat);
     writer.Write();
		// Writing everything ot a string to export
     outputJsonData = writer.GetResult();

	Jzon::Object rootNode;
        Jzon::Parser parser(rootNode,outputJsonData);
		if (!parser.Parse())
        {
                std::cout << "Error: " << parser.GetError() << std::endl;
        }
		rootNode.Add("user_uuid","user_uuid");
		rootNode.Add("media_uuid",jobConfig.faceAnalyzerConfig->filenameprefix);
		//rootNode.Add("protocol_version","protocol_version");

		boost::uuids::uuid uuid = boost::uuids::random_generator()();
		std::stringstream ss;
		ss << uuid;
		string temppath;
		temppath = jobConfig.faceAnalyzerConfig->faceThumbnailOutputPath +"/"+jobConfig.faceAnalyzerConfig->filenameprefix+".json";
		cout << temppath;
		Jzon::FileWriter::WriteFile(temppath, rootNode, Jzon::StandardFormat);

	// now open a file (perhaps using the unique Job ID as the file name, with a .json extension) and dump the data to it

	return true;
}
