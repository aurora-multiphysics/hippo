#include <fvCFD_moose.h>
#include "fluidThermo.H"
#include "compressibleMomentumTransportModels.H"
#include "fluidThermophysicalTransportModel.H"
#include "pimpleControl.H"
#include "pressureReference.H"
#include "hydrostaticInitialisation.H"
#include "CorrectPhi.H"
#include "localEulerDdtScheme.H"
#include "fvcSmooth.H"
#include "PrimitivePatchInterpolation.H"

/*
 * Code in here is directly copied from  buoyantFoam.C in openFOAM
 * changes are that the #includes have been expanded out
 * and field variables etc. are memebers of this class
 * so we can persist data across timesteps
 */

// This is a hack this macro is defined somewhere in moose and foam
// I had to undef it in the fvCFD_moose.h macro
// but I need to redefne it here
// The foam one is not in scope
#define NotImplemented
#include "fvModels.H"
#include "fvConstraints.H"
#undef NotImplemented

#include <FoamProblem.h>

namespace Hippo
{
using namespace Foam;
class buoyantFoamImpl
{
  pimpleControl pimple;
  bool correctPhi;
  bool checkMeshCourantNo;
  bool moveMeshOuterCorrectors;
  scalar cumulativeContErr = 0;
  IOMRFZoneList MRF;
  Foam::fvModels & fvModels;
  Foam::fvConstraints & fvConstraints;
  autoPtr<fluidThermo> pThermo;
  fluidThermo & thermo;
  volScalarField & p;
  volScalarField rho;
  volVectorField U;
  surfaceScalarField phi;
  autoPtr<compressible::momentumTransportModel> turbulence;
  autoPtr<fluidThermophysicalTransportModel> thermophysicalTransport;
  uniformDimensionedVectorField g;
  uniformDimensionedScalarField hRef;
  dimensionedScalar ghRef;
  volScalarField gh;
  surfaceScalarField ghf;
  uniformDimensionedScalarField pRef;
  volScalarField p_rgh;
  pressureReference pressureRef;
  volScalarField dpdt;
  volScalarField K;
  dimensionedScalar initialMass;
  volScalarField const & psi;
  autoPtr<surfaceVectorField> rhoUf;

public:
  buoyantFoamImpl(Foam::argList & args, Foam::Time & runTime, Foam::fvMesh & mesh)
    : pimple(mesh),
      correctPhi(pimple.dict().lookupOrDefault("correctPhi", mesh.dynamic())),
      checkMeshCourantNo(pimple.dict().lookupOrDefault("checkMeshCourantNo", false)),
      moveMeshOuterCorrectors(pimple.dict().lookupOrDefault("moveMeshOuterCorrectors", false)),
      MRF(mesh),
      fvModels(Foam::fvModels::New(mesh)),
      fvConstraints(Foam::fvConstraints::New(mesh)),
      pThermo(fluidThermo::New(mesh)),
      thermo(pThermo()),
      p(thermo.p()),
      rho(IOobject("rho", runTime.timeName(), mesh, IOobject::NO_READ, IOobject::NO_WRITE),
          thermo.rho()),
      U(IOobject("U", runTime.timeName(), mesh, IOobject::MUST_READ, IOobject::AUTO_WRITE), mesh),
      phi(IOobject(
              "phi", runTime.timeName(), mesh, IOobject::READ_IF_PRESENT, IOobject::AUTO_WRITE),
          linearInterpolate(rho * U) & mesh.Sf()),
      turbulence(compressible::momentumTransportModel::New(rho, U, phi, thermo)),
      thermophysicalTransport(fluidThermophysicalTransportModel::New(turbulence(), thermo)),
      g(IOobject("g", runTime.constant(), mesh, IOobject::MUST_READ, IOobject::NO_WRITE)),
      hRef(
          IOobject("hRef", runTime.constant(), mesh, IOobject::READ_IF_PRESENT, IOobject::NO_WRITE),
          dimensionedScalar(dimLength, 0)),
      ghRef(-mag(g) * hRef),
      gh("gh", (g & mesh.C()) - ghRef),
      ghf("ghf", (g & mesh.Cf()) - ghRef),
      pRef(
          IOobject("pRef", runTime.constant(), mesh, IOobject::READ_IF_PRESENT, IOobject::NO_WRITE),
          dimensionedScalar(dimPressure, 0)),
      p_rgh(IOobject("p_rgh", runTime.timeName(), mesh, IOobject::MUST_READ, IOobject::AUTO_WRITE),
            mesh),
      pressureRef(p, p_rgh, pimple.dict(), thermo.incompressible())

