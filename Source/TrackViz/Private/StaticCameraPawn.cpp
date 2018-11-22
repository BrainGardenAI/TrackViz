// Fill out your copyright notice in the Description page of Project Settings.

#include "StaticCameraPawn.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ConstructorHelpers.h"
#include "GameFramework/Controller.h"
#include "Components/ActorComponent.h"
#include "TrackViz.h"
#include "Engine.h"


// Sets default values
AStaticCameraPawn::AStaticCameraPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	ConstructorHelpers::FObjectFinderOptional<UStaticMesh> MeshFinder(TEXT("StaticMesh'/TrackViz/Marker.Marker'"));
	Mesh->SetStaticMesh(MeshFinder.Get());
	RootComponent = Mesh;
	PossessedController = nullptr;
}

