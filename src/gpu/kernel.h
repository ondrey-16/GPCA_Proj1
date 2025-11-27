//
// Created by ondrey on 20.11.2025.
//

#ifndef SHOAL_FISH_ANIMATION_KERNEL_H
#define SHOAL_FISH_ANIMATION_KERNEL_H

#include "cuda_runtime.h"

#define COUNT 10000
#define CELLSNUM 1025

struct cudaFishesParams {
    float x[COUNT];
    float y[COUNT];
    float vx[COUNT];
    float vy[COUNT];
    int ifRed[COUNT];
    cudaFishesParams() {}
};

struct cudaArrayPtrs {
    float* x;
    float* nx;
    float* y;
    float* ny;
    float* vx;
    float* nvx;
    float* vy;
    float* nvy;
    float* ax;
    float* ay;
    int* ifRed;
    int* nifRed;
    int* cellsStartIndexes;
    int* cellsEndIndexes;
    int* gridIndexes;
    int* fishesIndexes;
    void* basePtr;

    cudaArrayPtrs() {}
};

struct cudaVector {
    float x;
    float y;
    __host__ __device__ cudaVector(const float x, const float y) {
        this->x = x;
        this->y = y;
    }
};

__device__ float magnitude(float x, float y);
__device__ void setMagnitude(float* x, float* y, float mag);
__device__ void limit(float* x, float* y, float max);
__device__ float distance(float x1, float y1, float x2, float y2);
__device__ cudaVector add(float x1, float y1, float x2, float y2);
__device__ cudaVector sub(float x1, float y1, float x2, float y2);
__device__ cudaVector mul(float x1, float y1, float x2, float y2);
__device__ cudaVector div(float x1, float y1, float x2, float y2);
__device__ void normalizeSteerVector(cudaVector* steer, float* vx, float* vy, int fishIdx, int count);
__device__ void normalizeCohesionSteerVector(cudaVector* steer, float* x, float* y, float* vx, float* vy, int fishIdx,
    int count);
__device__ cudaVector avoidObstacle(float* x, float* y, float* vx, float* vy, int fishIdx);
__device__ int intersection(cudaVector rayStart, cudaVector rayDirection);
__device__ void flock(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int fishIdx, int fx,
    int fy, const int* cellsStartIndexes, const int* cellsEndIndexes);
__device__ void update(float* x, float* y, float* vx, float* vy, float* ax, float* ay, int fishIdx);
__device__ void borders(float* x, float* y, int fishIdx);

__global__ void splitFishesOnGridKernel(float* x, float* y, int N, int* gridIndexes);
__global__ void resetCellsStartEndIndexesKernel(int* cellsStartIndexes, int* cellsEndIndexes, int cellsN);
__global__ void findCellsStartEndIndexesKernel(int* cellsStartIndexes, int* cellsEndIndexes, int* gridIndexes, int N);
__global__ void initializeFishesIndexesKernel(int* fishesIndexes, int N);
__global__ void initializeSortedParamsKernel(int* ifRed, float* x, float* y, float* vx, float* vy, float* nx, float* ny,
    float* nvx, float* nvy, int* nifRed, int* fishesIndexes, int N);
__global__ void sortParamsKernel(int* ifRed, float* x, float* y, float* vx, float* vy, float* nx, float* ny, float* nvx,
    float* nvy, int N);
__global__ void computeForceKernel(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N,
    const int* gridIndexes, const int* cellsStartIndexes, const int* cellsEndIndexes);
__global__ void updateFishKernel(float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N);
__global__ void copyToVBOKernel(float* vbo, int* ifRed, float* x, float* y, float* vx, float* vy, int N);

void initializeFishesParams(cudaFishesParams* shoal);
cudaError_t initializeMemoryOnGPU(cudaArrayPtrs* dPtrs, cudaFishesParams* shoal);
cudaError_t updateShoalSceneOnGPU(cudaGraphicsResource_t cudaVBO, cudaArrayPtrs* dPtrs, float cx, float cy, float swR,
    float awR, float cwR, float swG, float awG, float cwG);
bool checkCudaError(cudaError_t cudaStatus, cudaArrayPtrs* dPtrs);
void freeMemoryOnGPU(cudaArrayPtrs* dPtrs);

#endif //SHOAL_FISH_ANIMATION_KERNEL_H