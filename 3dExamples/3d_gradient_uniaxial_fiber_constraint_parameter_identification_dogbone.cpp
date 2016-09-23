#include "nuto/mechanics/structures/unstructured/Structure.h"
#include "nuto/mechanics/timeIntegration/NewmarkDirect.h"

#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>

// Stahlfasern: l = 13 mm, d = 0.2 mm, A = 0.0314 mm^2, U = 0.628 mm, V = 0.4082 mm^3


int main(int argc, char* argv[])
{

    if ( not argc == 5  )
    {
        std::cout << "Invalid number of input arguments: matrixFractureEnergy, fibreCrossSection, maxBondStress, slipAtMaxBondStress" << std::endl;
        return 1;
    }

    // Parameters
    const int dimension = 3;
    const bool performLineSearch = true;
    const bool automaticTimeStepping = true;

    const double matrixYoungsModulus = 49083;   // concrete
    const double matrixTensileStrength = 6.6;
    const double matrixCompressiveStrength = 115;
    const double matrixPoissonsRatio = 0.0;

    const double matrixNonlocalRadius = 1.2  ;
    const double matrixFractureEnergy = std::stod(argv[1]); //0.01;
    const double matrixLengthX = 240.0;
    const double matrixLengthY = 20.0;
    const double matrixLengthZ = 40.0;

    const double fibreYoungsModulus = 2.1e5;    // steel
    const double fibrePoissonsRatio = 0.2;
    const double fibreCrossSection = std::stod(argv[2]);       //0.0314;
    const double fibreCircumference = 0.628;
    const double fibreLength = 13.0;
    const double fibreVolume = 0.4082;

    const double mInterfaceNormalStiffness = 1e6;
    const double mAlpha = 1;
    const double maxBondStress = std::stod(argv[3]); // 3.0
    const double mResidualBondStress = 1.e1;
    const double mSlipAtMaxBondStress = std::stod(argv[4]); // 0.01
    const double mSlipAtResidualBondStress = 1;

    const double mTimeStep = 1e-2;
    const double minTimeStep = 1e-4;
    const double maxTimeStep = 1e-1;
    const double mToleranceForce = 1e-4;
    const double mSimulationTime = 1.0;
    const double mLoad = 0.3;

    const NuTo::FullVector<double, 3> directionX = NuTo::FullVector<double, 3>::UnitX();
    const NuTo::FullVector<double, 3> directionY = NuTo::FullVector<double, 3>::UnitY();
    const NuTo::FullVector<double, 3> directionZ = NuTo::FullVector<double, 3>::UnitZ();

    char tmpChar[200];
    sprintf(tmpChar, "/home/phuschke/3d_parameter_identification/3d_gradient_uniaxial_dogbone_fractureEnergy_%f_fibreCrossSection_%f_maxBondStress_%f_mSlipAtMaxBondStress_%f/", matrixFractureEnergy,fibreCrossSection,maxBondStress,mSlipAtMaxBondStress);
    const std::string resultDir = tmpChar;

    const std::string meshFileMatrix("/home/phuschke/develop/nuto/myNutoExamples/MeshFiles/3d/3d_uniaxial_matrix_ehlers_fine.msh");
    const std::string meshFileFibre("/home/phuschke/develop/nuto/myNutoExamples/MeshFiles/3d/trusses200Dogbone.msh");



    try
    {
        std::cout << "***********************************" << std::endl;
        std::cout << "**      Structure                **" << std::endl;
        std::cout << "***********************************" << std::endl;

        NuTo::Structure myStructure(dimension);
        myStructure.SetVerboseLevel(10);
        myStructure.SetShowTime(false);
        myStructure.SetNumProcessors(4);

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Integration Scheme       **" << std::endl;
        std::cout << "***********************************" << std::endl;

        NuTo::NewmarkDirect myIntegrationScheme(&myStructure);
        myIntegrationScheme.SetTimeStep(mTimeStep);
        myIntegrationScheme.SetMinTimeStep(minTimeStep);
        myIntegrationScheme.SetMaxTimeStep(maxTimeStep);
        myIntegrationScheme.SetToleranceForce(mToleranceForce);
        myIntegrationScheme.SetAutomaticTimeStepping(automaticTimeStepping);
        myIntegrationScheme.SetPerformLineSearch(performLineSearch);
        myIntegrationScheme.SetResultDirectory(resultDir, true);

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Section                  **" << std::endl;
        std::cout << "***********************************" << std::endl;

        int matrixSection = myStructure.SectionCreate(NuTo::Section::VOLUME);

        int fibreSection = myStructure.SectionCreate(NuTo::Section::TRUSS);
        myStructure.SectionSetArea(fibreSection, fibreCrossSection);

        int bondSection = myStructure.SectionCreate(NuTo::Section::FIBRE_MATRIX_BOND);
        myStructure.SectionSetCircumference(bondSection, fibreCircumference);

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Material                 **" << std::endl;
        std::cout << "***********************************" << std::endl;

//        int matrixMaterial = myStructure.ConstitutiveLawCreate(NuTo::Constitutive::eConstitutiveType::LINEAR_ELASTIC_ENGINEERING_STRESS);
//        myStructure.ConstitutiveLawSetParameterDouble(matrixMaterial, NuTo::Constitutive::eConstitutiveParameter::YOUNGS_MODULUS, matrixYoungsModulus);
//        myStructure.ConstitutiveLawSetParameterDouble(matrixMaterial, NuTo::Constitutive::eConstitutiveParameter::POISSONS_RATIO, matrixPoissonsRatio);

        NuTo::FullVector<double, Eigen::Dynamic> myDamageLaw(1);
        myDamageLaw(0) = NuTo::Constitutive::eDamageLawType::ISOTROPIC_EXPONENTIAL_SOFTENING;

        int matrixMaterial = myStructure.ConstitutiveLawCreate(NuTo::Constitutive::eConstitutiveType::GRADIENT_DAMAGE_ENGINEERING_STRESS);
        myStructure.ConstitutiveLawSetParameterDouble(matrixMaterial, NuTo::Constitutive::eConstitutiveParameter::YOUNGS_MODULUS, matrixYoungsModulus);
        myStructure.ConstitutiveLawSetParameterDouble(matrixMaterial, NuTo::Constitutive::eConstitutiveParameter::POISSONS_RATIO, matrixPoissonsRatio);
        myStructure.ConstitutiveLawSetParameterDouble(matrixMaterial, NuTo::Constitutive::eConstitutiveParameter::NONLOCAL_RADIUS, matrixNonlocalRadius);
        myStructure.ConstitutiveLawSetParameterDouble(matrixMaterial, NuTo::Constitutive::eConstitutiveParameter::TENSILE_STRENGTH, matrixTensileStrength);
        myStructure.ConstitutiveLawSetParameterDouble(matrixMaterial, NuTo::Constitutive::eConstitutiveParameter::COMPRESSIVE_STRENGTH, matrixCompressiveStrength);
        myStructure.ConstitutiveLawSetParameterDouble(matrixMaterial, NuTo::Constitutive::eConstitutiveParameter::FRACTURE_ENERGY, matrixFractureEnergy);
        myStructure.ConstitutiveLawSetParameterFullVectorDouble(matrixMaterial, NuTo::Constitutive::eConstitutiveParameter::DAMAGE_LAW, myDamageLaw);

        int fibreMaterial = myStructure.ConstitutiveLawCreate(NuTo::Constitutive::eConstitutiveType::LINEAR_ELASTIC_ENGINEERING_STRESS);
        myStructure.ConstitutiveLawSetParameterDouble(fibreMaterial, NuTo::Constitutive::eConstitutiveParameter::YOUNGS_MODULUS, fibreYoungsModulus);
        myStructure.ConstitutiveLawSetParameterDouble(fibreMaterial, NuTo::Constitutive::eConstitutiveParameter::POISSONS_RATIO, fibrePoissonsRatio);

        int interfaceMaterial = myStructure.ConstitutiveLawCreate(NuTo::Constitutive::eConstitutiveType::FIBRE_MATRIX_BOND_STRESS_SLIP);
        myStructure.ConstitutiveLawSetParameterDouble(interfaceMaterial, NuTo::Constitutive::eConstitutiveParameter::NORMAL_STIFFNESS, mInterfaceNormalStiffness);
        myStructure.ConstitutiveLawSetParameterDouble(interfaceMaterial, NuTo::Constitutive::eConstitutiveParameter::ALPHA, mAlpha);
        myStructure.ConstitutiveLawSetParameterDouble(interfaceMaterial, NuTo::Constitutive::eConstitutiveParameter::MAX_BOND_STRESS, maxBondStress);
        myStructure.ConstitutiveLawSetParameterDouble(interfaceMaterial, NuTo::Constitutive::eConstitutiveParameter::RESIDUAL_BOND_STRESS, mResidualBondStress);
        myStructure.ConstitutiveLawSetParameterDouble(interfaceMaterial, NuTo::Constitutive::eConstitutiveParameter::SLIP_AT_MAX_BOND_STRESS, mSlipAtMaxBondStress);
        myStructure.ConstitutiveLawSetParameterDouble(interfaceMaterial, NuTo::Constitutive::eConstitutiveParameter::SLIP_AT_RESIDUAL_BOND_STRESS, mSlipAtResidualBondStress);

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Interpolation Type       **" << std::endl;
        std::cout << "***********************************" << std::endl;

        int matrixInterpolationType = myStructure.InterpolationTypeCreate(NuTo::Interpolation::eShapeType::TETRAHEDRON3D);
        myStructure.InterpolationTypeAdd(matrixInterpolationType, NuTo::Node::COORDINATES, NuTo::Interpolation::EQUIDISTANT1);
        myStructure.InterpolationTypeAdd(matrixInterpolationType, NuTo::Node::DISPLACEMENTS, NuTo::Interpolation::EQUIDISTANT2);
        myStructure.InterpolationTypeAdd(matrixInterpolationType, NuTo::Node::NONLOCALEQSTRAIN, NuTo::Interpolation::EQUIDISTANT1);

        int fibreInterpolationType = myStructure.InterpolationTypeCreate(NuTo::Interpolation::eShapeType::TRUSSXD);
        myStructure.InterpolationTypeAdd(fibreInterpolationType, NuTo::Node::COORDINATES, NuTo::Interpolation::EQUIDISTANT1);
        myStructure.InterpolationTypeAdd(fibreInterpolationType, NuTo::Node::DISPLACEMENTS, NuTo::Interpolation::EQUIDISTANT1);

        int interfaceInterpolationType = myStructure.InterpolationTypeCreate(NuTo::Interpolation::eShapeType::INTERFACE);
        myStructure.InterpolationTypeAdd(interfaceInterpolationType, NuTo::Node::COORDINATES, NuTo::Interpolation::EQUIDISTANT1);
        myStructure.InterpolationTypeAdd(interfaceInterpolationType, NuTo::Node::DISPLACEMENTS, NuTo::Interpolation::EQUIDISTANT1);

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Matrix                   **" << std::endl;
        std::cout << "***********************************" << std::endl;

        NuTo::FullMatrix<int, Eigen::Dynamic, Eigen::Dynamic> createdGroupIdMatrix = myStructure.ImportFromGmsh(meshFileMatrix, NuTo::ElementData::CONSTITUTIVELAWIP, NuTo::IpData::eIpDataType::STATICDATA);
        int groupIdMatrix = createdGroupIdMatrix.GetValue(0, 0);

        myStructure.ElementGroupSetSection(groupIdMatrix, matrixSection);
        myStructure.ElementGroupSetConstitutiveLaw(groupIdMatrix, matrixMaterial);
        myStructure.ElementTotalConvertToInterpolationType(1.e-6, 10);
        myStructure.ElementGroupSetInterpolationType(groupIdMatrix, matrixInterpolationType);
        myStructure.InterpolationTypeSetIntegrationType(matrixInterpolationType, NuTo::IntegrationType::IntegrationType3D4NGauss4Ip, NuTo::IpData::eIpDataType::STATICDATA);
        myStructure.ElementTotalConvertToInterpolationType(1.e-6, 10);

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Boundary Conditions      **" << std::endl;
        std::cout << "***********************************" << std::endl;

        NuTo::FullVector<double,dimension> nodeCoords;

        int groupNodeBCLeft = myStructure.GroupCreate(NuTo::Groups::eGroupId::Nodes);
        myStructure.GroupAddNodeCoordinateRange(groupNodeBCLeft, 0, 00.0 - 1e-6, 00.0 + 1e-6);

        myStructure.ConstraintLinearSetDisplacementNodeGroup(groupNodeBCLeft, directionX, 0);
        myStructure.ConstraintLinearSetDisplacementNodeGroup(groupNodeBCLeft, directionY, 0);
        myStructure.ConstraintLinearSetDisplacementNodeGroup(groupNodeBCLeft, directionZ, 0);

        int groupNodeBCRight = myStructure.GroupCreate(NuTo::Groups::eGroupId::Nodes);
        myStructure.GroupAddNodeCoordinateRange(groupNodeBCRight, 0, matrixLengthX-1e-6, matrixLengthX+1e-6);

        myStructure.ConstraintLinearSetDisplacementNodeGroup(groupNodeBCRight, directionY, 0);
        myStructure.ConstraintLinearSetDisplacementNodeGroup(groupNodeBCRight, directionZ, 0);


        std::cout << "***********************************" << std::endl;
        std::cout << "**      Fibre                    **" << std::endl;
        std::cout << "***********************************" << std::endl;

        NuTo::FullMatrix<int, Eigen::Dynamic, Eigen::Dynamic> createdGroupIdFibre = myStructure.ImportFromGmsh(meshFileFibre, NuTo::ElementData::CONSTITUTIVELAWIP, NuTo::IpData::eIpDataType::STATICDATA);
        int groupIdFiber = createdGroupIdFibre.GetValue(0, 0);

        myStructure.ElementGroupSetSection(groupIdFiber, fibreSection);
        myStructure.ElementGroupSetConstitutiveLaw(groupIdFiber, fibreMaterial);
        myStructure.ElementGroupSetInterpolationType(groupIdFiber, fibreInterpolationType);
        myStructure.ElementConvertToInterpolationType(groupIdFiber);

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Constraints              **" << std::endl;
        std::cout << "***********************************" << std::endl;

        int numGroups = 2;

        int groupMatrixNodes = myStructure.GroupCreate(NuTo::Groups::Nodes);
        myStructure.GroupAddNodeFromElementGroupCoordinateRange(groupMatrixNodes, groupIdMatrix, 0, 0, 10);

        int groupMatrixElements = myStructure.GroupCreate(NuTo::Groups::Elements);
        myStructure.GroupAddElementsFromNodes(groupMatrixElements, groupMatrixNodes, false);

        int groupConstraintNodes = myStructure.GroupCreate(NuTo::Groups::Nodes);
        myStructure.GroupAddNodeFromElementGroupCoordinateRange(groupConstraintNodes, groupIdFiber, 0, 0, 10);

        myStructure.GroupAddNodesFromElements(groupConstraintNodes, groupIdFiber);



        int numNearestNeighbours = 1;

        auto nodeIds = myStructure.GroupGetMemberIds(groupConstraintNodes);
        std::cout << "nodeIds.rows()"<< nodeIds.rows() << std::endl;
        for (int iNode = 0; iNode < nodeIds.rows(); ++iNode)
        {
            myStructure.ConstraintLinearEquationNodeToElementCreate(nodeIds.at(iNode, 0), groupIdMatrix, NuTo::Node::eAttributes::DISPLACEMENTS, numNearestNeighbours);
        }


        std::cout << "***********************************" << std::endl;
        std::cout << "**      Interface                **" << std::endl;
        std::cout << "***********************************" << std::endl;

        auto bondAndFibreElements = myStructure.InterfaceElementsCreate(groupIdFiber, interfaceInterpolationType, interfaceMaterial, bondSection, fibreInterpolationType, fibreMaterial, fibreSection);

        int groupNewFibreElements = myStructure.GroupCreate(NuTo::Groups::Elements);
        for (int i = 0; i < bondAndFibreElements.rows(); ++i)
        {
            myStructure.GroupAddElement(groupNewFibreElements, bondAndFibreElements.at(i,1));
        }

        std::cout << "***********************************" << std::endl;
        std::cout << "**      Loads                    **" << std::endl;
        std::cout << "***********************************" << std::endl;



        int timeDependentConstraint = myStructure.ConstraintLinearSetDisplacementNodeGroup(groupNodeBCRight, directionX, 1);


        std::cout << "***********************************" << std::endl;
        std::cout << "**      Visualization            **" << std::endl;
        std::cout << "***********************************" << std::endl;

        myStructure.AddVisualizationComponent(groupIdMatrix, NuTo::VisualizeBase::DISPLACEMENTS);
        myStructure.AddVisualizationComponent(groupIdMatrix, NuTo::VisualizeBase::ENGINEERING_STRAIN);
        myStructure.AddVisualizationComponent(groupIdMatrix, NuTo::VisualizeBase::ENGINEERING_STRESS);
        myStructure.AddVisualizationComponent(groupIdMatrix, NuTo::VisualizeBase::DAMAGE);
        myStructure.AddVisualizationComponent(groupIdMatrix, NuTo::VisualizeBase::LOCAL_EQ_STRAIN);

        myStructure.AddVisualizationComponent(groupNewFibreElements, NuTo::VisualizeBase::DISPLACEMENTS);


        std::cout << "***********************************" << std::endl;
        std::cout << "**      Solver                   **" << std::endl;
        std::cout << "***********************************" << std::endl;

        myStructure.NodeBuildGlobalDofs();


        myStructure.CalculateMaximumIndependentSets();


        int groupLoad = myStructure.GroupCreate(NuTo::Groups::eGroupId::Nodes);

        nodeCoords[0] = matrixLengthX;
        nodeCoords[1] = 0;
        nodeCoords[2] = 0;
        myStructure.GroupAddNodeRadiusRange(groupLoad, nodeCoords, 0, 1e-2);

        myIntegrationScheme.AddResultGroupNodeForce("myforce", groupNodeBCRight);
        auto nodeID = myStructure.GroupGetMemberIds(groupLoad);
        myIntegrationScheme.AddResultNodeDisplacements("mydisplacements", nodeID.at(0,0));



        NuTo::FullMatrix<double, 2, 2> timeDependentLoad;
        timeDependentLoad(0, 0) = 0;
        timeDependentLoad(1, 0) = 1.0 * mSimulationTime;

        timeDependentLoad(0, 1) = 0;
        timeDependentLoad(1, 1) = 1.0 * mLoad;

        myIntegrationScheme.SetTimeDependentConstraint(timeDependentConstraint, timeDependentLoad);

        myIntegrationScheme.Solve(mSimulationTime);

        std::string command = "paste " + resultDir + "myforce.dat " + resultDir + "mydisplacements.dat > " + resultDir + "forceDisp.dat";
        system(command.c_str());

        std::cout << "***********************************" << std::endl;
        std::cout << "**      END                      **" << std::endl;
        std::cout << "***********************************" << std::endl;

    } catch (NuTo::MechanicsException& e)
    {
        std::cout << e.ErrorMessage();

    } catch (NuTo::MathException& e)
    {
        std::cout << e.ErrorMessage();

    }

}
