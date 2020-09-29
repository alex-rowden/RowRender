#pragma once
#include "RowRender.h"
template<typename T>
class DataBuffer
{
public:
	static int counter;
	GLuint ssbo;
	DataBuffer(int num_elements);
	void clearBuffer();
};

