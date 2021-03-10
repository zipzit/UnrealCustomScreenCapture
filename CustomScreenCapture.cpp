#include "CustomScreenCapture.h"
#include <algorithm>    // max()
#include <fstream>

// define a print message function to print to screen
#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Green,text)  // display for 1.5 seconds
#define printFString(text, fstring, floatDisplayTime) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, float(floatDisplayTime), FColor::Magenta, FString::Printf(TEXT(text), fstring))  // display for x seconds

//// UE_LOG message with arguments
//int intVar = 5;
//float floatVar = 3.7f;
//FString fstringVar = "an fstring variable";
//UE_LOG(LogTemp, Warning, TEXT("Text, %d %f %s"), intVar, floatVar, *fstringVar);


// Sets default values
ACustomScreenCapture::ACustomScreenCapture()
    : resolutionX(1024)   // 1024 x 1024... 1027 = bomb
    , resolutionY(1024)
    , field_of_view(90.0f)
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    OurCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewportCamera"));
    OurCamera->SetupAttachment(RootComponent);
}

void ACustomScreenCapture::BeginPlay()
{
    Super::BeginPlay();

    // Resolution has to be a power of 2. This code finds the lowest RxR resolution which has equal or more pixel than set
    uint32_t higher = std::max(resolutionX, resolutionY);

    higher--;
    higher |= higher >> 1;
    higher |= higher >> 2;
    higher |= higher >> 4;
    higher |= higher >> 8;
    higher |= higher >> 16;
    higher++;

    internResolution = higher;

    printFString("internResolution is: %d", internResolution, 5.f);  // prints 1024 as expected...

    OurCamera->FieldOfView = field_of_view;
    renderTarget = NewObject<UTextureRenderTarget2D>();
    renderTarget->InitCustomFormat(internResolution, internResolution, EPixelFormat::PF_B8G8R8A8, true);

    renderTarget->UpdateResourceImmediate();

    sceneCapture = NewObject<USceneCaptureComponent2D>();
    sceneCapture->CaptureSource = SCS_FinalColorLDR;

    sceneCapture->TextureTarget = renderTarget;
    sceneCapture->SetupAttachment(OurCamera);
    sceneCapture->bCaptureEveryFrame = true;

    tickCount = 0;
}


// Called every frame
void ACustomScreenCapture::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    //Wait for tenth tic...
    tickCount++;
//if (tickCount == 10) {
        //FillTexture();
        TheHugeManateeTickGetData();
  //  }

    printFString("tickCount: %d", tickCount, 0.05f);

}

void ACustomScreenCapture::FillTexture()
{
    sceneCapture->TextureTarget = renderTarget;
    auto RenderTargetResource = renderTarget->GameThread_GetRenderTargetResource();

    if (RenderTargetResource)
    {
        TArray<FColor> buffer8;
        RenderTargetResource->ReadPixels(buffer8);

        // UE_LOG(LogTemp, Warning, TEXT("Fill Texture.. This line is for break point reasons"));

         //// ref: http://www.cplusplus.com/forum/general/22669/
         //std::ofstream datFile;
         //datFile.open("test.dat", std::ofstream::binary);
         //datFile.write((char*)buffer8, buffer8.length());
         //datFile.close();

    }
}

void ACustomScreenCapture::TheHugeManateeTickGetData() {
   //if (renderTarget == nullptr) {
        printFString("Inside TheHugeManateTickGet Data() no render target found... tickCount: %d", tickCount, 10.05f);
        renderTarget = NewObject<UTextureRenderTarget2D>();
        renderTarget->InitCustomFormat(1024, 1024, EPixelFormat::PF_B8G8R8A8, true);
        renderTarget->UpdateResourceImmediate();
        sceneCapture->TextureTarget = renderTarget;
   // }

    //sceneCapture->CaptureScene();   // This line generates a warning:  Scene Capture with bCaptureEveryFrame enabled --> major inefficiency... 
    //sceneCapture->UpdateContent();    // This should not be used if bCaptureEveryFrame is enabled, or the scene capture will render redundantly. 

    auto RenderTargetResource = renderTarget->GameThread_GetRenderTargetResource();

    if (RenderTargetResource) {
        TArray<FColor> buffer;
        RenderTargetResource->ReadPixels(buffer);

        printFString("GetSurfaceHeight: %d", renderTarget->GetSurfaceHeight(), 10.05f);  // displays 0
        printFString("GetSurfaceWidth: %d", renderTarget->GetSurfaceWidth(), 10.05f);


        /*cv::Mat wrappedImage(RenderTarget->GetSurfaceHeight(), RenderTarget->GetSurfaceWidth(), CV_8UC4,
            buffer.GetData());

        std::string OutputFile(TCHAR_TO_UTF8(*OutputVideoFile));
        cv::imwrite(OutputFile, wrappedImage);*/

        //// ref: http://www.cplusplus.com/forum/general/22669/
         std::ofstream datFile;
         datFile.open("test.dat", std::ofstream::binary);
         datFile.write((char*)buffer, buffer.length());
         datFile.close();
    }
}
