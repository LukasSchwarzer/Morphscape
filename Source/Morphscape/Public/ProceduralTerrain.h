#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DArray.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "ProceduralTerrain.generated.h"

UCLASS()
class MORPHSCAPE_API AProceduralTerrain : public AActor
{
    GENERATED_BODY()
    
public:
    AProceduralTerrain();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    void GenerateTerrain();
    void ApplyNoise();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Settings")
    int32 TerrainSize = 100; // Grid Size

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Settings")
    float GridSpacing = 100.0f; // Distance between points

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Settings")
    float NoiseScale = 20.0f; // Height variation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Settings")
    int32 Seed = 0; // Randomization Seed

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Settings")
	UTexture2D* NoiseTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Settings")
    float TextureTiling = 1.0f; // Number of times the texture tiles across the terrain

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Settings")
    UMaterialInterface* MasterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaussian Settings")
    float GaussianScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaussian Settings")
    int32 NumSpots = 10;

private:
    UPROPERTY(VisibleAnywhere)
    UProceduralMeshComponent* ProceduralMesh;

    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector2D> UVs;
    TArray<FVector> Normals;
    TArray<FProcMeshTangent> Tangents;
};
