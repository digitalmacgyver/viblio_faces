/*
	The entry point into the application. Parses command line options (or config files) and sets up the program
	flow based on said options

	Date: 01/07/2013

	Author: Jason Catchpole (jason@viblio.com)

*/

#include "boost/program_options.hpp"

#include <iostream>
#include <list>
#include <vector>
#include <stdio.h>
#include <string>

#include "FileSystem/FileSystem.h"
#include "VideoSource/FileVideoSource.h"
#include "Analytics.FaceAnalyzer/AnalyzerTypes.h"
#include "Analytics.FaceAnalyzer/FaceDetector.h"
#include "JobConfiguration.h"
#include "Analytics.FaceAnalyzer/FaceAnalyzerConfiguration.h"
#include "VideoProcessor.h"

#include <fstream>
#include <boost/make_shared.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/formatters/named_scope.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/utility/empty_deleter.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/support/exception.hpp>

#ifdef _WIN32
//#include <vld.h> // Visual Leak Detector
#endif 


#ifdef _WIN32
#include <windows.h>
void my_sleep( unsigned milliseconds ) {
  Sleep( milliseconds );
}
#else
#include <unistd.h>
void my_sleep( unsigned milliseconds ) {
  usleep( milliseconds * 1000 );
}
#endif


namespace po = boost::program_options;
using namespace std;
//using namespace cv;
using namespace boost;

// function prototypes
void ExtractFaceAnalysisParameters( po::variables_map variableMap, Analytics::FaceAnalyzer::FaceAnalyzerConfiguration *faceAnalyzer);

int g_verbosityLevel = 0;

string g_version = "0.75";

void InitLogging(string logfilePath)
{
	typedef log::sinks::synchronous_sink< log::sinks::text_file_backend > text_file_sink;
    boost::shared_ptr< text_file_sink > fileSink = boost::make_shared< text_file_sink >(
		log::keywords::time_based_rotation = log::sinks::file::rotation_at_time_point(0, 0, 0), // rotate at midnight
		log::keywords::file_name = logfilePath + "video_analyzer_%N.log",                           // file name pattern
		log::keywords::auto_flush = true,
        log::keywords::rotation_size = 10 * 1024 * 1024);

	fileSink->set_formatter(
		log::expressions::stream
			//<< log::expressions::format_named_scope("Scopes", "%n")
			<< "Timestamp: " << log::expressions::attr< boost::posix_time::ptime >("TimeStamp")
            << log::expressions::if_ (log::expressions::has_attr< string >("FaceID"))
			[
				// if "ID" is present then put it to the record
				log::expressions::stream << log::expressions::attr< string >("FaceID")
			]
			.else_
			[
				// otherwise add nothing
				log::expressions::stream << ""
			]
			<< ". Message: " << log::expressions::smessage 
    );

	// now add the console outputter
	typedef log::sinks::synchronous_sink< log::sinks::text_ostream_backend > text_console_sink;
    boost::shared_ptr< text_console_sink > consoleSink = boost::make_shared< text_console_sink >();

	consoleSink->locked_backend()->add_stream(
        boost::shared_ptr< std::ostream >(&std::clog, log::empty_deleter()));

	consoleSink->locked_backend()->auto_flush(true);

    // Register the sink in the logging core
    log::core::get()->add_sink(fileSink);
	log::core::get()->add_sink(consoleSink);
}

