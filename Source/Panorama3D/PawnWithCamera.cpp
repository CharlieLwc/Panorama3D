// Fill out your copyright notice in the Description page of Project Settings.

#include "Panorama3D.h"
#include "PawnWithCamera.h"


#define hf_PI_d2 1.570796327
// Sets default values
APawnWithCamera::APawnWithCamera()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create root components
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));


	//Create sphere components
	{
		SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
		SphereComponent->AttachTo(RootComponent);
		SphereComponent->InitSphereRadius(40.0f);

		// Create and position a mesh component so we can see where our sphere is
		SphereVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualRepresentation"));
		SphereVisual->AttachTo(SphereComponent);
		static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Engine/BasicShapes/Sphere"));
		if (SphereVisualAsset.Succeeded())
		{
			SphereVisual->SetStaticMesh(SphereVisualAsset.Object);
			SphereVisual->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
			SphereVisual->SetWorldScale3D(FVector(1.0f));
		}
	}

	//Create CameraSpringArm component
	OurCameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraSpringArm"));
	OurCameraSpringArm->AttachTo(RootComponent);
	OurCameraSpringArm->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 0.0f), FRotator(0.0f, 0.0f, 0.0f));
	OurCameraSpringArm->TargetArmLength = 0.f;
	OurCameraSpringArm->bEnableCameraLag = true;
	OurCameraSpringArm->CameraLagSpeed = 3.0f;

	//Create Camera component
	OurCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("GameCamera"));
	OurCamera->AttachTo(OurCameraSpringArm, USpringArmComponent::SocketName);

	//Take control of the default Player
	AutoPossessPlayer = EAutoReceiveInput::Player0;


	MaskSize = 512;
	focalLength = 900;
}

// Called when the game starts or when spawned
void APawnWithCamera::BeginPlay()
{
	Super::BeginPlay();

	creatStitchingMask();
	creatSimCaliMask();

	RV_MatInst = UMaterialInstanceDynamic::Create(MasterMaterialRef, this);

	RV_MatInst->SetTextureParameterValue(FName("CaliTex"), SimCaliMask);
	RV_MatInst->SetTextureParameterValue(FName("StitchingTex"), FigStitchingMask);


	SphereVisual->SetMaterial(0, RV_MatInst);
}

// Called every frame
void APawnWithCamera::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	//Rotate our actor's yaw, which will turn our camera because we're attached to it
	{
		FRotator NewRotation = OurCameraSpringArm->GetComponentRotation();
		NewRotation.Yaw += -CameraInput.X;
		OurCameraSpringArm->SetWorldRotation(NewRotation);
	}

	//Rotate our camera's pitch, but limit it so we're always looking downward
	{
		FRotator NewRotation = OurCameraSpringArm->GetComponentRotation();
		NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch - CameraInput.Y, -80.0f, 80.0f);
		OurCameraSpringArm->SetWorldRotation(NewRotation);
	}

	//Handle movement based on our "MoveX" and "MoveY" axes
	{
		if (!MovementInput.IsZero())
		{
			//Scale our movement input axis values by 100 units per second
			MovementInput = MovementInput.SafeNormal() * 100.0f;
			FVector NewLocation = GetActorLocation();
			NewLocation += GetActorForwardVector() * MovementInput.X * DeltaTime;
			NewLocation += GetActorRightVector() * MovementInput.Y * DeltaTime;
			SetActorLocation(NewLocation);
		}
	}
}

// Called to bind functionality to input
void APawnWithCamera::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	//Hook up every-frame handling for our four axes
	InputComponent->BindAxis("MoveForward", this, &APawnWithCamera::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &APawnWithCamera::MoveRight);
	InputComponent->BindAxis("CameraPitch", this, &APawnWithCamera::PitchCamera);
	InputComponent->BindAxis("CameraYaw", this, &APawnWithCamera::YawCamera);
}

