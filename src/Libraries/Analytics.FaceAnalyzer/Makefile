SOURCES=\
EyeDetector_OpenCV.cpp\
FaceAnalyzerConfiguration.cpp\
FaceAnalyzer.cpp\
Face.cpp\
FaceDetector.cpp\
FaceDetector_OpenCV.cpp\
FaceDetector_Neurotech.cpp\
FaceDetector_Orbeus.cpp\
Tracker_OpenTLD.cpp\
TrackingController.cpp\
FaceDetectionDetails.cpp\
Frame.cpp\
ThumbnailDetails.cpp\
../Jzon/Jzon.cpp\
Thumbnail.cpp

OBJS=$(SOURCES:.cpp=.o)
CFLAGS=-std=c++0x -I../ -I../../../OpenTLD/src/libopentld/tld/ -I$(NEUROTEC)/Include/ -L$(NEUROTEC)/Lib/$(NEUROTEC_ARCH) -DBOOST_LOG_DYN_LINK
LIBNAME=libFaceAnalyzer.a

%.o : %.cpp
	g++ -c $(CFLAGS) $(DEBUG) $(OPTIMIZE) $< -o $@

$(LIBNAME) : $(OBJS)
	ar rcs $@ $^

clean :
	rm -rf $(LIBNAME) $(OBJS)

