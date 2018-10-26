// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "TrackVizBPLibrary.h"
#include "TrackViz.h"
#include "Core.h"
#include "Engine/GameEngine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"


TArray<FTrackRecordEntry> UTrackVizBPLibrary::ReadTrackRecordsFromCSV(const FString& path)
{
	const TCHAR* delim = TEXT(",\t");

	TArray<FString> lines;
	FFileHelper::LoadFileToStringArray(lines, *path);
	FString headerLine = lines[0];
	lines.RemoveAt(0, 1, false);

	struct CSVInfo {
		uint16 PosXIndex = -1;
		uint16 PosYIndex = -1;
		uint16 PosZIndex = -1;
	} csvInfo;
	
	TArray<FString> headers;
	headerLine.ParseIntoArray(headers, delim);
	for (uint16 i = 0; i < headers.Num(); ++i) {
		FString header = headers[i];
		if (header.Equals(TEXT("POS_X"))) {
			csvInfo.PosXIndex = i;
		}
		if (header.Equals(TEXT("POS_Y"))) {
			csvInfo.PosYIndex = i;
		}
		if (header.Equals(TEXT("POS_Z"))) {
			csvInfo.PosZIndex = i;
		}
	}

	if (csvInfo.PosXIndex == -1 || csvInfo.PosYIndex == -1 || csvInfo.PosZIndex == -1) {
		GEngine->AddOnScreenDebugMessage(-1, 999, FColor::Red, TEXT("Parsing CSV file failed -- headers not found"));
		return TArray<FTrackRecordEntry>();
	}

	TArray<FTrackRecordEntry> result;
	result.Reserve(lines.Num());
	for (FString line : lines) {

		TArray<FString> fields;
		line.ParseIntoArray(fields, delim);
		FTrackRecordEntry entry;

		entry.Position.X = FCString::Atof(*fields[csvInfo.PosXIndex]) * 100;
		entry.Position.Y = FCString::Atof(*fields[csvInfo.PosYIndex]) * 100;
		entry.Position.Z = FCString::Atof(*fields[csvInfo.PosZIndex]) * 100;

		result.Add(entry);
	}

	return result;
}


void UTrackVizBPLibrary::VisualizeTrackRecords(
	UObject* WorldContextObject,
	const TArray<FTrackRecordEntry>& trackRecords,
	const FVector& startPosition,
	FColor color
)
{
	FVector currentPosition(startPosition);
	const UWorld* world = GEngine->GetWorldFromContextObject(WorldContextObject);
	for (FTrackRecordEntry rec : trackRecords) {
		FVector newPosition = startPosition + rec.Position;
		DrawDebugLine(world, currentPosition, newPosition, color, false, 999, 0, 1);
		currentPosition = newPosition;
	}
}
