#include <cstdio>
#include <cmath>
#include "NKWindow/NKMain.h"
#include "NKMath/NkQuat.h"

using namespace nkentseu;
using namespace nkentseu::math;

static const double kPi = 3.14159265358979323846;

double Lacet(double t, double amplitude, double duree) {
    if (t <= 0.0) return 0.0;
    if (t >= duree) return amplitude;
    return amplitude * 0.5 * (1.0 - cos(kPi * t / duree));
}

NkVec3f VitesseAngulaire(const NkQuatf &from, const NkQuatf &to,
                         float32 dtSeconds) {
    if (dtSeconds <= 0.f) { return NkVec3f(0.f, 0.f, 0.f); }
    NkQuatf delta = (to * from.Conjugate()).Normalized();
    if (delta.w < 0.f) {
        delta = NkQuatf(-delta.x, -delta.y, -delta.z, -delta.w);
    }
    const float32 angleRad = delta.Angle().Rad();
    if (angleRad <= 1e-6f) { return NkVec3f(0.f, 0.f, 0.f); }
    return delta.Axis() * (angleRad / dtSeconds);
}

NkQuatf Extrapoler(const NkQuatf &orientation, const NkVec3f &vitesse,
                   float32 dtSeconds) {
    const float32 speedRad = vitesse.Len();
    if (speedRad > 1e-6f) {
        const NkVec3f axis = vitesse * (1.f / speedRad);
        const NkQuatf delta(NkAngle::FromRad(speedRad * dtSeconds), axis);
        return (delta * orientation).Normalized();
    }
    return orientation;
}

float32 AngleDeg(const NkQuatf &a, const NkQuatf &b) {
    return (b * a.Conjugate()).Normalized().Angle().Deg();
}

struct Result { double avec; double sans; int count; };

Result Mesurer(double amplitude, double duree) {
    const double period = 1.0 / 72.0;
    const float32 periodSeconds = (float32)period;
    double sumAvec = 0.0, sumSans = 0.0;
    int count = 0;
    NkQuatf previousPose;
    bool havePrevious = false;

    for (double t = 0.0; t < duree; t += period) {
        const double angleT = Lacet(t, amplitude, duree);
        const NkQuatf poseT = NkQuatf::RotateY(NkAngle((float32)angleT));

        NkVec3f vitesse(0.f, 0.f, 0.f);
        if (havePrevious) {
            vitesse = VitesseAngulaire(previousPose, poseT, periodSeconds);
        }
        const NkQuatf predit = Extrapoler(poseT, vitesse, periodSeconds);

        const double angleTp = Lacet(t + period, amplitude, duree);
        const NkQuatf vraiTp = NkQuatf::RotateY(NkAngle((float32)angleTp));

        const float eAvec = AngleDeg(predit, vraiTp);
        const float eSans = AngleDeg(poseT, vraiTp);

        sumAvec += eAvec;
        sumSans += eSans;
        ++count;

        previousPose = poseT;
        havePrevious = true;
    }

    Result r;
    r.avec = count > 0 ? sumAvec / count : 0.0;
    r.sans = count > 0 ? sumSans / count : 0.0;
    r.count = count;
    return r;
}

int nkmain(const NkEntryState &state) {
    (void)state;

    Result rapide = Mesurer(90.0, 0.400);
    std::printf("rapide (90 deg en 400 ms) : avec %.2f deg, sans %.2f deg, rapport %.2f\n",
                rapide.avec, rapide.sans,
                rapide.sans > 0.0 ? rapide.sans / rapide.avec : 0.0);

    Result lent = Mesurer(30.0, 0.800);
    std::printf("lent (30 deg en 800 ms) : avec %.2f deg, sans %.2f deg, rapport %.2f\n",
                lent.avec, lent.sans,
                lent.sans > 0.0 ? lent.sans / lent.avec : 0.0);

    Result demi = Mesurer(180.0, 0.200);
    std::printf("demi-tour (180 deg en 200 ms) : avec %.2f deg, sans %.2f deg, rapport %.2f\n",
                demi.avec, demi.sans,
                demi.sans > 0.0 ? demi.sans / demi.avec : 0.0);

    return 0;
}
