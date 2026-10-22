// exo12 - Les quatre attenuations
//
// On mesure, avec le moteur lui-meme, le gain des quatre modeles
// d'attenuation a cinq distances, puis on recommence avec un facteur de
// chute de 2.
//
// Fichiers rendus :
//   attenuations.cpp : ce fichier
//   mesures.txt      : les quarante gains
//   courbe.png       : les quatre courbes du reglage rolloff 1
//   reponse.txt      : la reponse redigee
//
// Ce programme n'a ni fenetre ni sortie sonore : cfg.backend =
// AudioBackendType::NULL_OUTPUT. L'auditeur est a l'origine et regarde vers
// -Z : avant (0, 0, -1), haut (0, 1, 0). Le son est un sinus doux, place
// devant l'auditeur, en (0, 0, -d).
//
// Pour chaque mesure on rend 48 000 trames par blocs de 256 avec
// RenderToBuffer, et on calcule la racine de la moyenne des carres sur les
// DEUX oreilles ensemble. Chaque gain est le rapport de cette mesure a celle
// du modele NONE a la meme distance.

#include <cstdio>
#include <cmath>

#include <NKAudio/NkAudio.h>

using namespace nkentseu;
using namespace nkentseu::audio;

static const int32 kEchantillons = 48000;
static const int32 kBloc = 256;
static const int32 kCanaux = 2;

struct Modele {
	const char *nom;
	AttenuationModel modele;
};
static const Modele kModeles[4] = {
	{"NONE", AttenuationModel::NONE},
	{"INVERSE", AttenuationModel::INVERSE},
	{"LINEAR", AttenuationModel::LINEAR},
	{"EXPONENTIAL", AttenuationModel::EXPONENTIAL},
};

static const float kDistances[5] = {0.5f, 1.f, 2.f, 4.f, 8.f};
static const float kRolloffs[2] = {1.f, 2.f};

// ───────────────────────────────────────────────────────────────────────────
// Une mesure : on rend 48 000 trames et on renvoie la racine de la moyenne
// des carres, les deux oreilles ensemble.
// ───────────────────────────────────────────────────────────────────────────
static float Mesurer(AudioEngine &moteur, AttenuationModel modele, float distance, float rolloff) {
	// La source ne bouge pas pendant la mesure : on l'arrete avant de la
	// replacer.
	moteur.StopAll();

	VoiceParams p;
	p.looping = true;
	p.source3d.positional = true;
	p.source3d.position[0] = 0.f;
	p.source3d.position[1] = 0.f;
	p.source3d.position[2] = -distance;
	p.source3d.minDistance = 1.f;
	p.source3d.maxDistance = 100.f;
	p.source3d.rolloffFactor = rolloff;
	p.source3d.attenuationModel = modele;

	// Un sinus doux : le moteur adoucit de toute facon les fortes valeurs.
	AudioSample ton = AudioGenerator::GenerateTone(440.f, 2.f, WaveformType::SINE, 48000, 0.1f);
	AudioHandle voix = moteur.Play(ton, p);

	// L'auditeur est a l'origine et regarde vers -Z.
	moteur.SetListenerPosition(0.f, 0.f, 0.f);
	moteur.SetListenerOrientation(0.f, 0.f, -1.f, 0.f, 1.f, 0.f);

	double somme = 0.0;
	int64 total = 0;
	float tampon[256 * 2];
	int32 rendues = 0;
	while (rendues < kEchantillons) {
		const int32 frames = (kEchantillons - rendues) < kBloc ? (kEchantillons - rendues) : kBloc;
		moteur.RenderToBuffer(tampon, frames, kCanaux);
		for (int32 i = 0; i < frames * kCanaux; ++i) {
			somme += (double)tampon[i] * (double)tampon[i];
			++total;
		}
		rendues += frames;
	}

	moteur.Stop(voix);
	AudioLoader::Free(ton);
	return (float)std::sqrt(somme / (double)total);
}

int main() {
	// ── Le moteur de son, sans sortie ─────────────────────────────────────
	AudioEngineConfig cfg;
	cfg.backend = AudioBackendType::NULL_OUTPUT;
	cfg.sampleRate = 48000;
	AudioEngine &moteur = AudioEngine::Instance();
	if (!moteur.Initialize(cfg)) {
		std::printf("echec : moteur de son\n");
		return 1;
	}

	// Les mesures de reference : le modele NONE, pour chaque distance et
	// chaque facteur de chute. Un modele NONE n'attenue pas, mais le gain de
	// chute agit peut-etre quand meme : on le mesure, on ne le suppose pas.
	float reference[2][5];
	for (int r = 0; r < 2; ++r) {
		for (int d = 0; d < 5; ++d) {
			reference[r][d] = Mesurer(moteur, AttenuationModel::NONE, kDistances[d], kRolloffs[r]);
			std::printf("reference NONE rolloff %g, %g m : %f\n", (double)kRolloffs[r],
						(double)kDistances[d], (double)reference[r][d]);
		}
	}

	// Les quarante mesures.
	{
		std::FILE *f = std::fopen("mesures.txt", "w");
		if (!f) {
			std::printf("echec : ecriture de mesures.txt\n");
			moteur.Shutdown();
			return 1;
		}
		for (int r = 0; r < 2; ++r) {
			for (int m = 0; m < 4; ++m) {
				for (int d = 0; d < 5; ++d) {
					const float brute = Mesurer(moteur, kModeles[m].modele, kDistances[d], kRolloffs[r]);
					const float ref = reference[r][d];
					const float gain = (ref > 0.f) ? (brute / ref) : 0.f;
					std::fprintf(f, "%s, rolloff %g, %g m : %.3f\n", kModeles[m].nom,
								 (double)kRolloffs[r], (double)kDistances[d], (double)gain);
					std::printf("%s, rolloff %g, %g m : %.3f\n", kModeles[m].nom, (double)kRolloffs[r],
								(double)kDistances[d], (double)gain);
				}
			}
		}
		std::fclose(f);
	}

	moteur.Shutdown();
	std::printf("fin\n");
	return 0;
}
