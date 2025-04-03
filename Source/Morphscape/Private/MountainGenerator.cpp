#include "MountainGenerator.h"
#include "ProceduralMeshComponent.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h" // For GEngine
#include "DrawDebugHelpers.h" // Include for debug drawing


AMountainGenerator::AMountainGenerator()
{
    PrimaryActorTick.bCanEverTick = true;
    ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
    RootComponent = ProceduralMesh;
}

void AMountainGenerator::BeginPlay()
{
    Super::BeginPlay();
    GenerateTerrain();
}


void AMountainGenerator::GenerateTerrain()
{
    Vertices.Empty();
    Triangles.Empty();
    UVs.Empty();
    Normals.Empty();
    Tangents.Empty();
    TArray<FColor> VertexColors;

    float MinHeight = FLT_MAX;
    float MaxHeight = -FLT_MAX;

    // Generate a single peak
    FVector2D PeakLocation(FMath::FRand() * TerrainSize, FMath::FRand() * TerrainSize);
    FVector PeakLocation3D(PeakLocation, PeakHeight);
    DrawDebugSphere(GetWorld(), PeakLocation3D, 10.0f, 12, FColor::Blue, true);

    // Log the peak location
    UE_LOG(LogTemp, Warning, TEXT("Peak Location: X=%f, Y=%f, Z=%f"), PeakLocation3D.X, PeakLocation3D.Y, PeakLocation3D.Z);

    //float PeakHeight = FMath::FRandRange(0.5f, 1.0f) * NoiseScale;

    // Generate radial lines (ridges)
    int32 NumRidges = FMath::RandRange(3, 5);
    TArray<FVector2D> RidgeDirections;
    float AngleStep = 360.0f / NumRidges;

    for (int32 i = 0; i < NumRidges; i++)
    {
        float Angle = i * AngleStep + FMath::FRandRange(-15.0f, 15.0f);
        float Rad = FMath::DegreesToRadians(Angle);
        float RidgeDistance = FMath::FRandRange(MinRidgeDistance, MaxRidgeDistance);
        FVector2D RidgeDir = FVector2D(FMath::Cos(Rad), FMath::Sin(Rad)) * RidgeDistance;
        RidgeDirections.Add(RidgeDir);

        // Draw debug line for each ridge
        FVector RidgeEnd = FVector(PeakLocation + RidgeDir, 0.0f);
        DrawDebugLine(GetWorld(), PeakLocation3D, RidgeEnd, FColor::Red, true);
    }


    // Generate a flat grid of vertices
    for (int32 X = 0; X <= TerrainSize; X++)
    {
        for (int32 Y = 0; Y <= TerrainSize; Y++)
        {
            FVector Vertex(X, Y, 0.0f);
            Vertices.Add(Vertex);

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

    // Adjust terrain based on peak and ridges
    float ThresholdDistance = 10.0f; // Adjust this value as needed

    for (int32 i = 0; i < Vertices.Num(); i++)
    {
        FVector2D UV = UVs[i];
        float Z = 0.0f;

        // Influence from the peak
        float DistanceToPeak = FVector2D::Distance(UV, PeakLocation);
        if (DistanceToPeak < ThresholdDistance)
        {
            Z += PeakHeight;
        }

        // Influence from ridges
        for (const FVector2D& RidgeDir : RidgeDirections)
        {
            FVector2D RidgeEnd = PeakLocation + RidgeDir;
            float DistanceToRidge = FMath::PointDistToLine(
                FVector(UV, 0.0f),
                FVector(RidgeDir, 0.0f),
                FVector(PeakLocation, 0.0f)
            );
            if (DistanceToRidge < ThresholdDistance)
            {
                Z += PeakHeight;
            }
        }

        Vertices[i].Z = Z;

        // Track min and max heights
        MinHeight = FMath::Min(MinHeight, Z);
        MaxHeight = FMath::Max(MaxHeight, Z);
    }

    // After adjusting vertex heights, find and log the highest vertex
    float HighestZ = -FLT_MAX;
    FVector HighestVertex;

    for (const FVector& Vertex : Vertices)
    {
        if (Vertex.Z > HighestZ)
        {
            HighestZ = Vertex.Z;
            HighestVertex = Vertex;
        }
    }

    //UE_LOG(LogTemp, Warning, TEXT("Highest Vertex: X=%f, Y=%f, Z=%f"), HighestVertex.X, HighestVertex.Y, HighestVertex.Z);

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