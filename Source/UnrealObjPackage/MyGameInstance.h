// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "MyGameInstance.generated.h"

struct FStudentData
{
	FStudentData() {}
	FStudentData(int32 InOrder, const FString& InName)
		: Order(InOrder), Name(InName) {}

	int32 Order = -1;
	FString Name = TEXT("홍길동");

	friend FArchive& operator<<(FArchive& Ar, FStudentData& InStudentData)
	{
		Ar << InStudentData.Order << InStudentData.Name; 
		return Ar;
	}
};
/**
 * 
 */
UCLASS()
class UNREALOBJPACKAGE_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UMyGameInstance();
	
	virtual void Init() override;

	// 1. 애셋을 저장하기 위한 함수.
	void SaveStudentPackage() const;

	// 9. 애셋을 불러오기 위한 함수
	void LoadStudentPackage() const;

private:
	static const FString PackageName;
	static const FString AssetName;
	
	UPROPERTY()
	TObjectPtr<class UStudent> StudentSrc;
};
