#ifndef NLS_SOLUTION
#define NLS_SOLUTION

// #include "../testlib.h"
#include "../utils.cpp"
#include "NLS_object_ver2.cpp"
#include "NLS_repair_procedure.cpp"
#include "NLS_destroy_procedure.cpp"
#include "NLS_print_object.cpp"
#include "NLS_local_search.cpp"

namespace NLS_solution {
    const double alpha = 100;
    const double beta = 100;
    const double mul_threshold = 0.999;
    const double score_increased = 0.5;

    void process(Problem_Instance &instance, string outputFile, double time_limit, int max_iter) {
	    cerr << "******* VERSION: don't accept solution which violated upper_bound constraint *******\n";
	    cerr << "----------------NLS is working--------------------\n";

		/// init random best state
		clock_t startTime = clock();

		NLS_object best_obj;
		best_obj.Initialize(instance);

		vector<int> V;
		for (int i = 1; i <= instance.numInterventions; ++i) V.push_back(i);
		// instance.Random_Priority_Ascent_Delta_Sort(V);
		instance.Random_Priority_Descent_Delta_Sort(V);

		NLS_repair_procedure::process(instance, V, best_obj, alpha, beta);
		// NLS_repair_procedure::ore_thingking1(instance, V, best_obj, alpha, beta);

		double threshold = 0.15;
		bool flag = false;
		NLS_object current_obj = best_obj;

		if (best_obj.numFailedIntervention == 0 &&
			best_obj.numSatisfiedLBResources == instance.T * instance.numResources &&
			best_obj.numViolatedUBResources == 0) flag = true;

		for (int iter = 1; iter <= max_iter; ++iter) {
			cerr << "#ITER: " << iter << '\n';
			NLS_object temp_obj = current_obj;

			/// ruin and recreate
			int max_percentage = 30;
			vector<int> V = NLS_destroy_procedure::process(instance, temp_obj, max_percentage);
			// instance.Random_Priority_Ascent_Delta_Sort(V);
			instance.Random_Priority_Descent_Delta_Sort(V);

			NLS_repair_procedure::process(instance, V, temp_obj, alpha, beta);
			// NLS_repair_procedure::ore_thingking1(instance, V, temp_obj, alpha, beta);

			/// update result
			double best_score = best_obj.getScore(instance, alpha, beta);
			double temp_score = temp_obj.getScore(instance, alpha, beta);
			double current_score = current_obj.getScore(instance, alpha, beta);

			if ( temp_score + 1e-6 < current_score * (1  + threshold) ) {
			current_obj = temp_obj;
			if ( !flag || temp_obj.numFailedIntervention == 0 && temp_obj.get_OBJ(instance) + 1e-6 < best_obj.get_OBJ(instance)  ) {
				bool ok1 = ( temp_obj.numSatisfiedLBResources == instance.numResources * instance.T );
				bool ok2 = ( temp_obj.numViolatedUBResources == 0 );
				assert(ok2);
				if (ok1 && ok2) {
				best_obj = temp_obj;
				flag = true;
				cerr << "new best solution = " << best_obj.get_OBJ(instance)
					<< " found at: " << (double)(clock() - startTime) / CLOCKS_PER_SEC << '\n';
				}
			}
			}

			/// update threshold
			threshold *= mul_threshold;

			double current_time = (double)(clock() - startTime) / CLOCKS_PER_SEC;
			if (current_time > time_limit) break;
		}

		/// local search
		// int numLocalSearch = 0;
		// while (NLS_local_search::Change_time_start_intervetions(instance, best_obj, alpha, beta)) {
		//     cerr << "local search time: " << ++numLocalSearch <<
		//         " -> " << "SCORE = " << best_obj.getScore(instance, alpha, beta) <<
		//         " -> " << "OBJ = " << best_obj.get_OBJ(instance) << '\n';
		// }

		cerr << "NLS FINISH TIME = " << (double)(clock() - startTime) / CLOCKS_PER_SEC << '\n';
		cerr << "----------------NLS is done--------------------\n";

			// NLS_print_object::process(instance, best_obj, outputFile);
			if ( best_obj.numFailedIntervention > 0 ) {
			cout << "SOME INTERVENTIONS ARE NOT SCHEDULED !!!\n";
			exit(0);
			}

			ofstream out(outputFile);
			for (int i = 1; i <= instance.numInterventions; ++i) {
			out << instance.Intervention_name[i] << " " << best_obj.Time_Start_Intervention[i] << '\n';
			}

			cerr << "OBJ1 = " << best_obj.obj1 << '\n';
			cerr << "OBJ2 = " << best_obj.obj2 << '\n';
			cerr << "OBJ = " << best_obj.get_OBJ(instance) << '\n';
			// LAG O? DA^Y QUA'
    }

