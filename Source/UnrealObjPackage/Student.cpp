// Fill out your copyright notice in the Description page of Project Settings.


#include "Student.h"

UStudent::UStudent()
{
	Order = -1;
	Name = TEXT("이학생");
}

int32 UStudent::GetOrder() const
{
	return Order;
}

void UStudent::SetOrder(int32 InOrder)
{
	Order = InOrder;
}

const FString& UStudent::GetName() const
{
	return Name;
}

void UStudent::SetName(const FString& InName)
{
	Name = InName;
}

void UStudent::PrintInfo(const FString& Tag) const
{
	UE_LOG(LogTemp, Log, TEXT("[%s] 이름 %s 순번 %d"), *Tag, *Name, Order);
}
