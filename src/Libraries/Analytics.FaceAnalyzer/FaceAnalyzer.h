#ifndef __FACE_ANALYSIS_ANALYZER_H__
#define __FACE_ANALYSIS_ANALYZER_H__

#include <memory>
#include <opencv2/opencv.hpp>
#include "Analyzer.h"
//#include "FaceDetector_OpenCV.h"
#include "Jzon/Jzon.h"
#include "Frame.h"
namespace Analytics
{
	namespace FaceAnalyzer
	{

// fwd declarations
class FaceAnalyzerConfiguration;
class FaceDetector_OpenCV;
class TrackingController;
class FaceDetector_Neurotech;
class Frame;
class FaceAnalysis : public Analyzer
{
private:
	// The face detector we will use to identify faces in a given frame
	std::unique_ptr<FaceDetector_Neurotech> m_faceDetector;

	std::unique_ptr<TrackingController> m_trackingController;

	Frame frameInfo;

	int m_faceDetectionFrequency;
	int m_currentFrameNumber;

	float m_imageRescaleFactor;

	bool m_renderVisualization;

	// no copy constructor or assignment operator
	FaceAnalysis(const FaceAnalysis&);
	FaceAnalysis& operator=(const FaceAnalysis&);

public:
	FaceAnalysis(FaceAnalyzerConfiguration *faceAnalyzerConfig);
	~FaceAnalysis();

	void Process(const cv::Mat &frame, uint64_t frameTimestamp);

	void GetOutput(Jzon::Object*& root);
};

// end of namespaces
	}
}
#endif 