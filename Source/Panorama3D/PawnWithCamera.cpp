// Fill out your copyright notice in the Description page of Project Settings.

#include "Panorama3D.h"
#include "PawnWithCamera.h"


#define hf_PI_d2 1.570796327
// Sets default values
APawnWithCamera::APawnWithCamera()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	MaskSize = 512;
	focalLength = 900;
	InterpupillaryDistance = 0.10;

	FishEyePos1 = FVector(0.0, 0.05, 0.0);
	FishEyePos2 = FVector(0.05, 0.0, 0.0);
	FishEyePos3 = FVector(0.0, -0.05, 0.0);
	FishEyePos4 = FVector(-0.05, 0.0, 0.0);





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

	//Create Camera components
	{


		//Create CameraSpringArm component
		OurCameraBase = CreateDefaultSubobject<USceneComponent>(TEXT("CameraBaseComponent"));
		OurCameraBase->AttachTo(RootComponent);
		OurCameraBase->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 0.0f), FRotator(0.0f, 0.0f, 0.0f));

		//Create Camera component
		OurCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("GameCamera"));
		OurCamera->AttachTo(OurCameraBase);


		//Create LeftEye component
		LeftEyePos = CreateDefaultSubobject<USceneComponent>(TEXT("LeftEye"));
		LeftEyePos->AttachTo(OurCamera);


		//Create RightEye component
		RightEyePos = CreateDefaultSubobject<USceneComponent>(TEXT("RightEye"));
		RightEyePos->AttachTo(OurCamera);


	}

	//Take control of the default Player
	AutoPossessPlayer = EAutoReceiveInput::Player0;


	CalDirs();
	MoveCameras();
}

#if WITH_EDITOR
void APawnWithCamera::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	CalDirs();
	MoveCameras();

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void APawnWithCamera::PostInitProperties()
{
	Super::PostInitProperties();


	CalDirs();
	MoveCameras();

}

void APawnWithCamera::MoveCameras()
{
	LeftEyePos->SetRelativeLocationAndRotation(FVector(0.0, -InterpupillaryDistance*0.5f, 0.0f), FRotator(0.0f, 0.0f, 0.0f));
	RightEyePos->SetRelativeLocationAndRotation(FVector(0.0, InterpupillaryDistance*0.5f, 0.0f), FRotator(0.0f, 0.0f, 0.0f));
}

void APawnWithCamera::CalDirs()
{

	dir[0] = FishEyePos2 - FishEyePos1;
	dir[1] = FishEyePos3 - FishEyePos2;
	dir[2] = FishEyePos4 - FishEyePos3;
	dir[3] = FishEyePos1 - FishEyePos4;

	FishEyePos[0] = FishEyePos1;
	FishEyePos[1] = FishEyePos2;
	FishEyePos[2] = FishEyePos3;
	FishEyePos[3] = FishEyePos4;


	for (int i = 0; i < 4; i++)
	{
		length[i] = FVector::DotProduct(dir[i], dir[i]);
		dir[i].Normalize();
	}

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


	RV_MatInst->SetVectorParameterValue(FName("FishEyePos1"), FishEyePos1);
	RV_MatInst->SetVectorParameterValue(FName("FishEyePos2"), FishEyePos2);
	RV_MatInst->SetVectorParameterValue(FName("FishEyePos3"), FishEyePos3);
	RV_MatInst->SetVectorParameterValue(FName("FishEyePos4"), FishEyePos4);

	SphereVisual->SetMaterial(0, RV_MatInst);
}

