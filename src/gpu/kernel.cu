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

__device__ float magnitude(const float x, const float y) {
    return sqrtf(x * x + y * y);
}

__device__ void setMagnitude(float* x, float* y, const float mag) {
    if (magnitude(*x, *y) > 0) {
        const float scale = mag / magnitude(*x, *y);
        *x *= scale;
        *y *= scale;
    }
}

__device__ void limit(float* x, float* y, const float max) {
    float size = magnitude(*x, *y);
    if (size > max) {
        setMagnitude(x, y, max);
    }
}

__device__ float distance(const float x1, const float y1, const float x2, const float y2) {
    float dx = x1 - x2;
    float dy = y1 - y2;
    return sqrtf(dx*dx + dy*dy);
}

__device__ cudaVector add(const float x1, const float y1, const float x2, const float y2) {
    return cudaVector(x1 + x2, y1 + y2);
}

__device__ cudaVector sub(const float x1, const float y1, const float x2, const float y2) {
    return cudaVector(x1 - x2, y1 - y2);
}

__device__ cudaVector mul(const float x1, const float y1, const float x2, const float y2) {
    return cudaVector(x1 * x2, y1 * y2);
}

__device__ cudaVector div(const float x1, const float y1, const float x2, const float y2) {
    return cudaVector(x1 / x2, y1 / y2);
}

__device__ cudaVector separate(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, const int start, const int end, const int fishIdx, int* count) {
    float neighborDist = 25.0f;
    cudaVector steer(0, 0);

    for (int i = start; i < end; i++) {
        float dist = distance(x[fishIdx], y[fishIdx], x[i], y[i]);
        if (fishIdx != i && dist > 0 && dist < neighborDist && ifRed[fishIdx] == ifRed[i]) {
            cudaVector diff = sub(x[fishIdx], y[fishIdx], x[i], y[i]);
            diff.x /= dist;
            diff.y /= dist;
            steer = add(steer.x, steer.y, diff.x, diff.y);
            (*count)++;
        }
    }

    return steer;
}

__device__ cudaVector align(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, const int start, const int end, const int fishIdx, int* count) {
    float neighborDist = 50.0f;
    cudaVector steer(0, 0);

    for (int i = start; i < end; i++) {
        float dist = distance(x[fishIdx], y[fishIdx], x[i], y[i]);
        if (fishIdx != i && dist > 0 && dist < neighborDist && ifRed[fishIdx] == ifRed[i]) {
            steer = add(steer.x, steer.y, vx[i], vy[i]);
            (*count)++;
        }
    }

    return steer;
}

