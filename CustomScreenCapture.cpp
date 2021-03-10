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
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    ourCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewportCamera"));
    ourCamera->SetupAttachment(RootComponent);
}

void ACustomScreenCapture::BeginPlay()
{
    Super::BeginPlay();

    // Resolution has to be a power of 2. This code finds the lowest RxR resolution 
    // which has equal or more pixels than requested.
    uint32_t higher = std::max(resolutionX, resolutionY);

    /* higher--;
     higher |= higher >> 1;
     higher |= higher >> 2;
     higher |= higher >> 4;
     higher |= higher >> 8;
     higher |= higher >> 16;
     higher++;*/

     //internResolution = higher;
    internResolution = SmallestPowerOf2_GE_N(higher);

    printFString("internResolution is: %d", internResolution, 5.f);

    ourCamera->FieldOfView = field_of_view;
    renderTarget = NewObject<UTextureRenderTarget2D>();
    renderTarget->InitCustomFormat(internResolution, internResolution, EPixelFormat::PF_B8G8R8A8, true);  // true
    //  PF_B8G8R8A8   PF_FloatRGB      PF_G8=exception thrown  PF_G16=Exception Thrown    PF_Unknown=Exception Thrown
    renderTarget->UpdateResourceImmediate();

    sceneCapture = NewObject<USceneCaptureComponent2D>();
    sceneCapture->CaptureSource = SCS_FinalColorLDR;     // what does this line do?

    sceneCapture->TextureTarget = renderTarget;
    sceneCapture->SetupAttachment(ourCamera);
    sceneCapture->bCaptureEveryFrame = true;

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
    // this if loop fails.  What happens is after a few frames, renderTarget exists, but has no/odd content. 
    // and RenderTargetResource ends up as null. 
    //if (renderTarget == nullptr) { ...}
    renderTarget = NewObject<UTextureRenderTarget2D>();
    renderTarget->InitCustomFormat(internResolution, internResolution, EPixelFormat::PF_B8G8R8A8, true);   // true bInForceLinearGamma
    renderTarget->UpdateResourceImmediate();  // what does this line do?

    sceneCapture->TextureTarget = renderTarget;

    //sceneCapture->CaptureScene();   // This line generates a warning:  Scene Capture with bCaptureEveryFrame enabled --> major inefficiency... 
    //sceneCapture->UpdateContent();  // This should not be used if bCaptureEveryFrame is enabled, or the scene capture will render redundantly. 

    auto RenderTargetResource = renderTarget->GameThread_GetRenderTargetResource();

    if (RenderTargetResource) {
        TArray<FColor> buffer;
        RenderTargetResource->ReadPixels(buffer);

        printFString("tickCount: %d", tickCount, 0.05f);
        //printFString("Buffer Size: %d", buffer.Num(), 1.0f);

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