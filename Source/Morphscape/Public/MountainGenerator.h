// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "MountainGenerator.generated.h"

UCLASS()
class MORPHSCAPE_API AMountainGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMountainGenerator();

protected:
    virtual void BeginPlay() override;

public:
    void GenerateTerrain();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Settings")
    int32 TerrainSize = 100; // Grid Size

	//not yet used
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Settings")
    float GridSpacing = 100.0f; // Distance between points

	//not yet used
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Settings")
    int32 Seed = 0; // Randomization Seed

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Procedural Settings")
    UMaterialInterface* MasterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Settings")
    float MinRidgeDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Settings")
    float MaxRidgeDistance = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Settings")
    float PeakHeight = 100.0f; // Adjustable peak height

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Settings")
    float Falloff = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mountain Settings")
    float FalloffExponent = 2.0f;

private:
    UPROPERTY(VisibleAnywhere)
    UProceduralMeshComponent* ProceduralMesh;

    TArray<FVector> Vertices;
    TArray<int32> Triangles;
    TArray<FVector2D> UVs;
    TArray<FVector> Normals;
    TArray<FProcMeshTangent> Tangents;
};
