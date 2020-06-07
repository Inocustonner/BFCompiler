#pragma once
#include "types.hpp"

struct DataDirectory
{
	dword VirtualAddress;
	dword Size;

	constexpr static size_t size()
	{
		return sizeof(DataDirectory);
	}

	constexpr static size_t dataDirectoriesAvailable = 16;
};


struct PEOptionalHeader32
{
	enum DllCharacteristic : word
	{
		HightEntropyVa                = 0x0020, // Image can handle a high entropy 64-bit virtual address space.
		DynamicBase                   = 0x0040, // DLL can be relocated at load time.
		ForceIntegrity                = 0x0080, // Code Integrity checks are enforced
		CompatibleWithNX              = 0x0100, // Image is NX compatible.
		NoIsolation                   = 0x0200, // Isolation aware, but do not isolate the image.
		NoStructuredExceptionHandling = 0x0400, // Does not use structured exception (SE) handling.
		DoNotBindImage                = 0x0800, // Do not bind the image.
		AppContainer                  = 0x1000, // Image must execute in an AppContainer.
		WdmDRIVER                     = 0x2000, // A WDM driver.
		ControlFlowGuardSupport       = 0x4000, // Image supports Control Flow Guard.
		TerminalServerAware           = 0x8000, // Terminal Server aware.
	};

	enum Subsystem : word
	{
		Unknown                = 0,  // An unknown subsystem
		Native                 = 1,  // Device drivers and native Windows processes
		WindowsGui             = 2,  // The Windows graphical user interface (GUI) subsystem
		WindowsCui             = 3,  // The Windows character subsystem
		Os2Cui                 = 5,  // The OS/2 character subsystem
		PosixCui               = 7,  // The Posix character subsystem
		NativeWindows          = 8,  // Native Win9x driver
		WindowsCeGui           = 9,  // Windows CE
		EfiApplication         = 10, // Extensible Firmware Interface (EFI) application
		EfiBootServiceDriver   = 11, // EFI driver with boot services
		EfiRuntimeDriver       = 12, // EFI driver with run-time services
		EfiRom                 = 13, // EFI ROM image
		XBox                   = 14, // XBOX
		WindowsBootApplication = 16  // Windows boot application.
	};

	enum DataDirectoryTableId : dword
	{
		Export                = 0,
		Import                = 1,
		Resource              = 2,
		Exception             = 3,
		Certificate           = 4,
		BaseRelocation        = 5,
		Debug                 = 6,
		Architecture          = 7,
		GlobalPtr             = 8,
		TlsTable              = 9,
		LoadConfig            = 10,
		BoundImport           = 11,
		IAT                   = 12,
		DelayImportDescriptor = 13,
		ClrRuntimeHeader      = 14
	};

	word magicNumber = 0x10b;
	byte majorLinkerVersion;
	byte minorLinkerVersion;
	dword sizeOfCode;			// size of .text section
	dword sizeOfInitializedData;
	dword sizeOfUninitializeData;
	dword addressOfEntryPoint;	// The address of entry point relative to the image base when the executable file is loaded into memory
	dword baseOfCode;			// The address of start of .text section relative to the image base when the executable file is loaded into memory
	dword baseOfData;

	dword imageBase = 0x00400000; // Prefered address of the first byte of the image when is loaded into memory; multiple of 64Kb
	dword sectionAlignment;		  // alignment of section when they loaded in memory. The default is the page size. Should be greater than fileAlignment
	dword fileAlignment;		  // If the sectionAlignment is less than the architecture's page size, then fileAlignment must match sectionAlignment.
	word majorOperatingSystemVersion;
	word minorOperatingSystemVersion;
	word majorImageVersion;
	word minorImageVersion;
	word majorSubsystemVersion;
	word minorSubsystemVersion;
	dword win32VersionValue = 0; // reserved
	dword sizeOfImage;			 // size of image when loaded in memory. Must be multiple of sectionAlignment;
	dword sizeOfHeaders;		 // The combined size of an MS-DOS stub, PE header, and section headers rounded up to a multiple of fileAlignment.
	dword checkSum;
	word subsystem;
	word dllCharacteristics;
	dword sizeOfStackReserve;
	dword sizeOfStackCommit;
	dword sizeOfHeapReserve;
	dword sizeOfHeapCommit;
	dword loaderFlags = 0;		// reserved
	dword numberOfRvaAndSizes = DataDirectory::dataDirectoriesAvailable;	// The number of data-directory entries in the remainder of the optional header. Currently in windows available 16 data directories
	DataDirectory dataDirectories[DataDirectory::dataDirectoriesAvailable];

	constexpr static size_t size()
	{
		return sizeof(PEOptionalHeader32);
	}
};
