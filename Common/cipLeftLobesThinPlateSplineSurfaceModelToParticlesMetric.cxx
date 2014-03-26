/**
 *
 */

#ifndef __cipLeftLobesThinPlateSplineSurfaceModelToParticlesMetric_cxx
#define __cipLeftLobesThinPlateSplineSurfaceModelToParticlesMetric_cxx

#include "cipLeftLobesThinPlateSplineSurfaceModelToParticlesMetric.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "cipHelper.h"

cipLeftLobesThinPlateSplineSurfaceModelToParticlesMetric
::cipLeftLobesThinPlateSplineSurfaceModelToParticlesMetric()
{
  // The 'cipThinPlateSplineSurface' class wraps functionality for
  // constructing and accessing data for a TPS interpolating surface
  // given a set of surface points 
  this->LeftObliqueThinPlateSplineSurface = new cipThinPlateSplineSurface();

  this->LeftObliqueParticleToTPSMetric = new cipParticleToThinPlateSplineSurfaceMetric();
  this->LeftObliqueParticleToTPSMetric->SetThinPlateSplineSurface( this->LeftObliqueThinPlateSplineSurface );

  this->LeftObliqueNewtonOptimizer = new cipNewtonOptimizer< 2 >();
  this->LeftObliqueNewtonOptimizer->SetMetric( this->LeftObliqueParticleToTPSMetric );
}


cipLeftLobesThinPlateSplineSurfaceModelToParticlesMetric
::~cipLeftLobesThinPlateSplineSurfaceModelToParticlesMetric()
{
}

// Note that 'params' must have the same number of entries as the
// number of PCA modes in our model. Also note that
// 'SetMeanSurfacePoints' must be called prior to the 'GetValue'
// call. 
double cipLeftLobesThinPlateSplineSurfaceModelToParticlesMetric::GetValue( const std::vector< double >* const params )
{
  // First we must construct the TPS surface given the param values. 
  for ( unsigned int p=0; p<this->NumberOfSurfacePoints; p++ )
    {
      this->LeftObliqueSurfacePoints[p][2] = this->MeanPoints[p][2];

      for ( unsigned int m=0; m<this->NumberOfModes; m++ )      
	{
	  // Note that we need only adjust the z-coordinate, as the domain
	  // locations of the points (the x and y coordinates) remain
	  // fixed 
	  this->LeftObliqueSurfacePoints[p][2] += 
	    (*params)[m]*vcl_sqrt(this->Eigenvalues[m])*this->Eigenvectors[m][p];
	}       
    }

  // Now that we have our surface points, we can construct the TPS
  // surfaces corresponding to the left oblique boundaries
  this->LeftObliqueThinPlateSplineSurface->SetSurfacePoints( &this->LeftObliqueSurfacePoints );

  double value = this->FissureTermWeight*this->GetFissureTermValue() + this->VesselTermWeight*this->GetVesselTermValue();
  
  return value;
}

double cipLeftLobesThinPlateSplineSurfaceModelToParticlesMetric::GetFissureTermValue()
{
  double fissureTermValue = 0.0;

  double* position    = new double[3];
  double* loNormal    = new double[3];
  double* orientation = new double[3];

  cipNewtonOptimizer< 2 >::PointType* loDomainParams  = new cipNewtonOptimizer< 2 >::PointType( 2, 2 );
  cipNewtonOptimizer< 2 >::PointType* loOptimalParams = new cipNewtonOptimizer< 2 >::PointType( 2, 2 );

  for ( unsigned int i=0; i<this->NumberOfFissureParticles; i++ )
    {
    position[0] = this->FissureParticles->GetPoint(i)[0];
    position[1] = this->FissureParticles->GetPoint(i)[1];
    position[2] = this->FissureParticles->GetPoint(i)[2];

    orientation[0] = this->FissureParticles->GetPointData()->GetArray( "hevec2" )->GetTuple(i)[0];
    orientation[1] = this->FissureParticles->GetPointData()->GetArray( "hevec2" )->GetTuple(i)[1];
    orientation[2] = this->FissureParticles->GetPointData()->GetArray( "hevec2" )->GetTuple(i)[2];

    // Determine the domain locations for which the particle is closest
    // to the left oblique
    this->LeftObliqueParticleToTPSMetric->SetParticle( position );

    // The particle's x, and y location are a good place to initialize
    // the search for the domain locations that result in the smallest
    // distance between the particle and the TPS surfaces
    (*loDomainParams)[0] = position[0]; 
    (*loDomainParams)[1] = position[1]; 

    // Perform Newton line search to determine the closest point on
    // the current TPS surfaces
    this->LeftObliqueNewtonOptimizer->SetInitialParameters( loDomainParams );
    this->LeftObliqueNewtonOptimizer->Update();
    this->LeftObliqueNewtonOptimizer->GetOptimalParameters( loOptimalParams );

    // Get the distances between the particle and the TPS surfaces. This
    // is just the square root of the objective function value
    // optimized by the Newton method.
    double loDistance = vcl_sqrt( this->LeftObliqueNewtonOptimizer->GetOptimalValue() );

    // Get the TPS surface normals at the domain locations.
    this->LeftObliqueThinPlateSplineSurface->GetSurfaceNormal( (*loOptimalParams)[0], (*loOptimalParams)[1], loNormal );
    double loTheta = cip::GetAngleBetweenVectors( loNormal, orientation, true );

    // Now that we have the surface normals and distances, we can compute this 
    // particle's contribution to the overall objective function value. 
    fissureTermValue -= this->FissureParticleWeights[i]*std::exp( -0.5*std::pow(loDistance/this->FissureSigmaDistance,2) )*
      std::exp( -0.5*std::pow(loTheta/this->FissureSigmaTheta,2) );    
    }

  delete position;
  delete loNormal;
  delete orientation;

  return fissureTermValue;
}

