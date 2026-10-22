// exo11 - mesure.cpp
//
// Ce programme n'a ni fenetre ni sortie sonore : il rend le melange du
// moteur dans un tampon, et on mesure ce qu'il met dans chaque oreille.
//
// L'auditeur est a l'origine et regarde vers -Z : avant (0, 0, -1), haut
// (0, 1, 0). La pluie est placee a deux metres : a gauche en (-2, 0, 0),
// devant en (0, 0, -2), puis a droite en (2, 0, 0). Chaque position est
// mesuree avec positional vrai, puis faux.
//
// Pour chaque mesure on rend 48 000 trames par blocs de 256 avec
// RenderToBuffer. Les echantillons y sont ranges gauche, droite, gauche,
// droite. On calcule la racine de la moyenne des carres, pour l'oreille
// gauche puis pour l'oreille droite.
//
// Sortie : mesures.txt, six lignes, "devant 1 : G x D y".

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include <NKAudio/NkAudio.h>

using namespace nkentseu;
using namespace nkentseu::audio;

static const int32 kEchantillons = 48000; // 48 000 trames
static const int32 kBloc = 256;          // par blocs de 256
static const int32 kCanaux = 2;

struct Position {
	const char *nom;
	float pos[3];
};
static const Position kPositions[3] = {
	{"gauche", {-2.f, 0.f, 0.f}},
	{"devant", {0.f, 0.f, -2.f}},
	{"droite", {2.f, 0.f, 0.f}},
};

// ───────────────────────────────────────────────────────────────────────────
// Une mesure : on rend 48 000 trames et on renvoie les deux racines de la
// moyenne des carres, oreille gauche puis oreille droite.
// ───────────────────────────────────────────────────────────────────────────
static void Mesurer(AudioEngine &moteur, const Position &pos, bool positional,
					float &gauche, float &droite) {
	// La source ne bouge pas pendant la mesure : on l'arrete avant de la
	// replacer.
	moteur.StopAll();

	VoiceParams p;
	p.looping = true;
	p.source3d.positional = positional;
	p.source3d.position[0] = pos.pos[0];
	p.source3d.position[1] = pos.pos[1];
	p.source3d.position[2] = pos.pos[2];
	p.source3d.minDistance = 1.0f;
	p.source3d.maxDistance = 20.0f;
	p.source3d.rolloffFactor = 1.0f;
	p.source3d.attenuationModel = AttenuationModel::INVERSE;
	p.source3d.useHrtf = true;

	AudioSample pluie = AudioGenerator::GenerateNoise(4.f, WaveformType::NOISE_PINK);
	AudioHandle voix = moteur.Play(pluie, p);

	// L'auditeur est a l'origine et regarde vers -Z.
	moteur.SetListenerPosition(0.f, 0.f, 0.f);
	moteur.SetListenerOrientation(0.f, 0.f, -1.f, 0.f, 1.f, 0.f);

	double sommeGauche = 0.0, sommeDroite = 0.0;
	int64 echantillonsGauche = 0, echantillonsDroite = 0;

	float tampon[256 * 2];
	int32 rendues = 0;
	while (rendues < kEchantillons) {
		const int32 frames = (kEchantillons - rendues) < kBloc ? (kEchantillons - rendues) : kBloc;
		moteur.RenderToBuffer(tampon, frames, kCanaux);
		for (int32 i = 0; i < frames; ++i) {
			const float g = tampon[i * 2 + 0];
			const float d = tampon[i * 2 + 1];
			sommeGauche += (double)g * (double)g;
			sommeDroite += (double)d * (double)d;
			++echantillonsGauche;
			++echantillonsDroite;
		}
		rendues += frames;
	}

	gauche = (float)std::sqrt(sommeGauche / (double)echantillonsGauche);
	droite = (float)std::sqrt(sommeDroite / (double)echantillonsDroite);

	moteur.Stop(voix);
	AudioLoader::Free(pluie);
}

