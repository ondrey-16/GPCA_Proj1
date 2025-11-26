//
// Created by ondrey on 20.11.2025.
//

#ifndef SHOAL_FISH_ANIMATION_KERNEL_H
#define SHOAL_FISH_ANIMATION_KERNEL_H

#include "cuda_runtime.h"

#define COUNT 3000
#define CELLSNUM 1025

struct cudaFishesParams {
    const int N = COUNT;
    float x[COUNT];
    float y[COUNT];
    float vx[COUNT];
    float vy[COUNT];
    float ax[COUNT];
    float ay[COUNT];
    int ifRed[COUNT];

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
__device__ cudaVector separate(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int start, int end, int fishIdx, int* count);
__device__ cudaVector align(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int start, int end, int fishIdx, int* count);
__device__ cudaVector cohere(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int start, int end, int fishIdx, int* count);
__device__ void normalizeSteerVector(cudaVector* steer, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int fishIdx, int count);
__device__ void normalizeCohesionSteerVector(cudaVector* steer, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int fishIdx, int count);
__device__ cudaVector avoidObstacle(float* x, float* y, float* vx, float* vy, int N, int fishIdx);
__device__ int intersection(cudaVector rayStart, cudaVector rayDirection);
__device__ void flock(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int fishIdx, int fx, int fy, const int* cellsStartIndexes);
__device__ void update(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int fishIdx);
__device__ void borders(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int fishIdx);

__global__ void splitFishesOnGridKernel(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, int* indexes);
__global__ void resetCellsStartIndexesKernel(int* cellsStartIndexes, int cellsN);
__global__ void findCellsStartIndexesKernel(int* cellsStartIndexes, int* gridIndexes, int N);
__global__ void initializeFishesIndexesKernel(int* fishesIndexes, int N);
__global__ void initializeSortedParamsKernel(int* ifRed, float* x, float* y, float* vx, float* vy, float* nx, float* ny, float* nvx, float* nvy, int* fishesIndexes, int N);
__global__ void sortParamsKernel(int* ifRed, float* x, float* y, float* vx, float* vy, float* nx, float* ny, float* nvx, float* nvy, int N);
__global__ void computeForceKernel(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N, const int* gridIndexes, const int* cellsStartIndexes);
__global__ void updateFishKernel(int* ifRed, float* x, float* y, float* vx, float* vy, float* ax, float* ay, int N);

void initializeFishesParams(int N, cudaFishesParams* shoal);
void updateShoalSceneOnGPU(cudaFishesParams* shoal, int* ifRedOnDevice,  float* xOnDevice, float* yOnDevice, float* vxOnDevice, float* vyOnDevice,
    float* axOnDevice, float* ayOnDevice, float* nxOnDevice, float* nyOnDevice, float* nvxOnDevice, float* nvyOnDevice, int* nifOnRedDevice,
    int* gridIndexesOnDevice, int* cellsStartIndexesOnDevice, int* fishesIndexes, float cx, float cy, float swR,
    float awR, float cwR, float swG, float awG, float cwG);
void initializeMemoryOnGPU(cudaFishesParams* shoal, int** ifRedOnDevice, float** xOnDevice, float** yOnDevice, float** vxOnDevice, float** vyOnDevice,
    float** axOnDevice, float** ayOnDevice, float** nxOnDevice, float** nyOnDevice, float** nvxOnDevice, float** nvyOnDevice, int** nifRedOnDevice,
    int** gridIndexesOnDevice, int** cellsStartIndexesOnDevice, int** fishesIndexes);

void freeMemoryOnGPU(int** ifOnRedDevice, float** xOnDevice, float** yOnDevice, float** vxOnDevice, float** vyOnDevice,
    float** axOnDevice, float** ayOnDevice, float** nxOnDevice, float** nyOnDevice, float** nvxOnDevice,
    float** nvyOnDevice, int** nifOnRedDevice, int** gridIndexesOnDevice, int** cellsStartIndexesOnDevice, int** fishesIndexes);

#endif //SHOAL_FISH_ANIMATION_KERNEL_H