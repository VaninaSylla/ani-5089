// =============================================================================
//  Sprint 6, exercice 7 — le coût mesuré.
//
//  Exo5 a départagé les deux façons d'écrire « lacet puis tangage » ; exo6
//  a montré que la salle tient la tête en quaternion. Reste à payer la note :
//  mesurer le coût d'UNE composition, pour décider en connaissance de cause.
//
//  Deux chemins, même travail :
//    - matrices    : NkQuatf(NkMat4f::RotationY(lacet) * NkMat4f::RotationX(30)).Normalized()  ;
//    - quaternions : NkQuatf::RotateY(lacet) * NkQuatf::RotateX(30)  ;
//  Un million d'itérations par mesure, lacet parcourant 0 à 359,9° par pas
//  de 0,1° ((i % 3600) * 0,1 — trois tours complets), tangage figé à 30°.
//  Les quatre composantes du quaternion résultat sont SOMMÉES et les sommes
//  sont imprimées : la boucle ne peut pas être supprimée par le compilateur,
//  et la comparaison des sommes entre mesures montre qu'aucune mesure ne
//  file pas à côté de la précédente.
//
	// Avant toute mesure : les deux chemins sont confrontés sur un échantillon
	// de poses — comparer la vitesse de deux chemins qui donneraient des
	// rotations différentes n'aurait aucun sens. La comparaison tolère le
	// signe, parce que q et −q sont la même rotation (double couverture).
//
//  Trois mesures par chemin et par configuration, à la NkChrono (lecture en
//  float64). Le nom de la configuration vient de NDEBUG : le même cout.cpp
//  compilé en Debug et en Release imprime chacun ses six lignes, format
//  « <config>, <chemin> : <temps> ms » — les douze lignes atterrissent dans
//  mesures.txt. Le Release est obtenu par optimize("Speed") posé dans le
//  filter("config:Release") du projet (MaSalle.jenga / projet.jenga), au
//  même niveau que user3d().
//
//  Pas de fenêtre ici : c'est un banc de mesure, pas une salle — le verdict
//  est dans les douze nombres.
// =============================================================================
#include "NKWindow/NKMain.h"
#include "NKMath/NkQuat.h"
#include "NKMath/NkAngle.h"
#include "NKMath/NkMat.h"
#include "NKTime/NkChrono.h"

#include <cmath>
#include <cstdio>

using namespace nkentseu;
using namespace nkentseu::math;

static void ConfigureAppData(NkAppData &d) { d.appName = "MaSalle"; }
NK_REGISTER_ENTRY_APPDATA_UPDATER(ConfigureAppData)

namespace {

	const int64 kIterations = 1000000;
	const float32 kTangageFixe = 30.f;

	struct Mesure {
		float64 ms = 0.;
		float64 w = 0., x = 0., y = 0., z = 0.;
	};

	// Le chemin matriciel : deux rotations de matrice, leur produit, puis le
	// passage au quaternion — celui qu'exo5 utilisait comme juge.
	Mesure MesurerMatrices() {
		Mesure m;
		NkChrono chrono;
		for (int64 i = 0; i < kIterations; ++i) {
			const float32 lacetDeg = (float32)((i % 3600) * 0.1);
			const NkQuatf q =
				NkQuatf(NkMat4f::RotationY(NkAngle(lacetDeg)) *
						NkMat4f::RotationX(NkAngle(kTangageFixe)))
					.Normalized();
			m.w += (float64)q.w;
			m.x += (float64)q.x;
			m.y += (float64)q.y;
			m.z += (float64)q.z;
		}
		m.ms = chrono.Reset().ToMilliseconds();
		return m;
	}

	// Le chemin quaternion : le produit de Hamilton lui-même.
	Mesure MesurerQuaternions() {
		Mesure m;
		NkChrono chrono;
		for (int64 i = 0; i < kIterations; ++i) {
			const float32 lacetDeg = (float32)((i % 3600) * 0.1);
			const NkQuatf q = NkQuatf::RotateY(NkAngle(lacetDeg)) *
							  NkQuatf::RotateX(NkAngle(kTangageFixe));
			m.w += (float64)q.w;
			m.x += (float64)q.x;
			m.y += (float64)q.y;
			m.z += (float64)q.z;
		}
		m.ms = chrono.Reset().ToMilliseconds();
		return m;
	}

