// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Student.h"
#include "JsonObjectConverter.h"

UMyGameInstance::UMyGameInstance()
{
	
}

void UMyGameInstance::Init()
{
	Super::Init();
	FStudentData RawDataSrc(11, TEXT("abc"));

	// 저장할 파일의 위치
	const FString SavedDir = FPaths::Combine(FPlatformMisc::ProjectDir(), TEXT("Saved"));
	UE_LOG(LogTemp, Log, TEXT("저장할 파일위치 : %s"), *SavedDir);

	{
		const FString RawDataFilename(TEXT("RawData.bin"));
		FString RawDataAbsolutePath = FPaths::Combine(*SavedDir, *RawDataFilename);
		UE_LOG(LogTemp, Log, TEXT("저장할 파일 절대경로 : %s"), *RawDataAbsolutePath);

		FPaths::MakeStandardFilename(RawDataAbsolutePath);
		UE_LOG(LogTemp, Log, TEXT("저장할 파일 절대경로 : %s"), *RawDataAbsolutePath);
		
		// Ar는 Archive라는 뜻 ㅎ
		// 전달해준 Path에 write할 수 있는 아카이브 객체 생성됨.
		FArchive* RawFileWriterAr = IFileManager::Get().CreateFileWriter(*RawDataAbsolutePath);
		if (RawFileWriterAr != nullptr)
		{
			*RawFileWriterAr << RawDataSrc;
			RawFileWriterAr->Close();
			delete RawFileWriterAr;
			RawFileWriterAr = nullptr;

			// UE_LOG(LogTemp, Log, TEXT("[RawData] 이름 %s 순번 %d"), *RawDa);
		}
		FStudentData RawDataDest;
		FArchive* RawFileReaderAr = IFileManager::Get().CreateFileReader(*RawDataAbsolutePath);
		if (RawFileReaderAr != nullptr)
		{
			// << 연산자라 입력 같지만, Reader에 대한 << 연산자는 Reader에서 Dest로 쓴다.
			*RawFileReaderAr << RawDataDest;
			RawFileReaderAr->Close();
			delete RawFileReaderAr;
			RawFileReaderAr = nullptr;

			UE_LOG(LogTemp, Log, TEXT("[RawData] 이름 %s 순번 %d"), *RawDataDest.Name, RawDataDest.Order);
		}
	}

	StudentSrc = NewObject<UStudent>();
	StudentSrc->SetName(TEXT("김동호"));
	StudentSrc->SetOrder(27);
	{
		const FString ObjectDataFileName(TEXT("ObjectData.bin"));
		FString ObjcetDataAbsolutePath = FPaths::Combine(*SavedDir, *ObjectDataFileName);
		FPaths::MakeStandardFilename(ObjcetDataAbsolutePath);

		/*
		 * 버퍼를 만들어주고, 아카이브를 해당 버퍼에 연결해준다.
		 * 이후 아카이브를 Serialize함수에 전달하여 Student객체의 데이터가 BufferArray에 직렬화된 상태로 저장된다
		 * 그 다음 파일로 버퍼를 저장한다.
		 */
		TArray<uint8> BufferArray;
		FMemoryWriter MemoryWriterAr(BufferArray);
		StudentSrc->Serialize(MemoryWriterAr);

		{
			TUniquePtr<FArchive> FileWriterAr = TUniquePtr<FArchive>(IFileManager::Get().CreateFileWriter(*ObjcetDataAbsolutePath));
			if (FileWriterAr != nullptr)
			{
				*FileWriterAr << BufferArray;
				FileWriterAr->Close();
			}
		}
		
		TArray<uint8> BufferArrayFromFile;
		{
			TUniquePtr<FArchive> FileReaderAr = TUniquePtr<FArchive>(IFileManager::Get().CreateFileReader(*ObjcetDataAbsolutePath));
			if (FileReaderAr != nullptr)
			{
				*FileReaderAr << BufferArrayFromFile;
				FileReaderAr->Close();
			}
		}

		FMemoryReader  MemoryReaderAr(BufferArrayFromFile);
		UStudent* StudentDest = NewObject<UStudent>();
		StudentDest->Serialize(MemoryReaderAr);

		StudentDest->PrintInfo(TEXT("ObjectData"));
	}

	{
		FString JsonDataFileName(TEXT("StudenJsonData.txt"));
		FString JsonDataAbsolutePath = FPaths::Combine(*SavedDir, *JsonDataFileName);
		FPaths::MakeStandardFilename(JsonDataAbsolutePath);

		// TSharedRef의 사용법. MakeShared()를 이용하여 초기화한다.
		TSharedRef<FJsonObject> JsonObjectSrc = MakeShared<FJsonObject>();
		FJsonObjectConverter::UStructToJsonObject(StudentSrc->GetClass(), StudentSrc, JsonObjectSrc);

		FString JsonOutString;
		TSharedRef<TJsonWriter<TCHAR>> JsonWriterAr = TJsonWriterFactory<TCHAR>::Create(&JsonOutString);
		if (FJsonSerializer::Serialize(JsonObjectSrc, JsonWriterAr))
		{
			// 성공했다면 json이 만들어졌따.
			FFileHelper::SaveStringToFile(JsonOutString, *JsonDataAbsolutePath);
		}

		FString JsonInString;
		FFileHelper::LoadFileToString(JsonInString, *JsonDataAbsolutePath);
		
		TSharedRef<TJsonReader<TCHAR>> JsonReaderAr = TJsonReaderFactory<TCHAR>::Create(JsonInString);
		//읽어지지 않으면 null이 올 수 있기 때문에 ref아닌 ptr 사용한다.
		TSharedPtr<FJsonObject> JsonObjectDest;
		if (FJsonSerializer::Deserialize(JsonReaderAr, JsonObjectDest))
		{
			UStudent* JsonStudentDest = NewObject<UStudent>();
			if (FJsonObjectConverter::JsonObjectToUStruct(JsonObjectDest.ToSharedRef(), JsonStudentDest->GetClass(), JsonStudentDest))
			{
				JsonStudentDest->PrintInfo(TEXT("JsonData"));
			}
		}
	}
		
}
