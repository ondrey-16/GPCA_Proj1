//
// Created by ondrey on 18.11.2025.
//

#include <thrust/sort.h>
#include <thrust/device_ptr.h>
#include <chrono>

#include "../model/Fish.h"
#include "kernel.h"

__constant__ float maxVelocity;
__constant__ float maxForce;
__constant__ float alignmentWeightRed;
__constant__ float separationWeightRed;
__constant__ float cohesionWeightRed;
__constant__ float alignmentWeightGreen;
__constant__ float separationWeightGreen;
__constant__ float cohesionWeightGreen;
__constant__ float CX;
__constant__ float CY;

// Oblicza długość wektora
__device__ float magnitude(const float x, const float y) {
    return sqrtf(x * x + y * y);
}

// Skaluje wektor, aby miał wskazaną długość
__device__ void setMagnitude(float* x, float* y, const float mag) {
    if (magnitude(*x, *y) > 0) {
        const float scale = mag / magnitude(*x, *y);
        *x *= scale;
        *y *= scale;
    }
}

// Dopasowuje długość wektora, aby nie przekraczała maksymalnej długości
__device__ void limit(float* x, float* y, const float max) {
    float size = magnitude(*x, *y);
    if (size > max) {
        setMagnitude(x, y, max);
    }
}

// Oblicza odległość między punktami/wektorami
__device__ float distance(const float x1, const float y1, const float x2, const float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return sqrtf(dx*dx + dy*dy);
}

// Dodaje dwa wektory do siebie
__device__ cudaVector add(const float x1, const float y1, const float x2, const float y2) {
    return cudaVector(x1 + x2, y1 + y2);
}

// Odejmuje od siebie dwa wektory
__device__ cudaVector sub(const float x1, const float y1, const float x2, const float y2) {
    return cudaVector(x1 - x2, y1 - y2);
}

// Mnoży dwa wektory przez siebie
__device__ cudaVector mul(const float x1, const float y1, const float x2, const float y2) {
    return cudaVector(x1 * x2, y1 * y2);
}

// Dzieli dwa wektory przez siebie
__device__ cudaVector div(const float x1, const float y1, const float x2, const float y2) {
    return cudaVector(x1 / x2, y1 / y2);
}

// Dokonuje modyfikacji siły sterującej rybą na podstawie zachowania alignment/separation
__device__ void normalizeSteerVector(cudaVector* steer, float* vx, float* vy, int fishIdx, int count) {
    if (count > 0) {
        steer->x /= static_cast<float>(count);
        steer->y /= static_cast<float>(count);
        setMagnitude(&steer->x, &steer->y, maxVelocity);
        *steer = sub(steer->x, steer->y, vx[fishIdx], vy[fishIdx]);
        limit(&steer->x, &steer->y, maxForce);
    }
    else {
        steer->x = 0.f;
        steer->y = 0.f;
    }
}

// Dokonuje modyfikacji siły sterującej rybą na podstawie zachowania cohesion
__device__ void normalizeCohesionSteerVector(cudaVector* steer, float* x, float* y, float* vx, float* vy, int fishIdx, int count) {
    if (count > 0) {
        steer->x /= static_cast<float>(count);
        steer->y /= static_cast<float>(count);
        *steer = sub(steer->x, steer->y, x[fishIdx], y[fishIdx]);
        setMagnitude(&steer->x, &steer->y, maxVelocity);
        *steer = sub(steer->x, steer->y, vx[fishIdx], vy[fishIdx]);
        limit(&steer->x, &steer->y, maxForce);
    }
    else {
        steer->x = 0.0f;
        steer->y = 0.0f;
    }
}

// Oblicza siłę sterującą rybą w celu ominięcia napotykanej przeszkody
__device__ cudaVector avoidObstacle(float* x, float* y, float* vx, float* vy, const int fishIdx) {
    float vMag = magnitude(vx[fishIdx], vy[fishIdx]);
    if (vMag < 1e-6) {
        return cudaVector(0, 0);
    }
    cudaVector rayDirection(vx[fishIdx] / vMag, vy[fishIdx] / vMag);
    cudaVector rayStart(x[fishIdx], y[fishIdx]);
    if (intersection(rayStart, rayDirection) == 1) {
        cudaVector fromObstacle(rayStart.x - CX, rayStart.y - CY);
        float fromObstacleMag = magnitude(fromObstacle.x, fromObstacle.y);
        cudaVector fromObstacleDirection(fromObstacle.x / fromObstacleMag, fromObstacle.y / fromObstacleMag);
        cudaVector avoidForce(fromObstacleDirection.x * vMag, fromObstacleDirection.y * vMag);
        limit(&avoidForce.x, &avoidForce.y, maxForce);
        return avoidForce;
    }
    return cudaVector(0, 0);
}

