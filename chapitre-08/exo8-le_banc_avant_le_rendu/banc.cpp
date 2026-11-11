#include "chaine.h"

#include <cstdint>
#include <cstdio>

namespace {

const char *Verdict(bool ok) { return ok ? "accepte" : "REFUSE"; }

}  // namespace

int main() {
    ChaineEchange chaine;
    DescChaine desc;
    desc.imageCount = 3;
    chaine.Create(desc);

    for (int tour = 1; tour <= 6; ++tour) {
        std::uint32_t index = 0;
        const bool pris = chaine.AcquireImage(index);
        const bool rendu = chaine.ReleaseImage();
        std::printf("tour %d : emprunt %s index %u, rendu %s, a soumettre %u\n",
                    tour, Verdict(pris), static_cast<unsigned>(index),
                    Verdict(rendu),
                    static_cast<unsigned>(chaine.GetLastReleasedIndex()));
    }

    std::uint32_t index = 0;
    const bool premier = chaine.AcquireImage(index);
    const bool second = chaine.AcquireImage(index);
    std::printf("double emprunt : premier %s, second %s\n", Verdict(premier),
                Verdict(second));
    chaine.ReleaseImage();

    const bool sansEmprunt = chaine.ReleaseImage();
    std::printf("rendu sans emprunt : %s\n", Verdict(sansEmprunt));

    ChaineEchange trop;
    DescChaine grosse;
    grosse.imageCount = ChaineEchange::kMaxImages + 1u;
    std::printf("chaine de %u images : %s\n",
                static_cast<unsigned>(grosse.imageCount),
                Verdict(trop.Create(grosse)));

    if (trop.AcquireImage(index)) {
        return 1;
    }

    return 0;
}
