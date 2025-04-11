#pragma once
#include "CoreMinimal.h"
#include "ScreenPass.h"
#include "SceneTexturesConfig.h"


class FFullScreenPassVS : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FFullScreenPassVS);

	FFullScreenPassVS() = default;
	FFullScreenPassVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer) : FGlobalShader(Initializer) {}
};

class FFullScreenPassPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FFullScreenPassPS);
	SHADER_USE_PARAMETER_STRUCT(FFullScreenPassPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		RENDER_TARGET_BINDING_SLOTS()
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		SHADER_PARAMETER_RDG_UNIFORM_BUFFER(FSceneTextureUniformParameters, SceneTexturesStruct)
		SHADER_PARAMETER(float, Strenght)
	END_SHADER_PARAMETER_STRUCT()
};

class FRetroFogPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FRetroFogPS);
	SHADER_USE_PARAMETER_STRUCT(FRetroFogPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		RENDER_TARGET_BINDING_SLOTS()
		SHADER_PARAMETER_STRUCT_REF(FViewUniformShaderParameters, View)
		SHADER_PARAMETER_RDG_UNIFORM_BUFFER(FSceneTextureUniformParameters, SceneTexturesStruct)
		SHADER_PARAMETER(float, fOpacity)
		SHADER_PARAMETER(float, fQuantize)
		SHADER_PARAMETER(float, fStart)
		SHADER_PARAMETER(FVector2f, f2Curve)
		SHADER_PARAMETER(FVector3f, f3Color)
		SHADER_PARAMETER(uint32, bAutoColor)
		SHADER_PARAMETER(uint32, bDithering)
		SHADER_PARAMETER(uint32, bCurved)
	END_SHADER_PARAMETER_STRUCT()
};