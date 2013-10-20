#include <stdio.h>
#include <stdlib.h>

#include <NCore.h>
#include <NImages.h>
#include <NLExtractor.h>
#include <NLicensing.h>

#include <TutorialUtils.h>

const NChar title[] = N_T("DetectFacialFeatures");
const NChar description[] = N_T("Demonstrates detection of face and facial features in the image.");
const NChar version[] = N_T("4.5.0.0");
const NChar copyright[] = N_T("Copyright (C) 2006-2013 Neurotechnology");

int usage()
{
	printf(N_T("usage:\n"));
	printf(N_T("\t%s [image]\n"), title);
	printf(N_T("\n"));
	printf(N_T("\t[image] - filename of image.\n"));
	
	return -1;
}

void printNleFeaturePoint(const NChar * szName, NLFeaturePoint * pPoint)
{
	if (pPoint->Confidence == 0)
	{
		printf(N_T("\t\t%s feature unavailable. confidence: 0\n"), szName);
		return;
	}
	printf(N_T("\t\t%s feature found. X: %d, Y: %d, confidence: %d\n"), szName, pPoint->X, pPoint->Y, pPoint->Confidence);
}

void printBaseFeaturePoint(NLFeaturePoint * pPoint)
{
	if (pPoint->Confidence == 0)
	{
		printf(N_T("\t\tbase feature point unavailable. confidence: 0\n"));
		return;
	}
	printf(N_T("\t\tbase feature point found. X: %d, Y: %d, confidence: %d, code: %d\n"), pPoint->X, pPoint->Y, pPoint->Confidence, pPoint->Code);
}

