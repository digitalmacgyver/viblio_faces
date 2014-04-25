/*
	This application is designed for testing thumbnail still images. Essentially you provide
	it an image (or directory of images) and it will load each image and perform the thumbnail
	extraction on it. This is useful for testing the thumbnail extraction methodology
*/

#include "boost/program_options.hpp"

#include "FileSystem/FileSystem.h"
#include "Analytics.FaceAnalyzer/Thumbnail.h"
#include "Analytics.FaceAnalyzer/ThumbnailDetails.h"
#include "Analytics.FaceAnalyzer/Frame.h"
#include "Jzon/Jzon.h"

#include <opencv2/opencv.hpp>

#include <algorithm>
#include <string>

namespace po = boost::program_options;
using namespace std;
using namespace Analytics::FaceAnalyzer;

void PerformThumbnailExtration(const string &filename);
void DumpThumbnailJsonData(const string &imageFilename, ThumbnailDetails *thumbnailDetails);

int g_verbosityLevel = 0;

int main(int argc, char* argv[])
{
	// Declare the supported options.
	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("filename,f", po::value<string>(), "specify the input image filename to analyze")
		("directory,d", po::value<string>(), "specify the directory for which thumbnail processing will be performed on every image file found")
		("verbosity,v", po::value<int>(&g_verbosityLevel)->default_value(0), "set the verbosity level (between 0-3, 0 is off, 3 is most verbose)")
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

	bool hasFile = false;
	bool hasDir = false;

	if (variableMap.count("filename")) 
	{
		hasFile = true;
	} 
	else if(variableMap.count("directory")) 
	{
		hasDir = true;
	}
	else
	{
		cout << "Input filename or directory not specified, nothing to do" << endl;
		return 1;
	}

	// now that we have determined that we have some file or directory to process make sure it
	// actually exists
	if( hasDir && !FileSystem::DirectoryExists(variableMap["directory"].as<string>()) )
	{
		cout << "Directory " << variableMap["directory"].as<string>() << " does not exist, cannot continue" << endl;
		return 1;
	}
	else if( hasFile && !FileSystem::FileExists(variableMap["filename"].as<string>()) )
	{
		cout << "File " << variableMap["filename"].as<string>() << " does not exist, cannot continue" << endl;
		return 1;
	}
	else
	{
		// all good, it must exist
	}

	// create a list of files that we will be processing. If they specified a file then its all good we will only
	// have one entry
	map<string,bool> filesToProcess;
	if( hasFile )
		filesToProcess[variableMap["filename"].as<string>()] = false; // not a directory
	else
	{
		bool directoryContentsSuccess = FileSystem::GetDirectoryContents(variableMap["directory"].as<string>(), filesToProcess);
	}

	// now we must iterate over all the files and determine which ones are images we can process
	map<string,bool>::iterator startIter = filesToProcess.begin();
	for(; startIter != filesToProcess.end(); startIter++ )
	{
		std::string filename = (*startIter).first; 
		std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);
		if( FileSystem::HasSuffix(filename, ".png") || FileSystem::HasSuffix(filename, ".jpg") || FileSystem::HasSuffix(filename, ".pgm") )
		{
			// we have found an image, lets try and extract thumbnail details from it
			PerformThumbnailExtration(filename);
		}
	}

	return 0;
}

void PerformThumbnailExtration(const string &filename)
{
	// first we load the image in
	Mat thumbnailImage = imread(filename);

	namedWindow("Input Image");
	imshow("Input Image", thumbnailImage);
	waitKey(0);

	// then create the Frame object that will just contain the full resolution version of the thumbnail
	// we just read in
	Frame fullFrame(thumbnailImage);
	
	// now set a Rect that is slightly smaller than the entire thumbnail image
	Rect frameRect = Rect(thumbnailImage.cols*0.05f, thumbnailImage.rows*0.05f, thumbnailImage.cols*0.95f, thumbnailImage.rows*0.95f);

	// thumbDetails will store the resulting details the thumbnail generator will extract
	ThumbnailDetails *thumbDetails = new ThumbnailDetails();

	float finalConfidence = 0.0f;

	// extract/compute the thumbnail details
	Thumbnail thumbnailGenerator(NULL);
	bool thumbnailGenerationSuccess = thumbnailGenerator.ExtractThumbnail(frameRect, finalConfidence, *thumbDetails, fullFrame);

	// now we can dump the json data out for this thumbnail
	if( thumbnailGenerationSuccess )
		DumpThumbnailJsonData(filename, thumbDetails);
	else
		cout << "Failed to extract thumbnail information for image " << filename << endl;

	delete thumbDetails;
}

