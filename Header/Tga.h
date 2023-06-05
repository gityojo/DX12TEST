#pragma once

#ifndef _TGA_H_INCLUDED_
#define _TGA_H_INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>

class Tga
{
protected:
	char *mImageData;
	int mWidth;
	int mHeight;

	const int FORMAT_FULLCOLOR = 2;

	char* AlignmentImage(char *src, int width, int height, int bpp, int bit_vertical);

	std::string GetImageFormatLabel(int format);
	std::string GetOriginLabel(int bit_horizontal, int bit_vertical);

public:
	Tga();
	virtual ~Tga();

	char* GetImageData();
	int GetWidth();
	int GetHeight();
	bool ReadTGA(std::string filename);

};

#endif // _TGA_H_INCLUDED_
