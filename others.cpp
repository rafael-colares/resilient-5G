#include "others.hpp"

/* Writes a greeting message. */
void greetingMessage(){
    std::cout << "=================================================================" << std::endl;
    std::cout << "- This program was developed by Rafael Colares for Orange Labs. -" << std::endl;
    std::cout << "=================================================================" << std::endl;
}

/* Returns the path to parameter file. */
std::string getParameter(int argc, char *argv[]){
    std::string param;
    if (argc != 2){
		std::cerr << "A parameter file is required in the arguments. Please run the program in the following way: \n ./exec parameterFile.txt\n";
		throw std::invalid_argument( "@racolares: An argument is missing." );
	}
	else{
		param = argv[1];
        std::cout << "PARAMETER FILE: " << param << std::endl;
	}
    return param;
}