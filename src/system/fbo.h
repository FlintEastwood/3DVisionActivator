#pragma once

//////////////////////////////////////////////////////////////////////////////////////////

class Fbo
{
public:
	Fbo(int w, int h);
	~Fbo();

	void setViewport();

	void bind(unsigned int i = 0);
	void bindFrameBuffer();
	void unbindFrameBuffer();

	unsigned int getWidth() const		{ return width;  }
	unsigned int getHeight() const		{ return height; }
	unsigned int getTextureId() const	{ return textureId;  }

private:
	unsigned int	width,
					height,
					frameBufferId,
					depthBufferId,
					textureId;
};

//////////////////////////////////////////////////////////////////////////////////////////
