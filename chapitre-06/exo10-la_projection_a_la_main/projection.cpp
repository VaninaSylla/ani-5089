#include "NKWindow/NKMain.h"
#include "NKMath/NkMat.h"
#include "NKRenderer\Core\NkCamera.h"
#include <cmath>

using namespace nkentseu;
using namespace nkentseu::math;

NkMat4f Projection(float32 gauche, float32 droite, float32 haut, float32 bas, float32 nearPlane,
				   float32 farPlane, bool depthZeroToOne = false)
{
	NkMat4f result;
	float32 tanLeft = std::tan(gauche);
	float32 tanRight = std::tan(droite);
	float32 tanUp = std::tan(haut);
	float32 tanDown = std::tan(bas);
	result.mat[0][0] = 2.0f / (tanRight - tanLeft);
	result.mat[1][1] = 2.0f / (tanUp - tanDown);
	result.mat[2][0] = (tanRight + tanLeft) / (tanRight - tanLeft);
	result.mat[2][1] = (tanUp + tanDown) / (tanUp - tanDown);
	result.mat[2][3] = -1;
	if (depthZeroToOne) {
		result.mat[2][2] = farPlane / (nearPlane - farPlane);
		result.mat[3][2] = farPlane * nearPlane / (nearPlane - farPlane);
	}
	else {
		result.mat[2][2] = (farPlane + nearPlane) / (nearPlane - farPlane);
		result.mat[3][2] = 2.0f * farPlane * nearPlane / (nearPlane - farPlane);
	}
	return result;
}

void ImprimerLignes(const char *nom, const NkMat4f &ecart)
{
	for (int32 ligne = 0; ligne < 4; ++ligne) {
		printf("%s, ligne %d : %.9f %.9f %.9f %.9f\n", nom, ligne, ecart.mat[0][ligne], ecart.mat[1][ligne],
			   ecart.mat[2][ligne], ecart.mat[3][ligne]);
	}
}

float32 PlusGrandEcart(const NkMat4f &ecart)
{
	float32 maxEcart = 0;
	for (int32 col = 0; col < 4; ++col) {
		for (int32 ligne = 0; ligne < 4; ++ligne) {
			float32 e = std::fabs(ecart.mat[col][ligne]);
			if (e > maxEcart) { maxEcart = e; }
		}
	}
	return maxEcart;
}

int32 nkmain(const NkEntryState &state)
{
	const float32 fovY = 70.0f;
	const float32 aspect = 1280.0f / 720.0f;
	const float32 nearPlane = 0.05f;
	const float32 farPlane = 100.0f;

	float32 tanDemiHaut = std::tan(NkAngle(fovY * 0.5f).Rad());
	float32 demiLargeur = aspect * tanDemiHaut;
	float32 angleDemiLargeur = std::atan(demiLargeur);

	NkMat4f symetriqueMien = Projection(-angleDemiLargeur, angleDemiLargeur, NkAngle(fovY * 0.5f).Rad(),
										-NkAngle(fovY * 0.5f).Rad(), nearPlane, farPlane);
	NkMat4f symetriqueMoteur =
		NkMat4f::Perspective(NkAngle(fovY), aspect, nearPlane, farPlane);

	NkMat4f ecartSym;
	for (int32 col = 0; col < 4; ++col) {
		for (int32 ligne = 0; ligne < 4; ++ligne) {
			ecartSym.mat[col][ligne] = symetriqueMien.mat[col][ligne] - symetriqueMoteur.mat[col][ligne];
		}
	}
	ImprimerLignes("symetrique", ecartSym);
	printf("symetrique : plus grand ecart %.9f\n", PlusGrandEcart(ecartSym));

	float32 questGauche = NkAngle(-52.0f).Rad();
	float32 questDroite = NkAngle(45.0f).Rad();
	float32 questHaut = NkAngle(48.0f).Rad();
	float32 questBas = NkAngle(-50.0f).Rad();

	NkMat4f questMien = Projection(questGauche, questDroite, questHaut, questBas, nearPlane, farPlane);

	renderer::NkCamera3D cam;
	cam.SetNearFar(nearPlane, farPlane);
	cam.SetFovAsym(questGauche, questDroite, questHaut, questBas);
	const NkMat4f &questMoteur = cam.GetProj();

	NkMat4f ecartQuest;
	for (int32 col = 0; col < 4; ++col) {
		for (int32 ligne = 0; ligne < 4; ++ligne) {
			ecartQuest.mat[col][ligne] = questMien.mat[col][ligne] - questMoteur.mat[col][ligne];
		}
	}
	ImprimerLignes("quest", ecartQuest);
	printf("quest : plus grand ecart %.9f\n", PlusGrandEcart(ecartQuest));

	return 0;
}
