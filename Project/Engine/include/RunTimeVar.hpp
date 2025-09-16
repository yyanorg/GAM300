#pragma once
#include <GLFW/glfw3.h>


	//Placing only frequently used tmp vars (used in multiple files)
    namespace RunTimeVar {
	    struct Windows
	    {
			GLint width;
			GLint height;
	    };

		extern double deltaTime;
	    extern Windows window;
    }