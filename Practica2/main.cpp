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

#include "Archivos encavezados/Camera.h"
#include "Archivos encavezados/Object.h"
#include "Archivos encavezados/Scene.h"
#include "Archivos encavezados/Sphere.h"
#include "Archivos encavezados/Diffuse.h"
#include "Archivos encavezados/Metallic.h"
#include "Archivos encavezados/Crystalline.h"

#include "Archivos encavezados/random.h"
#include "Archivos encavezados/utils.h"

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

//cd cmake-build-debug
//mpiexec -n 12 .\cmake-build-debug.exe
//mpiexec -help2


int main(int argc, char** argv) {
    int nProcesses, rank; // Se declaran las variables para almacenar el número de procesos y el rango de cada proceso
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    MPI_Status status;
    //srand(time(0));

    int w = 256;// 1200; ancho
    int h = 256;// 800; alto
    int ns = 10; //numero de rayos (fotogramas)
    int message3[3]= {w,h,ns};

    if (rank==0) { //father process
        for (int i = 0; i < nProcesses; ++i) {
            // int frame = srand(time(0));
            MPI_Send(message3, 3, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }else{
        int message3[3];
        MPI_Recv(message3, 3, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        w = message3[0];
        h = message3[1];
        ns = message3[2];
        //Empieza con un parche 1:1
        int patch_x_size = w;
        int patch_y_size = h;
        int patch_x_idx = 1;
        int patch_y_idx = 1;

        int size = sizeof(unsigned char) * patch_x_size * patch_y_size * 3;
        unsigned char* data = (unsigned char*)calloc(size, 1);

        int patch_x_start = (patch_x_idx - 1) * patch_x_size;
        int patch_x_end = patch_x_idx * patch_x_size;
        int patch_y_start = (patch_y_idx - 1) * patch_y_size;
        int patch_y_end = patch_y_idx * patch_y_size;

        rayTracingCPU(data, w, h, ns, patch_x_start, patch_y_start, patch_x_end, patch_y_end);

        writeBMP("../imgCPUImg1.bmp", data, patch_x_size, patch_y_size);
        printf("Imagen creada.\n");

        free(data);
    }
    getchar();
    return (0);
}
