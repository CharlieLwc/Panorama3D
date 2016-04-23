// Fill out your copyright notice in the Description page of Project Settings.

#include "Panorama3D.h"
#include "PawnWithCamera.h"


// Sets default values
APawnWithCamera::APawnWithCamera()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APawnWithCamera::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APawnWithCamera::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// Called to bind functionality to input
void APawnWithCamera::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

}

