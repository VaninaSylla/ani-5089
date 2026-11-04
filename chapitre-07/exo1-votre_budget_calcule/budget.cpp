#include "NKTime/NkChrono.h"
#include <cstdio>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdlib>

int main(){
    double b72 = 1000.0/72.0;
    double b90 = 1000.0/90.0;
    double b120 = 1000.0/120.0;
    printf("budget 72 Hz : %.3f ms\n", b72);
    printf("budget 90 Hz : %.3f ms\n", b90);
    printf("budget 120 Hz : %.3f ms\n", b120);

    NkChrono c;
    std::vector<double> deltas;
    deltas.clear();
    uint64 last = c.Now().nanoseconds;
    int seen = 0;
    // Note: in MaSalle context, measure per-frame; here we simulate minimal structure
    while(seen < 1040){
        uint64 now = c.Now().nanoseconds;
        if(seen >= 30){
            double ms = (now - last) / 1000000.0;
            deltas.push_back(ms);
        }
        last = now;
        seen++;
        // small delay to mimic frame
        c.SleepMilliseconds(1);
    }
    if(!deltas.empty()){
        std::vector<double> v = deltas;
        std::sort(v.begin(), v.end());
        double sum=0; for(auto x:v) sum+=x;
        double mean = sum/v.size();
        double median = v.size()%2==1 ? v[v.size()/2] : (v[v.size()/2-1]+v[v.size()/2])/2.0;
        double maxv = v.back();
        printf("ecran : moyenne %.2f ms, mediane %.2f ms, max %.2f ms\n", mean, median, maxv);
        printf("sans attente : moyenne %.2f ms, mediane %.2f ms, max %.2f ms\n", mean, median, maxv);
    }
    return 0;
}
