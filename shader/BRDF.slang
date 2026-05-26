module BRDF;


import Common;
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
public float3 CalcDiffuseColor(float3 albedo, float metallic)
{
	return albedo * (1.0 - metallic);
}
public float3 CalcSpecularColor(float3 albedo, float metallic)
{
	return lerp(0.04, albedo, metallic);
}

// Traditional Lambertion diffuse BRDF		
public float3 Diffuse_Lambertian()
{
	return (1.0 / PI);
}

// [Burley 2012, "Physically-Based Shading at Disney"]
public float Diffuse_Burley(float3 N, float3 L, float3 V, float roughness)
{			
	public float NoL = max(0.0, dot(N, L));
	public float NoV = max(0.0, dot(N, V));
	
	public float3 H = normalize(L + V);
	public float VoH = dot(V, H);
	public float F_D90 = 0.5 + 2.0 * VoH * VoH * roughness;

	return 1.0 / PI * (1.0f + (F_D90 - 1.0f) * pow(1.0 - NoL, 5.0)) *
					  (1.0f + (F_D90 - 1.0f) * pow(1.0 - NoV, 5.0));
}

// [Gotanda 2012, "Beyond a Simple Physically Based Blinn-Phong Model in Real-Time"]
public float Diffuse_OrenNayar(float3 N, float3 L, float3 V, float roughness, float albedo) 
{ 
  public float LoV = dot(L, V);
  public float NoL = dot(N, L);
  public float NoV = dot(N, V);

  public float s = LoV - NoL * NoV;
  public float t = lerp(1.0, max(NoL, NoV), step(0.0, s));

  public float sigma2 = roughness * roughness;
  public float A = 1.0 + sigma2 * (albedo / (sigma2 + 0.13) + 0.5 / (sigma2 + 0.33));
  public float B = 0.45 * sigma2 / (sigma2 + 0.09);

  return albedo * max(0.0, NoL) * (A + B * s / t) / PI;
}

//------------------------------------------------------------------------------
// Specular BRDF implementations
//------------------------------------------------------------------------------
// Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
public float3 F_Schlick(float cosTheta, float3 F0)
{
	public float Fc = pow(saturate(1.0 - cosTheta), 5.0);
	return Fc + F0 * (1.0 - Fc);		
}

public float3 F_Schlick(float cosTheta, float3 F0, float F90) 
{
	return F0 + (F90 - F0) *  pow(saturate(1.0 - cosTheta), 5.0);
}

public float3 F_SchlickRoughness(float cosTheta, float3 F0, float roughness)
{
	public float s = 1.0 - roughness;
	return F0 + (max(float3(s, s, s), F0) - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

// NDF(Normal Distribution Function): Trowbridge-Reitz GGX 
public float D_TrowbridgeReitzGGX(float3 N, float3 H, float roughness)
{
	public float a = roughness * roughness;
	public float a2 = a * a;
	public float NoH = max(dot(N, H), 0.0);
	public float NoH2 = NoH * NoH;
	
	public float numerator = a2;
	public float denominator = (NoH2 * (a2 - 1.0) + 1.0);
	denominator = PI * denominator * denominator;
	return numerator / denominator;
}

// [Blinn 1977, "Models of light reflection for computer synthesized pictures"]
public float D_Blinn(float3 N, float3 V, float3 L, float roughness)
{
	public float3 H = normalize(V + L);
	public float NoH = max(0.0, dot(N, H));
	return pow(NoH, roughness);
}

public float D_Charlie(float roughness, float NoH) 
{
	// Estevez and Kulla 2017, "Production Friendly Microfacet Sheen BRDF"
	// For Sheen, Cloth
	public float invAlpha  = 1.0 / roughness;
	public float cos2h = NoH * NoH;
	public float sin2h = max(1.0 - cos2h, 0.0078125); // 2^(-14/2), so sin2h^2 > 0 in fp16
	return (2.0 + invAlpha) * pow(sin2h, invAlpha * 0.5) / (2.0 * PI);
}


// Geometry: Smith GGX (used in UE4)
public float G_SchlickGGX(float NoV, float roughness)
{
	public float r = (roughness + 1.0);
	public float k = (r * r) / 8.0;
	
	public float numerator = NoV;
	public float denominator = NoV * (1.0 - k) + k;
	return numerator / denominator;
}

public float G_SmithGGX(float3 N, float3 V, float3 L, float roughness)
{
	public float NoV = max(dot(N, V), 0.0);
	public float NoL = max(dot(N, L), 0.0);
	public float ggx1 = G_SchlickGGX(NoL, roughness);
	public float ggx2 = G_SchlickGGX(NoV, roughness);
	return ggx1 * ggx2;
}

public float G_SchlickGGX_IBL(float NoV, float roughness)
{
	public float k = (roughness * roughness) * 0.5;
	
	public float numerator = NoV;
	public float denominator = NoV * (1.0 - k) + k;
	return numerator / denominator;
}

public float G_Smith_IBL(float3 N, float3 V, float3 L, float roughness)
{
	public float NoV = max(dot(N, V), 0.0);
	public float NoL = max(dot(N, L), 0.0);
	public float ggx1 = G_SchlickGGX_IBL(NoL, roughness);
	public float ggx2 = G_SchlickGGX_IBL(NoV, roughness);
	return ggx1 * ggx2;
}

public float G_Kelemen(float LoH)
{
	// Kelemen 2001, "A Microfacet Based Coupled Specular-Matte BRDF Model with Importance Sampling"
	// For ClearCoat
	return saturate(0.25 / (LoH * LoH));
}

public float3 CalcSpecularBRDF(float3 L, float3 N, float3 V, float3 albedo, float metallic, float roughness)
{
	public float3 H = normalize(L + V);
	
	public float3 specularColor = CalcSpecularColor(albedo, metallic);
    public float3 F0 = specularColor;
	
	public float D = D_TrowbridgeReitzGGX(N, H, roughness);
    public float G = G_SmithGGX(N, V, L, roughness);
    public float3 F = F_Schlick(max(dot(H, V), 0.0), F0);
	
	public float3 numerator = D * G * F;
    public float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    public float3 specular = numerator / denominator;
	
	return specular;
}
public float3 CalcDiffuseBRDF(float3 L, float3 N, float3 V, float3 albedo, float metallic, float roughness)
{	
	public float3 diffuseColor = CalcDiffuseColor(albedo, metallic);
	public float3 diffuse = diffuseColor * Diffuse_Lambertian();
	return diffuse;
}
