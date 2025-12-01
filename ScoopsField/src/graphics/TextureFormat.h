#pragma once

#include <stdint.h>
#include <stdlib.h>


// KTX
#define KTX_MAGIC       BX_MAKEFOURCC(0xAB, 'K', 'T', 'X')
#define KTX_HEADER_SIZE 64

#define KTX_ETC1_RGB8_OES                             0x8D64
#define KTX_COMPRESSED_R11_EAC                        0x9270
#define KTX_COMPRESSED_SIGNED_R11_EAC                 0x9271
#define KTX_COMPRESSED_RG11_EAC                       0x9272
#define KTX_COMPRESSED_SIGNED_RG11_EAC                0x9273
#define KTX_COMPRESSED_RGB8_ETC2                      0x9274
#define KTX_COMPRESSED_SRGB8_ETC2                     0x9275
#define KTX_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2  0x9276
#define KTX_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 0x9277
#define KTX_COMPRESSED_RGBA8_ETC2_EAC                 0x9278
#define KTX_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC          0x9279
#define KTX_COMPRESSED_RGB_PVRTC_4BPPV1_IMG           0x8C00
#define KTX_COMPRESSED_RGB_PVRTC_2BPPV1_IMG           0x8C01
#define KTX_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG          0x8C02
#define KTX_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG          0x8C03
#define KTX_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG          0x9137
#define KTX_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG          0x9138
#define KTX_COMPRESSED_RGB_S3TC_DXT1_EXT              0x83F0
#define KTX_COMPRESSED_RGBA_S3TC_DXT1_EXT             0x83F1
#define KTX_COMPRESSED_RGBA_S3TC_DXT3_EXT             0x83F2
#define KTX_COMPRESSED_RGBA_S3TC_DXT5_EXT             0x83F3
#define KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT       0x8C4D
#define KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT       0x8C4E
#define KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT       0x8C4F
#define KTX_COMPRESSED_LUMINANCE_LATC1_EXT            0x8C70
#define KTX_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT      0x8C72
#define KTX_COMPRESSED_RGBA_BPTC_UNORM_ARB            0x8E8C
#define KTX_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB      0x8E8D
#define KTX_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB      0x8E8E
#define KTX_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB    0x8E8F
#define KTX_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT          0x8A54
#define KTX_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT          0x8A55
#define KTX_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT    0x8A56
#define KTX_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT    0x8A57
#define KTX_ATC_RGB_AMD                               0x8C92
#define KTX_ATC_RGBA_EXPLICIT_ALPHA_AMD               0x8C93
#define KTX_ATC_RGBA_INTERPOLATED_ALPHA_AMD           0x87EE
#define KTX_COMPRESSED_RGBA_ASTC_4x4_KHR              0x93B0
#define KTX_COMPRESSED_RGBA_ASTC_5x4_KHR              0x93B1
#define KTX_COMPRESSED_RGBA_ASTC_5x5_KHR              0x93B2
#define KTX_COMPRESSED_RGBA_ASTC_6x5_KHR              0x93B3
#define KTX_COMPRESSED_RGBA_ASTC_6x6_KHR              0x93B4
#define KTX_COMPRESSED_RGBA_ASTC_8x5_KHR              0x93B5
#define KTX_COMPRESSED_RGBA_ASTC_8x6_KHR              0x93B6
#define KTX_COMPRESSED_RGBA_ASTC_8x8_KHR              0x93B7
#define KTX_COMPRESSED_RGBA_ASTC_10x5_KHR             0x93B8
#define KTX_COMPRESSED_RGBA_ASTC_10x6_KHR             0x93B9
#define KTX_COMPRESSED_RGBA_ASTC_10x8_KHR             0x93BA
#define KTX_COMPRESSED_RGBA_ASTC_10x10_KHR            0x93BB
#define KTX_COMPRESSED_RGBA_ASTC_12x10_KHR            0x93BC
#define KTX_COMPRESSED_RGBA_ASTC_12x12_KHR            0x93BD
#define KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR      0x93D0
#define KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR      0x93D1
#define KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR      0x93D2
#define KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR      0x93D3
#define KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR      0x93D4
#define KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR      0x93D5
#define KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR      0x93D6
#define KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR      0x93D7
#define KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR     0x93D8
#define KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR     0x93D9
#define KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR     0x93DA
#define KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR    0x93DB
#define KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR    0x93DC
#define KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR    0x93DD

