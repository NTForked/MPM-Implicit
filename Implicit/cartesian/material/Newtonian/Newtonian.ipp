// *********************************************************************************
// File: Bingham.hpp
//
// Details: Bingham Material Model
//
// Dependency: MaterialBase, ParticleCloudMpm and MpmItems
//
// Author: Krishna Kumar, University of Cambridge
//
// Version: 1.0 - 2013
// *********************************************************************************

#include <corlib/linalg.hpp>
#include <cmath>

mpm::material::Newtonian::Newtonian() : MaterialBaseMpm() {
    registerVariable("youngModulus", youngModulus_);
    registerVariable("poissonRatio", poissonRatio_);
    registerVariable("density",  density_);
    registerVariable("tau0",  tau0_);
    registerVariable("mu",  mu_);
    registerVariable("strainCutoff", strainCutoff_);
}

mpm::material::Newtonian::~Newtonian() {}


// Compute the elasticity tensor
// \param[out] De  Elasticity tensor

void mpm::material::Newtonian::elasticityTensor(Mat6x6_ & De) const {
    // Bulk and shear modulus
    double K, G;
    double a1, a2;

    K = youngModulus_ / (3.0 * (1. - 2. * poissonRatio_));
    G = youngModulus_ / (2.0 * (1. + poissonRatio_));


    a1 = K + (4.0 / 3.0) * G;
    a2 = K - (2.0 / 3.0) * G;

    // compute elasticityTensor
    De(0,0)=a1;    De(0,1)=a2;    De(0,2)=a2;    De(0,3)=0;    De(0,4)=0;    De(0,5)=0;
    De(1,0)=a2;    De(1,1)=a1;    De(1,2)=a2;    De(1,3)=0;    De(1,4)=0;    De(1,5)=0;
    De(2,0)=a2;    De(2,1)=a2;    De(2,2)=a1;    De(2,3)=0;    De(2,4)=0;    De(2,5)=0;
    De(3,0)= 0;    De(3,1)= 0;    De(3,2)= 0;    De(3,3)=G;    De(3,4)=0;    De(3,5)=0;
    De(4,0)= 0;    De(4,1)= 0;    De(4,2)= 0;    De(4,3)=0;    De(4,4)=G;    De(4,5)=0;
    De(5,0)= 0;    De(5,1)= 0;    De(5,2)= 0;    De(5,3)=0;    De(5,4)=0;    De(5,5)=G;

    return;
}


// Compute the stress vector 2D
// \param[in]  F dstrain vector
// \param[out] S stress vector

void mpm::material::Newtonian::computeStress(const Vec6x1_& F, double& P, Vec6x1_& S, const ParticleCloudSoilPtr pCloudSoilPtr, const unsigned id) {

    ParticleSoilPtr pSoilPtr = pCloudSoilPtr -> giveParticlePtr(id);

    Mat6x6_ De;
    Vec6x1_ dS;
    Vec3x1_ Tau;
    Mat2x2_ VelGrad =  pSoilPtr -> getStrainRateSoilParticle();  // strainRate_

    double pOld = (S(0) + S(1) + S(2)) / 3.0;
    double dP = youngModulus_ / (3.0 * (1. - 2. * poissonRatio_)) * (F(0) + F(1) + F(2));
    double pNew = pOld + dP;

    double I2 = 0.5 * ((VelGrad(0,0) * VelGrad(0,0)) + (VelGrad(1,0) * VelGrad(1,0)) + (VelGrad(0,1) * VelGrad(0,1)) + (VelGrad(1,1) * VelGrad(1,1)));

    double factor;
    if (strainCutoff_ < 1.E-15)
        strainCutoff_ = 1.0E-15;
    if (std::sqrt(I2) > strainCutoff_)
        factor =  ((tau0_ / (sqrt(I2))) + 2 * mu_);
    else
        factor = 0.;

    double StrainRateI2 = std::sqrt(I2);
    pSoilPtr -> setStrainRateI2SoilParticle(StrainRateI2);

    Tau(0) = factor * VelGrad(0,0);
    Tau(1) = factor * VelGrad(1,1);
    Tau(2) = 0.;

    double traceTau2 = 0.5 * (Tau(0) * Tau(0) + Tau(1) * Tau(1) + Tau(2) * Tau(2));

    // Considered as Isotropic Linear Elastic 
    if (traceTau2 < (tau0_ * tau0_)) {
        ublas::noalias(dS) = ublas::prod(De, F);
        S += dS;
    }
    else {
        S(0) = Tau(0) + pNew;
        S(1) = Tau(1) + pNew;
        S(2) = Tau(2) + 0.;
        S(3) = factor * VelGrad(0,1);
        S(4) = factor * 0.;
        S(5) = factor * 0.;
    }

    return;
}