    void process_version_2(Problem_Instance &instance, string outputFile, double time_limit, int max_iter) {
	    cerr << "******* VERSION: accept solution which violated upper_bound constraint *******\n";
	    cerr << "----------------NLS is working--------------------\n";

        /// init random best state
        clock_t startTime = clock();

        NLS_object best_obj;
        best_obj.Initialize(instance);

        while (true) {
            vector<int> V;
            for (int i = 1; i <= instance.numInterventions; ++i) V.push_back(i);
            instance.Random_Priority_Descent_Delta_Sort(V);

            NLS_repair_procedure::process_version_2(instance, V, best_obj, alpha, beta);
            if (best_obj.numFailedIntervention == 0) break;
        }

        double threshold = 0.15;
        bool flag = false;
        NLS_object current_obj = best_obj;

        if (best_obj.numFailedIntervention == 0 &&
            best_obj.LBResources_cost <= 1e-5 &&
            best_obj.UBResources_cost <= 1e-5) flag = true;

			cerr << "T_T: " << flag << '\n';

        double alpha = 10, beta = 10;
        for (int iter = 1; iter <= max_iter; ++iter) {
            cerr << "#ITER: " << iter << '\n';
            NLS_object temp_obj = current_obj;

            /// ruin and recreate
            int max_percentage = 30;
            vector<int> V = NLS_destroy_procedure::process_version_2(instance, temp_obj, max_percentage);
            instance.Just_Shuffle(V);
            //instance.Random_Priority_Descent_Delta_Sort(V);

            // cerr << "AFTER ERASE: " << temp_obj.numViolatedUBResources << " "
            //     << temp_obj.numSatisfiedLBResources - instance.T * instance.numResources << '\n';

            NLS_repair_procedure::process_version_2(instance, V, temp_obj, alpha, beta);
            if ( temp_obj.numFailedIntervention > 0 ) {
                cerr << "FAILED SCHEDULE \n";
                continue;
            }
            // NLS_repair_procedure::MingAnhSenpaiIdea(instance, V, temp_obj, alpha, beta);

            // cerr << "AFTER INSERT: " << temp_obj.numViolatedUBResources << " "
            //     << temp_obj.numSatisfiedLBResources - instance.T * instance.numResources << '\n';
            cerr << "FLAG: " << flag << " --> " << temp_obj.get_OBJ(instance) << '\n';

            /// update result
            double temp_score = temp_obj.getScore(instance, alpha, beta);
            double current_score = current_obj.getScore(instance, alpha, beta);

            if ( temp_obj.LBResources_cost > 1e-6 ) alpha *= (1+score_increased);
            else alpha /= (1+score_increased);
            if ( temp_obj.UBResources_cost > 1e-6 ) beta *= (1+score_increased);
            else beta /= (1+score_increased);

            // cerr << "?? " << temp_obj.numFailedIntervention << " " << temp_obj.numSatisfiedLBResources << " " << temp_obj.numViolatedUBResources << '\n';
            // cerr << "## " << flag << '\n';

            if ( temp_score + 1e-6 < current_score * (1  + threshold) ) {
                current_obj = temp_obj;
            }

            if ( !flag || temp_obj.numFailedIntervention == 0 ) {
                bool ok1 = ( temp_obj.LBResources_cost <= 1e-5 );
                bool ok2 = ( temp_obj.UBResources_cost <= 1e-5 );
				bool ok3 = ( temp_obj.get_OBJ(instance) + 1e-6 < best_obj.get_OBJ(instance) );
				if (!flag) ok3 = true;
                if (ok1 && ok2 && ok3) {
                    best_obj = temp_obj;
                    flag = true;

                    cerr << "new best solution = " << best_obj.get_OBJ(instance)
                    << " found at: " << (double)(clock() - startTime) / CLOCKS_PER_SEC << '\n';

					// cerr << "WTF\n";
					// ofstream out(outputFile);
					// for (int i = 1; i <= instance.numInterventions; ++i) {
					// 	out << instance.Intervention_name[i] << " " << best_obj.Time_Start_Intervention[i] << '\n';
					// }
					// out.close();
					// exit(0);
                }
            }

            /// update threshold
            threshold *= mul_threshold;

            double current_time = (double)(clock() - startTime) / CLOCKS_PER_SEC;
            if (current_time > time_limit) break;
        }
		
		/// local search
		// while ( NLS_local_search::Change_time_start_interventions_better(instance, best_obj, alpha, beta) ) {
		// 	cerr << "local search: " << best_obj.get_OBJ(instance) << '\n';
		// }
		cerr << "BEFORE LS: " << best_obj.get_OBJ(instance) << '\n';
		// while ( NLS_local_search::Change_time_start_interventions_best(instance, best_obj, alpha, beta) ) {
		// 	cerr << "local search: " << best_obj.get_OBJ(instance) << '\n';
		// }

		int limit_number_local_search = 123;
		while (NLS_local_search::Swap_two_interventions_time_better(instance, best_obj, alpha, beta)) {
			cerr << "local search: " << best_obj.get_OBJ(instance) << '\n';
			if ( --limit_number_local_search <= 0 ) break;
		}

        cerr << "NLS FINISH TIME = " << (double)(clock() - startTime) / CLOCKS_PER_SEC << '\n';
        cerr << "----------------NLS is done--------------------\n";

            // NLS_print_object::process(instance, best_obj, outputFile);
            if ( best_obj.numFailedIntervention > 0 ) {
                cout << "SOME INTERVENTIONS ARE NOT SCHEDULED !!!\n";
                exit(0);
            }
            if ( !flag ) {
                cout << "NO SOLUTION FOUND\n";
                exit(0);
            }

            ofstream out(outputFile);
            for (int i = 1; i <= instance.numInterventions; ++i) {
                out << instance.Intervention_name[i] << " " << best_obj.Time_Start_Intervention[i] << '\n';
            }

            cerr << "OBJ1 = " << best_obj.obj1 << '\n';
            cerr << "OBJ2 = " << best_obj.obj2 << '\n';
            cerr << "OBJ = " << best_obj.get_OBJ(instance) << '\n';
            // LAG O? DA^Y QUA'
    }

