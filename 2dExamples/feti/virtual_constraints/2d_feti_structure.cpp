
#include <mpi.h>
#include <boost/mpi.hpp>

#include "mechanics/feti/StructureFeti.h"
#include <ctime>
#include <chrono>
#include "mechanics/feti/NewmarkFeti.h"
#include "../../../EnumsAndTypedefs.h"

#include "mechanics/nodes/NodeBase.h"

#include "boost/filesystem.hpp"

constexpr int dim = 2;
using Eigen::VectorXd;
using Eigen::MatrixXd;




constexpr   int         dimension                   = 2;
constexpr   double      thickness                   = 1.0;

// material
constexpr   double      youngsModulus               = 4.0e4;
constexpr   double      poissonsRatio               = 0.2;
constexpr   double      tensileStrength             = 3;
constexpr   double      compressiveStrength         = 30;
constexpr   double      fractureEnergy              = 0.1;

// integration
constexpr   bool        performLineSearch           = true;
constexpr   bool        automaticTimeStepping       = true;
constexpr   double      timeStep                    = 1e-0;
constexpr   double      minTimeStep                 = 1e-1;
constexpr   double      maxTimeStep                 =  1e-0;
constexpr   double      toleranceDisp              = 1e-6;
constexpr   double      simulationTime              = 3.0;
constexpr   double      loadFactor                  = 30.0;
constexpr   double      maxInterations              = 1;


const Eigen::Vector2d directionX    = Eigen::Vector2d::UnitX();
const Eigen::Vector2d directionY    = Eigen::Vector2d::UnitY();

void AssignSection(NuTo::StructureFeti& structure);
void AssignMaterial(NuTo::StructureFeti& structure);



