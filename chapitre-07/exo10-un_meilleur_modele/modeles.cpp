#include <cstdio>
#include <cmath>
#include <vector>
#include "NKWindow/NKMain.h"

using namespace nkentseu;

static void ConfigureAppData(NkAppData &d) { d.appName = "modeles"; }
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

static const double kPi = 3.14159265358979323846;

// Gestes cosinusoïdaux (accélère puis freine), sauf l'arrêt.
double LacetCos(double t, double amplitude, double duree) {
    if (t <= 0.0) return 0.0;
    if (t >= duree) return amplitude;
    return amplitude * 0.5 * (1.0 - cos(kPi * t / duree));
}

// Arrêt brusque : rotation à vitesse constante puis arrêt net.
double LacetArret(double t, double vitesseDegS, double tStop, double angleStop) {
    if (t <= tStop) return vitesseDegS * t;
    return angleStop;
}

struct Result { double meanV, maxV, meanA, maxA; int count; };

Result Mesurer(double (*lacetFn)(double, double, double), double p1, double p2,
               double duree, double apresMs) {
    const double period = 1.0 / 72.0;
    const double horizon = period;
    double end = duree + apresMs / 1000.0;
    double sumV = 0, sumA = 0, maxV = 0, maxA = 0;
    int count = 0;
    std::vector<double> ang;
    double t = 0.0;
    while (t <= end + 1e-9) {
        ang.push_back(lacetFn(t, p1, p2));
        t += period;
    }
    double dt = period;
    for (size_t i = 2; i < ang.size(); ++i) {
        double tNow = (double)i * period;
        if (tNow > end + 1e-9) break;
        double a_i = ang[i];
        double a_im1 = ang[i - 1];
        double a_im2 = ang[i - 2];
        // vraie pose a l'instant d'affichage (une periode plus tard)
        double aTrue = (i + 1 < ang.size()) ? ang[i + 1] : lacetFn(end, p1, p2);
        // modele vitesse constante : 2 dernieres poses
        double v = (a_i - a_im1) / dt;
        double predV = a_i + v * horizon;
        double eV = std::fabs(predV - aTrue);
        // modele acceleration : 3 dernieres poses
        double acc = (a_i - 2.0 * a_im1 + a_im2) / (dt * dt);
        double predA = a_i + v * horizon + 0.5 * acc * horizon * horizon;
        double eA = std::fabs(predA - aTrue);
        sumV += eV; sumA += eA;
        if (eV > maxV) maxV = eV;
        if (eA > maxA) maxA = eA;
        ++count;
    }
    Result r;
    r.meanV = count > 0 ? sumV / count : 0;
    r.maxV = maxV;
    r.meanA = count > 0 ? sumA / count : 0;
    r.maxA = maxA;
    r.count = count;
    return r;
}

int nkmain(const NkEntryState &state) {
    (void)state;

    // Rapide : 90 deg en 400 ms.
    Result rapide = Mesurer(LacetCos, 90.0, 0.400, 0.400, 200.0);
    std::printf("rapide (90 deg en 400 ms) : vitesse %.2f deg (max %.2f), acceleration %.2f deg (max %.2f)\n",
                rapide.meanV, rapide.maxV, rapide.meanA, rapide.maxA);

    // Lent : 45 deg en 1200 ms.
    Result lent = Mesurer(LacetCos, 45.0, 1.200, 1.200, 200.0);
    std::printf("lent (45 deg en 1200 ms) : vitesse %.2f deg (max %.2f), acceleration %.2f deg (max %.2f)\n",
                lent.meanV, lent.maxV, lent.meanA, lent.maxA);

    // Demi-tour brusque : 180 deg en 300 ms.
    Result demi = Mesurer(LacetCos, 180.0, 0.300, 0.300, 200.0);
    std::printf("demi-tour (180 deg en 300 ms) : vitesse %.2f deg (max %.2f), acceleration %.2f deg (max %.2f)\n",
                demi.meanV, demi.maxV, demi.meanA, demi.maxA);

    // Arret brusque : 60 deg/s pendant 500 ms puis stop net (30 deg).
    Result arret = Mesurer([](double t, double, double) -> double {
        // parameters unused; use globals via closure not possible here -> replacer
        return 0.0;
    }, 0, 0, 0, 0);
    (void)arret;
    // Arret a part car signature differente.
    {
        const double period = 1.0 / 72.0;
        const double horizon = period;
        const double vitesseDegS = 60.0;
        const double tStop = 0.500;
        const double angleStop = vitesseDegS * tStop; // 30 deg
        double end = tStop + 0.200;
        std::vector<double> ang;
        double t = 0.0;
        while (t <= end + 1e-9) {
            if (t <= tStop) ang.push_back(vitesseDegS * t);
            else ang.push_back(angleStop);
            t += period;
        }
        double sumV = 0, sumA = 0, maxV = 0, maxA = 0;
        int count = 0;
        for (size_t i = 2; i < ang.size(); ++i) {
            double tNow = (double)i * period;
            if (tNow > end + 1e-9) break;
            double a_i = ang[i], a_im1 = ang[i - 1], a_im2 = ang[i - 2];
            double aTrue = (i + 1 < ang.size()) ? ang[i + 1] : angleStop;
            double v = (a_i - a_im1) / period;
            double predV = a_i + v * horizon;
            double eV = std::fabs(predV - aTrue);
            double acc = (a_i - 2.0 * a_im1 + a_im2) / (period * period);
            double predA = a_i + v * horizon + 0.5 * acc * horizon * horizon;
            double eA = std::fabs(predA - aTrue);
            sumV += eV; sumA += eA;
            if (eV > maxV) maxV = eV;
            if (eA > maxA) maxA = eA;
            ++count;
        }
        std::printf("arret (60 deg/s puis stop net a 500 ms) : vitesse %.2f deg (max %.2f), acceleration %.2f deg (max %.2f)\n",
                    count > 0 ? sumV / count : 0, maxV,
                    count > 0 ? sumA / count : 0, maxA);
    }

    return 0;
}
