#include "pch.h"
#include "Distributions.h"

IMPLEMENT_CLASS(UDistribution, UObject)

IMPLEMENT_CLASS(UDistributionFloat, UDistribution)

IMPLEMENT_CLASS(UDistributionFloatConstant, UDistributionFloat)

IMPLEMENT_CLASS(UDistributionFloatUniform, UDistributionFloat)

IMPLEMENT_CLASS(UDistributionVector, UDistribution)

IMPLEMENT_CLASS(UDistributionVectorConstant, UDistributionVector)

IMPLEMENT_CLASS(UDistributionVectorUniform, UDistributionVector)
