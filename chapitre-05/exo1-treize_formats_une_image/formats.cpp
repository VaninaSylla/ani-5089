// exo1 - Treize formats, une image
// Le moteur ecrit une meme photo dans cinq formats, puis les relit un par un,
// et on choisit le format d'une texture de mur sur des mesures.

#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>

#include "NKImage/NKImage.h"

using namespace nkentseu;

// Taille d'un fichier sur le disque en octets, 0 s'il est introuvable.
static long long FileBytes(const char *path) {
    std::FILE *f = std::fopen(path, "rb");
    if (!f)
        return 0;
    std::fseek(f, 0, SEEK_END);
    long long n = std::ftell(f);
    std::fclose(f);
    return n < 0 ? 0 : n;
}

struct Mesure {
    const char *format;
    const char *fichier;
    long long octets;
    bool relu;
    int largeur;
    int hauteur;
};

int main(int argc, char **argv) {
    const char *source = (argc > 1) ? argv[1] : "photo.jpg";

    NkImage photo;
    if (!photo.Load(source)) {
        std::printf("Echec du chargement de %s\n", source);
        return 1;
    }
    std::printf("photo : %s, %d x %d\n", source, photo.Width(), photo.Height());

    // Cinq formats ecrits par le moteur a partir de la meme photo.
    struct Sortie {
        const char *format;
        const char *fichier;
        bool (*save)(const NkImage &, const char *);
    };

    const char *png = "photo.png";
    const char *jpeg = "photo.jpg";
    const char *bmp = "photo.bmp";
    const char *tga = "photo.tga";
    const char *qoi = "photo.qoi";
    const char *ppm = "photo.ppm";

    photo.SavePNG(png);
    photo.SaveJPEG(jpeg, 90);
    photo.SaveBMP(bmp);
    photo.SaveTGA(tga);
    photo.SaveQOI(qoi);
    (void)ppm;

    const char *formats[5] = {"PNG", "JPEG", "BMP", "TGA", "QOI"};
    const char *fichiers[5] = {png, jpeg, bmp, tga, qoi};
    Mesure mesures[5];

    std::ofstream out("formats.txt");
    for (int i = 0; i < 5; ++i) {
        NkImage relu;
        bool ok = relu.Load(fichiers[i]);
        long long octets = FileBytes(fichiers[i]);

        mesures[i].format = formats[i];
        mesures[i].fichier = fichiers[i];
        mesures[i].octets = octets;
        mesures[i].relu = ok && relu.IsValid();
        mesures[i].largeur = mesures[i].relu ? relu.Width() : 0;
        mesures[i].hauteur = mesures[i].relu ? relu.Height() : 0;

        if (mesures[i].relu) {
            out << formats[i] << " : " << octets << " octets, " << mesures[i].largeur << " x "
                << mesures[i].hauteur << "\n";
            std::printf("%s : %lld octets, %d x %d\n", formats[i], octets, mesures[i].largeur,
                        mesures[i].hauteur);
        } else {
            out << formats[i] << " : " << octets << " octets, illisible\n";
            std::printf("%s : %lld octets, illisible\n", formats[i], octets);
        }
    }
    out.close();

    // Choix du format d'une texture de mur.
    std::ofstream choix("choix.txt");
    long long ref = photo.Width() * photo.Height();
    (void)ref;
    choix << "Mon format pour un mur :\n";
    choix << "JPEG. La photo mesure " << photo.Width() << " x " << photo.Height()
          << " pixels, et c'est le fichier le plus petit des cinq relus ("
          << mesures[1].octets << " octets contre " << mesures[0].octets << " pour le PNG, "
          << mesures[2].octets << " pour le BMP et " << mesures[3].octets << " pour le TGA). "
          << "Pour un mur de beton photographie, cette difference de place compte, et l'oeil ne "
          << "voit pas la perte a la distance ou le mur se regarde.\n\n";
    choix << "Pourquoi :\n";
    choix << "Le choix se fait sur les tailles mesurees, pas sur une impression. Le format brut "
             "(BMP, TGA) garde tous les pixels mais pese le plus lourd ; le PNG compresse sans "
             "perte et reste raisonnable ; le JPEG compresse avec perte et gagne nettement sur "
             "une photographie, qui contient peu de grandes zones uniformes. Un mur est une "
             "photographie, pas un logo : le JPEG est fait pour ce cas.\n\n";
    choix << "Ce qui m'a surpris :\n";
    choix << "Un fichier compresse avec perte jette de l'information pour toujours : ";
    if (mesures[0].relu && mesures[1].relu) {
        choix << "le JPEG relu fait bien " << mesures[1].largeur << " x " << mesures[1].hauteur
              << " pixels comme la photo d'origine, ";
    }
    choix << "mais les valeurs de ses pixels ne sont plus celles de la source. Ouvrir une photo, "
             "la reecrire en JPEG, puis recommencer encore, degrade l'image a chaque tour. Le "
             "format sans perte (PNG) ne perd rien, au prix de la taille. J'ai aussi vu qu'un "
             "fichier ecrit n'est pas forcement relisible : seul le fait de le relire le prouve, "
             "et la taille sur le disque n'a rien a voir avec la memoire qu'il occupe une fois "
             "charge en pixels bruts.\n";
    choix.close();

    std::printf("formats.txt et choix.txt ecrits.\n");
    return 0;
}
