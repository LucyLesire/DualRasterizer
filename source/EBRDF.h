#pragma once

#include "EMath.h"
#include "ERGBColor.h"

namespace
{
	namespace BRDF
	{
		Elite::RGBColor Lambert(const Elite::RGBColor& diffuseColour, float diffuseReflectance)
		{
			return ((diffuseColour * Elite::RGBColor{ diffuseReflectance, diffuseReflectance, diffuseReflectance }) / float(E_PI));
		}

		Elite::RGBColor Phong(const Elite::RGBColor& color, float phongExponent, const Elite::FVector3& w0, const Elite::FVector3& w1, const Elite::FVector3& normal)
		{
			float reflect = Elite::Dot(Elite::Reflect(-w0, normal), w1);
			reflect = Elite::Clamp(reflect, 0.f, 1.f);
			return color * powf(reflect, phongExponent);
		}

		float NormalD(const Elite::FVector3& normal, const Elite::FVector3& halfVector, float roughnessSquared)
		{
			return ((roughnessSquared * roughnessSquared) / float(E_PI * powf(((Elite::Dot(normal, halfVector) * Elite::Dot(normal, halfVector)) * ((roughnessSquared * roughnessSquared) - 1) + 1), 2)));
		}

		Elite::RGBColor Fresnel(const Elite::FVector3& halfVector, const Elite::FVector3& viewDirection, Elite::RGBColor reflectivity = Elite::RGBColor{ 0.04f, 0.04f, 0.04f })
		{
			return reflectivity + ((Elite::RGBColor{ 1.f, 1.f, 1.f } - reflectivity) * powf((1 - Elite::Dot(halfVector, viewDirection)), 5));
		}

		float Geometry(const Elite::FVector3& normal, const Elite::FVector3& viewDirection, float roughnessSquared)
		{
			float k = ((roughnessSquared + 1) * (roughnessSquared + 1)) / 8;
			return (Elite::Dot(normal, viewDirection) / ((Elite::Dot(normal, viewDirection) * (1 - k)) + k));
		}

		Elite::RGBColor cookTorrence(const Elite::FVector3& normal, const Elite::FVector3& halfVector, const Elite::FVector3& viewDirection, const Elite::FVector3& lightDirection, float roughnessSquared, Elite::RGBColor reflectivity = Elite::RGBColor{ 0.04f, 0.04f, 0.04f })
		{
			return ((Fresnel(halfVector, viewDirection, reflectivity) * NormalD(normal, halfVector, roughnessSquared)
				* (Geometry(normal, viewDirection, roughnessSquared) * Geometry(normal, lightDirection, roughnessSquared))) / (4 * Elite::Dot(viewDirection, normal) * Elite::Dot(lightDirection, normal)));
		}
	}
}
