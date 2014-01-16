#include "gtest/gtest.h"
#include "Analytics.FaceAnalyzer/Thumbnail.h"
#include "Analytics.FaceAnalyzer/FaceAnalyzerConfiguration.h"
#include "opencv2/opencv.hpp"
#include "ThumbnailTests.h"

using namespace Analytics::FaceAnalyzer;
using namespace std;
using namespace cv;

namespace Analytics
{
	namespace FaceAnalyzer
	{

void ThumbnailTests::TestThumbnailHorizontalEyes()
{
	FaceAnalyzerConfiguration faceAnalyzerConfig;
	faceAnalyzerConfig.faceDetectorCascadeFile = "";
	faceAnalyzerConfig.eyeDetectorCascadeFile = "";

	Thumbnail thumbs(&faceAnalyzerConfig);

	// create an image to use
	Mat frame = Mat::zeros(500, 500, CV_8UC3);

	
	Point rightEye = Point(200, 200);
	Point leftEye = Point(300, 200);

	vector<Point> thumbnailPoints = thumbs.GetFaceRegion(frame, leftEye, rightEye,
														 100.0f, 1.0f, 1.5f, 1.0f);

	// now make sure the resulting points are where we think they should be
	EXPECT_EQ(9, thumbnailPoints.size()); // should be 9 points
	EXPECT_EQ(400, thumbnailPoints[0].x); // P1 should be to the right of left eye
	EXPECT_EQ(200, thumbnailPoints[0].y); // P1 should be at same horizontal level as left eye
	EXPECT_EQ(100, thumbnailPoints[3].x); // P4 should be to the left of right eye
	EXPECT_EQ(200, thumbnailPoints[3].y); // P4 should be at same horizontal level as right eye
	EXPECT_EQ(100, thumbnailPoints[4].x); // P5 should be to the left of right eye
	EXPECT_EQ(100, thumbnailPoints[4].y); // P5 should also be above right eye
	EXPECT_EQ(400, thumbnailPoints[2].x); // P3 should be to the right of left eye
	EXPECT_EQ(100, thumbnailPoints[2].y); // P3 should also be above left eye
	EXPECT_EQ(400, thumbnailPoints[1].x); // P2 should be to the right of left eye
	EXPECT_EQ(350, thumbnailPoints[1].y); // P2 should also be below left eye
	EXPECT_EQ(100, thumbnailPoints[5].x); // P6 should be to the left of right eye
	EXPECT_EQ(350, thumbnailPoints[5].y); // P6 should also be below right eye
	EXPECT_EQ(leftEye.x, thumbnailPoints[6].x); // left eye returned should be the same as what was fed in
	EXPECT_EQ(leftEye.y, thumbnailPoints[6].y); // left eye returned should be the same as what was fed in
	EXPECT_EQ(250, thumbnailPoints[7].x); // midpoint should be between left & right eyes
	EXPECT_EQ(200, thumbnailPoints[7].y); // midpoint should be between left & right eyes
}

void ThumbnailTests::TestThumbnaiVerticalEyes()
{

}

void ThumbnailTests::TestThumbnailEyesPositiveSmallAngle()
{
	FaceAnalyzerConfiguration faceAnalyzerConfig;
	faceAnalyzerConfig.faceDetectorCascadeFile = "";
	faceAnalyzerConfig.eyeDetectorCascadeFile = "";

	Thumbnail thumbs(&faceAnalyzerConfig);

	// create an image to use
	Mat frame = Mat::zeros(500, 500, CV_8UC3);

	// this is from an actual example
	Point rightEye = Point(126, 73);
	Point leftEye = Point(180, 80);

	vector<Point> thumbnailPoints = thumbs.GetFaceRegion(frame, leftEye, rightEye,
														 54.45f, 1.0f, 1.5f, 1.0f);

	// now make sure the resulting points are where we think they should be
	EXPECT_EQ(9, thumbnailPoints.size()); // should be 9 points
	EXPECT_EQ(233, thumbnailPoints[0].x); // P1 should be to the right of left eye
	EXPECT_EQ(86, thumbnailPoints[0].y); // P1 should be at same horizontal level as left eye
	EXPECT_EQ(72, thumbnailPoints[3].x); // P4 should be to the left of right eye
	EXPECT_EQ(66, thumbnailPoints[3].y); // P4 should be at same horizontal level as right eye
	EXPECT_EQ(78, thumbnailPoints[4].x); // P5 should be to the left of right eye
	EXPECT_EQ(12, thumbnailPoints[4].y); // P5 should also be above right eye
	EXPECT_EQ(239, thumbnailPoints[2].x); // P3 should be to the right of left eye
	EXPECT_EQ(32, thumbnailPoints[2].y); // P3 should also be above left eye
	EXPECT_EQ(222, thumbnailPoints[1].x); // P2 should be to the right of left eye
	EXPECT_EQ(166, thumbnailPoints[1].y); // P2 should also be below left eye
	EXPECT_EQ(61, thumbnailPoints[5].x); // P6 should be to the left of right eye
	EXPECT_EQ(146, thumbnailPoints[5].y); // P6 should also be below right eye
	EXPECT_EQ(leftEye.x, thumbnailPoints[6].x); // left eye returned should be the same as what was fed in
	EXPECT_EQ(leftEye.y, thumbnailPoints[6].y); // left eye returned should be the same as what was fed in
	EXPECT_EQ(152, thumbnailPoints[7].x); // midpoint should be between left & right eyes
	EXPECT_EQ(76, thumbnailPoints[7].y); // midpoint should be between left & right eyes
}

void ThumbnailTests::TestThumbnailEyesNegativeSmallAngle()
{
	FaceAnalyzerConfiguration faceAnalyzerConfig;
	faceAnalyzerConfig.faceDetectorCascadeFile = "";
	faceAnalyzerConfig.eyeDetectorCascadeFile = "";

	Thumbnail thumbs(&faceAnalyzerConfig);

	// create an image to use
	Mat frame = Mat::zeros(500, 500, CV_8UC3);

	// this is from an actual example
	Point rightEye = Point(126, 80);
	Point leftEye = Point(180, 73);

	vector<Point> thumbnailPoints = thumbs.GetFaceRegion(frame, leftEye, rightEye,
														 54.45f, 1.0f, 1.5f, 1.0f);

	// now make sure the resulting points are where we think they should be
	EXPECT_EQ(9, thumbnailPoints.size()); // should be 9 points
	EXPECT_EQ(233, thumbnailPoints[0].x); // P1 should be to the right of left eye
	EXPECT_EQ(66, thumbnailPoints[0].y); // P1 should be at same horizontal level as left eye
	EXPECT_EQ(72, thumbnailPoints[3].x); // P4 should be to the left of right eye
	EXPECT_EQ(86, thumbnailPoints[3].y); // P4 should be at same horizontal level as right eye
	EXPECT_EQ(65, thumbnailPoints[4].x); // P5 should be to the left of right eye
	EXPECT_EQ(32, thumbnailPoints[4].y); // P5 should also be above right eye
	EXPECT_EQ(226, thumbnailPoints[2].x); // P3 should be to the right of left eye
	EXPECT_EQ(12, thumbnailPoints[2].y); // P3 should also be above left eye
	EXPECT_EQ(243, thumbnailPoints[1].x); // P2 should be to the right of left eye
	EXPECT_EQ(146, thumbnailPoints[1].y); // P2 should also be below left eye
	EXPECT_EQ(82, thumbnailPoints[5].x); // P6 should be to the left of right eye
	EXPECT_EQ(166, thumbnailPoints[5].y); // P6 should also be below right eye
	EXPECT_EQ(leftEye.x, thumbnailPoints[6].x); // left eye returned should be the same as what was fed in
	EXPECT_EQ(leftEye.y, thumbnailPoints[6].y); // left eye returned should be the same as what was fed in
	EXPECT_EQ(152, thumbnailPoints[7].x); // midpoint should be between left & right eyes
	EXPECT_EQ(76, thumbnailPoints[7].y); // midpoint should be between left & right eyes
}

void ThumbnailTests::TestThumbnailEyesNegative45Angle()
{
	FaceAnalyzerConfiguration faceAnalyzerConfig;
	faceAnalyzerConfig.faceDetectorCascadeFile = "";
	faceAnalyzerConfig.eyeDetectorCascadeFile = "";

	Thumbnail thumbs(&faceAnalyzerConfig);

	// create an image to use
	Mat frame = Mat::zeros(500, 500, CV_8UC3);

	// this is from an actual example
	Point rightEye = Point(120, 300);
	Point leftEye = Point(170, 250);

	vector<Point> thumbnailPoints = thumbs.GetFaceRegion(frame, leftEye, rightEye,
														 70.7f, 1.0f, 1.5f, 1.0f);

	// now make sure the resulting points are where we think they should be
	EXPECT_EQ(9, thumbnailPoints.size()); // should be 9 points
	EXPECT_EQ(219, thumbnailPoints[0].x); // P1 should be to the right of left eye
	EXPECT_EQ(200, thumbnailPoints[0].y); // P1 should be at same horizontal level as left eye
	EXPECT_EQ(70, thumbnailPoints[3].x); // P4 should be to the left of right eye
	EXPECT_EQ(349, thumbnailPoints[3].y); // P4 should be at same horizontal level as right eye
	EXPECT_EQ(20, thumbnailPoints[4].x); // P5 should be to the left of right eye
	EXPECT_EQ(299, thumbnailPoints[4].y); // P5 should also be above right eye
	EXPECT_EQ(169, thumbnailPoints[2].x); // P3 should be to the right of left eye
	EXPECT_EQ(150, thumbnailPoints[2].y); // P3 should also be above left eye
	EXPECT_EQ(293, thumbnailPoints[1].x); // P2 should be to the right of left eye
	EXPECT_EQ(274, thumbnailPoints[1].y); // P2 should also be below left eye
	EXPECT_EQ(144, thumbnailPoints[5].x); // P6 should be to the left of right eye
	EXPECT_EQ(423, thumbnailPoints[5].y); // P6 should also be below right eye
	EXPECT_EQ(leftEye.x, thumbnailPoints[6].x); // left eye returned should be the same as what was fed in
	EXPECT_EQ(leftEye.y, thumbnailPoints[6].y); // left eye returned should be the same as what was fed in
	EXPECT_EQ(144, thumbnailPoints[7].x); // midpoint should be between left & right eyes
	EXPECT_EQ(275, thumbnailPoints[7].y); // midpoint should be between left & right eyes
}

void ThumbnailTests::TestThumbnailEyesPositive45Angle()
{
	FaceAnalyzerConfiguration faceAnalyzerConfig;
	faceAnalyzerConfig.faceDetectorCascadeFile = "";
	faceAnalyzerConfig.eyeDetectorCascadeFile = "";

	Thumbnail thumbs(&faceAnalyzerConfig);

	// create an image to use
	Mat frame = Mat::zeros(500, 500, CV_8UC3);

	// this is from an actual example
	Point rightEye = Point(120, 250);
	Point leftEye = Point(170, 300);

	vector<Point> thumbnailPoints = thumbs.GetFaceRegion(frame, leftEye, rightEye,
														 70.7f, 1.0f, 1.5f, 1.0f);

	// now make sure the resulting points are where we think they should be
	EXPECT_EQ(9, thumbnailPoints.size()); // should be 9 points
	EXPECT_EQ(219, thumbnailPoints[0].x); // P1 should be to the right of left eye
	EXPECT_EQ(349, thumbnailPoints[0].y); // P1 should be at same horizontal level as left eye
	EXPECT_EQ(70, thumbnailPoints[3].x); // P4 should be to the left of right eye
	EXPECT_EQ(200, thumbnailPoints[3].y); // P4 should be at same horizontal level as right eye
	EXPECT_EQ(119, thumbnailPoints[4].x); // P5 should be to the left of right eye
	EXPECT_EQ(150, thumbnailPoints[4].y); // P5 should also be above right eye
	EXPECT_EQ(268, thumbnailPoints[2].x); // P3 should be to the right of left eye
	EXPECT_EQ(299, thumbnailPoints[2].y); // P3 should also be above left eye
	EXPECT_EQ(144, thumbnailPoints[1].x); // P2 should be to the right of left eye
	EXPECT_EQ(423, thumbnailPoints[1].y); // P2 should also be below left eye
	EXPECT_EQ(-4, thumbnailPoints[5].x); // P6 should be to the left of right eye
	EXPECT_EQ(274, thumbnailPoints[5].y); // P6 should also be below right eye
	EXPECT_EQ(leftEye.x, thumbnailPoints[6].x); // left eye returned should be the same as what was fed in
	EXPECT_EQ(leftEye.y, thumbnailPoints[6].y); // left eye returned should be the same as what was fed in
	EXPECT_EQ(144, thumbnailPoints[7].x); // midpoint should be between left & right eyes
	EXPECT_EQ(274, thumbnailPoints[7].y); // midpoint should be between left & right eyes
}

void ThumbnailTests::TestThumbnailEyesPositiveLargeAngle()
{
	FaceAnalyzerConfiguration faceAnalyzerConfig;
	faceAnalyzerConfig.faceDetectorCascadeFile = "";
	faceAnalyzerConfig.eyeDetectorCascadeFile = "";

	Thumbnail thumbs(&faceAnalyzerConfig);

	// create an image to use
	Mat frame = Mat::zeros(500, 500, CV_8UC3);

	// this is from an actual example
	Point rightEye = Point(220, 220);
	Point leftEye = Point(260, 320);

	vector<Point> thumbnailPoints = thumbs.GetFaceRegion(frame, leftEye, rightEye,
														 107.7f, 1.0f, 1.5f, 1.0f);

	// now make sure the resulting points are where we think they should be
	EXPECT_EQ(9, thumbnailPoints.size()); // should be 9 points
	EXPECT_EQ(299, thumbnailPoints[0].x); // P1 should be to the right of left eye
	EXPECT_EQ(419, thumbnailPoints[0].y); // P1 should be at same horizontal level as left eye
	EXPECT_EQ(180, thumbnailPoints[3].x); // P4 should be to the left of right eye
	EXPECT_EQ(120, thumbnailPoints[3].y); // P4 should be at same horizontal level as right eye
	EXPECT_EQ(279, thumbnailPoints[4].x); // P5 should be to the left of right eye
	EXPECT_EQ(80, thumbnailPoints[4].y); // P5 should also be above right eye
	EXPECT_EQ(398, thumbnailPoints[2].x); // P3 should be to the right of left eye
	EXPECT_EQ(379, thumbnailPoints[2].y); // P3 should also be above left eye
	EXPECT_EQ(149, thumbnailPoints[1].x); // P2 should be to the right of left eye
	EXPECT_EQ(478, thumbnailPoints[1].y); // P2 should also be below left eye
	EXPECT_EQ(30, thumbnailPoints[5].x); // P6 should be to the left of right eye
	EXPECT_EQ(179, thumbnailPoints[5].y); // P6 should also be below right eye
	EXPECT_EQ(leftEye.x, thumbnailPoints[6].x); // left eye returned should be the same as what was fed in
	EXPECT_EQ(leftEye.y, thumbnailPoints[6].y); // left eye returned should be the same as what was fed in
	EXPECT_EQ(239, thumbnailPoints[7].x); // midpoint should be between left & right eyes
	EXPECT_EQ(269, thumbnailPoints[7].y); // midpoint should be between left & right eyes
}

void ThumbnailTests::TestThumbnailEyesNegativeLargeAngle()
{
	FaceAnalyzerConfiguration faceAnalyzerConfig;
	faceAnalyzerConfig.faceDetectorCascadeFile = "";
	faceAnalyzerConfig.eyeDetectorCascadeFile = "";

	Thumbnail thumbs(&faceAnalyzerConfig);

	// create an image to use
	Mat frame = Mat::zeros(500, 500, CV_8UC3);

	// this is from an actual example
	Point rightEye = Point(220, 320);
	Point leftEye = Point(260, 220);

	vector<Point> thumbnailPoints = thumbs.GetFaceRegion(frame, leftEye, rightEye,
														 107.7f, 1.0f, 1.5f, 1.0f);

	// now make sure the resulting points are where we think they should be
	EXPECT_EQ(9, thumbnailPoints.size()); // should be 9 points
	EXPECT_EQ(299, thumbnailPoints[0].x); // P1 should be to the right of left eye
	EXPECT_EQ(120, thumbnailPoints[0].y); // P1 should be at same horizontal level as left eye
	EXPECT_EQ(180, thumbnailPoints[3].x); // P4 should be to the left of right eye
	EXPECT_EQ(419, thumbnailPoints[3].y); // P4 should be at same horizontal level as right eye
	EXPECT_EQ(80, thumbnailPoints[4].x); // P5 should be to the left of right eye
	EXPECT_EQ(379, thumbnailPoints[4].y); // P5 should also be above right eye
	EXPECT_EQ(199, thumbnailPoints[2].x); // P3 should be to the right of left eye
	EXPECT_EQ(80, thumbnailPoints[2].y); // P3 should also be above left eye
	EXPECT_EQ(448, thumbnailPoints[1].x); // P2 should be to the right of left eye
	EXPECT_EQ(179, thumbnailPoints[1].y); // P2 should also be below left eye
	EXPECT_EQ(329, thumbnailPoints[5].x); // P6 should be to the left of right eye
	EXPECT_EQ(478, thumbnailPoints[5].y); // P6 should also be below right eye
	EXPECT_EQ(leftEye.x, thumbnailPoints[6].x); // left eye returned should be the same as what was fed in
	EXPECT_EQ(leftEye.y, thumbnailPoints[6].y); // left eye returned should be the same as what was fed in
	EXPECT_EQ(239, thumbnailPoints[7].x); // midpoint should be between left & right eyes
	EXPECT_EQ(270, thumbnailPoints[7].y); // midpoint should be between left & right eyes
}

TEST_F(ThumbnailTests, TestThumbnailHorizontalEyes) 
{
	TestThumbnailHorizontalEyes();
}

TEST_F(ThumbnailTests, TestThumbnaiVerticalEyes) 
{
	TestThumbnaiVerticalEyes();
}

TEST_F(ThumbnailTests, TestThumbnailEyesPositiveSmallAngle) 
{
	TestThumbnailEyesPositiveSmallAngle();
}

TEST_F(ThumbnailTests, TestThumbnailEyesNegativeSmallAngle) 
{
	TestThumbnailEyesNegativeSmallAngle();
}

TEST_F(ThumbnailTests, TestThumbnailEyesNegative45Angle) 
{
	TestThumbnailEyesNegative45Angle();
}

TEST_F(ThumbnailTests, TestThumbnailEyesPositive45Angle) 
{
	TestThumbnailEyesPositive45Angle();
}

TEST_F(ThumbnailTests, TestThumbnailEyesNegativeLargeAngle) 
{
	TestThumbnailEyesNegativeLargeAngle();
}

TEST_F(ThumbnailTests, TestThumbnailEyesPositiveLargeAngle) 
{
	TestThumbnailEyesPositiveLargeAngle();
}

	}
}