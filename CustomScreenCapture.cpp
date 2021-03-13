
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
    , outputFolderPath(TEXT("C:/Temp/"))   // was "."
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;  // Perform tick after all updates, movement, physics is complete
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    ourCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewportCamera"));
    ourCamera->SetupAttachment(RootComponent);
    sceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));   // *****
    sceneCapture->SetupAttachment(ourCamera);

    baseFilename = "ScreenTest";
}

void ACustomScreenCapture::BeginPlay()
{
    Super::BeginPlay();
    basePathFolder = std::string(TCHAR_TO_UTF8(*outputFolderPath));
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    PlatformFile.CreateDirectory(*outputFolderPath);
    if (basePathFolder.back() != '/')
        basePathFolder.append("/");

    // Resolution has to be a power of 2. 
    // Finds the lowest RxR resolution of equal size or larger.
    uint32_t higher = std::max(resolutionX, resolutionY);
    internResolution = SmallestPowerOf2_GE_N(higher);
    printFString("internResolution is: %d", internResolution, 5.f);

    ourCamera->FieldOfView = field_of_view;
    sceneCapture->FOVAngle = field_of_view;
    renderTarget = NewObject<UTextureRenderTarget2D>();
    renderTarget->InitCustomFormat(internResolution, internResolution, EPixelFormat::PF_B8G8R8A8, true);  // some testing with EPixelFormat::PF_FloatRGBA, true=force Linear Gamma
    renderTarget->UpdateResourceImmediate();

    sceneCapture->CaptureSource = SCS_FinalColorLDR;     // SCS_FinalColorLDR allows for post processing on the image.  default = SCS_SceneColorHDR
    sceneCapture->TextureTarget = renderTarget;
    sceneCapture->bCaptureEveryFrame = true;
    sceneCapture->bAlwaysPersistRenderingState = true;  // This allows velocities for Motion Blur and Temporal AA to be computed.
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

        baseFilename = basePathFolder + std::string("image");
        baseFilename += std::string("_number_");
        std::string fileName = baseFilename + std::to_string(tickCount) + std::string(".bgr8");
        std::ofstream targetFile(fileName, std::ofstream::binary);
        targetFile.write(reinterpret_cast<char*>(buffer8.GetData()), buffer8.Num() * sizeof(FColor));
        targetFile.close();
        // quick file inspect at https://www.onlinehexeditor.com/ or https://hexed.it/
        // debugger inspect is totally functional, too...
    }
}