void APawnWithCamera::MoveForward(float AxisValue)
{
	MovementInput.X = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void APawnWithCamera::MoveRight(float AxisValue)
{
	MovementInput.Y = FMath::Clamp<float>(AxisValue, -1.0f, 1.0f);
}

void APawnWithCamera::PitchCamera(float AxisValue)
{
	CameraInput.Y = AxisValue;
}

void APawnWithCamera::YawCamera(float AxisValue)
{
	CameraInput.X = AxisValue;
}


void APawnWithCamera::creatSimCaliMask()
{

	SimCaliMask = UTexture2D::CreateTransient(MaskSize, MaskSize, PF_A32B32G32R32F);

	// Allocate the texture HRI
	SimCaliMask->UpdateResource();


	int32 MipIndex = 0;
	uint32 NumRegions = 1;
	FUpdateTextureRegion2D* Region = new FUpdateTextureRegion2D(0, 0, 0, 0, MaskSize, MaskSize);

	int32 SrcBpp = 4;
	uint32 SrcPitch = MaskSize * 4 * SrcBpp;
	bool bFreeData = false;

	float* SrcData = new float[MaskSize*MaskSize * 4];

	float stepPitch = PI / MaskSize;
	float stepYaw = 2.0 * PI / MaskSize;
	int index = 0;


	float pitch = -hf_PI_d2;
	for (int yInd = 0; yInd<MaskSize; yInd++)
	{
		float yaw = -PI;
		for (int xInd = 0; xInd<MaskSize; xInd++)
		{

			float x = cos(yaw)*cos(pitch);
			float y = sin(yaw)*cos(pitch);
			float z = sin(pitch);

			float xoz = sqrt(x*x + z*z);

			float theta = -atan2(y, xoz);

			theta += hf_PI_d2;
			float arcLength = 2.0*focalLength*sin(theta * 0.5);

			float coordX = x / xoz * arcLength / 2880.f + 0.5;
			float coordY = 1.0 - (z / xoz * arcLength / 2880.f + 0.5);
			float coordNY = 1.0 - (-z / xoz * arcLength / 2880.f + 0.5);

			SrcData[index++] = coordX;	//A
			SrcData[index++] = coordY;	//B
			SrcData[index++] = coordX;	//R
			SrcData[index++] = coordNY;	//G


			yaw += stepYaw;
		}
		pitch += stepPitch;
	}


	UpdateTextureRegions(SimCaliMask, MipIndex, NumRegions, Region,
		SrcPitch, SrcBpp, (uint8*)SrcData, bFreeData);


}

void APawnWithCamera::creatStitchingMask()
{

	FigStitchingMask = UTexture2D::CreateTransient(MaskSize, MaskSize);

	FigStitchingMask->Filter = TF_Nearest;

	// Allocate the texture HRI
	FigStitchingMask->UpdateResource();


	int32 MipIndex = 0;
	uint32 NumRegions = 1;
	FUpdateTextureRegion2D* Region = new FUpdateTextureRegion2D(0, 0, 0, 0, MaskSize, MaskSize);
	int32 SrcBpp = 1;
	uint32 SrcPitch = MaskSize * 4 * SrcBpp;
	bool bFreeData = false;

	uint8* SrcData = new uint8[MaskSize*MaskSize * 4];

	float stepY = 2.0 / MaskSize;
	float stepXOZ = 2.0 / MaskSize;
	int index = 0;

	float xoz = -1.0 + stepXOZ*0.5;
	for (int yInd = 0; yInd<MaskSize; yInd++)
	{
		float y = -1.0 + stepY*0.5;
		for (int xInd = 0; xInd<MaskSize; xInd++)
		{
			float tempXOZ = xoz < 0 ? -sqrt(-xoz) : sqrt(xoz);

			float theta = atan2(y, tempXOZ);
			uint8 imageIndexLeft = 0;
			uint8 imageIndexRight = 0;

			if (theta > -PI && theta <= -hf_PI_d2)
			{
				imageIndexLeft = 2;
				imageIndexRight = 1;
			}
			else if (theta > -hf_PI_d2 && theta <= 0.0)
			{
				imageIndexLeft = 3;
				imageIndexRight = 2;
			}
			else if (theta > 0.0 && theta <= hf_PI_d2)
			{
				imageIndexLeft = 0;
				imageIndexRight = 3;
			}
			if (theta > hf_PI_d2 && theta <= PI)
			{
				imageIndexLeft = 1;
				imageIndexRight = 0;
			}

			SrcData[index++] = imageIndexLeft * 50;
			SrcData[index++] = imageIndexRight * 50;
			SrcData[index++] = 0;
			SrcData[index++] = 255;

			y += stepY;
		}
		xoz += stepXOZ;
	}


	UpdateTextureRegions(FigStitchingMask, MipIndex, NumRegions, Region,
		SrcPitch, SrcBpp, SrcData, bFreeData);


}

// Use this function to update the texture rects you want to change:
// NOTE: There is a method called UpdateTextureRegions in UTexture2D but it is compiled WITH_EDITOR and is not marked as ENGINE_API so it cannot be linked
// from plugins.


void APawnWithCamera::UpdateTextureRegions(UTexture2D* Texture, int32 MipIndex, uint32 NumRegions, FUpdateTextureRegion2D* Regions,
	uint32 SrcPitch, uint32 SrcBpp, uint8* SrcData, bool bFreeData)
{

	if (Texture->Resource)
	{
		struct FUpdateTextureRegionsData
		{
			FTexture2DResource* Texture2DResource;
			int32 MipIndex;
			uint32 NumRegions;
			FUpdateTextureRegion2D* Regions;
			uint32 SrcPitch;
			uint32 SrcBpp;
			uint8* SrcData;
		};

		FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

		RegionData->Texture2DResource = (FTexture2DResource*)Texture->Resource;
		RegionData->MipIndex = MipIndex;
		RegionData->NumRegions = NumRegions;
		RegionData->Regions = Regions;
		RegionData->SrcPitch = SrcPitch;
		RegionData->SrcBpp = SrcBpp;
		RegionData->SrcData = SrcData;


		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			UpdateTextureRegionsData,
			FUpdateTextureRegionsData*, RegionData, RegionData,
			bool, bFreeData, bFreeData,
			{
				for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
				{
					int32 CurrentFirstMip = RegionData->Texture2DResource->GetCurrentFirstMip();
					if (RegionData->MipIndex >= CurrentFirstMip)
					{
						RHIUpdateTexture2D(
							RegionData->Texture2DResource->GetTexture2DRHI(),
							RegionData->MipIndex - CurrentFirstMip,
							RegionData->Regions[RegionIndex],
							RegionData->SrcPitch,
							RegionData->SrcData
							+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
							+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
							);
					}
				}
		if (bFreeData)
		{
			FMemory::Free(RegionData->Regions);
			FMemory::Free(RegionData->SrcData);
		}
		delete RegionData;
			});


	}
}