// Funkcja pomocnicza wykorzystywana w avoidObstacle w celu określenia, czy ruch ryby przecina obszar dookoła przeszkody
__device__ int intersection(cudaVector rayStart, cudaVector rayDirection) {
    cudaVector toObstacle(CX - rayStart.x, CY - rayStart.y);
    float proj = rayDirection.x * toObstacle.x + rayDirection.y * toObstacle.y;

    if (proj > 150.f) {
        return 0;
    }

    if (proj < 0) {
        return 0;
    }

    float dist = magnitude(toObstacle.x - rayDirection.x * proj, toObstacle.y - rayDirection.y * proj);

    if (dist <= 50.0) {
        return 1;
    }
    return 0;
}

// Zmodyfikowany odpowiednik funkcji Flock na wersji CPU. Oblicza wypadkową siłę mającą działać w danej chwili na daną
// rybę, lecz wszystkie obliczenia są w jednej pętli. Do tego funkcje te wywoływane są osobno dla każdej z
// rozpatrywanych komórek reprezentujących sąsiedztwo danej ryby. Dla każdego zachowania separation/alignment/cohesion
// siły pochodzące od interakcją z każdą z ryb z sąsiedztwa są sumowane w jedne wypadkowe, które są potem odpowiednio
// normalizowane, aby końcowy efekt był równoważny z wersją CPU
__device__ void flock(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay,
    const int fishIdx, const int fx, const int fy, const int* cellsStartIndexes, const int* cellsEndIndexes) {
    int gridIndex;

    cudaVector sep(0, 0);
    cudaVector ali(0, 0);
    cudaVector coh(0, 0);
    int countSep = 0, countAli = 0, countCoh = 0;
    float aliCohNeighborDist = 50.0f;
    float sepNeighborDist = 25.0f;
    for (int j = fx > 0 ? fx - 1 : 0; j < 41 && j <= fx + 1; j++) {
        for (int k = fy > 0 ? fy - 1 : 0; k < 25 && k <= fy + 1; k++) {
            gridIndex = k * 41 + j;
            int start = cellsStartIndexes[gridIndex];
            int end = cellsEndIndexes[gridIndex];
            if (start == -1 || end == -1) {
                continue;
            }
            cudaVector partSep(0, 0);
            cudaVector partAli(0, 0);
            cudaVector partCoh(0, 0);
            for (int i = start; i < end; i++) {
                if (i == fishIdx) {
                    continue;
                }
                float dist = distance(x[fishIdx], y[fishIdx], x[i], y[i]);
                if (dist > 0 && dist < aliCohNeighborDist && ifRed[fishIdx] == ifRed[i]) {
                    partAli = add(partAli.x, partAli.y, vx[i], vy[i]);
                    partCoh = add(partCoh.x, partCoh.y, x[i], y[i]);
                    countAli++;
                    countCoh++;
                    if (dist < sepNeighborDist && ifRed[fishIdx] == ifRed[i]) {
                        cudaVector diff = sub(x[fishIdx], y[fishIdx], x[i], y[i]);
                        diff.x /= dist;
                        diff.y /= dist;
                        partSep = add(partSep.x, partSep.y, diff.x, diff.y);
                        countSep++;
                    }
                }
            }
            sep = add(sep.x, sep.y, partSep.x, partSep.y);
            ali = add(ali.x, ali.y, partAli.x, partAli.y);
            coh = add(coh.x, coh.y, partCoh.x, partCoh.y);
        }
    }
    normalizeSteerVector(&sep, vx, vy, fishIdx, countSep);
    normalizeSteerVector(&ali, vx, vy, fishIdx, countAli);
    normalizeCohesionSteerVector(&coh, x, y, vx, vy, fishIdx, countCoh);

    cudaVector avoid = avoidObstacle(x, y, vx, vy, fishIdx);

    float aw = (ifRed[fishIdx] == 1) ? alignmentWeightRed : alignmentWeightGreen;
    float sw = (ifRed[fishIdx] == 1) ? separationWeightRed : separationWeightGreen;
    float cw = (ifRed[fishIdx] == 1) ? cohesionWeightRed : cohesionWeightGreen;

    ax[fishIdx] = ali.x * aw + coh.x * cw + sep.x * sw + 2 * avoid.x;
    ay[fishIdx] = ali.y * aw + coh.y * cw + sep.y * sw + 2 * avoid.y;
}

