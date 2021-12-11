#ifndef __others__hpp
#define __others__hpp

#include <iostream>
#include <string>

/***************************************************
 *  This file hosts the list of auxiliary methods. 
 **************************************************/

/** Writes a greeting message. **/
void        greetingMessage();

/** Returns the path to parameter file. **/
std::string getParameter(int argc, char *argv[]);

#endif