int main(int argc, char* argv[])
{
    boost::mpi::environment  env(argc, argv);
    boost::mpi::communicator world;

    const int rank = world.rank();

    NuTo::StructureFeti structure(dim);
    structure.SetNumTimeDerivatives(0);
    structure.SetVerboseLevel(10);
    structure.SetShowTime(false);
    structure.GetLogger().OpenFile("output" + std::to_string(rank));
    structure.GetLogger().SetQuiet(true);


    std::string meshFile = argv[1] + std::to_string(rank);
    structure.GetLogger() << meshFile << "\n";

    const int interpolationTypeId = structure.InterpolationTypeCreate(eShapeType::QUAD2D);
    structure.InterpolationTypeAdd(interpolationTypeId, eDof::COORDINATES,     eTypeOrder::EQUIDISTANT1);
    structure.InterpolationTypeAdd(interpolationTypeId, eDof::DISPLACEMENTS,   eTypeOrder::EQUIDISTANT1);

    structure.ImportMeshJson(meshFile,interpolationTypeId);


    AssignMaterial(structure);
    AssignSection(structure);

    structure.GetLogger() << "**********************************************" << "\n";
    structure.GetLogger() << "**  virtual constraints                     **" << "\n";
    structure.GetLogger() << "**********************************************" << "\n\n";

    Eigen::VectorXd nodeCoords(2);


    if (structure.mRank == 1)
    {
        int groupNodesFakeConstraints00 = structure.GroupCreate(eGroupId::Nodes);
        int groupNodesFakeConstraints01 = structure.GroupCreate(eGroupId::Nodes);

        nodeCoords[0] = 10;
        nodeCoords[1] = 0;
        structure.GroupAddNodeRadiusRange(groupNodesFakeConstraints00, nodeCoords, 0, 1.e-6);
        structure.ConstraintLinearSetDisplacementNodeGroup(groupNodesFakeConstraints00, directionX, 0);


        nodeCoords[0] = 10;
        nodeCoords[1] = 10;
        structure.GroupAddNodeRadiusRange(groupNodesFakeConstraints01, nodeCoords, 0, 1.e-6);
        structure.ConstraintLinearSetDisplacementNodeGroup(groupNodesFakeConstraints01, directionX, 0);
        structure.ConstraintLinearSetDisplacementNodeGroup(groupNodesFakeConstraints01, directionY, 0);

        structure.GetLogger() << "Number of nodes that are constraint in 1st group: \t"
                              << structure.GroupGetNumMembers(groupNodesFakeConstraints00) << "\n";

        structure.GetLogger() << "Number of nodes that are constraint in 2nd group: \t"
                              << structure.GroupGetNumMembers(groupNodesFakeConstraints01) << "\n";


    }

    if (structure.mRank == 0)
    {
        int groupNodesFakeConstraints00 = structure.GroupCreate(eGroupId::Nodes);
        int groupNodesFakeConstraints01 = structure.GroupCreate(eGroupId::Nodes);

        nodeCoords[0] = 40;
        nodeCoords[1] = 0;
        structure.GroupAddNodeRadiusRange(groupNodesFakeConstraints00, nodeCoords, 0, 1.e-6);
        structure.ConstraintLinearSetDisplacementNodeGroup(groupNodesFakeConstraints00, directionX, 0);


        nodeCoords[0] = 40;
        nodeCoords[1] = 10;
        structure.GroupAddNodeRadiusRange(groupNodesFakeConstraints01, nodeCoords, 0, 1.e-6);
        structure.ConstraintLinearSetDisplacementNodeGroup(groupNodesFakeConstraints01, directionX, 0);
        structure.ConstraintLinearSetDisplacementNodeGroup(groupNodesFakeConstraints01, directionY, 0);

        structure.GetLogger() << "Number of nodes that are constraint in 1st group: \t"
                              << structure.GroupGetNumMembers(groupNodesFakeConstraints00) << "\n";

        structure.GetLogger() << "Number of nodes that are constraint in 2nd group: \t"
                              << structure.GroupGetNumMembers(groupNodesFakeConstraints01) << "\n";


    }

    if (structure.mRank == 2)
    {
        int groupNodesFakeConstraints00 = structure.GroupCreate(eGroupId::Nodes);
        int groupNodesFakeConstraints01 = structure.GroupCreate(eGroupId::Nodes);

        nodeCoords[0] = 40;
        nodeCoords[1] = 0;
        structure.GroupAddNodeRadiusRange(groupNodesFakeConstraints00, nodeCoords, 0, 1.e-6);
        structure.ConstraintLinearSetDisplacementNodeGroup(groupNodesFakeConstraints00, directionX, 0);


        nodeCoords[0] = 40;
        nodeCoords[1] = 10;
        structure.GroupAddNodeRadiusRange(groupNodesFakeConstraints01, nodeCoords, 0, 1.e-6);
        structure.ConstraintLinearSetDisplacementNodeGroup(groupNodesFakeConstraints01, directionX, 0);
        structure.ConstraintLinearSetDisplacementNodeGroup(groupNodesFakeConstraints01, directionY, 0);

        structure.GetLogger() << "Number of nodes that are constraint in 1st group: \t"
                              << structure.GroupGetNumMembers(groupNodesFakeConstraints00) << "\n";

        structure.GetLogger() << "Number of nodes that are constraint in 2nd group: \t"
                              << structure.GroupGetNumMembers(groupNodesFakeConstraints01) << "\n";


    }


    if (structure.mRank == 3)
    {
        int groupNodesFakeConstraints00 = structure.GroupCreate(eGroupId::Nodes);
        int groupNodesFakeConstraints01 = structure.GroupCreate(eGroupId::Nodes);

        nodeCoords[0] = 50;
        nodeCoords[1] = 0;
        structure.GroupAddNodeRadiusRange(groupNodesFakeConstraints00, nodeCoords, 0, 1.e-6);
        structure.ConstraintLinearSetDisplacementNodeGroup(groupNodesFakeConstraints00, directionX, 0);


        nodeCoords[0] = 50;
        nodeCoords[1] = 10;
        structure.GroupAddNodeRadiusRange(groupNodesFakeConstraints01, nodeCoords, 0, 1.e-6);
        structure.ConstraintLinearSetDisplacementNodeGroup(groupNodesFakeConstraints01, directionX, 0);
        structure.ConstraintLinearSetDisplacementNodeGroup(groupNodesFakeConstraints01, directionY, 0);

        structure.GetLogger() << "Number of nodes that are constraint in 1st group: \t"
                              << structure.GroupGetNumMembers(groupNodesFakeConstraints00) << "\n";

        structure.GetLogger() << "Number of nodes that are constraint in 2nd group: \t"
                              << structure.GroupGetNumMembers(groupNodesFakeConstraints01) << "\n";

    }

    structure.NodeBuildGlobalDofs(__PRETTY_FUNCTION__);

    structure.GetLogger() << "**********************************************" << "\n";
    structure.GetLogger() << "**  real constraints                        **" << "\n";
    structure.GetLogger() << "**********************************************" << "\n\n";


    int groupNodesLeftBoundary = structure.GroupCreate(eGroupId::Nodes);

    structure.GroupAddNodeCoordinateRange(groupNodesLeftBoundary,0,-1.e-6,+1.e-6);

    structure.ApplyConstraintsTotalFeti(groupNodesLeftBoundary);

    structure.GetLogger() << "**********************************************" << "\n";
    structure.GetLogger() << "**  load                                    **" << "\n";
    structure.GetLogger() << "**********************************************" << "\n\n";



    int loadNodeGroup = structure.GroupCreate(eGroupId::Nodes);

    nodeCoords[0] = 60;
    nodeCoords[1] = 0;
    structure.GroupAddNodeRadiusRange(loadNodeGroup, nodeCoords, 0, 1.e-6);

    structure.NodeInfo(10);
    // prescribe displacement of loadNodeGroup in Y direction
    std::map<int, double> dofIdAndPrescribedDisplacementMap;
    std::vector<int> nodeIds = structure.GroupGetMemberIds(loadNodeGroup);
    for (auto const& nodeId : nodeIds)
    {
        std::vector<int> dofIds = structure.NodeGetDofIds(nodeId, eDof::DISPLACEMENTS);
        dofIdAndPrescribedDisplacementMap.emplace(dofIds[1], 1.);
    }

    structure.ApplyPrescribedDisplacements(dofIdAndPrescribedDisplacementMap);

//    int loadId = structure.ConstraintLinearSetDisplacementNodeGroup(loadNodeGroup, directionY, 0);


    int loadId = structure.LoadCreateNodeGroupForce(0, loadNodeGroup, directionY, 0);


    structure.GetLogger() << "***********************************" << "\n";
    structure.GetLogger() << "**      Visualization            **" << "\n";
    structure.GetLogger() << "***********************************" << "\n\n";

    int groupAllElements = 9999;
    structure.GroupCreate(groupAllElements, eGroupId::Elements);
    structure.GroupAddElementsTotal(groupAllElements);
    structure.AddVisualizationComponent(groupAllElements, eVisualizeWhat::DISPLACEMENTS);
    structure.AddVisualizationComponent(groupAllElements, eVisualizeWhat::ENGINEERING_STRAIN);
    structure.AddVisualizationComponent(groupAllElements, eVisualizeWhat::ENGINEERING_STRESS);
//    structure.AddVisualizationComponent(groupAllElements, eVisualizeWhat::DAMAGE);


    structure.GetLogger() << "**********************************************" << "\n";
    structure.GetLogger() << "**  integration sheme                       **" << "\n";
    structure.GetLogger() << "**********************************************" << "\n\n";



    NuTo::NewmarkFeti myIntegrationScheme(&structure);
    boost::filesystem::path resultPath(std::string("/home/phuschke/results/feti/" + std::to_string(structure.mRank)));

    myIntegrationScheme.SetTimeStep                 ( timeStep                  );
    myIntegrationScheme.SetMaxNumIterations         ( maxInterations            );
    myIntegrationScheme.SetMinTimeStep              ( minTimeStep               );
    myIntegrationScheme.SetMaxTimeStep              ( maxTimeStep               );
    myIntegrationScheme.SetAutomaticTimeStepping    ( automaticTimeStepping     );
    myIntegrationScheme.SetResultDirectory          ( resultPath.string(), true );
    myIntegrationScheme.SetPerformLineSearch        ( performLineSearch         );
    myIntegrationScheme.SetToleranceResidual        ( eDof::DISPLACEMENTS, toleranceDisp );

    Eigen::Matrix2d dispRHS;
    dispRHS(0, 0) = 0;
    dispRHS(1, 0) = simulationTime;
    dispRHS(0, 1) = 0;
    dispRHS(1, 1) = loadFactor;

//    myIntegrationScheme.AddTimeDependentConstraint(loadId, dispRHS);
    myIntegrationScheme.SetTimeDependentLoadCase(loadId, dispRHS);

    structure.GetLogger() << "***********************************" << "\n";
    structure.GetLogger() << "**      Solve                    **" << "\n";
    structure.GetLogger() << "***********************************" << "\n\n";

    structure.GetLogger()   << "Total number of Dofs: \t"
                            << structure.GetNumTotalDofs() << "\n\n";

//    myIntegrationScheme.Solve(simulationTime);

}


void AssignSection(NuTo::StructureFeti& structure)
{
    structure.GetLogger() << "***********************************" << "\n";
    structure.GetLogger() << "**      Section                  **" << "\n";
    structure.GetLogger() << "***********************************" << "\n\n";

    int section00 = structure.SectionCreate(NuTo::eSectionType::PLANE_STRESS);
    structure.SectionSetThickness(section00, thickness);

    structure.ElementTotalSetSection(section00);
}

void AssignMaterial(NuTo::StructureFeti& structure)
{
    structure.GetLogger() << "***********************************" << "\n";
    structure.GetLogger() << "**      Material                 **" << "\n";
    structure.GetLogger() << "***********************************" << "\n\n";

    int material00 = structure.ConstitutiveLawCreate(eConstitutiveType::LINEAR_ELASTIC_ENGINEERING_STRESS);

    structure.ConstitutiveLawSetParameterDouble(material00, eConstitutiveParameter::YOUNGS_MODULUS, youngsModulus);
    structure.ConstitutiveLawSetParameterDouble(material00, eConstitutiveParameter::POISSONS_RATIO, poissonsRatio);

    structure.ElementTotalSetConstitutiveLaw(material00);

}
