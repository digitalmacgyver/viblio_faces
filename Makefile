# Location of Neurotec SDK
NEUROTEC ?= /opt/Neurotec_SDK

# Root of where we will deploy this tool
# and its dependencies
DEPLOY ?= /deploy/vatools

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
	upgrade.pl -db staging -app vatools -f package.tar.gz