      ,
      dpdt(IOobject("dpdt", runTime.timeName(), mesh),
           mesh,
           dimensionedScalar(p.dimensions() / dimTime, 0)),
      K("K", 0.5 * magSqr(U)),
      initialMass(fvc::domainIntegrate(rho)),
      psi(thermo.psi())
  {
    thermo.validate(args.executable(), "h", "e");
    mesh.schemes().setFluxRequired(p_rgh.name());
    hydrostaticInitialisation(p_rgh, p, rho, U, gh, ghf, pRef, thermo, pimple.dict());
    if (mesh.dynamic())
    {
      rhoUf = new surfaceVectorField(
          IOobject(
              "rhoUf", runTime.timeName(), mesh, IOobject::READ_IF_PRESENT, IOobject::AUTO_WRITE),
          fvc::interpolate(rho * U));
    }
    if (!fvModels.PtrListDictionary<fvModel>::size())
    {
      Info << "No fvModels present" << endl;
    }

    if (!fvConstraints.PtrListDictionary<fvConstraint>::size())
    {
      Info << "No fvConstraints present" << endl;
    }
    bool adjustTimeStep = runTime.controlDict().lookupOrDefault("adjustTimeStep", false);

    scalar maxCo = runTime.controlDict().lookupOrDefault<scalar>("maxCo", 1.0);

    scalar maxDeltaT = runTime.controlDict().lookupOrDefault<scalar>("maxDeltaT", great);

    // TempFields field(env);

    if (!mesh.foundObject<IOdictionary>("radiationProperties"))
    {
      typeIOobject<IOdictionary> radiationProperties("radiationProperties",
                                                     runTime.constant(),
                                                     mesh,
                                                     IOobject::MUST_READ,
                                                     IOobject::NO_WRITE,
                                                     false);

      if (radiationProperties.headerOk())
      {
        const word modelType(IOdictionary(radiationProperties).lookup("radiationModel"));

        if (modelType != "none")
        {
          FatalErrorInFunction << "Radiation model " << modelType
                               << " selected but not enabled in fvModels" << nl
                               << "To enable radiation add " << nl << nl << "radiation" << nl << "{"
                               << nl << "    type    radiation;" << nl
                               << "    libs    (\"libradiationModels.so\");" << nl << "}" << nl
                               << nl << "to " << radiationProperties.relativePath() / "fvModels"
                               << exit(FatalError);
        }
      }
    }

    turbulence->validate();

    //------------- "compressibleCourantNo.H"
    scalar CoNum = 0.0;
    scalar meanCoNum = 0.0;
    scalarField sumPhi(fvc::surfaceSum(mag(phi))().primitiveField() / rho.primitiveField());

    CoNum = 0.5 * gMax(sumPhi / mesh.V().field()) * runTime.deltaTValue();

    meanCoNum = 0.5 * (gSum(sumPhi) / gSum(mesh.V().field())) * runTime.deltaTValue();

    //------------------"setInitialDeltaT.H"
    Info << "Courant Number mean: " << meanCoNum << " max: " << CoNum << endl;

    if (adjustTimeStep)
    {
      if ((runTime.timeIndex() == 0) && (CoNum > small))
      {
        runTime.setDeltaT(
            min(maxCo * runTime.deltaTValue() / CoNum, min(runTime.deltaTValue(), maxDeltaT)));
      }
    }
  }

