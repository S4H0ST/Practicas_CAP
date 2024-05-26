//==================================================================================================
// Written in 2016 by Peter Shirley <ptrshrl@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related and
// neighboring rights to this software to the public domain worldwide. This software is distributed
// without any warranty.
//
// You should have received a copy (see file COPYING.txt) of the CC0 Public Domain Dedication along
// with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//==================================================================================================

#include <mpi.h>
#include <omp.h>

#include <float.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <limits>




#include "Header/random.h"
#include "Header/utils.h"
#include "Header/Camera.h"
#include "Header/Scene.h"
#include "Header/Sphere.h"
#include "Header/Metallic.h"
#include "Header/Diffuse.h"
#include "Header/Crystalline.h"


Scene randomScene() {
	int n = 500;
	Scene list;
	list.add(new Object(
		new Sphere(Vec3(0, -1000, 0), 1000),
		new Diffuse(Vec3(0.5, 0.5, 0.5))
	));

	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			float choose_mat = random();
			Vec3 center(a + 0.9f * random(), 0.2f, b + 0.9f * random());
			if ((center - Vec3(4, 0.2f, 0)).length() > 0.9f) {
				if (choose_mat < 0.8f) {  // diffuse
					list.add(new Object(
						new Sphere(center, 0.2f),
						new Diffuse(Vec3(random() * random(),
							random() * random(),
							random() * random()))
					));
				}
				else if (choose_mat < 0.95f) { // metal
					list.add(new Object(
						new Sphere(center, 0.2f),
						new Metallic(Vec3(0.5f * (1 + random()),
							0.5f * (1 + random()),
							0.5f * (1 + random())),
							0.5f * random())
					));
				}
				else {  // glass
					list.add(new Object(
						new Sphere(center, 0.2f),
						new Crystalline(1.5f)
					));
				}
			}
		}
	}

	list.add(new Object(
		new Sphere(Vec3(0, 1, 0), 1.0),
		new Crystalline(1.5f)
	));
	list.add(new Object(
		new Sphere(Vec3(-4, 1, 0), 1.0f),
		new Diffuse(Vec3(0.4f, 0.2f, 0.1f))
	));
	list.add(new Object(
		new Sphere(Vec3(4, 1, 0), 1.0f),
		new Metallic(Vec3(0.7f, 0.6f, 0.5f), 0.0f)
	));

	return list;
}


void rayTracingCPU(unsigned char* img, int w, int h, int ns = 10, int px = 0, int py = 0, int pw = -1, int ph = -1) {
	if (pw == -1) pw = w;
	if (ph == -1) ph = h;
	int patch_w = pw - px;
	Scene world = randomScene();
	world.setSkyColor(Vec3(0.5f, 0.7f, 1.0f));
	world.setInfColor(Vec3(1.0f, 1.0f, 1.0f));

	Vec3 lookfrom(13, 2, 3);
	Vec3 lookat(0, 0, 0);
	float dist_to_focus = 10.0;
	float aperture = 0.1f;

	Camera cam(lookfrom, lookat, Vec3(0, 1, 0), 20, float(w) / float(h), aperture, dist_to_focus);
#pragma omp parallel for
	for (int j = 0; j < (ph - py); j++) {
#pragma omp parallel for
		for (int i = 0; i < (pw - px); i++) {

			Vec3 col(0, 0, 0);
			for (int s = 0; s < ns; s++) {
				float u = float(i + px + random()) / float(w);
				float v = float(j + py + random()) / float(h);
				Ray r = cam.get_ray(u, v);
				col += world.getSceneColor(r);
			}
			col /= float(ns);
			col = Vec3(sqrt(col[0]), sqrt(col[1]), sqrt(col[2]));

			img[(j * patch_w + i) * 3 + 2] = char(255.99 * col[0]);
			img[(j * patch_w + i) * 3 + 1] = char(255.99 * col[1]);
			img[(j * patch_w + i) * 3 + 0] = char(255.99 * col[2]);
		}
	}
}

