#ifndef NLS_LOCAL_SEARCH
#define NLS_LOCAL_SEARCH

#include "../Problem_ver2.cpp"
#include "NLS_object_ver2.cpp"

namespace NLS_local_search {
    const double diff = 0.05;

    bool Change_time_start_interventions_better(Problem_Instance &instance, NLS_object &obj, double alpha, double beta, double diff = 0.05) {
        for (int i = 1; i <= instance.numInterventions; ++i) {
            if ( !obj.Time_Start_Intervention[i] ) continue;

            int oldTimeStart = obj.Time_Start_Intervention[i], best_start_Time = -1;
            double oldScore = obj.get_OBJ(instance);
            int ACLB = 0, VLUB = 0;
            obj.Erase_no_care_UB(instance, i, oldTimeStart, ACLB, VLUB);

            for (int newTimeStart = 1; newTimeStart <= instance.tmax[i]; ++newTimeStart) {
                if ( newTimeStart == oldTimeStart ) continue;
                if ( !obj.exclusionChecking(instance, i, newTimeStart) ) continue;

                double LB_cost = 0, UB_cost = 0; 
                obj.Insert_no_care_UB(instance, i, newTimeStart, ACLB, VLUB, LB_cost, UB_cost);

                bool ok = true; 
                if ( obj.LBResources_cost > 1e-4 || obj.UBResources_cost > 1e-4 ) ok = false;
                if (!ok) {
                    obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
                    continue; 
                }

                double newScore = obj.get_OBJ(instance);
                if (newScore + diff < oldScore) return true;

                obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
            }
            
            double LB_cost = 0, UB_cost = 0;
            obj.Insert_no_care_UB(instance, i, oldTimeStart, ACLB, VLUB, LB_cost, UB_cost);
        }
        return false;
    }

    bool Change_time_start_interventions_best(Problem_Instance &instance, NLS_object &obj, double alpha, double beta, double diff = 0.05) {
        double bestScore = obj.get_OBJ(instance);
        int bestIntervention = -1, bestTime = -1;

        for (int i = 1; i <= instance.numInterventions; ++i) {
            if ( !obj.Time_Start_Intervention[i] ) continue;

            int oldTimeStart = obj.Time_Start_Intervention[i], best_start_Time = -1;
            double oldScore = obj.get_OBJ(instance);
            int ACLB = 0, VLUB = 0;
            obj.Erase_no_care_UB(instance, i, oldTimeStart, ACLB, VLUB);

            for (int newTimeStart = 1; newTimeStart <= instance.tmax[i]; ++newTimeStart) {
                if ( newTimeStart == oldTimeStart ) continue;
                if ( !obj.exclusionChecking(instance, i, newTimeStart) ) continue;

                double LB_cost = 0, UB_cost = 0; 
                obj.Insert_no_care_UB(instance, i, newTimeStart, ACLB, VLUB, LB_cost, UB_cost);

                bool ok = true; 
                if ( obj.LBResources_cost > 1e-4 || obj.UBResources_cost > 1e-4 ) ok = false;
                if (!ok) {
                    obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
                    continue; 
                }

                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    bestScore = newScore;
                    bestIntervention = i;
                    bestTime = newTimeStart;
                }
                obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
            }

            double LB_cost = 0, UB_cost = 0;
            obj.Insert_no_care_UB(instance, i, oldTimeStart, ACLB, VLUB, LB_cost, UB_cost);
        }

        if (bestTime == -1) return false;
        int i = bestIntervention, newTime = bestTime, oldTime = obj.Time_Start_Intervention[i];

            // cerr << "?? " << bestScore << " " << obj.get_OBJ(instance) << '\n';

