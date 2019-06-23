#include "displacement.h"

static Texture t;

void displacementFunction(const struct RTCDisplacementFunctionNArguments* args)
{
	const float* nx = args->Ng_x;
	const float* ny = args->Ng_y;
	const float* nz = args->Ng_z;
	float* px = args->P_x;
	float* py = args->P_y;
	float* pz = args->P_z;
	unsigned int N = args->N;

	if(!t.isLoaded())
		t = Texture("textures/brick/brick_depth2.bmp", PLANAR, false);

	int w = t.getWidth();
	int h = t.getHeight();
	for (unsigned int i = 0; i<N; i++) {

		const glm::vec3 P = glm::vec3(px[i], py[i], pz[i]);

		//printf("OLD x - %f, y - %f, z - %f\n", px[i], py[i], pz[i]);

		glm::vec2 coords = glm::vec2(px[i], py[i]);
		coords[0] *= (w);
		coords[1] *= (h);
		coords[0] = (int)coords[0] % (w);
		coords[1] = (int)coords[1] % (h);

		float height = (1.0f - t.getPixelByUV(coords.x / (float)(w), coords.y / (float)(h)).r) * 0.2f;
		const glm::vec3 Ng = glm::vec3(nx[i], ny[i], nz[i]);
		const glm::vec3 dP = glm::vec3(0.f, 0.f, height); 
		px[i] += dP.x; py[i] += dP.y; pz[i] += dP.z;

		//printf("NEW x - %f, y - %f, z - %f\n", px[i], py[i], pz[i]);

	}

} 