int main(int argc, char** argv) {

	int nProcesses, rank;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
	MPI_Status status;
	MPI_Request request;
	srand((unsigned int)time(0));

	int w = 256;
	int h = 256;
	int ns = 10;

	int nHijos = nProcesses - 1;
	int patchWidth = w / nHijos;
	patchWidth = ((w / nHijos) + 7) & ~7; // Redondear al múltiplo de 8 más cercano
	int patchHeight = h / nHijos;

	if (rank == 0) {
		int hijo = 1;
		for (int i = 0; i < nHijos; ++i) {
			int mensaje2[4] = { i * patchWidth, 0, (i + 1) * patchWidth, h };
			printf("Main:\tenvia a hijo%d start = (%d, %d) / end = (%d, %d)\n", hijo, mensaje2[0], mensaje2[1], mensaje2[2], mensaje2[3]);
			fflush(stdout);
			MPI_Send(mensaje2, 4, MPI_INT, hijo, 0, MPI_COMM_WORLD);
			++hijo;
		}
		int mensaje2[4] = { -1, -1, -1, -1 };
		for (int i = 1; i <= nHijos; ++i) {
			MPI_Isend(mensaje2, 4, MPI_INT, i, 0, MPI_COMM_WORLD, &request);
		}
		printf("Main:\tTodos los parches enviados a los hijos.\n");
		fflush(stdout);

		// Crear un buffer para la imagen completa
		int sizeTotal = sizeof(unsigned char) * w * h * 3;
		unsigned char* img = (unsigned char*)calloc(sizeTotal, 1);

		// Recibir las imágenes de todos los hijos
		for (int i = 1; i <= nHijos; ++i) {
			int size;
			MPI_Recv(&size, 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// Recibir los datos del proceso hijo
			unsigned char* data = (unsigned char*)calloc(size, 1);
			MPI_Recv(data, size, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			printf("Main:\tImagen recibida del hijo %d\n", i);
			fflush(stdout);
			// Combinar las imágenes de los hijos en la imagen completa
			// Copiar los datos de la imagen en el buffer de la imagen completa
			int patch_x_size = patchWidth;
			int patch_y_size = patchHeight;
			for (int y = 0; y < patch_y_size; ++y) {
				for (int x = 0; x < patch_x_size; ++x) {
					for (int c = 0; c < 3; ++c) {
						img[((y * w) + (x + (i - 1) * patch_x_size)) * 3 + c] = data[(y * patch_x_size + x) * 3 + c];
					}
				}
			}
			// Escribir la imagen completa en un archivo
			char fileName[32];
			sprintf_s(fileName, sizeof(fileName), "../imgCPUFinal.bmp");
			writeBMP(fileName, img, w, h);
			printf("Main:\tImagen final creada\n");
			fflush(stdout);

			free(data);
		}
	}
	else {
		int mensaje2[4];
		while (1) {
			MPI_Recv(mensaje2, 4, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);

			if (mensaje2[0] == -1) {
				break;
			}
			printf("Hijo %d:\trecivido start = (%d, %d) / end = (%d, %d)\n", rank, mensaje2[0], mensaje2[1], mensaje2[2], mensaje2[3]);
			fflush(stdout);

			printf("Hijo %d:\traytracing start\n", rank);
			fflush(stdout);
			int patch_x_size = mensaje2[2] - mensaje2[0];
			int patch_y_size = mensaje2[3] - mensaje2[1];
			int size = sizeof(unsigned char) * patch_x_size * patch_y_size * 3;
			unsigned char* data = (unsigned char*)calloc(size, 1);
			rayTracingCPU(data, w, h, ns, mensaje2[0], mensaje2[1], mensaje2[2], mensaje2[3]);

			// Generar un nombre de archivo único para cada proceso hijo
			char fileName[32];
			sprintf_s(fileName, sizeof(fileName), "../imgCPUImg%d.bmp", rank);
			// Escribir el parche de imagen en el archivo
			writeBMP(fileName, data, patch_x_size, patch_y_size);
			printf("Hijo %d,\tImagen%d creada\n", rank, rank);
			fflush(stdout);

			// Enviar la imagen al proceso padre
			// Enviar el tamaño de los datos al proceso padre
			MPI_Send(&size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			// Enviar los datos al proceso padre
			MPI_Send(data, size, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
			printf("Hijo %d,\tImagen enviada al padre\n", rank);
			fflush(stdout);

			free(data);
		}
	}

	MPI_Finalize();
	return (0);
}


