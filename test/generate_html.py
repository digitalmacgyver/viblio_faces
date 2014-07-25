#!/usr/bin/env python

from __future__ import print_function

import codecs
import simplejson as json
from pprint import pprint
import markup
import argparse
import os
import sys

usage = '''
./generate_html.py --base_json path/to/base/base.json --new_json path/to/new/new.json --output_filename test.html

Looks in path/to/base for base.json, which is assumed to be the
baseline output of viblio_video_analyzer on a given video for
comparison.

Looks in path/to/new for new.json, which is assumed to be the output
of viblio_video_analyzer after some changes for comparison.

The output summary is placed in test.html
'''

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--base_json', action='store', dest='base_json', help='base json file that contains tracks')
    parser.add_argument('--new_json', action='store', dest='new_json', help='new json file that contains tracks')
    parser.add_argument('--output_filename', action='store', dest='output', help='html output filename')
    results = parser.parse_args()

    #Some sanity checks to see if the json files passed as arguments exist on the disk.
    if results.base_json is None or not os.path.isfile(results.base_json):
        print( usage )
        sys.exit( 1 )
    if results.new_json is None or not os.path.isfile(results.new_json):
        print( usage )
        sys.exit( 1 )
    json_data1 = open(results.base_json, 'r')
    data1 = json.load(json_data1)

    json_data2 = open(results.new_json, 'r')
    x = json_data2.read()
    data2 = json.loads( x )

    page = markup.page(case='upper')

    # Html page will be divided int two halves. The left half contains baseline tracks and right half contains new tracks.
    
    # Code for Baseline tracks (Left Half)
    page.div(style="background-color:green; width:50%; height:100%; float:left")
    page.h2("Baseline Tracks")
    page.h2.close()
    for x in range(0, len(data1["tracks"])):
        images = []
        for y in range(0, len(data1["tracks"][x]["faces"])):
            #pprint(data["tracks"][x]["faces"][y])
            image_name = (data1["tracks"][x]["faces"][y]["s3_key"])
            # Hack to put the files where I want them, this should be
            # a parameter.
            images.append( 'baseline-output/' + os.path.split( os.path.split( results.base_json )[0] )[-1] + '/' + image_name.split( '/' )[1] )

        name = (data1["tracks"][x]["track_id"])
        page.div(class_=name)
        page.img(width=100, height=100, src=images, class_=name)
        page.div.close()
    page.div.close()

    #Code for New tracks(Right Half)
    page.div(style="background-color:red; width:50%; height:100%; float:right")
    page.h2("New Tracks")
    page.h2.close()
    for x in range(0, len(data2["tracks"])):
        images = []
        for y in range(0, len(data2["tracks"][x]["faces"])):
            image_name = (data2["tracks"][x]["faces"][y]["s3_key"])
            # Hack
            images.append( 'output/' + os.path.split( os.path.split( results.new_json )[0] )[-1] + '/' + image_name.split( '/' )[1] )
        name = (data2["tracks"][x]["track_id"])
        page.div(class_=name)
        page.img(width=100, height=100, src=images, class_=name)
        page.div.close()
    page.div.close()

    # Saving the output in a html file
    # Hack
    f = open('test/'+results.output, "w")
    print(page, file=f)
    f.close()
    json_data1.close()
    json_data2.close()