#define KTX_A8                                        0x803C
#define KTX_R8                                        0x8229
#define KTX_R16                                       0x822A
#define KTX_RG8                                       0x822B
#define KTX_RG16                                      0x822C
#define KTX_R16F                                      0x822D
#define KTX_R32F                                      0x822E
#define KTX_RG16F                                     0x822F
#define KTX_RG32F                                     0x8230
#define KTX_RGBA8                                     0x8058
#define KTX_RGBA16                                    0x805B
#define KTX_RGBA16F                                   0x881A
#define KTX_R32UI                                     0x8236
#define KTX_RG32UI                                    0x823C
#define KTX_RGBA32UI                                  0x8D70
#define KTX_RGBA32F                                   0x8814
#define KTX_RGB565                                    0x8D62
#define KTX_RGBA4                                     0x8056
#define KTX_RGB5_A1                                   0x8057
#define KTX_RGB10_A2                                  0x8059
#define KTX_R8I                                       0x8231
#define KTX_R8UI                                      0x8232
#define KTX_R16I                                      0x8233
#define KTX_R16UI                                     0x8234
#define KTX_R32I                                      0x8235
#define KTX_R32UI                                     0x8236
#define KTX_RG8I                                      0x8237
#define KTX_RG8UI                                     0x8238
#define KTX_RG16I                                     0x8239
#define KTX_RG16UI                                    0x823A
#define KTX_RG32I                                     0x823B
#define KTX_RG32UI                                    0x823C
#define KTX_R8_SNORM                                  0x8F94
#define KTX_RG8_SNORM                                 0x8F95
#define KTX_RGB8_SNORM                                0x8F96
#define KTX_RGBA8_SNORM                               0x8F97
#define KTX_R16_SNORM                                 0x8F98
#define KTX_RG16_SNORM                                0x8F99
#define KTX_RGB16_SNORM                               0x8F9A
#define KTX_RGBA16_SNORM                              0x8F9B
#define KTX_SRGB8                                     0x8C41
#define KTX_SRGB8_ALPHA8                              0x8C43
#define KTX_RGBA32UI                                  0x8D70
#define KTX_RGB32UI                                   0x8D71
#define KTX_RGBA16UI                                  0x8D76
#define KTX_RGB16UI                                   0x8D77
#define KTX_RGBA8UI                                   0x8D7C
#define KTX_RGB8UI                                    0x8D7D
#define KTX_RGBA32I                                   0x8D82
#define KTX_RGB32I                                    0x8D83
#define KTX_RGBA16I                                   0x8D88
#define KTX_RGB16I                                    0x8D89
#define KTX_RGBA8I                                    0x8D8E
#define KTX_RGB8                                      0x8051
#define KTX_RGB8I                                     0x8D8F
#define KTX_RGB9_E5                                   0x8C3D
#define KTX_R11F_G11F_B10F                            0x8C3A

#define KTX_ZERO                                      0
#define KTX_RED                                       0x1903
#define KTX_ALPHA                                     0x1906
#define KTX_RGB                                       0x1907
#define KTX_RGBA                                      0x1908
#define KTX_BGRA                                      0x80E1
#define KTX_RG                                        0x8227

#define KTX_BYTE                                      0x1400
#define KTX_UNSIGNED_BYTE                             0x1401
#define KTX_SHORT                                     0x1402
#define KTX_UNSIGNED_SHORT                            0x1403
#define KTX_INT                                       0x1404
#define KTX_UNSIGNED_INT                              0x1405
#define KTX_FLOAT                                     0x1406
#define KTX_HALF_FLOAT                                0x140B
#define KTX_UNSIGNED_INT_5_9_9_9_REV                  0x8C3E
#define KTX_UNSIGNED_SHORT_5_6_5                      0x8363
#define KTX_UNSIGNED_SHORT_4_4_4_4                    0x8033
#define KTX_UNSIGNED_SHORT_5_5_5_1                    0x8034
#define KTX_UNSIGNED_INT_2_10_10_10_REV               0x8368
#define KTX_UNSIGNED_INT_10F_11F_11F_REV              0x8C3B


struct KtxFormatInfo
{
	uint32_t m_internalFmt;
	uint32_t m_internalFmtSrgb;
	uint32_t m_fmt;
	uint32_t m_type;
};

