//-------------------------------------------------------
// Input/Output Structs
//-------------------------------------------------------

float4x4 gWorldViewProj : WorldViewProjection;

Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossinessMap : GlossinessMap;

float4x4 gWorld: World;
float4x4 gViewInverse : ViewInverse;

float3 lightDirection = { 0.577f, -0.577f, -0.577f };

bool gFlat : Flat;
float gPi : PI;

struct VS_INPUT
{
	float4 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : WorldPos;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

//-------------------------------------------------------
// Vertex Shader
//-------------------------------------------------------
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	float4 pos = float4(input.Position.xyz, 1.f);
	output.Position = mul(pos, gWorldViewProj);
	float4 worldPos = float4(input.Position.xyz, input.Position.z);
	output.WorldPosition = mul(worldPos, gWorld);
	output.TexCoord = input.TexCoord;
	output.Normal = mul(normalize(input.Normal), (float3x3)gWorld);
	output.Tangent = mul(normalize(input.Tangent), (float3x3)gWorld);
	return output;
}

//-------------------------------------------------------
// Sampling
//-------------------------------------------------------
SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Border;//Or Mirror or Clamp or Border
	AddressV = Clamp;//Or Mirror or Clamp or Border
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Border;//Or Mirror or Clamp or Border
	AddressV = Clamp;//Or Mirror or Clamp or Border
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Border;//Or Mirror or Clamp or Border
	AddressV = Clamp;//Or Mirror or Clamp or Border
	BorderColor = float4(0.0f, 0.0f, 1.0f, 1.0f);
};

//-------------------------------------------------------
// Pixel Shaders
//-------------------------------------------------------
float4 Phong(float4 color, float phongExponent, float3 w0, float3 w1, float3 normal)
{
	float reflecting = saturate(dot(reflect(-w0, normal), w1));
	return color * pow(reflecting, phongExponent);
}

float4 PixelShadingPhong(VS_OUTPUT input, SamplerState sam)
{
	float4 lightColor = { 1.f, 1.f, 1.f, 1.f };
	float intensity = 7.f;

	float4 normalRgb = gNormalMap.Sample(sam, input.TexCoord);
	float3 normal = { normalRgb.x, normalRgb.y, normalRgb.z };
	float3 biNormal = cross(input.Tangent, input.Normal);
	float3x3 tangentSpaceAxis = float3x3(input.Tangent, biNormal, input.Normal);

	normal.x = 2.f * normal.x - 1.f;
	normal.y = 2.f * normal.y - 1.f;
	normal.z = 2.f * normal.z - 1.f;

	normal = mul(normal, tangentSpaceAxis);
	normal = normalize(normal);

	float observedArea = dot(normal, -lightDirection);
	observedArea = saturate(observedArea);
	observedArea /= gPi;

	float4 diffuseColor = gDiffuseMap.Sample(sam, input.TexCoord);

	float shininess = 25.f;

	float3 viewDirection = normalize(input.WorldPosition.xyz - gViewInverse[3].xyz);
	float4 specularColor = gSpecularMap.Sample(sam, input.TexCoord);
	float4 glossinessColor = gGlossinessMap.Sample(sam, input.TexCoord);

	float4 phongColor = Phong(specularColor, shininess * glossinessColor.x, lightDirection, viewDirection, input.Normal);
	
	float4 ambientColor = { 0.025f, 0.025f, 0.025f, 1.0f };
	
	diffuseColor += phongColor + ambientColor;
	float maxValue = max(diffuseColor.r, max(diffuseColor.g, diffuseColor.b));
	if (maxValue > 1.f)
		diffuseColor /= maxValue;


	float4 finalColor = lightColor * intensity * diffuseColor * observedArea;
	maxValue = max(finalColor.r, max(finalColor.g, finalColor.b));
	if (maxValue > 1.f)
		finalColor /= maxValue;
	
	return finalColor;
}

float4 PSPOINT(VS_OUTPUT input) : SV_TARGET
{
	if (gFlat)
	{
		return gDiffuseMap.Sample(samPoint, input.TexCoord);
	}
	return PixelShadingPhong(input, samPoint);
}

float4 PSLINEAR(VS_OUTPUT input) : SV_TARGET
{
	if (gFlat)
	{
		return gDiffuseMap.Sample(samLinear, input.TexCoord);
	}
	return PixelShadingPhong(input, samLinear);
}

float4 PSANISOTROPIC(VS_OUTPUT input) : SV_TARGET
{
	if (gFlat)
	{
		return gDiffuseMap.Sample(samAnisotropic, input.TexCoord);
	}
	return PixelShadingPhong(input, samAnisotropic);
}


//-------------------------------------------------------
// Rasterizer States
//-------------------------------------------------------
//RasterizerState gRasterizerStateBack
//{
//	CullMode = back;
//	FrontCounterClockWise = true;
//};
//
//RasterizerState gRasterizerStateFront
//{
//	CullMode = front;
//	FrontCounterClockWise = true;
//};
//
//RasterizerState gRasterizerStateBoth
//{
//	CullMode = none;
//	FrontCounterClockWise = true;
//};

//-------------------------------------------------------
// DepthStencil States
//-------------------------------------------------------
DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = zero;
	DepthFunc = less;
	StencilEnable = false;

	StencilReadMask = 0x0F;
	StencilWriteMask = 0x0F;

	FrontFaceStencilFunc = always;
	BackFaceStencilFunc = always;

	FrontFaceStencilDepthFail = keep;
	BackFaceStencilDepthFail = keep;

	FrontFaceStencilPass = keep;
	BackFaceStencilPass = keep;

	FrontFaceStencilFail = keep;
	BackFaceStencilFail = keep;
};

BlendState gBlendState
{
	BlendEnable[0] = true;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	BlendOp = add;
	SrcBlendAlpha = zero;
	DestBlendAlpha = zero;
	BlendOpAlpha = add;
	RenderTargetWriteMask[0] = 0x0F;
};

//-------------------------------------------------------
// Techniques
//-------------------------------------------------------
technique11 PointTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSPOINT()));
	}
}

technique11 LinearTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSLINEAR()));
	}
}

technique11 AnisotropicTechnique
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSANISOTROPIC()));
	}
}