// Modyfikuje parametry ruchu ryby
__device__ void update(float* x, float* y, float* vx, float* vy, float* ax, float* ay, const int fishIdx) {
    const cudaVector velocity = add(vx[fishIdx], vy[fishIdx], ax[fishIdx], ay[fishIdx]);
    vx[fishIdx] = velocity.x;
    vy[fishIdx] = velocity.y;
    limit(&vx[fishIdx], &vy[fishIdx], maxVelocity);
    const cudaVector location = add(vx[fishIdx], vy[fishIdx], x[fishIdx], y[fishIdx]);
    x[fishIdx] = location.x;
    y[fishIdx] = location.y;
    ax[fishIdx] = 0.0f;
    ay[fishIdx] = 0.0f;
    borders(x, y, fishIdx);
}

// Modyfikuje położenie ryby, aby mieściła się w zakresie okna
__device__ void borders(float* x, float* y, const int fishIdx) {
    if (x[fishIdx] > 2000) {
        x[fishIdx] = 0;
    }
    else if (x[fishIdx] <= 0) {
        x[fishIdx]= 2000;
    }
    if (y[fishIdx] > 1200) {
        y[fishIdx]= 0;
    }
    else if (y[fishIdx] <= 0) {
        y[fishIdx] = 1200;
    }
}

// Kernel wyznaczający dla każdej ryby numer indeksu komórki, w której w danej chwili się znajduje
__global__ void splitFishesOnGridKernel(float* x, float* y, int N, int* gridIndexes) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    int idxX = (int)(x[i] / 50.f);
    int idxY = (int)(y[i] / 50.f);

    idxX = max(0, min(40, idxX));
    idxY = max(0, min(24, idxY));

    gridIndexes[i] = 41 * idxY + idxX;
}

// Kernel resetujący wartości w tablicach reprezentujących początkowe i końcowe indeksy posortowanych ryb dla danej
// komórki
__global__ void resetCellsStartEndIndexesKernel(int* cellsStartIndexes, int* cellsEndIndexes, int cellsN) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= cellsN) return;
    cellsStartIndexes[i] = INT_MAX;
    cellsEndIndexes[i] = -1;
}

// Kernel inicjalizujący wstępnie tablicę reprezentującej kolejność indeksów ryb posortowanych względem położeń w
// komórkach
__global__ void initializeFishesIndexesKernel(int* fishesIndexes, int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    fishesIndexes[i] = i;
}

// Kernel wypełniający na podstawie tablicy indeksów ryb o zmodyfikowanej kolejności w odpowiedniej kolejności dane w
// pomocniczych tablicach reprezentujących parametry ruchu ryb
__global__ void initializeSortedParamsKernel(int* ifRed, float* x, float* y, float* vx, float* vy, float* nx, float* ny, float* nvx, float* nvy,  int* nifRed, int* fishesIndexes, int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    nx[i] = x[fishesIndexes[i]];
    ny[i] = y[fishesIndexes[i]];
    nvx[i] = vx[fishesIndexes[i]];
    nvy[i] = vy[fishesIndexes[i]];
    nifRed[i] = ifRed[fishesIndexes[i]];
}

// Kernel przenoszący dane z tablic pomocniczych do głównych tablic parametrów ruchu w celu zwrócenia tablic
// posortowanych względem indeksów komórek, w których ryby się znajdują
__global__ void sortParamsKernel(int* ifRed, float* x, float* y, float* vx, float* vy, float* nx, float* ny, float* nvx, float* nvy, int* nifRed, int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    x[i] = nx[i];
    y[i] = ny[i];
    vx[i] = nvx[i];
    vy[i] = nvy[i];
    ifRed[i] = nifRed[i];
}

