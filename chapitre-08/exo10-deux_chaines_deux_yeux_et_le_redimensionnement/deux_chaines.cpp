#include "NKWindow/NKMain.h"
#include "NKWindow/Core/NkWindow.h"
#include "NKWindow/Core/NkWindowConfig.h"
#include "NKEvent/NkEventSystem.h"
#include "NKEvent/NkWindowEvent.h"
#include "NKTime/NkChrono.h"

#include <cstdint>
#include <cstdio>

namespace {

constexpr std::uint32_t kMaxImages = 4;

class ChaineEchange {
public:
    bool Create(std::uint32_t imageCount, const char *nom) {
        if (imageCount == 0 || imageCount > kMaxImages) {
            return false;
        }
        mCount = imageCount;
        mNom = nom;
        mNext = 0;
        mAcquired = false;
        for (std::uint32_t i = 0; i < kMaxImages; ++i) {
            mHandles[i] = 0;
        }
        mAlive = true;
        return true;
    }

    void Destroy() {
        mAlive = false;
        mAcquired = false;
    }

    bool RegisterImage(std::uint32_t index, std::uint64_t handle) {
        if (!mAlive || index >= mCount) {
            return false;
        }
        mHandles[index] = handle;
        return true;
    }

    bool AcquireImage(std::uint32_t &index) {
        if (!mAlive || mAcquired) {
            return false;
        }
        mAcquired = true;
        index = mNext;
        return true;
    }

    bool ReleaseImage() {
        if (!mAlive || !mAcquired) {
            return false;
        }
        mAcquired = false;
        mNext = (mNext + 1u) % mCount;
        return true;
    }

    std::uint32_t GetLastReleasedIndex() const {
        return (mNext + mCount - 1u) % mCount;
    }

    std::uint64_t Submit() const {
        return mHandles[GetLastReleasedIndex()];
    }

    const char *Name() const { return mNom; }

private:
    std::uint64_t mHandles[kMaxImages] = {};
    std::uint32_t mCount = 1;
    std::uint32_t mNext = 0;
    bool mAcquired = false;
    bool mAlive = false;
    const char *mNom = "";
};

bool TrameOeil(ChaineEchange &chaine, std::uint32_t &index) {
    if (!chaine.AcquireImage(index)) {
        return false;
    }
    if (!chaine.ReleaseImage()) {
        return false;
    }
    (void)chaine.Submit();
    return true;
}

void EnregistrerImages(ChaineEchange &chaine, std::uint64_t base,
                       std::uint32_t images) {
    for (std::uint32_t i = 0; i < images; ++i) {
        chaine.RegisterImage(i, base + i);
    }
}

void RecreerLesChaines(ChaineEchange &gauche, ChaineEchange &droit,
                       std::uint32_t images, std::FILE *resize) {
    gauche.Destroy();
    std::fprintf(resize, "detruit chaine gauche\n");
    droit.Destroy();
    std::fprintf(resize, "detruit chaine droite\n");

    gauche.Create(images, "oeil gauche");
    std::fprintf(resize, "recree chaine gauche\n");
    droit.Create(images, "oeil droit");
    std::fprintf(resize, "recree chaine droite\n");

    EnregistrerImages(gauche, 0x3000, images);
    std::fprintf(resize, "reenregistre images gauche\n");
    EnregistrerImages(droit, 0x4000, images);
    std::fprintf(resize, "reenregistre images droite\n");
    std::fflush(resize);
}

static void ConfigureAppData(nkentseu::NkAppData &d) {
    d.appName = "Exo10DeuxChaines";
}
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

}  // namespace

int nkmain(const nkentseu::NkEntryState &state) {
    (void)state;

    const std::uint32_t images = 3;
    const int nombreTrames = 30;
    const int trameRedimension = 15;

    std::FILE *trace = std::fopen("trace.txt", "wb");
    std::FILE *resize = std::fopen("resize.txt", "wb");
    if (trace == nullptr || resize == nullptr) {
        return 1;
    }

    ChaineEchange gauche;
    ChaineEchange droit;
    gauche.Create(images, "oeil gauche");
    droit.Create(images, "oeil droit");
    EnregistrerImages(gauche, 0x1000, images);
    EnregistrerImages(droit, 0x2000, images);

    nkentseu::NkWindowConfig cfg;
    cfg.title = "Exo10 - deux chaines et le redimensionnement";
    cfg.width = 640;
    cfg.height = 360;
    cfg.centered = true;
    cfg.resizable = true;
    cfg.vsync = false;

    nkentseu::NkWindow fenetre(cfg);
    if (!fenetre.IsValid()) {
        std::fprintf(stderr, "la fenetre n'a pas pu s'ouvrir\n");
        std::fclose(trace);
        std::fclose(resize);
        return 1;
    }

    bool ferme = false;
    bool resizeDemande = false;
    bool resizeTraite = false;

    auto traiterEvenements = [&]() {
        nkentseu::NkEvent *ev = nullptr;
        while (nkentseu::NkEvents().PollEvent(ev)) {
            if (ev->As<nkentseu::NkWindowCloseEvent>() != nullptr) {
                ferme = true;
            }
            if (ev->As<nkentseu::NkWindowResizeEvent>() != nullptr &&
                resizeDemande && !resizeTraite) {
                RecreerLesChaines(gauche, droit, images, resize);
                resizeTraite = true;
            }
        }
    };

    for (int image = 1; image <= nombreTrames && !ferme; ++image) {
        if (image == trameRedimension) {
            fenetre.SetSize(800, 450);
            resizeDemande = true;
            for (int attente = 0; attente < 50 && !resizeTraite && !ferme;
                 ++attente) {
                traiterEvenements();
                if (!resizeTraite) {
                    nkentseu::NkChrono::SleepMilliseconds(1);
                }
            }
        }

        traiterEvenements();
        if (ferme) {
            break;
        }

        std::uint32_t indexGauche = 0;
        std::uint32_t indexDroit = 0;
        if (!TrameOeil(gauche, indexGauche) ||
            !TrameOeil(droit, indexDroit)) {
            std::fprintf(stderr, "une chaine a refuse une image\n");
            std::fclose(trace);
            std::fclose(resize);
            return 1;
        }

        std::fprintf(trace, "image : %d, gauche : %u, droit : %u\n", image,
                     static_cast<unsigned>(indexGauche),
                     static_cast<unsigned>(indexDroit));
        std::fflush(trace);

        nkentseu::NkChrono::SleepMilliseconds(16);
    }

    std::fclose(trace);
    std::fclose(resize);

    if (ferme) {
        std::printf("fenetre fermee avant la fin des trente images\n");
        return 0;
    }
    std::printf("trente images tracees, redimensionnement a l'image %d\n",
                trameRedimension);
    return 0;
}