__device__ void normalizeSteerVector(cudaVector* steer, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int fishIdx, int count) {
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

__device__ cudaVector cohere(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, const int start, const int end, const int fishIdx, int* count) {
    float neighborDist = 50.0f;
    cudaVector steer(0, 0);

    for (int i = start; i < end; i++) {
        float dist = distance(x[fishIdx], y[fishIdx], x[i], y[i]);
        if (fishIdx != i && dist > 0 && dist < neighborDist && ifRed[fishIdx] == ifRed[i]) {
            steer = add(steer.x, steer.y, x[i], y[i]);
            (*count)++;
        }
    }

    return steer;
}

__device__ void normalizeCohesionSteerVector(cudaVector* steer, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int fishIdx, int count) {
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

__device__ cudaVector avoidObstacle(float* x, float* y, float* vx, float* vy, int N, const int fishIdx) {
    float vMag = magnitude(vx[fishIdx], vy[fishIdx]);
    cudaVector rayDirection(vx[fishIdx] / vMag, vy[fishIdx] / vMag);
    cudaVector rayStart(x[fishIdx], y[fishIdx]);
    if (intersection(rayStart, rayDirection) == 1) {
        cudaVector fromObstacle(rayStart.x - CX, rayStart.y - CY);
        float toObstacleMag = magnitude(fromObstacle.x, fromObstacle.y);
        cudaVector toObstacleDirection(fromObstacle.x / toObstacleMag, fromObstacle.y / toObstacleMag);
        cudaVector avoidForce(toObstacleDirection.x * vMag, toObstacleDirection.y * vMag);
        return avoidForce;
    }
    return cudaVector(0, 0);
}

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

__device__ void flock(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, const int fishIdx, const int fx, const int fy, const int* cellsStartIndexes) {
    int gridIndex;

    cudaVector sep(0, 0);
    int count = 0;
    for (int j = fx > 0 ? fx - 1 : 0; j < 41 && j <= fx + 1; j++) {
        for (int k = fy > 0 ? fy - 1 : 0; k < 25 && k <= fy + 1; k++) {
            gridIndex = k * 41 + j;
            if (cellsStartIndexes[gridIndex] == -1) {
                continue;
            }
            cudaVector partSep(0, 0);
            int endIdx = gridIndex + 1;
            while (endIdx < 41 * 25 && cellsStartIndexes[endIdx] == -1) {
                endIdx++;
            }
            partSep = separate(ifRed, x, y, vx, vy, ax, ay, N, cellsStartIndexes[gridIndex], (gridIndex < 41 * 25 - 1) ? cellsStartIndexes[endIdx] : N,
                fishIdx, &count);
            sep = add(sep.x, sep.y, partSep.x, partSep.y);
        }
    }
    normalizeSteerVector(&sep, x, y, vx, vy, ax, ay, N, fishIdx, count);

    cudaVector ali(0, 0);
    count = 0;
    for (int j = fx > 0 ? fx - 1 : 0; j < 41 && j <= fx + 1; j++) {
        for (int k = fy > 0 ? fy - 1 : 0; k < 25 && k <= fy + 1; k++) {
            gridIndex = k * 41 + j;
            if (cellsStartIndexes[gridIndex] == -1) {
                continue;
            }
            cudaVector partAli(0, 0);
            int endIdx = gridIndex + 1;
            while (endIdx < 41 * 25 && cellsStartIndexes[endIdx] == -1) {
                endIdx++;
            }
            partAli = align(ifRed, x, y, vx, vy, ax, ay, N, cellsStartIndexes[gridIndex], (gridIndex < 41 * 25 - 1) ? cellsStartIndexes[endIdx] : N,
                fishIdx, &count);
            ali = add(ali.x, ali.y, partAli.x, partAli.y);
        }
    }
    normalizeSteerVector(&ali, x, y, vx, vy, ax, ay, N, fishIdx, count);

    cudaVector coh(0, 0);
    count = 0;
    for (int j = fx > 0 ? fx - 1 : 0; j < 41 && j <= fx + 1; j++) {
        for (int k = fy > 0 ? fy - 1 : 0; k < 25 && k <= fy + 1; k++) {
            gridIndex = k * 41 + j;
            if (cellsStartIndexes[gridIndex] == -1) {
                continue;
            }
            cudaVector partCoh(0, 0);
            int endIdx = gridIndex + 1;
            while (endIdx < 41 * 25 && cellsStartIndexes[endIdx] == -1) {
                endIdx++;
            }
            partCoh = cohere(ifRed, x, y, vx, vy, ax, ay, N, cellsStartIndexes[gridIndex], (gridIndex < 41 * 25 - 1) ? cellsStartIndexes[endIdx] : N,
                fishIdx, &count);
            coh = add(coh.x, coh.y, partCoh.x, partCoh.y);
        }
    }
    normalizeCohesionSteerVector(&coh, x, y, vx, vy, ax, ay, N, fishIdx, count);

    cudaVector avoid = avoidObstacle(x, y, vx, vy, N, fishIdx);

    float aw = (ifRed[fishIdx] == 1) ? alignmentWeightRed : alignmentWeightGreen;
    float sw = (ifRed[fishIdx] == 1) ? separationWeightRed : separationWeightGreen;
    float cw = (ifRed[fishIdx] == 1) ? cohesionWeightRed : cohesionWeightGreen;

    ax[fishIdx] = ali.x * aw + coh.x * cw + sep.x * sw + 1.5 * avoid.x;
    ay[fishIdx] = ali.y * aw + coh.y * cw + sep.y * sw + 1.5 * avoid.y;
}

__device__ void update(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, const int fishIdx) {
    const cudaVector velocity = add(vx[fishIdx], vy[fishIdx], ax[fishIdx], ay[fishIdx]);
    vx[fishIdx] = velocity.x;
    vy[fishIdx] = velocity.y;
    limit(&vx[fishIdx], &vy[fishIdx], maxVelocity);
    const cudaVector location = add(vx[fishIdx], vy[fishIdx], x[fishIdx], y[fishIdx]);
    x[fishIdx] = location.x;
    y[fishIdx] = location.y;
    ax[fishIdx] *= 0.0f;
    ay[fishIdx] *= 0.0f;
    borders(ifRed, x, y, vx, vy, ax, ay, N, fishIdx);
}

__device__ void borders(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, const int fishIdx) {
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

__global__ void splitFishesOnGridKernel(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int* gridIndexes) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    const int idxX = x[i] / 50;
    const int idxY = y[i] / 50;
    gridIndexes[i] = 41 * idxY + idxX;
}

__global__ void resetCellsStartIndexesKernel(int* cellsStartIndexes, int cellsN) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= cellsN) return;
    cellsStartIndexes[i] = -1;
}