double cipLeftLobesThinPlateSplineSurfaceModelToParticlesMetric::GetVesselTermValue()
{
  double vesselTermValue = 0.0;

  double* position    = new double[3];
  double* loNormal    = new double[3];
  double* orientation = new double[3];

  cipNewtonOptimizer< 2 >::PointType* loDomainParams  = new cipNewtonOptimizer< 2 >::PointType( 2, 2 );
  cipNewtonOptimizer< 2 >::PointType* loOptimalParams = new cipNewtonOptimizer< 2 >::PointType( 2, 2 );

  for ( unsigned int i=0; i<this->NumberOfVesselParticles; i++ )
    {
    position[0] = this->VesselParticles->GetPoint(i)[0];
    position[1] = this->VesselParticles->GetPoint(i)[1];
    position[2] = this->VesselParticles->GetPoint(i)[2];

    orientation[0] = this->VesselParticles->GetPointData()->GetArray( "hevec0" )->GetTuple(i)[0];
    orientation[1] = this->VesselParticles->GetPointData()->GetArray( "hevec0" )->GetTuple(i)[1];
    orientation[2] = this->VesselParticles->GetPointData()->GetArray( "hevec0" )->GetTuple(i)[2];

    // Determine the domain locations for which the particle is closest
    // to the left oblique TPS surfaces
    this->LeftObliqueParticleToTPSMetric->SetParticle( position );

    // The particle's x, and y location are a good place to initialize
    // the search for the domain locations that result in the smallest
    // distance between the particle and the TPS surfaces
    (*loDomainParams)[0] = position[0]; 
    (*loDomainParams)[1] = position[1]; 

    // Perform Newton line search to determine the closest point on
    // the current TPS surfaces
    this->LeftObliqueNewtonOptimizer->SetInitialParameters( loDomainParams );
    this->LeftObliqueNewtonOptimizer->Update();
    this->LeftObliqueNewtonOptimizer->GetOptimalParameters( loOptimalParams );

    // Get the distances between the particle and the TPS surfaces. This
    // is just the square root of the objective function value
    // optimized by the Newton method.
    double loDistance = vcl_sqrt( this->LeftObliqueNewtonOptimizer->GetOptimalValue() );

    // Get the TPS surface normals at the domain locations.
    this->LeftObliqueThinPlateSplineSurface->GetSurfaceNormal( (*loOptimalParams)[0], (*loOptimalParams)[1], loNormal );
    double loTheta = cip::GetAngleBetweenVectors( loNormal, orientation, true );

    // Now that we have the surface normals and distances, we can compute this 
    // particle's contribution to the overall objective function value. 
    vesselTermValue += this->VesselParticleWeights[i]*std::exp( -loDistance/this->VesselSigmaDistance )*
      std::exp( -loTheta/this->VesselSigmaTheta );
    }

  delete position;
  delete loNormal;
  delete orientation;

  return vesselTermValue;
}

double cipLeftLobesThinPlateSplineSurfaceModelToParticlesMetric::GetAirwayTermValue()
{
  return 0;
}

#endif