# 3DVisionActivator
Program to drive the Nvidia 3D Vision Shutter glasses without the Nvidia 3D Vision driver.

Dependencies:
- Visual Studio 2022
- GLEW 2.2.0
- SFML 2.6.1

Known Bugs:
- When app is closed with the "X", the Offscreen Thread will stay active in background - Please close with "ESC" key for now!
- When Nvidia IR emitter goes to sleep mode and the app is started, Win10 will crash with the BSOD "SYSTEM_SERVICE_EXEPTION" "ucx01000.sys" !
- Sync to Monitor refresh is still not working!