__global__ void initializeFishesIndexesKernel(int* fishesIndexes, int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    fishesIndexes[i] = i;
}

__global__ void initializeSortedParamsKernel(int* ifRed, float* x, float* y, float* vx, float* vy, float* nx, float* ny, float* nvx, float* nvy,  int* nifRed, int* fishesIndexes, int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    nx[i] = x[fishesIndexes[i]];
    ny[i] = y[fishesIndexes[i]];
    nvx[i] = vx[fishesIndexes[i]];
    nvy[i] = vy[fishesIndexes[i]];
    nifRed[i] = ifRed[fishesIndexes[i]];
}

__global__ void sortParamsKernel(int* ifRed, float* x, float* y, float* vx, float* vy, float* nx, float* ny, float* nvx, float* nvy, int* nifRed, int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    x[i] = nx[i];
    y[i] = ny[i];
    vx[i] = nvx[i];
    vy[i] = nvy[i];
    ifRed[i] = nifRed[i];
}

__global__ void findCellsStartIndexesKernel(int* cellsStartIndexes, int* gridIndexes, int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;

    if (i == 0) {
        cellsStartIndexes[gridIndexes[i]] = 0;
    }
    else {
        if (gridIndexes[i - 1] != gridIndexes[i]) {
            cellsStartIndexes[gridIndexes[i]] = i;
        }
    }
}

__global__ void computeForceKernel(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, const int* gridIndexes, const int* cellsStartIndexes) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    int gridIndex = gridIndexes[i];
    const int fy = gridIndex / 41;
    const int fx = gridIndex % 41;

    flock(ifRed, x, y, vx, vy, ax, ay, N, i, fx, fy, cellsStartIndexes);
}

__global__ void updateFishKernel(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;
    update(ifRed, x, y, vx, vy, ax, ay, N, i);
}

void initializeFishesParams(const int N, cudaFishesParams* shoal) {
    for (int i = 0; i < N; i++) {
        shoal->ax[i] = 0.0f;
        shoal->ay[i] = 0.5f;
        shoal->vx[i] = rand() % 4000 - 2000;
        shoal->vy[i] = rand() % 2400 - 1200;
        shoal->x[i] = rand() % 2000;
        shoal->y[i] = rand() % 1200;
        shoal->ifRed[i] = i % 2 == 0;
    }
}

