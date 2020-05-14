#ifndef NLS_REPAIR_PROCEDURE
#define NLS_REPAIR_PROCEDURE

// #include<bits/stdc++.h>
#include "../Problem_ver2.cpp"
#include "NLS_object_ver2.cpp"
#include "../utils.cpp"

namespace NLS_repair_procedure {
    int limit_Time_debug = 2;
    int Time_debug = 0;

    void ore_thingking1(Problem_Instance &instance, vector<int> lsVer, NLS_object &obj, double alpha, double beta) {
        for (int i : lsVer) {
            double best_score = -1;
            int best_start_Time = -1;
            for (int start_Time = 1; start_Time <= instance.tmax[i]; ++start_Time) {
                /// check exclusion constraint 
                if ( !obj.exclusionChecking(instance, i, start_Time) ) continue; 

                /// get cost of start_Time
                double cc = 0;
                bool ok = obj.Insert(instance, i, start_Time, cc);
                if (!ok) { 
                    bool tmpOK = obj.Erase(instance, i, start_Time);
                    assert(tmpOK == true); 
                    continue; 
                }

                double score = 0;
                score = cc * 100000000;

                if (best_start_Time == -1 || best_score > score + 1e-6) {
                    best_score = score;
                    best_start_Time = start_Time;
                }

                obj.Erase(instance, i, start_Time);
            }

            if ( best_start_Time == -1 ) continue;
            else {
                double cc = 0;
                bool ok = obj.Insert(instance, i, best_start_Time, cc);
                // obj.Interventions_at_time[best_start_Time].push_back(i);
                assert( ok == true );
            }
        }
    }

    void process(Problem_Instance &instance, vector<int> lsVer, NLS_object &obj, double alpha, double beta) {
        for (int i : lsVer) {
            double best_score = -1;
            int best_start_Time = -1;
            for (int start_Time = 1; start_Time <= instance.tmax[i]; ++start_Time) {
                /// check exclusion constraint 
                if ( !obj.exclusionChecking(instance, i, start_Time) ) continue; 

                /// get cost of start_Time
                double increase_satisfied_LB = 0;
                bool ok = obj.Insert(instance, i, start_Time, increase_satisfied_LB);
                if (!ok) { 
                    bool tmpOK = obj.Erase(instance, i, start_Time);
                    assert(tmpOK == true); 
                    continue; 
                }

                double obj_score = obj.get_OBJ(instance);
                double satisfied_score = instance.T * instance.numResources - obj.numSatisfiedLBResources;
                double total_score = obj_score + alpha * satisfied_score;// + increase_satisfied_LB * 100000000;
                // double total_score = increase_satisfied_LB * 100000000;

                if ( best_start_Time == -1 || best_score > total_score + 1e-6 ) {
                    best_score = total_score;
                    best_start_Time = start_Time;
                }

                obj.Erase(instance, i, start_Time);

                    // cerr << "# " << best_start_Time << " " << instance.T << '\n';
            }

            if ( best_start_Time == -1 ) continue;
            else {
                double increase_satisfied_LB = 0;
                bool ok = obj.Insert(instance, i, best_start_Time, increase_satisfied_LB);
                // obj.Interventions_at_time[best_start_Time].push_back(i);
                assert( ok == true );
            }
        }
    }  

