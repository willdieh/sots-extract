# sots-extract
Sword of the Stars 2 Image Extractor

[below is copied from my original post to Kerberos Productions Forums http://www.kerberos-productions.com/forums/viewtopic.php?f=50&t=57842]

Greetings,

I've been working on a project to extract the SOTS2 textures for a few weeks now and it feels like a good time to post it up for comment.
I know previous work by others had some limited success with extracting the 2D bitmaps used for Avatar and Badge images, but I've been able to decode the in-game ship and model textures as well and have created an executable to both extract and re-convert to SOTS2 format.

Background:
SOTS2 texture files generally have a .tga~ file extension and are stored in several different folders throughout the assets file structure.
Despite the .tga~ extension, these files are not stored in any sort of TGA format. Basic 2D bitmaps (used for splash screens, badges, avatars) are stored within the .tga~ container file in 32bit uncompressed RGBA format but more complicated in-game textures such as those used on 3D models are stored in compressed format with MIP Maps (lower resolution versions of the main image). Both types of files have a custom header and footer wrapping the raw data. The compression format used for 3D model textures is the DirectX compatible BC3 (aka DXT5) block compression algorithm that stores 16x16 pixel blocks of image data using only 16 bytes of data.

The utility is command line only. It will take .tga~ files and convert them to .DDS (Direct Draw Surface) files which can be edited using GIMP and the GIMP DDS plugin (a 32bit plugin, so install 32bit plugin support if using the 64bit version of GIMP).

DDS files are a fairly standard texture file format and I felt it was more appropriate to export to this format than say a standard Bitmap (.BMP) as the compressed 3d model texture files contain MIP Maps which cannot translate directly into flat 2D bitmaps without using an image editor like GIMP. The uncompressed 2d avatars/badges/splash screens are also exported in DDS format but can still be edited/converted in GIMP. If you choose not to use GIMP, I believe there are other free DDS viewers out there.

The utility offers the ability to convert a DDS file back into a .tga~ file which I've tested successfully by converting a ship texture file to DDS, editing it in GIMP, then converting the edited DDS file back to a SOTS2 compatible .tga~ file. The modified texture was visible in game (I put some kind of graffiti looking markings on my Morrigi command sections :)) More info can be found a couple of posts down.

I wanted to get this out for comment and see if anyone has any feedback on its usefulness. I really just wanted to share the file format information I've been able to determine so that perhaps it can inspire others. I'm completely new to these concepts and had to learn everything as I went. There may be others that are already familiar with DDS/BC3 formatted files and can contribute further.

The utility (extract.exe) has built-in command line help so just execute it in the command prompt for basic usage.

NOTICE, DISCLAIMER
This software is BETA and should be used at your own risk. Author makes no express or implied guarantee that this software will work without problems. It's written in C++ which is a relatively low-level language and performs file operations. A bug under these conditions may cause data corruption or other unexpected problems. Use at your own risk. :)


[Author's Production Notes]
Block Compression
https://msdn.microsoft.com/en-us/library/windows/desktop/bb694531%28v=vs.85%29.aspx

DDS Programming Guide
https://msdn.microsoft.com/en-us/library/windows/desktop/bb943982(v=vs.85).aspx

DXGI_FORMAT (lists the file type codes for DirectX)
https://msdn.microsoft.com/en-us/library/windows/desktop/bb173059(v=vs.85).aspx

Bitmap File Structure
https://en.wikipedia.org/wiki/BMP_file_format#DIB_header_(bitmap_information_header)

Mipmaps
https://en.wikipedia.org/wiki/Mipmap

Kerberos File Format Info Thread
http://www.kerberos-productions.com/forums/viewtopic.php?f=50&t=21858


[] = Decimal Conversion (DWORD/4 Bytes)
{} = Decimal Conversion (WORD/2 Bytes)
<> = Decimal Conversion (BYTE)
'' = Text 
NOTE!! All conversions from Hex to Decimal done based on Little-Endian storage!

DWORDn = Unknown DWORD
WORDn = Unknown WORD

Header Layout Disassembly
Badge TGA~
	-------------------------------------------------------------------------
	Format = Leading Signature (in some files), Standard Header, Data, Footer
	-------------------------------------------------------------------------
	
	LEADING SIGNATURE
	Some Non-EOF badges start with "0xFB 0xFC 0xFD 0xFE" + the following race specific value:	
	RACE	Hex 		WORD			DWORD
	Human 	0016 0300	5632 0003 		202240
	Hiver	ec15 0300	5612 0003 		202220
	Liir	f615 0300	5622 0003 		202230
	Morrigi	f615 0300	5622 0003 		202230
	Tarka 	ec15 0300	5612 0003 		202220
	Zuul	2a19 0300	6442 0003 		203050
	
	EOF Badges (Loa) have no leading signature (Does this mean it's irrelevant?)

	STANDARD HEADER
	NonLoa  = 'SHYL' + DWORD1 + <9> + 'Texture2D' + [28] + [256] + [256] + [1] + [1] + [0] + [262144]   + [1024] + [262144]   + Bytes 0x3E [62] forward = data
	NonLoa  = 'SHYL' + DWORD1 + <9> + 'Texture2D' + [28] + [512] + [512] + [1] + [1] + [0] + [1048576]  + [2048] + [1048576]  + Bytes 0x3E [62] forward = data
	Loa 	= 'SHYL' + DWORD1 + <9> + 'Texture2D' + [28] + [512] + [512] + [1] + [1] + [0] + [1048576]  + [2048] + [1048576]  + Bytes 0x36 [54] forward = data
	
	<9> = String Length
	[28] = file type code. 28 is a standard, uncompressed bitmap format. See DXGI_FORMAT link above
	[256]/[512] = width, height. Two sizes supported, 256x256 or 512x512?
	[1]/[1]/[0] = MIPMap count (1) followed by two unknown DWORDS (1, 0)
	[16384]/[1048576] = Number of Pixels?
	[1024]/[2048] = Pitch (bytes per line for uncompressed image)
	
	DWORD1 
	SOTS2 "Magic Number" -- No idea what this is...
	Some samples, converted to decimal (Little-Endian). Time stamp? Signature? Hash? Version Control? 
	
		RACE	Hex			WORD			DWORD
		Human	695C 22B5	23657 46370		3038927977 *Matches a Hiver Entry!?!
		Human	270C E5B3	03111 46053		3018132519
		Human	3026 24B5	09776 46372		3039045168
		Human	D540 26B5 	16597 46374		3039183061
		Human	06B7 42B5 	46854 46402		3041048326
		Human	966E 09B5	28310 46345		3037294230
		
		Hiver	695c 22b5	23657 46370		3038927977 *Matches a Human Entry!?!
		Hiver	18b7 7fb6	46872 46719		3061823256
		Hiver  	2c05 3db6	01324 46653		3057452332
		Hiver	778e 3cb7	36471 46908		3074199159
		Hiver	9102 1cb8	00657 47132		3088843409
		
		Liir	31e3 3eb5	58161 46398		3040797489
		Liir	8a32 10b6	12938 46608		3054514826
		Liir	5d34 d2b6	13405 46802		3067229277
		
		Loa		54e0 45bc	57428 48197		3158696020
		Loa		2271 34bc	28962 48180		3157553442
		Loa		0fea 2ebd 	59919 48430		3173968399
		Loa		9ed7 edbc	55198 48365		3169703838
	
	FOOTER
	27 DWORD (108 bytes) blank data

Avatar TGA~
	Format = Leading Signature (if not LOA), Standard Header, Data
	
	LEADING SIGNATURE
	All Non-EOF badges start with "0xFB 0xFC 0xFD 0xFE" + the following race specific value:	
	RACE	Hex 		WORD			DWORD
	Human 	0016 0300	5632 0003 		202240
	Hiver	ec15 0300	5612 0003 		202220
	Liir	f615 0300	5622 0003 		202230
	Morrigi	f615 0300	5622 0003 		202230
	Tarka 	ec15 0300	5612 0003 		202220
	Zuul	2a19 0300	6442 0003 		203050
	
	EOF Badges (Loa) have no leading signature (Does this mean it's irrelevant?)

	STANDARD HEADER
	All Races = 'SHYL' + DWORD1 + <9> + 'Texture2D' + [28] + [128] + [128] + [1] + [1] + [0] + [65536] + [512] + [65536] + Bytes 0x3E [62] forward = data?
	
	<9> = String Length
	[28] = file type code. 28 is a standard, uncompressed bitmap format. See DXGI_FORMAT link above
	[256]/[512] = width, height. Two sizes supported, 256x256 or 512x512?
	[1]/[1]/[0] = MIPMap count (1) followed by two unknown DWORDS (1, 0)
	[4096] = Number of Pixels
	[512] = Pitch (bytes per line for uncompressed image)
	
	DWORD1 
	Time stamp? Signature? Hash? Version Control?
	
	FOOTER
	27 DWORD (108 bytes) blank data
	
	All TGA format specific Headers/Footers are removed from the file. 32bit color (RGBA), 1 byte per color + 1 byte alpha. 
	
Damage Effect~
	Likely a rendered/animated scene of some kind. Probably not decodable without Kerberos pipeline/tools.
	Format = Standard Header, Data
	
	STANDARD HEADER 
	Examples:
	Hvy/Med Turret Destroyed, Larg/Med/Sm Blood Spurt, Smoke Stage01
	'SHYL' + DWORD1 + <6> + 'Effect' + [0] + [0] + [0] + [100] + [4] + <8> + 'NewNode0' + [0] + [0] + [0] + [0] + [100]
		
		Sample data after header from Hvy Turret Destroyed:
		[22] + [0] + [0] + [1] + [18] + [1] + [1] + [24] + [2] + [1] + [24] + [3] + [1] + [25] + [4] + [1] + [4] + [5] + [3] + [12] + [8] + [3] + [17] + [11] + [1] + [2] + [12] + [1] + [3] + [13] + [1] + [5] + [14] + [4] + [6] + [18] + [4] + [8] + [22] + [3] + [10] + [25]+ [3] + [11] + [28] + [1] + [13] + [29] + [3] + [14] + [32] + [3] + [15] + [35] + [3] + [16] + [38] + [3] + [29] + [41] + [1] + [30] + [42] + [1] + [33] + [43] + [3] + [46] + [45] ....

	Elect Damage, Sparks Lg Effect
	'SHYL' + DWORD1 + <6> + 'Effect' + [0] + [0] + [0] + [45] + [5] + <12> + 'Shock Sphere' + [0] + [0] + [2880] 
	
	DWORD1 
		Time stamp? Signature? Hash? Version Control?
		
	
Galaxy Effect~
	
	Format = Standard Header, Type 1 Data Block, Type 2 Data Block, Footer
	
	STANDARD HEADER
	
	Bar Galaxy
	'SHYL' + DWORD1 + <6> + 'Effect' + [0] + [0] + [0] + [100] + [11] + <12> + 'Dense Fuzz A' + [402]
	
	Spiral Galaxy
	'SHYL' + DWORD1 + <6> + 'Effect' + [0] + [0] + [0] + [100] + [10] + <5> + 'Stars' + [2748]
	
	<6> = String Length
	[0]/[0]/[0] = flags? x/y/z position/values?
	[100] = ????
	[10]/[11] = file type/item code?
	[402]/[2748] = offset to beginning of Type 2 Data Block
	
	Type 1 Data Block seems to contain a series of bytes with usually incremental values. Could be positional, frame, vertex, or color map data? Not 100% incremental but has an overall incremental pattern. No obvious data pairs X = 1, Y = 2, ... More like 1, 2, 2, 3, 4, 4, 5, 7
	Type 2 Data Block is very randomized. Could be raster/image data or some kind of RLE/block compressed format? Could be binary data? Within this block are references to function names related to geometric rendering.
	Type 2 Data Block seems to multiple sections, usually separated by rendering function names, but I can't find offset values to separate the chunks or any kind of data that would segment it...
	
	DWORD1 
		Time stamp? Signature? Hash? Version Control?
	
Textures TGA~
	
	STANDARD HEADER																		
	All Races = 'SHYL' + DWORD1 + <9> + 'Texture2D' + [77] + [16]   + [16]   + [5]  + [1] + [0] +   [256]     + [64]   + [256]     + [64]    + [32]  + [320]     + [16]   + [16]  + [336]     + [16]   + [16]  + [352]     + [16]  + [16] + [368] | Data (Blank.tga~, 586 bytes [110 byte header][368 byte data][108 byte footer])
	All Races = 'SHYL' + DWORD1 + <9> + 'Texture2D' + [77] + [32]   + [32]   + [6]  + [1] + [0] +   [1024]    + [128]  + [1024]    + [256]   + [64]  + [1280]    + [64]   + [32]  + [1344]    + [16]   + [16]  + [1360]    + [16]  + [16] + [1376]    + [16] + [16] + [1396] | Data
	All Races = 'SHYL' + DWORD1 + <9> + 'Texture2D' + [77] + [64]   + [64]   + [7]  + [1] + [0] +   [4096]    + [256]  + [4096]    + [1024]  + [128] + [5120]    + [256]  + [64]  + [5376]    + [64]   + [32]  + [5440]    + [16]  + [16] + [5456]    + [16] + [16] + [5472]    + [16] + [16] + [5488] | Data
	All Races = 'SHYL' + DWORD1 + <9> + 'Texture2D' + [77] + [128]  + [128]  + [8]  + [1] + [0] +   [16384]   + [512]  + [16384]   + [4096]  + [256] + [20480]   + [1024] + [128] + [21504]   + [256]  + [64]  + [21760]   + [64]  + [32] + [21824]   + [16] + [16] + [21840]   + [16] + [16] + [21856]   + [16] + [16] + [21872] | Data
	
	All Races = 'SHYL' + DWORD1 + <9> + 'Texture2D' + [77] + [256]  + [32]   + [9]  + [1] + [0] +   [8192]    + [1024] + [8192]    + [2048]  + [512] + [10240]   + [512]  + [256] + [10752]   + [128]  + [128] + [10880]   + [64]  + [64] + [10944]   + [32] + [32] + [10976]   + [16] + [16] + [10992]   + [16] + [16] + [11008]   + [16] + [16] + [11024] | Data (Tarka_Lights_02.tga~, 11290 bytes [158 byte header][11024 bytes data][108 byte footer]
	All Races = 'SHYL' + DWORD1 + <9> + 'Texture2D' + [77] + [256]  + [64]   + [9]  + [1] + [0] +   [16384]   + [1024] + [16384]   + [4096]  + [512] + [20480]   + [1024] + [256] + [21504]   + [256]  + [128] + [21760]   + [64]  + [64] + [21824]   + [32] + [32] + [21856]   + [16] + [16] + [21872]   + [16] + [16] + [21888]   + [16] + [16] + [21904] | Data (Hiver_Gate_Energy.tga~, 22170 bytes) [158 byte header][21904 byte data][108 byte footer]
	All Races = 'SHYL' + DWORD1 + <9> + 'Texture2D' + [77] + [256]  + [256]  + [9]  + [1] + [0] +   [65536]   + [1024] + [65536]   + [16384] + [512] + [81920]   + [4096] + [256] + [86016]   + [1024] + [128] + [87040]   + [256] + [64] + [87296]   + [64] + [32] + [87360]   + [16] + [16] + [87376]   + [16] + [16] + [87392]   + [16] + [16] + [87408] | Data (Tarka_Pulse_Yellow_Diffuse.tga~, 87666 bytes) [158 byte header][87392 byte data][108 byte footer]
	
	All Races = 'SHYL' + DWORD1 + <9> + 'Texture2D' + [77] + [512]  + [512]  + [10] + [1] + [0] +   [262144]  + [2048] + [262144] ... 
	All Races = 'SHYL' + DWORD1 + <9> + 'Texture2D' + [77] + [1024] + [1024] + [11] + [1] + [0] +   [1048576] + [4096] + [1048576] ...
	All Races = 'SHYL' + DWORD1 + <9> + 'Texture2D' + [77] + [2048] + [2048] + [12] + [1] + [0] +   [4194304] + [8192] + [4194304] ...
	
	<9> = String Length
	[77] = file type/item code?
	[64/128/256/512/1024/2048] = width, height
	[7/8/9/10/11/12] = No. of Mipmaps
	[1]/[0] = flags? terminating string?
	[256/512/1024/2048/4096/8192] = Bytes per row? 64px X 4bytes(32bit)=256. Used to calc Pitch (total / X = pitch)
	[4096/16384/65536/262144/1048576/4194304] = Number of Pixels? 64x64=4096

	
	DWORD1 
	Time stamp? Signature? Hash? Version Control?

	FOOTER
	27 DWORD (108 bytes) blank data
	
BC3 Data Structure 
	The data section of these files is stored in BC3 (DirectX Block Compression 3)/DXT5 format. The BC3 format is defined as follows:
	Algorithm applied to 16x16 blocks of pixels. 16 Bytes per 256 Pixels  	
	
	Alpha stored in first 8 bytes, Color stored in second 8 bytes.
	
	Byte	Desc
	1		Alpha A
	2		Alpha B
	3-8		Alpha Color Map (3 bits alpha map per pixel, 16 pixels)
	9-10	Color A (16bit RGB: Red=5bit, Green=6bit, Blue=5bit)
	11-12	Color B (16bit RGB: Red=5bit, Green=6bit, Blue=5bit)
	13-16	Color Map (2 bits color map per pixel, 16 pixels)
	
	If Alpha A > Alpha B, then Alpha Color Map has 6 alpha values, else Alpha Color Map has 4 alpha values and 100% alpha + 100% opaque 
	
	Color Map Values
	00 	= Color A
	01 	= Color B
	10 	= 2/3 * Color A + 1/3 * Color B
	11  = 1/3 * Color A + 2/3 * Color B
	
	Alpha Map Values
	000 = Alpha A 
	001 = Alpha B 
	010 = 6/7 * Alpha A + 1/7 Alpha B	or	4/5 * Alpha A + 1/5 Alpha B
	011 = 5/7 * Alpha A + 2/7 Alpha B	or 	3/5 * Alpha A + 2/5 Alpha B
	100 = 4/7 * Alpha A + 3/7 Alpha B	or 	2/5 * Alpha A + 3/5 Alpha B
	101 = 3/7 * Alpha A + 4/7 Alpha B	or  1/5 * Alpha A + 4/5 Alpha B
	110 = 2/7 * Alpha A + 5/7 Alpha B	or	0 (0% alpha)
	111 = 1/7 * Alpha A + 6/7 Alpha B 	or	255 (100% alpha)
	
	


DDS File Format
	(see DDS Programming Guide above for data structure layout)
	0x44 0x44 0x43 0x20 0x7c 0x00 0x00 0x00
	0x07 0x10 0x0A 0x00 (compressed+pitch+MIP maps) or 0x07 0x10 0x08 0x00 (compressed+pitch [no MIP maps])
	dword [height]
	dword [width]
	dword [Pitch or Total bytes in top level compressed texture?] Pitch = (width / 16) / 4 (for compressed)
	dword [depth, unused] 0x00 0x00 0x00 0x00
	dword [MIP Map Count]
	
	dword (x11) [reserved] 0x00 0x00 0x00 0x00 
	
	Pixel Format Section
	0x20 0x00 0x00 0x00
	0x04 (Compressed) or 0x41 (uncompressed ARGB) or 0x40 (uncompressed RGB) + 0x00 0x00 0x00
	dword [compression type: DXT1, DXT2, DXT3, DXT4 or DXT5) 
	dword [uncompressed RGB bit count]
	dword [uncompressed red bitmask]
	dword [uncompressed green bitmask]
	dword [uncompressed blue bitmask]
	dword [uncompressed alpha bitmask]
	
	dword [capabilities] 0x00 0x10 0x00 0x00 (no MIPMAP) or 0x08 0x10 0x40 0x00 (MIPMAP)
	dword [addl caps] only for cube textures
	dword [unused]
	dword [unused]
	dword [unused]
	


	