// Called every frame
void APawnWithCamera::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	//Rotate our actor's yaw, which will turn our camera because we're attached to it
	{
		FRotator NewRotation = OurCameraBase->GetComponentRotation();
		NewRotation.Yaw += -CameraInput.X;
		OurCameraBase->SetWorldRotation(NewRotation);
	}

	//Rotate our camera's pitch, but limit it so we're always looking downward
	{
		FRotator NewRotation = OurCameraBase->GetComponentRotation();
		NewRotation.Pitch = FMath::Clamp(NewRotation.Pitch - CameraInput.Y, -80.0f, 80.0f);
		OurCameraBase->SetWorldRotation(NewRotation);
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




	FVector LeftEyePosition = LeftEyePos->GetComponentLocation() - OurCamera->GetComponentLocation();
	LeftEyePosition.Normalize();

	FVector RightEyePosition = RightEyePos->GetComponentLocation() - OurCamera->GetComponentLocation();
	RightEyePosition.Normalize();

	FVector4 LeftEyeWeight;
	FVector4 RightEyeWeight;
	
	for (int i = 0; i < 4; i++)
	{
		LeftEyeWeight[i] = FVector::DotProduct((LeftEyePosition - FishEyePos[i]), dir[i]) / length[i];
		RightEyeWeight[i] = FVector::DotProduct((RightEyePosition - FishEyePos[i]), dir[i]) / length[i];
	}

	RV_MatInst->SetVectorParameterValue(FName("LeftEyePos"), LeftEyePosition);
	RV_MatInst->SetVectorParameterValue(FName("RightEyePos"), RightEyePosition);
	RV_MatInst->SetVectorParameterValue(FName("RightWeight"), LeftEyePosition);
	RV_MatInst->SetVectorParameterValue(FName("LeftWeight"), RightEyePosition);
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

typedef union {
	float f;
	char c[4];
}FLOAT_CONV;


/************************************************************
Conversion little endian float data to big endian
*************************************************************/
static float __ltobf(float data)
{
	FLOAT_CONV d1, d2;

	d1.f = data;

	d2.c[0] = d1.c[0];
	d2.c[1] = d1.c[1];
	d2.c[2] = d1.c[2];
	d2.c[3] = d1.c[3];

	return d2.f;
}

void APawnWithCamera::creatSimCaliMask()
{

	// Allocate the texture HRI
	SimCaliMask = UTexture2D::CreateTransient(MaskSize, MaskSize, PF_A32B32G32R32F);
	SimCaliMask->Filter = TF_Nearest;
//	SimCaliMask->UpdateResource();




	int32 MipIndex = 0;
	uint32 NumRegions = 1;
	FUpdateTextureRegion2D* Region = new FUpdateTextureRegion2D(0, 0, 0, 0, MaskSize, MaskSize);

	int32 SrcBpp = sizeof(FLinearColor);
	uint32 SrcPitch = MaskSize * SrcBpp;
	bool bFreeData = true;


	FVector4 *sourceData = new FVector4[MaskSize*MaskSize];


	float* SrcData = new float[MaskSize*MaskSize * 4];

	float stepPitch = PI / MaskSize;
	float stepYaw = 2.0 * PI / MaskSize;
	int index = 0;
	int index2 = 0;
	float testStep = 1.0 / MaskSize;

	float testx = 0.f;
	float pitch = -hf_PI_d2;
	for (int yInd = 0; yInd<MaskSize; yInd++)
	{
		float testy = 0.f;
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

			float temp = theta;

			coordX = coordX > 1.f ? 1.f : coordX;
			coordX = coordX < 0.f ? 0.f : coordX;
			coordY = coordY > 1.f ? 1.f : coordY;
			coordY = coordY < 0.f ? 0.f : coordY;

			if (testy > 1.0)
				testy = 0.0;

			if (testx < 0.5)
			{
				sourceData[index2][0] = __ltobf(testy);
				sourceData[index2][1] = 0.0;
				sourceData[index2][2] = temp;
				sourceData[index2][3] = 1.0;
				
				index2++;

				SrcData[index++] = testy;
				SrcData[index++] = 0.0;
				SrcData[index++] = 0.0;
				SrcData[index++] = 1.0;


			}
			else
			{
				sourceData[index2][0] = 0.0;
				sourceData[index2][1] = __ltobf(1.0 - testy);
				sourceData[index2][2] = temp;
				sourceData[index2][3] = 1.0;


				index2++;

				SrcData[index++] = 0.0;
				SrcData[index++] = 1.0 - testy;
				SrcData[index++] = 0.0;
				SrcData[index++] = 1.0;
			}

			testy += testStep;
			yaw += stepYaw;
		}
		testx += testStep;
		pitch += stepPitch;
	}

	FTexture2DMipMap& Mip = SimCaliMask->PlatformData->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(Data, sourceData, MaskSize*SrcPitch);
	Mip.BulkData.Unlock();
	SimCaliMask->UpdateResource();
	
//	UpdateTextureRegions(SimCaliMask, MipIndex, NumRegions, Region,	SrcPitch, SrcBpp, (uint8*)sourceData, bFreeData);

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

