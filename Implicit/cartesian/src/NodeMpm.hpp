// *********************************************************************************
// File: NodeMpm.hpp
//
// Details: Class for nodes in MPM. Add/give elements and near-by partickes
//
// Dependency: MpmItems.hpp for template parameters
//
// Author: Krishna Kumar and Samila Bandara, University of Cambridge
//
// Version: 1.0 - Samila Bandara
// *********************************************************************************

#ifndef CARTESIAN_SRC_NODEMPM_H_
#define CARTESIAN_SRC_NODEMPM_H_

#include <cassert>
#include <iostream>
#include <set>
#include <utility>
#include <vector>

#include <boost/array.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include "corlib/NodeBasic.hpp"
#include "cartesian/src/MpmItems.hpp"

namespace mpm {
template<typename MPM_ITEMS>
class NodeMpm;
}

// \brief class for nodes in MPM
// \details add/give elements and near-by particles
// \tparam MPM_ITEMS  Contains all the template parameters

template<typename MPM_ITEMS>
class mpm::NodeMpm : public corlib::NodeBasic<MPM_ITEMS::dim> {
public:
    typedef MPM_ITEMS MpmItems;

    static const unsigned dof = MpmItems::dof;
    static const unsigned dim = MpmItems::dim;

    typedef typename MpmItems::NodeHandle NodeType;
    typedef typename MpmItems::ElementHandle ElementType;
    typedef typename MpmItems::ParticleHandle ParticleType;

    typedef typename MpmItems::NodePtr NodePtr;
    typedef typename MpmItems::ElementPtr ElementPtr;
    typedef typename MpmItems::ParticlePtr ParticlePtr;

    typedef mpm::CompareFunctor<ElementPtr> ElementCompare;
    typedef std::set<ElementPtr, ElementCompare> ElementSet;
    typedef mpm::CompareFunctor<NodePtr> NodeCompare;
    typedef std::set<NodePtr, NodeCompare> NodeSet;
    typedef mpm::CompareFunctor<ParticlePtr> ParticleCompare;
    typedef std::set<ParticlePtr, ParticleCompare> ParticleSet;

    typedef boost::numeric::ublas::bounded_vector<double, dim> VecDim;
    typedef boost::numeric::ublas::bounded_vector<double, dof> VecDof;

    // Constructor with id number
    NodeMpm(const unsigned& id);

    // Constructor with id and coordinates
    NodeMpm(const unsigned& id, const VecDim& cv) : NodeBasic_(id, cv) { }

protected:
    typedef corlib::NodeBasic<dim> NodeBasic_;

private:
    typedef std::vector<unsigned> ConstraintGenVec_;
    typedef std::vector<double> ConstraintGenSlopeVec_;
    typedef std::vector<std::pair<unsigned, int>> ConstraintFricVec_;
    typedef std::vector<std::pair<double, int>> ConstraintFricSlopeVec_;

    typedef ublas::bounded_matrix<unsigned, 3, 2> Mat3x2_;

//------------------------------------------------------------------------------
public:
    // check values
    void printCheckNode() {
        std::cout << id_ << ", mass= " << massSoil_ << ", intForce= " << intForceSoil_<< "\n";
    }

//------------------------------------------------------------------------------

    // assign nodal mass contribution from each soil particle
    void assignSoilMass(double nMass) {
        massSoil_ += nMass;
    }

    // assign nodal momentum contribution from each soil particle
    void assignSoilMomentum(VecDof nMomentum) {
        momentumSoil_ += nMomentum;
    }

    // assign nodal external/body force contribution from each soil particle
    void assignExternalForce(VecDof nExtForce) {
        extForceSoil_ += nExtForce;
    }

    // assign pressure force to node
    void assignPressureForce(VecDof nPressureForce) {
        presForceSoil_ += nPressureForce;
    }

    // assign nodal internal force contribution from each soil particle
    void assignInternalForce(VecDof nIntForce) {
        intForceSoil_ -= nIntForce;
    }

//------------------------------------------------------------------------------

    // calculate velocity for soil
    void computeSoilVelocity();

    // calculate velocity for soil for mesh with slope & horizontal boundaries
    void computeSoilVelocityMixedMesh();

