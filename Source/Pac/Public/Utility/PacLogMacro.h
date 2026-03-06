#pragma once

// Declare the log category 
PAC_API DECLARE_LOG_CATEGORY_EXTERN(PacLog, Log, All); // Main log for the Pac Project
PAC_API DECLARE_LOG_CATEGORY_EXTERN(PacCharacterLog, Log, All); // Log For the Character
PAC_API DECLARE_LOG_CATEGORY_EXTERN(PacLogDebug, Log, All); // Temp Log for Debugging
PAC_API DECLARE_LOG_CATEGORY_EXTERN(PacLogEffect, Log, All); // Temp Log for Debugging


// Basic Log macros
#define PAC_ERROR(Format, ...) UE_LOG(PacLog, Error, TEXT(Format), ##__VA_ARGS__)
#define PAC_WARNING(Format, ...) UE_LOG(PacLog, Warning, TEXT(Format), ##__VA_ARGS__)
#define PAC_LOG(Format, ...) UE_LOG(PacLog, Log, TEXT(Format), ##__VA_ARGS__)

// Debug Log macros
#define PAC_DEBUG_LOG(Format, ...) UE_LOG(PacLogDebug, Log, TEXT(Format), ##__VA_ARGS__)
#define PAC_DEBUG_WARNING(Format, ...) UE_LOG(PacLogDebug, Warning, TEXT(Format), ##__VA_ARGS__)
#define PAC_DEBUG_ERROR(Format, ...) UE_LOG(PacLogDebug, Error, TEXT(Format), ##__VA_ARGS__)

// Log macros with category
#define PAC_LOG_CATEGORY(Category, Format, ...) UE_LOG(Category, Log, TEXT(Format), ##__VA_ARGS__)
#define PAC_WARNING_CATEGORY(Category, Format, ...) UE_LOG(Category, Warning, TEXT(Format), ##__VA_ARGS__)
#define PAC_ERROR_CATEGORY(Category, Format, ...) UE_LOG(Category, Error, TEXT(Format), ##__VA_ARGS__)

// On-screen log macros
#define LOG_SCREEN(Key, DisplayTime, Color, Format, ...) if(GEngine){ \
GEngine->AddOnScreenDebugMessage(Key, DisplayTime, Color, FString::Printf(TEXT(Format), ##__VA_ARGS__)); \
}

#define LOG_SCREEN_SIMPLE(Format, ...) if(GEngine){ \
GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT(Format), ##__VA_ARGS__)); \
}
