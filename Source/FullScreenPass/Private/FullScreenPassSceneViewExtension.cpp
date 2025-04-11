#include "FullScreenPassSceneViewExtension.h"
#include "FullScreenPassShaders.h"

#include "FXRenderingUtils.h"
#include "PostProcess/PostProcessInputs.h"
#include "DynamicResolutionState.h"


static TAutoConsoleVariable<int32> CVarEnabled(
	TEXT("r.FSP"),
	0,
	TEXT("Controls Full Screen Pass plugin\n")
	TEXT(" 0: disabled\n")
	TEXT(" 1: enabled (default)"));

static TAutoConsoleVariable<float> CVarEffectStrenght(
	TEXT("r.FSP.EffectStrenght"),
	0.1f,
	TEXT("Controls an amount of depth buffer blending to the base color."));

// RetroFog console variables
static TAutoConsoleVariable<int32> CVarRetroFogEnabled(
	TEXT("r.RetroFog"),
	1,
	TEXT("Controls Retro Fog effect\n")
	TEXT(" 0: disabled\n")
	TEXT(" 1: enabled"));

static TAutoConsoleVariable<float> CVarRetroFogOpacity(
	TEXT("r.RetroFog.Opacity"),
	1.0f,
	TEXT("Controls fog opacity"));

static TAutoConsoleVariable<float> CVarRetroFogQuantize(
	TEXT("r.RetroFog.Quantize"),
	255.0f,
	TEXT("Controls color quantization"));

static TAutoConsoleVariable<float> CVarRetroFogStart(
	TEXT("r.RetroFog.Start"),
	0.0f,
	TEXT("Controls fog start distance"));

static TAutoConsoleVariable<float> CVarRetroFogCurveX(
	TEXT("r.RetroFog.CurveX"),
	0.0f,
	TEXT("Controls fog curve X parameter"));

static TAutoConsoleVariable<float> CVarRetroFogCurveY(
	TEXT("r.RetroFog.CurveY"),
	1.0f,
	TEXT("Controls fog curve Y parameter"));

static TAutoConsoleVariable<float> CVarRetroFogColorR(
	TEXT("r.RetroFog.ColorR"),
	0.0f,
	TEXT("Controls fog color red component"));

static TAutoConsoleVariable<float> CVarRetroFogColorG(
	TEXT("r.RetroFog.ColorG"),
	0.0f,
	TEXT("Controls fog color green component"));

static TAutoConsoleVariable<float> CVarRetroFogColorB(
	TEXT("r.RetroFog.ColorB"),
	0.0f,
	TEXT("Controls fog color blue component"));

static TAutoConsoleVariable<int32> CVarRetroFogAutoColor(
	TEXT("r.RetroFog.AutoColor"),
	0,
	TEXT("Controls automatic color sampling\n")
	TEXT(" 0: disabled\n")
	TEXT(" 1: enabled"));

static TAutoConsoleVariable<int32> CVarRetroFogDithering(
	TEXT("r.RetroFog.Dithering"),
	1,
	TEXT("Controls dithering\n")
	TEXT(" 0: disabled\n")
	TEXT(" 1: enabled"));

static TAutoConsoleVariable<int32> CVarRetroFogCurved(
	TEXT("r.RetroFog.Curved"),
	1,
	TEXT("Controls curved fog\n")
	TEXT(" 0: disabled\n")
	TEXT(" 1: enabled"));


FFullScreenPassSceneViewExtension::FFullScreenPassSceneViewExtension(const FAutoRegister& AutoRegister) :
	FSceneViewExtensionBase(AutoRegister)
{
}

void FFullScreenPassSceneViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs)
{
	if (CVarEnabled->GetInt() == 0 && CVarRetroFogEnabled->GetInt() == 0)
	{
		return;
	}

	Inputs.Validate();

	const FIntRect PrimaryViewRect = UE::FXRenderingUtils::GetRawViewRectUnsafe(View);

	FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, PrimaryViewRect);

	if (!SceneColor.IsValid())
	{
		return;
	}

	const FScreenPassTextureViewport Viewport(SceneColor);

	FRDGTextureDesc SceneColorDesc = SceneColor.Texture->Desc;
	SceneColorDesc.Format = PF_FloatRGBA;
	FLinearColor ClearColor(0., 0., 0., 0.);
	SceneColorDesc.ClearValue = FClearValueBinding(ClearColor);

	FRDGTexture* ResultTexture = GraphBuilder.CreateTexture(SceneColorDesc, TEXT("FulllScreenPassResult"));
	FScreenPassRenderTarget ResultRenderTarget = FScreenPassRenderTarget(ResultTexture, SceneColor.ViewRect, ERenderTargetLoadAction::EClear);
	
	FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FFullScreenPassVS> ScreenPassVS(GlobalShaderMap);

	if (CVarRetroFogEnabled->GetInt() == 1)
	{
		TShaderMapRef<FRetroFogPS> RetroFogPS(GlobalShaderMap);
		FRetroFogPS::FParameters* Parameters = GraphBuilder.AllocParameters<FRetroFogPS::FParameters>();
		Parameters->View = View.ViewUniformBuffer;
		Parameters->SceneTexturesStruct = Inputs.SceneTextures;
		Parameters->fOpacity = CVarRetroFogOpacity->GetFloat();
		Parameters->fQuantize = CVarRetroFogQuantize->GetFloat();
		Parameters->fStart = CVarRetroFogStart->GetFloat();
		Parameters->f2Curve = FVector2f(CVarRetroFogCurveX->GetFloat(), CVarRetroFogCurveY->GetFloat());
		Parameters->f3Color = FVector3f(CVarRetroFogColorR->GetFloat(), CVarRetroFogColorG->GetFloat(), CVarRetroFogColorB->GetFloat());
		Parameters->bAutoColor = CVarRetroFogAutoColor->GetInt();
		Parameters->bDithering = CVarRetroFogDithering->GetInt();
		Parameters->bCurved = CVarRetroFogCurved->GetInt();
		Parameters->RenderTargets[0] = ResultRenderTarget.GetRenderTargetBinding();

		AddDrawScreenPass(
			GraphBuilder,
			RDG_EVENT_NAME("RetroFogShader"),
			View,
			Viewport,
			Viewport,
			ScreenPassVS,
			RetroFogPS,
			Parameters
		);
	}
	else if (CVarEnabled->GetInt() == 1)
	{
		TShaderMapRef<FFullScreenPassPS> ScreenPassPS(GlobalShaderMap);
		FFullScreenPassPS::FParameters* Parameters = GraphBuilder.AllocParameters<FFullScreenPassPS::FParameters>();
		Parameters->View = View.ViewUniformBuffer;
		Parameters->SceneTexturesStruct = Inputs.SceneTextures;
		Parameters->Strenght = CVarEffectStrenght->GetFloat();
		Parameters->RenderTargets[0] = ResultRenderTarget.GetRenderTargetBinding();

		AddDrawScreenPass(
			GraphBuilder,
			RDG_EVENT_NAME("FullScreenPassShader"),
			View,
			Viewport,
			Viewport,
			ScreenPassVS,
			ScreenPassPS,
			Parameters
		);
	}

	AddCopyTexturePass(GraphBuilder, ResultTexture, SceneColor.Texture);
}