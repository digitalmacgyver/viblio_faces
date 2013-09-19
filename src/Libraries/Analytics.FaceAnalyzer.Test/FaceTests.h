#ifndef __FACE_TESTS_H__
#define __FACE_TESTS_H__

#include "gtest/gtest.h"
//#include "Analytics.FaceAnalyzer/Face.h"

namespace Analytics
{
	namespace FaceAnalyzer
	{

class FaceTests : public ::testing::Test
{
public:
	FaceTests(){};
	~FaceTests(){};

	void TestMergeFunctionalitySimple();
	void TestMergeFunctionalityLessSimple();
	void TestMergeFunctionalityMultipleInterleaved();
};

	}
}

#endif