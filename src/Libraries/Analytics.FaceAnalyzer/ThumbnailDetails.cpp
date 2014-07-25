#include "ThumbnailDetails.h"

namespace Analytics {

  namespace FaceAnalyzer {

    ThumbnailDetails::ThumbnailDetails(void) {
      backgroundUniformity = 0.0f;
      sharpness = 0.0f;
      confidence = 0.0f;
      grayscaleDensity = 0.0f;
      //thumbnaildeets = new FaceDetectionDetails;
    }

    ThumbnailDetails::~ThumbnailDetails(void)
    {
    }


    void ThumbnailDetails::FillThumbnailDetails(const cv::Mat &frame,  FaceDetectionDetails ThumbnailLocation)
    {
      thumbnail = frame.clone();
      thumbnaildeets = ThumbnailLocation;
    }

  }
}