    void process_version_2(Problem_Instance &instance, vector<int> lsVer, NLS_object &obj, double alpha, double beta) {
        /// NO CARE ABOUT VIOLATING UPPER BOUND CONSTRAINT
        for (int i : lsVer) {
            double best_score = -1;
            int best_start_Time = -1;
            int org = obj.numFailedIntervention;
            for (int start_Time = 1; start_Time <= instance.tmax[i]; ++start_Time) {
                /// check exclusion constraint 
                if ( !obj.exclusionChecking(instance, i, start_Time) ) continue; 
                
                /// check limited time step constraint
                if ( start_Time + instance.delta[i][start_Time] > instance.T+1 ) continue;

                /// get cost of start_Time
                int numAcceptedLB = 0, numViolatedUB = 0;
                double costLB = 0, costUB = 0;
                double oreLB = obj.LBResources_cost, oreUB = obj.UBResources_cost;
                obj.Insert_no_care_UB(instance, i, start_Time, numAcceptedLB, numViolatedUB, costLB, costUB);

                double obj_score = obj.get_OBJ(instance);
                double LB_score = costLB * alpha;
                double UB_score = costUB * beta;
                double total_score = obj_score - LB_score + UB_score;

                if ( best_start_Time == -1 || best_score > total_score + 1e-6 ) {
                    best_score = total_score;
                    best_start_Time = start_Time;
                }

                obj.Erase_no_care_UB(instance, i, start_Time, numAcceptedLB, numViolatedUB);

                assert(numAcceptedLB == 0);
                assert(numViolatedUB == 0);
            }

            if ( best_start_Time == -1 ) continue;
            else {
                int nothing1 = 0, nothing2 = 0;
                double nothing3 = 0, nothing4 = 0;
                obj.Insert_no_care_UB(instance, i, best_start_Time, nothing1, nothing2, nothing3, nothing4);
                
                    // if (obj.UBResources_cost > 1e-4) {
                    //     cerr << "NOW:\n";
                    //     cerr << obj.UBResources_cost << '\n';
                    //     exit(0);
                    // }
            }
        }
    }  

    struct MingAnhSenpai_data {
        int t;
        double costUB, score;
        MingAnhSenpai_data() {};
        MingAnhSenpai_data(int t, double costUB, double score) : t(t), costUB(costUB), score(score) {};
    };

    void MingAnhSenpaiIdea(Problem_Instance &instance, vector<int> lsVer, NLS_object &obj, double alpha, double beta) {
	    int lim_swapped_percent = 20; /// 50

        for (int i : lsVer) {
            vector<MingAnhSenpai_data> lsTime;

            for (int start_Time = 1; start_Time <= instance.tmax[i]; ++start_Time) {
                /// check exclusion constraint 
                if ( !obj.exclusionChecking(instance, i, start_Time) ) continue; 

                /// check limited time step constraint
                if ( start_Time + instance.delta[i][start_Time] > instance.T+1 ) continue;
                
                /// add to vector 
                int numAcceptedLB = 0, numViolatedUB = 0;
                double costLB = 0, costUB = 0;

                obj.Insert_no_care_UB(instance, i, start_Time, numAcceptedLB, numViolatedUB, costLB, costUB);

                double obj_score = obj.get_OBJ(instance);
                double LB_score = alpha * numAcceptedLB;
                double UB_score = 2 * 1000 * alpha * numViolatedUB;
		        double total_score = obj_score /*- LB_score + UB_score*/;

                assert(numViolatedUB >= 0);
                lsTime.push_back({ start_Time, costUB, total_score });

                obj.Erase_no_care_UB(instance, i, start_Time, numAcceptedLB, numViolatedUB);

                assert(numAcceptedLB == 0);
                assert(numViolatedUB == 0);
            }

            /// population-based sort 
            for (int i = 0; i < (int) lsTime.size()-1; ++i) {
		        int swapped_percent = Utils::integer_random_generator(0, 101);
                bool didswap = false;
                for (int j = 0; j < (int) lsTime.size()-1; ++j) {
                    if ( swapped_percent < lim_swapped_percent || (lsTime[j].costUB<=1e-6 && lsTime[j+1].costUB<=1e-6) ) {
                        if ( lsTime[j].score > lsTime[j+1].score ){
                            swap( lsTime[j], lsTime[j+1] );
                            didswap = true;
                        }
                    }
                    else {
                        if ( lsTime[j].costUB > lsTime[j+1].costUB ){
                            swap( lsTime[j], lsTime[j+1] );
                            didswap = true;
                        }
                    }
                }
                if (!didswap) break;
            }

            /// get best time start
            if ( lsTime.size() ) {
                int best_start_Time = lsTime[0].t;
                int nothing1 = 0, nothing2 = 0;
                double nothing3 = 0, nothing4 = 0;
                obj.Insert_no_care_UB(instance, i, best_start_Time, nothing1, nothing2, nothing3, nothing4);
            }
        }
    }
}

#endif // NLS_REPAIR_PROCEDURE  
