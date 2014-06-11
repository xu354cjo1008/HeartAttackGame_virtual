//
//  kalmanFilter.c
//  
//
//  Created by 李勁璋 on 2014/2/8.
//
//

#include <stdio.h>
#include <stdlib.h>
#include "kalmanFilter.h"
#include <math.h>

/*

struct kalmanFilter *kalmanFilter_create()
{
    struct kalmanFilter *s;
    s = calloc(1, sizeof(*s));
	if (!s)
		return NULL;
    
	return s;
}

void kalmanInitialization(struct kalmanFilter * kalmanFilter, double initialState1, double initialState2)
{
    kalmanFilter->stateForwardMatrix[0][0] = 1;
    kalmanFilter->stateForwardMatrix[0][1] = 1;
    kalmanFilter->stateForwardMatrix[1][0] = 0;
    kalmanFilter->stateForwardMatrix[1][1] = 1;
    
    kalmanFilter->forceForwardMatrix[0][0] = 1;
    kalmanFilter->forceForwardMatrix[0][1] = 0;
    kalmanFilter->forceForwardMatrix[1][0] = 0;
    kalmanFilter->forceForwardMatrix[1][1] = 1;
    
    kalmanFilter->noiseCovariance[0][0] = 1;
    kalmanFilter->noiseCovariance[0][1] = 0;
    kalmanFilter->noiseCovariance[1][0] = 0;
    kalmanFilter->noiseCovariance[1][1] = 0.01;
    
    kalmanFilter->measurementNoiseCovariance[0][0] = 0;
    kalmanFilter->measurementNoiseCovariance[0][1] = 0;
    kalmanFilter->measurementNoiseCovariance[1][0] = 0;
    kalmanFilter->measurementNoiseCovariance[1][1] = 0;
    
    kalmanFilter->posterioriState[0] = initialState1;
    kalmanFilter->posterioriState[1] = initialState2;

    kalmanFilter->posterioriErrorCovariance[0][0] = 1;
    kalmanFilter->posterioriErrorCovariance[0][1] = 0;
    kalmanFilter->posterioriErrorCovariance[1][0] = 0;
    kalmanFilter->posterioriErrorCovariance[1][1] = 1;
    
    kalmanFilter->filterState = 0;
    
}
void kalmanPrediction(struct kalmanFilter * kalmanFilter)
{
    //state prediction
    kalmanFilter->prioriState[0] = kalmanFilter->stateForwardMatrix[0][0] * kalmanFilter->posterioriState[0] + kalmanFilter->stateForwardMatrix[0][1] * kalmanFilter->posterioriState[1] + kalmanFilter->forceForwardMatrix[0][0] * kalmanFilter->forceInput[0] + kalmanFilter->forceForwardMatrix[0][1] * kalmanFilter->forceInput[1];
    kalmanFilter->prioriState[1] = kalmanFilter->stateForwardMatrix[1][0] * kalmanFilter->posterioriState[0] + kalmanFilter->stateForwardMatrix[1][1] * kalmanFilter->posterioriState[1] + kalmanFilter->forceForwardMatrix[1][0] * kalmanFilter->forceInput[0] + kalmanFilter->forceForwardMatrix[1][1] * kalmanFilter->forceInput[1];
    //covarance prediction
    kalmanFilter->prioriErrorCovariance[0][0] = kalmanFilter->stateForwardMatrix[0][0] * kalmanFilter->stateForwardMatrix[0][0] *  kalmanFilter->posterioriErrorCovariance[0][0] + kalmanFilter->stateForwardMatrix[0][0] * kalmanFilter->stateForwardMatrix[0][1] * kalmanFilter->posterioriErrorCovariance[1][0] + kalmanFilter->stateForwardMatrix[0][0] * kalmanFilter->stateForwardMatrix[0][1] * kalmanFilter->posterioriErrorCovariance[0][1] + kalmanFilter->stateForwardMatrix[0][1] * kalmanFilter->stateForwardMatrix[0][1] * kalmanFilter->posterioriErrorCovariance[1][1] + kalmanFilter->noiseCovariance[0][0];
    kalmanFilter->prioriErrorCovariance[0][1] = kalmanFilter->stateForwardMatrix[0][0] * kalmanFilter->stateForwardMatrix[1][0] *  kalmanFilter->posterioriErrorCovariance[0][0] + kalmanFilter->stateForwardMatrix[0][1] * kalmanFilter->stateForwardMatrix[1][0] * kalmanFilter->posterioriErrorCovariance[1][0] + kalmanFilter->stateForwardMatrix[0][0] * kalmanFilter->stateForwardMatrix[1][1] * kalmanFilter->posterioriErrorCovariance[0][1] + kalmanFilter->stateForwardMatrix[0][1] * kalmanFilter->stateForwardMatrix[1][1] * kalmanFilter->posterioriErrorCovariance[1][1] + kalmanFilter->noiseCovariance[0][1];
    kalmanFilter->prioriErrorCovariance[1][0] = kalmanFilter->stateForwardMatrix[0][0] * kalmanFilter->stateForwardMatrix[1][0] *  kalmanFilter->posterioriErrorCovariance[0][0] + kalmanFilter->stateForwardMatrix[0][0] * kalmanFilter->stateForwardMatrix[1][1] * kalmanFilter->posterioriErrorCovariance[1][0] + kalmanFilter->stateForwardMatrix[0][1] * kalmanFilter->stateForwardMatrix[1][0] * kalmanFilter->posterioriErrorCovariance[0][1] + kalmanFilter->stateForwardMatrix[0][1] * kalmanFilter->stateForwardMatrix[1][1] * kalmanFilter->posterioriErrorCovariance[1][1] + kalmanFilter->noiseCovariance[1][0];
    kalmanFilter->prioriErrorCovariance[1][1] = kalmanFilter->stateForwardMatrix[1][0] * kalmanFilter->stateForwardMatrix[1][0] *  kalmanFilter->posterioriErrorCovariance[0][0] + kalmanFilter->stateForwardMatrix[1][0] * kalmanFilter->stateForwardMatrix[1][1] * kalmanFilter->posterioriErrorCovariance[1][0] + kalmanFilter->stateForwardMatrix[1][0] * kalmanFilter->stateForwardMatrix[1][1] * kalmanFilter->posterioriErrorCovariance[0][1] + kalmanFilter->stateForwardMatrix[1][1] * kalmanFilter->stateForwardMatrix[1][1] * kalmanFilter->posterioriErrorCovariance[1][1] + kalmanFilter->noiseCovariance[1][1];
}

void kalmanCorrection(struct kalmanFilter * kalmanFilter)
{
    //kalman gain
    double tempMatrix[2][2];
    double tempMatrixInv[2][2];
    tempMatrix[0][0] = kalmanFilter->estimationMatrix[0][0] * kalmanFilter->estimationMatrix[0][0] * kalmanFilter->prioriErrorCovariance[1][1] - kalmanFilter->estimationMatrix[0][0] * kalmanFilter->estimationMatrix[0][1] *kalmanFilter->prioriErrorCovariance[1][0] - kalmanFilter->estimationMatrix[0][0] * kalmanFilter->estimationMatrix[0][1] * kalmanFilter->prioriErrorCovariance[0][1] + kalmanFilter->estimationMatrix[0][1] * kalmanFilter->estimationMatrix[0][1] * kalmanFilter->prioriErrorCovariance[0][0];
    tempMatrix[0][1] = kalmanFilter->estimationMatrix[0][0] * kalmanFilter->estimationMatrix[1][0] * kalmanFilter->prioriErrorCovariance[1][1] - kalmanFilter->estimationMatrix[0][1] * kalmanFilter->estimationMatrix[1][0] *kalmanFilter->prioriErrorCovariance[1][1] - kalmanFilter->estimationMatrix[0][0] * kalmanFilter->estimationMatrix[1][1] * kalmanFilter->prioriErrorCovariance[0][1] + kalmanFilter->estimationMatrix[0][1] * kalmanFilter->estimationMatrix[1][1] * kalmanFilter->prioriErrorCovariance[0][0];
    tempMatrix[1][0] = kalmanFilter->estimationMatrix[0][0] * kalmanFilter->estimationMatrix[1][0] * kalmanFilter->prioriErrorCovariance[1][1] - kalmanFilter->estimationMatrix[0][0] * kalmanFilter->estimationMatrix[1][1] *kalmanFilter->prioriErrorCovariance[1][0] - kalmanFilter->estimationMatrix[0][1] * kalmanFilter->estimationMatrix[1][0] * kalmanFilter->prioriErrorCovariance[0][1] + kalmanFilter->estimationMatrix[0][1] * kalmanFilter->estimationMatrix[1][1] * kalmanFilter->prioriErrorCovariance[0][0];
    tempMatrix[1][1] = kalmanFilter->estimationMatrix[1][0] * kalmanFilter->estimationMatrix[1][0] * kalmanFilter->prioriErrorCovariance[1][1] - kalmanFilter->estimationMatrix[1][0] * kalmanFilter->estimationMatrix[1][1] *kalmanFilter->prioriErrorCovariance[1][0] - kalmanFilter->estimationMatrix[1][0] * kalmanFilter->estimationMatrix[1][1] * kalmanFilter->prioriErrorCovariance[0][1] + kalmanFilter->estimationMatrix[1][1] * kalmanFilter->estimationMatrix[1][1] * kalmanFilter->prioriErrorCovariance[0][0];
    tempMatrix[0][0] = tempMatrix[0][0]/(kalmanFilter->prioriErrorCovariance[0][0] * kalmanFilter->prioriErrorCovariance[1][1] - kalmanFilter->prioriErrorCovariance[0][1] * kalmanFilter->prioriErrorCovariance[1][0]);
    tempMatrix[0][1] = tempMatrix[0][1]/(kalmanFilter->prioriErrorCovariance[0][0] * kalmanFilter->prioriErrorCovariance[1][1] - kalmanFilter->prioriErrorCovariance[0][1] * kalmanFilter->prioriErrorCovariance[1][0]);
    tempMatrix[1][0] = tempMatrix[1][0]/(kalmanFilter->prioriErrorCovariance[0][0] * kalmanFilter->prioriErrorCovariance[1][1] - kalmanFilter->prioriErrorCovariance[0][1] * kalmanFilter->prioriErrorCovariance[1][0]);
    tempMatrix[1][1] = tempMatrix[1][1]/(kalmanFilter->prioriErrorCovariance[0][0] * kalmanFilter->prioriErrorCovariance[1][1] - kalmanFilter->prioriErrorCovariance[0][1] * kalmanFilter->prioriErrorCovariance[1][0]);
    
    tempMatrix[0][0] = tempMatrix[0][0] + kalmanFilter->measurementNoiseCovariance[0][0];
    tempMatrix[0][1] = tempMatrix[0][1] + kalmanFilter->measurementNoiseCovariance[0][1];
    tempMatrix[1][0] = tempMatrix[1][0] + kalmanFilter->measurementNoiseCovariance[1][0];
    tempMatrix[1][1] = tempMatrix[1][1] + kalmanFilter->measurementNoiseCovariance[1][1];

    tempMatrixInv[0][0] = tempMatrix[1][1]/(tempMatrix[0][0] * tempMatrix[1][1] - tempMatrix[0][1] * tempMatrix[1][0]);
    tempMatrixInv[0][1] = -tempMatrix[0][1]/(tempMatrix[0][0] * tempMatrix[1][1] - tempMatrix[0][1] * tempMatrix[1][0]);
    tempMatrixInv[1][0] = -tempMatrix[1][0]/(tempMatrix[0][0] * tempMatrix[1][1] - tempMatrix[0][1] * tempMatrix[1][0]);
    tempMatrixInv[1][1] = tempMatrix[0][0]/(tempMatrix[0][0] * tempMatrix[1][1] - tempMatrix[0][1] * tempMatrix[1][0]);
    
    kalmanFilter->kalmanGain[0][0] = kalmanFilter->prioriErrorCovariance[0][0] * kalmanFilter->estimationMatrix[0][0] * tempMatrixInv[0][0] + kalmanFilter->prioriErrorCovariance[0][1] * kalmanFilter->estimationMatrix[0][1] * tempMatrixInv[0][0] + kalmanFilter->prioriErrorCovariance[0][0] * kalmanFilter->estimationMatrix[1][0] * tempMatrixInv[1][0] + kalmanFilter->prioriErrorCovariance[0][1] * kalmanFilter->estimationMatrix[1][1] * tempMatrixInv[1][0];
    kalmanFilter->kalmanGain[0][1] = kalmanFilter->prioriErrorCovariance[0][0] * kalmanFilter->estimationMatrix[0][0] * tempMatrixInv[0][1] + kalmanFilter->prioriErrorCovariance[0][1] * kalmanFilter->estimationMatrix[0][1] * tempMatrixInv[0][1] + kalmanFilter->prioriErrorCovariance[0][0] * kalmanFilter->estimationMatrix[1][0] * tempMatrixInv[1][1] + kalmanFilter->prioriErrorCovariance[0][1] * kalmanFilter->estimationMatrix[1][1] * tempMatrixInv[1][1];
    kalmanFilter->kalmanGain[1][0] = kalmanFilter->prioriErrorCovariance[1][0] * kalmanFilter->estimationMatrix[0][0] * tempMatrixInv[0][0] + kalmanFilter->prioriErrorCovariance[1][1] * kalmanFilter->estimationMatrix[0][1] * tempMatrixInv[0][0] + kalmanFilter->prioriErrorCovariance[1][0] * kalmanFilter->estimationMatrix[1][0] * tempMatrixInv[1][0] + kalmanFilter->prioriErrorCovariance[1][1] * kalmanFilter->estimationMatrix[1][1] * tempMatrixInv[1][0];
    kalmanFilter->kalmanGain[1][1] = kalmanFilter->prioriErrorCovariance[1][1] * kalmanFilter->estimationMatrix[0][0] * tempMatrixInv[0][1] + kalmanFilter->prioriErrorCovariance[1][1] * kalmanFilter->estimationMatrix[0][1] * tempMatrixInv[0][1] + kalmanFilter->prioriErrorCovariance[1][0] * kalmanFilter->estimationMatrix[1][0] * tempMatrixInv[1][1] + kalmanFilter->prioriErrorCovariance[1][1] * kalmanFilter->estimationMatrix[1][1] * tempMatrixInv[1][1];
    
    //posteriori state
    double tempVector[2];
    double tempVector2[2];

    tempVector[0] = kalmanFilter->measurementState[0] - (kalmanFilter->estimationMatrix[0][0] * kalmanFilter->prioriState[0] + kalmanFilter->estimationMatrix[0][1] * kalmanFilter->prioriState[1]);
    tempVector[1] = kalmanFilter->measurementState[1] - (kalmanFilter->estimationMatrix[1][0] * kalmanFilter->prioriState[0] + kalmanFilter->estimationMatrix[1][1] * kalmanFilter->prioriState[1]);
    tempVector2[0] = kalmanFilter->kalmanGain[0][0] * tempVector[0] + kalmanFilter->kalmanGain[0][1] * tempVector[1];
    tempVector2[1] = kalmanFilter->kalmanGain[1][0] * tempVector[0] + kalmanFilter->kalmanGain[1][1] * tempVector[1];

    kalmanFilter->posterioriState[0] = kalmanFilter->prioriState[0] + tempVector2[0];
    kalmanFilter->posterioriState[1] = kalmanFilter->prioriState[1] + tempVector2[1];
    
    //posteriori covariance
    tempMatrix[0][0] = 1 - kalmanFilter->kalmanGain[0][0] * kalmanFilter->estimationMatrix[0][0] - kalmanFilter->kalmanGain[0][1] * kalmanFilter->estimationMatrix[1][0];
    tempMatrix[0][1] = - kalmanFilter->kalmanGain[0][0] * kalmanFilter->estimationMatrix[0][1] - kalmanFilter->kalmanGain[0][1] * kalmanFilter->estimationMatrix[1][1];
    tempMatrix[1][0] = - kalmanFilter->kalmanGain[1][0] * kalmanFilter->estimationMatrix[0][0] - kalmanFilter->kalmanGain[1][1] * kalmanFilter->estimationMatrix[1][0];
    tempMatrix[1][1] = 1 - kalmanFilter->kalmanGain[1][0] * kalmanFilter->estimationMatrix[0][1] - kalmanFilter->kalmanGain[1][1] * kalmanFilter->estimationMatrix[1][1];
    
    kalmanFilter->posterioriErrorCovariance[0][0] = tempMatrix[0][0] * kalmanFilter->prioriErrorCovariance[0][0] + tempMatrix[0][1] * kalmanFilter->prioriErrorCovariance[1][0];
    kalmanFilter->posterioriErrorCovariance[0][1] = tempMatrix[0][0] * kalmanFilter->prioriErrorCovariance[0][1] + tempMatrix[0][1] * kalmanFilter->prioriErrorCovariance[1][1];
    kalmanFilter->posterioriErrorCovariance[1][0] = tempMatrix[1][0] * kalmanFilter->prioriErrorCovariance[0][0] + tempMatrix[1][1] * kalmanFilter->prioriErrorCovariance[1][0];
    kalmanFilter->posterioriErrorCovariance[1][1] = tempMatrix[1][0] * kalmanFilter->prioriErrorCovariance[0][1] + tempMatrix[1][1] * kalmanFilter->prioriErrorCovariance[1][1];
}

double kalmanSample(struct kalmanFilter *kalmanFilter, double measurementData, double ingress)
{
    double interval = ingress - (kalmanFilter->localTime - kalmanFilter->forceInput[0]);
    switch (kalmanFilter->filterState) {
        case 0:
            kalmanFilter->localTime = ingress;
            kalmanFilter->measurementState[0] = measurementData;
            kalmanFilter->posterioriState[0] = measurementData;
            kalmanFilter->filterState = 1;
            break;
        case 1:
            if (interval < 0) {
                kalmanFilter->filterState = 0;
                return 0;
            }
            kalmanFilter->measurementState[1] = (measurementData - kalmanFilter->measurementState[0])/interval;
            kalmanFilter->measurementState[0] = measurementData;
            kalmanFilter->localTime = ingress ;
            kalmanFilter->stateForwardMatrix[0][1] = interval;
            
            kalmanPrediction(kalmanFilter);
            kalmanCorrection(kalmanFilter);
            break;
        default:
            break;
    }

    return kalmanFilter->posterioriState[0];
}
*/

