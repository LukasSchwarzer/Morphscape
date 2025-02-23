#include "ProceduralTerrain.h"
#include "ProceduralMeshComponent.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h" // For GEngine

AProceduralTerrain::AProceduralTerrain()
{
    PrimaryActorTick.bCanEverTick = true;
    ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
    RootComponent = ProceduralMesh;
}

void AProceduralTerrain::BeginPlay()
{
    Super::BeginPlay();
    GenerateTerrain();
}

void AProceduralTerrain::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AProceduralTerrain::GenerateTerrain()
{
    Vertices.Empty();
    Triangles.Empty();
    UVs.Empty();
    Normals.Empty();
    Tangents.Empty();
    TArray<FColor> VertexColors;

    float MinHeight = FLT_MAX;
    float MaxHeight = -FLT_MAX;

    TArray<FVector2D> SpotCenters;
    TArray<float> SpotHeights;

    // Generate random centers and heights for the spots
    for (int32 i = 0; i < NumSpots; i++)
    {
        SpotCenters.Add(FVector2D(FMath::FRand(), FMath::FRand()));
        SpotHeights.Add(FMath::FRandRange(0.1f, 1.0f)); // Random height for each spot
    }

    // Generate a flat grid of vertices
    for (int32 X = 0; X <= TerrainSize; X++)
    {
        for (int32 Y = 0; Y <= TerrainSize; Y++)
        {
            Vertices.Add(FVector(X, Y, 0.0f)); // Flat grid

            // UVs from 0 to 1
            float U = static_cast<float>(X) / TerrainSize;
            float V = static_cast<float>(Y) / TerrainSize;
            UVs.Add(FVector2D(U, V));

            if (X < TerrainSize && Y < TerrainSize)
            {
                int32 A = X * (TerrainSize + 1) + Y;
                int32 B = A + 1;
                int32 C = A + TerrainSize + 1;
                int32 D = C + 1;

                Triangles.Add(A);
                Triangles.Add(B);
                Triangles.Add(C);

                Triangles.Add(B);
                Triangles.Add(D);
                Triangles.Add(C);
            }
        }
    }

    // Apply Gaussian function to displace vertices
    for (int32 i = 0; i < Vertices.Num(); i++)
    {
        FVector2D UV = UVs[i];
        float Z = 0.0f;

        for (int32 j = 0; j < NumSpots; j++)
        {
            FVector2D Center = SpotCenters[j];
            float Height = SpotHeights[j];
            float Sigma = 0.1f * GaussianScale; // Control the spread of the Gaussian

            float DistanceSquared = FMath::Square(UV.X - Center.X) + FMath::Square(UV.Y - Center.Y);
            Z += Height * FMath::Exp(-DistanceSquared / (2 * FMath::Square(Sigma))) * NoiseScale;
        }

        Vertices[i].Z = Z;

        // Track min and max heights
        MinHeight = FMath::Min(MinHeight, Z);
        MaxHeight = FMath::Max(MaxHeight, Z);
    }

    if (Vertices.Num() > 0)
    {
        // Assign colors based on normalized height
        for (int32 i = 0; i < Vertices.Num(); i++)
        {
            float HeightValue = Vertices[i].Z;
            float NormalizedValue = (HeightValue - MinHeight) / (MaxHeight - MinHeight);
            FColor VertexColor = FColor::MakeRedToGreenColorFromScalar(NormalizedValue);
            VertexColors.Add(VertexColor);
        }

        ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);

        if (MasterMaterial)
        {
            ProceduralMesh->SetMaterial(0, MasterMaterial);
        }

        UE_LOG(LogTemp, Warning, TEXT("Mesh section created successfully."));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("No vertices generated."));
    }
}