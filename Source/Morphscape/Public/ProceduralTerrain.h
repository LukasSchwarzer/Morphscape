#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DArray.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
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
    
    UFUNCTION(BlueprintCallable, Category = "Procedural Generation")
    void GenerateTerrain();
    void ApplyNoise();
    void SimulateErosion();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Settings")
    int32 TerrainSize = 100; // Grid Size

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Settings")
    float GridSpacing = 100.0f; // Distance between points

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Settings")
    float NoiseScale = 20.0f; // Height variation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Settings")
    int32 Seed = 0; // Randomization Seed

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Settings")
    UMaterialInterface* MasterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaussian Settings")
    float GaussianScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gaussian Settings")
    int32 NumSpots = 10;

    UPROPERTY(VisibleAnywhere)
    UProceduralMeshComponent* ProceduralMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StaticMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    UStaticMesh* GeneratedStaticMesh;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    UPCGGraph* PCGGraph;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG")
    UPCGComponent* PCGComponent;

private:

    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector2D> UVs;
    TArray<FVector> Normals;
    TArray<FProcMeshTangent> Tangents;

    UStaticMesh* ConvertProceduralMeshToStaticMesh();

};
