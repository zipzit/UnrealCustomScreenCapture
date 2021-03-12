
#include "CustomScreenCapture.h"
#include <algorithm>    // max()
#include <fstream>

// define a print message function to print to screen
#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Green,text)  // display for 1.5 seconds
#define printFString(text, fstring, floatDisplayTime) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, float(floatDisplayTime), FColor::Magenta, FString::Printf(TEXT(text), fstring))  // display for x seconds

// UE_LOG message with arguments
//int intVar = 5;  //float floatVar = 3.7f;    //FString fstringVar = "an fstring variable";
//UE_LOG(LogTemp, Warning, TEXT("Text, %d %f %s"), intVar, floatVar, *fstringVar);

inline const uint32_t SmallestPowerOf2_GE_N(uint32_t N)
{
    N--;
    N |= N >> 1;
    N |= N >> 2;
    N |= N >> 4;
    N |= N >> 8;
    N |= N >> 16;
    N++;
    return N;
}

// Sets default values
ACustomScreenCapture::ACustomScreenCapture()
    : resolutionX(1024)
    , resolutionY(1024)
    , field_of_view(90.0f)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;  // Perform tick after all updates, movement, physics is complete
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    ourCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewportCamera"));
    ourCamera->SetupAttachment(RootComponent);
    sceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));   // *****
    sceneCapture->SetupAttachment(ourCamera);
}

void ACustomScreenCapture::BeginPlay()
{
    Super::BeginPlay();

    // Resolution has to be a power of 2. 
    // Finds the lowest RxR resolution of equal size or larger.
    uint32_t higher = std::max(resolutionX, resolutionY);
    internResolution = SmallestPowerOf2_GE_N(higher);
    printFString("internResolution is: %d", internResolution, 5.f);

    ourCamera->FieldOfView = field_of_view;
    sceneCapture->FOVAngle = field_of_view;
    renderTarget = NewObject<UTextureRenderTarget2D>();
    renderTarget->InitCustomFormat(internResolution, internResolution, EPixelFormat::PF_B8G8R8A8, true);  // some testing with EPixelFormat::PF_FloatRGBA, true=force Linear Gamma
    //renderTarget->InitCustomFormat(internResolution, internResolution, EPixelFormat::PF_FloatRGBA, true);  // some testing with EPixelFormat::PF_FloatRGBA, true
    renderTarget->UpdateResourceImmediate();

    //sceneCapture = NewObject<USceneCaptureComponent2D>();   // ***** moved to constructor
    sceneCapture->CaptureSource = SCS_FinalColorLDR;     // SCS_FinalColorLDR allows for post processing on the image.  default = SCS_SceneColorHDR
    sceneCapture->TextureTarget = renderTarget;
    sceneCapture->bCaptureEveryFrame = true;
    sceneCapture->bAlwaysPersistRenderingState = true;  // test this line?
    //sceneCapture->UpdateDrawFrustum();  // test this line?
    tickCount = 0;
}

// Called every frame
void ACustomScreenCapture::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    tickCount++;
    //if (tickCount == 10) { ...  }    //Wait for tenth tic...
    FillTexture();
    printFString("tickCount: %d", tickCount, 0.05f);
}

void ACustomScreenCapture::FillTexture()
{
    //if (renderTarget == nullptr) {
    //    print("oops. renderTarget was null...");
    //    renderTarget = NewObject<UTextureRenderTarget2D>();
    //    renderTarget->InitCustomFormat(internResolution, internResolution, EPixelFormat::PF_B8G8R8A8, true);  // some testing with EPixelFormat::PF_FloatRGBA, true
    //    //renderTarget->InitCustomFormat(internResolution, internResolution, EPixelFormat::PF_FloatRGBA, false);   //PF_B8G8R8A8   true bInForceLinearGamma
    //    renderTarget->UpdateResourceImmediate();  // what does this line do?
    //}

    // test here...  I want to see if this color makes it to the buffer array...
    //renderTarget->ClearColor = FLinearColor::Green;
    //renderTarget->UpdateResourceImmediate();

    //UTexture2D* imageRendered = sceneCapture->TextureTarget->ConstructTexture2D(this, "CameraImage", EObjectFlags::RF_NoFlags, CTF_DeferCompression);
    // note: ConstructTexture2D only works in Editor mode, the render target size must be a square, and the size of the square must be a factor of 2
    //sceneCapture->TextureTarget = renderTarget;
    //sceneCapture->CaptureScene();   // This line generates a warning:  Scene Capture with bCaptureEveryFrame enabled --> major inefficiency... 
    //sceneCapture->UpdateContent();  // This should not be used if bCaptureEveryFrame is enabled, or the scene capture will render redundantly. 


    //auto RenderTargetResource = renderTarget->GameThread_GetRenderTargetResource();  // type FRenderTarget*
    //auto RenderTargetResource = renderTarget->GetRenderTargetResource();  // type FRenderTarget*
    //auto RenderTargetResource = sceneCapture->TextureTarget->GetRenderTargetResource();  // type FRenderTarget*
    //FRenderTarget* RenderTargetResource = sceneCapture->TextureTarget->GameThread_GetRenderTargetResource();
    //FTextureRenderTargetResource* RenderTargetResource = sceneCapture->TextureTarget->GameThread_GetRenderTargetResource();
    auto RenderTargetResource = renderTarget->GameThread_GetRenderTargetResource();

    if (RenderTargetResource) {
        TArray<FColor> buffer8;
        RenderTargetResource->ReadPixels(buffer8);
      /*  TArray<FFloat16Color> buffer2;
        RenderTargetResource->ReadFloat16Pixels(buffer2);*/

        printFString("if (RenderTargetResource) tickCount: %d", tickCount, 0.05f);

        // process data with OpenCV
        /*cv::Mat wrappedImage(RenderTarget->GetSurfaceHeight(), RenderTarget->GetSurfaceWidth(), CV_8UC4,
            buffer.GetData());
        std::string OutputFile(TCHAR_TO_UTF8(*OutputVideoFile));
        cv::imwrite(OutputFile, wrappedImage);*/

        // or save data to file (testing only...)
        ////// ref: http://www.cplusplus.com/forum/general/22669/
        //std::ofstream datFile;
        //datFile.open("test.dat", std::ofstream::binary);
        //datFile.write((char*)buffer, buffer.length());
        //datFile.close();
    }
}
