#ifndef VESA_H
#define VESA_H
typedef struct
{
    u16   ModeAttributes;        
    u8    WinAAttributes;        
    u8    WinBAttributes;         
    u16   WinGranularity;         
    u16   WinSize;               
    u16   WinASegment;            
    u16   WinBSegment;           

    void*      WinFuncPtr;             
    u16   BytesPerScanLine;      
    u16   XResolution;           
    u16   YResolution;            
    u8    XCharSize;             
    u8    YCharSize;              
    u8    NumberOfPlanes;         
    u8    BitsPerPixel;           
    u8    NumberOfBanks;         
    u8    MemoryModel;            
    u8    BankSize;               
    u8    NumberOfImagePages;     
    u8    res1;                   
    u8    RedMaskSize;          
    u8    RedFieldPosition;       
    u8    GreenMaskSize;         
    u8    GreenFieldPosition;    
    u8    BlueMaskSize;           
    u8    BlueFieldPosition;    
    u8    RsvdMaskSize;           
    u8    RsvdFieldPosition;      
    u8    DirectColorModeInfo;    
    uintptr_t  PhysBasePtr;          
    u32   OffScreenMemOffset;     
    u16   OffScreenMemSize;      
    u8    res2[206];              
} __attribute__((packed)) VESA_MODE_INFO;

typedef struct
{
    u8   VESASignature[4];
    u16  VESAVersion;
    uintptr_t OEMStringPtr;
    u8   Capabilities[4];
    u16* VideoModes;
    u16  TotalMemory;
    u16  OemSoftwareRev;
    uintptr_t OemVendorNamePtr;
    uintptr_t OemProductNamePtr;
    uintptr_t OemProductRevPtr;
    u8   Reserved[222];
    u8   OemData[256];
} __attribute__ ((packed)) VGA_INFO;
extern VESA_MODE_INFO mib;

//void _init_VESA(DIR *root_);
#endif