  bool run(Foam::Time & runtime, Foam::fvMesh & mesh)
  {
    if (!runtime.loop())
    {
      return false;
    }
    bool adjustTimeStep = runtime.controlDict().lookupOrDefault("adjustTimeStep", false);

    scalar maxCo = runtime.controlDict().lookupOrDefault<scalar>("maxCo", 1.0);

    scalar maxDeltaT = runtime.controlDict().lookupOrDefault<scalar>("maxDeltaT", great);
    //--------------------end readTimeControls
    // These need to be passed in as origional value used
    correctPhi = pimple.dict().lookupOrDefault("correctPhi", correctPhi);

    checkMeshCourantNo = pimple.dict().lookupOrDefault("checkMeshCourantNo", checkMeshCourantNo);

    moveMeshOuterCorrectors =
        pimple.dict().lookupOrDefault("moveMeshOuterCorrectors", moveMeshOuterCorrectors);

    // Store divrhoU from the previous mesh so that it can be mapped
    // and used in correctPhi to ensure the corrected phi has the
    // same divergence
    autoPtr<volScalarField> divrhoU;
    if (correctPhi)
    {
      divrhoU = new volScalarField("divrhoU", fvc::div(fvc::absolute(phi, rho, U)));
    }

    {
      //------------------- "compressibleCourantNo.H"
      scalar CoNum = 0.0;
      scalar meanCoNum = 0.0;

      {
        scalarField sumPhi(fvc::surfaceSum(mag(phi))().primitiveField() / rho.primitiveField());

        CoNum = 0.5 * gMax(sumPhi / mesh.V().field()) * runtime.deltaTValue();

        meanCoNum = 0.5 * (gSum(sumPhi) / gSum(mesh.V().field())) * runtime.deltaTValue();
      }

      Info << "Courant Number mean: " << meanCoNum << " max: " << CoNum << endl;

      //----------------- "setDeltaT.H"
      if (adjustTimeStep)
      {
        scalar deltaT = maxCo * runtime.deltaTValue() / (CoNum + small);
        deltaT = min(deltaT, fvModels.maxDeltaT());
        deltaT =
            min(min(deltaT, runtime.deltaTValue() + 0.1 * deltaT), 1.2 * runtime.deltaTValue());
        runtime.setDeltaT(min(deltaT, maxDeltaT));

        Info << "deltaT = " << runtime.deltaTValue() << endl;
      }
    }

    fvModels.preUpdateMesh();

    // Store momentum to set rhoUf for introduced faces.
    autoPtr<volVectorField> rhoU;
    if (rhoUf.valid())
    {
      rhoU = new volVectorField("rhoU", rho * U);
    }

    // Update the mesh for topology change, mesh to mesh mapping
    mesh.update();

    // TODO: Where did this come from?
    // runtime++;

    Info << "Time = " << runtime.userTimeName() << nl << endl;

    // --- Pressure-velocity PIMPLE corrector loop
    while (pimple.loop())
    {
      if (!pimple.flow())
      {
        if (pimple.models())
        {
          fvModels.correct();
        }

        if (pimple.thermophysics())
        {

          //--------------"EEqn.H"
          {
            volScalarField & he = thermo.he();

            fvScalarMatrix EEqn(
                fvm::ddt(rho, he) + fvm::div(phi, he) + fvc::ddt(rho, K) + fvc::div(phi, K) +
                    (he.name() == "e" ? fvc::div(fvc::absolute(phi, rho, U), p / rho) : -dpdt) +
                    thermophysicalTransport->divq(he) ==
                rho * (U & g) + fvModels.source(rho, he));

            EEqn.relax();

            fvConstraints.constrain(EEqn);

            EEqn.solve();

            fvConstraints.constrain(he);

            thermo.correct();
          }
        }
      }
      else
      {
        if (pimple.firstPimpleIter() || moveMeshOuterCorrectors)
        {
          // Move the mesh
          mesh.move();

          if (mesh.changing())
          {
            gh = (g & mesh.C()) - ghRef;
            ghf = (g & mesh.Cf()) - ghRef;

            MRF.update();

            if (correctPhi)
            {
              //---------------------- "correctPhi.H"
              // Calculate absolute flux
              // from the mapped surface velocity
              phi = mesh.Sf() & rhoUf();

              correctUphiBCs(rho, U, phi, true);

              CorrectPhi(
                  phi, p_rgh, rho, psi, dimensionedScalar("rAUf", dimTime, 1), divrhoU(), pimple);

              // Make the fluxes relative to the mesh-motion
              fvc::makeRelative(phi, rho, U);
            }

            if (checkMeshCourantNo)
            {
              //---------------"meshCourantNo.H"
              scalar meshCoNum = 0.0;
              scalar meanMeshCoNum = 0.0;

              {
                scalarField sumPhi(fvc::surfaceSum(mag(mesh.phi()))().primitiveField());

                meshCoNum = 0.5 * gMax(sumPhi / mesh.V().field()) * runtime.deltaTValue();

                meanMeshCoNum =
                    0.5 * (gSum(sumPhi) / gSum(mesh.V().field())) * runtime.deltaTValue();
              }

              Info << "Mesh Courant Number mean: " << meanMeshCoNum << " max: " << meshCoNum
                   << endl;
            }
          }
        }

        if (pimple.firstPimpleIter() && !pimple.simpleRho())
        {

          //----------------- "rhoEqn.H"
          {
            fvScalarMatrix rhoEqn(fvm::ddt(rho) + fvc::div(phi) == fvModels.source(rho));

            fvConstraints.constrain(rhoEqn);

            rhoEqn.solve();

            fvConstraints.constrain(rho);
          }
        }

        if (pimple.models())
        {
          fvModels.correct();
        }
        // Solve the Momentum equation

        //------------------- "UEqn.H"
        MRF.correctBoundaryVelocity(U);

        tmp<fvVectorMatrix> tUEqn(fvm::ddt(rho, U) + fvm::div(phi, U) + MRF.DDt(rho, U) +
                                      turbulence->divDevTau(U) ==
                                  fvModels.source(rho, U));
        fvVectorMatrix & UEqn = tUEqn.ref();

        UEqn.relax();

        fvConstraints.constrain(UEqn);

        if (pimple.momentumPredictor())
        {
          solve(UEqn ==
                fvc::reconstruct((-ghf * fvc::snGrad(rho) - fvc::snGrad(p_rgh)) * mesh.magSf()));

          fvConstraints.constrain(U);
          K = 0.5 * magSqr(U);
        }

        if (pimple.thermophysics())
        {
          //------------------ "EEqn.H"
          {
            volScalarField & he = thermo.he();

            fvScalarMatrix EEqn(
                fvm::ddt(rho, he) + fvm::div(phi, he) + fvc::ddt(rho, K) + fvc::div(phi, K) +
                    (he.name() == "e" ? fvc::div(fvc::absolute(phi, rho, U), p / rho) : -dpdt) +
                    thermophysicalTransport->divq(he) ==
                rho * (U & g) + fvModels.source(rho, he));

            EEqn.relax();

            fvConstraints.constrain(EEqn);

            EEqn.solve();

            fvConstraints.constrain(he);

            thermo.correct();
          }
        }

        // --- Pressure corrector loop
        while (pimple.correct())
        {

          //-------------"pEqn.H"
          rho = thermo.rho();
          rho.relax();

          // Thermodynamic density needs to be updated by psi*d(p) after the
          // pressure solution
          const volScalarField psip0(psi * p);

          const volScalarField rAU("rAU", 1.0 / UEqn.A());
          const surfaceScalarField rhorAUf("rhorAUf", fvc::interpolate(rho * rAU));
          volVectorField HbyA(constrainHbyA(rAU * UEqn.H(), U, p_rgh));

          if (pimple.nCorrPiso() <= 1)
          {
            tUEqn.clear();
          }

          surfaceScalarField phiHbyA(
              "phiHbyA",
              fvc::interpolate(rho) * fvc::flux(HbyA) +
                  MRF.zeroFilter(rhorAUf * fvc::ddtCorr(rho, U, phi, rhoUf)));

          MRF.makeRelative(fvc::interpolate(rho), phiHbyA);

          bool adjustMass = mesh.schemes().steady() && adjustPhi(phiHbyA, U, p_rgh);

          const surfaceScalarField phig(-rhorAUf * ghf * fvc::snGrad(rho) * mesh.magSf());

          phiHbyA += phig;

          // Update the pressure BCs to ensure flux consistency
          constrainPressure(p_rgh, rho, U, phiHbyA, rhorAUf, MRF);

          fvc::makeRelative(phiHbyA, rho, U);

          fvScalarMatrix p_rghEqn(p_rgh, dimMass / dimTime);

          if (pimple.transonic())
          {
            const surfaceScalarField phid(
                "phid", (fvc::interpolate(psi) / fvc::interpolate(rho)) * phiHbyA);

            const fvScalarMatrix divPhidp(fvm::div(phid, p));
            phiHbyA -= divPhidp.flux();

            fvScalarMatrix p_rghDDtEqn(fvc::ddt(rho) + psi * correction(fvm::ddt(p_rgh)) +
                                           fvc::div(phiHbyA) + divPhidp ==
                                       fvModels.source(psi, p_rgh, rho.name()));

            while (pimple.correctNonOrthogonal())
            {
              p_rghEqn = p_rghDDtEqn - fvm::laplacian(rhorAUf, p_rgh);

              // Relax the pressure equation to ensure diagonal-dominance
              p_rghEqn.relax();

              p_rghEqn.setReference(pressureRef.refCell(), pressureRef.refValue());

              p_rghEqn.solve();
            }
          }
          else
          {
            fvScalarMatrix p_rghDDtEqn(fvc::ddt(rho) + psi * correction(fvm::ddt(p_rgh)) +
                                           fvc::div(phiHbyA) ==
                                       fvModels.source(psi, p_rgh, rho.name()));

            while (pimple.correctNonOrthogonal())
            {
              p_rghEqn = p_rghDDtEqn - fvm::laplacian(rhorAUf, p_rgh);

              p_rghEqn.setReference(pressureRef.refCell(), pressureRef.refValue());

              p_rghEqn.solve();
            }
          }

          phi = phiHbyA + p_rghEqn.flux();

          if (mesh.schemes().steady())
          {
            //---------------------- "incompressible/continuityErrs.H"
            {
              volScalarField contErr(fvc::div(phi));

              scalar sumLocalContErr =
                  runtime.deltaTValue() * mag(contErr)().weightedAverage(mesh.V()).value();

              scalar globalContErr =
                  runtime.deltaTValue() * contErr.weightedAverage(mesh.V()).value();
              cumulativeContErr += globalContErr;

              Info << "time step continuity errors : sum local = " << sumLocalContErr
                   << ", global = " << globalContErr << ", cumulative = " << cumulativeContErr
                   << endl;
            }
          }
          else
          {
            p = p_rgh + rho * gh + pRef;

            const bool constrained = fvConstraints.constrain(p);

            // Thermodynamic density update
            thermo.correctRho(psi * p - psip0);

            if (constrained)
            {
              rho = thermo.rho();
            }

            //----------------- "rhoEqn.H"
            {
              fvScalarMatrix rhoEqn(fvm::ddt(rho) + fvc::div(phi) == fvModels.source(rho));

              fvConstraints.constrain(rhoEqn);

              rhoEqn.solve();

              fvConstraints.constrain(rho);
            }
            //------------------- "compressibleContinuityErrs.H"
            {
              dimensionedScalar totalMass = fvc::domainIntegrate(rho);

              scalar sumLocalContErr =
                  (fvc::domainIntegrate(mag(rho - thermo.rho())) / totalMass).value();

              scalar globalContErr = (fvc::domainIntegrate(rho - thermo.rho()) / totalMass).value();

              cumulativeContErr += globalContErr;

              Info << "time step continuity errors : sum local = " << sumLocalContErr
                   << ", global = " << globalContErr << ", cumulative = " << cumulativeContErr
                   << endl;
            }
          }

          // Explicitly relax pressure for momentum corrector
          p_rgh.relax();

          p = p_rgh + rho * gh + pRef;

          // Correct the momentum source with the pressure gradient flux
          // calculated from the relaxed pressure
          U = HbyA + rAU * fvc::reconstruct((phig + p_rghEqn.flux()) / rhorAUf);
          U.correctBoundaryConditions();
          fvConstraints.constrain(U);
          K = 0.5 * magSqr(U);

          if (mesh.schemes().steady())
          {
            if (fvConstraints.constrain(p))
            {
              p_rgh = p - rho * gh - pRef;
              p_rgh.correctBoundaryConditions();
            }
          }

          // For steady closed-volume compressible cases adjust the pressure level
          // to obey overall mass continuity
          if (adjustMass && !thermo.incompressible())
          {
            p += (initialMass - fvc::domainIntegrate(thermo.rho())) / fvc::domainIntegrate(psi);
            p_rgh = p - rho * gh - pRef;
            p_rgh.correctBoundaryConditions();
          }

          if (mesh.schemes().steady() || pimple.simpleRho() || adjustMass)
          {
            rho = thermo.rho();
          }

          if (mesh.schemes().steady() || pimple.simpleRho())
          {
            rho.relax();
          }

          // Correct rhoUf if the mesh is moving
          fvc::correctRhoUf(rhoUf, rho, U, phi, MRF);

          if (thermo.dpdt())
          {
            dpdt = fvc::ddt(p);

            if (mesh.moving())
            {
              dpdt -= fvc::div(fvc::meshPhi(rho, U), p);
            }
          }
          //-----------------------------------
        }

        if (pimple.turbCorr())
        {
          turbulence->correct();
          thermophysicalTransport->correct();
        }
      }
    }

    rho = thermo.rho();
    runtime.write();

    Info << "ExecutionTime = " << runtime.elapsedCpuTime() << " s"
         << "  ClockTime = " << runtime.elapsedClockTime() << " s" << nl << endl;
    return true;
  }

