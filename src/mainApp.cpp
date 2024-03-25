
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "mainApp.h"
#include "offscreenRenderApp.h"
#include <GL/glew.h>
#ifdef WIN32
#include <windows.h>
#include <GL/wglew.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void createOffscreenRenderAppThread(void* data)
{
	MainApp& mainApp = *(static_cast<MainApp*>(data));
	OffscreenRenderApp app(mainApp.videoMode, mainApp.sharedShutterState);
    app.run();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MainApp::MainApp(const sf::VideoMode& videoMode, const std::string& title)
	: App(videoMode, title)
	, videoMode(videoMode)
	, sharedShutterState(videoMode.width, videoMode.height)
	, subThread(&createOffscreenRenderAppThread, this)
	, shutterGlasses()
	, flipEyes(false)
{
	subThread.launch();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainApp::processEvent(sf::Event event)
{
    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::Escape)
			exit(0);

		if (event.key.code == sf::Keyboard::F1)
		{
			sharedShutterState.lock();
			sharedShutterState.stereo = !sharedShutterState.stereo;
			sharedShutterState.unlock();
		}

		if (event.key.code == sf::Keyboard::F2)
			flipEyes = !flipEyes;

		if (event.key.code == sf::Keyboard::F3)
			shutterGlasses.nextRefreshRate();

		return;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainApp::update(float dt, const sf::Keyboard& input)
{
	sharedShutterState.lock();

	if (input.isKeyPressed(sf::Keyboard::LShift) || input.isKeyPressed(sf::Keyboard::RShift))
	{
		if (input.isKeyPressed(sf::Keyboard::F5))	sharedShutterState.convergence -= 0.01f;
		if (input.isKeyPressed(sf::Keyboard::F6))	sharedShutterState.separation -= 0.01f;
	}
	else
	{
		if (input.isKeyPressed(sf::Keyboard::F5))	sharedShutterState.convergence += 0.01f;
		if (input.isKeyPressed(sf::Keyboard::F6))	sharedShutterState.separation += 0.01f;
	}

	sharedShutterState.unlock();

	shutterGlasses.toggleEyes( (int)0xffff0000 );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MainApp::render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	sharedShutterState.lock();
	sharedShutterState.getCurrentFbo(shutterGlasses.isLeftEye() ? !flipEyes : flipEyes).bind();
	sharedShutterState.unlock();

	const float fullscreenQuadTexCoords[] = { 0.0f, 0.0f,  1.0f, 0.0f,  1.0f, 1.0f,  0.0f, 1.0f };
	const float fullscreenQuadVertices[]  = { -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,   1.0f, 1.0f, -1.0f,  -1.0f, 1.0f, -1.0f };

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		glTexCoordPointer(2, GL_FLOAT, 0, fullscreenQuadTexCoords);
		glVertexPointer(3, GL_FLOAT, 0, fullscreenQuadVertices);
		glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	window.display();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////