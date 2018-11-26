// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "MarkerMeshActor.generated.h"

/**
 * 
 */
UCLASS()
class AMarkerMeshActor : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	UPROPERTY(Category = Mesh, EditAnywhere)
	class UStaticMeshComponent* Mesh;

	UPROPERTY(Category = PointData, EditAnywhere)
	int32 TrackIndex = -1;

	UPROPERTY(Category = PointData, EditAnywhere)
	int32 PointIndex = -1;

	UPROPERTY(Category = PointData, EditAnywhere)
	FColor Color = FColor::Transparent;
	
	AMarkerMeshActor();
};
