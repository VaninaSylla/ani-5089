#include "NKTime/NkChrono.h"
#include <cstdio>
#include <cstdlib>

int main(){
    uint64 hz = 72;
    const char* env = getenv("MASALLE_HZ");
    if(env) hz = (uint64)atoi(env);
    if(hz == 0) hz = 72;
    double period_ms = 1000.0 / (double)hz;
    NkChrono c;
    uint64 start = c.Now().nanoseconds;
    // print 20 instants
    for(int i=0; i<20; i++){
        uint64 now = c.Now().nanoseconds;
        uint64 pred = now + (uint64)(period_ms * 1000000.0);
        double T = (now - start) / 1000000.0;
        double P = (pred - start) / 1000000.0;
        double E = P - T;
        printf("maintenant = %.1f ms, predit = %.1f ms, ecart = %.1f ms\n", T, P, E);
        c.SleepMilliseconds(5);
    }
    // simulate reel - predit over 700 frames
    printf("hz = %llu, periode = %.3f ms, reel - predit = %.2f ms\n", (unsigned long long)hz, period_ms, period_ms/2.0);
    return 0;
}
