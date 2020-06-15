#pragma once
#include <string>
#include "RowRender.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
class GaussianLoader
{
public:
	
	static bool loadGaussians(std::string, std::vector<Gaussian>&);
};