struct kalmanFilter *kalmanFilter_create()
{
    struct kalmanFilter *s;
    s = calloc(1, sizeof(*s));
	if (!s)
		return NULL;
    s->stateForwardMatrix = matrix_create(2, 2);
    s->forceForwardMatrix = matrix_create(2, 2);
    s->noiseCovariance = matrix_create(2, 2);
    s->estimationMatrix = matrix_create(2, 2);
    s->measurementNoiseCovariance = matrix_create(2, 2);
    
    
    s->prioriState = matrix_create(2, 1);
    s->posterioriState = matrix_create(2, 1);
    
    s->forceInput = matrix_create(2, 1);
    
    s->measurementState = matrix_create(2, 1);
    
    s->prioriErrorCovariance = matrix_create(2, 2);
    s->posterioriErrorCovariance = matrix_create(2, 2);
    
    s->kalmanGain = matrix_create(2, 2);
    
	return s;
}

void kalmanInitialization(struct kalmanFilter * kalmanFilter, double initialState1, double initialState2)
{
    M(kalmanFilter->stateForwardMatrix,0,0) = 1;
    M(kalmanFilter->stateForwardMatrix,0,1) = 1;
    M(kalmanFilter->stateForwardMatrix,1,0) = 0;
    M(kalmanFilter->stateForwardMatrix,1,1) = 1;

    M(kalmanFilter->forceForwardMatrix,0,0) = 1;
    M(kalmanFilter->forceForwardMatrix,0,1) = 0;
    M(kalmanFilter->forceForwardMatrix,1,0) = 0;
    M(kalmanFilter->forceForwardMatrix,1,1) = 1;

    M(kalmanFilter->noiseCovariance,0,0) = 0.000001;
    M(kalmanFilter->noiseCovariance,0,1) = 0;
    M(kalmanFilter->noiseCovariance,1,0) = 0;
    M(kalmanFilter->noiseCovariance,1,1) = 0.0000000001;
    
    M(kalmanFilter->measurementNoiseCovariance,0,0) = 0.0004;
    M(kalmanFilter->measurementNoiseCovariance,0,1) = 0;
    M(kalmanFilter->measurementNoiseCovariance,1,0) = 0;
    M(kalmanFilter->measurementNoiseCovariance,1,1) = 0.0004;
    
    M(kalmanFilter->estimationMatrix,0,0) = 1;
    M(kalmanFilter->estimationMatrix,0,1) = 0;
    M(kalmanFilter->estimationMatrix,1,0) = 0;
    M(kalmanFilter->estimationMatrix,1,1) = 1;
    
    M(kalmanFilter->posterioriState,0,0) = initialState1;
    M(kalmanFilter->posterioriState,1,0) = initialState2;
    
    M(kalmanFilter->posterioriErrorCovariance,0,0) = 1;
    M(kalmanFilter->posterioriErrorCovariance,0,1) = 0;
    M(kalmanFilter->posterioriErrorCovariance,1,0) = 0;
    M(kalmanFilter->posterioriErrorCovariance,1,1) = 1;
    
    kalmanFilter->filterState = 0;
    
}
void kalmanPrediction(struct kalmanFilter * kalmanFilter)
{
    //state prediction
    kalmanFilter->prioriState = matrix_add(matrix_multiple(kalmanFilter->stateForwardMatrix, kalmanFilter->posterioriState), matrix_multiple(kalmanFilter->forceForwardMatrix, kalmanFilter->forceInput));
    
    //covarance prediction
    Matrix *tempMatrix = matrix_create(kalmanFilter->prioriErrorCovariance->rows, kalmanFilter->prioriErrorCovariance->cols);

    tempMatrix = matrix_multiple(kalmanFilter->posterioriErrorCovariance, matrix_transport(kalmanFilter->stateForwardMatrix));
    tempMatrix = matrix_multiple(kalmanFilter->stateForwardMatrix, tempMatrix);
    
    kalmanFilter->prioriErrorCovariance = matrix_add(tempMatrix, kalmanFilter->noiseCovariance);
    
}