int main(int argc, NChar **argv)
//int main(int argc, char **argv)
{
	NResult result = N_OK;
	HNLExtractor extractor = NULL;
	HNLTemplate tmpl = NULL;
	NleExtractionStatus status;

	HNImage image = NULL, grayscale = NULL;

	int faceCount;
	NleFace *faces = NULL;
	NleDetectionDetails details;
	int i, j;
	const NChar * components = N_T("Biometrics.FaceDetection,Biometrics.FaceExtraction");
	NBool available = NFalse;
	NBool detectAllFeatures = NTrue;
	NBool detectBaseFeatures = NTrue;
	NBool detectGender = NTrue;
	NBool detectExpression = NTrue;
	NBool detectMouthOpen = NTrue;
	NBool detectBlink = NTrue;
	NBool detectGlasses = NTrue;
	NBool detectDarkGlasses = NTrue;
	NleTemplateSize templateSize = nletsSmall;

	OnStart(title, description, version, copyright, argc, argv);

	if (argc < 2)
	{
		OnExit();
		return usage();
	}

	// check the license first
	result = NLicenseObtainComponents(N_T("/local"), N_T("5000"), components, &available);
	if (NFailed(result))
	{
		PrintErrorMsg(N_T("NLicenseObtainComponents() failed, result = %d\n"), result);
		goto FINALLY;
	}

	if (!available)
	{
		printf(N_T("Licenses for %s not available\n"), components);
		result = N_E_FAILED;
		goto FINALLY;
	}

	// read image
	
	result = NImageCreateFromFileEx(argv[1], NULL, 0, NULL, &image);
	if (NFailed(result))
	{
		PrintErrorMsg(N_T("NImageCreateFromFile() failed (result = %d)!"), result);
		goto FINALLY;
	}

	// convert image to grayscale
	result = NImageToGrayscale(image, &grayscale);
	if (NFailed(result))
	{
		PrintErrorMsg(N_T("NImageToGrayscale() failed (result = %d)!"), result);
		goto FINALLY;
	}

	NObjectFree(image);
	image = NULL;

	// create an extractor
	result = NleCreate(&extractor);
	if (NFailed(result))
	{
		PrintErrorMsg(N_T("NleCreate() failed (result = %d)!"), result);
		goto FINALLY;
	}

	result = NObjectSetParameterEx(extractor, NLEP_DETECT_ALL_FEATURE_POINTS, -1, &detectAllFeatures, sizeof(NBool));
	if (NFailed(result))
	{
		PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
		goto FINALLY;
	}

	result = NObjectSetParameterEx(extractor, NLEP_DETECT_BASE_FEATURE_POINTS, -1, &detectBaseFeatures, sizeof(NBool));
	if (NFailed(result))
	{
		PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
		goto FINALLY;
	}

	result = NObjectSetParameterEx(extractor, NLEP_DETECT_GENDER, -1, &detectGender, sizeof(NBool));
	if (NFailed(result))
	{
		PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
		goto FINALLY;
	}

	result = NObjectSetParameterEx(extractor, NLEP_DETECT_EXPRESSION, -1, &detectExpression, sizeof(NBool));
	if (NFailed(result))
	{
		PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
		goto FINALLY;
	}

	result = NObjectSetParameterEx(extractor, NLEP_DETECT_BLINK, -1, &detectBlink, sizeof(NBool));
	if (NFailed(result))
	{
		PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
		goto FINALLY;
	}

	result = NObjectSetParameterEx(extractor, NLEP_DETECT_MOUTH_OPEN, -1, &detectMouthOpen, sizeof(NBool));
	if (NFailed(result))
	{
		PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
		goto FINALLY;
	}

	result = NObjectSetParameterEx(extractor, NLEP_DETECT_GLASSES, -1, &detectGlasses, sizeof(NBool));
	if (NFailed(result))
	{
		PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
		goto FINALLY;
	}

	result = NObjectSetParameterEx(extractor, NLEP_DETECT_DARK_GLASSES, -1, &detectDarkGlasses, sizeof(NBool));
	if (NFailed(result))
	{
		PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
		goto FINALLY;
	}

	result = NObjectSetParameterEx(extractor, NLEP_TEMPLATE_SIZE, -1, &templateSize, sizeof(NleTemplateSize));
	if (NFailed(result))
	{
		PrintErrorMsg(N_T("NObjectSetParameter() failed (result = %d)!"), result);
		goto FINALLY;
	}

	// detect all faces that are suitable for face recognition in the image
	result = NleDetectFaces(extractor, grayscale, &faceCount, &faces);
	if (NFailed(result))
	{
		PrintErrorMsg(N_T("NleDetectFaces() failed (result = %d)!"), result);
		goto FINALLY;
	}

	printf(N_T("found faces:\n"));
	for (i = 0; i < faceCount; ++i)
	{
		printf(N_T("\tlocation = (%d, %d), width = %d, height = %d, confidence = %.2f\n"),
				faces[i].Rectangle.X, faces[i].Rectangle.Y,
				faces[i].Rectangle.Width, faces[i].Rectangle.Height,
				faces[i].Confidence);

		// detect features for current face
		result = NleDetectFacialFeatures(extractor, grayscale, &faces[i], &details);
		if (NFailed(result))
		{
			PrintErrorMsg(N_T("NleDetectFacialFeatures() failed (result = %d), maybe feature points were not found!"), result);
			continue;
		}

		printNleFeaturePoint(N_T("LeftEyeCenter"), &details.LeftEyeCenter);
		printNleFeaturePoint(N_T("RightEyeCenter"), &details.RightEyeCenter);
		printNleFeaturePoint(N_T("MouthCenter"), &details.MouthCenter);
		printNleFeaturePoint(N_T("NoseTip"), &details.NoseTip);

		printf(N_T("\n"));
		for(j = 0; j < NLE_FEATURE_POINT_COUNT; j++)
		{
			printBaseFeaturePoint(&details.Points[j]);
		}

		// gender, expression and other proeprties only detected during extraction
		result = NleExtractUsingDetails(extractor, grayscale, &details, &status, &tmpl);
		if(NFailed(result))
		{
			PrintErrorMsg(N_T("NleExtractUsingDetails() failed (result = %d)"), result);
			continue;
		}
		NObjectFree(tmpl); tmpl = NULL;
		printf(N_T("\n"));
		printf(N_T("\t\textraction status: %d\n"), status);
		
		if(details.GenderConfidence == 255) printf(N_T("\t\tgender not detected\n"));
		printf(N_T("\t\tgender: %s, confidence: %d\n"), (details.Gender == ngMale ? N_T("Male") : N_T("Female")), details.GenderConfidence);
		if (details.ExpressionConfidence == 255) printf(N_T("\t\texpression not detected\n"));
		else if (details.Expression == nleSmile) printf(N_T("\t\texpression: smile, confidence: %d\n"), details.GenderConfidence);
		if (details.BlinkConfidence == 255) printf(N_T("\t\tblink not detected\n"));
		else printf(N_T("\t\tblink: %s, confidence: %d\n"), (details.Properties & nlpBlink) == nlpBlink ? N_T("Yes") : N_T("No"), details.BlinkConfidence);
		if (details.MouthOpenConfidence == 255) printf(N_T("\t\tmouth open not detected\n"));
		else printf(N_T("\t\tmouth open: %s, confidence: %d\n"), (details.Properties & nlpMouthOpen) == nlpMouthOpen ? N_T("Yes") : N_T("No"), details.MouthOpenConfidence);
		if (details.GlassesConfidence == 255) printf(N_T("\t\tglasses not detected\n"));
		else printf(N_T("\t\tglasses: %s, confidence: %d\n"), (details.Properties & nlpGlasses) == nlpGlasses ? N_T("Yes") : N_T("No"), details.GlassesConfidence);
		if (details.DarkGlassesConfidence == 255) printf(N_T("\t\tdark glasses not detected\n"));
		else printf(N_T("\t\tdark glasses: %s, confidence: %d\n"), (details.Properties & nlpDarkGlasses) == nlpDarkGlasses ? N_T("Yes") : N_T("No"), details.DarkGlassesConfidence);
	}

	result = N_OK;
FINALLY:
	NObjectFree(image);
	NObjectFree(grayscale);
	NObjectFree(extractor);
	NObjectFree(tmpl);
	NFree(faces);

	{
		NResult result2 = NLicenseReleaseComponents(components);
		if (NFailed(result2))
		{
			PrintErrorMsg(N_T("NLicenseReleaseComponents() failed, result = %d\n"), result2);
		}
	}

	OnExit();
	return result;
}
