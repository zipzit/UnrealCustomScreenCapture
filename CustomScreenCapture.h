 // ref: https://forums.unrealengine.com/development-discussion/c-gameplay-programming/1627098-get-image-from-scenecapture2d

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Actor.h"
#include "Engine/Classes/Camera/CameraComponent.h"
#include "Engine/Classes/Components/SceneCaptureComponent2D.h"
#include "Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "CustomScreenCapture.generated.h"

    UCLASS()
class BP_3RDPERSON_MEOW_API ACustomScreenCapture : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    ACustomScreenCapture();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    // Create the image in texture
    void FillTexture();
    void TheHugeManateeTickGetData();

    // Textures need to be power of 2
    // Texture has to be a square
    uint32_t internResolution;

    UTextureRenderTarget2D* renderTarget;
    class USceneCaptureComponent2D* sceneCapture;
    class UCameraComponent* OurCamera;
    uint32_t tickCount;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
    UPROPERTY(EditAnywhere, Category = "Output Information", meta = (ClampMin = "32", ClampMax = "4096", UIMin = "32", UIMax = "4096"))
        uint32 resolutionX;

    UPROPERTY(EditAnywhere, Category = "Output Information", meta = (ClampMin = "32", ClampMax = "4096", UIMin = "32", UIMax = "4096"))
        uint32 resolutionY;

    UPROPERTY(EditAnywhere, Category = "Output Information", meta = (ClampMin = "20.0", ClampMax = "179.9", UIMin = "20.0", UIMax = "179.9"))
        float field_of_view;
};