        int ACLB = 0, VLUB = 0;
        double costLB = 0, costUB = 0;
        obj.Erase_no_care_UB(instance, i, oldTime, ACLB, VLUB);
        obj.Insert_no_care_UB(instance, i, newTime, ACLB, VLUB, costLB, costUB);
        return true;
    }

    bool Swap_two_interventions_time_better(Problem_Instance &instance, NLS_object &obj, double alpha, double beta, double diff = 0.05) {
        double oldScore = obj.get_OBJ(instance);

        for (int i = 1; i <= instance.numInterventions; ++i) 
        for (int j = 1; j <= instance.numInterventions; ++j) {
            int time_i = obj.Time_Start_Intervention[i];
            int time_j = obj.Time_Start_Intervention[j];
            if (time_j > instance.tmax[i] || time_i > instance.tmax[j]) continue;

            int nAC = 0, nVL = 0;
            double costLB = 0, costUB = 0;
            obj.Erase_no_care_UB(instance, i, time_i, nAC, nVL);
            obj.Erase_no_care_UB(instance, j, time_j, nAC, nVL);
            
            if ( !obj.exclusionChecking(instance, i, time_j) || !obj.exclusionChecking(instance, j, time_i) ) {
                obj.Insert_no_care_UB(instance, i, time_i, nAC, nVL, costLB, costUB);
                obj.Insert_no_care_UB(instance, j, time_j, nAC, nVL, costLB, costUB);
                continue;
            }

            obj.Insert_no_care_UB(instance, i, time_j, nAC, nVL, costLB, costUB);
            obj.Insert_no_care_UB(instance, j, time_i, nAC, nVL, costLB, costUB);

            if ( obj.LBResources_cost > 1e-4 || obj.UBResources_cost > 1e-4 ) {
                obj.Erase_no_care_UB(instance, i, time_j, nAC, nVL);
                obj.Erase_no_care_UB(instance, j, time_i, nAC, nVL);
                obj.Insert_no_care_UB(instance, i, time_i, nAC, nVL, costLB, costUB);
                obj.Insert_no_care_UB(instance, j, time_j, nAC, nVL, costLB, costUB);
                continue;
            }

            double newScore = obj.get_OBJ(instance);
            if ( newScore + diff < oldScore ) return true;
            obj.Erase_no_care_UB(instance, i, time_j, nAC, nVL);
            obj.Erase_no_care_UB(instance, j, time_i, nAC, nVL);
            obj.Insert_no_care_UB(instance, i, time_i, nAC, nVL, costLB, costUB);
            obj.Insert_no_care_UB(instance, j, time_j, nAC, nVL, costLB, costUB);
        }
        return false;
    }

    bool Swap_two_interventions_time_best(Problem_Instance &instance, NLS_object &obj, double alpha, double beta, double diff = 0.05) {
        double bestScore = obj.get_OBJ(instance);
        int best_i = -1, best_j = -1;

        for (int i = 1; i <= instance.numInterventions; ++i) 
        for (int j = 1; j <= instance.numInterventions; ++j) {
            int time_i = obj.Time_Start_Intervention[i];
            int time_j = obj.Time_Start_Intervention[j];
            if (time_j > instance.tmax[i] || time_i > instance.tmax[j]) continue;

            int nAC = 0, nVL = 0;
            double costLB = 0, costUB = 0;
            obj.Erase_no_care_UB(instance, i, time_i, nAC, nVL);
            obj.Erase_no_care_UB(instance, j, time_j, nAC, nVL);
            
            if ( !obj.exclusionChecking(instance, i, time_j) || !obj.exclusionChecking(instance, j, time_i) ) {
                obj.Insert_no_care_UB(instance, i, time_i, nAC, nVL, costLB, costUB);
                obj.Insert_no_care_UB(instance, j, time_j, nAC, nVL, costLB, costUB);
                continue;
            }
            
            obj.Insert_no_care_UB(instance, i, time_j, nAC, nVL, costLB, costUB);
            obj.Insert_no_care_UB(instance, j, time_i, nAC, nVL, costLB, costUB);

            if ( obj.LBResources_cost > 1e-4 || obj.UBResources_cost > 1e-4 ) {
                obj.Erase_no_care_UB(instance, i, time_j, nAC, nVL);
                obj.Erase_no_care_UB(instance, j, time_i, nAC, nVL);
                obj.Insert_no_care_UB(instance, i, time_i, nAC, nVL, costLB, costUB);
                obj.Insert_no_care_UB(instance, j, time_j, nAC, nVL, costLB, costUB);
                continue;
            }

            double newScore = obj.get_OBJ(instance);
            if ( newScore + diff < bestScore ) {
                best_i = i; best_j = j;
                bestScore = newScore;
            }
            obj.Erase_no_care_UB(instance, i, time_j, nAC, nVL);
            obj.Erase_no_care_UB(instance, j, time_i, nAC, nVL);
            obj.Insert_no_care_UB(instance, i, time_i, nAC, nVL, costLB, costUB);
            obj.Insert_no_care_UB(instance, j, time_j, nAC, nVL, costLB, costUB);
        }

        if( best_i == -1 || best_j == -1 ) return false;

        int i = best_i, j = best_j, nAC = 0, nVL = 0;
        double costLB = 0, costUB = 0;
        int time_i = obj.Time_Start_Intervention[i], time_j = obj.Time_Start_Intervention[j];
        obj.Erase_no_care_UB(instance, i, time_i, nAC, nVL);
        obj.Erase_no_care_UB(instance, j, time_j, nAC, nVL);
        obj.Insert_no_care_UB(instance, i, time_j, nAC, nVL, costLB, costUB);
        obj.Insert_no_care_UB(instance, j, time_i, nAC, nVL, costLB, costUB);
        return true;
    }

    bool dp_Change_time_interventions(Problem_Instance &instance, NLS_object &obj, double alpha, double beta, double diff = 0.05) {
        vector<pair<int, int> > V;
        for (int i = 1; i <= instance.numInterventions; ++i) { 
            int start_time = obj.Time_Start_Intervention[i];
            int end_time = start_time + instance.delta[i][start_time];
            V.push_back( make_pair(end_time, i) );   
        }
        sort(V.begin(), V.end());
        
        bool returnVal = false;
        for (auto foo : V) {
            int i = foo.second;
            if ( !obj.Time_Start_Intervention[i] ) continue;

            int oldTimeStart = obj.Time_Start_Intervention[i], bestTimeStart = oldTimeStart;
            double bestScore = obj.get_OBJ(instance);
            int ACLB = 0, VLUB = 0;
            obj.Erase_no_care_UB(instance, i, oldTimeStart, ACLB, VLUB);

            for (int newTimeStart = 1; newTimeStart <= instance.tmax[i]; ++newTimeStart) {
                if ( newTimeStart == oldTimeStart ) continue;
                if ( !obj.exclusionChecking(instance, i, newTimeStart) ) continue;

                double LB_cost = 0, UB_cost = 0; 
                obj.Insert_no_care_UB(instance, i, newTimeStart, ACLB, VLUB, LB_cost, UB_cost);

                bool ok = true; 
                if ( obj.LBResources_cost > 1e-4 || obj.UBResources_cost > 1e-4 ) ok = false;
                if (!ok) {
                    obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
                    continue; 
                }

                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    bestScore = newScore;
                    bestTimeStart = newTimeStart;
                }
                obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
            }

            double LB_cost = 0, UB_cost = 0;
            obj.Insert_no_care_UB(instance, i, bestTimeStart, ACLB, VLUB, LB_cost, UB_cost);
            if (bestTimeStart != oldTimeStart) returnVal = true;
        }
        return returnVal;
    }

    bool dp_Change_time_interventions_random_time_sort(Problem_Instance &instance, NLS_object &obj, double alpha, double beta, double diff = 0.05) {
        vector<pair<int, int> > V;
        for (int i = 1; i <= instance.numInterventions; ++i) { 
            int start_time = obj.Time_Start_Intervention[i];
            int end_time = start_time + instance.delta[i][start_time];
            V.push_back( make_pair(end_time, i) );   
        }
        sort(V.begin(), V.end());
        
        bool returnVal = false;
        for (auto foo : V) {
            int i = foo.second;
            if ( !obj.Time_Start_Intervention[i] ) continue;

            int oldTimeStart = obj.Time_Start_Intervention[i], bestTimeStart = oldTimeStart;
            double bestScore = obj.get_OBJ(instance);
            int ACLB = 0, VLUB = 0;
            obj.Erase_no_care_UB(instance, i, oldTimeStart, ACLB, VLUB);

            vector<int> time_order;
            for (int newTimeStart = 1; newTimeStart <= instance.tmax[i]; ++newTimeStart) {
                if ( newTimeStart == oldTimeStart ) continue;
                if ( !obj.exclusionChecking(instance, i, newTimeStart) ) continue;
                time_order.push_back(newTimeStart);
            }

            int percent = rand() % 100;
            if (percent < 50) reverse(time_order.begin(), time_order.end()); 

            for (int newTimeStart : time_order) {
                double LB_cost = 0, UB_cost = 0; 
                obj.Insert_no_care_UB(instance, i, newTimeStart, ACLB, VLUB, LB_cost, UB_cost);

                bool ok = true; 
                if ( obj.LBResources_cost > 1e-4 || obj.UBResources_cost > 1e-4 ) ok = false;
                if (!ok) {
                    obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
                    continue; 
                }

                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    bestScore = newScore;
                    bestTimeStart = newTimeStart;
                }
                obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
            }

            double LB_cost = 0, UB_cost = 0;
            obj.Insert_no_care_UB(instance, i, bestTimeStart, ACLB, VLUB, LB_cost, UB_cost);
            if (bestTimeStart != oldTimeStart) returnVal = true;
        }
        return returnVal;
    }

    bool dp_Change_time_interventions_more_complex(Problem_Instance &instance, NLS_object &obj, double alpha, double beta, double diff = 0.05) {
        vector<pair<int, int> > V;
        for (int i = 1; i <= instance.numInterventions; ++i) { 
            int start_time = obj.Time_Start_Intervention[i];
            int end_time = start_time + instance.delta[i][start_time];
            V.push_back( make_pair(end_time, i) );   
        }
        sort(V.begin(), V.end());
        
        bool returnVal = false;
        for (auto foo : V) {
            int i = foo.second;
            if ( !obj.Time_Start_Intervention[i] ) continue;

            int oldTimeStart = obj.Time_Start_Intervention[i], bestTimeStart = oldTimeStart;
            double bestScore = obj.get_OBJ(instance);
            int ACLB = 0, VLUB = 0;
            obj.Erase_no_care_UB(instance, i, oldTimeStart, ACLB, VLUB);

            vector<int> time_order;
            for (int newTimeStart = 1; newTimeStart <= instance.tmax[i]; ++newTimeStart) {
                if ( newTimeStart == oldTimeStart ) continue;
                if ( !obj.exclusionChecking(instance, i, newTimeStart) ) continue;
                time_order.push_back(newTimeStart);
            }

            int percent = rand() % 100;
            if (percent < 50) reverse(time_order.begin(), time_order.end()); 

            for (int newTimeStart : time_order) {
                double LB_cost = 0, UB_cost = 0; 
                obj.Insert_no_care_UB(instance, i, newTimeStart, ACLB, VLUB, LB_cost, UB_cost);

                bool ok = true; 
                if ( obj.LBResources_cost > 1e-4 || obj.UBResources_cost > 1e-4 ) ok = false;
                if (!ok) {
                    obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
                    continue; 
                }

                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    bestScore = newScore;
                    bestTimeStart = newTimeStart;
                }
                obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
            }

            double LB_cost = 0, UB_cost = 0;
            obj.Insert_no_care_UB(instance, i, bestTimeStart, ACLB, VLUB, LB_cost, UB_cost);
            if (bestTimeStart != oldTimeStart) returnVal = true;

            for (auto preFoo : V) {
                if (preFoo == foo) break;
                int j = preFoo.second;
                if ( !obj.Time_Start_Intervention[j] ) continue;

                int oldTimeStart = obj.Time_Start_Intervention[j], bestTimeStart = oldTimeStart;
                double bestScore = obj.get_OBJ(instance);
                int ACLB = 0, VLUB = 0;
                obj.Erase_no_care_UB(instance, j, oldTimeStart, ACLB, VLUB);

                vector<int> time_order;
                for (int newTimeStart = 1; newTimeStart <= instance.tmax[j]; ++newTimeStart) {
                    if ( newTimeStart == oldTimeStart ) continue;
                    if ( !obj.exclusionChecking(instance, j, newTimeStart) ) continue;
                    time_order.push_back(newTimeStart);
                }

                int percent = rand() % 100;
                if (percent < 50) reverse(time_order.begin(), time_order.end()); 

                for (int newTimeStart : time_order) {
                    double LB_cost = 0, UB_cost = 0; 
                    obj.Insert_no_care_UB(instance, j, newTimeStart, ACLB, VLUB, LB_cost, UB_cost);

                    bool ok = true; 
                    if ( obj.LBResources_cost > 1e-4 || obj.UBResources_cost > 1e-4 ) ok = false;
                    if (!ok) {
                        obj.Erase_no_care_UB(instance, j, newTimeStart, ACLB, VLUB);
                        continue; 
                    }

                    double newScore = obj.get_OBJ(instance);
                    if ( newScore + diff < bestScore ) {
                        bestScore = newScore;
                        bestTimeStart = newTimeStart;
                    }
                    obj.Erase_no_care_UB(instance, j, newTimeStart, ACLB, VLUB);
                }

                double LB_cost = 0, UB_cost = 0;
                obj.Insert_no_care_UB(instance, j, bestTimeStart, ACLB, VLUB, LB_cost, UB_cost);
            }
        }
        return returnVal;
    }

    bool dp_Change_time_and_swap_interventions(Problem_Instance &instance, NLS_object &obj, double alpha, double beta, double diff = 0.05) {
        vector<pair<int, int> > V;
        for (int i = 1; i <= instance.numInterventions; ++i) { 
            int start_time = obj.Time_Start_Intervention[i];
            int end_time = start_time + instance.delta[i][start_time];
            V.push_back( make_pair(end_time, i) );   
        }
        sort(V.begin(), V.end());
        
        bool returnVal = false;
        for (auto foo : V) {
            int i = foo.second;
            if ( !obj.Time_Start_Intervention[i] ) continue;

            int oldTimeStart = obj.Time_Start_Intervention[i], bestTimeStart = oldTimeStart;
            double bestScore = obj.get_OBJ(instance);
            int ACLB = 0, VLUB = 0;
            obj.Erase_no_care_UB(instance, i, oldTimeStart, ACLB, VLUB);

            vector<int> time_order;
            for (int newTimeStart = 1; newTimeStart <= instance.tmax[i]; ++newTimeStart) {
                if ( newTimeStart == oldTimeStart ) continue;
                if ( !obj.exclusionChecking(instance, i, newTimeStart) ) continue;
                time_order.push_back(newTimeStart);
            }

            int percent = rand() % 100;
            if (percent < 50) reverse(time_order.begin(), time_order.end()); 

            for (int newTimeStart : time_order) {
                double LB_cost = 0, UB_cost = 0; 
                obj.Insert_no_care_UB(instance, i, newTimeStart, ACLB, VLUB, LB_cost, UB_cost);

                bool ok = true; 
                if ( obj.LBResources_cost > 1e-4 || obj.UBResources_cost > 1e-4 ) ok = false;
                if (!ok) {
                    obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
                    continue; 
                }

                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    bestScore = newScore;
                    bestTimeStart = newTimeStart;
                }
                obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
            }

            double LB_cost = 0, UB_cost = 0;
            obj.Insert_no_care_UB(instance, i, bestTimeStart, ACLB, VLUB, LB_cost, UB_cost);
            if (bestTimeStart != oldTimeStart) returnVal = true;

            for (auto preFoo : V) {
                if (preFoo == foo) break;
                int j = preFoo.second;
                
                /// swap i and j
                int time_i = obj.Time_Start_Intervention[i];
                int time_j = obj.Time_Start_Intervention[j];
                if (time_j > instance.tmax[i] || time_i > instance.tmax[j]) continue;

                int nAC = 0, nVL = 0;
                double costLB = 0, costUB = 0;
                obj.Erase_no_care_UB(instance, i, time_i, nAC, nVL);
                obj.Erase_no_care_UB(instance, j, time_j, nAC, nVL);
                
                if ( !obj.exclusionChecking(instance, i, time_j) ) {
                    obj.Insert_no_care_UB(instance, i, time_i, nAC, nVL, costLB, costUB);
                    obj.Insert_no_care_UB(instance, j, time_j, nAC, nVL, costLB, costUB);
                    continue;
                }
                obj.Insert_no_care_UB(instance, i, time_j, nAC, nVL, costLB, costUB);
                
                if ( !obj.exclusionChecking(instance, j, time_i) ) {
                    obj.Erase_no_care_UB(instance, i, time_j, nAC, nVL);
                    obj.Insert_no_care_UB(instance, i, time_i, nAC, nVL, costLB, costUB);
                    obj.Insert_no_care_UB(instance, j, time_j, nAC, nVL, costLB, costUB);
                    continue;
                }
                obj.Insert_no_care_UB(instance, j, time_i, nAC, nVL, costLB, costUB);

                if ( obj.LBResources_cost > 1e-4 || obj.UBResources_cost > 1e-4 ) {
                    obj.Erase_no_care_UB(instance, i, time_j, nAC, nVL);
                    obj.Erase_no_care_UB(instance, j, time_i, nAC, nVL);
                    obj.Insert_no_care_UB(instance, i, time_i, nAC, nVL, costLB, costUB);
                    obj.Insert_no_care_UB(instance, j, time_j, nAC, nVL, costLB, costUB);
                    continue;
                }

                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    bestScore = newScore;
                    returnVal = true;
                    continue;
                }
                obj.Erase_no_care_UB(instance, i, time_j, nAC, nVL);
                obj.Erase_no_care_UB(instance, j, time_i, nAC, nVL);
                obj.Insert_no_care_UB(instance, i, time_i, nAC, nVL, costLB, costUB);
                obj.Insert_no_care_UB(instance, j, time_j, nAC, nVL, costLB, costUB);
            } 
        }

        return returnVal;
    }

    bool dp_Change_time_and_swap_interventions_update_version(Problem_Instance &instance, NLS_object &obj, double alpha, double beta, double diff = 0.05) {
        vector<pair<int, int> > V;
        for (int i = 1; i <= instance.numInterventions; ++i) { 
            int start_time = obj.Time_Start_Intervention[i];
            int end_time = start_time + instance.delta[i][start_time];
            V.push_back( make_pair(end_time, i) );   
        }
        sort(V.begin(), V.end());
        
        bool returnVal = false;
        for (auto foo : V) {
            int i = foo.second;
            if ( !obj.Time_Start_Intervention[i] ) continue;

            int oldTimeStart = obj.Time_Start_Intervention[i], bestTimeStart = oldTimeStart;
            double bestScore = obj.get_OBJ(instance);
            int ACLB = 0, VLUB = 0;
            obj.Erase_no_care_UB(instance, i, oldTimeStart, ACLB, VLUB);

            vector<int> time_order;
            for (int newTimeStart = 1; newTimeStart <= instance.tmax[i]; ++newTimeStart) {
                if ( newTimeStart == oldTimeStart ) continue;
                if ( !obj.exclusionChecking(instance, i, newTimeStart) ) continue;
                time_order.push_back(newTimeStart);
            }

            int percent = rand() % 100;
            if (percent < 50) reverse(time_order.begin(), time_order.end()); 

            for (int newTimeStart : time_order) {
                double LB_cost = 0, UB_cost = 0; 
                obj.Insert_no_care_UB(instance, i, newTimeStart, ACLB, VLUB, LB_cost, UB_cost);

                bool ok = true; 
                if ( obj.LBResources_cost > 1e-4 || obj.UBResources_cost > 1e-4 ) ok = false;
                if (!ok) {
                    obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
                    continue; 
                }

                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    bestScore = newScore;
                    bestTimeStart = newTimeStart;
                }
                obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
            }

            double LB_cost = 0, UB_cost = 0;
            obj.Insert_no_care_UB(instance, i, bestTimeStart, ACLB, VLUB, LB_cost, UB_cost);
            if (bestTimeStart != oldTimeStart) returnVal = true;

            for (auto preFoo : V) {
                if (preFoo == foo) break;
                int j = preFoo.second;
                
                /// swap i and j
                int time_i = obj.Time_Start_Intervention[i];
                int time_j = obj.Time_Start_Intervention[j];
                if (time_j > instance.tmax[i] || time_i > instance.tmax[j]) continue;

                /// check exclusion constraint 
                bool ok_exclusion = true;
                for (auto foo : instance.Exclusion_list[i][time_j]) {
                    if (!ok_exclusion) break;
                    int k = foo.first, t = foo.second, cur_t = obj.Time_Start_Intervention[k];
                    if (k == j) cur_t = time_i;
                    if (cur_t == t) ok_exclusion = false;
                }
                for (auto foo : instance.Exclusion_list[j][time_i]) {
                    if (!ok_exclusion) break;
                    int k = foo.first, t = foo.second, cur_t = obj.Time_Start_Intervention[k];
                    if (k == i) cur_t = time_j;
                    if (cur_t == t) ok_exclusion = false;
                }
                if (!ok_exclusion) continue;

                /// check resource constraint 
                obj.Erase_Only_Resources(instance, obj, i, time_i);
                obj.Erase_Only_Resources(instance, obj, j, time_j);
                obj.Insert_Only_Resources(instance, obj, i, time_j);
                obj.Insert_Only_Resources(instance, obj, j, time_i);
                
                bool ok_resource = true;
                if (obj.LBResources_cost > 1e-5 || obj.UBResources_cost > 1e-5) ok_resource = false;
                
                obj.Erase_Only_Resources(instance, obj, i, time_j);
                obj.Erase_Only_Resources(instance, obj, j, time_i);
                obj.Insert_Only_Resources(instance, obj, i, time_i);
                obj.Insert_Only_Resources(instance, obj, j, time_j);
                
                if (!ok_resource) continue;

                /// compare score
                int nAC = 0, nVL = 0;
                double costLB = 0, costUB = 0;
                obj.Erase_no_care_UB(instance, i, time_i, nAC, nVL);
                obj.Erase_no_care_UB(instance, j, time_j, nAC, nVL);
                obj.Insert_no_care_UB(instance, i, time_j, nAC, nVL, costLB, costUB);
                obj.Insert_no_care_UB(instance, j, time_i, nAC, nVL, costLB, costUB);
                
                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    bestScore = newScore;
                    returnVal = true;
                    continue;
                }

                obj.Erase_no_care_UB(instance, i, time_j, nAC, nVL);
                obj.Erase_no_care_UB(instance, j, time_i, nAC, nVL);
                obj.Insert_no_care_UB(instance, i, time_i, nAC, nVL, costLB, costUB);
                obj.Insert_no_care_UB(instance, j, time_j, nAC, nVL, costLB, costUB);
            } 
        }
        return returnVal;
    }

    bool opt2(Problem_Instance &instance, NLS_object &obj, double alpha, double beta, double to_per, double diff = 0.05) {
        double bestScore = obj.get_OBJ(instance);
        double oldScore = bestScore;
        int best_i = -1, best_j = -1, time_i = -1, time_j = -1;

        for (int i = 1; i <= instance.numInterventions; ++i) {
            int percent = rand() % 101;
            if ( percent > to_per ) continue;
            if ( !obj.Time_Start_Intervention[i] ) continue;
            
            for (int new_i = 1; new_i <= instance.tmax[i]; ++new_i) {
                if ( new_i + instance.delta[i][new_i] > instance.T + 1 ) continue;

                int nAC = 0, nVL = 0, old_i = obj.Time_Start_Intervention[i];
                double costLB = 0, costUB = 0;
                obj.Erase_no_care_UB(instance, i, old_i, nAC, nVL);
                obj.Insert_no_care_UB(instance, i, new_i, nAC, nVL, costLB, costUB);

                for (int j = 1; j < i; ++j) 
                for (int new_j = 1; new_j <= instance.tmax[j]; ++new_j) {
                    if ( new_j + instance.delta[j][new_j] > instance.T + 1 ) continue;

                    int old_j = obj.Time_Start_Intervention[j];
                    obj.Erase_no_care_UB(instance, j, old_j, nAC, nVL);
                    obj.Insert_no_care_UB(instance, j, new_j, nAC, nVL, costLB, costUB);

                    if ( !obj.exclusionChecking(instance, i, new_i) || !obj.exclusionChecking(instance, j, new_j) ) {
                        obj.Erase_no_care_UB(instance, j, new_j, nAC, nVL);
                        obj.Insert_no_care_UB(instance, j, old_j, nAC, nVL, costLB, costUB);
                        continue;
                    }

                    if ( obj.LBResources_cost < 1e-5 && obj.UBResources_cost < 1e-5 ) {
                        double newScore = obj.get_OBJ(instance);
                        if ( newScore + diff < bestScore ) {
                            bestScore = newScore;
                            best_i = i; best_j = j;
                            time_i = new_i; time_j = new_j;
                        }
                    }

                    obj.Erase_no_care_UB(instance, j, new_j, nAC, nVL);
                    obj.Insert_no_care_UB(instance, j, old_j, nAC, nVL, costLB, costUB);
                }

                obj.Erase_no_care_UB(instance, i, new_i, nAC, nVL);
                obj.Insert_no_care_UB(instance, i, old_i, nAC, nVL, costLB, costUB);
            }

            assert( abs(oldScore - obj.get_OBJ(instance)) <= 1e-7 );
            assert( obj.numFailedIntervention == 0 );
            assert( obj.UBResources_cost < 1e-6 );
            assert( obj.LBResources_cost < 1e-6 );
        }

        if (best_i == -1) return false;
        
        int nAC = 0, nVL = 0;
        double costLB = 0, costUB = 0;
        obj.Erase_no_care_UB(instance, best_i, obj.Time_Start_Intervention[best_i], nAC, nVL);
        obj.Erase_no_care_UB(instance, best_j, obj.Time_Start_Intervention[best_j], nAC, nVL);
        obj.Insert_no_care_UB(instance, best_i, time_i, nAC, nVL, costLB, costUB);
        obj.Insert_no_care_UB(instance, best_j, time_j, nAC, nVL, costLB, costUB);

            if ( abs(bestScore-obj.get_OBJ(instance)) > 1e-7 ) {
                cerr << "HUHU: \n";
                cerr << setprecision(10) << fixed << bestScore << '\n';
                cerr << setprecision(10) << fixed << obj.get_OBJ(instance) << '\n'; 
                exit(0);
            }
            assert( abs(bestScore-obj.get_OBJ(instance)) <= 1e-7 );

        return true;
    }

    bool opt2_version2(Problem_Instance &instance, NLS_object &obj, double alpha, double beta, double to_per, int size_top, double diff = 0.05) {
            assert( obj.numFailedIntervention == 0 );
            assert( obj.LBResources_cost <= 1e-6 );
            assert( obj.UBResources_cost <= 1e-6 );
                    
        int best_i = -1, best_j = -1, time_i = -1, time_j = -1;
        double bestScore = obj.get_OBJ(instance);
        double oldScore = bestScore;
            
        for (int i = 1; i <= instance.numInterventions; ++i) {
                // cerr << "# " << i << '\n';
            int percent = rand() % 101;
            if (percent > to_per) continue;

            int nAC = 0, nVL = 0;
            double costLB = 0, costUB = 0;

            int old_i = obj.Time_Start_Intervention[i];
            obj.Erase_no_care_UB(instance, i, old_i, nAC, nVL);

            for (int j = 1; j < i; ++j) {
                int old_j = obj.Time_Start_Intervention[j];
                obj.Erase_no_care_UB(instance, j, old_j, nAC, nVL);

                /// create top_i, top_j
                vector<pair<double, int> > top_i;
                for (int new_i = 1; new_i <= instance.tmax[i]; ++new_i) {
                    if ( new_i + instance.delta[i][new_i] > instance.T + 1 ) continue;
                    if ( !obj.exclusionChecking(instance, i, new_i) ) continue;

                    /// resources constraint
                    bool ok = obj.check_resource_constraint(instance, obj, i, new_i); 
                    if (!ok) continue;

                    /// get score   
                    obj.Insert_no_care_UB(instance, i, new_i, nAC, nVL, costLB, costUB);
                    double score = obj.get_OBJ(instance);
                    top_i.push_back( make_pair(score, new_i) );
                    obj.Erase_no_care_UB(instance, i, new_i, nAC, nVL);
                }

                vector<pair<double, int> > top_j;
                for (int new_j = 1; new_j <= instance.tmax[j]; ++new_j) {
                    if ( new_j + instance.delta[j][new_j] > instance.T + 1 ) continue;
                    if ( !obj.exclusionChecking(instance, j, new_j) ) continue;

                    /// resources constraint
                    bool ok = obj.check_resource_constraint(instance, obj, j, new_j); 
                    if (!ok) continue;

                    /// get score
                    obj.Insert_no_care_UB(instance, j, new_j, nAC, nVL, costLB, costUB);
                    double score = obj.get_OBJ(instance);
                    top_j.push_back( make_pair(score, new_j) );
                    obj.Erase_no_care_UB(instance, j, new_j, nAC, nVL);
                }

                /// resize top_i, top_j
                sort(top_i.begin(), top_i.end());
                sort(top_j.begin(), top_j.end());
                while ( (int) top_i.size() > size_top ) top_i.pop_back();
                while ( (int) top_j.size() > size_top ) top_j.pop_back();
 
                /// find best state
                for (auto foo_i : top_i) for (auto foo_j : top_j) {   
                    int new_i = foo_i.second, new_j = foo_j.second;

                    /// exclusion constraint 
                    bool ok_exclusion = true;
                    for (auto foo : instance.Exclusion_list[i][new_i]) {
                        if (!ok_exclusion) break;
                        int k = foo.first, t = foo.second, cur_t = obj.Time_Start_Intervention[k];
                        if (k == j) cur_t = new_j;
                        if (cur_t == t) ok_exclusion = false;
                    }
                    for (auto foo : instance.Exclusion_list[j][new_j]) {
                        if (!ok_exclusion) break;
                        int k = foo.first, t = foo.second, cur_t = obj.Time_Start_Intervention[k];
                        if (k == i) cur_t = new_i;
                        if (cur_t == t) ok_exclusion = false;
                    }
                    if (!ok_exclusion) continue; 

                    /// resource constraint
                    bool ok_resource = true;
                    obj.Insert_Only_Resources(instance, obj, i, new_i);
                    if ( !obj.check_resource_constraint(instance, obj, j, new_j) ) ok_resource = false;
                    obj.Erase_Only_Resources(instance, obj, i, new_i);
                    if (!ok_resource) continue;

                    /// get score
                    obj.Insert_no_care_UB(instance, i, new_i, nAC, nVL, costLB, costUB);
                    obj.Insert_no_care_UB(instance, j, new_j, nAC, nVL, costLB, costUB);
                    assert( obj.LBResources_cost < 1e-5 );
                    assert( obj.UBResources_cost < 1e-5 );
                    assert( obj.numFailedIntervention == 0 );

                    double newScore = obj.get_OBJ(instance);
                    if ( newScore < bestScore ) {
                        // cerr << "+";
                        bestScore = newScore;
                        best_i = i; best_j = j;
                        time_i = new_i; time_j = new_j;

                            // cerr << "??? " << best_i << "(" << time_i << ")" << " " << best_j << "(" << time_j << ") --> " << newScore << '\n';
                    }

                    obj.Erase_no_care_UB(instance, i, new_i, nAC, nVL);
                    obj.Erase_no_care_UB(instance, j, new_j, nAC, nVL);
                }

                obj.Insert_no_care_UB(instance, j, old_j, nAC, nVL, costLB, costUB);
            }

                    // cerr << "T_T: " << best_i <<  " " << time_i <<  " " << best_j << " " << time_j << '\n';
            
            obj.Insert_no_care_UB(instance, i, old_i, nAC, nVL, costLB, costUB);

                if ( abs(obj.get_OBJ(instance) - oldScore) > 1e-6 ) {
                    cerr << setprecision(10) << fixed << oldScore << '\n';
                    cerr << setprecision(10) << fixed << obj.get_OBJ(instance) << '\n';
                    exit(0);
                }
                assert( abs(obj.get_OBJ(instance) - oldScore) <= 1e-6 );
        }

            // cerr << "T_T: " << best_i <<  " " << time_i <<  " " << best_j << " " << time_j << '\n';
            assert( abs(obj.get_OBJ(instance) - oldScore) <= 1e-6 );
        
        if (best_i == -1) return false;
        if ( bestScore + diff > oldScore ) return false;

            // cerr << "vcc: " << best_i << " " << time_i << '\n';
            
            if ( bestScore - obj.get_OBJ(instance) > 1e-7 ) {
                cerr << "NOOO\n";
                cerr << setprecision(10) << fixed << bestScore << " ";
                cerr << setprecision(10) << fixed << obj.get_OBJ(instance) << " ";
                cerr << '\n'; 
            }
            assert( bestScore - obj.get_OBJ(instance) <= 1e-7 );

            // cerr << "plz: " << best_i << " " << time_i << '\n';

        int nAC = 0, nVL = 0;
        double costLB = 0, costUB = 0;
        obj.Erase_no_care_UB(instance, best_i, obj.Time_Start_Intervention[best_i], nAC, nVL);
        obj.Erase_no_care_UB(instance, best_j, obj.Time_Start_Intervention[best_j], nAC, nVL);
        obj.Insert_no_care_UB(instance, best_i, time_i, nAC, nVL, costLB, costUB);
        obj.Insert_no_care_UB(instance, best_j, time_j, nAC, nVL, costLB, costUB);  
            
            if ( abs(bestScore - obj.get_OBJ(instance)) > 1e-6 ) {
                cerr << "WA!!!: ";
                cerr << best_i << "(" << time_i << "), " << best_j << "(" << time_j << ")\n"; 
                cerr << setprecision(10) << fixed << bestScore << " --> ";
                cerr << setprecision(10) << fixed << obj.get_OBJ(instance) << "\n";
                exit(0);
            }
            assert( abs(bestScore - obj.get_OBJ(instance)) <= 1e-6 );
            assert( obj.numFailedIntervention == 0 );
            assert( obj.LBResources_cost <= 1e-7 );
            assert( obj.UBResources_cost <= 1e-7 );

        return true;
    }
    
    double k_exchange_brute(Problem_Instance &instance, NLS_object &obj, vector<pair<int, int> >V, 
        vector<int> type, vector<int> &ans, int curID, int cnt, int chlen, int diff, double &bestScore ) {

        if (cnt == 0) {
            double oldScore = obj.get_OBJ(instance);

            int r = curID-1, l = r - (int) ans.size() + 1;
            assert(0 <= l && l <= r && r < (int) V.size());
            for (int id = l; id <= r; ++id) {
                int i = V[id].second;

                int nAC = 0, nVL = 0, newTime = obj.Time_Start_Intervention[i] + type[id-l];
                double costLB = 0, costUB = 0;
                obj.Erase_no_care_UB(instance, i, obj.Time_Start_Intervention[i], nAC, nVL);
                obj.Insert_no_care_UB(instance, i, newTime, nAC, nVL, costLB, costUB);
            }

            bool ok = true;
            assert( obj.numFailedIntervention == 0 );
            if ( obj.LBResources_cost > 1e-5 || obj.UBResources_cost > 1e-5 ) ok = false;
            if ( ok ) {
                for (int id = l; id <= r; ++id) {
                    int i = V[id].second;
                    if ( !obj.exclusionChecking(instance, i, obj.Time_Start_Intervention[i]) ) { ok = false; break; }
                }
            }  

            double newScore = obj.get_OBJ(instance);
            if ( ok && newScore + 1e-9 < bestScore ) {
                bestScore = newScore;
                for (int i = 0; i < (int) ans.size(); ++i) ans[i] = type[i];
            }

            for (int id = l; id <= r; ++id) {
                int i = V[id].second;

                int nAC = 0, nVL = 0, oldTime = obj.Time_Start_Intervention[i] - type[id-l];
                double costLB = 0, costUB = 0;
                obj.Erase_no_care_UB(instance, i, obj.Time_Start_Intervention[i], nAC, nVL);
                obj.Insert_no_care_UB(instance, i, oldTime, nAC, nVL, costLB, costUB);
            }

            assert( abs(obj.get_OBJ(instance) - oldScore) <= 1e-6 );

            // if (ok) 
            return bestScore;
            // return (double) 1e9 + 7;
        }

        int i = V[curID].second;
        double score = (double) bestScore;
        for (int dir = -chlen; dir <= chlen; ++dir) {
            int t = obj.Time_Start_Intervention[i] + dir;
            if ( dir == 0 || t <= 0 || t > instance.tmax[i] ) continue;
            if ( t + instance.delta[i][t] > instance.T + 1 ) continue;

            int tt = (int) ans.size() - cnt;
            type[tt] = dir; 
            score = min( score, k_exchange_brute(instance, obj, V, type, ans, curID+1, cnt-1, chlen, diff, bestScore) );
            score = min( score, bestScore );
        }
        return score;
    }
    bool dp_with_k_exchange(Problem_Instance &instance, NLS_object &obj, double alpha, double beta, int sz_k, int chlen, double diff = 0.05) {
        vector<pair<int, int> > V;
        for (int i = 1; i <= instance.numInterventions; ++i) { 
            int start_time = obj.Time_Start_Intervention[i];
            int end_time = start_time + instance.delta[i][start_time];
            V.push_back( make_pair(end_time, i) );   
        }
        sort(V.begin(), V.end());
        
        int curID = -1;
        bool returnVal = false;
        for (auto foo : V) {
            ++curID;
            int i = foo.second;
            if ( !obj.Time_Start_Intervention[i] ) continue;

            int oldTimeStart = obj.Time_Start_Intervention[i], bestTimeStart = oldTimeStart;
            double bestScore = obj.get_OBJ(instance);
            int ACLB = 0, VLUB = 0;
            obj.Erase_no_care_UB(instance, i, oldTimeStart, ACLB, VLUB);

            vector<int> time_order;
            for (int newTimeStart = 1; newTimeStart <= instance.tmax[i]; ++newTimeStart) {
                if ( newTimeStart == oldTimeStart ) continue;
                if ( !obj.exclusionChecking(instance, i, newTimeStart) ) continue;
                time_order.push_back(newTimeStart);
            }

            int percent = rand() % 100;
            if (percent < 50) reverse(time_order.begin(), time_order.end()); 

            for (int newTimeStart : time_order) {
                double LB_cost = 0, UB_cost = 0; 
                obj.Insert_no_care_UB(instance, i, newTimeStart, ACLB, VLUB, LB_cost, UB_cost);

                bool ok = true; 
                if ( obj.LBResources_cost > 1e-4 || obj.UBResources_cost > 1e-4 ) ok = false;
                if (!ok) {
                    obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
                    continue; 
                }

                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    //cerr << "^";
                    bestScore = newScore;
                    bestTimeStart = newTimeStart;
                }
                obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
            }

            double LB_cost = 0, UB_cost = 0;
            obj.Insert_no_care_UB(instance, i, bestTimeStart, ACLB, VLUB, LB_cost, UB_cost);
            if (bestTimeStart != oldTimeStart) returnVal = true;

            /// SWAP
            for (auto preFoo : V) {
                if (preFoo == foo) break;
                int j = preFoo.second;
                
                /// swap i and j
                int time_i = obj.Time_Start_Intervention[i];
                int time_j = obj.Time_Start_Intervention[j];
                if (time_j > instance.tmax[i] || time_i > instance.tmax[j]) continue;

                /// check exclusion constraint 
                bool ok_exclusion = true;
                for (auto foo : instance.Exclusion_list[i][time_j]) {
                    if (!ok_exclusion) break;
                    int k = foo.first, t = foo.second, cur_t = obj.Time_Start_Intervention[k];
                    if (k == j) cur_t = time_i;
                    if (cur_t == t) ok_exclusion = false;
                }
                for (auto foo : instance.Exclusion_list[j][time_i]) {
                    if (!ok_exclusion) break;
                    int k = foo.first, t = foo.second, cur_t = obj.Time_Start_Intervention[k];
                    if (k == i) cur_t = time_j;
                    if (cur_t == t) ok_exclusion = false;
                }
                if (!ok_exclusion) continue;

                /// check resource constraint 
                obj.Erase_Only_Resources(instance, obj, i, time_i);
                obj.Erase_Only_Resources(instance, obj, j, time_j);
                obj.Insert_Only_Resources(instance, obj, i, time_j);
                obj.Insert_Only_Resources(instance, obj, j, time_i);
                
                bool ok_resource = true;
                if (obj.LBResources_cost > 1e-5 || obj.UBResources_cost > 1e-5) ok_resource = false;
                
                obj.Erase_Only_Resources(instance, obj, i, time_j);
                obj.Erase_Only_Resources(instance, obj, j, time_i);
                obj.Insert_Only_Resources(instance, obj, i, time_i);
                obj.Insert_Only_Resources(instance, obj, j, time_j);
                
                if (!ok_resource) continue;

                /// compare score
                int nAC = 0, nVL = 0;
                double costLB = 0, costUB = 0;
                obj.Erase_no_care_UB(instance, i, time_i, nAC, nVL);
                obj.Erase_no_care_UB(instance, j, time_j, nAC, nVL);
                obj.Insert_no_care_UB(instance, i, time_j, nAC, nVL, costLB, costUB);
                obj.Insert_no_care_UB(instance, j, time_i, nAC, nVL, costLB, costUB);
                
                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    //cerr << "~";
                    bestScore = newScore;
                    returnVal = true;
                    continue;
                }

                obj.Erase_no_care_UB(instance, i, time_j, nAC, nVL);
                obj.Erase_no_care_UB(instance, j, time_i, nAC, nVL);
                obj.Insert_no_care_UB(instance, i, time_i, nAC, nVL, costLB, costUB);
                obj.Insert_no_care_UB(instance, j, time_j, nAC, nVL, costLB, costUB);
            } 

            /// EXCHANGE
            if ( curID - sz_k + 1 < 0 ) continue;

            vector<int> type, ans;
            type.resize(sz_k, 0); ans.resize(sz_k, 0);

            double copyScore = bestScore;
            double score_exchange = k_exchange_brute(instance, obj, V, type, ans, curID-sz_k+1, sz_k, chlen, diff, copyScore );

                if ( abs(score_exchange - copyScore) > 1e-5 ) {
                    cerr << setprecision(10) << fixed << score_exchange << '\n';
                    cerr << setprecision(10) << fixed << copyScore << '\n';
                    exit(0);
                }

            if (score_exchange + diff < bestScore) {
                    cerr << "*";
                    // cerr << bestScore << " " << score_exchange;
                    //cerr << bestScore << " ";

                bestScore = score_exchange;
                for (int id = curID-sz_k+1; id <= curID; ++id) {
                    int inter = V[id].second;
                    int oldTime = obj.Time_Start_Intervention[inter];
                    int newTime = oldTime + ans[id-curID+sz_k-1];

                    int nAC = 0, nVL = 0;
                    double costLB = 0, costUB = 0;
                    obj.Erase_no_care_UB(instance, inter, oldTime, nAC, nVL);
                    obj.Insert_no_care_UB(instance, inter, newTime, nAC, nVL, costLB, costUB);
                }
                returnVal = true;
                    
                    // for (int i = 1; i <= instance.numInterventions; ++i) {
                    //     if ( !obj.exclusionChecking(instance, i, obj.Time_Start_Intervention[i]) ) {
                    //         assert(10 == 11);
                    //     }
                    // }
                    // assert( obj.numFailedIntervention == 0 );
                    // assert( obj.LBResources_cost < 1e-5 );
                    // assert( obj.UBResources_cost < 1e-5 );
                    // assert( abs(obj.get_OBJ(instance)-score_exchange) <= 1e-5 );
                    // cerr << " --> " << bestScore << " " << obj.get_OBJ(instance) << '\n';
            }
        }
        return returnVal;
    }

    //all LS
    bool all_LS(Problem_Instance &instance, NLS_object &obj, double alpha, double beta, double diff = 0.05) {
        vector<pair<int, int> > V;
        for (int i = 1; i <= instance.numInterventions; ++i) { 
            int start_time = obj.Time_Start_Intervention[i];
            int end_time = start_time + instance.delta[i][start_time];
            V.push_back( make_pair(end_time, i) );   
        }
        sort(V.begin(), V.end());
        
        int curID = -1;
        bool returnVal = false;
        for (auto foo : V) {
            ++curID;
            int i = foo.second;
            if ( !obj.Time_Start_Intervention[i] ) continue;

            int oldTimeStart = obj.Time_Start_Intervention[i], bestTimeStart = oldTimeStart;
            double bestScore = obj.get_OBJ(instance);
            int ACLB = 0, VLUB = 0;
            obj.Erase_no_care_UB(instance, i, oldTimeStart, ACLB, VLUB);

            vector<int> time_order;
            for (int newTimeStart = 1; newTimeStart <= instance.tmax[i]; ++newTimeStart) {
                if ( newTimeStart == oldTimeStart ) continue;
                if ( !obj.exclusionChecking(instance, i, newTimeStart) ) continue;
                time_order.push_back(newTimeStart);
            }

            int percent = rand() % 100;
            if (percent < 50) reverse(time_order.begin(), time_order.end()); 

            for (int newTimeStart : time_order) {
                double LB_cost = 0, UB_cost = 0; 
                obj.Insert_no_care_UB(instance, i, newTimeStart, ACLB, VLUB, LB_cost, UB_cost);

                bool ok = true; 
                if ( obj.LBResources_cost > 1e-4 || obj.UBResources_cost > 1e-4 ) ok = false;
                if (!ok) {
                    obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
                    continue; 
                }

                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    //cerr << "^";
                    bestScore = newScore;
                    bestTimeStart = newTimeStart;
                }
                obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
            }

            double LB_cost = 0, UB_cost = 0;
            obj.Insert_no_care_UB(instance, i, bestTimeStart, ACLB, VLUB, LB_cost, UB_cost);
            if (bestTimeStart != oldTimeStart) returnVal = true;

            /// SWAP
            for (auto preFoo : V) {
                if (preFoo == foo) break;
                int j = preFoo.second;
                
                /// swap i and j
                int time_i = obj.Time_Start_Intervention[i];
                int time_j = obj.Time_Start_Intervention[j];
                if (time_j > instance.tmax[i] || time_i > instance.tmax[j]) continue;

                /// check exclusion constraint 
                bool ok_exclusion = true;
                for (auto foo : instance.Exclusion_list[i][time_j]) {
                    if (!ok_exclusion) break;
                    int k = foo.first, t = foo.second, cur_t = obj.Time_Start_Intervention[k];
                    if (k == j) cur_t = time_i;
                    if (cur_t == t) ok_exclusion = false;
                }
                for (auto foo : instance.Exclusion_list[j][time_i]) {
                    if (!ok_exclusion) break;
                    int k = foo.first, t = foo.second, cur_t = obj.Time_Start_Intervention[k];
                    if (k == i) cur_t = time_j;
                    if (cur_t == t) ok_exclusion = false;
                }
                if (!ok_exclusion) continue;

                /// check resource constraint 
                obj.Erase_Only_Resources(instance, obj, i, time_i);
                obj.Erase_Only_Resources(instance, obj, j, time_j);
                obj.Insert_Only_Resources(instance, obj, i, time_j);
                obj.Insert_Only_Resources(instance, obj, j, time_i);
                
                bool ok_resource = true;
                if (obj.LBResources_cost > 1e-5 || obj.UBResources_cost > 1e-5) ok_resource = false;
                
                obj.Erase_Only_Resources(instance, obj, i, time_j);
                obj.Erase_Only_Resources(instance, obj, j, time_i);
                obj.Insert_Only_Resources(instance, obj, i, time_i);
                obj.Insert_Only_Resources(instance, obj, j, time_j);
                
                if (!ok_resource) continue;

                /// compare score
                int nAC = 0, nVL = 0;
                double costLB = 0, costUB = 0;
                obj.Erase_no_care_UB(instance, i, time_i, nAC, nVL);
                obj.Erase_no_care_UB(instance, j, time_j, nAC, nVL);
                obj.Insert_no_care_UB(instance, i, time_j, nAC, nVL, costLB, costUB);
                obj.Insert_no_care_UB(instance, j, time_i, nAC, nVL, costLB, costUB);
                
                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    //cerr << "~";
                    bestScore = newScore;
                    returnVal = true;
                    continue;
                }

                obj.Erase_no_care_UB(instance, i, time_j, nAC, nVL);
                obj.Erase_no_care_UB(instance, j, time_i, nAC, nVL);
                obj.Insert_no_care_UB(instance, i, time_i, nAC, nVL, costLB, costUB);
                obj.Insert_no_care_UB(instance, j, time_j, nAC, nVL, costLB, costUB);
            } 

            /// EXCHANGE sz_k = 4, int chlen = 1, 
            int sz_k = 4, chlen = 1; 
            if ( curID - sz_k + 1 < 0 ) continue;

            vector<int> type, ans;
            type.resize(sz_k, 0); ans.resize(sz_k, 0);

            double copyScore = bestScore;
            double score_exchange = k_exchange_brute(instance, obj, V, type, ans, curID-sz_k+1, sz_k, chlen, diff, copyScore );

            if (score_exchange + diff < bestScore) {
                    cerr << "*";
                    // cerr << bestScore << " " << score_exchange;
                    //cerr << bestScore << " ";

                bestScore = score_exchange;
                for (int id = curID-sz_k+1; id <= curID; ++id) {
                    int inter = V[id].second;
                    int oldTime = obj.Time_Start_Intervention[inter];
                    int newTime = oldTime + ans[id-curID+sz_k-1];

                    int nAC = 0, nVL = 0;
                    double costLB = 0, costUB = 0;
                    obj.Erase_no_care_UB(instance, inter, oldTime, nAC, nVL);
                    obj.Insert_no_care_UB(instance, inter, newTime, nAC, nVL, costLB, costUB);
                }
                returnVal = true;
                    
                    for (int i = 1; i <= instance.numInterventions; ++i) {
                        if ( !obj.exclusionChecking(instance, i, obj.Time_Start_Intervention[i]) ) {
                            assert(10 == 11);
                        }
                    }
                    assert( obj.numFailedIntervention == 0 );
                    assert( obj.LBResources_cost < 1e-5 );
                    assert( obj.UBResources_cost < 1e-5 );
                    assert( abs(obj.get_OBJ(instance)-score_exchange) <= 1e-5 );
                    // cerr << " --> " << bestScore << " " << obj.get_OBJ(instance) << '\n';
            }


            /// EXCHANGE sz_k = 2, int chlen = 3, 
            sz_k = 2; chlen = 3; 
            if ( curID - sz_k + 1 < 0 ) continue;

            type.resize(sz_k, 0); ans.resize(sz_k, 0);

            copyScore = bestScore;
            score_exchange = k_exchange_brute(instance, obj, V, type, ans, curID-sz_k+1, sz_k, chlen, diff, copyScore );

            if (score_exchange + diff < bestScore) {
                    cerr << "~";
                    // cerr << bestScore << " " << score_exchange;
                    //cerr << bestScore << " ";

                bestScore = score_exchange;
                for (int id = curID-sz_k+1; id <= curID; ++id) {
                    int inter = V[id].second;
                    int oldTime = obj.Time_Start_Intervention[inter];
                    int newTime = oldTime + ans[id-curID+sz_k-1];

                    int nAC = 0, nVL = 0;
                    double costLB = 0, costUB = 0;
                    obj.Erase_no_care_UB(instance, inter, oldTime, nAC, nVL);
                    obj.Insert_no_care_UB(instance, inter, newTime, nAC, nVL, costLB, costUB);
                }
                returnVal = true;
                    
                    for (int i = 1; i <= instance.numInterventions; ++i) {
                        if ( !obj.exclusionChecking(instance, i, obj.Time_Start_Intervention[i]) ) {
                            assert(10 == 11);
                        }
                    }
                    assert( obj.numFailedIntervention == 0 );
                    assert( obj.LBResources_cost < 1e-5 );
                    assert( obj.UBResources_cost < 1e-5 );
                    assert( abs(obj.get_OBJ(instance)-score_exchange) <= 1e-5 );
                    // cerr << " --> " << bestScore << " " << obj.get_OBJ(instance) << '\n';
            }
        }
        return returnVal;
    }

    bool all_LS_ver2_for_each_ant(Problem_Instance &instance, NLS_object &obj, double alpha, double beta, double diff = 0.05) {
        vector<pair<int, int> > V;
        for (int i = 1; i <= instance.numInterventions; ++i) { 
            int start_time = obj.Time_Start_Intervention[i];
            int end_time = start_time + instance.delta[i][start_time];
            V.push_back( make_pair(end_time, i) );   
        }
        sort(V.begin(), V.end());
        
        int curID = -1;
        bool returnVal = false;
        for (auto foo : V) {
            ++curID;
            int i = foo.second;
            if ( !obj.Time_Start_Intervention[i] ) continue;

            int oldTimeStart = obj.Time_Start_Intervention[i], bestTimeStart = oldTimeStart;
            double bestScore = obj.get_OBJ(instance);
            int ACLB = 0, VLUB = 0;
            obj.Erase_no_care_UB(instance, i, oldTimeStart, ACLB, VLUB);

            vector<int> time_order;
            for (int newTimeStart = 1; newTimeStart <= instance.tmax[i]; ++newTimeStart) {
                if ( newTimeStart == oldTimeStart ) continue;
                if ( !obj.exclusionChecking(instance, i, newTimeStart) ) continue;
                time_order.push_back(newTimeStart);
            }

            int percent = rand() % 100;
            if (percent < 50) reverse(time_order.begin(), time_order.end()); 

            for (int newTimeStart : time_order) {
                double LB_cost = 0, UB_cost = 0; 
                obj.Insert_no_care_UB(instance, i, newTimeStart, ACLB, VLUB, LB_cost, UB_cost);

                bool ok = true; 
                if ( obj.LBResources_cost > 1e-4 || obj.UBResources_cost > 1e-4 ) ok = false;
                if (!ok) {
                    obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
                    continue; 
                }

                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    //cerr << "^";
                    bestScore = newScore;
                    bestTimeStart = newTimeStart;
                }
                obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
            }

            double LB_cost = 0, UB_cost = 0;
            obj.Insert_no_care_UB(instance, i, bestTimeStart, ACLB, VLUB, LB_cost, UB_cost);
            if (bestTimeStart != oldTimeStart) returnVal = true;

            /// SWAP
            for (auto preFoo : V) {
                if (preFoo == foo) break;
                int j = preFoo.second;
                
                /// swap i and j
                int time_i = obj.Time_Start_Intervention[i];
                int time_j = obj.Time_Start_Intervention[j];
                if (time_j > instance.tmax[i] || time_i > instance.tmax[j]) continue;

                /// check exclusion constraint 
                bool ok_exclusion = true;
                for (auto foo : instance.Exclusion_list[i][time_j]) {
                    if (!ok_exclusion) break;
                    int k = foo.first, t = foo.second, cur_t = obj.Time_Start_Intervention[k];
                    if (k == j) cur_t = time_i;
                    if (cur_t == t) ok_exclusion = false;
                }
                for (auto foo : instance.Exclusion_list[j][time_i]) {
                    if (!ok_exclusion) break;
                    int k = foo.first, t = foo.second, cur_t = obj.Time_Start_Intervention[k];
                    if (k == i) cur_t = time_j;
                    if (cur_t == t) ok_exclusion = false;
                }
                if (!ok_exclusion) continue;

                /// check resource constraint 
                obj.Erase_Only_Resources(instance, obj, i, time_i);
                obj.Erase_Only_Resources(instance, obj, j, time_j);
                obj.Insert_Only_Resources(instance, obj, i, time_j);
                obj.Insert_Only_Resources(instance, obj, j, time_i);
                
                bool ok_resource = true;
                if (obj.LBResources_cost > 1e-5 || obj.UBResources_cost > 1e-5) ok_resource = false;
                
                obj.Erase_Only_Resources(instance, obj, i, time_j);
                obj.Erase_Only_Resources(instance, obj, j, time_i);
                obj.Insert_Only_Resources(instance, obj, i, time_i);
                obj.Insert_Only_Resources(instance, obj, j, time_j);
                
                if (!ok_resource) continue;

                /// compare score
                int nAC = 0, nVL = 0;
                double costLB = 0, costUB = 0;
                obj.Erase_no_care_UB(instance, i, time_i, nAC, nVL);
                obj.Erase_no_care_UB(instance, j, time_j, nAC, nVL);
                obj.Insert_no_care_UB(instance, i, time_j, nAC, nVL, costLB, costUB);
                obj.Insert_no_care_UB(instance, j, time_i, nAC, nVL, costLB, costUB);
                
                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    //cerr << "~";
                    bestScore = newScore;
                    returnVal = true;
                    continue;
                }

                obj.Erase_no_care_UB(instance, i, time_j, nAC, nVL);
                obj.Erase_no_care_UB(instance, j, time_i, nAC, nVL);
                obj.Insert_no_care_UB(instance, i, time_i, nAC, nVL, costLB, costUB);
                obj.Insert_no_care_UB(instance, j, time_j, nAC, nVL, costLB, costUB);
            } 

            /// EXCHANGE sz_k = 3, int chlen = 1, 
            int sz_k = 3, chlen = 1; 
            if ( curID - sz_k + 1 >= 0 ) {
                vector<int> type, ans;
                type.resize(sz_k, 0); ans.resize(sz_k, 0);

                double copyScore = bestScore;
                double score_exchange = k_exchange_brute(instance, obj, V, type, ans, curID-sz_k+1, sz_k, chlen, diff, copyScore );
                    assert( abs(score_exchange - copyScore) <= 1e-7 );

                if (score_exchange + diff < bestScore) {
                        cerr << "*";
                        //cerr << bestScore << " " << score_exchange;
                        //cerr << bestScore << " ";

                    bestScore = score_exchange;
                    for (int id = curID-sz_k+1; id <= curID; ++id) {
                        int inter = V[id].second;
                        int oldTime = obj.Time_Start_Intervention[inter];
                        int newTime = oldTime + ans[id-curID+sz_k-1];

                        int nAC = 0, nVL = 0;
                        double costLB = 0, costUB = 0;
                        obj.Erase_no_care_UB(instance, inter, oldTime, nAC, nVL);
                        obj.Insert_no_care_UB(instance, inter, newTime, nAC, nVL, costLB, costUB);
                    }
                    returnVal = true;
                        
                        for (int i = 1; i <= instance.numInterventions; ++i) {
                            if ( !obj.exclusionChecking(instance, i, obj.Time_Start_Intervention[i]) ) {
                                assert(10 == 11);
                            }
                        }
                        assert( obj.numFailedIntervention == 0 );
                        assert( obj.LBResources_cost < 1e-5 );
                        assert( obj.UBResources_cost < 1e-5 );
                        assert( abs(obj.get_OBJ(instance)-score_exchange) <= 1e-5 );
                        // cerr << " --> " << bestScore << " " << obj.get_OBJ(instance) << '\n';
                }
            }


            /// EXCHANGE sz_k = 2, int chlen = 2, 
            sz_k = 2; chlen = 2; 
            if ( curID - sz_k + 1 >= 0 ) {
                vector<int> type, ans;
                type.resize(sz_k, 0); ans.resize(sz_k, 0);

                double copyScore = bestScore;
                double score_exchange = k_exchange_brute(instance, obj, V, type, ans, curID-sz_k+1, sz_k, chlen, diff, copyScore );
                    assert( abs(score_exchange - copyScore) <= 1e-7 );

                if (score_exchange + diff < bestScore) {
                        cerr << "~";

                    bestScore = score_exchange;
                    for (int id = curID-sz_k+1; id <= curID; ++id) {
                        int inter = V[id].second;
                        int oldTime = obj.Time_Start_Intervention[inter];
                        int newTime = oldTime + ans[id-curID+sz_k-1];

                        int nAC = 0, nVL = 0;
                        double costLB = 0, costUB = 0;
                        obj.Erase_no_care_UB(instance, inter, oldTime, nAC, nVL);
                        obj.Insert_no_care_UB(instance, inter, newTime, nAC, nVL, costLB, costUB);
                    }
                    returnVal = true;
                }
            }

            /// PERMUTATION sz_p = 3
            assert( abs(bestScore-obj.get_OBJ(instance)) <= 1e-6 );

            int sz_p = 3;
            if ( curID - sz_p + 1 >= 0 ) {
                bool flag_permutation = false;
                vector<int> per, oldTime, best_per;
                for (int j = curID - sz_p + 1; j <= curID; ++j) {
                    per.push_back(j - curID + sz_p - 1);
                    best_per.push_back(j - curID + sz_p - 1);
                    oldTime.push_back( obj.Time_Start_Intervention[V[j].second] );
                }

                do {
                    
                    bool cek_start_time = true;
                    int curPoint = curID - sz_p + 1;
                    for (int j = 0; j < (int) per.size(); ++j) {
                        int inter = V[curPoint].second, t = oldTime[per[j]];
                            // cerr << ":)) " << inter << " " << t << '\n';
                        if ( instance.tmax[inter] < t ) { cek_start_time = false; break; }
                        if ( t + instance.delta[inter][t] > instance.T+1 ) { cek_start_time = false; break; }   
                        ++curPoint;
                    }
                    if (!cek_start_time) continue;

                    curPoint = curID - sz_p + 1;                    
                    double oldScore = obj.get_OBJ(instance);
                    for (int j = 0; j < (int) per.size(); ++j) {
                        int inter = V[curPoint].second;

                        int nAC = 0, nVL = 0;
                        double costLB = 0, costUB = 0; 
                        obj.Erase_no_care_UB(instance, inter, obj.Time_Start_Intervention[inter], nAC, nVL);
                        obj.Insert_no_care_UB(instance, inter, oldTime[per[j]], nAC, nVL, costLB, costUB);
                        
                        ++curPoint;
                    }

                    /// check constraint and update val
                    bool ok = (obj.numFailedIntervention == 0);
                    if ( obj.LBResources_cost > 1e-6 || obj.UBResources_cost > 1e-6 ) ok = false;
                    if ( ok ) {
                        for (int j = curID-sz_p+1; j <= curID; ++j) {
                            int inter = V[j].second;
                            if ( !obj.exclusionChecking(instance, inter, obj.Time_Start_Intervention[inter]) ) {
                                ok = false; break;
                            } 
                        }
                    }
                    if (ok) {
                        double newScore = obj.get_OBJ(instance);
                        if ( newScore + diff < bestScore ) {
                            bestScore = newScore;
                            best_per = per;
                            flag_permutation = true;

                                // cerr << "clgt: ";
                                // for (int x : best_per) cerr << x << " ";
                                // cerr << "--> " << setprecision(5) << fixed << " " <<
                                //     obj.LBResources_cost << " " << obj.UBResources_cost << " " << obj.get_OBJ(instance) << '\n'; 
                        }
                    }

                    /// cumback
                    curPoint = curID - sz_p + 1;
                    for (int j = 0; j < (int) per.size(); ++j) {
                        int inter = V[curPoint].second;

                        int nAC = 0, nVL = 0;
                        double costLB = 0, costUB = 0;
                        obj.Erase_no_care_UB(instance, inter, obj.Time_Start_Intervention[inter], nAC, nVL);
                        obj.Insert_no_care_UB(instance, inter, oldTime[j], nAC, nVL, costLB, costUB);

                        ++curPoint;
                    }
                    assert( abs(obj.get_OBJ(instance) - oldScore) <= 1e-6 );

                } while (next_permutation(per.begin(), per.end()));

                if ( flag_permutation ) {
                    cerr << "@";

                    int curPoint = curID - sz_p + 1;
                    for (int j = 0; j < (int) best_per.size(); ++j) {
                        int inter = V[curPoint].second;

                        int nAC = 0, nVL = 0;
                        double costLB = 0, costUB = 0; 
                        obj.Erase_no_care_UB(instance, inter, obj.Time_Start_Intervention[inter], nAC, nVL);
                        obj.Insert_no_care_UB(instance, inter, oldTime[best_per[j]], nAC, nVL, costLB, costUB);
                        
                        ++curPoint;
                    }
                    returnVal = true;

                }

                if ( abs(obj.get_OBJ(instance)-bestScore) > 1e-6 ) {
                    cerr << "### " << setprecision(10) << fixed << obj.get_OBJ(instance) << " " << bestScore << '\n';
                    exit(0); 
                }
                assert( abs(obj.get_OBJ(instance)-bestScore) <= 1e-6 );
            }

                // just for sure 
                // for (int i = 1; i <= instance.numInterventions; ++i) {
                //     if ( !obj.exclusionChecking(instance, i, obj.Time_Start_Intervention[i]) ) {
                //         assert(10 == 11);
                //     }
                // }

                // if ( obj.UBResources_cost > 1e-6 ) {
                //     cerr << setprecision(10) << fixed << " " << obj.UBResources_cost << '\n';
                //     exit(0);
                // }
                // assert( obj.numFailedIntervention == 0 );
                // assert( obj.LBResources_cost <= 1e-6 );
                // assert( obj.UBResources_cost <= 1e-6 );
                // assert( abs(bestScore - obj.get_OBJ(instance)) <= 1e-6 );
                // cerr << " --> " << bestScore << " " << obj.get_OBJ(instance) << '\n';
        }
        return returnVal;
    }

    bool dp_with_k_exchange_ver2(Problem_Instance &instance, NLS_object &obj, double alpha, double beta, int sz_k, int chlen, int sz_p, double diff = 0.05) {
        vector<pair<int, int> > V;
        for (int i = 1; i <= instance.numInterventions; ++i) { 
            int start_time = obj.Time_Start_Intervention[i];
            int end_time = start_time + instance.delta[i][start_time];
            V.push_back( make_pair(end_time, i) );   
        }
        sort(V.begin(), V.end());
        
        int curID = -1;
        bool returnVal = false;
        for (auto foo : V) {
            ++curID;
            int i = foo.second;
            if ( !obj.Time_Start_Intervention[i] ) continue;

            int oldTimeStart = obj.Time_Start_Intervention[i], bestTimeStart = oldTimeStart;
            double bestScore = obj.get_OBJ(instance);
            int ACLB = 0, VLUB = 0;
            obj.Erase_no_care_UB(instance, i, oldTimeStart, ACLB, VLUB);

            vector<int> time_order;
            for (int newTimeStart = 1; newTimeStart <= instance.tmax[i]; ++newTimeStart) {
                if ( newTimeStart == oldTimeStart ) continue;
                if ( !obj.exclusionChecking(instance, i, newTimeStart) ) continue;
                time_order.push_back(newTimeStart);
            }

            int percent = rand() % 100;
            if (percent < 50) reverse(time_order.begin(), time_order.end()); 

            for (int newTimeStart : time_order) {
                double LB_cost = 0, UB_cost = 0; 
                obj.Insert_no_care_UB(instance, i, newTimeStart, ACLB, VLUB, LB_cost, UB_cost);

                bool ok = true; 
                assert( obj.numFailedIntervention == 0 );
                if ( obj.LBResources_cost > 1e-4 || obj.UBResources_cost > 1e-4 ) ok = false;
                if (!ok) {
                    obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
                    continue; 
                }

                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    //cerr << "^";
                    bestScore = newScore;
                    bestTimeStart = newTimeStart;
                }
                obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
            }

            double LB_cost = 0, UB_cost = 0;
            obj.Insert_no_care_UB(instance, i, bestTimeStart, ACLB, VLUB, LB_cost, UB_cost);
            if (bestTimeStart != oldTimeStart) returnVal = true;

            /// SWAP
            for (auto preFoo : V) {
                if (preFoo == foo) break;
                int j = preFoo.second;
                
                /// swap i and j
                int time_i = obj.Time_Start_Intervention[i];
                int time_j = obj.Time_Start_Intervention[j];
                if (time_j > instance.tmax[i] || time_i > instance.tmax[j]) continue;

                /// check exclusion constraint 
                bool ok_exclusion = true;
                for (auto foo : instance.Exclusion_list[i][time_j]) {
                    if (!ok_exclusion) break;
                    int k = foo.first, t = foo.second, cur_t = obj.Time_Start_Intervention[k];
                    if (k == j) cur_t = time_i;
                    if (cur_t == t) ok_exclusion = false;
                }
                for (auto foo : instance.Exclusion_list[j][time_i]) {
                    if (!ok_exclusion) break;
                    int k = foo.first, t = foo.second, cur_t = obj.Time_Start_Intervention[k];
                    if (k == i) cur_t = time_j;
                    if (cur_t == t) ok_exclusion = false;
                }
                if (!ok_exclusion) continue;

                /// check resource constraint 
                obj.Erase_Only_Resources(instance, obj, i, time_i);
                obj.Erase_Only_Resources(instance, obj, j, time_j);
                obj.Insert_Only_Resources(instance, obj, i, time_j);
                obj.Insert_Only_Resources(instance, obj, j, time_i);
                
                bool ok_resource = true;
                if (obj.LBResources_cost > 1e-5 || obj.UBResources_cost > 1e-5) ok_resource = false;
                
                obj.Erase_Only_Resources(instance, obj, i, time_j);
                obj.Erase_Only_Resources(instance, obj, j, time_i);
                obj.Insert_Only_Resources(instance, obj, i, time_i);
                obj.Insert_Only_Resources(instance, obj, j, time_j);
                
                if (!ok_resource) continue;

                /// compare score
                int nAC = 0, nVL = 0;
                double costLB = 0, costUB = 0;
                obj.Erase_no_care_UB(instance, i, time_i, nAC, nVL);
                obj.Erase_no_care_UB(instance, j, time_j, nAC, nVL);
                obj.Insert_no_care_UB(instance, i, time_j, nAC, nVL, costLB, costUB);
                obj.Insert_no_care_UB(instance, j, time_i, nAC, nVL, costLB, costUB);

                assert( obj.numFailedIntervention == 0 );
                assert( obj.LBResources_cost <= 1e-5 );
                assert( obj.UBResources_cost <= 1e-5 );
                
                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    //cerr << "~";
                    bestScore = newScore;
                    returnVal = true;
                    continue;
                }

                obj.Erase_no_care_UB(instance, i, time_j, nAC, nVL);
                obj.Erase_no_care_UB(instance, j, time_i, nAC, nVL);
                obj.Insert_no_care_UB(instance, i, time_i, nAC, nVL, costLB, costUB);
                obj.Insert_no_care_UB(instance, j, time_j, nAC, nVL, costLB, costUB);
            } 

            /// EXCHANGE
            if ( curID - sz_k + 1 >= 0 ) { 
                vector<int> type, ans;
                type.resize(sz_k, 0); ans.resize(sz_k, 0);

                double copyScore = bestScore;
                double score_exchange = k_exchange_brute(instance, obj, V, type, ans, curID-sz_k+1, sz_k, chlen, diff, copyScore );
                if ( abs(score_exchange-copyScore) > 1e-7 ) {
                        cerr << "SCORE EXCHANGE != COPY SCORE\n";
                    cerr << setprecision(10) << fixed << " " << score_exchange << '\n';
                    cerr << setprecision(10) << fixed << " " << copyScore << '\n';
                    exit(0);
                }
                assert( abs(score_exchange-copyScore) <= 1e-7 );

                if (score_exchange + diff < bestScore) {
                        cerr << "*";

                    bestScore = score_exchange;
                    for (int id = curID-sz_k+1; id <= curID; ++id) {
                        int inter = V[id].second;
                        int oldTime = obj.Time_Start_Intervention[inter];
                        int newTime = oldTime + ans[id-curID+sz_k-1];

                        int nAC = 0, nVL = 0;
                        double costLB = 0, costUB = 0;
                        obj.Erase_no_care_UB(instance, inter, oldTime, nAC, nVL);
                        obj.Insert_no_care_UB(instance, inter, newTime, nAC, nVL, costLB, costUB);
                    }
                    returnVal = true;

                    assert( abs(bestScore-obj.get_OBJ(instance)) <= 1e-6 );
                }
            }

            /// PERMUTATION sz_p
            assert( abs(bestScore-obj.get_OBJ(instance)) <= 1e-6 );

            if ( curID - sz_p + 1 >= 0 ) {
                bool flag_permutation = false;
                vector<int> per, oldTime, best_per;
                for (int j = curID - sz_p + 1; j <= curID; ++j) {
                    per.push_back(j - curID + sz_p - 1);
                    best_per.push_back(j - curID + sz_p - 1);
                    oldTime.push_back( obj.Time_Start_Intervention[V[j].second] );
                }

                do {
                    
                    bool cek_start_time = true;
                    int curPoint = curID - sz_p + 1;
                    for (int j = 0; j < (int) per.size(); ++j) {
                        int inter = V[curPoint].second, t = oldTime[per[j]];
                        if ( instance.tmax[inter] < t ) { cek_start_time = false; break; }
                        if ( t + instance.delta[inter][t] > instance.T+1 ) { cek_start_time = false; break; }   
                        ++curPoint;
                    }
                    if (!cek_start_time) continue;

                    curPoint = curID - sz_p + 1;                    
                    double oldScore = obj.get_OBJ(instance);
                    for (int j = 0; j < (int) per.size(); ++j) {
                        int inter = V[curPoint].second;

                        int nAC = 0, nVL = 0;
                        double costLB = 0, costUB = 0; 
                        obj.Erase_no_care_UB(instance, inter, obj.Time_Start_Intervention[inter], nAC, nVL);
                        obj.Insert_no_care_UB(instance, inter, oldTime[per[j]], nAC, nVL, costLB, costUB);
                        
                        ++curPoint;
                    }

                    /// check constraint and update val
                    bool ok = (obj.numFailedIntervention == 0);
                    if ( obj.LBResources_cost > 1e-6 || obj.UBResources_cost > 1e-6 ) ok = false;
                    if ( ok ) {
                        for (int j = curID-sz_p+1; j <= curID; ++j) {
                            int inter = V[j].second;
                            if ( !obj.exclusionChecking(instance, inter, obj.Time_Start_Intervention[inter]) ) {
                                ok = false; break;
                            } 
                        }
                    }
                    if (ok) {
                        double newScore = obj.get_OBJ(instance);
                        if ( newScore + diff < bestScore ) {
                            bestScore = newScore;
                            best_per = per;
                            flag_permutation = true;

                                // cerr << "clgt: ";
                                // for (int x : best_per) cerr << x << " ";
                                // cerr << "--> " << setprecision(5) << fixed << " " <<
                                //     obj.LBResources_cost << " " << obj.UBResources_cost << " " << obj.get_OBJ(instance) << '\n'; 
                        }
                    }

                    /// cumback
                    curPoint = curID - sz_p + 1;
                    for (int j = 0; j < (int) per.size(); ++j) {
                        int inter = V[curPoint].second;

                        int nAC = 0, nVL = 0;
                        double costLB = 0, costUB = 0;
                        obj.Erase_no_care_UB(instance, inter, obj.Time_Start_Intervention[inter], nAC, nVL);
                        obj.Insert_no_care_UB(instance, inter, oldTime[j], nAC, nVL, costLB, costUB);

                        ++curPoint;
                    }
                    assert( abs(obj.get_OBJ(instance) - oldScore) <= 1e-6 );
                    assert( obj.numFailedIntervention == 0 );
                    assert( obj.LBResources_cost <= 1e-6 );
                    assert( obj.UBResources_cost <= 1e-6 );

                } while (next_permutation(per.begin(), per.end()));

                if ( flag_permutation ) {
                    cerr << "@";

                    int curPoint = curID - sz_p + 1;
                    for (int j = 0; j < (int) best_per.size(); ++j) {
                        int inter = V[curPoint].second;

                        int nAC = 0, nVL = 0;
                        double costLB = 0, costUB = 0; 
                        obj.Erase_no_care_UB(instance, inter, obj.Time_Start_Intervention[inter], nAC, nVL);
                        obj.Insert_no_care_UB(instance, inter, oldTime[best_per[j]], nAC, nVL, costLB, costUB);
                        
                        ++curPoint;
                    }
                    returnVal = true;

                }

                if ( abs(obj.get_OBJ(instance)-bestScore) > 1e-6 ) {
                    cerr << "### " << setprecision(10) << fixed << obj.get_OBJ(instance) << " " << bestScore << '\n';
                    exit(0); 
                }
                assert( abs(obj.get_OBJ(instance)-bestScore) <= 1e-6 );
            }

                // just for sure 
                // for (int i = 1; i <= instance.numInterventions; ++i) {
                //     if ( !obj.exclusionChecking(instance, i, obj.Time_Start_Intervention[i]) ) {
                //         assert(10 == 11);
                //     }
                // }

                // if ( obj.UBResources_cost > 1e-6 ) {
                //     cerr << setprecision(10) << fixed << " " << obj.UBResources_cost << '\n';
                //     exit(0);
                // }
                // assert( obj.numFailedIntervention == 0 );
                // assert( obj.LBResources_cost <= 1e-6 );
                // assert( obj.UBResources_cost <= 1e-6 );
                // assert( abs(bestScore - obj.get_OBJ(instance)) <= 1e-6 );
                // cerr << " --> " << bestScore << " " << obj.get_OBJ(instance) << '\n';
        }
        return returnVal;
    }

    bool dp_with_k_exchange_ver3(Problem_Instance &instance, NLS_object &obj, double alpha, double beta, int sz_k, int chlen, double diff = 0.0001) {
        vector<pair<int, int> > V;
        for (int i = 1; i <= instance.numInterventions; ++i) { 
            int start_time = obj.Time_Start_Intervention[i];
            int end_time = start_time + instance.delta[i][start_time];
            V.push_back( make_pair(end_time, i) );   
        }
        sort(V.begin(), V.end());
        
        int curID = -1;
        bool returnVal = false;
        for (auto foo : V) {
            ++curID;
            int i = foo.second;
            if ( !obj.Time_Start_Intervention[i] ) continue;

            int oldTimeStart = obj.Time_Start_Intervention[i], bestTimeStart = oldTimeStart;
            double bestScore = obj.get_OBJ(instance);
            int ACLB = 0, VLUB = 0;
            obj.Erase_no_care_UB(instance, i, oldTimeStart, ACLB, VLUB);

            vector<int> time_order;
            for (int newTimeStart = 1; newTimeStart <= instance.tmax[i]; ++newTimeStart) {
                if ( newTimeStart == oldTimeStart ) continue;
                if ( !obj.exclusionChecking(instance, i, newTimeStart) ) continue;
                time_order.push_back(newTimeStart);
            }

            int percent = rand() % 100;
            if (percent < 50) reverse(time_order.begin(), time_order.end()); 

            for (int newTimeStart : time_order) {
                double LB_cost = 0, UB_cost = 0; 
                obj.Insert_no_care_UB(instance, i, newTimeStart, ACLB, VLUB, LB_cost, UB_cost);

                bool ok = true; 
                if ( obj.LBResources_cost > 1e-4 || obj.UBResources_cost > 1e-4 ) ok = false;
                if (!ok) {
                    obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
                    continue; 
                }

                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    //cerr << "^";
                    bestScore = newScore;
                    bestTimeStart = newTimeStart;
                }
                obj.Erase_no_care_UB(instance, i, newTimeStart, ACLB, VLUB);
            }

            double LB_cost = 0, UB_cost = 0;
            obj.Insert_no_care_UB(instance, i, bestTimeStart, ACLB, VLUB, LB_cost, UB_cost);
            if (bestTimeStart != oldTimeStart) returnVal = true;

            /// SWAP
            for (auto preFoo : V) {
                if (preFoo == foo) break;
                int j = preFoo.second;
                
                /// swap i and j
                int time_i = obj.Time_Start_Intervention[i];
                int time_j = obj.Time_Start_Intervention[j];
                if (time_j > instance.tmax[i] || time_i > instance.tmax[j]) continue;

                /// check exclusion constraint 
                bool ok_exclusion = true;
                for (auto foo : instance.Exclusion_list[i][time_j]) {
                    if (!ok_exclusion) break;
                    int k = foo.first, t = foo.second, cur_t = obj.Time_Start_Intervention[k];
                    if (k == j) cur_t = time_i;
                    if (cur_t == t) ok_exclusion = false;
                }
                for (auto foo : instance.Exclusion_list[j][time_i]) {
                    if (!ok_exclusion) break;
                    int k = foo.first, t = foo.second, cur_t = obj.Time_Start_Intervention[k];
                    if (k == i) cur_t = time_j;
                    if (cur_t == t) ok_exclusion = false;
                }
                if (!ok_exclusion) continue;

                /// check resource constraint 
                obj.Erase_Only_Resources(instance, obj, i, time_i);
                obj.Erase_Only_Resources(instance, obj, j, time_j);
                obj.Insert_Only_Resources(instance, obj, i, time_j);
                obj.Insert_Only_Resources(instance, obj, j, time_i);
                
                bool ok_resource = true;
                if (obj.LBResources_cost > 1e-5 || obj.UBResources_cost > 1e-5) ok_resource = false;
                
                obj.Erase_Only_Resources(instance, obj, i, time_j);
                obj.Erase_Only_Resources(instance, obj, j, time_i);
                obj.Insert_Only_Resources(instance, obj, i, time_i);
                obj.Insert_Only_Resources(instance, obj, j, time_j);
                
                if (!ok_resource) continue;

                /// compare score
                int nAC = 0, nVL = 0;
                double costLB = 0, costUB = 0;
                obj.Erase_no_care_UB(instance, i, time_i, nAC, nVL);
                obj.Erase_no_care_UB(instance, j, time_j, nAC, nVL);
                obj.Insert_no_care_UB(instance, i, time_j, nAC, nVL, costLB, costUB);
                obj.Insert_no_care_UB(instance, j, time_i, nAC, nVL, costLB, costUB);
                
                double newScore = obj.get_OBJ(instance);
                if ( newScore + diff < bestScore ) {
                    //cerr << "~";
                    bestScore = newScore;
                    returnVal = true;
                    continue;
                }

                obj.Erase_no_care_UB(instance, i, time_j, nAC, nVL);
                obj.Erase_no_care_UB(instance, j, time_i, nAC, nVL);
                obj.Insert_no_care_UB(instance, i, time_i, nAC, nVL, costLB, costUB);
                obj.Insert_no_care_UB(instance, j, time_j, nAC, nVL, costLB, costUB);
            } 

            /// EXCHANGE
            if ( curID - sz_k + 1 >= 0 ) {

	            vector<int> type, ans;
	            type.resize(sz_k, 0); ans.resize(sz_k, 0);

	            double copyScore = bestScore;
	            double score_exchange = k_exchange_brute(instance, obj, V, type, ans, curID-sz_k+1, sz_k, chlen, diff, copyScore );
                if ( abs(score_exchange-copyScore) > 1e-7 ) {
                        cerr << "SCORE EXCHANGE != COPY SCORE\n";
                    cerr << setprecision(10) << fixed << " " << score_exchange << '\n';
                    cerr << setprecision(10) << fixed << " " << copyScore << '\n';
                    exit(0);
                }
                assert( abs(score_exchange-copyScore) <= 1e-7 );

	            if (score_exchange + diff < bestScore) {
	                    cerr << "*";
	                    // cerr << bestScore << " " << score_exchange;
	                    //cerr << bestScore << " ";

	                bestScore = score_exchange;
	                for (int id = curID-sz_k+1; id <= curID; ++id) {
	                    int inter = V[id].second;
	                    int oldTime = obj.Time_Start_Intervention[inter];
	                    int newTime = oldTime + ans[id-curID+sz_k-1];

	                    int nAC = 0, nVL = 0;
	                    double costLB = 0, costUB = 0;
	                    obj.Erase_no_care_UB(instance, inter, oldTime, nAC, nVL);
	                    obj.Insert_no_care_UB(instance, inter, newTime, nAC, nVL, costLB, costUB);
	                }
	                returnVal = true;
	                    
	                    // for (int i = 1; i <= instance.numInterventions; ++i) {
	                    //     if ( !obj.exclusionChecking(instance, i, obj.Time_Start_Intervention[i]) ) {
	                    //         assert(10 == 11);
	                    //     }
	                    // }
	                    // assert( obj.numFailedIntervention == 0 );
	                    // assert( obj.LBResources_cost < 1e-5 );
	                    // assert( obj.UBResources_cost < 1e-5 );
	                    // assert( abs(obj.get_OBJ(instance)-score_exchange) <= 1e-5 );
	                    // cerr << " --> " << bestScore << " " << obj.get_OBJ(instance) << '\n';
	            }
	        }


            /// PERMUTATION sz_p = 5
            assert( abs(bestScore-obj.get_OBJ(instance)) <= 1e-6 );

            int sz_p = 5;
            if ( curID - sz_p + 1 >= 0 ) {
                bool flag_permutation = false;
                vector<int> per, oldTime, best_per;
                for (int j = curID - sz_p + 1; j <= curID; ++j) {
                    per.push_back(j - curID + sz_p - 1);
                    best_per.push_back(j - curID + sz_p - 1);
                    oldTime.push_back( obj.Time_Start_Intervention[V[j].second] );
                }

                do {
                    
                    bool cek_start_time = true;
                    int curPoint = curID - sz_p + 1;
                    for (int j = 0; j < (int) per.size(); ++j) {
                        int inter = V[curPoint].second, t = oldTime[per[j]];
                            // cerr << ":)) " << inter << " " << t << '\n';
                        if ( instance.tmax[inter] < t ) { cek_start_time = false; break; }
                        if ( t + instance.delta[inter][t] > instance.T+1 ) { cek_start_time = false; break; }   
                        ++curPoint;
                    }
                    if (!cek_start_time) continue;

                    curPoint = curID - sz_p + 1;                    
                    double oldScore = obj.get_OBJ(instance);
                    for (int j = 0; j < (int) per.size(); ++j) {
                        int inter = V[curPoint].second;

                        int nAC = 0, nVL = 0;
                        double costLB = 0, costUB = 0; 
                        obj.Erase_no_care_UB(instance, inter, obj.Time_Start_Intervention[inter], nAC, nVL);
                        obj.Insert_no_care_UB(instance, inter, oldTime[per[j]], nAC, nVL, costLB, costUB);
                        
                        ++curPoint;
                    }

                    /// check constraint and update val
                    bool ok = (obj.numFailedIntervention == 0);
                    if ( obj.LBResources_cost > 1e-6 || obj.UBResources_cost > 1e-6 ) ok = false;
                    if ( ok ) {
                        for (int j = curID-sz_p+1; j <= curID; ++j) {
                            int inter = V[j].second;
                            if ( !obj.exclusionChecking(instance, inter, obj.Time_Start_Intervention[inter]) ) {
                                ok = false; break;
                            } 
                        }
                    }
                    if (ok) {
                        double newScore = obj.get_OBJ(instance);
                        if ( newScore + diff < bestScore ) {
                            bestScore = newScore;
                            best_per = per;
                            flag_permutation = true;

                                // cerr << "clgt: ";
                                // for (int x : best_per) cerr << x << " ";
                                // cerr << "--> " << setprecision(5) << fixed << " " <<
                                //     obj.LBResources_cost << " " << obj.UBResources_cost << " " << obj.get_OBJ(instance) << '\n'; 
                        }
                    }

                    /// cumback
                    curPoint = curID - sz_p + 1;
                    for (int j = 0; j < (int) per.size(); ++j) {
                        int inter = V[curPoint].second;

                        int nAC = 0, nVL = 0;
                        double costLB = 0, costUB = 0;
                        obj.Erase_no_care_UB(instance, inter, obj.Time_Start_Intervention[inter], nAC, nVL);
                        obj.Insert_no_care_UB(instance, inter, oldTime[j], nAC, nVL, costLB, costUB);

                        ++curPoint;
                    }
                    assert( abs(obj.get_OBJ(instance) - oldScore) <= 1e-6 );

                } while (next_permutation(per.begin(), per.end()));

                if ( flag_permutation ) {
                    cerr << "@";

                    int curPoint = curID - sz_p + 1;
                    for (int j = 0; j < (int) best_per.size(); ++j) {
                        int inter = V[curPoint].second;

                        int nAC = 0, nVL = 0;
                        double costLB = 0, costUB = 0; 
                        obj.Erase_no_care_UB(instance, inter, obj.Time_Start_Intervention[inter], nAC, nVL);
                        obj.Insert_no_care_UB(instance, inter, oldTime[best_per[j]], nAC, nVL, costLB, costUB);
                        
                        ++curPoint;
                    }
                    returnVal = true;

                }

                if ( abs(obj.get_OBJ(instance)-bestScore) > 1e-6 ) {
                    cerr << "### " << setprecision(10) << fixed << obj.get_OBJ(instance) << " " << bestScore << '\n';
                    exit(0); 
                }
                assert( abs(obj.get_OBJ(instance)-bestScore) <= 1e-6 );
            }
        }

        assert( obj.numFailedIntervention == 0 );
        assert( obj.UBResources_cost < 1e-6 );
        assert( obj.LBResources_cost < 1e-6 );
        return returnVal;
    }
}

#endif // NLS_LOCAL_SEARCH