	// Avant de chronométrer : les deux chemins donnent-ils la MÊME rotation ?
	// Échantillon de 26 poses lacet (pas de 0,1°), comparaison composante par
	// composante à 1e-5 près — après alignement du signe : un quaternion et
	// son opposé sont la même rotation (double couverture), et les deux
	// chemins ne choisissent pas toujours la même moitié : NkAngle(191.8°)
	// est wrapé en −168.2° par le moteur, et la matrice est relue par un
	// constructeur qui pick sa propre composante dominante. Même matrice,
	// quaternions parfois opposés — ce n'est pas un désaccord.
	bool RotationsIdentiques() {
		int echantillons = 0, signeInverse = 0;
		for (int32 pas = 0; pas < 3600; pas += 137) {
			const float32 lacetDeg = (float32)pas * 0.1f;
			NkQuatf a =
				NkQuatf(NkMat4f::RotationY(NkAngle(lacetDeg)) *
						NkMat4f::RotationX(NkAngle(kTangageFixe)))
					.Normalized();
			NkQuatf b = NkQuatf::RotateY(NkAngle(lacetDeg)) *
						NkQuatf::RotateX(NkAngle(kTangageFixe));
			++echantillons;
			// Alignement du signe : si le produit scalaire est négatif, a et b
			// sont opposés — on flippe b avant de comparer.
			const float32 dot = a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
			if (dot < 0.f) {
				b = NkQuatf{-b.x, -b.y, -b.z, -b.w};
				++signeInverse;
			}
			if (std::fabs(a.w - b.w) > 1e-5f || std::fabs(a.x - b.x) > 1e-5f ||
				std::fabs(a.y - b.y) > 1e-5f || std::fabs(a.z - b.z) > 1e-5f) {
				std::printf("rotations DIFFERENTES a lacet %.1f : "
							"matrices (%.6f, %.6f, %.6f, %.6f), "
							"quaternions (%.6f, %.6f, %.6f, %.6f)\n",
							(double)lacetDeg, (double)a.w, (double)a.x,
							(double)a.y, (double)a.z, (double)b.w, (double)b.x,
							(double)b.y, (double)b.z);
				return false;
			}
		}
		std::printf("memes rotations : oui (%d poses echantillonnees, dont %d "
					"avec quaternion oppose — q et -q, meme rotation)\n",
					echantillons, signeInverse);
		return true;
	}

} // namespace

int nkmain(const NkEntryState &state) {
	(void)state;

	const char *config =
#ifdef NDEBUG
		"Release";
#else
		"Debug";
#endif

	// Échauffement : la première mesure d'une série est souvent la plus
	// bruyante (horloge, mise en cache). On la jette, les six mesures
	// viennent ensuite.
	volatile float64 chaud = 0.;
	for (int64 i = 0; i < 100000; ++i) {
		const float32 lacetDeg = (float32)((i % 3600) * 0.1);
		const NkQuatf q = NkQuatf::RotateY(NkAngle(lacetDeg)) *
						  NkQuatf::RotateX(NkAngle(kTangageFixe));
		chaud += (float64)(q.w + q.x + q.y + q.z);
	}
	(void)chaud;

	std::printf("[cout] %lld iterations par mesure ; lacet 0 a 359.9 deg par "
				"pas de 0.1 ; tangage fixe a %.0f deg\n",
				(long long)kIterations, (double)kTangageFixe);
	RotationsIdentiques();

	for (int mesure = 0; mesure < 3; ++mesure) {
		const Mesure m = MesurerMatrices();
		std::printf("%s, matrices : %.3f ms\n", config, m.ms);
		std::printf("sommes matrices, mesure %d : w=%.6f x=%.6f y=%.6f z=%.6f\n",
					mesure + 1, m.w, m.x, m.y, m.z);
		std::fflush(stdout);
	}
	for (int mesure = 0; mesure < 3; ++mesure) {
		const Mesure m = MesurerQuaternions();
		std::printf("%s, quaternions : %.3f ms\n", config, m.ms);
		std::printf("sommes quaternions, mesure %d : w=%.6f x=%.6f y=%.6f z=%.6f\n",
					mesure + 1, m.w, m.x, m.y, m.z);
		std::fflush(stdout);
	}
	return 0;
}
