#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <cstring>
#include "NKWindow/NKMain.h"
#include "NKMath/NkQuat.h"

using namespace nkentseu;
using namespace nkentseu::math;

static const double kPi = 3.14159265358979323846;

double Lacet(double t) {
    const double amplitude = 90.0, duree = 0.400;
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

float32 LacetDeg(const NkQuatf &q) {
    return (float32)(2.0 * atan2((double)q.y, (double)q.w) * 180.0 / kPi);
}

struct TimedPose {
    double time;
    NkQuatf pose;
    NkVec3f velocity;
};

static const uint32 kMaxSamples = 64u;

class Historique {
public:
    void Ajouter(double time, const NkQuatf &pose, const NkVec3f &velocity) {
        samples[head] = TimedPose{time, pose, velocity};
        head = (head + 1u) % kMaxSamples;
        if (count < kMaxSamples) ++count;
    }
    const TimedPose &SampleAt(double now, double latencySeconds) const {
        if (latencySeconds <= 0.0 || count <= 1u) {
            return samples[(head + kMaxSamples - 1u) % kMaxSamples];
        }
        const double target = now - latencySeconds;
        // Chercher l'echantillon le plus recent dont le temps <= target.
        const TimedPose *best = nullptr;
        uint32 index = head;
        for (uint32 i = 0u; i < count; ++i) {
            index = (index + kMaxSamples - 1u) % kMaxSamples; // du plus recent au plus ancien
            if (samples[index].time <= target) {
                best = &samples[index];
                break; // premier (plus recent) qui convient
            }
        }
        if (best != nullptr) return *best;
        // Aucun echantillon assez vieux : renvoyer le plus ancien disponible.
        return samples[(head + kMaxSamples - (count - 1u)) % kMaxSamples];
    }
    uint32 Count() const { return count; }
private:
    TimedPose samples[kMaxSamples];
    uint32 head = 0u;
    uint32 count = 0u;
};

NkQuatf PredictAt(const TimedPose &sample, double when, double periode,
                  bool borne) {
    double dt = (when - sample.time);
    if (borne) {
        dt = fmax(-0.1, fmin(0.1, dt));
    }
    return Extrapoler(sample.pose, sample.velocity, (float32)dt);
}

// ---------------- PNG minimal (RGB, blocs deflate stockes) ----------------
struct Png {
    std::vector<unsigned char> data;
    int width, height;
    Png(int w, int h) : width(w), height(h) {
        data.assign((size_t)w * (size_t)h * 3, 0);
    }
    void Pixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        size_t off = (size_t)y * (size_t)width * 3 + (size_t)x * 3;
        data[off + 0] = r;
        data[off + 1] = g;
        data[off + 2] = b;
    }
    void Line(int x0, int y0, int x1, int y1, unsigned char r, unsigned char g,
              unsigned char b) {
        int dx = abs(x1 - x0), dy = abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;
        for (;;) {
            Pixel(x0, y0, r, g, b);
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx) { err += dx; y0 += sy; }
        }
    }
    std::vector<unsigned char> Encode() {
        std::vector<unsigned char> raw;
        raw.push_back(0);
        for (int y = 0; y < height; ++y) {
            raw.push_back(0);
            for (int x = 0; x < width; ++x) {
                size_t off = (size_t)y * (size_t)width * 3 + (size_t)x * 3;
                raw.push_back(data[off + 0]);
                raw.push_back(data[off + 1]);
                raw.push_back(data[off + 2]);
            }
        }
        std::vector<unsigned char> zlib;
        zlib.push_back(0x78);
        zlib.push_back(0x01);
        size_t pos = 0;
        while (pos < raw.size()) {
            bool last = (pos + 65535 >= raw.size());
            unsigned short len = (unsigned short)(last ? raw.size() - pos : 65535);
            unsigned short nlen = (unsigned short)~len;
            unsigned char bfinal = last ? 1 : 0;
            unsigned char btype = 0;
            zlib.push_back((unsigned char)(bfinal | (btype << 1)));
            zlib.push_back((unsigned char)(len & 0xff));
            zlib.push_back((unsigned char)((len >> 8) & 0xff));
            zlib.push_back((unsigned char)(nlen & 0xff));
            zlib.push_back((unsigned char)((nlen >> 8) & 0xff));
            for (unsigned short i = 0; i < len; ++i) zlib.push_back(raw[pos++]);
        }
        unsigned long adler = 1;
        for (unsigned char c : raw) {
            adler = (adler + c) % 65521;
        }
        zlib.push_back(0);
        zlib.push_back(0);
        zlib.push_back(0);
        zlib.push_back(1);

        std::vector<unsigned char> png;
        const unsigned char sig[8] = {0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A};
        png.insert(png.end(), sig, sig + 8);

        auto addChunk = [&](const char *type, const std::vector<unsigned char> &content) {
            unsigned char lenb[4];
            size_t n = content.size();
            lenb[0] = (unsigned char)((n >> 24) & 0xff);
            lenb[1] = (unsigned char)((n >> 16) & 0xff);
            lenb[2] = (unsigned char)((n >> 8) & 0xff);
            lenb[3] = (unsigned char)(n & 0xff);
            for (int i = 0; i < 4; ++i) png.push_back(lenb[i]);
            for (int i = 0; i < 4; ++i) png.push_back((unsigned char)type[i]);
            for (unsigned char c : content) png.push_back(c);
            std::vector<unsigned char> crcBuf;
            crcBuf.insert(crcBuf.end(), (const unsigned char*)type, (const unsigned char*)type + 4);
            crcBuf.insert(crcBuf.end(), content.begin(), content.end());
            unsigned int crc = 0xffffffff;
            for (unsigned char c : crcBuf) {
                crc ^= c;
                for (int b = 0; b < 8; ++b) {
                    crc = (crc & 1) ? (0xEDB88320 ^ (crc >> 1)) : (crc >> 1);
                }
            }
            crc ^= 0xffffffff;
            unsigned char crcb[4];
            crcb[0] = (unsigned char)((crc >> 24) & 0xff);
            crcb[1] = (unsigned char)((crc >> 16) & 0xff);
            crcb[2] = (unsigned char)((crc >> 8) & 0xff);
            crcb[3] = (unsigned char)(crc & 0xff);
            for (int i = 0; i < 4; ++i) png.push_back(crcb[i]);
        };

        {
            std::vector<unsigned char> ihdr;
            unsigned char w[4] = {(unsigned char)((width >> 24) & 0xff),
                                  (unsigned char)((width >> 16) & 0xff),
                                  (unsigned char)((width >> 8) & 0xff),
                                  (unsigned char)(width & 0xff)};
            unsigned char h[4] = {(unsigned char)((height >> 24) & 0xff),
                                  (unsigned char)((height >> 16) & 0xff),
                                  (unsigned char)((height >> 8) & 0xff),
                                  (unsigned char)(height & 0xff)};
            ihdr.insert(ihdr.end(), w, w + 4);
            ihdr.insert(ihdr.end(), h, h + 4);
            ihdr.push_back(8);
            ihdr.push_back(2);
            ihdr.push_back(0);
            ihdr.push_back(0);
            ihdr.push_back(0);
            addChunk("IHDR", ihdr);
        }
        addChunk("IDAT", zlib);
        addChunk("IEND", std::vector<unsigned char>());
        return png;
    }
    bool Save(const char *path) {
        auto bytes = Encode();
        FILE *f = fopen(path, "wb");
        if (!f) return false;
        fwrite(bytes.data(), 1, bytes.size(), f);
        fclose(f);
        return true;
    }
};