// Kernel wyznaczający początkowy i końcowy indeks zakresów ryb, które znajdują się wewnątrz danej komórki, posortowane
// względem indeksów tych komórek. Zapisuje je w utworzonych w tym celu tablicach przy pomocy atomowych operacji
// gwarantujących bezpieczeństwo synchronizacji danych
__global__ void findCellsStartEndIndexesKernel(int* cellsStartIndexes, int* cellsEndIndexes, int* gridIndexes, int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;

    int gridIndex = gridIndexes[i];
    atomicMin(&cellsStartIndexes[gridIndex], i);
    atomicMax(&cellsEndIndexes[gridIndex], i + 1);
}

// Kernel obliczający siły, jakie mają działać na ryby, w celu ich odpowiedniego przesunięcia
__global__ void computeForceKernel(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N,
    const int* gridIndexes, const int* cellsStartIndexes, const int* cellsEndIndexes) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;

    int gridIndex = gridIndexes[i];
    const int fy = gridIndex / 41;
    const int fx = gridIndex % 41;

    flock(ifRed, x, y, vx, vy, ax, ay, i, fx, fy, cellsStartIndexes, cellsEndIndexes);
}

// Kernel aktualizujący parametry ruchu ryb
__global__ void updateFishKernel(float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    update(x, y, vx, vy, ax, ay, i);
}

// Kernel kopiujący dane położenia i kąta nachylenia ryb do przygotowanego bufora do ich wizualizacji
__global__ void copyToVBOKernel(float *vbo, int *ifRed, float *x, float *y, float *vx, float *vy, int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    vbo[4 * i] = x[i];
    vbo[4 * i + 1] = y[i];
    vbo[4 * i + 2] = -atan2f(vy[i], vx[i]);
    vbo[4 * i + 3] = ifRed[i] == 1 ? 1.0f : 0.0f;
}

// Inicjalizuje wstępne parametry ruchu ryb
void initializeFishesParams(cudaFishesParams* shoal) {
    for (int i = 0; i < COUNT; i++) {
        shoal->vx[i] = (rand() % 70 - 35) / 10.0f;
        shoal->vy[i] = (rand() % 70 - 35) / 10.0f;
        shoal->x[i] = rand() % 2000;
        shoal->y[i] = rand() % 1200;
        shoal->ifRed[i] = i % 2 == 0;
    }
}

// Inicjalizuje wstępne parametry ruchu ryb, alokuje potrzebną pamięć i przenosi dane z CPU na GPU. Inicjalizuje
// niezbędne stałe będące się znajdować w constant memory
cudaError_t initializeMemoryOnGPU(cudaArrayPtrs* dPtrs, cudaFishesParams* shoal) {
    cudaError_t status;

    float mf = 0.5f;
    float mv = 3.5f;
    float cx = 0.0f;
    float cy = 0.0f;

    initializeFishesParams(shoal);

    cudaMemcpyToSymbol(maxVelocity, &mv, sizeof(float));
    cudaMemcpyToSymbol(maxForce, &mf, sizeof(float));
    cudaMemcpyToSymbol(CX, &cx, sizeof(float));
    cudaMemcpyToSymbol(CY, &cy, sizeof(float));

    status = cudaMalloc(reinterpret_cast<void**>(&dPtrs->gridIndexes), sizeof(int) * COUNT);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMalloc(reinterpret_cast<void**>(&dPtrs->cellsStartIndexes), sizeof(int) * 41 * 25);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMalloc(reinterpret_cast<void**>(&dPtrs->cellsEndIndexes), sizeof(int) * 41 * 25);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMalloc(reinterpret_cast<void**>(&dPtrs->fishesIndexes), sizeof(int) * COUNT);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMalloc(reinterpret_cast<void**>(&dPtrs->ifRed), sizeof(int) * COUNT);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMalloc(reinterpret_cast<void**>(&dPtrs->x), sizeof(float) * COUNT);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMalloc(reinterpret_cast<void**>(&dPtrs->y), sizeof(float) * COUNT);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMalloc(reinterpret_cast<void**>(&dPtrs->vx), sizeof(float) * COUNT);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMalloc(reinterpret_cast<void**>(&dPtrs->vy), sizeof(float) * COUNT);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMalloc(reinterpret_cast<void**>(&dPtrs->ax), sizeof(float) * COUNT);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMalloc(reinterpret_cast<void**>(&dPtrs->ay), sizeof(float) * COUNT);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMalloc(reinterpret_cast<void**>(&dPtrs->nx), sizeof(float) * COUNT);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMalloc(reinterpret_cast<void**>(&dPtrs->ny), sizeof(float) * COUNT);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMalloc(reinterpret_cast<void**>(&dPtrs->nvx), sizeof(float) * COUNT);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMalloc(reinterpret_cast<void**>(&dPtrs->nvy), sizeof(float) * COUNT);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMalloc(reinterpret_cast<void**>(&dPtrs->nifRed), sizeof(int) * COUNT);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }

    status = cudaMemcpy(dPtrs->x, shoal->x,sizeof(float) * COUNT, cudaMemcpyHostToDevice);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMemcpy(dPtrs->y, shoal->y,sizeof(float) * COUNT, cudaMemcpyHostToDevice);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMemcpy(dPtrs->vx, shoal->vx,sizeof(float) * COUNT, cudaMemcpyHostToDevice);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMemcpy(dPtrs->vy, shoal->vy,sizeof(float) * COUNT, cudaMemcpyHostToDevice);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    status = cudaMemcpy(dPtrs->ifRed, shoal->ifRed,sizeof(int) * COUNT, cudaMemcpyHostToDevice);
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    return status;
}

