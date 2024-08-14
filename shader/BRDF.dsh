#pragma once

#include "Common.dsh"
/*
 * N: normalized normal of the macro surface
 * L: normalized light ray direction from surface's position towards the light origin
 * V: normalized view vector: from surface's position to the view's origin
 * H: half vector of L & V, equal normalize(L+V)
 * D = Microfacet NDF
 * G = Shadowing and masking
 * F = Fresnel
 * Vis = G / (4*NoL*NoV)
 * f = Microfacet specular BRDF = D*G*F / (4*NoL*NoV) = D*Vis*F
 * F0: F when <cos(N, V)=1>
 * float3 f0 = 0.16 * reflectance * reflectance * (1.0 - metallic) + baseColor * metallic; (metallic:0/1)
 */

//------------------------------------------------------------------------------
// Diffuse BRDF implementations
//------------------------------------------------------------------------------
float3 CalcDiffuseColor(float3 albedo, float metallic)
{
	return albedo * (1.0 - metallic);
}
float3 CalcSpecularColor(float3 albedo, float metallic)
{
	return lerp(0.04, albedo, metallic);
}

// Traditional Lambertion diffuse BRDF		
float3 Diffuse_Lambertian()
{
	return (1.0 / PI);
}

// [Burley 2012, "Physically-Based Shading at Disney"]
float Diffuse_Burley(float3 N, float3 L, float3 V, float roughness)
{			
	float NoL = max(0.0, dot(N, L));
	float NoV = max(0.0, dot(N, V));
	
	float3 H = normalize(L + V);
	float VoH = dot(V, H);
	float F_D90 = 0.5 + 2.0 * VoH * VoH * roughness;

	return 1.0 / PI * (1.0f + (F_D90 - 1.0f) * pow(1.0 - NoL, 5.0)) *
					  (1.0f + (F_D90 - 1.0f) * pow(1.0 - NoV, 5.0));
}

// [Gotanda 2012, "Beyond a Simple Physically Based Blinn-Phong Model in Real-Time"]
float Diffuse_OrenNayar(float3 N, float3 L, float3 V, float roughness, float albedo) 
{ 
  float LoV = dot(L, V);
  float NoL = dot(N, L);
  float NoV = dot(N, V);

  float s = LoV - NoL * NoV;
  float t = lerp(1.0, max(NoL, NoV), step(0.0, s));

  float sigma2 = roughness * roughness;
  float A = 1.0 + sigma2 * (albedo / (sigma2 + 0.13) + 0.5 / (sigma2 + 0.33));
  float B = 0.45 * sigma2 / (sigma2 + 0.09);

  return albedo * max(0.0, NoL) * (A + B * s / t) / PI;
}

//------------------------------------------------------------------------------
// Specular BRDF implementations
//------------------------------------------------------------------------------
// Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
float3 F_Schlick(float cosTheta, float3 F0)
{
	float Fc = pow(saturate(1.0 - cosTheta), 5.0);
	return Fc + F0 * (1.0 - Fc);		
}

float3 F_Schlick(float cosTheta, float3 F0, float F90) 
{
	return F0 + (F90 - F0) *  pow(saturate(1.0 - cosTheta), 5.0);
}

float3 F_SchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	float s = 1.0 - roughness;
	return F0 + (max(float3(s, s, s), F0) - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

// NDF(Normal Distribution Function): Trowbridge-Reitz GGX 
float D_TrowbridgeReitzGGX(float3 N, float3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NoH = max(dot(N, H), 0.0);
	float NoH2 = NoH * NoH;
	
	float numerator = a2;
	float denominator = (NoH2 * (a2 - 1.0) + 1.0);
	denominator = PI * denominator * denominator;
	return numerator / denominator;
}

// [Blinn 1977, "Models of light reflection for computer synthesized pictures"]
float D_Blinn(float3 N, float3 V, float3 L, float roughness)
{
	float3 H = normalize(V + L);
	float NoH = max(0.0, dot(N, H));
	return pow(NoH, roughness);
}

float D_Charlie(float roughness, float NoH) 
{
	// Estevez and Kulla 2017, "Production Friendly Microfacet Sheen BRDF"
	// For Sheen, Cloth
	float invAlpha  = 1.0 / roughness;
	float cos2h = NoH * NoH;
	float sin2h = max(1.0 - cos2h, 0.0078125); // 2^(-14/2), so sin2h^2 > 0 in fp16
	return (2.0 + invAlpha) * pow(sin2h, invAlpha * 0.5) / (2.0 * PI);
}


// Geometry: Smith GGX (used in UE4)
float G_SchlickGGX(float NoV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;
	
	float numerator = NoV;
	float denominator = NoV * (1.0 - k) + k;
	return numerator / denominator;
}

float G_SmithGGX(float3 N, float3 V, float3 L, float roughness)
{
	float NoV = max(dot(N, V), 0.0);
	float NoL = max(dot(N, L), 0.0);
	float ggx1 = G_SchlickGGX(NoL, roughness);
	float ggx2 = G_SchlickGGX(NoV, roughness);
	return ggx1 * ggx2;
}

float G_SchlickGGX_IBL(float NoV, float roughness)
{
	float k = (roughness * roughness) * 0.5;
	
	float numerator = NoV;
	float denominator = NoV * (1.0 - k) + k;
	return numerator / denominator;
}

float G_Smith_IBL(float3 N, float3 V, float3 L, float roughness)
{
	float NoV = max(dot(N, V), 0.0);
	float NoL = max(dot(N, L), 0.0);
	float ggx1 = G_SchlickGGX_IBL(NoL, roughness);
	float ggx2 = G_SchlickGGX_IBL(NoV, roughness);
	return ggx1 * ggx2;
}

float G_Kelemen(float LoH)
{
	// Kelemen 2001, "A Microfacet Based Coupled Specular-Matte BRDF Model with Importance Sampling"
	// For ClearCoat
	return saturate(0.25 / (LoH * LoH));
}

float3 CalcSpecularBRDF(float3 L, float3 N, float3 V, float3 albedo, float metallic, float roughness)
{
	float3 H = normalize(L + V);
	
	float3 specularColor = CalcSpecularColor(albedo, metallic);
    float3 F0 = specularColor;
	
	float D = D_TrowbridgeReitzGGX(N, H, roughness);
    float G = G_SmithGGX(N, V, L, roughness);
    float3 F = F_Schlick(max(dot(H, V), 0.0), F0);
	
	float3 numerator = D * G * F;
    float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    float3 specular = numerator / denominator;
	
	return specular;
}
float3 CalcDiffuseBRDF(float3 L, float3 N, float3 V, float3 albedo, float metallic, float roughness)
{	
	float3 diffuseColor = CalcDiffuseColor(albedo, metallic);
	float3 diffuse = diffuseColor * Diffuse_Lambertian();
	return diffuse;
}
