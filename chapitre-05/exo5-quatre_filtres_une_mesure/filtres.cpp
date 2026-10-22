// exo5 - Quatre filtres, une mesure
//
// On prend une image, on la reduit a 2048 x 2048 pour en faire la source,
// puis on reduit cette source en 256 x 256 avec chacun des quatre filtres
// du moteur. On chronometre chaque filtre, on garde la moyenne de vingt
// reductions, et on compare les quatre resultats pixel par pixel au
// resultat bilineaire.
//
// Fichiers rendus :
//   filtres.cpp    : ce fichier
//   mesures.txt    : les quatre lignes de mesures
//   nearest.png    : le resultat du plus proche voisin
//   bilinear.png   : le resultat du bilineaire
//   bicubic.png    : le resultat du bicubique
//   lanczos3.png   : le resultat du Lanczos-3
//   reponse.txt    : la reponse redigee
//
// IMPORTANT : ce programme se mesure en RELEASE. En Debug, on chronometre
// surtout le compilateur.

#include <cstdio>
#include <cstdint>
#include <cmath>

#include <NKImage/Core/NkImage.h>
#include <NKTime/NkChrono.h>
#include <NKFileSystem/NkPath.h>

using namespace nkentseu;

// ── La photo de depart ────────────────────────────────────────────────────
// Chemin RELATIF : il part du dossier d'ou le programme est lance.
static const char *kPhoto = "murs/beton-photo.jpg";

// ── Les tailles ───────────────────────────────────────────────────────────
static const int32 kTailleSource = 2048;   // la source, apres reduction
static const int32 kTailleResultat = 256;  // le resultat de chaque filtre
static const int kRepetitions = 20;        // reductions par filtre

// ── Les quatre filtres du moteur ──────────────────────────────────────────
struct Filtre {
	const char *nom;    // le nom de l'enum, tel qu'il doit etre ecrit
	NkResizeFilter f;   // le filtre
	const char *fichier;// le PNG de sortie
};

static const Filtre kFiltres[4] = {
	{"NK_NEAREST", NkResizeFilter::NK_NEAREST, "nearest.png"},
	{"NK_BILINEAR", NkResizeFilter::NK_BILINEAR, "bilinear.png"},
	{"NK_BICUBIC", NkResizeFilter::NK_BICUBIC, "bicubic.png"},
	{"NK_LANCZOS3", NkResizeFilter::NK_LANCZOS3, "lanczos3.png"},
};

static const int kIndiceBilineaire = 1;

// ───────────────────────────────────────────────────────────────────────────
// Compte les pixels qui different sur au moins un canal de couleur entre
// deux images de meme taille. C'est la comparaison demanded par l'exercice :
// elle est faite par le programme, pas a l'oeil.
// ───────────────────────────────────────────────────────────────────────────
static int CompterDifferences(const NkImage &a, const NkImage &b) {
	if (a.Width() != b.Width() || a.Height() != b.Height()) {
		return -1;
	}
	int different = 0;
	for (int32 y = 0; y < a.Height(); ++y) {
		for (int32 x = 0; x < a.Width(); ++x) {
			const math::NkColor ca = a.GetPixel(x, y);
			const math::NkColor cb = b.GetPixel(x, y);
			if (ca.r != cb.r || ca.g != cb.g || ca.b != cb.b) {
				++different;
			}
		}
	}
	return different;
}