    // from Implicit--update the nodal velocity
    void updateNodalVelocity(VecDof& velocity);

//------------------------------------------------------------------------------

    // give velocity for soil
    VecDof giveSoilVelocity() {
        return velocitySoil_;
    }

    // give acceleration for soil
    VecDof giveSoilAcceleration() {
        return accelerationSoil_;
    }

    // give lumped mass of nodes
    double giveLumpedMassAtNodes() {
        return massSoil_;
    }

    // give pressure force at nodes
    VecDof givePressureTermAtNodes() {
        return presForceSoil_;
    }

    // give external force at nodes
    VecDof giveExtForceTermAtNodes() {
        return extForceSoil_;
    }


//------------------------------------------------------------------------------
         // calculate acceleration for soil and update velocity

    void computeSoilAccelerationAndVelocity(const double dt, const double miu);

    // calculate acceleration for soil and update velocity for slope & hor. bound.
    void computeSoilAccelerationAndVelocityMixedMesh(const double dt, const double miu);

//------------------------------------------------------------------------------
                 // Store constraints for explicit MPM
              // store the general constraints directions (0/1/2) in a vector

    void storeGeneralConstraint(const unsigned& direction);

    // store friction constraints normal direction (0/1/2) & sign (-1/1) in a vector
    void storeFrictionConstraint(const unsigned& normDirection, const int& signNormDirection);

    // store the slope angle for general constraints for sloped mesh
    void storeGeneralConstraintSlopeBn(const double& slopeAngle);

    // store the slope angle and sign (-1/1) for friction constraints for sloped mesh
    void storeFrictionConstraintSlopeBn(const double& slopeAngle, const int& signNormDirection);

//------------------------------------------------------------------------------              // intialise data and pointers to particles

    void initialise();

//------------------------------------------------------------------------------
           // add element pointer where the node is located to a set

    void addElement(ElementPtr ePtr) {
        elementSet_.insert(ePtr);
    }

    // retun element set
    ElementSet giveElements() {
        return elementSet_;
    }

//------------------------------------------------------------------------------
    // add other connected nodes which only containes particles
    void addNode(NodePtr nPtr) {
        nodeSet_.insert(nPtr);
    }

    NodeSet giveNodeSet() {
        return nodeSet_;
    }

//------------------------------------------------------------------------------

    // NOT IN USE: add particles that refer this node
    void addParticle(ParticlePtr pPtr) {
        particleSet_.insert(pPtr);
    }

    // NOT IN USE: give particle set which refer this node
    ElementSet giveParticles() {
        return particleSet_;
    }

//------------------------------------------------------------------------------

protected:
    // Apply constraints for explicit MPM
    // apply general constraints normal to 0/1/2 direction boundaries for V (v OR a)
    void applyGeneralConstraints_(VecDof& V);

    // apply friction constraints to limit acceleration (0/1/2 direction boundaries)
    void applyFrictionConstraints_(const double dt, const double miu);

    // apply general constraints to slope boundary for V (v OR a)
    void applyGeneralConstraintsSlope_(VecDof& V);

    // apply friction constraints to limit acceleration (slope boundary)
    void applyFrictionConstraintsSlope_(const double dt, const double miu);

    // send sign
    double sign_(double var);

    using NodeBasic_::id_;
    using NodeBasic_::coord_;
    ConstraintGenVec_ genConstraints_;              // direction for general constraints
    ConstraintFricVec_ fricConstraints_;            // direction and sign for friction constraints
    ConstraintGenSlopeVec_ genConstraintsSlope_;    // slope angle for general constraints
    ConstraintFricSlopeVec_ fricConstraintsSlope_;  // slope angle and sign for friction constraint
    double massSoil_;            // nodal mass for soil
    VecDof velocitySoil_;        // velocity for soil
    VecDof momentumSoil_;        // momentum for soil
    VecDof extForceSoil_;        // external force for soil
    VecDof presForceSoil_;
    VecDof intForceSoil_;        // external force for soil
    VecDof accelerationSoil_;    // acceleration for soil
    ElementSet elementSet_;      // not in use
    NodeSet nodeSet_;
    ParticleSet particleSet_;    // not in use
};

#include "NodeMpm.ipp"

#endif  // CARTESIAN_SRC_NODEMPM_H_
