// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "TrackVizBPLibrary.h"
#include "TrackViz.h"
#include "Core.h"
#include "Engine/GameEngine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Core/Public/HAL/FileManagerGeneric.h"


void UTrackVizBPLibrary::DrawLine(UObject* WorldContextObject, FVector from, FVector to, FColor color, bool removable, float thickness)
{
	DrawDebugLine(GEngine->GetWorldFromContextObjectChecked(WorldContextObject), from, to, color, removable, 999999, 0, thickness);
}

void UTrackVizBPLibrary::DrawArrow(UObject* WorldContextObject, FVector position, FRotator rotator, FColor color, bool removable, float thickness)
{
	DrawDebugDirectionalArrow(
		GEngine->GetWorldFromContextObjectChecked(WorldContextObject),
		position,
		position + rotator.Vector() * 10,
		0,
		color,
		removable,
		999999,
		0,
		thickness
	);
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
		int QuatXIndex = -1;
		int QuatYIndex = -1;
		int QuatZIndex = -1;
		int QuatWIndex = -1;
	} csvInfo;
	
	TArray<FString> headers = ParseLineIntoArray(headerLine);
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
		if (header.Equals(TEXT("Q_X"))) {
			csvInfo.QuatXIndex = i;
		}
		if (header.Equals(TEXT("Q_Y"))) {
			csvInfo.QuatYIndex = i;
		}
		if (header.Equals(TEXT("Q_Z"))) {
			csvInfo.QuatZIndex = i;
		}
		if (header.Equals(TEXT("Q_W"))) {
			csvInfo.QuatWIndex = i;
		}
	}

	if (csvInfo.PosXIndex == -1 || csvInfo.PosYIndex == -1 || csvInfo.PosZIndex == -1) {
		GEngine->AddOnScreenDebugMessage(-1, 999999, FColor::Red, TEXT("Parsing CSV file failed -- headers not found"));
		return {TArray<FVector>(), TArray<FRotator>(), false, TEXT("")};
	}

	bool RotatorsKnown = csvInfo.QuatXIndex != -1 && csvInfo.QuatYIndex != -1 && csvInfo.QuatZIndex != -1 && csvInfo.QuatWIndex != -1;

	TArray<FVector> positions;
	TArray<FRotator> rotators;
	positions.Reserve(lines.Num());
	if (RotatorsKnown) {
		rotators.Reserve(lines.Num());
	}
	for (FString line : lines) {
		TArray<FString> fields = ParseLineIntoArray(line);
		FVector position(
			FCString::Atof(*fields[csvInfo.PosXIndex]) * 100,
			FCString::Atof(*fields[csvInfo.PosYIndex]) * 100,
			FCString::Atof(*fields[csvInfo.PosZIndex]) * 100
		);
		positions.Add(position);
		if (RotatorsKnown) {
			FQuat Quat(
				FCString::Atof(*fields[csvInfo.QuatXIndex]),
				FCString::Atof(*fields[csvInfo.QuatYIndex]),
				FCString::Atof(*fields[csvInfo.QuatZIndex]),
				FCString::Atof(*fields[csvInfo.QuatWIndex])
			);
			rotators.Add(Quat.Rotator());
		}
	}

	return {positions, rotators, RotatorsKnown, FPaths::GetBaseFilename(path)};
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
		DrawLine(WorldContextObject, currentPosition, newPosition, color, false, thickness);
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

TArray<FString> UTrackVizBPLibrary::ParseLineIntoArray(const FString& Line)
{
	static const FRegexPattern DELIM_REGEX(TEXT("(,\t)|(,)|(\t)"));
	FRegexMatcher Matcher(DELIM_REGEX, Line);
	TArray<FString> result;

	int32 prev = 0;
	while (Matcher.FindNext()) {
		result.Add(Line.Mid(prev, Matcher.GetMatchBeginning() - prev));
		prev = Matcher.GetMatchEnding();
	}
	result.Add(Line.Mid(prev, Line.Len() - prev));

	for (const auto& s : result) {
		UE_LOG(LogTemp, Log, TEXT("%s"), *s);
	}

	return result;
}