int main() {
	// ── 1. La photo ───────────────────────────────────────────────────────
	NkImage photo;
	bool chargee = photo.Load(kPhoto);
	if (!chargee) {
		// Le chemin relatif depend du dossier de lancement. On tente alors
		// le chemin du moteur, pour que le programme reste utilisable lance
		// d'ailleurs.
		NkString trouve = NkPath::LocateResource(kPhoto, false);
		if (!trouve.Empty()) {
			chargee = photo.Load(trouve.CStr());
			std::printf("[filtres] chemin relatif introuvable, chemin du moteur utilise : %s\n",
						trouve.CStr());
		}
	}
	if (!chargee || !photo.IsValid()) {
		std::printf("[filtres] ERREUR : impossible de charger %s\n", kPhoto);
		return 1;
	}
	std::printf("[filtres] photo chargee : %s (%d x %d)\n", kPhoto, (int)photo.Width(),
				(int)photo.Height());

	// ── 2. La source : 2048 x 2048 ────────────────────────────────────────
	// "Reduisez-la d'abord en 2048 x 2048 avec Resize, et gardez ce
	// resultat : c'est votre source."
	NkImage source = photo.Resize(kTailleSource, kTailleSource, NkResizeFilter::NK_BILINEAR);
	if (!source.IsValid() || source.Width() != kTailleSource || source.Height() != kTailleSource) {
		std::printf("[filtres] ERREUR : la source n'est pas en %d x %d\n", kTailleSource,
					kTailleSource);
		return 1;
	}
	std::printf("[filtres] source : %d x %d\n", (int)source.Width(), (int)source.Height());

	// ── 3. Les quatre filtres, chronometres ───────────────────────────────
	// On garde le premier resultat de chaque filtre (les reductions sont
	// deterministes : les vingt donnent le meme resultat).
	NkImage resultats[4];
	double totalUs[4] = {0.0, 0.0, 0.0, 0.0};
	double premierUs[4] = {0.0, 0.0, 0.0, 0.0};
	double minUs[4] = {0.0, 0.0, 0.0, 0.0};
	double maxUs[4] = {0.0, 0.0, 0.0, 0.0};

	for (int i = 0; i < 4; ++i) {
		for (int r = 0; r < kRepetitions; ++r) {
			NkChrono chrono; // demarre a la construction
			NkImage reduite = source.Resize(kTailleResultat, kTailleResultat, kFiltres[i].f);
			const double us = chrono.Elapsed().ToMicroseconds();

			if (!reduite.IsValid() || reduite.Width() != kTailleResultat ||
				reduite.Height() != kTailleResultat) {
				std::printf("[filtres] ERREUR : %s n'a pas donne une image %d x %d\n",
							kFiltres[i].nom, kTailleResultat, kTailleResultat);
				return 1;
			}

			totalUs[i] += us;
			if (r == 0) {
				premierUs[i] = us;
				resultats[i] = std::move(reduite);
			}
			if (r == 0 || us < minUs[i]) {
				minUs[i] = us;
			}
			if (us > maxUs[i]) {
				maxUs[i] = us;
			}
		}
		const double moyenne = totalUs[i] / (double)kRepetitions;
		std::printf("[filtres] %-12s moyenne %8.1f us  (premiere %8.1f, min %8.1f, max %8.1f)\n",
					kFiltres[i].nom, moyenne, premierUs[i], minUs[i], maxUs[i]);
	}

	// ── 4. Les quatre PNG ─────────────────────────────────────────────────
	for (int i = 0; i < 4; ++i) {
		if (!resultats[i].SavePNG(kFiltres[i].fichier)) {
			std::printf("[filtres] ERREUR : ecriture impossible de %s\n", kFiltres[i].fichier);
			return 1;
		}
		std::printf("[filtres] ecrit : %s (%d x %d)\n", kFiltres[i].fichier,
					(int)resultats[i].Width(), (int)resultats[i].Height());
	}

	// ── 5. La comparaison, pixel par pixel, contre le bilineaire ──────────
	int32 differences[4] = {0, 0, 0, 0};
	for (int i = 0; i < 4; ++i) {
		differences[i] = CompterDifferences(resultats[i], resultats[kIndiceBilineaire]);
	}

	// ── 6. Les quatre lignes de mesures ───────────────────────────────────
	{
		std::FILE *f = std::fopen("mesures.txt", "w");
		if (!f) {
			std::printf("[filtres] ERREUR : impossible d'ecrire mesures.txt\n");
			return 1;
		}
		for (int i = 0; i < 4; ++i) {
			const long long t = (long long)std::llround(totalUs[i] / (double)kRepetitions);
			std::fprintf(f, "%s : %lld us, differences : %d\n", kFiltres[i].nom, t, differences[i]);
			std::printf("[filtres] %s : %lld us, differences : %d\n", kFiltres[i].nom, t, differences[i]);
		}
		std::fclose(f);
	}

	// ── 7. Le detail, pour la reponse ────────────────────────────────────
	std::printf("[filtres] detail (microsecondes, moyenne de %d reductions) :\n", kRepetitions);
	for (int i = 0; i < 4; ++i) {
		std::printf("[filtres]   %-12s moyenne %8.1f  premiere %8.1f  min %8.1f  max %8.1f  "
					"differences %d\n",
					kFiltres[i].nom, totalUs[i] / (double)kRepetitions, premierUs[i], minUs[i],
					maxUs[i], differences[i]);
	}
	const double base = totalUs[kIndiceBilineaire] / (double)kRepetitions;
	for (int i = 0; i < 4; ++i) {
		const double moy = totalUs[i] / (double)kRepetitions;
		std::printf("[filtres]   %-12s x%.2f le bilineaire\n", kFiltres[i].nom, moy / base);
	}

	std::printf("[filtres] fin\n");
	return 0;
}
