#include <omp.h>
#include <stdio.h>
#include <cstdlib>
#include <ctime>

#include "Header/Camera.h"
#include "Header/Object.h"
#include "Header/Scene.h"
#include "Header/Sphere.h"
#include "Header/Diffuse.h"
#include "Header/Metallic.h"
#include "Header/Crystalline.h"

#include "Header/random.h" // Assuming this header contains the customRandom function
#include "Header/utils.h"

Scene randomScene() {
    int n = 500;
    Scene list;
    list.add(new Object(
        new Sphere(Vec3(0, -1000, 0), 1000),
        new Diffuse(Vec3(0.5, 0.5, 0.5))
    ));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = customRandom();
            Vec3 center(a + 0.9f * customRandom(), 0.2f, b + 0.9f * customRandom());
            if ((center - Vec3(4, 0.2f, 0)).length() > 0.9f) {
                if (choose_mat < 0.8f) {  // diffuse
                    list.add(new Object(
                        new Sphere(center, 0.2f),
                        new Diffuse(Vec3(customRandom() * customRandom(),
                            customRandom() * customRandom(),
                            customRandom() * customRandom()))
                    ));
                }
                else if (choose_mat < 0.95f) { // metal
                    list.add(new Object(
                        new Sphere(center, 0.2f),
                        new Metallic(Vec3(0.5f * (1 + customRandom()),
                            0.5f * (1 + customRandom()),
                            0.5f * (1 + customRandom())),
                            0.5f * customRandom())
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

#pragma omp parallel for schedule(dynamic)
    for (int j = 0; j < (ph - py); j++) {
        for (int i = 0; i < (pw - px); i++) {
            Vec3 col(0, 0, 0);
            for (int s = 0; s < ns; s++) {
                float u = float(i + px + customRandom()) / float(w);
                float v = float(j + py + customRandom()) / float(h);
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
    double startTime, endTime;

    int width = 256; // ancho de la imagen
    int height = 256; // alto de la imagen
    int rayNumber = 10; //brillo de la imagen

    startTime = omp_get_wtime();

    omp_set_num_threads(8);
#pragma omp parallel
    {
        int thread_num = omp_get_thread_num();
        printf("Thread %d started.\n", thread_num);

        time_t seed = time(0) + thread_num;
        srand(seed); // Semilla única para cada hilo

        int size = sizeof(unsigned char) * width * height * 3; //3 canales de color
        unsigned char* data = (unsigned char*)calloc(size, 1); //inicializamos la imagen a 0

        int patch_x_start = 0;
        int patch_x_end = width; //ancho de la imagen
        int patch_y_start = 0;
        int patch_y_end = height; //alto de la imagen

        rayTracingCPU(data, width, height, rayNumber, patch_x_start, patch_y_start, patch_x_end, patch_y_end);

        char fileName[32]; //archivo de la imagen
        sprintf(fileName, "./imgCPUImg%d.bmp", thread_num);
        writeBMP(fileName, data, width, height); //escribimos la imagen en un archivo
        free(data);

        printf("Thread %d finished creating image.\n", thread_num);
    }

    endTime = omp_get_wtime();
    printf("Tiempo total de ejecucion: %f segundos\n", endTime - startTime); //Tiempo total de ejecucion

    return 0;
}
