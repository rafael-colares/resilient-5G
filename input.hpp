#ifndef __input__hpp
#define __input__hpp

#include <iostream>
#include <fstream>
#include <string>
#include <vector>


/*****************************************************************************************
 * This class stores all the information recovered from the parameter file, that is,
 * input/output file paths, execution and control parameters.						
*****************************************************************************************/
class Input{

private:
    /***** Input file paths *****/
    const std::string   parameters_file;
    std::string         node_file;
    std::string         link_file;
    std::string         demand_file;
    std::string         vnf_file;

    /***** Optimization parameters*****/
    bool                linear_relaxation;
    int                 time_limit;

    /***** Output file paths *****/
    std::string         output_file;
    
public:
	/********************************************/
	/*				Constructors				*/
	/********************************************/
	/** Constructor initializes the object with the information contained in the parameter file. @param file The address of the parameter file (usually the address of file 'parameters.txt'). **/
    Input(const std::string file);
    /** Constructor always need a parameter file. **/
    Input() = delete;



	/********************************************/
	/*				    Getters	    			*/
	/********************************************/
    /* Returns the parameters file. */
    const std::string& getParameterFile()  const { return this->parameters_file; }

    /* Returns the node file. */
    const std::string& getNodeFile()       const { return this->node_file; }

    /* Returns the link file. */
    const std::string& getLinkFile()       const { return this->link_file; }

    /* Returns the demand file. */
    const std::string& getDemandFile()     const { return this->demand_file; }

    /* Returns the VNF file. */
    const std::string& getVnfFile()        const { return this->vnf_file; }

    /* Returns true if linear relaxation is to be applied. */
    const bool&        isRelaxation()      const { return this->linear_relaxation; }

    /* Returns time limit in seconds to be applied. */
    const int&         getTimeLimit()      const { return this->time_limit; }

    /* Returns the output file. */
    const std::string& getOutputFile()     const { return this->output_file; }

	/********************************************/
	/*				    Methods	    			*/
	/********************************************/
    /* Returns the pattern value in the parameters file. */
    std::string getParameterValue(const std::string pattern);

	/********************************************/
	/*				    Output	    			*/
	/********************************************/
    /* Print the parameters stored in the parameter file. */
    void print();

};

#endif