void initializeMemoryOnGPU(cudaFishesParams* shoal, int** ifRedOnDevice, float** xOnDevice, float** yOnDevice, float** vxOnDevice, float** vyOnDevice,
    float** axOnDevice, float** ayOnDevice, float** nxOnDevice, float** nyOnDevice, float** nvxOnDevice, float** nvyOnDevice, int** nifRedOnDevice,
    int** gridIndexesOnDevice, int** cellsStartIndexesOnDevice, int** fishesIndexes) {
    cudaError_t status;

    float mf = 0.5f;
    float mv = 3.5f;
    float cx = 0.0f;
    float cy = 0.0f;

    cudaMemcpyToSymbol(maxVelocity, &mv, sizeof(float));
    cudaMemcpyToSymbol(maxForce, &mf, sizeof(float));
    cudaMemcpyToSymbol(CX, &cx, sizeof(float));
    cudaMemcpyToSymbol(CY, &cy, sizeof(float));

    status = cudaMalloc(reinterpret_cast<void**>(gridIndexesOnDevice), sizeof(int) * shoal->N);
    if (status != cudaSuccess) {
        std::cerr << "Malloc gridIndexesOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMalloc(reinterpret_cast<void**>(cellsStartIndexesOnDevice), sizeof(int) * 41 * 25);
    if (status != cudaSuccess) {
        std::cerr << "Malloc cellsStartIndexesOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMalloc(reinterpret_cast<void**>(fishesIndexes), sizeof(int) * shoal->N);
    if (status != cudaSuccess) {
        std::cerr << "Malloc cellsStartIndexesOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMalloc(reinterpret_cast<void**>(ifRedOnDevice), sizeof(int) * shoal->N);
    if (status != cudaSuccess) {
        std::cerr << "Malloc xOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMalloc(reinterpret_cast<void**>(xOnDevice), sizeof(float) * shoal->N);
    if (status != cudaSuccess) {
        std::cerr << "Malloc xOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMalloc(reinterpret_cast<void**>(yOnDevice), sizeof(float) * shoal->N);
    if (status != cudaSuccess) {
        std::cerr << "Malloc yOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMalloc(reinterpret_cast<void**>(vxOnDevice), sizeof(float) * shoal->N);
    if (status != cudaSuccess) {
        std::cerr << "Malloc vxOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMalloc(reinterpret_cast<void**>(vyOnDevice), sizeof(float) * shoal->N);
    if (status != cudaSuccess) {
        std::cerr << "Malloc vyOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMalloc(reinterpret_cast<void**>(axOnDevice), sizeof(float) * shoal->N);
    if (status != cudaSuccess) {
        std::cerr << "Malloc axOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMalloc(reinterpret_cast<void**>(ayOnDevice), sizeof(float) * shoal->N);
    if (status != cudaSuccess) {
        std::cerr << "Malloc ayOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMalloc(reinterpret_cast<void**>(nxOnDevice), sizeof(float) * shoal->N);
    if (status != cudaSuccess) {
        std::cerr << "Malloc xOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMalloc(reinterpret_cast<void**>(nyOnDevice), sizeof(float) * shoal->N);
    if (status != cudaSuccess) {
        std::cerr << "Malloc yOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMalloc(reinterpret_cast<void**>(nvxOnDevice), sizeof(float) * shoal->N);
    if (status != cudaSuccess) {
        std::cerr << "Malloc vxOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMalloc(reinterpret_cast<void**>(nvyOnDevice), sizeof(float) * shoal->N);
    if (status != cudaSuccess) {
        std::cerr << "Malloc vyOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMalloc(reinterpret_cast<void**>(nifRedOnDevice), sizeof(int) * shoal->N);
    if (status != cudaSuccess) {
        std::cerr << "Malloc vyOnDevice: " << status << std::endl;
        exit(1);
    }

    status = cudaMemcpy(*xOnDevice, shoal->x,sizeof(float) * shoal->N, cudaMemcpyHostToDevice);
    if (status != cudaSuccess) {
        std::cerr << "Memcpy to device xOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMemcpy(*yOnDevice, shoal->y,sizeof(float) * shoal->N, cudaMemcpyHostToDevice);
    if (status != cudaSuccess) {
        std::cerr << "Memcpy to device yOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMemcpy(*vxOnDevice, shoal->vx,sizeof(float) * shoal->N, cudaMemcpyHostToDevice);
    if (status != cudaSuccess) {
        std::cerr << "Memcpy to device vxOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMemcpy(*vyOnDevice, shoal->vy,sizeof(float) * shoal->N, cudaMemcpyHostToDevice);
    if (status != cudaSuccess) {
        std::cerr << "Memcpy to device vyOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMemcpy(*axOnDevice, shoal->ax,sizeof(float) * shoal->N, cudaMemcpyHostToDevice);
    if (status != cudaSuccess) {
        std::cerr << "Memcpy to device axOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMemcpy(*ayOnDevice, shoal->ay,sizeof(float) * shoal->N, cudaMemcpyHostToDevice);
    if (status != cudaSuccess) {
        std::cerr << "Memcpy to device ayOnDevice: " << status << std::endl;
        exit(1);
    }
    status = cudaMemcpy(*ifRedOnDevice, shoal->ifRed,sizeof(int) * shoal->N, cudaMemcpyHostToDevice);
    if (status != cudaSuccess) {
        std::cerr << "Memcpy to device ayOnDevice: " << status << std::endl;
        exit(1);
    }
}

void updateShoalSceneOnGPU(cudaFishesParams* shoal, int* ifRedOnDevice,  float* xOnDevice, float* yOnDevice, float* vxOnDevice, float* vyOnDevice,
    float* axOnDevice, float* ayOnDevice, float* nxOnDevice, float* nyOnDevice, float* nvxOnDevice, float* nvyOnDevice, int* nifRedOnDevice,
    int* gridIndexesOnDevice, int* cellsStartIndexesOnDevice, int* fishesIndexes, float cx, float cy, float swR,
    float awR, float cwR, float swG, float awG, float cwG) {
    cudaError_t status;

    cudaMemcpyToSymbol(CX, &cx, sizeof(float));
    cudaMemcpyToSymbol(CY, &cy, sizeof(float));
    cudaMemcpyToSymbol(alignmentWeightRed, &awR, sizeof(float));
    cudaMemcpyToSymbol(separationWeightRed, &swR, sizeof(float));
    cudaMemcpyToSymbol(cohesionWeightRed, &cwR, sizeof(float));
    cudaMemcpyToSymbol(alignmentWeightGreen, &awG, sizeof(float));
    cudaMemcpyToSymbol(separationWeightGreen, &swG, sizeof(float));
    cudaMemcpyToSymbol(cohesionWeightGreen, &cwG, sizeof(float));

    dim3 blockDim(256);
    dim3 gridDim((shoal->N + blockDim.x - 1) / blockDim.x);

    auto start = std::chrono::high_resolution_clock::now();

    splitFishesOnGridKernel<<<gridDim, blockDim>>>(ifRedOnDevice, xOnDevice, yOnDevice, vxOnDevice, vyOnDevice, axOnDevice, ayOnDevice, shoal->N, gridIndexesOnDevice);
    status = cudaDeviceSynchronize();
    if (status != cudaSuccess) {
        std::cerr << "splitFishesOnGridKernel: " << status << std::endl;
        exit(1);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto splitDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    start = std::chrono::high_resolution_clock::now();

    initializeFishesIndexesKernel<<<gridDim, blockDim>>>(fishesIndexes, shoal->N);
    status = cudaDeviceSynchronize();
    if (status != cudaSuccess) {
        std::cerr << "initializeFishesIndexesKernel: " << status << std::endl;
        exit(1);
    }

    thrust::device_ptr<int> dFishesIndexes(fishesIndexes);
    thrust::sort_by_key(gridIndexesOnDevice, gridIndexesOnDevice + shoal->N, dFishesIndexes);

    initializeSortedParamsKernel<<<gridDim, blockDim>>>(ifRedOnDevice, xOnDevice, yOnDevice, vxOnDevice, vyOnDevice,
        nxOnDevice, nyOnDevice, nvxOnDevice, nvyOnDevice, nifRedOnDevice, fishesIndexes, shoal->N);
    status = cudaDeviceSynchronize();
    if (status != cudaSuccess) {
        std::cerr << "initializeSortedParamsKernel: " << status << std::endl;
        exit(1);
    }

    sortParamsKernel<<<gridDim, blockDim>>>(ifRedOnDevice, xOnDevice, yOnDevice, vxOnDevice, vyOnDevice,
        nxOnDevice, nyOnDevice, nvxOnDevice, nvyOnDevice, nifRedOnDevice, shoal->N);
    status = cudaDeviceSynchronize();
    if (status != cudaSuccess) {
        std::cerr << "sortParamsKernel: " << status << std::endl;
        exit(1);
    }

    end = std::chrono::high_resolution_clock::now();
    auto thrustSortDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    start = std::chrono::high_resolution_clock::now();
    dim3 gridDimForCells((CELLSNUM + blockDim.x - 1) / blockDim.x);
    resetCellsStartIndexesKernel<<<gridDimForCells, blockDim>>>(cellsStartIndexesOnDevice, CELLSNUM);
    status = cudaDeviceSynchronize();
    if (status != cudaSuccess) {
        std::cerr << "resetCellsStartIndexesKernel: " << status << std::endl;
        exit(1);
    }
    findCellsStartIndexesKernel<<<gridDim, blockDim>>>(cellsStartIndexesOnDevice, gridIndexesOnDevice, shoal->N);
    end = std::chrono::high_resolution_clock::now();
    auto cellsStartDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    start = std::chrono::high_resolution_clock::now();
    computeForceKernel<<<gridDim, blockDim>>>(ifRedOnDevice, xOnDevice, yOnDevice, vxOnDevice, vyOnDevice, axOnDevice, ayOnDevice, shoal->N, gridIndexesOnDevice, cellsStartIndexesOnDevice);
    status = cudaDeviceSynchronize();
    if (status != cudaSuccess) {
        std::cerr << "computeForceKernel: " << status << std::endl;
        exit(1);
    }
    end = std::chrono::high_resolution_clock::now();
    auto computeForcesDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    start = std::chrono::high_resolution_clock::now();
    updateFishKernel<<<gridDim, blockDim>>>(ifRedOnDevice, xOnDevice, yOnDevice, vxOnDevice, vyOnDevice, axOnDevice, ayOnDevice, shoal->N);
    status = cudaDeviceSynchronize();
    if (status != cudaSuccess) {
        std::cerr << "updateFishKernel: " << status << std::endl;
        exit(1);
    }
    end = std::chrono::high_resolution_clock::now();
    auto updateDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    start = std::chrono::high_resolution_clock::now();
    status = cudaMemcpy(shoal->x, xOnDevice,sizeof(float) * shoal->N, cudaMemcpyDeviceToHost);
    if (status != cudaSuccess) {
        std::cerr << "Memcpy to host x: " << status << std::endl;
        exit(1);
    }
    status = cudaMemcpy(shoal->y, yOnDevice,sizeof(float) * shoal->N, cudaMemcpyDeviceToHost);
    if (status != cudaSuccess) {
        std::cerr << "Memcpy to host y: " << status << std::endl;
        exit(1);
    }
    status = cudaMemcpy(shoal->vx, vxOnDevice,sizeof(float) * shoal->N, cudaMemcpyDeviceToHost);
    if (status != cudaSuccess) {
        std::cerr << "Memcpy to host vx: " << status << std::endl;
        exit(1);
    }
    status = cudaMemcpy(shoal->vy, vyOnDevice,sizeof(float) * shoal->N, cudaMemcpyDeviceToHost);
    if (status != cudaSuccess) {
        std::cerr << "Memcpy to host vy: " << status << std::endl;
        exit(1);
    }
    status = cudaMemcpy(shoal->ax, axOnDevice,sizeof(float) * shoal->N, cudaMemcpyDeviceToHost);
    if (status != cudaSuccess) {
        std::cerr << "Memcpy to host ax: " << status << std::endl;
        exit(1);
    }
    status = cudaMemcpy(shoal->ay, ayOnDevice,sizeof(float) * shoal->N, cudaMemcpyDeviceToHost);
    if (status != cudaSuccess) {
        std::cerr << "Memcpy to host ay: " << status << std::endl;
        exit(1);
    }
    status = cudaMemcpy(shoal->ifRed, ifRedOnDevice,sizeof(int) * shoal->N, cudaMemcpyDeviceToHost);
    if (status != cudaSuccess) {
        std::cerr << "Memcpy to host ifRed: " << status << std::endl;
        exit(1);
    }
    end = std::chrono::high_resolution_clock::now();
    auto copyingDuration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // std::cout << "Times: " << splitDuration.count() << " " << thrustSortDuration.count() << " " <<
    //     cellsStartDuration.count() << " " << computeForcesDuration.count() << " " << updateDuration.count() << " " <<
    //         copyingDuration.count() << std::endl;
}

void freeMemoryOnGPU(int** ifRedOnDevice, float** xOnDevice, float** yOnDevice, float** vxOnDevice, float** vyOnDevice,
    float** axOnDevice, float** ayOnDevice, float** nxOnDevice, float** nyOnDevice, float** nvxOnDevice,
    float** nvyOnDevice, int** nifRedOnDevice, int** gridIndexesOnDevice, int** cellsStartIndexesOnDevice, int** fishesIndexes) {
    cudaFree(*ifRedOnDevice);
    cudaFree(*xOnDevice);
    cudaFree(*yOnDevice);
    cudaFree(*vxOnDevice);
    cudaFree(*vyOnDevice);
    cudaFree(*axOnDevice);
    cudaFree(*ayOnDevice);
    cudaFree(*nxOnDevice);
    cudaFree(*nyOnDevice);
    cudaFree(*nvxOnDevice);
    cudaFree(*nvyOnDevice);
    cudaFree(*nifRedOnDevice);
    cudaFree(*gridIndexesOnDevice);
    cudaFree(*cellsStartIndexesOnDevice);
    cudaFree(*fishesIndexes);
}

