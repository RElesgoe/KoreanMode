// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the KOREANMODE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// KOREANMODE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef KOREANMODE_EXPORTS
#define KOREANMODE_API __declspec(dllexport)
#else
#define KOREANMODE_API __declspec(dllimport)
#endif

// This class is exported from the KoreanMode.dll
class KOREANMODE_API CKoreanMode {
public:
	CKoreanMode(void);
	// TODO: add your methods here.
};

extern KOREANMODE_API int nKoreanMode;

KOREANMODE_API int fnKoreanMode(void);
