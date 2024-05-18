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
#include <stdio.h>
#include <math.h>

#include <float.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <string>

#include "Header/Camera.h"
#include "Header/Object.h"
#include "Header/Scene.h"
#include "Header/Sphere.h"
#include "Header/Diffuse.h"
#include "Header/Metallic.h"
#include "Header/Crystalline.h"

#include "Header/random.h"
#include "Header/utils.h"

Scene randomScene() {
    int n = 500;
    Scene list; // es
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
//mpiexec -n 12 .\Practica2.exe
//mpiexec -help2


int main(int argc, char** argv) {
    double start_time = omp_get_wtime(); //establece el tiempo de inicio
    int nProcesses, rank; // Se declaran las variables para almacenar el número de procesos y el rango de cada proceso

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    MPI_Status status; 
    MPI_Request request; 

    srand(time(0));

    int width = 256; // 1200; ancho
    int height = 256; // 800; alto
    int rayNumber = 10; // numero de rayos

    fflush(stdout);
    int currentFrame, random, mensaje;    
    int info[2]; //frame actual y numero frame (probar que funciona igualmente sin pasar w h y s)

    //maestro
    if (rank == 0) {
        //if (argc > 1) {
        //    totalFrames = std::stoi(argv[1]); // Convierte el argumento a entero
        //}
        printf("Entramos en el padre: \n");
        for (int i = 1; i <= nProcesses - 1; i++) {
            
            srand(i);
            currentFrame = i;//Mirar como arreglar para que se envíen todos los frames existentes desde el 0

            info[0] = currentFrame; //enviamos el numero de proceso
            info[1] = rand(); //enviamos 
            printf("Enviamos currentFrame: %d y un numero random: %d al hijo %d\n", info[0], info[1], i);

			fflush(stdout); //comprobar que esto no borre los debugs
            
            MPI_Isend(info, 2, MPI_INT, i, 0, MPI_COMM_WORLD, &request); //Aquí solo enviaría el currentFrame y el random lo haría en el hijo en vez de en el padre
            printf("Enviado mensaje al proceso %d\n", i);
        }
       
        printf("Se han enviado todos los frames, esperando a ser resueltos\n");
        
        fflush(stdout);

        
        info[0] = -1;
        
        for (int i = 1; i < nProcesses - 1; i++) {
            MPI_Send(info, 2, MPI_INT, i, 0, MPI_COMM_WORLD);
            //COMPROBAR SI INFO SE ENVIA BIEN SIN "&"
        }
        
        for (int i = 1; i < nProcesses - 1; i++) {
            MPI_Recv(&mensaje, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE); //Esperamos a que cada proceso haya terminado para calcular el tiempo final
            //COMPROBAR SI INFO SE ENVIA BIEN CON "&"
            printf("Recibido mensaje de finalizado del hijo: %d\n", mensaje); 
            fflush(stdout);
        }
       
        double end_time = omp_get_wtime(); //establece el tiempo de finalización
        if (rank == 0) {

			printf("Tiempo de ejecucion total del programa: %f segundos.\n", end_time - start_time);

        }
        
        MPI_Finalize();
    }
    else {
        //hijo
        while (1) {

			MPI_Recv(info, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &status); 
            
            if (info[0] == -1) {
                mensaje = rank;//Enviamos al padre, cuál es el hijo que ha terminado
                MPI_Send(&mensaje, 1, MPI_INT, 0, 0, MPI_COMM_WORLD); //Enviamos al padre que el hijo ya ha terminado su misión
				printf("Hijo %d,\tFinaliza\n", rank); //imprimimos que el hijo ha terminado
                break; //terminamos el hijo
            }
            
            currentFrame = info[0];
            printf("Recibido frame numero: %d en el hijo: %d\n", currentFrame, rank);
			random = info[1]; //Recibimos y guardamos el random
            printf("Recibido random: %d en el hijo: %d\n", random, rank);
            
            fflush(stdout);

            //Empieza con un parche 1:1 (Mirar si se puede simplificar)
            
			int patch_x_idx = 1; //se refiere a la posición del parche en x
			int patch_y_idx = 1; //se refiere a la posición del parche en y
            srand(random); //sirve para que cada hijo use el mismo random como semilla para cuando usen rand entiendo
            int size = sizeof(unsigned char) * width * height * 3;
            unsigned char* data = (unsigned char*)calloc(size, 1);


            int patch_x_start = (patch_x_idx - 1) * width;
            int patch_x_end = patch_x_idx * width;
            int patch_y_start = (patch_y_idx - 1) * height;
            int patch_y_end = patch_y_idx * height;

            printf("Hijo %d:\tcalculando frame %d\n", rank, currentFrame);
            fflush(stdout);
            rayTracingCPU(data, width, height, rayNumber, patch_x_start, patch_y_start, patch_x_end, patch_y_end);

            char fileName[32];
            sprintf(fileName, "../imgCPUImg%d.bmp", currentFrame);
            writeBMP(fileName, data, width, height);
            printf("Hijo %d,\tImagen%d creada\n", rank, currentFrame);
            fflush(stdout);
            free(data);

        }
        MPI_Finalize();
    }


    

    
    return (0);
}
