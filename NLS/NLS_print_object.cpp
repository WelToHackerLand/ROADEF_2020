#ifndef NLS_PRINT_OBJECT 
#define NLS_PRINT_OBJECT

#include "../Problem.cpp"
#include "NLS_object.cpp"

namespace NLS_print_object {
    // void debug_workload_for_sure() {
    //     cerr << "-----------debug_workload_is_working-------------\n";
    //     cerr << "------------debug_workload_is_done---------------\n";
    // }

    void process(Problem_Instance instance, NLS_object obj, string outputFile) {
        if ( obj.numFailedIntervention > 0 ) {
            cout << "SOME INTERVENTIONS ARE NOT SCHEDULED !!!\n";
            exit(0);
        }

        ofstream out(outputFile);
        for (int i = 1; i <= instance.numInterventions; ++i) {
            out << instance.Intervention_name[i] << " " << obj.Time_Start_Intervention[i] << '\n';
        }

        cerr << "OBJ1 = " << obj.obj1 << '\n';
        cerr << "OBJ2 = " << obj.obj2 << '\n';
        cerr << "OBJ = " << obj.get_OBJ(instance) << '\n';
    }
}

#endif // NLS_PRINT_OBJECT