int nkmain(const NkEntryState &state) {
    (void)state;
    const float64 period = 1.0 / 72.0;

    Historique hist;
    double latences[] = {0.0, 0.010, 0.020, 0.040, 0.080};
    int nLat = 5;
    double avecMean[5] = {0}, avecMax[5] = {0}, sansMean[5] = {0}, sansMax[5] = {0};
    int avecCnt[5] = {0};

    NkQuatf previousPose;
    bool havePrevious = false;

    double bestT = 0, bestSampleTime = 0, bestDiff = 1e9;
    TimedPose bestSample;
    bool haveBest = false;
    const double borneLatency = 1.0;

    double t = 0.0;
    while (t <= 3.0001) {
        double angle = Lacet(t - 1.0);
        NkQuatf pose = NkQuatf::RotateY(NkAngle((float32)angle));
        NkVec3f velocity(0.f, 0.f, 0.f);
        if (havePrevious) {
            velocity = VitesseAngulaire(previousPose, pose, (float32)period);
        }
        hist.Ajouter(t, pose, velocity);
        previousPose = pose;
        havePrevious = true;

        if (t >= 1.0 && t <= 1.9001) {
            double when = t + period;
            NkQuatf vraiPose = NkQuatf::RotateY(NkAngle((float32)Lacet(when - 1.0)));
            for (int i = 0; i < nLat; ++i) {
                const TimedPose &sample = hist.SampleAt(t, latences[i]);
                NkQuatf avec = PredictAt(sample, when, period, true);
                NkQuatf sans = sample.pose;
                double eA = AngleDeg(avec, vraiPose);
                double eS = AngleDeg(sans, vraiPose);
                avecMean[i] += eA; sansMean[i] += eS;
                if (eA > avecMax[i]) avecMax[i] = eA;
                if (eS > sansMax[i]) sansMax[i] = eS;
                avecCnt[i]++;
            }
        }

        const TimedPose &bs = hist.SampleAt(t, borneLatency);
        double diff = fabs(bs.time - 1.2);
        if (diff < bestDiff) {
            bestDiff = diff; bestT = t; bestSampleTime = bs.time;
            bestSample = bs; haveBest = true;
        }

        t += period;
    }

    for (int i = 0; i < nLat; ++i) {
        int cnt = avecCnt[i] > 0 ? avecCnt[i] : 1;
        avecMean[i] /= cnt;
        sansMean[i] /= cnt;
        std::printf("latence %d ms : avec %.2f deg (max %.2f), sans %.2f deg (max %.2f)\n",
                    (int)(latences[i] * 1000.0),
                    avecMean[i], avecMax[i], sansMean[i], sansMax[i]);
        fflush(stdout);
    }

    const TimedPose &sample = haveBest ? bestSample : hist.SampleAt(bestT, borneLatency);
    double when = bestT + period;
    NkQuatf vraiPose = NkQuatf::RotateY(NkAngle((float32)Lacet(when - 1.0)));
    double horizonMs = (when - sample.time) * 1000.0;
    NkQuatf preditSansBorne = Extrapoler(sample.pose, sample.velocity,
                                         (float32)(when - sample.time));
    NkQuatf preditAvecBorne = PredictAt(sample, when, period, true);
    std::printf("sans borne : echantillon a %.3f s, horizon %.0f ms, predit %.1f deg, vrai %.1f deg\n",
                sample.time, horizonMs, LacetDeg(preditSansBorne), LacetDeg(vraiPose));
    std::printf("avec borne : echantillon a %.3f s, horizon %.0f ms, predit %.1f deg, vrai %.1f deg\n",
                sample.time, horizonMs, LacetDeg(preditAvecBorne), LacetDeg(vraiPose));
    fflush(stdout);

    int W = 640, H = 480;
    Png png(W, H);
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x)
            png.Pixel(x, y, 255, 255, 255);
    int ox = 60, oy = H - 60;
    int sx = W - 40, sy = 40;
    png.Line(ox, oy, sx, oy, 0, 0, 0);
    png.Line(ox, oy, ox, sy, 0, 0, 0);
    double ymax = 0.0;
    for (int i = 0; i < nLat; ++i) ymax = fmax(ymax, fmax(avecMean[i], sansMean[i]));
    ymax *= 1.25;
    auto xpix = [&](double lat) -> int {
        return ox + (int)((lat / 0.090) * (double)(sx - ox));
    };
    auto ypix = [&](double val) -> int {
        return oy - (int)((val / ymax) * (double)(oy - sy));
    };
    // Ticks et legende : avec = bleu (prediction), sans = rouge (echantillon brut)
    for (int i = 0; i < nLat; ++i) {
        int x = xpix(latences[i]);
        png.Line(x, oy, x, oy + 5, 0, 0, 0);
    }
    int lx = ox + 10, ly = sy + 10;
    png.Line(lx, ly, lx + 20, ly, 0, 0, 200);
    png.Line(lx, ly + 14, lx + 20, ly + 14, 200, 0, 0);
    // Texte de la legende (police bitmap 5x7) : "AVEC" a gauche du bleu,
    // "SANS" a gauche du rouge.
    static const uint8_t font5x7[95][5] = {
        {0x00,0x00,0x00,0x00,0x00}, // space
        {0x00,0x00,0x5f,0x00,0x00}, // !
        {0x00,0x07,0x00,0x07,0x00}, // "
        {0x14,0x7f,0x14,0x7f,0x14}, // #
        {0x24,0x2a,0x7f,0x2a,0x12}, // $
        {0x23,0x13,0x08,0x64,0x62}, // %
        {0x36,0x49,0x55,0x22,0x50}, // &
        {0x00,0x05,0x03,0x00,0x00}, // '
        {0x00,0x1c,0x22,0x41,0x00}, // (
        {0x00,0x41,0x22,0x1c,0x00}, // )
        {0x14,0x08,0x3e,0x08,0x14}, // *
        {0x08,0x08,0x3e,0x08,0x08}, // +
        {0x00,0x50,0x30,0x00,0x00}, // ,
        {0x08,0x08,0x08,0x08,0x08}, // -
        {0x00,0x60,0x60,0x00,0x00}, // .
        {0x20,0x10,0x08,0x04,0x02}, // /
        {0x3e,0x51,0x49,0x45,0x3e}, // 0
        {0x00,0x42,0x7f,0x40,0x00}, // 1
        {0x42,0x61,0x51,0x49,0x46}, // 2
        {0x21,0x41,0x45,0x4b,0x31}, // 3
        {0x18,0x14,0x12,0x7f,0x10}, // 4
        {0x27,0x45,0x45,0x45,0x39}, // 5
        {0x3c,0x4a,0x49,0x49,0x30}, // 6
        {0x01,0x71,0x09,0x05,0x03}, // 7
        {0x36,0x49,0x49,0x49,0x36}, // 8
        {0x06,0x49,0x49,0x29,0x1e}, // 9
        {0x00,0x36,0x36,0x00,0x00}, // :
        {0x00,0x56,0x36,0x00,0x00}, // ;
        {0x08,0x14,0x22,0x41,0x00}, // <
        {0x14,0x14,0x14,0x14,0x14}, // =
        {0x00,0x41,0x22,0x14,0x08}, // >
        {0x02,0x01,0x51,0x09,0x06}, // ?
        {0x32,0x49,0x79,0x41,0x3e}, // @
        {0x7e,0x11,0x11,0x11,0x7e}, // A
        {0x7f,0x49,0x49,0x49,0x36}, // B
        {0x3e,0x41,0x41,0x41,0x22}, // C
        {0x7f,0x41,0x41,0x22,0x1c}, // D
        {0x7f,0x49,0x49,0x49,0x41}, // E
        {0x7f,0x09,0x09,0x09,0x01}, // F
        {0x3e,0x41,0x49,0x49,0x7a}, // G
        {0x7f,0x08,0x08,0x08,0x7f}, // H
        {0x00,0x41,0x7f,0x41,0x00}, // I
        {0x20,0x40,0x41,0x3f,0x01}, // J
        {0x7f,0x08,0x14,0x22,0x41}, // K
        {0x7f,0x40,0x40,0x40,0x40}, // L
        {0x7f,0x02,0x0c,0x02,0x7f}, // M
        {0x7f,0x04,0x08,0x10,0x7f}, // N
        {0x3e,0x41,0x41,0x41,0x3e}, // O
        {0x7f,0x09,0x09,0x09,0x06}, // P
        {0x3e,0x41,0x51,0x21,0x5e}, // Q
        {0x7f,0x09,0x09,0x19,0x66}, // R
        {0x26,0x49,0x49,0x49,0x32}, // S
        {0x01,0x01,0x7f,0x01,0x01}, // T
        {0x3f,0x40,0x40,0x40,0x3f}, // U
        {0x1f,0x20,0x40,0x20,0x1f}, // V
        {0x3f,0x40,0x38,0x40,0x3f}, // W
        {0x63,0x14,0x08,0x14,0x63}, // X
        {0x07,0x08,0x70,0x08,0x07}, // Y
        {0x61,0x51,0x49,0x45,0x43}, // Z
        {0x7f,0x41,0x41,0x00,0x00}, // [
        {0x02,0x04,0x08,0x10,0x20}, // backslash
        {0x00,0x41,0x41,0x7f,0x00}, // ]
        {0x04,0x02,0x01,0x02,0x04}, // ^
        {0x40,0x40,0x40,0x40,0x40}, // _
        {0x00,0x01,0x02,0x04,0x00}, // `
        {0x20,0x54,0x54,0x54,0x78}, // a
        {0x7f,0x48,0x44,0x44,0x38}, // b
        {0x38,0x44,0x44,0x44,0x20}, // c
        {0x38,0x44,0x44,0x48,0x7f}, // d
        {0x38,0x54,0x54,0x54,0x18}, // e
        {0x08,0x7e,0x09,0x01,0x00}, // f
        {0x0c,0x52,0x52,0x52,0x3e}, // g
        {0x7f,0x08,0x04,0x04,0x78}, // h
        {0x00,0x44,0x7d,0x40,0x00}, // i
        {0x20,0x40,0x44,0x3d,0x00}, // j
        {0x7f,0x10,0x28,0x44,0x00}, // k
        {0x00,0x41,0x7f,0x40,0x00}, // l
        {0x7c,0x04,0x18,0x04,0x78}, // m
        {0x7c,0x08,0x04,0x04,0x78}, // n
        {0x38,0x44,0x44,0x44,0x38}, // o
        {0x7c,0x14,0x14,0x14,0x08}, // p
        {0x08,0x14,0x14,0x18,0x7c}, // q
        {0x7c,0x08,0x04,0x04,0x00}, // r
        {0x14,0x54,0x54,0x54,0x20}, // s
        {0x04,0x3f,0x44,0x40,0x20}, // t
        {0x3c,0x40,0x40,0x40,0x7c}, // u
        {0x1c,0x40,0x40,0x40,0x3c}, // v
        {0x3c,0x40,0x30,0x40,0x3c}, // w
        {0x44,0x28,0x10,0x28,0x44}, // x
        {0x0c,0x40,0x40,0x40,0x3c}, // y
        {0x44,0x64,0x54,0x4c,0x44}, // z
        {0x00,0x08,0x36,0x41,0x00}, // {
        {0x00,0x00,0x7f,0x00,0x00}, // |
        {0x00,0x41,0x36,0x08,0x00}, // }
        {0x08,0x04,0x08,0x10,0x08}  // ~
    };
    auto drawChar = [&](int x, int y, char c, unsigned char r, unsigned char g,
                        unsigned char b) {
        if (c < 32 || c > 126) c = '?';
        const uint8_t *glyph = font5x7[c - 32];
        for (int col = 0; col < 5; ++col) {
            uint8_t bits = glyph[col];
            for (int row = 0; row < 7; ++row) {
                if (bits & (1 << row)) {
                    png.Pixel(x + col, y + row, r, g, b);
                }
            }
        }
    };
    auto drawText = [&](int x, int y, const char *s, unsigned char r,
                        unsigned char g, unsigned char b) {
        int xx = x;
        while (*s) {
            drawChar(xx, y, *s, r, g, b);
            xx += 6;
            ++s;
        }
    };
    drawText(lx + 24, ly - 2, "AVEC (prediction)", 0, 0, 200);
    drawText(lx + 24, ly + 12, "SANS (echantillon)", 200, 0, 0);
    for (int i = 0; i + 1 < nLat; ++i) {
        png.Line(xpix(latences[i]), ypix(avecMean[i]),
                 xpix(latences[i + 1]), ypix(avecMean[i + 1]), 0, 0, 200);
        png.Line(xpix(latences[i]), ypix(sansMean[i]),
                 xpix(latences[i + 1]), ypix(sansMean[i + 1]), 200, 0, 0);
    }
    for (int i = 0; i < nLat; ++i) {
        for (int dx = -2; dx <= 2; ++dx)
            for (int dy = -2; dy <= 2; ++dy) {
                png.Pixel(xpix(latences[i]) + dx, ypix(avecMean[i]) + dy, 0, 0, 200);
                png.Pixel(xpix(latences[i]) + dx, ypix(sansMean[i]) + dy, 200, 0, 0);
            }
    }
    png.Save("courbe.png");
    std::printf("[courbe.png] ecrite\n");
    return 0;
}