void DumpThumbnailJsonData(const string &imageFilename, ThumbnailDetails *thumbnailDetails)
{
	Jzon::Object root;// = new Jzon::Object;

	root.Add("face_confidence",thumbnailDetails->GetDetailedInformation().faceDetectionConfidence);
	root.Add("face_rotation_pitch",thumbnailDetails->GetDetailedInformation().pitch);
	root.Add("face_rotation_roll",thumbnailDetails->GetDetailedInformation().roll);
	root.Add("face_rotation_yaw",thumbnailDetails->GetDetailedInformation().yaw);
	root.Add("width",thumbnailDetails->GetDetailedInformation().faceRect.width);
	root.Add("height",thumbnailDetails->GetDetailedInformation().faceRect.height);
	root.Add("backgroundUniformity",thumbnailDetails->backgroundUniformity);
	root.Add("sharpness",thumbnailDetails->sharpness);
	root.Add("totalConfidence",thumbnailDetails->confidence);
	root.Add("grayscaleDensity",thumbnailDetails->grayscaleDensity);
	
	Jzon::Array left_eye;
	left_eye.Add(int(thumbnailDetails->GetDetailedInformation().leftEye.x));left_eye.Add(int(thumbnailDetails->GetDetailedInformation().leftEye.x));
	root.Add("leftEyeCenter",left_eye);
	root.Add("leftEyeConfidence",thumbnailDetails->GetDetailedInformation().leftEyeConfidence);
	Jzon::Array right_eye;
	right_eye.Add(int(thumbnailDetails->GetDetailedInformation().rightEye.x));right_eye.Add(int(thumbnailDetails->GetDetailedInformation().rightEye.y));
	root.Add("rightEyeCenter",right_eye);
	root.Add("rightEyeConfidence",thumbnailDetails->GetDetailedInformation().rightEyeConfidence);
	root.Add("interEyeDistance",thumbnailDetails->GetDetailedInformation().intereyeDistance);
	Jzon::Array NoseLocation;
	NoseLocation.Add(int(thumbnailDetails->GetDetailedInformation().noseLocation.x));NoseLocation.Add(int(thumbnailDetails->GetDetailedInformation().noseLocation.y));
	root.Add("noseLocation",NoseLocation);
	root.Add("noseLocationConfidence",thumbnailDetails->GetDetailedInformation().noseLocationConfidence);
	Jzon::Array MouthLocation;
	MouthLocation.Add(int(thumbnailDetails->GetDetailedInformation().mouthLocation.x));MouthLocation.Add(int(thumbnailDetails->GetDetailedInformation().mouthLocation.y));
	root.Add("mouthLocation",MouthLocation);
	root.Add("mouthLocationConfidence",thumbnailDetails->GetDetailedInformation().mouthLocationConfidence);
	root.Add("mouthOpen",thumbnailDetails->GetDetailedInformation().mouthOpen);
	root.Add("mouthOpenConfidence",thumbnailDetails->GetDetailedInformation().mouthOpenConfidence);
	root.Add("isMale",thumbnailDetails->GetDetailedInformation().isMale);
	root.Add("genderConfidence",thumbnailDetails->GetDetailedInformation().genderConfidence);
	root.Add("blinking",thumbnailDetails->GetDetailedInformation().blinking);
	root.Add("blinkingConfidence",thumbnailDetails->GetDetailedInformation().blinkingConfidence);
	root.Add("expression",thumbnailDetails->GetDetailedInformation().GetExpression());
	root.Add("expressionConfidence",thumbnailDetails->GetDetailedInformation().expressionConfidence);
	root.Add("wearingGlasses",thumbnailDetails->GetDetailedInformation().wearingGlasses);
	root.Add("glassesConfidence",thumbnailDetails->GetDetailedInformation().glassesConfidence);
	root.Add("wearingDarkGlasses",thumbnailDetails->GetDetailedInformation().wearingDarkGlasses);
	root.Add("wearingDarkGlassesConfidence",thumbnailDetails->GetDetailedInformation().wearingDarkGlassesConfidence);

	// form a path for the resulting json file
	string jsonFilePath = imageFilename;
	string imageFileSuffix = FileSystem::FileSuffix(imageFilename);

	if( imageFileSuffix.empty() )
	{
		cout << "Error: Cannot find the file suffix for input file " << imageFilename << ". Cannot establish resulting json filename" << endl;
		return;
	}

	jsonFilePath = imageFilename.substr(0, imageFilename.length() - imageFileSuffix.length()) + ".json";

	Jzon::FileWriter::WriteFile(jsonFilePath, root, Jzon::StandardFormat);
}