#ifndef UTILS
#define UTILS

#include<bits/stdc++.h>
#include <chrono>
#include <random>

namespace Utils {

    // dynamic seed
    std::mt19937 mt = std::mt19937(std::chrono::high_resolution_clock::now().time_since_epoch().count());

    // fixed seed
    //std::mt19937 Utils::mt = std::mt19937(0);

    int integer_random_generator(const int &a, const int &b){
        assert(b>a);
        return std::uniform_int_distribution<int>{a, b-1}(mt);
    }

    double real_random_generator(const double &a, const double &b){
        return std::uniform_real_distribution<double>{a, b}(mt);
    }

    void shuffle(std::vector<int> &vec){
        std::shuffle(vec.begin(), vec.end(), mt);
    }

    template <typename T>
    void remove(std::vector<T> &c, T &element){
        for (auto it = c.begin(); it != c.end(); /* "it" updated inside loop body */ ){
            if (*it == element){
                it = c.erase(it);
                break;
            }
            else {
                ++it;
            }
        }
    }

    int position(std::vector<int>& vec, int element){
        auto it = std::find(vec.begin(), vec.end(), element);
        if (it == vec.end())
        {
        // element not in vector
            assert(false);
            return -1;
        } 
        else return std::distance(vec.begin(), it);
    }

}



#endif // UTILS