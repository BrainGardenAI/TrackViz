// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TrackVizGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TRACKVIZ_API ATrackVizGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TrackVizProperties)
	FVector startPosition = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TrackVizProperties)
	FString tracksDir = "tracks";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TrackVizProperties)
	bool isRelativePath = true;

	void BeginPlay() override;
};
