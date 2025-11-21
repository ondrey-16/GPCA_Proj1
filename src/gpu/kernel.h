//
// Created by ondrey on 20.11.2025.
//

#ifndef SHOAL_FISH_ANIMATION_KERNEL_H
#define SHOAL_FISH_ANIMATION_KERNEL_H

#include "cuda_runtime.h"

#define COUNT 10000
#define CELLSNUM 1025

struct cudaFishesParams {
    const int N = COUNT;
    float x[COUNT];
    float y[COUNT];
    float vx[COUNT];
    float vy[COUNT];
    float ax[COUNT];
    float ay[COUNT];

    cudaFishesParams() {}
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
__device__ cudaVector separate(float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int start, int end, int fishIdx, int* count);
__device__ cudaVector align(float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int start, int end, int fishIdx, int* count);
__device__ cudaVector cohere(float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int start, int end, int fishIdx, int* count);
__device__ void normalizeSteerVector(cudaVector* steer, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int fishIdx, int count);
__device__ void normalizeCohesionSteerVector(cudaVector* steer, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int fishIdx, int count);
__device__ void flock(float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int fishIdx, int fx, int fy, const int* cellsStartIndexes);
__device__ void update(float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int fishIdx);
__device__ void borders(float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int fishIdx);

__global__ void splitFishesOnGridKernel(float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int* indexes);
__global__ void resetCellsStartIndexesKernel(int* cellsStartIndexes, int cellsN);
__global__ void findCellsStartIndexesKernel(int* cellsStartIndexes, int* gridIndexes, int N);
__global__ void initializeFishesIndexesKernel(int* fishesIndexes, int N);
__global__ void initializeSortedParamsKernel(float* x, float* y, float* vx, float* vy, float* nx, float* ny, float* nvx, float* nvy, int* fishesIndexes, int N);
__global__ void sortParamsKernel(float* x, float* y, float* vx, float* vy, float* nx, float* ny, float* nvx, float* nvy, int N);
__global__ void computeForceKernel(float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, const int* gridIndexes, const int* cellsStartIndexes);
__global__ void updateFishKernel(float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N);

void initializeFishesParams(int N, cudaFishesParams* shoal);
void updateShoalSceneOnGPU(cudaFishesParams* shoal, float* xOnDevice, float* yOnDevice, float* vxOnDevice, float* vyOnDevice,
    float* axOnDevice, float* ayOnDevice, float* nxOnDevice, float* nyOnDevice, float* nvxOnDevice, float* nvyOnDevice,
    int* gridIndexesOnDevice, int* cellsStartIndexesOnDevice, int* fishesIndexes);
void initializeMemoryOnGPU(cudaFishesParams* shoal, float** xOnDevice, float** yOnDevice, float** vxOnDevice, float** vyOnDevice,
    float** axOnDevice, float** ayOnDevice, float** nxOnDevice, float** nyOnDevice, float** nvxOnDevice, float** nvyOnDevice,
    int** gridIndexesOnDevice, int** cellsStartIndexesOnDevice, int** fishesIndexes);

void freeMemoryOnGPU(float** xOnDevice, float** yOnDevice, float** vxOnDevice, float** vyOnDevice,
    float** axOnDevice, float** ayOnDevice, int** gridIndexesOnDevice, int** cellsStartIndexesOnDevice);

#endif //SHOAL_FISH_ANIMATION_KERNEL_H