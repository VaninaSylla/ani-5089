#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <mutex>
#include <thread>

namespace {

constexpr int kTampons = 3;

struct Soumission {
    int tampon;
    std::uint32_t largeurPasse;
    std::uint32_t hauteurPasse;
};

class Carte {
public:
    Carte(std::uint32_t largeurCible, std::uint32_t hauteurCible)
        : mLargeurCible(largeurCible), mHauteurCible(hauteurCible) {
        for (int i = 0; i < kTampons; ++i) {
            mFini[i] = true;
        }
        mFil = std::thread([this] { Boucle(); });
    }

    ~Carte() {
        {
            std::lock_guard<std::mutex> verrou(mMutex);
            mArret = true;
        }
        mCondition.notify_all();
        mFil.join();
    }

    void Soumettre(const Soumission &s) {
        std::lock_guard<std::mutex> verrou(mMutex);
        mFini[s.tampon] = false;
        mFile.push_back(s);
        mCondition.notify_all();
    }

    void AttendreTampon(int tampon) {
        std::unique_lock<std::mutex> verrou(mMutex);
        mCondition.wait(verrou, [this, tampon] { return mFini[tampon]; });
    }

private:
    void Boucle() {
        for (;;) {
            std::unique_lock<std::mutex> verrou(mMutex);
            mCondition.wait(verrou, [this] { return mArret || !mFile.empty(); });
            if (mArret) {
                return;
            }
            Soumission s = mFile.front();
            mFile.pop_front();
            if (s.largeurPasse == mLargeurCible &&
                s.hauteurPasse == mHauteurCible) {
                mFini[s.tampon] = true;
                mCondition.notify_all();
            }
        }
    }

    std::uint32_t mLargeurCible;
    std::uint32_t mHauteurCible;
    std::mutex mMutex;
    std::condition_variable mCondition;
    std::deque<Soumission> mFile;
    bool mFini[kTampons];
    bool mArret = false;
    std::thread mFil;
};

struct Renderer {
    std::uint64_t cibleFinale = 0;
    std::uint32_t largeur = 1280;
    std::uint32_t hauteur = 720;

    void SetFinalColorTarget(std::uint64_t poignee) { cibleFinale = poignee; }

    void SetRenderSizeOverride(std::uint32_t l, std::uint32_t h) {
        largeur = l;
        hauteur = h;
    }
};

}  // namespace

int main() {
    const std::uint32_t largeurCible = 640;
    const std::uint32_t hauteurCible = 360;

    Carte carte(largeurCible, hauteurCible);
    Renderer rendu;

    rendu.SetFinalColorTarget(0x1000);
#ifndef SANS_TAILLE
    rendu.SetRenderSizeOverride(largeurCible, hauteurCible);
#endif

    const int nombreImages = 12;
    for (int image = 1; image <= nombreImages; ++image) {
        const int tampon = (image - 1) % kTampons;
        carte.AttendreTampon(tampon);
        carte.Soumettre({tampon, rendu.largeur, rendu.hauteur});
        std::printf("image : %d, etat : dessinee\n", image);
        std::fflush(stdout);
    }
    return 0;
}
