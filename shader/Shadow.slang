module Shadow;

import Common;
import Lighting;

public Texture2D shadowTex;
public TextureCube cubeShadowTex;

#define USE_LIGHT_SHADOWMAP_INDEX -1

public float CalcSpotDirectionalLightShadow(LightInfo lightInfo, float3 posWorld, int shadow_channel = USE_LIGHT_SHADOWMAP_INDEX)
{
	public float4 posInLightSpace = mul(float4(posWorld, 1.0), lightInfo.lightViewProj);
	public float3 projCoords = posInLightSpace.xyz / posInLightSpace.w;
	projCoords.xy = TexCoordFromPosNDC(projCoords.xyz);
	public float current_depth = projCoords.z;
	public float bias = lightInfo.shadowBias;
	public float shadow = 0.0;
	public int channel = shadow_channel == USE_LIGHT_SHADOWMAP_INDEX ? lightInfo.shadowMapIndex : shadow_channel;
	[branch]
	if (lightInfo.useSoftShadow == 0)
	{
		float closest_depth = shadowTex.SampleLevel(shadow_map_sampler, projCoords.xy, 0)[channel];
		shadow = current_depth - bias > closest_depth ? 1.0 : 0.0;
	}
	else
	{
		float w;
		float h;
		shadowTex.GetDimensions(w, h);
		float2 inv_texel_size = float2(1 / w, 1 / h);
		[unroll]
		for (int x = -1; x <= 1; ++x)
		{
			[unroll]
			for (int y = -1; y <= 1; ++y)
			{
				float closest_depth = shadowTex.SampleLevel(shadow_map_sampler, projCoords.xy + float2(x, y) * inv_texel_size, 0)[channel];
				shadow += current_depth - bias > closest_depth ? 1.0 : 0.0;
			}
		}
		shadow /= 9.0;
	}
	shadow = current_depth > 1.0 ? 0.0 : shadow;
	return shadow;
}

public float CalcPointLightShadow(CameraInfo cameraInfo, LightInfo lightInfo, float3 posWorld)
{
	public float3 fragToLight = posWorld - lightInfo.posWorld;
    public float far_plane = cameraInfo.farPlane;
	public float current_depth = length(fragToLight);
	
	public float shadow = 0.0;
	public float bias = lightInfo.shadowBias;
	[branch]
	if (lightInfo.useSoftShadow == 0)
	{
		float depth = cubeShadowTex.SampleLevel(shadow_map_sampler, fragToLight, 0).r;
		float closest_depth = depth * far_plane;
		shadow = current_depth - bias > closest_depth ? 1.0 : 0.0;
	}
	else
	{
		int samples = 20;
		float3 viewPos = cameraInfo.posWorld;
		float viewDistance = length(viewPos - posWorld);
		float diskRadius = (1.0 + (viewDistance / far_plane)) / 50.0;
		float3 gridSamplingDisk[20] = {
		   float3(1, 1,  1), float3(1, -1,  1), float3(-1, -1,  1), float3(-1, 1,  1),
		   float3(1, 1, -1), float3(1, -1, -1), float3(-1, -1, -1), float3(-1, 1, -1),
		   float3(1, 1,  0), float3(1, -1,  0), float3(-1, -1,  0), float3(-1, 1,  0),
		   float3(1, 0,  1), float3(-1,  0,  1), float3(1,  0, -1), float3(-1, 0, -1),
		   float3(0, 1,  1), float3(0, -1,  1), float3(0, -1, -1), float3(0, 1, -1)
		};
		[unroll]
		for (int i = 0; i < samples; ++i)
		{
			float depth = cubeShadowTex.SampleLevel(shadow_map_sampler, fragToLight + gridSamplingDisk[i] * diskRadius, 0).r;
			float closest_depth = depth * far_plane;
			shadow += current_depth - bias > closest_depth ? 1.0 : 0.0;
		}
		shadow /= float(samples);
	}
	return shadow;
}
public float CalcShadow(CameraInfo cameraInfo, LightInfo lightInfo, float3 posWorld)
{
	[branch]
    if (lightInfo.type == LIGHT_TYPE_DIRECTIONAL || lightInfo.type == LIGHT_TYPE_SPOT)
        return CalcSpotDirectionalLightShadow(lightInfo, posWorld);
    else if (lightInfo.type == LIGHT_TYPE_POINT)
        return CalcPointLightShadow(cameraInfo, lightInfo, posWorld);
    else
        return 0.0;
}
