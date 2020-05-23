#ifndef NLS_BITMASK_LOCALSEARCH
#define NLS_BITMASK_LOCALSEARCH

#include "../Problem_ver2.cpp"
#include "NLS_object_ver2.cpp"

namespace NLS_bitmask_localsearch { 
    bool getBIT(int n, int k) { return n & (1<<k); }

    bool process(Problem_Instance &instance, NLS_object &obj, int numBit, double diff = 0.005) {
        vector<vector<int> > V;
        vector<int> order;

        V.resize(instance.T+1);
        for (int i = 1; i <= instance.numInterventions; ++i) {
            int t = obj.Time_Start_Intervention[i];
            V[t].push_back(i);
        }
        for (int t = 1; t <= instance.T; ++t) {
            random_shuffle(V[t].begin(), V[t].end());
            for (int i : V[t]) order.push_back(i);
        }   

        vector<pair<int, int> > lsTime;
        for (int t = 1; t <= instance.T; ++t) {
            if (V[t].size()) {
                for (int i = 0; i < (int) V[t].size(); ++i) lsTime.push_back( make_pair(t, V[t][i]) );
            }
            else lsTime.push_back( make_pair(t, -1) );
        }

        vector<vector<NLS_object> > dp;
        dp.resize(2);
        for (int i = 0; i <= 1; ++i) {
            dp[i].resize( (1<<numBit) );
            for (int mask = 0; mask < (1<<numBit); ++mask) {
                if (i == 0 && mask == 0) dp[i][mask] = obj;
                else dp[i][mask].valid = false;
            }
        }

        int type = 0, curPoint = -1;
        for (int id = 0; id < (int) lsTime.size(); ++id) {
            int t = lsTime[id].first;
            if ( lsTime[id].second > 0 ) ++curPoint;

            for (int mask = (1<<numBit)-1; mask >= 0; --mask) {
                if ( !dp[type][mask].valid ) continue;

                    // cerr << "??? " << type << " " << t << " " << mask << '\n';

                /// (mask, type) -> (mask-(1<<bit), type)
                NLS_object tmpOBJ = dp[type][mask];
                for (int bit = 0; bit < (int) numBit; ++bit) {
                    if ( !getBIT(mask, bit) ) continue;
                    int nMask = mask - (1<<bit), inter = order[curPoint-bit];

                    // time limit constaint
                    if ( t > instance.tmax[inter] ) continue;
                    if ( t + instance.delta[inter][t] > instance.T+1 ) continue;

                    // exclusion constraint
                    if ( !tmpOBJ.exclusionChecking(instance, inter, t) ) continue; 

                    // resource constraint
                    int oldTime = tmpOBJ.Time_Start_Intervention[inter];
                    assert( oldTime <= 0 );
                    if ( !tmpOBJ.check_resource_constraint(instance, tmpOBJ, inter, t) ) continue;

                    // compare score
                    int nAC = 0, nVL = 0;
                    double costLB = 0, costUB = 0;
                    tmpOBJ.Insert_no_care_UB(instance, inter, t, nAC, nVL, costLB, costUB);
                    double newScore = tmpOBJ.get_OBJ(instance);
                    if ( !dp[type][nMask].valid || dp[type][nMask].get_OBJ(instance)-newScore > 1e-9 ) dp[type][nMask] = tmpOBJ;
                    tmpOBJ.Erase_no_care_UB(instance, inter, t, nAC, nVL);
                        
                        // for (int cc = 1; cc <= instance.numInterventions; ++cc) {
                        //     if ( tmpOBJ.Time_Start_Intervention[cc] != dp[type][mask].Time_Start_Intervention[cc] ) {
                        //         for (int i = 1; i <= instance.numInterventions; ++i) cerr << dp[type][mask].Time_Start_Intervention[i] << " ";
                        //         cerr << '\n';
                        //         for (int i = 1; i <= instance.numInterventions; ++i) cerr << tmpOBJ.Time_Start_Intervention[i] << " ";
                        //         cerr << '\n';

                        //         cerr << dp[type][mask].Time_Start_Intervention[lsTime[id+1].second] << '\n';
                        //         cerr << "xxx\n";
                        //         exit(0);
                        //     }
                        // }
                }

                if ( id >= (int) lsTime.size()-1 ) continue;
                
                /// (mask, type) -> (mask, 1-type) 
                if (lsTime[id+1].second <= 0) {
                    assert(lsTime[id+1].second == -1);
                    if ( !dp[1-type][mask].valid || dp[1-type][mask].get_OBJ(instance) - dp[type][mask].get_OBJ(instance) > 1e-9 ) 
                        dp[1-type][mask] = dp[type][mask];
                }

                /// (mask, type) -> (mask', 1-type)
                if (lsTime[id+1].second > 0) {
                    if ( getBIT(mask, numBit-1) ) continue;

                    int i = lsTime[id+1].second, oldTime = tmpOBJ.Time_Start_Intervention[i];
                        
                        if ( lsTime[id+1].first != oldTime ) {
                            cerr << "??? " << mask << " " << i << " " << oldTime << " " << lsTime[id+1].first << '\n';
                            cerr << "cc " << lsTime[id].first << " " << lsTime[id].second << '\n';
                            cerr << dp[type][mask].Time_Start_Intervention[i] << '\n';
                            exit(0);
                        }
                        assert(lsTime[id+1].first == oldTime);
                    
                    // /// intervention i : [1, oldTime]
                    int nAC = 0, nVL = 0;
                    double costLB = 0, costUB = 0;
                    tmpOBJ.Erase_no_care_UB(instance, i, oldTime, nAC, nVL);
                    for (int newTime = 1; newTime <= max(instance.tmax[i], oldTime); ++newTime) {
                        if ( newTime + instance.delta[i][newTime] > instance.T+1 ) continue;
                        if ( !tmpOBJ.exclusionChecking(instance, i, newTime) ) continue;
                        if ( !tmpOBJ.check_resource_constraint(instance, tmpOBJ, i, newTime) ) continue;

                        tmpOBJ.Insert_no_care_UB(instance, i, newTime, nAC, nVL, costLB, costUB);
                        double newScore = tmpOBJ.get_OBJ(instance);
                        if ( !dp[1-type][mask*2].valid || dp[1-type][mask*2].get_OBJ(instance) - newScore > 1e-9 ) dp[1-type][mask*2] = tmpOBJ;
                        tmpOBJ.Erase_no_care_UB(instance, i, newTime, nAC, nVL);
                    }
                    tmpOBJ.Insert_no_care_UB(instance, i, oldTime, nAC, nVL, costLB, costUB);

                    /// internvention i : [t, oldTime];
                    if ( !getBIT(mask, numBit-1) ) {
                        int nMask = mask * 2 + 1, oldTime = dp[type][mask].Time_Start_Intervention[i];
                        
                        int nAC = 0, nVL = 0;
                        double costLB = 0, costUB = 0;

                        dp[type][mask].Erase_no_care_UB(instance, i, oldTime, nAC, nVL);
                        double newScore = dp[type][mask].get_OBJ(instance);
                        if ( !dp[1-type][nMask].valid || dp[1-type][nMask].get_OBJ(instance) - newScore > 1e-9 ) dp[1-type][nMask] = dp[type][mask];
                        dp[type][mask].Insert_no_care_UB(instance, i, oldTime, nAC, nVL, costLB, costUB);
                    }
                }
            }

            if ( id == (int) lsTime.size()-1 ) break;
            for (int mask = 0; mask < (1<<numBit); ++mask) dp[type][mask].valid = false;
            type = 1 - type;
        }

            // cerr << "T_T: " << type << " " << dp[type][0].valid << '\n';
            // cerr << "??? " << setprecision(10) << fixed << dp[type][0].get_OBJ(instance) << '\n';
            // cerr << "plz " << setprecision(10) << fixed << obj.get_OBJ(instance) << '\n';

        if ( dp[type][0].valid && obj.get_OBJ(instance) - dp[type][0].get_OBJ(instance) >= diff ) {
            obj = dp[type][0];
            assert( obj.numFailedIntervention == 0 );
            assert( obj.LBResources_cost <= 1e-5 );
            assert( obj.UBResources_cost <= 1e-5 );
                // cerr << "3dm\n";
            return true;
        }
        return false;
    }
}

#endif //NLS_BITMASK_LOCALSEARCH