static const KtxFormatInfo s_translateKtxFormat[] =
{
	{ KTX_ZERO,                                     KTX_ZERO,                                       KTX_ZERO,                                     KTX_ZERO,                         }, // Unknown
	{ KTX_ALPHA,                                    KTX_ZERO,                                       KTX_ALPHA,                                    KTX_UNSIGNED_BYTE,                }, // A8
	{ KTX_R8,                                       KTX_ZERO,                                       KTX_RED,                                      KTX_UNSIGNED_BYTE,                }, // R8
	{ KTX_RG8,                                      KTX_ZERO,                                       KTX_RG,                                       KTX_UNSIGNED_BYTE,                }, // RG8
	{ KTX_RGBA8,                                    KTX_ZERO,                               KTX_RGBA,                                     KTX_UNSIGNED_BYTE,                }, // RGBA8
	{ KTX_R16,                                      KTX_ZERO,                                       KTX_RED,                                      KTX_UNSIGNED_SHORT,               }, // R16
	{ KTX_RG16,                                     KTX_ZERO,                                       KTX_RG,                                       KTX_UNSIGNED_SHORT,               }, // RG16
	{ KTX_RGBA16,                                   KTX_ZERO,                                       KTX_RGBA,                                     KTX_UNSIGNED_SHORT,               }, // RGBA16
	{ KTX_RGB10_A2,                                 KTX_ZERO,                                       KTX_RGBA,                                     KTX_UNSIGNED_INT_2_10_10_10_REV,  }, // RGB10A2
	{ KTX_RGB565,                                   KTX_ZERO,                                       KTX_RGB,                                      KTX_UNSIGNED_SHORT_5_6_5,         }, // B5G6R5
	{ KTX_RGB5_A1,                                  KTX_ZERO,                                       KTX_BGRA,                                     KTX_UNSIGNED_SHORT_5_5_5_1,       }, // BGR5A1
	{ KTX_RGBA4,                                    KTX_ZERO,                                       KTX_BGRA,                                     KTX_UNSIGNED_SHORT_4_4_4_4,       }, // BGRA4
	{ KTX_BGRA,                                     KTX_ZERO,                               KTX_BGRA,                                     KTX_UNSIGNED_BYTE,                }, // BGRA8

	{ KTX_COMPRESSED_RGBA_S3TC_DXT1_EXT,            KTX_ZERO,        KTX_RGBA,									  KTX_ZERO,                         }, // BC1
	{ KTX_COMPRESSED_RGBA_S3TC_DXT3_EXT,            KTX_ZERO,        KTX_RGBA,									  KTX_ZERO,                         }, // BC2
	{ KTX_COMPRESSED_RGBA_S3TC_DXT5_EXT,            KTX_ZERO,        KTX_RGBA,									  KTX_ZERO,                         }, // BC3
	{ KTX_COMPRESSED_LUMINANCE_LATC1_EXT,           KTX_ZERO,                                       KTX_RED,									  KTX_ZERO,                         }, // BC4
	{ KTX_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT,     KTX_ZERO,                                       KTX_RG,										  KTX_ZERO,                         }, // BC5
	{ KTX_COMPRESSED_RGBA_BPTC_UNORM_ARB,           KTX_ZERO,                                       KTX_RGBA,									  KTX_ZERO,                         }, // BC7

	{ KTX_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB,     KTX_ZERO,                                       KTX_RGB,									  KTX_ZERO,                         }, // BC6H
	{ KTX_ZERO,                                     KTX_ZERO,                                       KTX_ZERO,                                     KTX_ZERO,                         }, // Unknown

	{ KTX_R8,                                       KTX_ZERO,                                       KTX_RED,                                      KTX_UNSIGNED_BYTE,                }, // R8
	{ KTX_RG8,                                      KTX_ZERO,                                       KTX_RG,                                       KTX_UNSIGNED_BYTE,                }, // RG8
	{ KTX_RGBA8,                                    KTX_ZERO,                               KTX_RGBA,                                     KTX_UNSIGNED_BYTE,                }, // RGBA8
	{ KTX_R16,                                      KTX_ZERO,                                       KTX_RED,                                      KTX_UNSIGNED_SHORT,               }, // R16
	{ KTX_RG16,                                     KTX_ZERO,                                       KTX_RG,                                       KTX_UNSIGNED_SHORT,               }, // RG16
	{ KTX_RGBA16,                                   KTX_ZERO,                                       KTX_RGBA,                                     KTX_UNSIGNED_SHORT,               }, // RGBA16

	{ KTX_R16F,                                     KTX_ZERO,                                       KTX_RED,                                      KTX_HALF_FLOAT,                   }, // R16F
	{ KTX_RG16F,                                    KTX_ZERO,                                       KTX_RG,                                       KTX_FLOAT,                        }, // RG16F
	{ KTX_RGBA16F,                                  KTX_ZERO,                                       KTX_RGBA,                                     KTX_HALF_FLOAT,                   }, // RGBA16F
	{ KTX_R32F,                                     KTX_ZERO,                                       KTX_RED,                                      KTX_FLOAT,                        }, // R32F
	{ KTX_RG32F,                                    KTX_ZERO,                                       KTX_RG,                                       KTX_FLOAT,                        }, // RG32F
	{ KTX_RGBA32F,                                  KTX_ZERO,                                       KTX_RGBA,                                     KTX_FLOAT,                        }, // RGBA32F

	{ KTX_R11F_G11F_B10F,                           KTX_ZERO,                                       KTX_RGB,                                      KTX_UNSIGNED_INT_10F_11F_11F_REV, }, // RG11B10F

	{ KTX_R8UI,                                     KTX_ZERO,                                       KTX_RED,                                      KTX_UNSIGNED_BYTE,                }, // R8U
	{ KTX_RG8UI,                                    KTX_ZERO,                                       KTX_RG,                                       KTX_UNSIGNED_BYTE,                }, // RG8U
	{ KTX_RGBA8UI,                                  KTX_ZERO,                                       KTX_RGBA,                                     KTX_UNSIGNED_BYTE,                }, // RGBA8U
	{ KTX_R16UI,                                    KTX_ZERO,                                       KTX_RED,                                      KTX_UNSIGNED_SHORT,               }, // R16U
	{ KTX_RG16UI,                                   KTX_ZERO,                                       KTX_RG,                                       KTX_UNSIGNED_SHORT,               }, // RG16U
	{ KTX_RGBA16UI,                                 KTX_ZERO,                                       KTX_RGBA,                                     KTX_UNSIGNED_SHORT,               }, // RGBA16U
	{ KTX_R32UI,                                    KTX_ZERO,                                       KTX_RED,                                      KTX_UNSIGNED_INT,                 }, // R32U
	{ KTX_RG32UI,                                   KTX_ZERO,                                       KTX_RG,                                       KTX_UNSIGNED_INT,                 }, // RG32U
	{ KTX_RGBA32UI,                                 KTX_ZERO,                                       KTX_RGBA,                                     KTX_UNSIGNED_INT,                 }, // RGBA32U

	{ KTX_R8I,                                      KTX_ZERO,                                       KTX_RED,                                      KTX_BYTE,                         }, // R8I
	{ KTX_RG8I,                                     KTX_ZERO,                                       KTX_RG,                                       KTX_BYTE,                         }, // RG8I
	{ KTX_RGBA8I,                                   KTX_ZERO,                                       KTX_RGBA,                                     KTX_BYTE,                         }, // RGBA8I
	{ KTX_R16I,                                     KTX_ZERO,                                       KTX_RED,                                      KTX_SHORT,                        }, // R16I
	{ KTX_RG16I,                                    KTX_ZERO,                                       KTX_RG,                                       KTX_SHORT,                        }, // RG16I
	{ KTX_RGBA16I,                                  KTX_ZERO,                                       KTX_RGBA,                                     KTX_SHORT,                        }, // RGBA16I
	{ KTX_R32I,                                     KTX_ZERO,                                       KTX_RED,                                      KTX_INT,                          }, // R32I
	{ KTX_RG32I,                                    KTX_ZERO,                                       KTX_RG,                                       KTX_INT,                          }, // RG32I
	{ KTX_RGBA32I,                                  KTX_ZERO,                                       KTX_RGBA,                                     KTX_INT,                          }, // RGBA32I

	{ KTX_RGBA8,                                    KTX_SRGB8_ALPHA8,                               KTX_RGBA,                                     KTX_UNSIGNED_BYTE,                }, // RGBA8 SRGB
	{ KTX_BGRA,                                    KTX_SRGB8_ALPHA8,                               KTX_BGRA,                                     KTX_UNSIGNED_BYTE,}, // BGRA8 SRGB

	{ KTX_COMPRESSED_RGBA_S3TC_DXT1_EXT,            KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,        KTX_RGBA,									  KTX_ZERO,                         }, // BC1
	{ KTX_COMPRESSED_RGBA_S3TC_DXT3_EXT,            KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,        KTX_RGBA,									  KTX_ZERO,                         }, // BC2
	{ KTX_COMPRESSED_RGBA_S3TC_DXT5_EXT,            KTX_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,        KTX_RGBA,									  KTX_ZERO,                         }, // BC3
	{ KTX_COMPRESSED_RGBA_BPTC_UNORM_ARB,           KTX_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB,       KTX_RGBA,									  KTX_ZERO,                         }, // BC7

	{ KTX_ZERO,           KTX_ZERO,       KTX_ZERO,									  KTX_ZERO,                         }, // 
	{ KTX_ZERO,           KTX_ZERO,       KTX_ZERO,									  KTX_ZERO,                         }, // 
	{ KTX_ZERO,           KTX_ZERO,       KTX_ZERO,									  KTX_ZERO,                         }, // 
	{ KTX_ZERO,           KTX_ZERO,       KTX_ZERO,									  KTX_ZERO,                         }, // 
	{ KTX_ZERO,           KTX_ZERO,       KTX_ZERO,									  KTX_ZERO,                         }, // 

	{ KTX_COMPRESSED_RGBA_ASTC_4x4_KHR,             KTX_ZERO,       KTX_RGBA,									  KTX_ZERO,                         }, // ASTC4x4
	{ KTX_COMPRESSED_RGBA_ASTC_5x4_KHR,             KTX_ZERO,       KTX_RGBA,									  KTX_ZERO,                         }, // ASTC5x4
	{ KTX_COMPRESSED_RGBA_ASTC_5x5_KHR,             KTX_ZERO,       KTX_RGBA,									  KTX_ZERO,                         }, // ASTC5x5
	{ KTX_COMPRESSED_RGBA_ASTC_6x5_KHR,             KTX_ZERO,       KTX_RGBA,									  KTX_ZERO,                         }, // ASTC6x5
	{ KTX_COMPRESSED_RGBA_ASTC_6x6_KHR,             KTX_ZERO,       KTX_RGBA,									  KTX_ZERO,                         }, // ASTC6x6
	{ KTX_COMPRESSED_RGBA_ASTC_8x5_KHR,             KTX_ZERO,       KTX_RGBA,									  KTX_ZERO,                         }, // ASTC8x5
	{ KTX_COMPRESSED_RGBA_ASTC_8x6_KHR,             KTX_ZERO,       KTX_RGBA,									  KTX_ZERO,                         }, // ASTC8x6
	{ KTX_COMPRESSED_RGBA_ASTC_8x8_KHR,             KTX_ZERO,       KTX_RGBA,									  KTX_ZERO,                         }, // ASTC8x8
	{ KTX_COMPRESSED_RGBA_ASTC_10x5_KHR,            KTX_ZERO,      KTX_RGBA,									  KTX_ZERO,                         }, // ASTC10x5
	{ KTX_COMPRESSED_RGBA_ASTC_10x6_KHR,            KTX_ZERO,      KTX_RGBA,									  KTX_ZERO,                         }, // ASTC10x6
	{ KTX_COMPRESSED_RGBA_ASTC_10x8_KHR,            KTX_ZERO,      KTX_RGBA,									  KTX_ZERO,                         }, // ASTC10x8
	{ KTX_COMPRESSED_RGBA_ASTC_10x10_KHR,           KTX_ZERO,     KTX_RGBA,									  KTX_ZERO,                         }, // ASTC10x10
	{ KTX_COMPRESSED_RGBA_ASTC_12x10_KHR,           KTX_ZERO,     KTX_RGBA,									  KTX_ZERO,                         }, // ASTC12x10
	{ KTX_COMPRESSED_RGBA_ASTC_12x12_KHR,           KTX_ZERO,     KTX_RGBA,									  KTX_ZERO,                         }, // ASTC12x12

	{ KTX_COMPRESSED_RGBA_ASTC_4x4_KHR,             KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR,       KTX_RGBA,									  KTX_ZERO,                         }, // ASTC4x4
	{ KTX_COMPRESSED_RGBA_ASTC_5x4_KHR,             KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR,       KTX_RGBA,									  KTX_ZERO,                         }, // ASTC5x4
	{ KTX_COMPRESSED_RGBA_ASTC_5x5_KHR,             KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR,       KTX_RGBA,									  KTX_ZERO,                         }, // ASTC5x5
	{ KTX_COMPRESSED_RGBA_ASTC_6x5_KHR,             KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR,       KTX_RGBA,									  KTX_ZERO,                         }, // ASTC6x5
	{ KTX_COMPRESSED_RGBA_ASTC_6x6_KHR,             KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR,       KTX_RGBA,									  KTX_ZERO,                         }, // ASTC6x6
	{ KTX_COMPRESSED_RGBA_ASTC_8x5_KHR,             KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR,       KTX_RGBA,									  KTX_ZERO,                         }, // ASTC8x5
	{ KTX_COMPRESSED_RGBA_ASTC_8x6_KHR,             KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR,       KTX_RGBA,									  KTX_ZERO,                         }, // ASTC8x6
	{ KTX_COMPRESSED_RGBA_ASTC_8x8_KHR,             KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR,       KTX_RGBA,									  KTX_ZERO,                         }, // ASTC8x8
	{ KTX_COMPRESSED_RGBA_ASTC_10x5_KHR,            KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR,      KTX_RGBA,									  KTX_ZERO,                         }, // ASTC10x5
	{ KTX_COMPRESSED_RGBA_ASTC_10x6_KHR,            KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR,      KTX_RGBA,									  KTX_ZERO,                         }, // ASTC10x6
	{ KTX_COMPRESSED_RGBA_ASTC_10x8_KHR,            KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR,      KTX_RGBA,									  KTX_ZERO,                         }, // ASTC10x8
	{ KTX_COMPRESSED_RGBA_ASTC_10x10_KHR,           KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR,     KTX_RGBA,									  KTX_ZERO,                         }, // ASTC10x10
	{ KTX_COMPRESSED_RGBA_ASTC_12x10_KHR,           KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR,     KTX_RGBA,									  KTX_ZERO,                         }, // ASTC12x10
	{ KTX_COMPRESSED_RGBA_ASTC_12x12_KHR,           KTX_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR,     KTX_RGBA,									  KTX_ZERO,                         }, // ASTC12x12

	{ KTX_ZERO,             KTX_ZERO,       KTX_ZERO,									  KTX_ZERO, }, // ASTC4x4
	{ KTX_ZERO,             KTX_ZERO,       KTX_ZERO,									  KTX_ZERO, }, // ASTC5x4
	{ KTX_ZERO,             KTX_ZERO,       KTX_ZERO,									  KTX_ZERO, }, // ASTC5x5
	{ KTX_ZERO,             KTX_ZERO,       KTX_ZERO,									  KTX_ZERO, }, // ASTC6x5
	{ KTX_ZERO,             KTX_ZERO,       KTX_ZERO,									  KTX_ZERO, }, // ASTC6x6
	{ KTX_ZERO,             KTX_ZERO,       KTX_ZERO,									  KTX_ZERO, }, // ASTC8x5
	{ KTX_ZERO,             KTX_ZERO,       KTX_ZERO,									  KTX_ZERO, }, // ASTC8x6
	{ KTX_ZERO,             KTX_ZERO,       KTX_ZERO,									  KTX_ZERO, }, // ASTC8x8
	{ KTX_ZERO,            KTX_ZERO,      KTX_ZERO,									  KTX_ZERO, }, // ASTC10x5
	{ KTX_ZERO,            KTX_ZERO,      KTX_ZERO,									  KTX_ZERO, }, // ASTC10x6
	{ KTX_ZERO,            KTX_ZERO,      KTX_ZERO,									  KTX_ZERO, }, // ASTC10x8
	{ KTX_ZERO,           KTX_ZERO,     KTX_ZERO,									  KTX_ZERO, }, // ASTC10x10
	{ KTX_ZERO,           KTX_ZERO,     KTX_ZERO,									  KTX_ZERO, }, // ASTC12x10
	{ KTX_ZERO,           KTX_ZERO,     KTX_ZERO,									  KTX_ZERO,                         }, // ASTC12x12
};

