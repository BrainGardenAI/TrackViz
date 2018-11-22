// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "StaticCameraPawn.generated.h"

UCLASS()
class TRACKVIZ_API AStaticCameraPawn : public ADefaultPawn
{
	GENERATED_BODY()

private:
	class AController* PossessedController;

public:
	UPROPERTY(Category = Mesh, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Mesh;

	UPROPERTY(Category = PointData, EditAnywhere)
	int32 TrackIndex = -1;

	UPROPERTY(Category = PointData, EditAnywhere)
	int32 PointIndex = -1;

	UPROPERTY(Category = PointData, EditAnywhere)
	FColor Color = FColor::Transparent;

	// Sets default values for this pawn's properties
	AStaticCameraPawn();
};