  /* Append the face temperatures onto the vector */
  size_t append_patch_face_temperatures(int patch_id, std::vector<double> & foamT)
  {
    auto & T = thermo.T().boundaryField()[patch_id];
    for (auto x : T)
    {
      foamT.push_back(x);
    }
    return T.size();
  }

  /* Get the number of faces in the given boundary patch. */
  size_t patch_size(int patch_id)
  {
    // TODO(hsaunders1904): there must be a way of getting the number of
    // without looking into the boundary field of the temperature.
    return thermo.T().boundaryField()[patch_id].size();
  }

  void set_patch_face_temperatures(int patch_id, const std::vector<double> & moose_T)
  {
    auto & patch = thermo.T().boundaryFieldRef()[patch_id];
    assert(moose_T.size() == static_cast<size_t>(patch.size()));
    std::copy(moose_T.begin(), moose_T.end(), patch.begin());
    // printf("setting patch face temp: %f\n", moose_T.at(0));

    // printf("patch_id: %d\n", patch_id);
    // printf("patch.size(): %d\n", patch.size());
    // printf("assigned values: \n");
    // for (auto v : patch)
    // {
    //   printf("%f ", v);
    // }
    // printf("\n");
  }
};

buoyantFoamApp::~buoyantFoamApp() = default;
buoyantFoamApp::buoyantFoamApp(FoamInterface * interface)
  : _interface(interface),
    _impl(std::make_unique<buoyantFoamImpl>(
        _interface->getArglist(), _interface->getRuntime(), _interface->getMesh()))
{
}

bool
buoyantFoamApp::run()
{
  return _impl->run(_interface->getRuntime(), _interface->getMesh());
}

size_t
buoyantFoamApp::append_patch_face_T(int patch_id, std::vector<double> & foamT)
{
  return _impl->append_patch_face_temperatures(patch_id, foamT);
}

size_t
buoyantFoamApp::patch_size(int patch_id)
{
  return _impl->patch_size(patch_id);
}

void
buoyantFoamApp::set_patch_face_t(int patch_id, const std::vector<double> & moose_T)
{
  _impl->set_patch_face_temperatures(patch_id, moose_T);
}

}
