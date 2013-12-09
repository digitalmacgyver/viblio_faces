#include "FaceAnalyzerConfiguration.h"
#include "Thumbnail.h"
#include "ThumbnailDetails.h"
#include "EyeDetector_OpenCV.h"
#include "Jzon/Jzon.h"
#include <numeric>
#include "Frame.h"

// Neurotech related libraries for token image generation
#include <NImages.h>
#include <Ntfi.h>
#include <NLExtractor.h>
#include <NLicensing.h>

using namespace cv;
using namespace std;

#define M_PI 3.1415926535897932384626433832795

namespace Analytics
{
	namespace FaceAnalyzer
	{


Thumbnail::Thumbnail(FaceAnalyzerConfiguration *faceAnalyzerConfig) :
	m_confidenceWeightFaceDetected(0.2f),
	m_confidenceWeightEyesDetected(0.15f),
	m_confidenceWeightIntereyeDistance(0.15f),
	m_confidenceWeightNoseDetected(0.04f),
	m_confidenceWeightMouthDetected(0.03f),
	m_confidenceWeightGenderDetected(0.03f),
	m_confidenceWeightQuality(0.4f), // all these confidence weights must add to 1
	m_intereyeDistanceLowerBound(40.0f),
	m_intereyeDistanceUpperBound(100.0f),
	m_upperThumbnailRegion(1.5f),
	m_lowerThumbnailRegion(2.7f),
	m_leftRightThumbnailRegion(1.0f)
{  
	tokenFaceExtractor = NULL;
	has_eyecascade= false;
	face_detector_check.reset( new FaceDetector_Neurotech() );
//	thumbnail_details.reset(new ThumbnailDetails());
	//face_detector_check.reset( new FaceDetector_Neurotech(faceAnalyzerConfig->faceDetectorCascadeFile));
	//face_detector_neuro.reset(new FaceDetector_Neurotech());
	//if(!faceAnalyzerConfig->eyeDetectorCascadeFile.empty())
	//{   has_eyecascade = true;
	//	eye_detector_check.reset(new EyeDetector_OpenCV(faceAnalyzerConfig->eyeDetectorCascadeFile));
	//}
	
	Thumbnail_enlarge_percentage = 0.75f;
	NBool available = false;

	NResult result = N_OK;

	const NChar * components = N_T("Biometrics.FaceDetection,Biometrics.FaceSegmentation,Biometrics.FaceQualityAssessment");
	try
	{	
		result = NLicenseObtainComponents(N_T("/local"), N_T("5000"), components, &available);
		if( NFailed(result) )
			throw runtime_error("Failed to obtain Neurotech licenses for the Token & Image quality related components");
	}
	catch(Exception e)
	{
		cout << "Exception caught while attempting to obtain Neurotech product license. Cannot continue" << endl;
		if (!available)
			cout << "Neurotech Licenses for " << components << "  not available" << endl;
		throw e;
	}
	
	try
	{	
		result = NtfiCreate(&tokenFaceExtractor);
		if( NFailed(result) )
			throw runtime_error("Failed to Token image extractor");
	}
	catch(Exception e)
	{
		cout << "Exception caught while attempting to create a Neurotech token image extractor. Cannot continue" << endl;
		
		throw e;
	}

}


Thumbnail::~Thumbnail()
{
	if( tokenFaceExtractor != NULL )
		NObjectFree(tokenFaceExtractor);
}



bool Thumbnail::ExtractThumbnail( const cv::Mat &frame, const cv::Rect &ThumbnailLocation, float &confidence, ThumbnailDetails &thumbnail_details,Frame &origFrame)
{   
	cv::Mat thumbnail;
	confidence = 0.0f;
	
	float scalefactor = origFrame.getscale();
	

	Rect enlarged_thumbnail((int)(ThumbnailLocation.x*scalefactor)-(int)(ThumbnailLocation.width*Thumbnail_enlarge_percentage*scalefactor),(int)(ThumbnailLocation.y*scalefactor)-(int)(ThumbnailLocation.height*scalefactor*Thumbnail_enlarge_percentage),(int)(ThumbnailLocation.width*scalefactor)+(int)(ThumbnailLocation.width*scalefactor*(Thumbnail_enlarge_percentage*2)),(int)(ThumbnailLocation.height*scalefactor)+(int)(ThumbnailLocation.height*scalefactor*Thumbnail_enlarge_percentage*2));
	Rect constrainedRect = ConstrainRect(enlarged_thumbnail, Size(origFrame.getcols(), origFrame.getrows()));

	thumbnail = origFrame.GetThumbnail(constrainedRect);
 
	thumbnail_details.SetThumbnail(thumbnail);

	// perform a detailed face extraction to get some detailed information
	vector<FaceDetectionDetails> detectedFaces = face_detector_check->Detect(thumbnail, true);

	if( detectedFaces.size() > 1 || detectedFaces.size() == 0 )
	{
		// if we find either no faces or more than 1 face in this 'little' region then we have 0 confidence in
		// this thumbnail
		return false;
	}
	
	// now use the detailed information to create a token image
	FaceDetectionDetails detailedFaceInfo;
	detailedFaceInfo = detectedFaces.at(0);

	// we have detected the face at least
	confidence += m_confidenceWeightFaceDetected * (detailedFaceInfo.faceDetectionConfidence/100.0f);

	// if we have detected the eyes then this gives us even more confidence
	if( detailedFaceInfo.rightEyeConfidence > 0 && detailedFaceInfo.leftEyeConfidence > 0 )
	{
		confidence += ((m_confidenceWeightEyesDetected/2.0f) * detailedFaceInfo.rightEyeConfidence) + 
					  ((m_confidenceWeightEyesDetected/2.0f) * detailedFaceInfo.leftEyeConfidence);

		// based on the intereye distance calculate how much additional confidence to add
		float adjustedIntereyeDist = max(0.0f, detailedFaceInfo.intereyeDistance - m_intereyeDistanceLowerBound);

		float intereyeWeightAdjusted = min(1.0f, adjustedIntereyeDist / (m_intereyeDistanceUpperBound - m_intereyeDistanceLowerBound));

		confidence += m_confidenceWeightIntereyeDistance * intereyeWeightAdjusted;
	}

	thumbnail_details.SetDetailedInformation(detailedFaceInfo);

	// check to make sure we have the prerequisite information for token extraction before proceeding
	if(detailedFaceInfo.leftEyeConfidence <= 0 || detailedFaceInfo.rightEyeConfidence <= 0)
	{
		return false;
	}

	// we need both eyes in order to accurately compute the thumbnail region and to undo any roll
	GetFaceRegion(thumbnail, detailedFaceInfo.leftEye, detailedFaceInfo.rightEye, detailedFaceInfo.intereyeDistance,
				  m_upperThumbnailRegion, m_lowerThumbnailRegion, m_leftRightThumbnailRegion);

	// Make use of the nose and mouth location information in our confidence estimation. Having one or both
	// of these gives us more confidence
	confidence += m_confidenceWeightNoseDetected * detailedFaceInfo.noseLocationConfidence;
	confidence += m_confidenceWeightMouthDetected * detailedFaceInfo.noseLocationConfidence;

	NResult result ;
	NPoint first;
	NPoint second;
	HNImage token = NULL;
	HNImage image = NULL;
	HNtfiAttributes ntfiAttributes = NULL;
	NDouble quality;

	// first convert out thumbnail into an HNImage
	bool success = MatToHNImage(thumbnail, &image);

	if(!success)
	{
		// Failed to convert the OpenCV Mat into a Neurotech HNImage

		if(image)
			NObjectFree(image);

		return false;
	}

	// now setup the left and right eye positions and use them to create the token image
	first.X = detailedFaceInfo.rightEye.x;
	first.Y = detailedFaceInfo.rightEye.y;
	second.X = detailedFaceInfo.leftEye.x;
	second.Y = detailedFaceInfo.leftEye.y;
	result = NtfiCreateTokenFaceImageEx(tokenFaceExtractor, image, &first, &second, &token);
	if (NFailed(result))
	{
		cout << "NtfiCreateTokenFaceImage()failed (result = " << result<< ")!" << endl;
		if(image)
			NObjectFree(image);
		if( token)
			NObjectFree(token);
		
		return false;
	}

	//const NChar * savePath = N_T("C:\\temp\\tokenTest.jpg");
	//result = NImageSaveToFileEx(token, savePath, NULL, NULL, 0);

	// now that we have the token image lets test it to establish its quality
	result = NtfiTestTokenFaceImage(tokenFaceExtractor, token, &ntfiAttributes, &quality);
	if (NFailed(result))
	{
		cout << "NtfiCreateTokenFaceImage() quality extractor failed (result = " << result<< ")!" << endl;
		if(image)
			NObjectFree(image);

		if(token)
			NObjectFree(token);

		if (ntfiAttributes)
			NObjectFree(ntfiAttributes);

		return false;
	}

	confidence += m_confidenceWeightQuality * quality;

	// Determine the background uniformity - see page 729 in the SDK documentation
	double backgroundUniformity = 0.0f;
	NtfiAttributesGetBackgroundUniformity(ntfiAttributes, &backgroundUniformity);

	// determine the image sharpness
	double sharpness = 0.0f;
	NtfiAttributesGetSharpness(ntfiAttributes, &sharpness);
	
	// lastly, determine the grayscale density
	double grayscaleDensity = 0.0f;
	NtfiAttributesGetGrayscaleDensity(ntfiAttributes, &grayscaleDensity);
	
	thumbnail_details.backgroundUniformity = backgroundUniformity;
	thumbnail_details.grayscaleDensity = grayscaleDensity;
	thumbnail_details.sharpness = sharpness;
	// Now convert the token image back into an OpenCV Mat
	//Mat tokenMat = HNImageToMat(&token);
	
	//imshow("Token image", tokenMat);
	//waitKey(0);

	if(image)
		NObjectFree(image);
	if(token)
		NObjectFree(token);
	if (ntfiAttributes)
		NObjectFree(ntfiAttributes);

	confidence = quality;

	return true;
}

bool Thumbnail::MatToHNImage(const Mat &matImage, HNImage *hnImage)
{
	unsigned char *pixelData = new unsigned char[matImage.rows*matImage.cols*matImage.channels()];
	
	for (int row=0;row<matImage.rows;row++)
	{
		const unsigned char *data = matImage.ptr(row);
		for (int col=0;col<matImage.cols;col++)
		{
			// then use *data for the pixel value, assuming you know the order, RGB etc           
			// Note 'rgb' is actually stored B,G,R
		   pixelData[(row * matImage.cols + col)*3 + 2]= *data++;
		   pixelData[(row * matImage.cols + col)*3 + 1] = *data++;
		   pixelData[(row * matImage.cols + col)*3 + 0]= *data++;
		}
	}

	NResult result = NImageCreateFromDataEx(npfRgb, matImage.cols, matImage.rows, matImage.cols*matImage.channels(), matImage.cols*matImage.channels(), pixelData, (matImage.cols*matImage.channels())*matImage.rows, 0, hnImage);

	delete [] pixelData;

	if(NFailed(result))
	{
		cout << "NImageCreateFromDataEx failed (result = " << result<< ")!" << endl;
		return false;
	}

	// success
	return true;
}

Mat Thumbnail::HNImageToMat(HNImage *hnImage)
{
	NUInt width = 0, height = 0;

	NImageGetWidth(*hnImage, &width);
	NImageGetHeight(*hnImage, &height);
	NInt channels = 0;
	NImageGetPlaneCount(*hnImage, &channels);
	Mat matImage;

	if( channels == 3 )
	{
		matImage = Mat(height, width, CV_8UC3);
		NImageCopyToData(*hnImage, npfRgb, matImage.cols, matImage.rows, matImage.step, matImage.data, matImage.step * matImage.rows, 0, 0, 0);
		cvtColor(matImage, matImage, CV_BGR2RGB);
	}
	else if( channels == 1 )
	{
		matImage = Mat(height, width, CV_8UC1);
		NImageCopyToData(*hnImage, npfGrayscale, matImage.cols, matImage.rows, matImage.step, matImage.data, matImage.step * matImage.rows, 0, 0, 0);
	}
	else
	{
		cout << "Unsupported number of channels when converting from HNImage to Mat" << endl;
	}

	return matImage;
}

/*
    This function determines where the actual face region is based on the provided
    eye coordinates and the amount of space desired above, below and beside the eyes, 
    i.e. the rectangle that encloses the face region. 
    It determines whether any portion of the face region will fall outside 
    of the image region and if so returns false. If the oriented face region
    is inside the image bounds the function rotates the image such that the eyes are 
    at the same horizontal level and returns the new eye coordinates. The rotated version
    of the image is stored in the member variable
*/
bool Thumbnail::GetFaceRegion(const Mat &frame, Point leftEye, Point rightEye,
                              float intereyeDistance, float aboveEyeBuffer, float belowEyeBuffer, float besideEyeBuffer)
{
    if( (frame.empty() ) )//|| (m_pRotatedTempImg == NULL) )
        return false;

	// there is a possibility for the face to be upside down, we can account for that here, we will
	// simply swap the eyes over and the buffers around the thumbnail region, we'll set the final rotation amount
	// to 180 + theta
	Point tempEye;
    float rotationCorrection = 0;
    float currentAboveEyeBuf = aboveEyeBuffer;
    float currentBelowEyeBuf = belowEyeBuffer;
    if( rightEye.x > leftEye.x ) // this means the person's left eye is on the right of the right eye
    {
		tempEye = rightEye;
		rightEye = leftEye;
		leftEye = tempEye;
        rotationCorrection = 180;
        currentAboveEyeBuf = belowEyeBuffer;
        currentBelowEyeBuf = aboveEyeBuffer;
    }
    else
    {
		// nothing to do as eyes are correct already
    }

    // now lets determine the midpoint between the eyes. The midpoint is needed so that we can
    // rotate around this point
    float theta = 0;
    float thetaRadians = 0;
    float midpointX = 0;
    float midpointY = 0;
    if( leftEye.y != rightEye.y ) // eyes not on the same level
    {
		// we'll use a bit of trig here
        float opposite = float( leftEye.y - rightEye.y );
        float adjacent = float( leftEye.x - rightEye.x );

        if( adjacent == 0 && leftEye.x == rightEye.x )
            // this means the eyes are in the same x position, we've possibly been passed the left or right eye twice 
            // (i.e. the same eye)
            return false;

        thetaRadians = atan((float)opposite/adjacent);

        theta = thetaRadians * float(180/M_PI);

        // we now have the angle of the horizontal line connecting the two eyes
		// using this angle we can compute a triangle such that the hypotenuse will be
		// half its current length (i.e. half the intereye distance).
        // We proceed by then calculating the opposite and adjacent edges for this new
		// triangle such that we will be computing the x & y coordinates of this midpoint

        // first compute the adjacent
        adjacent = cos(thetaRadians) * float(intereyeDistance/2);

        // then compute the opposite
        opposite = tan(thetaRadians) * adjacent;

        // the X & Y midpoint coordinates can now be computed
        midpointX = rightEye.x + adjacent;
        midpointY = rightEye.y + opposite;

    }
    else
    {
        // Ha, the eyes are already at the same horizontal level, computing the mid point is easy
        theta = 0;
    
        midpointX = rightEye.x + float(intereyeDistance/2);
        midpointY = float(rightEye.y);
    }

	// Next we wish to determine if any part of the thumbnail will fall outside the bounds of the 
	// image. If it does then we don't like this thumbnail so much because it means some of
	// the face might be cut off/out. To determine this we will compute several (6) points around
	// the boundary of the thumbnail and determine if any of these fall outside the bounds of the 
	// image. Point P1 is on the line that connects the two eyes and is on the right of the person's
	// right eye. The diagram below shows the 6 points and the person's face in the middle:
	// P5-----------P3
	// |            |
	// |    ^   ^   |
	// |    '   '   |
	// P4     0     P1
	// |    =====   |
	// |            |
	// |            |
	// |            |
	// P6-----------P2

	Point P1, P2, P3, P4, P5, P6;
    float DistanceBesideEyes = besideEyeBuffer * intereyeDistance;
    if( leftEye.y == rightEye.y ) 
    {
        // if the eyes are at the same level this is easy!
        P1.x = leftEye.x + DistanceBesideEyes;
        P1.y = (float)leftEye.y;

        P4.x = rightEye.x - DistanceBesideEyes;
        P4.y = (float)rightEye.y;
    }
    else
    {
        // if not its harder but not too bad since we know the angle
        float changeInX = cos(thetaRadians) * DistanceBesideEyes;
        float changeInY = sin(thetaRadians) * DistanceBesideEyes;

        P1.x = leftEye.x + changeInX;
        P1.y = leftEye.y + changeInY;

        P4.x = rightEye.x - changeInX;
        P4.y = rightEye.y - changeInY;
    }

    if( (P1.x >= frame.cols) || (P1.y >= frame.rows) || (P1.x < 0) || (P1.y < 0) ||
        (P4.x >= frame.cols) || (P4.y >= frame.rows) || (P4.x < 0) || (P4.y < 0) )
        return false; // if P is outside the bounds of the image stop right now!

    // The other points P2, P3, P5 and P6 are now computed. These points are on the border of the rectangle
    // (please see the diagram)
    float DistanceAboveEye = currentAboveEyeBuf * intereyeDistance;// * float(betweenEyeDist/m_iRecognitionIntereyeDist);
    float DistanceBelowEye = currentBelowEyeBuf * intereyeDistance;
    
    float beta, gamma, psi; // we'll use greek letters for the angles we'll use in these calculations

    if( leftEye.y == rightEye.y )
    {
        // if the eyes are already horizontal this is always easy
        P3.x = P1.x;
        P3.y = P1.y - DistanceAboveEye;

        P2.x = P1.x;
        P2.y = P1.y + DistanceBelowEye;

        P5.x = P4.x;
        P5.y = P4.y - DistanceAboveEye;

        P6.x = P4.x;
        P6.y = P4.y + DistanceBelowEye;
    }
    else
    {
        // now it really starts to get complicated

        // FIRST COMPUTE P2
        // we can compute beta (the last angle in the right angle triangle containing theta) since all the angles in a
        // triangle sum to 180 degrees
        beta = 180 - abs(theta) - 90;

        // this enables us to compute psi which is the angle between P and P2 where the length of the side of the right angle
        // triangle formed by these two points together with a right angle is the length "DistanceBelowEye"
        // We know that beta + psi = 90 (since the line from the left eye to P is normal to the edge of the rectangle), therefore
        // psi = 90 - beta
        if(  theta < 0 )
            psi = 90 - abs(beta);
        else
            psi = beta;

        // now that we have psi we can compute the change in x and the change in y that takes us from P to P2 thereby finding
        // the point P2
        float deltaX = sin(psi * float(M_PI/180)) * DistanceBelowEye;
        float deltaY = cos(psi * float(M_PI/180)) * DistanceBelowEye;

        if( theta >= 0 )
            deltaX = -deltaX;

        P2.x = P1.x + deltaX;
        P2.y = P1.y + deltaY;

        // the sides are symmetric so add the same to the other side
        P6.x = P4.x + deltaX;
        P6.y = P4.y + deltaY;

        // NOW COMPUTE P3
        // gamma is the angle formed from the right angle triangle between P and P3, it can be computed as 90 - theta
        if(  theta < 0 )
            gamma = 90 - abs(theta);
        else
            gamma = theta;

        deltaX = cos(gamma * float(M_PI/180)) * DistanceAboveEye;
        deltaY = sin(gamma * float(M_PI/180)) * DistanceAboveEye;

        if( theta < 0 )
            deltaX = -deltaX;

        P3.x = P1.x + deltaX;
        P3.y = P1.y - deltaY; // its always minus

        P5.x = P4.x + deltaX;
        P5.y = P4.y - deltaY;
    }

    // now that we have the angle that we need in order to level the eyes we can establish whether the face actually
    // has enough space around it such that we could extract a thumbnail of the given size
    if( (P2.x >= frame.cols) || (P2.y >= frame.rows) || (P2.x < 0) || (P2.y < 0) ||
        (P3.x >= frame.cols) || (P3.y >= frame.rows) || (P3.x < 0) || (P3.y < 0) ||
        (P5.x >= frame.cols) || (P5.y >= frame.rows) || (P5.x < 0) || (P5.y < 0) ||
        (P6.x >= frame.cols) || (P6.y >= frame.rows) || (P6.x < 0) || (P6.y < 0) )
        return false; // if P2, P3, P5 or P6 are outside the bounds of the image stop

	Point2f center;
    center.x = midpointX;
    center.y = midpointY;

	Mat rotatedImg;

	Mat transformMatrix = getRotationMatrix2D(center, theta + rotationCorrection, 1.0);
	cv::warpAffine(frame, rotatedImg, transformMatrix, rotatedImg.size());
    
    // after the image has been rotated about the midpoint we know where the eyes are, they will be at the same level as
    // the midpoint and will still be spaced the same distance from the midpoint
    leftEye.x = cvRound(midpointX + (intereyeDistance/2));
    leftEye.y = cvRound(midpointY);

    rightEye.x = cvRound(midpointX - (intereyeDistance/2));
    rightEye.y = cvRound(midpointY);

	Point midpoint = Point(midpointX, midpointY);

	// first render the 6 points we use as the border points in addition to points for the 2 eyes and the mid point just
	// for visualization purposes
	Mat frameCopy = frame.clone();
	circle( frameCopy, leftEye, 2, Scalar(255,0,0) ); // left eye
	circle( frameCopy, rightEye, 2, Scalar(255,0,0) ); // right eye
	circle( frameCopy, midpoint, 2, Scalar(255,0,0) ); // mid point between the eyes
	circle( frameCopy, P1, 2, Scalar(0,255,0) ); // point P1
	circle( frameCopy, P2, 2, Scalar(0,255,0) ); // point P2
	circle( frameCopy, P3, 2, Scalar(0,255,0) ); // point P3
	circle( frameCopy, P4, 2, Scalar(0,255,0) ); // point P4
	circle( frameCopy, P5, 2, Scalar(0,255,0) ); // point P5
	circle( frameCopy, P6, 2, Scalar(0,255,0) ); // point P6
	line( frameCopy, P5, P6, Scalar(0,255,0) ); // left hand edge line from P5 -> P6
	line( frameCopy, P5, P3, Scalar(0,255,0) ); // top edge line from P5 -> P3
	line( frameCopy, P3, P2, Scalar(0,255,0) ); // left hand edge line from P3 -> P2
	line( frameCopy, P2, P6, Scalar(0,255,0) ); // left hand edge line from P2 -> P6
	
	// end of visualization

	// as a final step we must compute the final cropping rectangle which will essentially
	// be the same region as that defined by the points P1 -> P6, except those coordinates
	// were in the unrotated image, we now want to find the same coordinates in the rotated
	// image
	Mat rotatedCroppedImg = rotatedImg(Rect(rightEye.x - DistanceBesideEyes, rightEye.y - DistanceAboveEye, intereyeDistance + DistanceBesideEyes*2, DistanceAboveEye + DistanceBelowEye));

	/*namedWindow("rotated_thumb_region");
	imshow("rotated_thumb_region", frameCopy);
	namedWindow("rotated_thumb");
	imshow("rotated_thumb", rotatedImg);
	namedWindow("rotated_cropped_thumb");
	imshow("rotated_cropped_thumb", rotatedCroppedImg);
	waitKey(0);
	destroyWindow("rotated_thumb_region");
	destroyWindow("rotated_cropped_thumb");
	destroyWindow("rotated_thumb");*/

    return true;
}

// Ensures that the rect passed in is valid based on the image size it is supposedly from. Returns
// a rect that is sure to be inside the bounds of the image
Rect Thumbnail::ConstrainRect(const Rect &rectToConstrain, const Size &imageSize)
{
	Rect constrainedRect = rectToConstrain;

	if( rectToConstrain.x < 0 )
	{
		// the left edge of the rect is beyond the left edge of the image
		constrainedRect.width += rectToConstrain.x; // will basically subtract this amount from the width to ensure the right edge of the rect stays in the same place
		constrainedRect.x = 0; // because we are going to add it here
	}

	if( rectToConstrain.y < 0 )
	{
		// the top edge of the rect is beyond the top edge of the image
		constrainedRect.height += rectToConstrain.y; // will basically subtract this amount from the height to ensure the bottom edge of the rect stays in the same place
		constrainedRect.y = 0; // because we are going to add it here
	}

	if( (rectToConstrain.x + rectToConstrain.width) > imageSize.width )
	{
		// the right hand edge of the rect goes beyond the edge of the image... crop the right edge so it falls on the edge of the image instead
		int rightEdgeDifference = (rectToConstrain.x + rectToConstrain.width) - imageSize.width;
		constrainedRect.width -= rightEdgeDifference;
	}

	if( (rectToConstrain.y + rectToConstrain.height) > imageSize.height )
	{
		// the bottom hand edge of the rect goes beyond the bottom edge of the image... crop the bottom edge so it falls on the edge of the image instead
		int bottomEdgeDifference = (rectToConstrain.y + rectToConstrain.height) - imageSize.height;
		constrainedRect.height -= bottomEdgeDifference;
	}

	return constrainedRect;
}


	}
}