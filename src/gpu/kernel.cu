//
// Created by ondrey on 18.11.2025.
//

#include "cuda_runtime.h"

struct cudaFishesParams {
    int N;
    float* x;
    float* y;
    float* vx;
    float* vy;
    float* ax;
    float* ay;
    cudaFishesParams(const int n) {
        this->N = n;
        this->x = new float[N];
        this->y = new float[N];
        this->vx = new float[N];
        this->vy = new float[N];
        this->ax = new float[N];
        this->ay = new float[N];
    }
};

struct cudaVector {
    float x;
    float y;
    cudaVector(const float x, const float y) {
        this->x = x;
        this->y = y;
    }
};

__constant__ float maxVelocity;
__constant__ float maxForce;
__constant__ float alignmentWeight;
__constant__ float separationWeight;
__constant__ float cohesionWeight;

__device__ float magnitude(float x, float y);
__device__ void setMagnitude(float* x, float* y, float mag);
__device__ void limit(float* x, float* y, float max);
__device__ float distance(float x1, float y1, float x2, float y2);
__device__ cudaVector add(float x1, float y1, float x2, float y2);
__device__ cudaVector sub(float x1, float y1, float x2, float y2);
__device__ cudaVector mul(float x1, float y1, float x2, float y2);
__device__ cudaVector div(float x1, float y1, float x2, float y2);
__device__ cudaVector separate(const cudaFishesParams* shoal, int start, int end, int fishIdx, int* count);
__device__ cudaVector align(const cudaFishesParams* shoal, int start, int end, int fishIdx, int* count);
__device__ cudaVector cohere(const cudaFishesParams* shoal, int start, int end, int fishIdx, int* count);
__device__ void normalizeSteerVector(cudaVector* steer, const cudaFishesParams* shoal, int fishIdx, int count);
__device__ void normalizeCohesionSteerVector(cudaVector* steer, const cudaFishesParams* shoal, int fishIdx, int count);
__device__ void flock(const cudaFishesParams* shoal, int fishIdx, int fx, int fy, const int* cellsStartIndexes);
__device__ void update(const cudaFishesParams* shoal, int fishIdx);
__device__ void borders(const cudaFishesParams* shoal, int fishIdx);

__global__ void splitFishesOnGridKernel(cudaFishesParams* shoal, int* indexes);
__global__ void computeForceKernel(const cudaFishesParams* shoal, int* gridIndexes, int* cellsStartIndexes);

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
    return cudaVector(x1 + y1, x2 + y2);
}

__device__ cudaVector sub(const float x1, const float y1, const float x2, const float y2) {
    return cudaVector(x1 - y1, x2 - y2);
}

__device__ cudaVector mul(const float x1, const float y1, const float x2, const float y2) {
    return cudaVector(x1 * x2, y1 * y2);
}

__device__ cudaVector div(const float x1, const float y1, const float x2, const float y2) {
    return cudaVector(x1 / x2, y1 / y2);
}

__device__ cudaVector separate(const cudaFishesParams* shoal, const int start, const int end, const int fishIdx, int* count) {
    float neighborDist = 25.0f;
    cudaVector steer(0, 0);

    for (int i = start; i < end; i++) {
        float dist = distance(shoal->x[fishIdx], shoal->y[fishIdx], shoal->x[i], shoal->y[i]);
        if (fishIdx != i && dist > 0 && dist < neighborDist) {
            cudaVector diff = sub(shoal->x[fishIdx], shoal->y[fishIdx], shoal->x[i], shoal->y[i]);
            diff.x /= dist;
            diff.y /= dist;
            steer = add(steer.x, steer.y, diff.x, diff.y);
            count++;
        }
    }

    return steer;
}

__device__ cudaVector align(const cudaFishesParams* shoal, const int start, const int end, const int fishIdx, int* count) {
    float neighborDist = 25.0f;
    cudaVector steer(0, 0);

    for (int i = start; i < end; i++) {
        float dist = distance(shoal->x[fishIdx], shoal->y[fishIdx], shoal->x[i], shoal->y[i]);
        if (fishIdx != i && dist > 0 && dist < neighborDist) {
            steer = add(steer.x, steer.y, shoal->vx[i], shoal->vy[i]);
            count++;
        }
    }

    return steer;
}

__device__ void normalizeSteerVector(cudaVector* steer, const cudaFishesParams* shoal, int fishIdx, int count) {
    if (count > 0) {
        steer->x /= static_cast<float>(count);
        steer->y /= static_cast<float>(count);
        setMagnitude(&steer->x, &steer->y, maxVelocity);
        *steer = sub(steer->x, steer->y, shoal->vx[fishIdx], shoal->vy[fishIdx]);
        limit(&steer->x, &steer->y, maxForce);
    }
    else {
        steer->x = 0.f;
        steer->y = 0.f;
    }
}

__device__ cudaVector cohere(const cudaFishesParams* shoal, const int start, const int end, const int fishIdx, int* count) {
    float neighborDist = 25.0f;
    cudaVector steer(0, 0);

    for (int i = start; i < end; i++) {
        float dist = distance(shoal->x[fishIdx], shoal->y[fishIdx], shoal->x[i], shoal->y[i]);
        if (fishIdx != i && dist > 0 && dist < neighborDist) {
            steer = add(steer.x, steer.y, shoal->x[i], shoal->y[i]);
            count++;
        }
    }

    return steer;
}