int main(int argc, char* argv[])
{
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("filename,f", po::value<string>(), "specify the input video filename to analyze")
		("video_rotation,r", po::value<int>()->default_value(0), "amount (in degrees) to rotate each frame prior to any processing")
		("verbosity,v", po::value<int>(&g_verbosityLevel)->default_value(0), "set the verbosity level (between 0-3, 0 is off, 3 is most verbose)")
		("analyzers", po::value< vector<string> >(), "analyzers to use. Options include \"FaceAnalysis\"")
		("face_detector_cascade_file", po::value<string>(), "the path to the cascade file to use for face detection")
		("eye_detector_cascade_file", po::value<string>(), "the path to the cascade file to use for eye detection")
		("face_thumbnail_path,p", po::value<string>(), "the location to put output facial thumbnails generated")
		("filename_prefix", po::value<string>(), "the filename prefix that need to be appended")
		("face_detection_frequency", po::value<int>()->default_value(15), "set how often we should perform face detection, e.g. a value of 3 means we only check every third frame, lower numbers means we check more frequently but this will be slower")
		("face_image_resize", po::value<float>()->default_value(1.0f), "specifies rescale factor for frames prior to performing face processing")
		("lost_track_process_frequency", po::value<int>()->default_value(5), "set how often a lost face should perform processing when attempting to regain the track, e.g. a value of 5 means we only check every fifth frame, lower numbers means we check more frequently but this will be slower")
		("Thumbnail_generation_frequency", po::value<int>()->default_value(1500), "set how often a thumbnail should be generated in milliseconds, e.g. a value of 1500 means we only check every frame after 1500 milliseconds and do thumbnail processing")
		("discarded_tracker_frequency", po::value<int>()->default_value(90000), "set how often a track should be pushed to discarded state in milliseconds, e.g. a value of 90000 means one and half minute")
		("maximum_concurrent_trackers", po::value<int>()->default_value(15), "set how many trackers are allowed to exist at one time")
		("render_visualization", "determines whether visualizations will be rendered")
		("log_file_path", po::value<string>(), "the path where any log files should be placed")
		("face_detector", po::value<string>()->default_value("neurotech"), "specifies which detector to use. Options include \"Neurotech\", \"Orbeus\", or \"OpenCV\"")
		("orbeus_api_key", po::value<string>(), "If the \"face_detector\", is set to \"Orbeus\" then this option must be provided. It specifies the API key to use in queries to the Orbeus API")
		("orbeus_secret_key", po::value<string>(), "If the \"face_detector\", is set to \"Orbeus\" then this option must be provided. It specifies the secret key to use in queries to the Orbeus API")
		("orbeus_namespace", po::value<string>(), "If the \"face_detector\", is set to \"Orbeus\" then this option is optional. It specifies the namespace to use in queries to the Orbeus API")
		("orbeus_user_id", po::value<string>(), "If the \"face_detector\", is set to \"Orbeus\" then this option is optional. It specifies the user_id parameter to use in queries to the Orbeus API")
		;

	po::variables_map variableMap;
	po::store(po::parse_command_line(argc, argv, desc), variableMap);
	po::notify(variableMap);    

	// they asked for help, lets give it to them
	if (variableMap.count("help")) 
	{
		cout << desc << "\n";
		return 1;

	}

	string logFilePath = "";
	// determine if they provided a log file path, if so make sure it ends in a trailing slash
	if( variableMap.count("log_file_path") )
	{
		logFilePath = variableMap["log_file_path"].as<string>();

		if( logFilePath.back() != '\\' && logFilePath.back() != '/' )
		{
			cout << "Log file path provided does not contain a trailing slash, adding a backslash '\\'. If you are on linux consider adding a '/'" << endl;
			logFilePath.append("\\");
		}
	}

	InitLogging(logFilePath); // initialize the logging
	
    log::add_common_attributes();

	log::core::get()->add_global_attribute("TimeStamp", log::attributes::local_clock());


	BOOST_LOG_TRIVIAL(info) << "OpenCV version: " << CV_VERSION;

	// If we want to test directory creation..............
	//std::string folder= "RamsriGolla";
	//bool file;
	//FileSystem::CreateDirectory(folder);
	//Jzon::Object root;
	

	JobConfiguration jobConfig;

	if (variableMap.count("filename")) 
	{
		BOOST_LOG_TRIVIAL(error) << "Input filename to analyze was set to " 
			 << variableMap["filename"].as<string>();

		jobConfig.videoSourceFilename = variableMap["filename"].as<string>();
	} 
	else 
	{
		BOOST_LOG_TRIVIAL(error) << "Input filename not specified, nothing to do";
		return 1;
	}

	// Analyzer config extraction from parameters and config file
	if( variableMap.count("analyzers") )
	{
		vector<string>::const_iterator startIter = variableMap["analyzers"].as<vector<string>>().begin();
		vector<string>::const_iterator endIter = variableMap["analyzers"].as<vector<string>>().end();
		for(; startIter!=endIter; startIter++)
		{
			// convert the string to lower case first
			//std::transform((*startIter).begin(), (*startIter).end(), (*startIter).begin(), ::tolower);

			// check to see if this value passed in by the user is known to us
			Analytics::AnalyzerType analyzerType = Analytics::ConvertStringToAnalyzerType(*startIter);
			if( analyzerType == Analytics::UnknownAnalyzer )
			{
				BOOST_LOG_TRIVIAL(error) << "Error - The specified analyzer named " << *startIter << " is not valid";
			}
			else
			{
				// we know what kind of analyzer this is so examine the variable map for the corresponding config for this analyzer
				switch(analyzerType)
				{
				case Analytics::Analyzer_FaceAnalysis:
					{
						jobConfig.isFaceAnalyzerEnabled = true;
						jobConfig.faceAnalyzerConfig = new Analytics::FaceAnalyzer::FaceAnalyzerConfiguration();
						ExtractFaceAnalysisParameters( variableMap, jobConfig.faceAnalyzerConfig );
					} break;
				default:
					{
						// do nothing
					} break;
				};
			}
		}
	}

	// now that we have all our config data nice and neat lets start processing

	const NChar * components = N_T("Biometrics.FaceDetection,Biometrics.FaceExtraction,Biometrics.FaceSegmentation,Biometrics.FaceQualityAssessment");

	NResult result = N_OK;
	NBool available = NFalse;

	int max_license_attempts = 108;
	int license_wait_secs = 300;
	int license_attempt = 0;

	while ( !available ) {
	  try {
	    if ( license_attempt > max_license_attempts ) {
	      BOOST_LOG_TRIVIAL(error) << "Failed to obtain license after " << license_attempt << " attempts.";
	      throw runtime_error( "Failed to obtain Neurotech license after exceeding max_license_attempts." );
	    } else {
	      license_attempt++;
	    }	  

	    BOOST_LOG_TRIVIAL( info ) << "Attempting to acquire Neurotech licenses.";
	    result = NLicenseObtainComponents(N_T("/local"), N_T("5000"), components, &available);
	    if ( NFailed( result ) || !available ) {
	      BOOST_LOG_TRIVIAL( info ) << "Failed to acquire Neurotech license.";
	      
	      BOOST_LOG_TRIVIAL( info ) << "Waiting " << license_wait_secs << " seconds for license.";
	      my_sleep( license_wait_secs * 1000 );
	    } else {
	      BOOST_LOG_TRIVIAL( info ) << "Neurotech license acquired.";
	    }
	  } catch ( int e ) {
	    BOOST_LOG_TRIVIAL(error) << "Exception caught while attempting to obtain Neurotech product license. Cannot continue";
	    if (!available)
	      BOOST_LOG_TRIVIAL(error) << "Neurotech Licenses for " << components << "  not available";
	    throw e;
	  }
	}

	try {
	  VideoProcessor videoProcessor(jobConfig);
	  
	  videoProcessor.PerformProcessing();

	  videoProcessor.OutputJobSummaryStatistics();

	  videoProcessor.DumpOutput(jobConfig);

	  if( jobConfig.faceAnalyzerConfig != NULL )
	    delete jobConfig.faceAnalyzerConfig;
	} catch ( Exception e ) {
	  BOOST_LOG_TRIVIAL( error ) << "Exception thrown in face detection.";
	  
	  if ( available ) {
	    NResult result2 = NLicenseReleaseComponents(components);
	    if (NFailed(result2)) {
	      BOOST_LOG_TRIVIAL(error) << "NLicenseReleaseComponents() failed (result = " << result2<< ")";
	    }
	  }
	  throw e;
	}

	if ( available ) {
	  NResult result2 = NLicenseReleaseComponents(components);
	  if (NFailed(result2)) {
	    BOOST_LOG_TRIVIAL(error) << "NLicenseReleaseComponents() failed (result = " << result2<< ")";
	  }
	}

	return 0;
}

