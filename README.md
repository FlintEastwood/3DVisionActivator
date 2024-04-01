# 3DVisionActivator
Program to drive the Nvidia 3D Vision Shutter glasses without the Nvidia 3D Vision driver.
Initially made by "cms": https://www.mtbs3d.com/phpbb/viewtopic.php?p=23830#p23830

### Keys:
- F1 - toggles stereo
- F2 - swaps eyes
- F3 - next Monitor profile(or loops to the first)
- ~~Shift+F5/+F6 - modifies the convergence~~
- F5 / F6 - modifies the separation
- X / S - modifies the x timing (Delay from Monitor-Refresh-Start to Shutter open ??)
- Y / A - modifies the y timing (Shutter open time)
- W / Q - modifies the w timing (No effect??)
- ESC - Close App

## !!! Don't start the app when IR-emitter is in sleep-mode !!!

### How it works:
There are two rendering contexts (in separate threads).
The offscreen thread is not bound by vsync and continually renders the scene to a set of frame buffer objects (one for each eye).
The visible context is bound by vsync and has access to these fbos.
It renders the appropriate eye fbo to the window and toggles the 3dVision's "eye."
The IR-emitter Init sequence is now borrowed from "libnvstusb" and works really good.
A file named "MonitorTimings.ini" holds the Timing profiles. The program will initially use the top profile. (You may want to change this)
If you have a 3D-Vision certified Monitor, you can find this timings with NvTimingsEd. The EDID_ID can be found with CRU (Custom Resolution Utility).

### Dependencies:
- Visual Studio 2022
- GLEW 2.2.0
- SFML 2.6.1

### Known Bugs:
- FIXED: ~~When app is closed with the "X", the Offscreen Thread will stay active in background - Please close with "ESC" key for now!~~
- When Nvidia IR emitter goes to sleep-mode and the app is started, Win10 will crash with the BSOD "SYSTEM_SERVICE_EXEPTION" "ucx01000.sys" ! (Workaround is to unplug and plug the IR emitter)
- When the IR emitter is woken up from sleep-mode by another 3D-Vision app (NvStTest.exe for example) and the app is started, it hangs on initializing the IR emitter. (Workaround is to unplug and plug the IR emitter)
- FIXED! ~~Sync to Monitor refresh is still not working!~~
- The cube sometimes flickers - mostly with a lot of background activity- FBO flipping seems to miss the VSync sporadically.
