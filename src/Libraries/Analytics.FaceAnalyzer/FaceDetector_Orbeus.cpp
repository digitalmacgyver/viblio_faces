#include "FaceDetector_Orbeus.h"
#include "FaceAnalyzerConfiguration.h"

#include <opencv2/opencv.hpp>
#include <boost/log/trivial.hpp>

#include <stdexcept>
#include <vector>

#include <stdio.h>
#include <stdlib.h>

#include "jpeg-compressor/jpge.h"

#include "RekognitionApi/RESTapi.h"
#include "RekognitionApi/Base64Codec.h"
#include "RekognitionApi/json/json.h"

using namespace std;
using namespace cv;

namespace Analytics
{
	namespace FaceAnalyzer
	{

static int CurlResponseCallback(char *data, size_t size, size_t nmemb, FaceDetector_Orbeus *faceDetectorInstance);

FaceDetector_Orbeus::FaceDetector_Orbeus(FaceAnalyzerConfiguration *faceAnalyzerConfiguration)
	:
	 FaceDetector(faceAnalyzerConfiguration)
{
	if( faceAnalyzerConfiguration == NULL )
	{
		BOOST_LOG_TRIVIAL(error) << "Face Analyzer configuration provided is NULL. Cannot construct Orbeus face detector";
		throw runtime_error("Face Analyzer configuration is NULL");
	}

	if( faceAnalyzerConfiguration->orbeusApiKey.empty() )
	{
		BOOST_LOG_TRIVIAL(error) << "When using the Orbeus API the API key must be provided";
		throw runtime_error("Orbeus API Key not provided");
	}

	if( faceAnalyzerConfiguration->orbeusSecretKey.empty() )
	{
		BOOST_LOG_TRIVIAL(error) << "When using the Orbeus API the secret key must be provided";
		throw runtime_error("Orbeus secret Key not provided");
	}

	m_apiKey = faceAnalyzerConfiguration->orbeusApiKey;
	m_apiSecret = faceAnalyzerConfiguration->orbeusSecretKey;

	// this buffer will be allocated the first time we get a frame to perform detection on, its only then do we have
	// an idea as to the size the buffer needs to be
	m_jpgBuffer = NULL;

	m_jpgBufferSize = -1;
}

FaceDetector_Orbeus::~FaceDetector_Orbeus()
{
	if(m_jpgBuffer != NULL)
	{
		delete [] m_jpgBuffer;
		m_jpgBuffer = NULL;
	}
}

std::vector<FaceDetectionDetails> FaceDetector_Orbeus::Detect(const Mat &frame, bool getDetailedInformation)
{
	vector<FaceDetectionDetails> faces;

	Json::Value response;
	const string api_addr_base = "http://rekognition.com/func/api/?";
	map<string, string> query_config;

	// for testing only, please use client specific key and secret!
	query_config["api_key"] = m_apiKey;
	query_config["api_secret"] = m_apiSecret;
	if(!getDetailedInformation)
		query_config["jobs"] = "face_part_aggressive";
	else
		query_config["jobs"] = "face_part_detail_gender_emotion_age_race_glass_mouth_open_wide_aggressive_beauty";
	
	// take the input frame and compress it using the JPEG compressor
	// Fill in the compression parameter structure.
	jpge::params jpgParams;
	jpgParams.m_quality = 90;
	//params.m_subsampling = (subsampling < 0) ? ((actual_comps == 1) ? jpge::Y_ONLY : jpge::H2V2) : static_cast<jpge::subsampling_t>(subsampling);
	jpgParams.m_two_pass_flag = true;


	// Writes JPEG image to memory buffer. 
	// On entry, buf_size is the size of the output buffer pointed at by pBuf, which should be at least ~1024 bytes. 
	// If return value is true, buf_size will be set to the size of the compressed data.
	int newJpgBufferSize = frame.rows * frame.cols * 2; // really overkill
	if( newJpgBufferSize != m_jpgBufferSize )
	{
		// if these two sizes are different it means we have to reinitialize our jpg buffer for storing the JPEG compressed data
		if( m_jpgBuffer != NULL)
			delete [] m_jpgBuffer;

		m_jpgBufferSize = newJpgBufferSize;
		m_jpgBuffer = new char[m_jpgBufferSize];
	}
	
	bool compressionResult = compress_image_to_jpeg_file_in_memory((void*)m_jpgBuffer, newJpgBufferSize, frame.cols, frame.rows, 3, frame.data, jpgParams);
	if( !compressionResult )
	{
		// failure! we couldn't compress the frame
		return faces;
	}

	string jpgData;
	jpgData.append(m_jpgBuffer, newJpgBufferSize);

	rekognition_api::Base64Codec base64_codec;
	string encoded;
	base64_codec.Encode(jpgData, &encoded);
	query_config["base64"] = encoded;

	if (!rekognition_api::APICall(api_addr_base, query_config, &response)) 
	{
		BOOST_LOG_TRIVIAL(error) << "Orbeus API call failed";
		return faces;
	}

	if( !response.isMember("face_detection") )
	{
		BOOST_LOG_TRIVIAL(error) << "No face detection information was provided in the response from the Orbeus API";
		return faces;
	}

	// For the format of the results, please refer to our doc:
	// http://rekognition.com/docs/
	const Json::Value face_detection = response["face_detection"];
	for (unsigned int i = 0; i < face_detection.size(); ++i) 
	{
		if( !face_detection[i].isMember("boundingbox") || !face_detection[i]["boundingbox"].isMember("tl") ||
			!face_detection[i]["boundingbox"].isMember("size") || !face_detection[i].isMember("confidence") )
			continue;

		FaceDetectionDetails currentFaceDeets;

		Rect currentFaceRect;
		currentFaceRect.x= face_detection[i]["boundingbox"]["tl"]["x"].asInt() ;
		currentFaceRect.y = face_detection[i]["boundingbox"]["tl"]["y"].asInt() ; 
		currentFaceRect.width = face_detection[i]["boundingbox"]["size"]["width"].asInt();
		currentFaceRect.height = face_detection[i]["boundingbox"]["size"]["height"].asInt();
		Rect constrainedRect = ConstrainRect(currentFaceRect, Size(frame.cols, frame.rows));

		currentFaceDeets.faceRect = constrainedRect;
		currentFaceDeets.faceDetectionConfidence = face_detection[i]["confidence"].asDouble();

		if( getDetailedInformation )
		{
			currentFaceDeets.hasAdditionalFaceInformation = true;

			// pull out the pose
			if( face_detection[i].isMember("pose") && 
				face_detection[i]["pose"].isMember("yaw") && face_detection[i]["pose"].isMember("pitch") && face_detection[i]["pose"].isMember("roll") )
			{
				currentFaceDeets.yaw = face_detection[i]["pose"]["yaw"].asInt();
				currentFaceDeets.pitch = face_detection[i]["pose"]["pitch"].asInt();
				currentFaceDeets.roll = face_detection[i]["pose"]["roll"].asInt();
			}

			// pull out the position of the nose
			if( face_detection[i].isMember("nose") &&
				face_detection[i]["nose"].isMember("x") && face_detection[i]["nose"].isMember("y") )
			{
				currentFaceDeets.noseLocation.x = face_detection[i]["nose"]["x"].asInt();
				currentFaceDeets.noseLocation.y = face_detection[i]["nose"]["y"].asInt();
				currentFaceDeets.noseLocationConfidence = 50;
			}

			// pull out the position of the left eye
			if( face_detection[i].isMember("eye_left") &&
				face_detection[i]["eye_left"].isMember("x") && face_detection[i]["eye_left"].isMember("y") )
			{
				currentFaceDeets.leftEye.x = face_detection[i]["eye_left"]["x"].asInt();
				currentFaceDeets.leftEye.y = face_detection[i]["eye_left"]["y"].asInt();
				currentFaceDeets.leftEyeConfidence = 0.5f;
			}

			// pull out the position of the right eye
			if( face_detection[i].isMember("eye_right") && 
				face_detection[i]["eye_right"].isMember("x") && face_detection[i]["eye_right"].isMember("y") )
			{
				currentFaceDeets.rightEye.x = face_detection[i]["eye_right"]["x"].asInt();
				currentFaceDeets.rightEye.y = face_detection[i]["eye_right"]["y"].asInt();
				currentFaceDeets.rightEyeConfidence = 0.5f;
			}

			if( currentFaceDeets.rightEyeConfidence > 0.0f && currentFaceDeets.leftEyeConfidence > 0.0f )
			{
				// we can only calculate the intereye distance if we have both eye locations
				if( currentFaceDeets.leftEye.y == currentFaceDeets.rightEye.y )
					currentFaceDeets.intereyeDistance = float(currentFaceDeets.leftEye.x - currentFaceDeets.rightEye.x);
				else
					currentFaceDeets.intereyeDistance = sqrt( pow( currentFaceDeets.leftEye.x - currentFaceDeets.rightEye.x, 2 )
															 +pow( currentFaceDeets.leftEye.y - currentFaceDeets.rightEye.y, 2 ) );
			}
			else
				currentFaceDeets.intereyeDistance = 0.0f;

			// determine if the person is wearing glasses or not
			if( face_detection[i].isMember("glasses") )
			{
				currentFaceDeets.glassesConfidence = face_detection[i]["glasses"].asDouble();
				if(currentFaceDeets.glassesConfidence > 0.0f)
					currentFaceDeets.wearingGlasses = true;
			}
		}

		faces.push_back(currentFaceDeets);
	}

	return faces;
}

	}
}