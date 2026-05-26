module Lighting;


import Common;
import BRDF;

public float GetDistanceAttenuation(float3 distanceToLight, float falloffRadius)
{
	// Square Attenuation Algorithm from Filament: https://google.github.io/filament/Filament.html
    public float invRadius = 1.0 / falloffRadius;	
    public float distanceSquare = dot(distanceToLight, distanceToLight);
    public float factor = distanceSquare * invRadius * invRadius;
    public float smoothFactor = max(1.0 - factor * factor, 0.0);
    return (smoothFactor * smoothFactor) / max(distanceSquare, 1e-4);
}

public float GetSpotAngleAttenuation(float3 L, float3 lightDirection, float2 lightInOutCutoff)
{
    public float theta = dot(lightDirection, -L);
	return smoothstep(lightInOutCutoff.y, lightInOutCutoff.x, theta);
}

public float3 CalcLightingPhong(MaterialParam materialParam, LightInfo lightInfo, float lightAttenuation,
                           float3 N, float3 L, float3 V, float shadow)
{
    public float3 albedo = materialParam.albedo;
    public float3 H = normalize(L + V);

    // diffuse
	public float diffuse = max(dot(N, L), 0.0);
	
	// specular
	public float specular = pow(max(0.0, dot(H, N)), 16);

    // 0.5 : specular strenght
    return albedo * lightInfo.color * (diffuse + specular * 0.5) * (1.0 - shadow) * lightAttenuation * lightInfo.intensity;
}

// N, L, V must be normalized, and start point is fragment
public float3 CalcLightingBRDF(float3 albedo, float metallic, float roughness, float3 light_color, float light_intensity, float lightAttenuation, float3 N, float3 L, float3 V, float shadow)
{
	public float NoL = max(dot(N, L), 0.0);
	
	public float3 specular = CalcSpecularBRDF(L, N, V, albedo, metallic, roughness);    	
	public float3 diffuse = CalcDiffuseBRDF (L, N, V, albedo, metallic, roughness);		
    
    public float3 Lo = ((diffuse + specular) * light_color * NoL) * (1.0 - shadow) * light_intensity * lightAttenuation;
    return Lo;
}

public float3 CalcLightingBRDF(MaterialParam materialParam, LightInfo lightInfo, float lightAttenuation,
                           float3 N, float3 L, float3 V, float shadow)
{
    public float3 albedo = materialParam.albedo;
	public float metallic = materialParam.metallic;
	public float roughness = materialParam.roughness;
	return CalcLightingBRDF(albedo, metallic, roughness, lightInfo.color, lightInfo.intensity, lightAttenuation, N, L, V, shadow);
}

public float3 CalcLightingIBL(MaterialParam materialParam, LightInfo lightInfo, float ambientOcclusion)
{
    return ambientOcclusion * materialParam.albedo * lightInfo.color;
}

public void AddEmissive(float3 emissive, inout float3 color)
{
	// to do: calc camera's exposure
	color.rgb += emissive;
}
