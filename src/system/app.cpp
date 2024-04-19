
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include "app.h"
#include <GL/glew.h>
#ifdef WIN32
#include <windows.h>
#include <GL/wglew.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

App::App(const sf::VideoMode& videoMode, const std::string& title)
	: window(videoMode, title, sf::Style::Default)                     //sf::Style::Close
    , fov(45.0f)
    , nearZ(0.1f)
    , farZ(100.0f)
{
	window.setVerticalSyncEnabled(true);
	glEnable(GL_TEXTURE_2D);
    glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	resize(videoMode.width, videoMode.height);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App::run()
{
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                //window.close();
		exit(0);

            if (event.type == sf::Event::Resized)
                resize(event.size.width, event.size.height);


            processEvent(event);
        }

        update(clock.restart().asSeconds(), sfKeyboard);
        render();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App::resize(int width, int height)
{
    if (height == 0)
        height = 1;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

	double ymax = nearZ * tan(fov * 4.0*atan(1.0) / 360.0);	//4.0*atan(1.0) = PI
	double ymin = -ymax;

	double aspect = (double)width/(double)height;
	double xmin = ymin * aspect;
	double xmax = ymax * aspect;

	glFrustum(xmin, xmax, ymin, ymax, nearZ, farZ);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