void ExtractFaceAnalysisParameters( po::variables_map variableMap, Analytics::FaceAnalyzer::FaceAnalyzerConfiguration *faceAnalyzerConfig )
{
	if( faceAnalyzerConfig == NULL )
		return; // can't do aything

	if (variableMap.count("face_detector_cascade_file")) 
	{
		faceAnalyzerConfig->faceDetectorCascadeFile = variableMap["face_detector_cascade_file"].as<string>();
	}

	if (variableMap.count("eye_detector_cascade_file")) 
	{
		faceAnalyzerConfig->eyeDetectorCascadeFile = variableMap["eye_detector_cascade_file"].as<string>();
	}

	if (variableMap.count("face_thumbnail_path")) 
	{
		faceAnalyzerConfig->faceThumbnailOutputPath = variableMap["face_thumbnail_path"].as<string>();
	}
	if (variableMap.count("filename_prefix")) 
	{
		faceAnalyzerConfig->filenameprefix= variableMap["filename_prefix"].as<string>();
	}

	if (variableMap.count("face_detection_frequency")) 
	{
		faceAnalyzerConfig->faceDetectionFrequency = variableMap["face_detection_frequency"].as<int>();
	}

	if(variableMap.count("face_detector"))
	{
		faceAnalyzerConfig->faceDetectorType = Analytics::FaceAnalyzer::FaceDetector::ConvertStringToDetectorType(variableMap["face_detector"].as<string>());
	}

	if(variableMap.count("orbeus_user_id"))
	{
		faceAnalyzerConfig->orbeusUserId = variableMap["orbeus_user_id"].as<string>();
	}

	if(variableMap.count("orbeus_api_key"))
	{
		faceAnalyzerConfig->orbeusApiKey = variableMap["orbeus_api_key"].as<string>();
	}

	if(variableMap.count("orbeus_secret_key"))
	{
		faceAnalyzerConfig->orbeusSecretKey = variableMap["orbeus_secret_key"].as<string>();
	}

	if(variableMap.count("orbeus_namespace"))
	{
		faceAnalyzerConfig->orbeusNamespace = variableMap["orbeus_namespace"].as<string>();
	}

	if (variableMap.count("lost_track_process_frequency")) 
	{
		faceAnalyzerConfig->lostFaceProcessFrequency = variableMap["lost_track_process_frequency"].as<int>();
	}

	if (variableMap.count("Thumbnail_generation_frequency")) 
	{
		faceAnalyzerConfig->Thumbnail_generation_frequency = variableMap["Thumbnail_generation_frequency"].as<int>();
	}

	if (variableMap.count("discarded_tracker_frequency")) 
	{
		faceAnalyzerConfig->discarded_tracker_frequency = variableMap["discarded_tracker_frequency"].as<int>();
	}

	if (variableMap.count("maximum_concurrent_trackers")) 
	{
		faceAnalyzerConfig->maximumNumberActiveTrackers = variableMap["maximum_concurrent_trackers"].as<int>();
	}

	if (variableMap.count("face_image_resize")) 
	{
		faceAnalyzerConfig->imageRescaleFactor = variableMap["face_image_resize"].as<float>();
	}

	if (variableMap.count("render_visualization")) 
	{
		faceAnalyzerConfig->renderVisualization = true;
	}
}

