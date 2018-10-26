// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "TrackVizBPLibrary.generated.h"


USTRUCT(BlueprintType)
struct FTrackRecordEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = TrackRecordEntry)
	FVector Position;
};


UCLASS()
class UTrackVizBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static TArray<FTrackRecordEntry> ReadTrackRecordsFromCSV(const FString& path);

	UFUNCTION(
		BlueprintCallable,
		meta = (HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject")
	)
	static void VisualizeTrackRecords(
		UObject* WorldContextObject,
		const TArray<FTrackRecordEntry>& trackRecords,
		const FVector& startPosition,
		FColor color
	);
};
