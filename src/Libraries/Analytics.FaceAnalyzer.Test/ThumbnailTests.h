#ifndef __THUMBNAIL_TESTS_H__
#define __THUMBNAIL_TESTS_H__

#include "gtest/gtest.h"

namespace Analytics
{
	namespace FaceAnalyzer
	{

class ThumbnailTests : public ::testing::Test
{
public:
	ThumbnailTests(){};
	~ThumbnailTests(){};

	void TestThumbnailHorizontalEyes();
	void TestThumbnaiVerticalEyes();
	void TestThumbnailEyesPositiveSmallAngle();
	void TestThumbnailEyesNegativeSmallAngle();
	void TestThumbnailEyesNegative45Angle();
	void TestThumbnailEyesPositive45Angle();
	void TestThumbnailEyesNegativeLargeAngle();
	void TestThumbnailEyesPositiveLargeAngle();
};

	}
}

#endif