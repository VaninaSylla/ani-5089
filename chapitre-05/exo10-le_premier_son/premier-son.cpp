// exo10 - Le premier son
//
// Faire jouer un son au demarrage, sans le placer dans l'espace, et compter
// ce que cela coute en lignes et en temps.
//
// Fichiers rendus :
//   premier-son.cpp : ce fichier
//   demarrage.txt    : les quatre lignes affichees au demarrage
//   reponse.txt      : la reponse redigee
//
// Le son est fabrique par le moteur lui-meme (AudioGenerator::GenerateTone),
// sans aucun fichier : un sinus de 440 Hz d'une seconde. Il est joue une
// fois, sans position : VoiceParams par defaut, donc source3d.positional
// reste a faux, et le son sort au milieu des deux oreilles.
//
// Le son joue sur un autre fil : le programme s'arreterait juste apres Play
// sans rien faire entendre. Il faut donc une boucle, meme courte, pendant
// que le moteur joue.

#include <cstdio>

#include <NKAudio/NkAudio.h>
#include <NKTime/NkChrono.h>

using namespace nkentseu::audio;

int main() {
	// ─────────────────────────────────────────────────────────────────
	// 1. L'initialisation du moteur de son : AVANT la boucle.
	// ─────────────────────────────────────────────────────────────────
	AudioEngineConfig cfg;
	cfg.sampleRate = 48000;
	AudioEngine &moteur = AudioEngine::Instance();
	if (!moteur.Initialize(cfg)) {
		std::printf("echec : le moteur de son ne s'est pas initialise\n");
		return 1;
	}

	// ─────────────────────────────────────────────────────────────────
	// 2. Les quatre valeurs annoncees par le moteur, juste apres
	//    l'initialisation.
	// ─────────────────────────────────────────────────────────────────
	std::printf("dorsal : %s\n", moteur.GetBackendName());
	std::printf("frequence : %d\n", moteur.GetSampleRate());
	std::printf("tampon : %d\n", moteur.GetBufferSize());
	std::printf("latence : %f\n", (double)moteur.GetLatencyMs());
	std::fflush(stdout);

	// ─────────────────────────────────────────────────────────────────
	// 3. Le son : un sinus de 440 Hz, d'une seconde, joue une fois,
	//    sans position dans l'espace.
	// ─────────────────────────────────────────────────────────────────
	AudioSample ton = AudioGenerator::GenerateTone(440.f, 1.f);
	AudioHandle voix = moteur.Play(ton);
	(void)voix;

	// ─────────────────────────────────────────────────────────────────
	// 4. La boucle : le son joue sur un autre fil, pendant qu'elle
	//    tourne. Deux secondes, le temps que le son se fasse entendre.
	// ─────────────────────────────────────────────────────────────────
	nkentseu::NkChrono chrono;
	while (chrono.Elapsed().ToSeconds() < 2.0) {
		nkentseu::NkChrono::SleepMilliseconds(10);
	}

	// ─────────────────────────────────────────────────────────────────
	// 5. La fermeture : le moteur d'abord, l'echantillon ensuite. Une
	//    voix qui joue encore lit ses donnees : on libere donc apres
	//    Shutdown, jamais avant.
	// ─────────────────────────────────────────────────────────────────
	moteur.Shutdown();
	AudioLoader::Free(ton);

	std::printf("fin\n");
	return 0;
}
