#if 0
#include "icoFoamInterface.h"
#include "FoamInterface.h"
#include "pisoControl.H"

//fvCFD is included from here
#include "../FoamInterfaceImpl.h"

namespace Hippo {
  namespace icoFoam {
    using namespace Foam;
    struct FoamAppImpl
    {
      Foam::pisoControl piso_;
      Foam::IOdictionary physicalProperties_;
      Foam::dimensionedScalar nu_;
      Foam::volScalarField p_;
      Foam::volVectorField U_;
      Foam::surfaceScalarField phi_;
      label pRefCell_;
      scalar pRefValue_;
      scalar cumulativeContErr_;
      bool
      run(EnvImpl &env);
      FoamAppImpl(Hippo::EnvImpl &env)
        : piso_{env.mesh_}
        , physicalProperties_{
            IOobject("physicalProperties",
                     env.runtime_.constant(),
                     env.mesh_,
                     IOobject::MUST_READ_IF_MODIFIED,
                     IOobject::NO_WRITE)
          }
        , nu_{
            "nu",
            dimViscosity,
            physicalProperties_.lookup("nu")
          }
        , p_{
            IOobject("p",
                     env.runtime_.timeName(),
                     env.mesh_,
                     IOobject::MUST_READ,
                     IOobject::AUTO_WRITE),
            env.mesh_
          }
        , U_{
            IOobject ("U",
                      env.runtime_.timeName(),
                      env.mesh_,
                      IOobject::MUST_READ,
                      IOobject::AUTO_WRITE ),
            env.mesh_
          }
        , phi_ {
            IOobject ("phi",
                      env.runtime_.timeName(),
                      env.mesh_,
                      IOobject::READ_IF_PRESENT,
                      IOobject::AUTO_WRITE
                      ),
            fvc::flux(U_)
          }
        , pRefCell_{0}
        , pRefValue_{0.0}
        , cumulativeContErr_{0.0}
      {
        setRefCell(p_, env.mesh_.solution().dict().subDict("PISO"), pRefCell_, pRefValue_);
        env.mesh_.schemes().setFluxRequired(p_.name());
      }

    };


    bool
    FoamAppImpl::run(EnvImpl &env)
    {
      if (!env.runtime_.loop()) {
        return false;
      }
      //Info<< "Time = " << env.runtime_.userTimeName() << nl << endl;


      scalar CoNum = 0.0;
      scalar meanCoNum = 0.0;

      {
        scalarField sumPhi
          (
           fvc::surfaceSum(mag(phi_))().primitiveField()
           );

        CoNum = 0.5*gMax(sumPhi/env.mesh_.V().field())*env.runtime_.deltaTValue();

        meanCoNum =
          0.5*(gSum(sumPhi)/gSum(env.mesh_.V().field()))*env.runtime_.deltaTValue();
      }


      fvVectorMatrix UEqn
        (
         fvm::ddt(U_)
         + fvm::div(phi_, U_)
         - fvm::laplacian(nu_, U_)
         );

      if (piso_.momentumPredictor())

        {
          solve(UEqn == -fvc::grad(p_));
        }

      // --- PISO loop
      while (piso_.correct())
        {
          volScalarField rAU(1.0/UEqn.A());
          volVectorField HbyA(constrainHbyA(rAU*UEqn.H(), U_, p_));
          surfaceScalarField phiHbyA
            (
             "phiHbyA",
             fvc::flux(HbyA)
             + fvc::interpolate(rAU)*fvc::ddtCorr(U_, phi_)
             );

          adjustPhi(phiHbyA, U_, p_);

          // Update the pressure BCs to ensure flux consistency
          constrainPressure(p_, U_, phiHbyA, rAU);

          // Non-orthogonal pressure corrector loop
          while (piso_.correctNonOrthogonal())
            {
              // Pressure corrector

              fvScalarMatrix pEqn
                (
                 fvm::laplacian(rAU, p_) == fvc::div(phiHbyA)
                 );

              pEqn.setReference(pRefCell_, pRefValue_);

              pEqn.solve();

              if (piso_.finalNonOrthogonalIter())
                {
                  phi_ = phiHbyA - pEqn.flux();
                }
            }

          {
            volScalarField contErr(fvc::div(phi_));

            scalar sumLocalContErr = env.runtime_.deltaTValue()*
              mag(contErr)().weightedAverage(env.mesh_.V()).value();

            scalar globalContErr = env.runtime_.deltaTValue()*
              contErr.weightedAverage(env.mesh_.V()).value();
            cumulativeContErr_ += globalContErr;

            //   Info<< "time step continuity errors : sum local = " << sumLocalContErr
            //      << ", global = " << globalContErr
            //     << ", cumulative = " << env.cumulativeContErr_
            //    << endl;
          }

          U_ = HbyA - rAU*fvc::grad(p_);
          U_.correctBoundaryConditions();
        }

      env.runtime_.write();

      //Info<< "ExecutionTime = " << env.runtime_.elapsedCpuTime() << " s"
      //    << "  ClockTime = " << env.runtime_.elapsedClockTime() << " s"
      //    << nl << endl;

      return true;
    }

    FoamApp::FoamApp(Env &env)
      : impl_{new FoamAppImpl{*env.getImpl()}}
    {}

    bool
    FoamApp::run(Env &env)
    {
      return impl_->run(*env.getImpl());
    }

    FoamApp::~FoamApp() = default;
  }
}
#endif
