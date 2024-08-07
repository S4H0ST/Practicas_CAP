﻿#include <mpi.h>
#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <omp.h>

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

#pragma omp parallel for collapse(2)
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
//Comando para ejecutar el programa
//cd cmake-build-debug
// cmake -DCMAKE_BUILD_TYPE=Release ..
// MSBuild Practica2.sln /p:Configuration=Release
//mpiexec -n 4 .\main.exe
//mpiexec -help2
int main(int argc, char** argv) {
    int nProcesses, rank;
    double startTime, endTime; //tiempo de inicio y fin de ejecucion

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);//Proceso actual
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);//numero de procesos
    MPI_Status status;
    MPI_Request request;

    srand(time(0));

    int width = 256; // ancho de la imagen
    int height = 256; // alto de la imagen
    int rayNumber = 10; //brillo de la imagen

    fflush(stdout);
    int currentFrame, random, mensaje;
    int info[2]; //es el frame y el numero random del frame

    // Master process
    if (rank == 0) {
        startTime = MPI_Wtime();

        for (int i = 1; i < nProcesses; i++) { //Enviamos los frames a los hijos
            srand(i);
            currentFrame = i;

            info[0] = currentFrame;
            info[1] = rand();
            printf("Enviamos currentFrame: %d y un numero random: %d al hijo %d\n", info[0], info[1], i);

            fflush(stdout);

            MPI_Isend(info, 2, MPI_INT, i, 0, MPI_COMM_WORLD, &request); //Enviamos el frame y el numero random al hijo
            printf("Enviado mensaje al proceso %d\n", i);
        }

        printf("Se han enviado todos los frames, esperando a ser resueltos\n");

        fflush(stdout);

        // Esperamos a que los hijos terminen
        for (int i = 1; i < nProcesses; i++) { //Recibimos los mensajes de los hijos
            MPI_Recv(&mensaje, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("Recibido mensaje de finalizado del hijo: %d\n", mensaje);
            fflush(stdout);
        }

        // Enviamos mensaje de finalizacion a los hijos
        for (int i = 1; i < nProcesses; i++) {
            info[0] = -1; //Enviamos -1 para que los hijos sepan que deben finalizar
            MPI_Send(info, 2, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        endTime = MPI_Wtime();
        printf("Padre, Tiempo total de ejecucion: %f segundos\n", endTime - startTime); //Tiempo total de ejecucion

        MPI_Finalize();
    }
    else {
        // Proceso hijo
        while (1) {
            MPI_Recv(info, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, &status); //Recibimos el frame y el numero random del frame

            if (info[0] == -1) { //Si el frame es -1, finalizamos
                mensaje = rank;
                MPI_Send(&mensaje, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                printf("Hijo %d,\tFinaliza\n", rank);
                break;
            }

            currentFrame = info[0];
            printf("Recibido frame numero: %d en el hijo: %d\n", currentFrame, rank);
            random = info[1];
            printf("Recibido random: %d en el hijo: %d\n", random, rank);

            fflush(stdout);

            srand(random);
            int size = sizeof(unsigned char) * width * height * 3; //3 canales de color
            unsigned char* data = (unsigned char*)calloc(size, 1); //inicializamos la imagen a 0

            int patch_x_start = 0;
            int patch_x_end = width; //ancho de la imagen
            int patch_y_start = 0;
            int patch_y_end = height; //alto de la imagen

            printf("Hijo %d:\tcalculando frame %d\n", rank, currentFrame);
            fflush(stdout);

            startTime = MPI_Wtime(); //iniciamos el tiempo de ejecucion del hijo
            rayTracingCPU(data, width, height, rayNumber, patch_x_start, patch_y_start, patch_x_end, patch_y_end);
            endTime = MPI_Wtime(); //finalizamos el tiempo de ejecucion del hijo

            printf("Hijo %d,\tTiempo de ejecucion: %f segundos\n", rank, endTime - startTime);

            char fileName[32]; //archivo de la imagen
            sprintf(fileName, "../imgCPUImg%d.bmp", currentFrame);
            writeBMP(fileName, data, width, height); //escribimos la imagen en un archivo
            printf("Hijo %d,\tImagen%d creada\n", rank, currentFrame);
            fflush(stdout);
            free(data);

            mensaje = rank;
            MPI_Send(&mensaje, 1, MPI_INT, 0, 0, MPI_COMM_WORLD); //Enviamos mensaje de finalizado al padre
        }
        MPI_Finalize();
    }
    return 0;
}
