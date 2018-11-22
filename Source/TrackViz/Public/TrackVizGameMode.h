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

private:

	const float LineThickness = 1;

	bool bRotationEnabled = false;
	bool bPawnsVisible;
	bool bIsComparison;
	APlayerController* PC;
	FIntPoint MouseCursorPosition;
	APawn* DefaultPawn;
	TArray<const class AStaticCameraPawn*> Pawns;
	TArray<struct FTrackRecord> TrackRecords;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TrackVizProperties)
	FVector startPosition = FVector(0, 0, 0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TrackVizProperties)
	FString tracksDir = "tracks";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TrackVizProperties)
	bool isRelativePath = true;

	ATrackVizGameMode();
	void BeginPlay() override;
	void Tick(float DeltaSeconds) override;

private:
	void OnClick();
	void OnRelease();
	void OnPressedX();
	UFUNCTION()
	void OnPressedStaticPawn(AActor* actor, FKey key);
	void SetPawnsVisibility(bool visibility);
	void TogglePawnsVisibility();
	void ShowTooltip();
};