int main() {
	// ── Le moteur de son, sans sortie ─────────────────────────────────────
	AudioEngineConfig cfg;
	cfg.backend = AudioBackendType::NULL_OUTPUT;
	cfg.sampleRate = 48000;
	cfg.enableHrtf = true; // le meme rendu binaural que dans la salle
	AudioEngine &moteur = AudioEngine::Instance();
	if (!moteur.Initialize(cfg)) {
		std::printf("echec : moteur de son\n");
		return 1;
	}
	// Meme jeu binaural que dans la salle, genere par le moteur : on mesure
	// ainsi exactement ce qu'on entend au casque.
	if (!moteur.GenerateSyntheticHrtf()) {
		std::printf("echec : jeu HRTF\n");
		return 1;
	}

	if (std::getenv("POSITIONS") != nullptr) {
		// On mesure ce que l'oreille recoit a chacune des trois positions
		// d'ecoute de la salle : la source est la pluie, derriere la fenetre,
		// et l'auditeur est place (et oriente) comme la camera.
		struct Ecoute {
			const char *nom;
			float pos[3];
			float cible[3];
		};
		const Ecoute positions[3] = {
			{"pres de la fenetre", {-1.55f, 1.60f, -0.90f}, {-2.00f, 1.40f, -0.90f}},
			{"au centre", {0.00f, 1.60f, 0.00f}, {-1.98f, 1.40f, -0.90f}},
			{"coin oppose", {1.60f, 1.60f, 1.60f}, {-1.98f, 1.40f, -0.90f}},
		};
		for (int sans = 0; sans < 2; ++sans) {
			for (int i = 0; i < 3; ++i) {
				moteur.StopAll();
				VoiceParams p;
				p.looping = true;
				p.source3d.positional = (sans == 0);
				p.source3d.position[0] = -1.98f;
				if (const char *sx = std::getenv("SALLE_SOURCE_X")) {
					p.source3d.position[0] = (float)std::atof(sx);
				}
				p.source3d.position[1] = 1.40f;
				p.source3d.position[2] = -0.90f;
				p.source3d.minDistance = 1.0f;
				p.source3d.maxDistance = 20.0f;
				p.source3d.rolloffFactor = 1.0f;
				p.source3d.attenuationModel = AttenuationModel::INVERSE;
				p.source3d.useHrtf = true;
				AudioSample pluie = AudioGenerator::GenerateNoise(4.f, WaveformType::NOISE_PINK);
				AudioHandle voix = moteur.Play(pluie, p);

				const Ecoute &e = positions[i];
				const float fx = e.cible[0] - e.pos[0];
				const float fy = e.cible[1] - e.pos[1];
				const float fz = e.cible[2] - e.pos[2];
				const float inv = 1.f / std::sqrt(fx * fx + fy * fy + fz * fz);
				moteur.SetListenerPosition(e.pos[0], e.pos[1], e.pos[2]);
				moteur.SetListenerOrientation(fx * inv, fy * inv, fz * inv, 0.f, 1.f, 0.f);

				double sg = 0.0, sd = 0.0;
				float tampon[256 * 2];
				for (int bloc = 0; bloc < 96000 / 256; ++bloc) { // 2 secondes
					moteur.RenderToBuffer(tampon, 256, 2);
					for (int k = 0; k < 256; ++k) {
						sg += (double)tampon[k * 2] * (double)tampon[k * 2];
						sd += (double)tampon[k * 2 + 1] * (double)tampon[k * 2 + 1];
					}
				}
				const double distance = std::sqrt(
					(e.pos[0] + 1.98) * (e.pos[0] + 1.98) + (e.pos[1] - 1.40) * (e.pos[1] - 1.40) +
					(e.pos[2] + 0.90) * (e.pos[2] + 0.90));
				std::printf("%-18s %s  d=%.2f m  gauche %.6f  droite %.6f  total %.6f\n", e.nom,
							sans ? "SANS positional" : "AVEC positional  ", distance,
							std::sqrt(sg / 96000.0), std::sqrt(sd / 96000.0),
							std::sqrt((sg + sd) / 192000.0));
				std::fflush(stdout);
				moteur.Stop(voix);
				AudioLoader::Free(pluie);
			}
		}
		moteur.Shutdown();
		return 0;
	}

	if (std::getenv("PROGRESSION") != nullptr) {
		// On joue la pluie comme dans la salle, et on suit le niveau.
		VoiceParams p;
		p.looping = true;
		p.source3d.positional = true;
		p.source3d.position[0] = -1.98f;
		p.source3d.position[1] = 1.40f;
		p.source3d.position[2] = -0.90f;
		p.source3d.minDistance = 1.0f;
		p.source3d.maxDistance = 20.0f;
		p.source3d.rolloffFactor = 1.0f;
		p.source3d.attenuationModel = AttenuationModel::INVERSE;
		p.source3d.useHrtf = true;
		AudioSample pluie = AudioGenerator::GenerateNoise(4.f, WaveformType::NOISE_PINK);
		AudioHandle voix = moteur.Play(pluie, p);
		moteur.SetListenerPosition(0.f, 0.f, 0.f);
		moteur.SetListenerOrientation(0.f, 0.f, -1.f, 0.f, 1.f, 0.f);
		std::printf("seconde : rms total, rms gauche, rms droite\n");
		float tampon[256 * 2];
		for (int seconde = 0; seconde < 30; ++seconde) {
			double sg = 0.0, sd = 0.0;
			for (int bloc = 0; bloc < 48000 / 256; ++bloc) {
				moteur.RenderToBuffer(tampon, 256, 2);
				for (int i = 0; i < 256; ++i) {
					sg += (double)tampon[i * 2] * (double)tampon[i * 2];
					sd += (double)tampon[i * 2 + 1] * (double)tampon[i * 2 + 1];
				}
			}
			std::printf("%2d : %.6f  %.6f  %.6f\n", seconde, std::sqrt(sg / 48000.0),
						std::sqrt(sg / 48000.0), std::sqrt(sd / 48000.0));
			std::fflush(stdout);
		}
		moteur.Stop(voix);
		AudioLoader::Free(pluie);
		moteur.Shutdown();
		return 0;
	}

	float gauche = 0.f, droite = 0.f;
	{
		std::FILE *f = std::fopen("mesures.txt", "w");
		if (!f) {
			std::printf("echec : ecriture de mesures.txt\n");
			moteur.Shutdown();
			return 1;
		}
		for (int i = 0; i < 3; ++i) {
			for (int k = 0; k < 2; ++k) {
				const bool positional = (k == 0);
				Mesurer(moteur, kPositions[i], positional, gauche, droite);
				std::fprintf(f, "%s %d : %f %f\n", kPositions[i].nom, positional ? 1 : 0,
							 (double)gauche, (double)droite);
				std::printf("%s %d : %f %f\n", kPositions[i].nom, positional ? 1 : 0,
							(double)gauche, (double)droite);
			}
		}
		std::fclose(f);
	}

	moteur.Shutdown();
	std::printf("fin\n");
	return 0;
}
