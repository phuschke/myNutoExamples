#include "mechanics/structures/unstructured/Structure.h"
#include "mechanics/timeIntegration/NewmarkDirect.h"
#include "mechanics/constitutive/laws/PhaseField.h"
#include "mechanics/constraints/Constraints.h"
#include "mechanics/elements/ElementBase.h"
#include "mechanics/interpolationtypes/InterpolationType.h"
#include "mechanics/integrationtypes/IntegrationTypeBase.h"
#include "mechanics/constitutive/inputoutput/ConstitutiveCalculateStaticData.h"

#include "../../EnumsAndTypedefs.h"
#include <boost/filesystem.hpp>
#include <mechanics/groups/Group.h>

#include "mechanics/sections/SectionPlane.h"
#include "mechanics/constraints/ConstraintCompanion.h"
#include "mechanics/timeIntegration/postProcessing/PostProcessor.h"

using std::cout;
using std::endl;
using NuTo::Constitutive::ePhaseFieldEnergyDecomposition;
using NuTo::Constraint::Component;
using NuTo::Constraint::RhsRamp;

// geometry
constexpr int dimension = 2;
constexpr double thickness = 1.0; // mm

// material
constexpr double youngsModulus = 2.1e5; // N/mm^2
constexpr double poissonsRatio = 0.3;
constexpr double fractureEnergy = 2.7; // N/mm

constexpr ePhaseFieldEnergyDecomposition energyDecomposition = ePhaseFieldEnergyDecomposition::ISOTROPIC;

// integration
constexpr bool performLineSearch = true;
constexpr bool automaticTimeStepping = true;
constexpr double timeStep = 1.e-4;
constexpr double minTimeStep = 1.e-10;
constexpr double maxTimeStep = 1.e-4;
constexpr double timeStepPostProcessing = 1.e-5;
constexpr double simulationTime = 1.e-2;
constexpr double loadFactor = simulationTime;

// auxiliary
constexpr double toleranceCrack = 1e-6;
constexpr double toleranceDisp = 1e-6;
constexpr double tol = 1.0e-8;

