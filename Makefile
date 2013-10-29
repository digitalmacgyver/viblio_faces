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

all : cvblobs tld mftracker analytics FileSystem VideoSource viblio package

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

viblio : 
	( cd src/apps/viblio_video_analyzer; make $(MAKEARGS) )

clean : 
	( cd OpenTLD/src/3rdparty/cvblobs ; make clean )
	( cd OpenTLD/src/libopentld/tld ; make clean )
	( cd OpenTLD/src/libopentld/mftracker ; make clean )
	( cd src/Libraries/Analytics.FaceAnalyzer ; make clean )
	( cd src/Libraries/FileSystem ; make clean )
	( cd src/Libraries/VideoSource ; make clean )
	( cd src/apps/viblio_video_analyzer; make clean )
	$(RM) -rf _package
	$(RM) -f package.tar.gz

package:
	$(RM) -rf _package
	mkdir -p _package/bin
	mkdir -p _package/lib
	mkdir -p _package/license

	cp src/apps/viblio_video_analyzer/viblio_video_analyzer _package/bin
	cp -r $(NEUROTEC)/Lib/$(NEUROTEC_ARCH)/* _package/lib
	cp $(NEUROTEC)/Bin/$(NEUROTEC_ARCH)/Activation/* _package/license

	( cd _package; tar zcf ../package.tar.gz . )
	@echo
	@echo "To deploy, copy package.tar.gz to /tmp on the target machine, then"
	@echo "cd $(DEPLOY); tar zxvf /tmp/package.tar.gz"
	@echo
