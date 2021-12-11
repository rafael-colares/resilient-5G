#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

#include "tools/others.hpp"
#include "instance/data.hpp"
#include "solver/model.hpp"
// TODO Check Leo's makefile
int main(int argc, char *argv[]) {
    greetingMessage();
    std::string parameterFile = getParameter(argc, argv);

    Data data(parameterFile);
    data.print();
    IloEnv env;
	
    try
    {
        /* Model construct */
        Model model(env, data);

        /* Model run */
        model.run();

        /* Print results */
        model.printResult();
    }
    catch (const IloException& e) { env.end(); std::cerr << "Exception caught: " << e << std::endl; return 1; }
    catch (...) { env.end(); std::cerr << "Unknown exception caught!" << std::endl; return 1; }
    

    /*** Finalization ***/
    env.end();
    return 0;
}