__device__ void normalizeCohesionSteerVector(cudaVector* steer, const cudaFishesParams* shoal, int fishIdx, int count) {
    if (count > 0) {
        steer->x /= static_cast<float>(count);
        steer->y /= static_cast<float>(count);
        *steer = sub(steer->x, steer->y, shoal->x[fishIdx], shoal->y[fishIdx]);
        setMagnitude(&steer->x, &steer->y, maxVelocity);
        *steer = sub(steer->x, steer->y, shoal->vx[fishIdx], shoal->vy[fishIdx]);
        limit(&steer->x, &steer->y, maxForce);
    }
    else {
        steer->x = 0.f;
        steer->y = 0.f;
    }
}

__device__ void flock(const cudaFishesParams* shoal, const int fishIdx, const int fx, const int fy, const int* cellsStartIndexes) {
    int gridIndex;

    cudaVector sep(0, 0);
    int count = 0;
    for (int j = fx > 0 ? fx - 1 : 0; j < 40 && j <= fx + 1; j++) {
        for (int k = fy > 0 ? fy - 1 : 0; k < 24 && j <= fy + 1; k++) {
            gridIndex = k * 41 + j;
            cudaVector partSep(0, 0);
            partSep = separate(shoal, cellsStartIndexes[gridIndex], (gridIndex < 41 * 25 - 1) ? cellsStartIndexes[gridIndex + 1] : shoal->N,
                fishIdx, &count);
            sep = add(sep.x, sep.y, partSep.x, partSep.y);
        }
    }
    normalizeSteerVector(&sep, shoal, fishIdx, count);

    cudaVector ali(0, 0);
    count = 0;
    for (int j = fx > 0 ? fx - 1 : 0; j < 40 && j <= fx + 1; j++) {
        for (int k = fy > 0 ? fy - 1 : 0; k < 24 && j <= fy + 1; k++) {
            gridIndex = k * 41 + j;
            cudaVector partAli(0, 0);
            partAli = align(shoal, cellsStartIndexes[gridIndex], (gridIndex < 41 * 25 - 1) ? cellsStartIndexes[gridIndex + 1] : shoal->N,
                fishIdx, &count);
            ali = add(ali.x, ali.y, partAli.x, partAli.y);
        }
    }
    normalizeSteerVector(&ali, shoal, fishIdx, count);

    cudaVector coh(0, 0);
    count = 0;
    for (int j = fx > 0 ? fx - 1 : 0; j < 40 && j <= fx + 1; j++) {
        for (int k = fy > 0 ? fy - 1 : 0; k < 24 && j <= fy + 1; k++) {
            gridIndex = k * 41 + j;
            cudaVector partCoh(0, 0);
            partCoh = cohere(shoal, cellsStartIndexes[gridIndex], (gridIndex < 41 * 25 - 1) ? cellsStartIndexes[gridIndex + 1] : shoal->N,
                fishIdx, &count);
            coh = add(coh.x, coh.y, partCoh.x, partCoh.y);
        }
    }
    normalizeSteerVector(&coh, shoal, fishIdx, count);

    shoal->ax[fishIdx] = ali.x * alignmentWeight + coh.x * cohesionWeight + sep.x * separationWeight;
    shoal->ay[fishIdx] = ali.y * alignmentWeight + coh.y * cohesionWeight + sep.y * separationWeight;
}

__device__ void update(const cudaFishesParams* shoal, const int fishIdx) {
    const cudaVector velocity = add(shoal->vx[fishIdx], shoal->vy[fishIdx], shoal->ax[fishIdx], shoal->ay[fishIdx]);
    shoal->vx[fishIdx] = velocity.x;
    shoal->vy[fishIdx] = velocity.y;
    limit(&shoal->vx[fishIdx], &shoal->vy[fishIdx], maxVelocity);
    const cudaVector location = add(shoal->vx[fishIdx], shoal->vy[fishIdx], shoal->x[fishIdx], shoal->y[fishIdx]);
    shoal->x[fishIdx] = location.x;
    shoal->y[fishIdx] = location.y;
    shoal->ax[fishIdx] *= 0.0f;
    shoal->ay[fishIdx] *= 0.0f;
    borders(shoal, fishIdx);
}

__device__ void borders(const cudaFishesParams* shoal, const int fishIdx) {
    if (shoal->x[fishIdx] > 2000) {
        shoal->x[fishIdx] = 0;
    }
    else if (shoal->x[fishIdx] < 0) {
        shoal->x[fishIdx]= 2000;
    }
    if (shoal->y[fishIdx]> 1200) {
        shoal->y[fishIdx]= 0;
    }
    else if (shoal->y[fishIdx] < 0) {
        shoal->y[fishIdx] = 1200;
    }
}

__global__ void splitFishesOnGridKernel(const cudaFishesParams* shoal, int* gridIndexes) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    const int idxX = shoal->x[i] / 50;
    const int idxY = shoal->y[i] / 50;
    gridIndexes[i] = 41 * idxY + idxX;
}

__global__ void computeForceKernel(const cudaFishesParams* shoal, const int* gridIndexes, const int* cellsStartIndexes) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    int gridIndex = gridIndexes[i];
    const int fy = gridIndex / 41;
    const int fx = gridIndex % 41;

    flock(shoal, i, fx, fy, cellsStartIndexes);
}

__global__ void updateFishKernel(const cudaFishesParams* shoal) {
    const int i = blockIdx.x * blockDim.x + threadIdx.x;
    update(shoal, i);
}