// Sprawdza czy nie pojawił się błąd. Jeżeli tak, zwalnia pamięć i przygotowuje funkcję ją wywołującą do zakończenia
// pracy
bool checkCudaError(cudaError_t cudaStatus, cudaArrayPtrs* dPtrs) {
    if (cudaStatus != cudaSuccess) {
        std::cerr << cudaGetErrorString(cudaStatus) << std::endl;
        freeMemoryOnGPU(dPtrs);
        return false;
    }
    return true;
}

// Główna funkcja wywoływana w celu modyfikacji położeń ryb dla potrzeb renderowania klatki. W celu odpowiedniego
// zsynchronizowania, aby dane były wiarygodne, prace wywoływanych kerneli są synchronizowane przed wywołanie następnych
cudaError_t updateShoalSceneOnGPU(cudaGraphicsResource_t cudaVBO, cudaArrayPtrs* dPtrs, float cx, float cy, float swR,
    float awR, float cwR, float swG, float awG, float cwG) {

    cudaError_t status;

    float* vbo;
    size_t numBytes = 0;

    status = cudaGraphicsMapResources(1, &cudaVBO, 0);
    if (!checkCudaError(status, dPtrs)) {
        return status;
        }
    status = cudaGraphicsResourceGetMappedPointer((void**)&vbo, &numBytes, cudaVBO);
    if (!checkCudaError(status, dPtrs)) {
        return status;
        }

    cudaMemcpyToSymbol(CX, &cx, sizeof(float));
    cudaMemcpyToSymbol(CY, &cy, sizeof(float));
    cudaMemcpyToSymbol(alignmentWeightRed, &awR, sizeof(float));
    cudaMemcpyToSymbol(separationWeightRed, &swR, sizeof(float));
    cudaMemcpyToSymbol(cohesionWeightRed, &cwR, sizeof(float));
    cudaMemcpyToSymbol(alignmentWeightGreen, &awG, sizeof(float));
    cudaMemcpyToSymbol(separationWeightGreen, &swG, sizeof(float));
    cudaMemcpyToSymbol(cohesionWeightGreen, &cwG, sizeof(float));

    dim3 blockDim(256);
    dim3 gridDim((COUNT + blockDim.x - 1) / blockDim.x);

    auto start = std::chrono::high_resolution_clock::now();

    splitFishesOnGridKernel<<<gridDim, blockDim>>>(dPtrs->x, dPtrs->y, COUNT, dPtrs->gridIndexes);
    status = cudaDeviceSynchronize();
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto splitDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    start = std::chrono::high_resolution_clock::now();

    initializeFishesIndexesKernel<<<gridDim, blockDim>>>(dPtrs->fishesIndexes, COUNT);
    status = cudaDeviceSynchronize();
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }

    thrust::device_ptr<int> dFishesIndexes(dPtrs->fishesIndexes);
    thrust::sort_by_key(dPtrs->gridIndexes, dPtrs->gridIndexes + COUNT, dFishesIndexes);

    initializeSortedParamsKernel<<<gridDim, blockDim>>>(dPtrs->ifRed, dPtrs->x, dPtrs->y, dPtrs->vx, dPtrs->vy,
        dPtrs->nx, dPtrs->ny, dPtrs->nvx, dPtrs->nvy, dPtrs->nifRed, dPtrs->fishesIndexes, COUNT);
    status = cudaDeviceSynchronize();
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }

    sortParamsKernel<<<gridDim, blockDim>>>(dPtrs->ifRed, dPtrs->x, dPtrs->y, dPtrs->vx, dPtrs->vy,
        dPtrs->nx, dPtrs->ny, dPtrs->nvx, dPtrs->nvy, dPtrs->nifRed, COUNT);
    status = cudaDeviceSynchronize();
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }

    end = std::chrono::high_resolution_clock::now();
    auto thrustSortDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    start = std::chrono::high_resolution_clock::now();
    dim3 gridDimForCells((CELLSNUM + blockDim.x - 1) / blockDim.x);
    resetCellsStartEndIndexesKernel<<<gridDimForCells, blockDim>>>(dPtrs->cellsStartIndexes, dPtrs->cellsEndIndexes,CELLSNUM);
    status = cudaDeviceSynchronize();
    if (!checkCudaError(status, dPtrs)) {
        return status;
    }
    findCellsStartEndIndexesKernel<<<gridDim, blockDim>>>(dPtrs->cellsStartIndexes, dPtrs->cellsEndIndexes, dPtrs->gridIndexes, COUNT);
    end = std::chrono::high_resolution_clock::now();
    auto cellsStartDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    start = std::chrono::high_resolution_clock::now();
    computeForceKernel<<<gridDim, blockDim>>>(dPtrs->ifRed, dPtrs->x, dPtrs->y, dPtrs->vx, dPtrs->vy, dPtrs->ax,
        dPtrs->ay, COUNT, dPtrs->gridIndexes, dPtrs->cellsStartIndexes, dPtrs->cellsEndIndexes);
    status = cudaDeviceSynchronize();
    if (!checkCudaError(status, dPtrs)) {
        return status;
        }
    end = std::chrono::high_resolution_clock::now();
    auto computeForcesDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    start = std::chrono::high_resolution_clock::now();
    updateFishKernel<<<gridDim, blockDim>>>(dPtrs->x, dPtrs->y, dPtrs->vx, dPtrs->vy, dPtrs->ax, dPtrs->ay, COUNT);
    status = cudaDeviceSynchronize();
    if (!checkCudaError(status, dPtrs)) {
        return status;
        }
    end = std::chrono::high_resolution_clock::now();
    auto updateDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    start = std::chrono::high_resolution_clock::now();
    copyToVBOKernel<<<gridDim, blockDim>>>(vbo, dPtrs->ifRed, dPtrs->x, dPtrs->y, dPtrs->vx, dPtrs->vy, COUNT);
    end = std::chrono::high_resolution_clock::now();
    auto copyingDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);

    cudaGraphicsUnmapResources(1, &cudaVBO, 0);

    std::cout << "Operation times: Splitting to cells grids: " << splitDuration.count() / 1e9 <<
        "s, Thrust sort: " << thrustSortDuration.count() / 1e9 << "s, Finding start indexes: " <<
        cellsStartDuration.count() / 1e9 << "s, Computing forces: " << computeForcesDuration.count() / 1e9 <<
        "s, Updating locations and velocities: " << updateDuration.count() / 1e9 << "s, Copying data to VBO: " <<
            copyingDuration.count() / 1e9 << "s" << std::endl;
    return status;
}

// Zwalnia zaalokowaną pamięć na GPU
void freeMemoryOnGPU(cudaArrayPtrs* dPtrs) {
    cudaFree(dPtrs->x);
    cudaFree(dPtrs->y);
    cudaFree(dPtrs->vx);
    cudaFree(dPtrs->vy);
    cudaFree(dPtrs->ax);
    cudaFree(dPtrs->ay);
    cudaFree(dPtrs->nx);
    cudaFree(dPtrs->ny);
    cudaFree(dPtrs->nvx);
    cudaFree(dPtrs->nvy);
    cudaFree(dPtrs->ifRed);
    cudaFree(dPtrs->nifRed);
    cudaFree(dPtrs->cellsStartIndexes);
    cudaFree(dPtrs->gridIndexes);
    cudaFree(dPtrs->fishesIndexes);
}
