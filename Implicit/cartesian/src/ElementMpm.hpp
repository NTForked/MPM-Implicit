// *********************************************************************************
// File: ElementMpm.hpp
//
// Details: Element Template for MPM. Has access to its nodes,
//          which can be set, returned and accesses
//          writes connectivity and computes the size
//
// Dependency: MPM_ITEMS contains all template parameters
//
// Author: Samila Bandara, University of Cambridge
//
// Version: 1.0
// *********************************************************************************

#ifndef CARTESIAN_SRC_ELEMENTMPM_H_
#define CARTESIAN_SRC_ELEMENTMPM_H_

#include <cassert>
#include <iostream>
#include <set>
#include <vector>

#include <boost/array.hpp>
#include <boost/numeric/ublas/vector.hpp>

#include "cartesian/src/MpmItems.hpp"
#include "corlib/ElementBasic.hpp"
#include "cartesian/src/CompareFunctor.hpp"
#include "corlib/Shape.hpp"

namespace mpm {
template<typename MPM_ITEMS>
class ElementMpm;
namespace ublas = boost::numeric::ublas;
}

// *********************************************************************************
// \brief Element template for MPM
// \details  Has pointers to its nodes which can be set, returned
// and accessed. Can write its connectivity and compute a size measure.
// \tparam MPM_ITEMS  Contains all the template parameters

template<typename MPM_ITEMS>
class mpm::ElementMpm : public corlib::ElementBasic<typename MPM_ITEMS::NodeHandle, MPM_ITEMS::nNodes, MPM_ITEMS::myShape> {
public:
    typedef MPM_ITEMS MpmItems;

    static const unsigned dim = MpmItems::dim;
    static const unsigned numNodes = MpmItems::nNodes;
    static const corlib::shape myShape = MpmItems::myShape;

    typedef typename MpmItems::NodeHandle NodeType;
    typedef typename MpmItems::ParticleHandle ParticleType;

    typedef typename MpmItems::NodePtr NodePtr;
    typedef typename MpmItems::ParticleSoilPtr ParticleSoilPtr;
    typedef typename MpmItems::ParticlePtr ParticlePtr;

    typedef ublas::bounded_matrix<double, dim, numNodes> MatDimNV;
    typedef ublas::bounded_vector<double, dim> VecDim;

protected:
    typedef corlib::ElementBasic<NodeType, numNodes, myShape> ElementBasic_;

    typedef boost::array<NodePtr, numNodes> NodePtrArray_;
    typedef std::vector<NodePtr> NodeVec_;
    typedef std::vector<ParticleSoilPtr> ParticleSoilVec_;
//---------------------------------------------------------------------------

public:
    ElementMpm() : ElementBasic_() {
        particlesInElement_.clear();
    }

    ~ElementMpm() {
        particlesInElement_.clear();
    }

    // Compute coordinates of center of element and length of each side
    void computeCtrCoordAndLength();

    // Give coordinates of center of element
    VecDim giveCenterCoordOfElement() {
        return ctrECoord_;
    }

    // Give length of each side of element
    VecDim giveLengthsOfElement() {
        return eLength_;
    }

    // Add pointers to particles which are inside element
    void addParticle(ParticleSoilPtr pPtr) {
        particlesInElement_.push_back(pPtr);
    }

    // Give particles which are inside element
    ParticleSoilVec_& giveParticles() {
        return particlesInElement_;
    }

//------------------------------------------------------------------------------

    // Give nodes of element
    NodeVec_ giveNodes() {
        NodeVec_ nVec;
        for (unsigned i = 0; i < nodes_.size(); i++)
            nVec.push_back(nodes_[i]);
        return nVec;
    }

    // Give node pointer relevant to the given indice
    NodePtr& giveNode(const unsigned ind) const {
        return nodes_[ind];
    }

    // Give the location of the given node in the element
    unsigned giveNodeLocation(NodePtr ptrNode);

    // Check whether a given node is included in the elment
    bool isNodeIncluded(NodePtr ptrNode);


//------------------------------------------------------------------------------

    // void stiffnessIntegrand ();

    // intialise data and pointers to particles
    void initialise() {
        particlesInElement_.clear();
    }

    // print check
    void printCheckElem() {
        std::cout << "id of node1 = " << ctrECoord_ << "\n";
    }

protected:
    using ElementBasic_::nodes_;      // pointers to its nodes
    ParticleSoilVec_ particlesInElement_; // vector of particle pointers
    VecDim ctrECoord_;                // coodinates of center
    VecDim eLength_;                  // length of each direction

};

#include "ElementMpm.ipp"

#endif  // CARTESIAN_SRC_ELEMENTMPM_H_
