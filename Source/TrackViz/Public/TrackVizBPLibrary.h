// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "TrackVizBPLibrary.generated.h"


USTRUCT(BlueprintType)
struct FTrackRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TrackRecord)
	TArray<FVector> Positions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TrackRecord)
	FString FileName;
};


UCLASS()
class UTrackVizBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void DrawLine(UObject* WorldContextObject, FVector from, FVector to, FColor color, bool removable, float thickness);

	UFUNCTION(BlueprintCallable)
	static FTrackRecord ReadTrackRecordFromFile(const FString& path);

	UFUNCTION(BlueprintCallable, meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject"))
	static void DrawTrackRecord(
		UObject* WorldContextObject,
		const FTrackRecord& trackRecord,
		FVector startPosition,
		FColor color,
		float thickness
	);

	UFUNCTION(BlueprintCallable)
	static TArray<FTrackRecord> ReadTrackRecordsFromDir(const FString& path);

	UFUNCTION(BlueprintCallable)
	static TArray<FColor> GetColorsForTrackRecords(const TArray<FTrackRecord>& trackRecords);
};
