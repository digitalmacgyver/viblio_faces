# Location of Neurotec SDK
NEUROTEC ?= /opt/Neurotec_SDK

# Root of where we will deploy this tool
# and its dependencies
DEPLOY ?= /deploy/vatools

TEST_ARGS ?= --analyzers FaceAnalysis --discarded_tracker_frequency 85000 --maximum_concurrent_trackers 10 --face_detector orbeus --orbeus_api_key zdN9xO1srMEFoEsq --orbeus_secret_key bvi5Li9bcQPE3W5S --orbeus_namespace fd_test_2 --orbeus_user_id test

#TEST_ARGS ?= --analyzers FaceAnalysis --discarded_tracker_frequency 5000 --maximum_concurrent_trackers 10 

# Architecture of the linux we are building for
ARCH=$(shell uname -i)
ifeq ($(ARCH),i386)
  ARCH=x86
endif
NEUROTEC_ARCH=Linux_$(ARCH)

MAKEARGS="NEUROTEC=$(NEUROTEC)" \
	"NEUROTEC_ARCH=$(NEUROTEC_ARCH)" \
	"DEPLOY=$(DEPLOY)"

all : cvblobs tld mftracker analytics FileSystem VideoSource jpeg-compressor RekognitionApi viblio package

cvblobs : 
	( cd OpenTLD/src/3rdparty/cvblobs ; make )

tld : 
	( cd OpenTLD/src/libopentld/tld ; make )

mftracker : 
	( cd OpenTLD/src/libopentld/mftracker ; make )

analytics : 
	( cd src/Libraries/Analytics.FaceAnalyzer ; make $(MAKEARGS) )

FileSystem : 
	( cd src/Libraries/FileSystem ; make )

VideoSource :
	( cd src/Libraries/VideoSource ; make )

jpeg-compressor :
	( cd src/Libraries/jpeg-compressor ; make )

RekognitionApi :
	( cd src/Libraries/RekognitionApi ; make )

viblio : 
	( cd src/apps/viblio_video_analyzer; make $(MAKEARGS) )

clean : 
	( cd OpenTLD/src/3rdparty/cvblobs ; make clean )
	( cd OpenTLD/src/libopentld/tld ; make clean )
	( cd OpenTLD/src/libopentld/mftracker ; make clean )
	( cd src/Libraries/Analytics.FaceAnalyzer ; make clean )
	( cd src/Libraries/FileSystem ; make clean )
	( cd src/Libraries/VideoSource ; make clean )
	( cd src/Libraries/RekognitionApi ; make clean )
	( cd src/Libraries/jpeg-compressor ; make clean )
	( cd src/apps/viblio_video_analyzer; make clean )
	$(RM) -rf _package
	$(RM) -f package.tar.gz

test: testfast
	( LD_LIBRARY_PATH=_package/lib ; _package/bin/viblio_video_analyzer -f test/test_videos/many-moving.mp4 --face_thumbnail_path test/output/many-moving --filename_prefix test $(TEST_ARGS) )
	( LD_LIBRARY_PATH=_package/lib ; _package/bin/viblio_video_analyzer -f test/test_videos/many-static.mp4 --face_thumbnail_path test/output/many-static --filename_prefix test $(TEST_ARGS) )
	( ./test/generate_html.py --base_json test/baseline-output/many-moving/test.json --new_json test/output/many-moving/test.json --output_filename test-many-moving.html )
	( ./test/generate_html.py --base_json test/baseline-output/many-static/test.json --new_json test/output/many-static/test.json --output_filename test-many-static.html )
	echo "View test-many-moving.html and test-many-static.html"

testfast: all .FORCE
	( LD_LIBRARY_PATH=_package/lib ; _package/bin/viblio_video_analyzer -f test/test_videos/no-face.mp4 --face_thumbnail_path test/output/no-face --filename_prefix test $(TEST_ARGS) )
	( LD_LIBRARY_PATH=_package/lib ; _package/bin/viblio_video_analyzer -f test/test_videos/two-faces.avi --face_thumbnail_path test/output/two-faces --filename_prefix test $(TEST_ARGS) )
	( LD_LIBRARY_PATH=_package/lib ; _package/bin/viblio_video_analyzer -f test/test_videos/two-faces-track-test.mp4 --face_thumbnail_path test/output/two-faces-track-test --filename_prefix test $(TEST_ARGS) )
	( LD_LIBRARY_PATH=_package/lib ; _package/bin/viblio_video_analyzer -f test/test_videos/three-faces-track-test.mp4 --face_thumbnail_path test/output/three-faces-track-test --filename_prefix test $(TEST_ARGS) )
	( LD_LIBRARY_PATH=_package/lib ; _package/bin/viblio_video_analyzer -f test/test_videos/few-faces.flv --face_thumbnail_path test/output/few-faces --filename_prefix test $(TEST_ARGS) )
	( LD_LIBRARY_PATH=_package/lib ; _package/bin/viblio_video_analyzer -f test/test_videos/many-short.mp4 --face_thumbnail_path test/output/many-short --filename_prefix test $(TEST_ARGS) )
	( ./test/generate_html.py --base_json test/baseline-output/no-face/test.json --new_json test/output/no-face/test.json --output_filename test-no-face.html )
	( ./test/generate_html.py --base_json test/baseline-output/two-faces/test.json --new_json test/output/two-faces/test.json --output_filename test-two-faces.html )
	( ./test/generate_html.py --base_json test/baseline-output/two-faces-track-test/test.json --new_json test/output/two-faces-track-test/test.json --output_filename test-two-faces-track-test.html )
	( ./test/generate_html.py --base_json test/baseline-output/three-faces-track-test/test.json --new_json test/output/three-faces-track-test/test.json --output_filename test-three-faces-track-test.html )
	( ./test/generate_html.py --base_json test/baseline-output/few-faces/test.json --new_json test/output/few-faces/test.json --output_filename test-few-faces.html )
	( ./test/generate_html.py --base_json test/baseline-output/many-short/test.json --new_json test/output/many-short/test.json --output_filename test-many-short.html )
	echo "View test-no-face.html test-two-faces.html test-two-faces-track-test.html test-three-faces-track-test.html test-few-faces.html test-many-short.html"

.FORCE:

install_linux_deps:
	apt-get -y install libopencv-dev
	apt-get -y install libcurl4-openssl-dev
	/usr/local/bin/check-and-install-software.pl -db staging -app Neurotec_SDK

package:
	$(RM) -rf _package
	mkdir -p _package/bin
	mkdir -p _package/lib
	mkdir -p _package/license

	cp src/apps/viblio_video_analyzer/viblio_video_analyzer _package/bin
	cp -r $(NEUROTEC)/Lib/$(NEUROTEC_ARCH)/* _package/lib
	cp $(NEUROTEC)/Bin/$(NEUROTEC_ARCH)/Activation/* _package/license

	( cd _package; tar zcf ../packaging/package.tar.gz . )
	( cd packaging; tar zcf ../package.tar.gz . ; rm package.tar.gz )

bump:
	upgrade.pl -db staging -app vatools -f package.tar.gz -bump
