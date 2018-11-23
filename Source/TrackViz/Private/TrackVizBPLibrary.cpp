// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "TrackVizBPLibrary.h"
#include "TrackViz.h"
#include "Core.h"
#include "Engine/GameEngine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Core/Public/HAL/FileManagerGeneric.h"


void UTrackVizBPLibrary::DrawLine(UObject* WorldContextObject, FVector from, FVector to, FColor color, float thickness)
{
	DrawDebugLine(GEngine->GetWorldFromContextObjectChecked(WorldContextObject), from, to, color, false, 999999, 0, thickness);
}

FTrackRecord UTrackVizBPLibrary::ReadTrackRecordFromFile(const FString& path)
{
	const TCHAR* delim = TEXT(",");

	TArray<FString> lines;
	FFileHelper::LoadFileToStringArray(lines, *path);
	FString headerLine = lines[0];
	lines.RemoveAt(0, 1, false);

	struct {
		int PosXIndex = -1;
		int PosYIndex = -1;
		int PosZIndex = -1;
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
		GEngine->AddOnScreenDebugMessage(-1, 999999, FColor::Red, TEXT("Parsing CSV file failed -- headers not found"));
		return {TArray<FVector>()};
	}

	TArray<FVector> positions;
	positions.Reserve(lines.Num());
	for (FString line : lines) {
		TArray<FString> fields;
		line.ParseIntoArray(fields, delim);
		FVector position(
			FCString::Atof(*fields[csvInfo.PosXIndex]) * 100,
			FCString::Atof(*fields[csvInfo.PosYIndex]) * 100,
			FCString::Atof(*fields[csvInfo.PosZIndex]) * 100
		);
		positions.Add(position);
	}

	return {positions, FPaths::GetBaseFilename(path)};
}


void UTrackVizBPLibrary::DrawTrackRecord(
	UObject* WorldContextObject,
	const FTrackRecord& trackRecord,
	FVector startPosition,
	FColor color,
	float thickness
) {
	FVector currentPosition(startPosition + trackRecord.Positions[0]);
	for (int i = 1; i < trackRecord.Positions.Num(); ++i) {
		auto position = trackRecord.Positions[i];
		FVector newPosition = startPosition + position;
		DrawLine(WorldContextObject, currentPosition, newPosition, color, thickness);
		currentPosition = newPosition;
	}
}

TArray<FTrackRecord> UTrackVizBPLibrary::ReadTrackRecordsFromDir(const FString& path)
{
	IFileManager& fileManager = IFileManager::Get();
	TArray<FString> fileNames;
	fileManager.FindFiles(fileNames, *path, TEXT("txt"));

	TArray<FTrackRecord> trackRecords;
	trackRecords.Reserve(fileNames.Num());
	for (const FString& fileName : fileNames) {
		FString fullFileName = FPaths::ConvertRelativePathToFull(path, fileName);
		trackRecords.Add(ReadTrackRecordFromFile(fullFileName));
	}
	
	return trackRecords;
}


TArray<FColor> UTrackVizBPLibrary::GetColorsForTrackRecords(const TArray<FTrackRecord>& trackRecords)
{
	static const TMap<FString, FColor> colorMap({
		{"black", FColor::Black},
		{"blue", FColor::Blue},
		{"cyan", FColor::Cyan},
		{"emerald", FColor::Emerald},
		{"green", FColor::Green},
		{"magenta", FColor::Magenta},
		{"orange", FColor::Orange},
		{"purple", FColor::Purple},
		{"red", FColor::Red},
		{"silver", FColor::Silver},
		{"turquoise", FColor::Turquoise},
		{"white", FColor::White},
		{"yellow", FColor::Yellow},
	});

	TArray<TPair<FString, bool>> colorUsed({
		TPair<FString, bool>("red", false),
		TPair<FString, bool>("green", false),
		TPair<FString, bool>("blue", false),
		TPair<FString, bool>("cyan", false),
		TPair<FString, bool>("magenta", false),
		TPair<FString, bool>("yellow", false),
		TPair<FString, bool>("black", false),
		TPair<FString, bool>("white", false),
		TPair<FString, bool>("emerald", false),
		TPair<FString, bool>("orange", false),
		TPair<FString, bool>("purple", false),
		TPair<FString, bool>("turquoise", false),
	});

	FColor defaultColor = FColor::Silver;
	TArray<FColor> colors;
	colors.Reserve(trackRecords.Num());
	for (int32 i = 0; i < trackRecords.Num(); ++i) {
		colors.Add(defaultColor);
	}

	uint8 usedColorCount = 0;
	for (int32 i = 0; i < trackRecords.Num(); ++i) {
		const FTrackRecord& trackRecord = trackRecords[i];
		for (TPair<FString, bool>& entry : colorUsed) {
			const FString& colorStr = entry.Key;
			const FString& fileName = trackRecord.FileName.ToLower();
			if (fileName.StartsWith(colorStr) || fileName.EndsWith(colorStr))
			{
				entry.Value = true;
				colors[i] = *colorMap.Find(colorStr);
				usedColorCount += 1;
				break;
			}
		}
	}

	int8 colorIndex = 0;
	for (int32 i = 0; i < trackRecords.Num(); ++i) {
		if (colors[i] != defaultColor || usedColorCount == colorUsed.Num()) {
			continue;
		}
		while (colorUsed[colorIndex].Value) {
			++colorIndex;
			if (colorIndex >= colorUsed.Num()) {
				colorIndex = 0;
			}
		}
		colors[i] = *colorMap.Find(colorUsed[colorIndex].Key);
		++colorIndex;
		if (colorIndex >= colorUsed.Num()) {
			colorIndex = 0;
		}
	}

	return colors;
}
