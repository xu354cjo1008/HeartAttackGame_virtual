//
//  kalmanFilter.h
//  
//
//  Created by 李勁璋 on 2014/2/8.
//
//
// x(n) = Ax(n-1)+ Bu(n-1) + w(n-1)
//
// A = [  1  t  ]
//     [  0  1  ]
// B = [ -1 -t  ]
//     [  0 -1  ]
//
// z(n) = Hx(n) + v
//
// H = [  1  0  ]
//     [  0  1  ]
//
//
//
//
//
//
//
//
#ifndef _kalmanFilter_h
#define _kalmanFilter_h
#include "matrix.h"

struct kalmanFilter {

   /* double stateForwardMatrix[2][2];
    double forceForwardMatrix[2][2];
    double noiseCovariance[2][2];
    double estimationMatrix[2][2];
    double measurementNoiseCovariance[2][2];
    
    
    double prioriState[2];
    double posterioriState[2];
    
    double forceInput[2];
    
    double measurementState[2];
    
    double prioriErrorCovariance[2][2];
    double posterioriErrorCovariance[2][2];

    double kalmanGain[2][2];*/
    
    Matrix *stateForwardMatrix;
    Matrix *forceForwardMatrix;
    Matrix *noiseCovariance;
    Matrix *estimationMatrix;
    Matrix *measurementNoiseCovariance;
    
    
    Matrix *prioriState;
    Matrix *posterioriState;
    
    Matrix *forceInput;
    
    Matrix *measurementState;
    
    Matrix *prioriErrorCovariance;
    Matrix *posterioriErrorCovariance;
    
    Matrix *kalmanGain;
    
    double localTime;
    
    int filterState;

};

struct kalmanFilter *kalmanFilter_create();

void kalmanInitialization(struct kalmanFilter * kalmanFilter, double initialState1, double initialState2);

double kalmanSample(struct kalmanFilter *kalmanFilter, double measurementData, double ingress);

#endif