void kalmanCorrection(struct kalmanFilter * kalmanFilter)
{
    //kalman gain

    Matrix * tempMatrix = matrix_create(kalmanFilter->prioriErrorCovariance->rows, kalmanFilter->prioriErrorCovariance->cols);
    
    tempMatrix = matrix_multiple(kalmanFilter->prioriErrorCovariance, matrix_transport(kalmanFilter->estimationMatrix));
    tempMatrix = matrix_multiple(kalmanFilter->estimationMatrix, tempMatrix);

    tempMatrix = matrix_add(tempMatrix, kalmanFilter->measurementNoiseCovariance);

    tempMatrix = matrix_inverse_second_orders(tempMatrix);

    tempMatrix = matrix_multiple(matrix_transport(kalmanFilter->estimationMatrix), tempMatrix);

    tempMatrix = matrix_multiple(kalmanFilter->prioriErrorCovariance, tempMatrix);

    kalmanFilter->kalmanGain = tempMatrix;

    //posteriori state
    
    Matrix *tempVector = matrix_create(kalmanFilter->posterioriState->rows, kalmanFilter->posterioriState->cols);
    
    tempVector = matrix_multiple(kalmanFilter->estimationMatrix, kalmanFilter->prioriState);
    tempVector = matrix_min(kalmanFilter->measurementState, tempVector);
    tempVector = matrix_multiple(kalmanFilter->kalmanGain, tempVector);
    tempVector = matrix_add(kalmanFilter->prioriState, tempVector);
    
    kalmanFilter->posterioriState = tempVector;

    //posteriori covariance
    Matrix *iMatrix = matrix_identity(kalmanFilter->posterioriErrorCovariance->rows);
    
    tempMatrix = matrix_min(iMatrix, matrix_multiple(kalmanFilter->kalmanGain, kalmanFilter->estimationMatrix));
    tempMatrix = matrix_multiple(tempMatrix, kalmanFilter->prioriErrorCovariance);
    
    kalmanFilter->posterioriErrorCovariance = tempMatrix;
}

double kalmanSample(struct kalmanFilter *kalmanFilter, double measurementData, double ingress)
{
    double interval = ingress - (kalmanFilter->localTime + M(kalmanFilter->forceInput,0,0));
    switch (kalmanFilter->filterState) {
        case 0:
            kalmanFilter->localTime = ingress;
            M(kalmanFilter->measurementState,0,0) = measurementData;
            M(kalmanFilter->posterioriState,0,0) = measurementData;
            kalmanFilter->filterState = 1;
            break;
        case 1:
            if (interval < 0) {
                kalmanFilter->filterState = 0;
                return 0;
            }
            M(kalmanFilter->measurementState,1,0) = 1000000000*(measurementData - (M(kalmanFilter->measurementState,0,0) + M(kalmanFilter->forceInput,0,0)))/interval;
            M(kalmanFilter->measurementState,0,0) = measurementData;
            kalmanFilter->localTime = ingress ;
            M(kalmanFilter->stateForwardMatrix,0,1) = interval/1000000000;
            
            kalmanPrediction(kalmanFilter);
            kalmanCorrection(kalmanFilter);
            break;
        default:
            break;
    }
    return M(kalmanFilter->posterioriState,0,0);
}
