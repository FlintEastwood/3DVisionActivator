
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
using namespace std; 
#include "offscreenRenderApp.h"
#include <GL/glew.h>
#ifdef WIN32
#include <windows.h>
#include <GL/wglew.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

OffscreenRenderApp::OffscreenRenderApp(const sf::VideoMode& videoMode, SharedShutterState& sharedShutterState)
	: App(videoMode, "")
	, sharedShutterState(sharedShutterState)
	, yRotation(0.0f)
{
	window.setVerticalSyncEnabled(false);
	window.setVisible(false);
	window.setFramerateLimit(120);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OffscreenRenderApp::processEvent(sf::Event event)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OffscreenRenderApp::update(float dt, const sf::Keyboard& input)
{
	/*
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { (SHORT)0, (SHORT)20 });
	cout << "Offscreen Frametime: " << dt << "       \n"
	     << "Offscreen FPS: " << (1/dt) << "         \n"
		 << "                                        \n"
		 << "                                        \n";
	*/
	yRotation += 75.0f * dt;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OffscreenRenderApp::render()
{
	sharedShutterState.lock();
	Fbo& currentFbo = sharedShutterState.getCurrentFbo(shutterGlasses.isLeftEye());
	currentFbo.bindFrameBuffer();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	/*
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { (SHORT)0, (SHORT)20 });
	cout << "Separation : " << sharedShutterState.separation << "         \n"
		 << "Convergence: " << sharedShutterState.convergence << "         \n"
		 << "                                        \n"
		 << "                                        \n";
	*/
	if (sharedShutterState.stereo)
	{
		bool swap = shutterGlasses.isLeftEye();
		shutterGlasses.toggleEyes();

		//glRotatef((swap ? -1.0f : 1.0f) * (sharedShutterState.convergence * sharedShutterState.separation), 0.0f, 1.0f, 0.0f); // Toe in for convergence ?
		glTranslatef((swap ? -1.0f : 1.0f) * sharedShutterState.separation, 0.0, 0.0f);
	}
	
	glTranslatef(0.0f, 0.12f, -4.2f);
	glRotatef(45.0f, 1.0f, 0.0f, 0.0f);
	//glRotatef(45.0f + yRotation, 0.0f, 1.0f, 0.0f);
	glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
	
	const float cubeVertices[] = {	-1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,
									-1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
									-1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,
									-1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
									 1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,
									-1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f		};

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, cubeVertices);

		glColor3f(1.0f, 0.8f, 0.8f);	glDrawArrays(GL_QUADS, 0, 8); //Rd
		//glColor3f(1.0f, 1.0f, 0.0f);	glDrawArrays(GL_QUADS, 4, 4); //Ye
		glColor3f(0.8f, 1.0f, 0.8f);	glDrawArrays(GL_QUADS, 8, 8); //Gn
		//glColor3f(0.0f, 1.0f, 1.0f);	glDrawArrays(GL_QUADS,12, 4); //turq
		glColor3f(0.8f, 0.8f, 1.0f);	glDrawArrays(GL_QUADS,16, 8); //bl
		//glColor3f(1.0f, 0.0f, 1.0f);	glDrawArrays(GL_QUADS,20, 4); //vio

	glDisableClientState(GL_VERTEX_ARRAY);

	currentFbo.unbindFrameBuffer();
	sharedShutterState.unlock();

	window.display();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
