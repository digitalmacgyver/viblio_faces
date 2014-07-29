SOURCES=\
viblio_video_analyzer.cpp\
VideoProcessor.cpp\
../../Libraries/Jzon/Jzon.cpp

OBJS=$(SOURCES:.cpp=.o)
CFLAGS=-std=c++0x -I../../Libraries -I$(NEUROTEC)/Include/ -DBOOST_LOG_DYN_LINK
EXECNAME=viblio_video_analyzer

LD_OPTS=-Wl,-rpath=$(DEPLOY)/lib

%.o : %.cpp
	g++ -c $(CFLAGS) $(DEBUG) $(OPTIMIZE) $< -o $@

$(EXECNAME) : $(OBJS)
	g++ -std=c++0x $(OPTIMIZE) $(DEBUG) -o $@ $(OBJS) $(LD_OPTS) -L../../Libraries/FileSystem -L../../Libraries/VideoSource -L../../Libraries/Analytics.FaceAnalyzer/ -L../../Libraries/RekognitionApi/ -L../../Libraries/jpeg-compressor/ -L../../../../faces/OpenTLD/src/libopentld/tld/ -L../../../../faces/OpenTLD/src/3rdparty/cvblobs/ -L../../../../faces/OpenTLD/src/libopentld/mftracker -L$(NEUROTEC)/Lib/$(NEUROTEC_ARCH)/ -lVideoSource -lFaceAnalyzer -lFileSystem -lRekognitionApi -lJpegcompressor -lOpenTld -lopencv_imgproc -lopencv_core -lcvblobs -lmftracker -lopencv_highgui -lopencv_objdetect -lboost_program_options -lboost_system -lboost_filesystem -lboost_thread -lboost_date_time -lboost_log -lboost_log_setup -lopencv_video -lNCore -lNLicensing -lNBiometrics -lNMediaProc -lNMedia -lcurl

clean :
	rm -rf $(EXECNAME) $(OBJS)