struct KtxFormatInfo2
{
	uint32_t m_internalFmt;
	SDL_GPUTextureFormat m_format;
};

static const KtxFormatInfo2 s_translateKtxFormat2[] =
{
	{ KTX_A8,                           SDL_GPU_TEXTUREFORMAT_A8_UNORM    },
	{ KTX_RED,                          SDL_GPU_TEXTUREFORMAT_INVALID    },
	{ KTX_RGB,                          SDL_GPU_TEXTUREFORMAT_INVALID  },
	{ KTX_RGBA,                         SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM },
	{ KTX_COMPRESSED_RGB_S3TC_DXT1_EXT, SDL_GPU_TEXTUREFORMAT_BC1_RGBA_UNORM   },
};

static SDL_GPUTextureFormat TranslateTextureFormat(uint32_t internalFormat, uint32_t baseInternalFormat)
{
	SDL_GPUTextureFormat format = SDL_GPU_TEXTUREFORMAT_INVALID;
	bool hasAlpha = false;
	bool srgb = false;

	for (uint32_t ii = 0; ii < _countof(s_translateKtxFormat); ++ii)
	{
		if (s_translateKtxFormat[ii].m_internalFmt == internalFormat)
		{
			format = (SDL_GPUTextureFormat)ii;
			break;
		}

		if (s_translateKtxFormat[ii].m_internalFmtSrgb == internalFormat
			&& s_translateKtxFormat[ii].m_fmt == baseInternalFormat)
		{
			format = (SDL_GPUTextureFormat)ii;
			srgb = true;
			break;
		}
	}

	if (format == SDL_GPU_TEXTUREFORMAT_INVALID)
	{
		for (uint32_t ii = 0; ii < _countof(s_translateKtxFormat2); ++ii)
		{
			if (s_translateKtxFormat2[ii].m_internalFmt == internalFormat)
			{
				format = s_translateKtxFormat2[ii].m_format;
				break;
			}
		}
	}

	return format;
}