    void MingAnhSenpaiSolution(Problem_Instance &instance, string outputFile, double time_limit, int max_iter) {
	    cerr << "******* VERSION: test new solution *******\n";
	    cerr << "----------------NLS is working--------------------\n";

		/// init random best state
		clock_t startTime = clock();

		NLS_object best_obj;
		best_obj.Initialize(instance);

		vector<int> V;
		for (int i = 1; i <= instance.numInterventions; ++i) V.push_back(i);
		instance.Random_Priority_Descent_Delta_Sort(V);

		// NLS_repair_procedure::process_version_2(instance, V, best_obj, alpha, beta);
		NLS_repair_procedure::MingAnhSenpaiIdea(instance, V, best_obj, alpha, beta);

		double threshold = 0.15;
		bool flag = false;
		NLS_object current_obj = best_obj;
		double best_cost = 1e10;

		if (best_obj.numFailedIntervention == 0 &&
			best_obj.numSatisfiedLBResources == instance.T * instance.numResources &&
			best_obj.numViolatedUBResources == 0) {
			flag = true;
			best_cost = best_obj.get_OBJ(instance);
			cerr << "new best solution = " << best_cost
			<< " found at: " << (double)(clock() - startTime) / CLOCKS_PER_SEC << '\n';
			}

		for (int iter = 1; iter <= max_iter; ++iter) {
			cerr << "#ITER: " << iter << '\n';
			NLS_object temp_obj = current_obj;

			/// ruin and recreate
			int max_percentage = 40;
			vector<int> V = NLS_destroy_procedure::process_version_2(instance, temp_obj, max_percentage);

			Utils::shuffle(V);
			NLS_repair_procedure::MingAnhSenpaiIdea(instance, V, temp_obj, alpha, beta);
			cerr << current_obj.UBResources_cost << " - " << temp_obj.UBResources_cost << "\n";

			/// update result
			if ( temp_obj.UBResources_cost > 0.001 && current_obj.UBResources_cost > 0.001 ) {
			cerr << "TYPE 1\n";
			double score_temp = temp_obj.UBResources_cost;
			double score_current = current_obj.UBResources_cost;
			if ( score_temp + 1e-6 < score_current*(1+threshold) ){
				current_obj = temp_obj;
			}
			}
			else if ( temp_obj.UBResources_cost <= 0.001 && current_obj.UBResources_cost <= 0.001 ) {
			cerr << "TYPE 2\n";
			double temp_score = temp_obj.get_OBJ(instance);
			double current_score = current_obj.get_OBJ(instance);
			if ( temp_score + 1e-6 < current_score * (1 + threshold) )
				current_obj = temp_obj;
			}
			else if ( temp_obj.UBResources_cost <= 0.001 && current_obj.UBResources_cost > 0.001 ) {
			cerr << "TYPE 3\n";
			current_obj = temp_obj;
			}
			else if ( temp_obj.UBResources_cost > 0.001 && current_obj.UBResources_cost <= 0.001) {
			cerr << "TYPE 4\n"; // MUST BE BETTER THAN THE BEST
			double temp_score = temp_obj.get_OBJ(instance);
			double BEST_score = best_cost;
			if ( temp_score + 1e-6 < BEST_score ) current_obj = temp_obj;
			}

			if (temp_obj.numFailedIntervention == 0 && temp_obj.get_OBJ(instance) + 1e-6 < best_cost ) {
			bool ok1 = true /*( temp_obj.numSatisfiedLBResources == instance.numResources * instance.T )*/;
			bool ok2 = ( temp_obj.UBResources_cost <= 1e-6 /*temp_obj.numViolatedUBResources == 0*/ );
			if (ok1 && ok2) {
				best_obj = temp_obj;
				best_cost = best_obj.get_OBJ(instance);
				flag = true;

				cerr << "new best solution = " << best_obj.get_OBJ(instance)
				<< " found at: " << (double)(clock() - startTime) / CLOCKS_PER_SEC << '\n';
			}
			}

			/// update threshold
			threshold *= mul_threshold;

			double current_time = (double)(clock() - startTime) / CLOCKS_PER_SEC;
			if (current_time > time_limit) break;
		}

		cerr << "NLS FINISH TIME = " << (double)(clock() - startTime) / CLOCKS_PER_SEC << '\n';
		cerr << "----------------NLS is done--------------------\n";

		// NLS_print_object::process(instance, best_obj, outputFile);
		if ( best_obj.numFailedIntervention > 0 || best_obj.UBResources_cost >= 1e-6 ) {
		cout << "SOME INTERVENTIONS ARE NOT SCHEDULED !!!\n";
		exit(0);
		}
		if ( !flag ) {
		cout << "NO SOLUTION FOUND\n";
		exit(0);
		}

		ofstream out(outputFile);
		for (int i = 1; i <= instance.numInterventions; ++i) {
		out << instance.Intervention_name[i] << " " << best_obj.Time_Start_Intervention[i] << '\n';
		}

		cerr << "OBJ1 = " << best_obj.obj1 << '\n';
		cerr << "OBJ2 = " << best_obj.obj2 << '\n';
		cerr << "OBJ = " << best_obj.get_OBJ(instance) << '\n';
		// LAG O? DA^Y QUA'
    }