boost::filesystem::path resultPath("results_single_edge_notched_tension_test/");
const boost::filesystem::path meshFilePath("single_edge_notched_tension_test_quads_coarse.msh");

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        cout << "Input arguments: length scale parameter, artificial viscosity, subdirectory" << endl;
        return EXIT_FAILURE;
    }

    double lengthScaleParameter = std::stod(argv[1]); // 0.005; // mm
    double artificialViscosity = std::stod(argv[2]); // 0.0001; // Ns/mm^2

    boost::filesystem::create_directory(resultPath);
    boost::filesystem::path resultPath("results_single_edge_notched_tension_test/" + std::string(argv[3]));
    cout << "**********************************************" << endl;
    cout << "**  strucutre                               **" << endl;
    cout << "**********************************************" << endl;

    NuTo::Structure structure(dimension);
    structure.SetNumTimeDerivatives(1);
    structure.SetShowTime(false);

    cout << "**********************************************" << endl;
    cout << "**  integration scheme                       **" << endl;
    cout << "**********************************************" << endl;

    NuTo::NewmarkDirect integrationScheme(&structure);

    integrationScheme.SetTimeStep(timeStep);
    integrationScheme.SetMinTimeStep(minTimeStep);
    integrationScheme.SetMaxTimeStep(maxTimeStep);
    integrationScheme.SetAutomaticTimeStepping(automaticTimeStepping);
    integrationScheme.SetPerformLineSearch(performLineSearch);
    integrationScheme.SetToleranceResidual(NuTo::Node::eDof::CRACKPHASEFIELD, toleranceCrack);
    integrationScheme.SetToleranceResidual(NuTo::Node::eDof::DISPLACEMENTS, toleranceDisp);
    integrationScheme.PostProcessing().SetResultDirectory(resultPath.string(), true);
    integrationScheme.PostProcessing().SetMinTimeStepPlot(timeStepPostProcessing);

    std::ofstream file;
    file.open(std::string(resultPath.string() + "parameters.txt"));
    file << "dimension            \t" << dimension << std::endl;
    file << "performLineSearch    \t" << performLineSearch << std::endl;
    file << "automaticTimeStepping\t" << automaticTimeStepping << std::endl;
    file << "youngsModulus        \t" << youngsModulus << std::endl;
    file << "poissonsRatio        \t" << poissonsRatio << std::endl;
    file << "thickness            \t" << thickness << std::endl;
    file << "lengthScaleParameter \t" << lengthScaleParameter << std::endl;
    file << "fractureEnergy       \t" << fractureEnergy << std::endl;
    file << "artificialViscosity  \t" << artificialViscosity << std::endl;
    file << "timeStep             \t" << timeStep << std::endl;
    file << "minTimeStep          \t" << minTimeStep << std::endl;
    file << "maxTimeStep          \t" << maxTimeStep << std::endl;
    file << "toleranceDisp        \t" << toleranceDisp << std::endl;
    file << "toleranceCrack       \t" << toleranceCrack << std::endl;
    file << "simulationTime       \t" << simulationTime << std::endl;
    file << "loadFactor           \t" << loadFactor << std::endl;
    file.close();

    cout << "**********************************************" << endl;
    cout << "**  section                                 **" << endl;
    cout << "**********************************************" << endl;

    auto section = NuTo::SectionPlane::Create(thickness, true);
    structure.ElementTotalSetSection(section);

    cout << "**********************************************" << endl;
    cout << "**  material                                **" << endl;
    cout << "**********************************************" << endl;

    NuTo::ConstitutiveBase* phaseField = new NuTo::PhaseField(youngsModulus, poissonsRatio, lengthScaleParameter,
                                                              fractureEnergy, artificialViscosity, energyDecomposition);

    int matrixMaterial = structure.AddConstitutiveLaw(phaseField);

    cout << "**********************************************" << endl;
    cout << "**  geometry                                **" << endl;
    cout << "**********************************************" << endl;

    auto createdGroupIds = structure.ImportFromGmsh(meshFilePath.string());
    int groupId = createdGroupIds[0].first;

    const int interpolationTypeId = structure.InterpolationTypeCreate(eShapeType::QUAD2D);
    structure.InterpolationTypeAdd(interpolationTypeId, eDof::COORDINATES, eTypeOrder::EQUIDISTANT1);
    structure.InterpolationTypeAdd(interpolationTypeId, eDof::DISPLACEMENTS, eTypeOrder::EQUIDISTANT1);
    structure.InterpolationTypeAdd(interpolationTypeId, eDof::CRACKPHASEFIELD, eTypeOrder::EQUIDISTANT1);
    structure.ElementGroupSetInterpolationType(groupId, interpolationTypeId);

    structure.InterpolationTypeInfo(interpolationTypeId);
    structure.ElementTotalConvertToInterpolationType(1.e-9, 1);
    structure.ElementTotalSetSection(section);
    structure.ElementTotalSetConstitutiveLaw(matrixMaterial);

    cout << "**********************************************" << endl;
    cout << "**  bc                                      **" << endl;
    cout << "**********************************************" << endl;

    auto& groupNodesBcBottom = structure.GroupGetNodeCoordinateRange(eDirection::Y, -tol, tol);

    Vector2d coordinateBottom({0.0, 0.0});
    auto& groupNodesBcBottomLeft = structure.GroupGetNodeRadiusRange(coordinateBottom);

    structure.Constraints().Add(eDof::DISPLACEMENTS, Component(groupNodesBcBottom, {NuTo::eDirection::Y}, 0.));
    structure.Constraints().Add(eDof::DISPLACEMENTS, Component(groupNodesBcBottomLeft, {NuTo::eDirection::X}, 0.));


    cout << "**********************************************" << endl;
    cout << "**  load                                    **" << endl;
    cout << "**********************************************" << endl;

    auto& groupNodesLoadTop = structure.GroupGetNodeCoordinateRange(eDirection::Y, 1. - tol, 1. + tol);
    structure.Constraints().Add(eDof::DISPLACEMENTS, Component(groupNodesLoadTop, {NuTo::eDirection::Y},
                                                               RhsRamp(simulationTime, loadFactor)));

    cout << "**********************************************" << endl;
    cout << "**  visualization                           **" << endl;
    cout << "**********************************************" << endl;

    structure.AddVisualizationComponent(groupId, eVisualizeWhat::DISPLACEMENTS);
    structure.AddVisualizationComponent(groupId, eVisualizeWhat::CRACK_PHASE_FIELD);

    cout << "**********************************************" << endl;
    cout << "**  solver                                  **" << endl;
    cout << "**********************************************" << endl;

    structure.NodeBuildGlobalDofs();
    structure.CalculateMaximumIndependentSets();

    Eigen::MatrixXd dispRHS(dimension, dimension);
    dispRHS(0, 0) = 0;
    dispRHS(1, 0) = simulationTime;
    dispRHS(0, 1) = 0;
    dispRHS(1, 1) = loadFactor;

    int groupIdPostProcess = structure.GroupCreateNodeGroup();
    structure.GroupAddNodeCoordinateRange(groupIdPostProcess, eDirection::Y, 1. - tol, 1. + tol);
    integrationScheme.PostProcessing().AddResultGroupNodeForce("force", groupIdPostProcess);
    integrationScheme.PostProcessing().AddResultNodeDisplacements("displacements", groupNodesLoadTop.GetMemberIds()[0]);

    std::cout << "Number of elements: \t" << structure.GetNumElements() << std::endl;
    std::cout << "Number of dofs: \t" << structure.GetNumTotalDofs() << std::endl;
    std::cout << "Number of nodes: \t" << structure.GetNumNodes() << std::endl;
    integrationScheme.Solve(simulationTime);


    cout << "**********************************************" << endl;
    cout << "**  end                                     **" << endl;
    cout << "**********************************************" << endl;
}