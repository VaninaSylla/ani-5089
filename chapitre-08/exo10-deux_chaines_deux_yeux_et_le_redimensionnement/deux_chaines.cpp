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

}  // namespace

int main() {
    const std::uint32_t images = 3;
    const int nombreTrames = 30;
    const int trameRedimension = 15;

    std::FILE *trace = std::fopen("trace.txt", "w");
    std::FILE *resize = std::fopen("resize.txt", "w");
    if (trace == nullptr || resize == nullptr) {
        return 1;
    }

    ChaineEchange gauche;
    ChaineEchange droit;
    gauche.Create(images, "oeil gauche");
    droit.Create(images, "oeil droit");
    EnregistrerImages(gauche, 0x1000, images);
    EnregistrerImages(droit, 0x2000, images);

    for (int image = 1; image <= nombreTrames; ++image) {
        if (image == trameRedimension) {
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
        }

        std::uint32_t indexGauche = 0;
        std::uint32_t indexDroit = 0;
        if (!TrameOeil(gauche, indexGauche) ||
            !TrameOeil(droit, indexDroit)) {
            std::fclose(trace);
            std::fclose(resize);
            return 1;
        }

        std::fprintf(trace, "image : %d, gauche : %u, droit : %u\n", image,
                     static_cast<unsigned>(indexGauche),
                     static_cast<unsigned>(indexDroit));
    }

    std::fclose(trace);
    std::fclose(resize);
    std::printf("trente images tracees, redimensionnement a l'image %d\n",
                trameRedimension);
    return 0;
}
