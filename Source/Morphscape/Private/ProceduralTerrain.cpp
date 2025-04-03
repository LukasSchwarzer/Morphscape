#include "ProceduralTerrain.h"
#include "ProceduralMeshComponent.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h" // For GEngine
#include "ProceduralMeshConversion.h"
#include "MeshDescription.h"
#include "StaticMeshDescription.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/Package.h"
#include "UObject/GarbageCollection.h"

AProceduralTerrain::AProceduralTerrain()
{
    PrimaryActorTick.bCanEverTick = true;
    ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));

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
    //delete the old static mesh
    if (GeneratedStaticMesh)
    {
        FString ExistingPackageName = GeneratedStaticMesh->GetPackage()->GetName();
        
        // Unload and mark for garbage collection
        GeneratedStaticMesh->ClearFlags(RF_Standalone);
        GeneratedStaticMesh->ConditionalBeginDestroy();
        GeneratedStaticMesh = nullptr;

        // Force unload the package
        if (UPackage* ExistingPackage = FindPackage(nullptr, *ExistingPackageName))
        {
            ExistingPackage->ClearFlags(RF_Standalone);
            ExistingPackage->ConditionalBeginDestroy();
            
            // Force garbage collection
            CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
        }
    }

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
            Vertices.Add(FVector(X * GridSpacing, Y * GridSpacing, 0.0f)); // Flat grid

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

    //SimulateErosion();

    if (Vertices.Num() > 0)
    {
        /*
        // Assign colors based on normalized height
        for (int32 i = 0; i < Vertices.Num(); i++)
        {
            float HeightValue = Vertices[i].Z;
            float NormalizedValue = (HeightValue - MinHeight) / (MaxHeight - MinHeight);
            FColor VertexColor = FColor::MakeRedToGreenColorFromScalar(NormalizedValue);
            VertexColors.Add(VertexColor);
        }

        ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);
        */
        ProceduralMesh->CreateMeshSection(0, Vertices, Triangles, Normals, UVs, TArray<FColor>(), Tangents, true);


        GeneratedStaticMesh = ConvertProceduralMeshToStaticMesh();


        if(GeneratedStaticMesh)
        {
            StaticMeshComponent->SetStaticMesh(GeneratedStaticMesh);
            UE_LOG(LogTemp, Warning, TEXT("q."));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to create static mesh."));
        }


        //assign material
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

    //PCG
    /*
    if (PCGComponent && PCGGraph)
    {
        // Assign the PCG graph to the component
        PCGComponent->SetGraph(PCGGraph);

        //UPCGData* MeshPCGData = PCGComponent->CreateActorPCGData(this, true);

        PCGComponent->Generate();

        // Iterate over generated points and place assets
        
        for (const FPCGPoint& Point : PCGComponent->GetGeneratedPoints())
        {
            // Example: Spawn a static mesh at each point
            FTransform Transform(Point.Transform);
            GetWorld()->SpawnActor<AStaticMeshActor>(StaticMeshActorClass, Transform);
        }
        
    }
    */ 
}

UStaticMesh* AProceduralTerrain::ConvertProceduralMeshToStaticMesh()
{

    //FString ActorName = GetName();
    FString MeshName = TEXT("SM_RandomMesh");

    FString Path = FString(TEXT("/Game/Meshes/ProceduralTerrain/"));
    FString PackageName = Path + MeshName;

    UPackage* Package = CreatePackage(*PackageName);
    UStaticMesh* StaticMesh = NewObject<UStaticMesh>(Package, FName(*MeshName), RF_Public | RF_Standalone);

    StaticMesh->bAllowCPUAccess = true;
    StaticMesh->NeverStream = true;
    StaticMesh->InitResources();
    StaticMesh->SetLightingGuid();
    
    
    FMeshDescription MeshDescription = BuildMeshDescription(ProceduralMesh);
    UStaticMeshDescription* SMDescription = StaticMesh->CreateStaticMeshDescription();
    SMDescription->SetMeshDescription(MeshDescription);
    //the following second parameter sets collision
    StaticMesh->BuildFromStaticMeshDescriptions({SMDescription}, false);
    StaticMesh->MarkPackageDirty();
    
    FAssetRegistryModule::AssetCreated(StaticMesh);

    //load the static mesh from the package name
    UStaticMesh* LoadedStaticMesh = LoadObject<UStaticMesh>(nullptr, *PackageName);

    if (LoadedStaticMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("StaticMesh loaded successfully: %s"), *PackageName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load StaticMesh from: %s"), *PackageName);
    }
    return LoadedStaticMesh;
}

void AProceduralTerrain::SimulateErosion()
{
    // Number of erosion iterations
    int32 ErosionIterations = 100;
    float TalusAngle = 0.1f; // Threshold slope angle for erosion

    // Track erosion amount for visualization
    TArray<float> ErosionAmounts;
    ErosionAmounts.Init(0.0f, Vertices.Num());

    for (int32 Iteration = 0; Iteration < ErosionIterations; Iteration++)
    {
        for (int32 X = 0; X < TerrainSize; X++)
        {
            for (int32 Y = 0; Y < TerrainSize; Y++)
            {
                int32 Index = X * (TerrainSize + 1) + Y;
                FVector& Vertex = Vertices[Index];

                // Check neighbors
                TArray<int32> NeighborIndices;
                if (X > 0) NeighborIndices.Add((X - 1) * (TerrainSize + 1) + Y); // Left
                if (X < TerrainSize) NeighborIndices.Add((X + 1) * (TerrainSize + 1) + Y); // Right
                if (Y > 0) NeighborIndices.Add(X * (TerrainSize + 1) + (Y - 1)); // Down
                if (Y < TerrainSize) NeighborIndices.Add(X * (TerrainSize + 1) + (Y + 1)); // Up

                for (int32 NeighborIndex : NeighborIndices)
                {
                    FVector& NeighborVertex = Vertices[NeighborIndex];
                    float Slope = Vertex.Z - NeighborVertex.Z;

                    if (Slope > TalusAngle)
                    {
                        float ErosionAmount = (Slope - TalusAngle) * 0.5f;
                        Vertex.Z -= ErosionAmount;
                        NeighborVertex.Z += ErosionAmount;

                        // Track erosion amount
                        ErosionAmounts[Index] += ErosionAmount;
                        ErosionAmounts[NeighborIndex] += ErosionAmount;
                    }
                }
            }
        }
    }

    // Find max erosion value for normalization
    float MaxErosion = 0.0f;
    for (float ErosionValue : ErosionAmounts)
    {
        MaxErosion = FMath::Max(MaxErosion, ErosionValue);
    }

    // Visualize erosion by setting vertex colors
    TArray<FColor> VertexColors;
    for (int32 i = 0; i < Vertices.Num(); i++)
    {
        float ErosionValue = ErosionAmounts[i];
        float NormalizedErosion = MaxErosion > 0 ? ErosionValue / MaxErosion : 0.0f;
        FColor VertexColor = FColor::MakeRedToGreenColorFromScalar(NormalizedErosion);
        VertexColors.Add(VertexColor);
    }

    // Update the mesh with the new vertex colors
    ProceduralMesh->UpdateMeshSection(0, Vertices, Normals, UVs, VertexColors, Tangents);
}