static SDL_GPUTextureFormat vkTextureFormatTranslation[185]
{
	SDL_GPU_TEXTUREFORMAT_INVALID,

	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R4G4_UNORM_PACK8,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R4G4B4A4_UNORM_PACK16,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_B4G4R4A4_UNORM_PACK16,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R5G6B5_UNORM_PACK16,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_B5G6R5_UNORM_PACK16,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R5G5B5A1_UNORM_PACK16,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_B5G5R5A1_UNORM_PACK16,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A1R5G5B5_UNORM_PACK16,
	SDL_GPU_TEXTUREFORMAT_R8_UNORM, //VK_FORMAT_R8_UNORM,
	SDL_GPU_TEXTUREFORMAT_R8_SNORM, //VK_FORMAT_R8_SNORM,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R8_USCALED,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R8_SSCALED,
	SDL_GPU_TEXTUREFORMAT_R8_UINT, //VK_FORMAT_R8_UINT,
	SDL_GPU_TEXTUREFORMAT_R8_INT, //VK_FORMAT_R8_SINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R8_SRGB,
	SDL_GPU_TEXTUREFORMAT_R8G8_UNORM, //VK_FORMAT_R8G8_UNORM,
	SDL_GPU_TEXTUREFORMAT_R8G8_SNORM, //VK_FORMAT_R8G8_SNORM,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R8G8_USCALED,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R8G8_SSCALED,
	SDL_GPU_TEXTUREFORMAT_R8G8_UINT, //VK_FORMAT_R8G8_UINT,
	SDL_GPU_TEXTUREFORMAT_R8G8_INT, //VK_FORMAT_R8G8_SINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R8G8_SRGB,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R8G8B8_UNORM,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R8G8B8_SNORM,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R8G8B8_USCALED,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R8G8B8_SSCALED,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R8G8B8_UINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R8G8B8_SINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R8G8B8_SRGB,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_B8G8R8_UNORM,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_B8G8R8_SNORM,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_B8G8R8_USCALED,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_B8G8R8_SSCALED,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_B8G8R8_UINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_B8G8R8_SINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_B8G8R8_SRGB,
	SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, //VK_FORMAT_R8G8B8A8_UNORM,
	SDL_GPU_TEXTUREFORMAT_R8G8B8A8_SNORM, //VK_FORMAT_R8G8B8A8_SNORM,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R8G8B8A8_USCALED,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R8G8B8A8_SSCALED,
	SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UINT, //VK_FORMAT_R8G8B8A8_UINT,
	SDL_GPU_TEXTUREFORMAT_R8G8B8A8_INT, //VK_FORMAT_R8G8B8A8_SINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R8G8B8A8_SRGB,
	SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM, //VK_FORMAT_B8G8R8A8_UNORM,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_B8G8R8A8_SNORM,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_B8G8R8A8_USCALED,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_B8G8R8A8_SSCALED,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_B8G8R8A8_UINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_B8G8R8A8_SINT,
	SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB, //VK_FORMAT_B8G8R8A8_SRGB,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A8B8G8R8_UNORM_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A8B8G8R8_SNORM_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A8B8G8R8_USCALED_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A8B8G8R8_UINT_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A8B8G8R8_SINT_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A8B8G8R8_SRGB_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A2R10G10B10_UNORM_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A2R10G10B10_SNORM_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A2R10G10B10_USCALED_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A2R10G10B10_UINT_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A2R10G10B10_SINT_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A2B10G10R10_UNORM_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A2B10G10R10_SNORM_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A2B10G10R10_USCALED_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A2B10G10R10_UINT_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_A2B10G10R10_SINT_PACK32,
	SDL_GPU_TEXTUREFORMAT_R16_UNORM, //VK_FORMAT_R16_UNORM,
	SDL_GPU_TEXTUREFORMAT_R16_SNORM, //VK_FORMAT_R16_SNORM,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R16_USCALED,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R16_SSCALED,
	SDL_GPU_TEXTUREFORMAT_R16_UINT, //VK_FORMAT_R16_UINT,
	SDL_GPU_TEXTUREFORMAT_R16_INT, //VK_FORMAT_R16_SINT,
	SDL_GPU_TEXTUREFORMAT_R16_FLOAT, //VK_FORMAT_R16_SFLOAT,
	SDL_GPU_TEXTUREFORMAT_R16G16_UNORM, //VK_FORMAT_R16G16_UNORM,
	SDL_GPU_TEXTUREFORMAT_R16G16_SNORM, //VK_FORMAT_R16G16_SNORM,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R16G16_USCALED,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R16G16_SSCALED,
	SDL_GPU_TEXTUREFORMAT_R16G16_UINT, //VK_FORMAT_R16G16_UINT,
	SDL_GPU_TEXTUREFORMAT_R16G16_INT, //VK_FORMAT_R16G16_SINT,
	SDL_GPU_TEXTUREFORMAT_R16G16_FLOAT, //VK_FORMAT_R16G16_SFLOAT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R16G16B16_UNORM,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R16G16B16_SNORM,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R16G16B16_USCALED,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R16G16B16_SSCALED,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R16G16B16_UINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R16G16B16_SINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R16G16B16_SFLOAT,
	SDL_GPU_TEXTUREFORMAT_R16G16B16A16_UNORM, //VK_FORMAT_R16G16B16A16_UNORM,
	SDL_GPU_TEXTUREFORMAT_R16G16B16A16_SNORM, //VK_FORMAT_R16G16B16A16_SNORM,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R16G16B16A16_USCALED,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R16G16B16A16_SSCALED,
	SDL_GPU_TEXTUREFORMAT_R16G16B16A16_UINT, //VK_FORMAT_R16G16B16A16_UINT,
	SDL_GPU_TEXTUREFORMAT_R16G16B16A16_INT, //VK_FORMAT_R16G16B16A16_SINT,
	SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT, //VK_FORMAT_R16G16B16A16_SFLOAT,
	SDL_GPU_TEXTUREFORMAT_R32_UINT, //VK_FORMAT_R32_UINT,
	SDL_GPU_TEXTUREFORMAT_R32_INT, //VK_FORMAT_R32_SINT,
	SDL_GPU_TEXTUREFORMAT_R32_FLOAT, //VK_FORMAT_R32_SFLOAT,
	SDL_GPU_TEXTUREFORMAT_R32G32_UINT, //VK_FORMAT_R32G32_UINT,
	SDL_GPU_TEXTUREFORMAT_R32G32_INT, //VK_FORMAT_R32G32_SINT,
	SDL_GPU_TEXTUREFORMAT_R32G32_FLOAT, //VK_FORMAT_R32G32_SFLOAT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R32G32B32_UINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R32G32B32_SINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R32G32B32_SFLOAT,
	SDL_GPU_TEXTUREFORMAT_R32G32B32A32_UINT, //VK_FORMAT_R32G32B32A32_UINT,
	SDL_GPU_TEXTUREFORMAT_R32G32B32A32_INT, //VK_FORMAT_R32G32B32A32_SINT,
	SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT, //VK_FORMAT_R32G32B32A32_SFLOAT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R64_UINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R64_SINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R64_SFLOAT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R64G64_UINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R64G64_SINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R64G64_SFLOAT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R64G64B64_UINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R64G64B64_SINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R64G64B64_SFLOAT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R64G64B64A64_UINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R64G64B64A64_SINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_R64G64B64A64_SFLOAT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_B10G11R11_UFLOAT_PACK32,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
	SDL_GPU_TEXTUREFORMAT_D16_UNORM, //VK_FORMAT_D16_UNORM,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_X8_D24_UNORM_PACK32,
	SDL_GPU_TEXTUREFORMAT_D32_FLOAT, //VK_FORMAT_D32_SFLOAT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_S8_UINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_D16_UNORM_S8_UINT,
	SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT, //VK_FORMAT_D24_UNORM_S8_UINT,
	SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT, //VK_FORMAT_D32_SFLOAT_S8_UINT,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_BC1_RGB_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_BC1_RGB_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_BC1_RGBA_UNORM, //VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_BC1_RGBA_UNORM_SRGB, //VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_BC2_RGBA_UNORM, //VK_FORMAT_BC2_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_BC2_RGBA_UNORM_SRGB, //VK_FORMAT_BC2_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM, //VK_FORMAT_BC3_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM_SRGB, //VK_FORMAT_BC3_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_BC4_R_UNORM, //VK_FORMAT_BC4_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_BC4_SNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_BC5_RG_UNORM, //VK_FORMAT_BC5_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_BC5_SNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_BC6H_RGB_UFLOAT, //VK_FORMAT_BC6H_UFLOAT_BLOCK,
	SDL_GPU_TEXTUREFORMAT_BC6H_RGB_FLOAT, //VK_FORMAT_BC6H_SFLOAT_BLOCK,
	SDL_GPU_TEXTUREFORMAT_BC7_RGBA_UNORM, //VK_FORMAT_BC7_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_BC7_RGBA_UNORM_SRGB, //VK_FORMAT_BC7_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_EAC_R11_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_EAC_R11_SNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_INVALID, //VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_4x4_UNORM, //VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_4x4_UNORM_SRGB, //VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_5x4_UNORM, //VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_5x4_UNORM_SRGB, //VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_5x5_UNORM, //VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_5x5_UNORM_SRGB, //VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_6x5_UNORM, //VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_6x5_UNORM_SRGB, //VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_6x6_UNORM, //VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_6x6_UNORM_SRGB, //VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_8x5_UNORM, //VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_8x5_UNORM_SRGB, //VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_8x6_UNORM, //VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_8x6_UNORM_SRGB, //VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_8x8_UNORM, //VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_8x8_UNORM_SRGB, //VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x5_UNORM, //VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x5_UNORM_SRGB, //VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x6_UNORM, //VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x6_UNORM_SRGB, //VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x8_UNORM, //VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x8_UNORM_SRGB, //VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x10_UNORM, //VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_10x10_UNORM_SRGB, //VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_12x10_UNORM, //VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_12x10_UNORM_SRGB, //VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_12x12_UNORM, //VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
	SDL_GPU_TEXTUREFORMAT_ASTC_12x12_UNORM_SRGB, //VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
};
