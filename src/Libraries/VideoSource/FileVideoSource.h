#ifndef __FILE_VIDEOSOURCE__
#define __FILE_VIDEOSOURCE__

#include "VideoSource.h"

namespace VideoSource
{

class FileVideoSource : public VideoSourceBase
{
private:
	cv::VideoCapture *m_videoFile;

public:
	FileVideoSource(const std::string &filename);
	~FileVideoSource();

	// For file video sources the next frame is always available and ready
	virtual bool NextFrameReady(){ return true; };

	// Returns the next frame from the file
	virtual cv::Mat GetNextFrame();

	// Returns the total number of frames in this file
	virtual int NumberFrames();
};

} // end of namespace

#endif