#Viblio Face Detector Test Instructions

#The goal :
To compare any new update to face detection code with the previous/baseline version. We visualize the face tracks output from a test video(test.mp4) by the new version and the previous baseline version of the face detection code.

#Instructions:

Run the file "generate_html.py" to  generate a html file that visulaizes baseline and new tracks side by side. <br/>
The arguments are explained below: <br/>
```
-base_json           :    The json file containing baseline tracks. Note that all the image files(tracks) should be present in the same folder that contains this baseline json file. The folder "Baseline_Tracks" serves as an example. 
-new_json            :    The json file containing new tracks. Note that all the image files(tracks) should be present in teh same folder that contains the new tracks. The folder "New_Tracks" serves as an example. 
-output              :    The html filename with path where the visualization result is stored. 
```

Running the code -


    ```
    $generate_html.py -base_json ./Baseline_Tracks/baseline_tracks.json -new_json ./New_Tracks/new_tracks.json -output resultsFaceDetection.html
    ```

Note :   Everytime we update the face detection code, we need to run it on the test video (test.mp4) that generates a json file along with tracks (eg: "New_Tracks" folder). Then this can be passed in place of new_tracks.json file above to visualize the new results.
