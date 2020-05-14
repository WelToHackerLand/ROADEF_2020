#include "NLS_object.cpp"
#include "../Problem.cpp"

namespace NLS_check_output {
    void process(string outputFile, Problem_Instance &instance) {
        ifstream inp(outputFile);

        NLS_object obj;
        obj.Initialize(instance);

        for (int id = 1; id <= instance.numInterventions; ++id) {
                cerr << id << '\n';
            string Name;
            int start_Time;

            inp >> Name >> start_Time;
            
                cerr << "??? " << Name << " " << start_Time << '\n';

            int i = instance.Map_name_Intervention[Name];

                // continue;

            double cc = 0;
            obj.Insert(instance, i, start_Time, cc);
        }

        cout << "OBJ1 = " << obj.obj1 << '\n';
        cerr << "OBJ2 = " << obj.obj2 << '\n';
        cerr << "OBJ = " << obj.get_OBJ(instance) << '\n';
    }
}