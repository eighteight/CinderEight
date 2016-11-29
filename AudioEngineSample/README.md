#AudioEngineSample
##FMOD and RTMIDI wrapper for Cinder

Features:

- multi-channel line input recording
- multi-channel audio sample playback with callbacks
- line input playback rate adjust
- midi input with event callbacks
- audio and midi device polling
- 3d positioning of listener and samples

overwrite settings/AudioEngine_Settings.xml with settings/AudioEngine_Settings - Empty.xml 
and run the app. You should see some playback and recording devices in the parameter window.
add settings tags for the devices you want to use. for example:

`
<playback>`
	`<device id="3" name="MOTU 1" playerposx="55" playerposy="0" playerposz = "0" playerlookatx="0" playerlookaty="0" playerlookatz="1"></device>`
	`<device id="5" name="MOTU 2" playerposx="55" playerposy="0" playerposz = "0" playerlookatx="0" playerlookaty="0" playerlookatz="1"></device>`
`</playback>`
`<recording>`
	`<device id="5" name="" ></device>`
`</recording>`
`<midi>`
	`<device id="2" name="nano Key" ></device>`
`</midi>`

add sounds to the assets directory and add tags to AudioEngine_Content.xml
