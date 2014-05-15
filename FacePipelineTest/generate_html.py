from __future__ import print_function
import json
from pprint import pprint
import markup
import argparse
import os
import sys

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('-base_json', action='store', dest='base_json', help='base json file that contains tracks')
    parser.add_argument('-new_json', action='store', dest='new_json', help='new json file that contains tracks')
    parser.add_argument('-output_filename', action='store', dest='output', help='html output filename')
    results = parser.parse_args()

    #Some sanity checks to see if the json files passed as arguments exist on the disk.
    if not os.path.isfile(results.base_json):
        raise Exception('base_json file not found')
    if not os.path.isfile(results.new_json):
        raise Exception('new_json file not found')
    json_data1 = open(results.base_json)
    data1 = json.load(json_data1)

    json_data2 = open(results.new_json)
    data2 = json.load(json_data2)

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
            str = (data1["tracks"][x]["faces"][y]["s3_key"])
            images.append(os.path.dirname(os.path.realpath(results.base_json)) + '/' + str.split('/')[1])

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
            str = (data2["tracks"][x]["faces"][y]["s3_key"])
            images.append(os.path.dirname(os.path.realpath(results.new_json)) + '/' + str.split('/')[1])
        name = (data2["tracks"][x]["track_id"])
        page.div(class_=name)
        page.img(width=100, height=100, src=images, class_=name)
        page.div.close()
    page.div.close()

    # Saving the output in a html file
    f = open(results.output, "w")
    print(page, file=f)
    f.close()
    json_data1.close()
    json_data2.close()

