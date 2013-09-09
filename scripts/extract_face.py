#!/usr/bin/env python

import os
import sys
import shutil

video_file = sys.argv[1]
face_file = sys.argv[2]

work_dir = '/tmp/faces-' + str( os.getpid() )

os.makedirs( work_dir )

cmd = '/home/viblio/bin/viblio_video_analyzer -f %s --analyzers FaceAnalysis --face_detector_cascade_file /home/viblio/bin/haarcascade_frontalface_alt.xml --face_thumbnail_path %s' % ( video_file, work_dir )

os.system( cmd )

faces = os.listdir( work_dir )
if len(faces)!=0:
    sub=os.listdir(work_dir+"/"+faces[0])

if len( faces ) == 0:
        shutil.rmtree( work_dir )
        sys.exit( "No faces detected in this video: " + video_file )
else:
    shutil.copyfile( work_dir + '/' + faces[0]+'/'+sub[0], face_file )
    print work_dir + '/' + faces[0]+'/'+sub[0]
    shutil.rmtree( work_dir )

sys.exit( 0 )
