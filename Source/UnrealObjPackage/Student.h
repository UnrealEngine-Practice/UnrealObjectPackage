// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Student.generated.h"

/**
 * 
 */
UCLASS()
class  UStudent : public UObject
{
	GENERATED_BODY()
public:
	UStudent();
	int32 GetOrder() const;
	void SetOrder(int32 InOrder);

	// UObject에 있는 가상함수이다.
	virtual void Serialize(FArchive& Ar) override;
	const FString& GetName() const;
	void SetName(const FString& InName);
	void PrintInfo(const FString& Tag = TEXT("")) const;
private:
	UPROPERTY()
	int32	Order;

	UPROPERTY()
	FString Name;
};

inline void UStudent::Serialize(FArchive& Ar)
{
	UObject::Serialize(Ar);
	Ar << Order << Name;
}
