// #include "testlib.h"
#include "Problem_ver2.cpp"
#include "config.cpp"
#include "utils.cpp"
#include "NLS/NLS_solution_ver2.cpp"
// #include "NLS/NLS_check_output.cpp"
#include "NLS/ACO_solution_ver2.cpp"
#include "NLS/NLSACO_solution.cpp"
#include "NLS/percent_heap.cpp"

int main(int argc, char* argv[]) {
//    srand(time(NULL));

    config::parse_arguments(argc, argv);
    Problem_Instance instance;

    instance.read_Json_file( config::input );
    // instance.debug_parameter( config::input );

    // percent_heap cc;
    // cc.test();

    // NLS_check_output::process("log_A07-Copy.txt", instance);
    // exit(0);

    // NLS_solution::process( instance, config::output, config::time_limit, config::max_iter );
    // NLS_solution::process_version_2( instance, config::output, config::time_limit, config::max_iter );
    // NLS_solution::MingAnhSenpaiSolution( instance, config::output, config::time_limit, config::max_iter );
    ACO_solution::process( instance, config::output, config::time_limit, 10, 10 );
    // NLSACO_solution::process( instance, config::output, config::time_limit, 10, 10 );

    return 0;
} 