// Compute the stress vector 3D
// \param[in]  F dstrain vector
// \param[out] S stress vector

void mpm::material::Newtonian::computeStress3D(const Vec6x1_& F, double& P, Vec6x1_& S, const ParticleCloudSoilPtr pCloudSoilPtr, const unsigned id) {

    ParticleSoilPtr pSoilPtr = pCloudSoilPtr->giveParticlePtr(id);

    Mat6x6_ De;
    Vec6x1_ dS;
    Mat3x3_ VelGrad = pSoilPtr->getStrainRateSoilParticle();  // strainRate_
    Vec3x1_ Tau;
    double pOld = (S(0) + S(1) + S(2)) / 3.0;
    double dP = youngModulus_ / (3.0 * (1. - 2. * poissonRatio_)) * (F(0)+F(1)+F(2));
    double pNew = pOld + dP;

    double I2 = 0.5*((VelGrad(0,0) * VelGrad(0,0)) + (VelGrad(0,1) * VelGrad(0,1)) + (VelGrad(0,2) * VelGrad(0,2)) + (VelGrad(1,0) * VelGrad(1,0)) + (VelGrad(1,1) * VelGrad(1,1)) + (VelGrad(1,2) * VelGrad(1,2)) + (VelGrad(2,0) * VelGrad(2,0)) + (VelGrad(2,1) * VelGrad(2,1)) + (VelGrad(2,2) * VelGrad(2,2)));
    double factor;

//    if (strainCutoff_ < 1.E-15)
//            strainCutoff_ = 1.0E-15;
//    if (std::sqrt(I2) > strainCutoff_)
    if(std::sqrt(I2) > 1.0E-15)
        factor =  ((tau0_ / (sqrt(I2))) + 2 * mu_);
    else
        factor = 0.;

    double StrainRateI2 = std::sqrt(I2);
    pSoilPtr -> setStrainRateI2SoilParticle(StrainRateI2);

    Tau(0) = factor * VelGrad(0,0);
    Tau(1) = factor * VelGrad(1,1);
    Tau(2) = factor * VelGrad(2,2);
// Tau(3) = factor * VelGrad(0,1);
// Tau(4) = factor * VelGrad(1,2);
// Tau(5) = factor * VelGrad(0,2);


    double traceTau2 = 0.5* (Tau(0) * Tau(0) + Tau(1) * Tau(1) + Tau(2) * Tau(2));// + 2*Tau(3)*Tau(3) + 2*Tau(4)*Tau(4) + 2*Tau(5)*Tau(5));

    if (traceTau2 < (tau0_ * tau0_)) {
        ublas::noalias(dS) = ublas::prod(De, F);
        S += dS;
    }
    else {
        S(0) = Tau(0) + pNew;
        S(1) = Tau(1) + pNew;
        S(2) = Tau(2) + pNew;
        S(3) = factor * VelGrad(0,1);
        S(4) = factor * VelGrad(1,2);
        S(5) = factor * VelGrad(0,2);
        //      std::cout << "Plastic" << std::endl;
    }
    return;
}


double mpm::material::Newtonian::sign_(double var) {
    double a;
    if (var > 0.0) a = 1.;
    if (var < 0.0) a = -1.;
    return a;
}
