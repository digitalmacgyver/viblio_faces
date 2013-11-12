SOURCES=\
EyeDetector_OpenCV.cpp\
FaceAnalyzerConfiguration.cpp\
FaceAnalyzer.cpp\
Face.cpp\
FaceDetector_OpenCV.cpp\
FaceDetector_Neurotech.cpp\
Tracker_OpenTLD.cpp\
TrackingController.cpp\
FaceDetectionDetails.cpp\
ThumbnailDetails.cpp\
../Jzon/Jzon.cpp\
Thumbnail.cpp

OBJS=$(SOURCES:.cpp=.o)
CFLAGS=-std=c++0x -I../ -I../../../OpenTLD/src/libopentld/tld/ -I$(NEUROTEC)/Include/ -L$(NEUROTEC)/Lib/$(NEUROTEC_ARCH)
LIBNAME=libFaceAnalyzer.a

%.o : %.cpp
	g++ -c $(CFLAGS) $< -o $@

$(LIBNAME) : $(OBJS)
	ar rcs $@ $^

clean :
	rm -rf $(LIBNAME) $(OBJS)