	void WinterSolution(Problem_Instance &instance, string outputFile, double time_limit, int max_iter) {
		 cerr << "******* VERSION: accept solution which violated upper_bound constraint *******\n";
	    cerr << "----------------NLS is working--------------------\n";

        /// init random best state
        clock_t startTime = clock();

        NLS_object best_obj;
        best_obj.Initialize(instance);

        while (true) {
            vector<int> V;
            for (int i = 1; i <= instance.numInterventions; ++i) V.push_back(i);
            instance.Random_Priority_Descent_Delta_Sort(V);

            NLS_repair_procedure::process_version_2(instance, V, best_obj, alpha, beta);
            if (best_obj.numFailedIntervention == 0) break;
        }

        double threshold = 0.15;
        bool flag = false;
        NLS_object current_obj = best_obj;

        if (best_obj.numFailedIntervention == 0 &&
            best_obj.LBResources_cost <= 1e-5 &&
            best_obj.UBResources_cost <= 1e-5) flag = true;

			cerr << "T_T: " << flag << '\n';

        double alpha = 10, beta = 10;
        for (int iter = 1; iter <= max_iter; ++iter) {
            cerr << "#ITER: " << iter << '\n';
            NLS_object temp_obj = current_obj;

            /// ruin and recreate
            int max_percentage = 30;
            vector<int> V = NLS_destroy_procedure::process_version_2(instance, temp_obj, max_percentage);
            instance.Just_Shuffle(V);

            NLS_repair_procedure::process_version_2(instance, V, temp_obj, alpha, beta);
            if ( temp_obj.numFailedIntervention > 0 ) {
                cerr << "FAILED SCHEDULE \n";
                continue;
            }

            cerr << "FLAG: " << flag << " --> " << temp_obj.get_OBJ(instance) << '\n';

            /// update result
            double temp_score = temp_obj.getScore(instance, alpha, beta);
            double current_score = current_obj.getScore(instance, alpha, beta);

            if ( temp_obj.LBResources_cost > 1e-6 ) alpha *= (1+score_increased);
            else alpha /= (1+score_increased);
            if ( temp_obj.UBResources_cost > 1e-6 ) beta *= (1+score_increased);
            else beta /= (1+score_increased);

            if ( temp_score + 1e-6 < current_score * (1  + threshold) ) {
                current_obj = temp_obj;
            }

            if ( !flag || temp_obj.numFailedIntervention == 0 ) {
                bool ok1 = ( temp_obj.LBResources_cost <= 1e-5 );
                bool ok2 = ( temp_obj.UBResources_cost <= 1e-5 );
				bool ok3 = ( temp_obj.get_OBJ(instance) + 1e-6 < best_obj.get_OBJ(instance) );
				if (!flag) ok3 = true;
                if (ok1 && ok2 && ok3) {
                    best_obj = temp_obj;
                    flag = true;

                    cerr << "new best solution = " << best_obj.get_OBJ(instance)
                    << " found at: " << (double)(clock() - startTime) / CLOCKS_PER_SEC << '\n';
                }
            }

            /// update threshold
            threshold *= mul_threshold;

            double current_time = (double)(clock() - startTime) / CLOCKS_PER_SEC;
            if (current_time > time_limit) break;
        }
		
		cerr << "BEFORE LS: " << best_obj.get_OBJ(instance) << '\n';

		// int limit_number_local_search = 123;
		// while (NLS_local_search::Swap_two_interventions_time_better(instance, best_obj, alpha, beta)) {
		// 	cerr << "local search: " << best_obj.get_OBJ(instance) << '\n';
		// 	if ( --limit_number_local_search <= 0 ) break;
		// }

        cerr << "NLS FINISH TIME = " << (double)(clock() - startTime) / CLOCKS_PER_SEC << '\n';
        cerr << "----------------NLS is done--------------------\n";

            // NLS_print_object::process(instance, best_obj, outputFile);
            if ( best_obj.numFailedIntervention > 0 ) {
                cout << "SOME INTERVENTIONS ARE NOT SCHEDULED !!!\n";
                exit(0);
            }
            if ( !flag ) {
                cout << "NO SOLUTION FOUND\n";
                exit(0);
            }

            ofstream out(outputFile);
            for (int i = 1; i <= instance.numInterventions; ++i) {
                out << instance.Intervention_name[i] << " " << best_obj.Time_Start_Intervention[i] << '\n';
            }

            cerr << "OBJ1 = " << best_obj.obj1 << '\n';
            cerr << "OBJ2 = " << best_obj.obj2 << '\n';
            cerr << "OBJ = " << best_obj.get_OBJ(instance) << '\n';
            // LAG O? DA^Y QUA'
	}
}

#endif // NLS_SOLUTION