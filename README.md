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

